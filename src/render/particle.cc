#include "particle.h"

ParticleGenerator::ParticleGenerator(glm::vec2 scale, Shader shader, Texture2D texture, unsigned amount) 
    : scale_(scale), shader_(shader), texture_(texture), amount_(amount)
{
    init();
}

ParticleGenerator::~ParticleGenerator(){
    delete(instance_data_);
}

void ParticleGenerator::Update(float dt, GameObject& object, unsigned int new_particles, glm::vec2 offset){
    // 每一帧新产生粒子
    for(unsigned int i = 0; i < new_particles; i ++){
        int unused_particle = firstUnusedParticle();
        if(unused_particle == -1)
            break;
        respawnParticle(particles_[unused_particle], object, offset);
    }

    for(unsigned int i = 0; i < amount_; i ++){
        Particle& p = particles_[i];
        p.life_ -= dt;  // 减少粒子生命值
        if(p.life_ > 0.0f){  // 如果粒子还存活
            p.position_ -= p.velocity_ * dt;  // 移动粒子
            p.color_.a -= dt * 2.5;  // 粒子逐渐变透明
        }
    }
}

// 储存上一个找到的已消亡粒子的位置，从这个位置开始查找可以加快查找速度，因为下一个消亡的粒子总是在上一个消亡粒子的右边
unsigned last_used_particle = 0;
unsigned int ParticleGenerator::firstUnusedParticle() {
    for(unsigned int i = last_used_particle; i < amount_; i ++){
        if(particles_[i].life_ <= 0.0f){
            last_used_particle = i;
            return i;
        }
    }

    for(unsigned int i = 0; i < last_used_particle; i ++) {
        if(particles_[i].life_ <= 0.0f){
            last_used_particle = i;
            return i;
        }
    }
    
    // 未找到消亡粒子
    last_used_particle = 0;
    return -1;
}

// 重新生成一个粒子
// 实际上是为一个粒子重新赋值
void ParticleGenerator::respawnParticle(Particle& particle, GameObject& object, glm::vec2 offset){
    float random = ((rand() % 100) - 50) / 10.0f;  // 随机生成-5到5的数
    float r_color = 0.5 + ((rand() % 100) / 100.0f);  // 随机生成一个大于0.5的颜色值
    particle.position_ = object.position_ + random + offset;
    particle.color_ = glm::vec4(r_color, r_color, r_color, 1.0f);
    particle.life_ = 0.8f;
    particle.velocity_ = object.velocity_ * 0.1f;
}

// 绘制所有未消亡的粒子
void ParticleGenerator::Draw(){
    // add混合，粒子叠加在一起以产生发光效果，更加灼热
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    shader_.Use();
    shader_.SetFloat("scale", 10 * scale_.x);

    glBindVertexArray(vao_);

    // for(Particle particle : particles_){
    //     if(particle.life_ > 0.0f){
    //         shader_.SetVector2f("offset", particle.position_);
    //         shader_.SetVector4f("color", particle.color_);
    //         texture_.Bind();
    //         // glDrawArrays(GL_TRIANGLES, 0, 6);
    //         glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    //     }
    // }
    
    texture_.Bind();
    unsigned int cnt = 0;
    for(Particle particle : particles_){
        if(particle.life_ > 0.0f){
            instance_data_[cnt ++] = particle.position_.x;
            instance_data_[cnt ++] = particle.position_.y;
            instance_data_[cnt ++] = particle.color_.r;
            instance_data_[cnt ++] = particle.color_.g;
            instance_data_[cnt ++] = particle.color_.b;
            instance_data_[cnt ++] = particle.color_.a;
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * cnt, instance_data_);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, cnt / 6);
    
    glBindVertexArray(0);

    // 还原默认的混合模式
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// 初始化粒子生成器，1. 填充VAO 2. 根据amount_生成粒子对象
void ParticleGenerator::init() {
    unsigned int vbo;
    // float particle_quad[] = {
    //     0.0f, 1.0f, 0.0f, 1.0f,
    //     1.0f, 0.0f, 1.0f, 0.0f,
    //     0.0f, 0.0f, 0.0f, 0.0f,

    //     0.0f, 1.0f, 0.0f, 1.0f,
    //     1.0f, 1.0f, 1.0f, 1.0f,
    //     1.0f, 0.0f, 1.0f, 0.0f
    // };

    // 优化，使用GL_TRIANGLE_STRIP渲染方式，节省需要传递的数据量
    float particle_quad[] = {
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // 用于实例化的vbo
    glGenBuffers(1, &instance_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo_);

    // 这里的6表示<vec2 offset, vec4 color>
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * amount_, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));

    /*
        告诉OpenGL该什么时候更新顶点属性的数据
        参数1，需要设置的顶点属性
        参数2，属性除数，默认情况下是0，即在每次顶点着色器迭代时都更新数据
               设置为1时表示希望在渲染一个新实例的时候更新，2则表示2个实例，以此类推
    */
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    // 生成amount_数量的粒子实例
    for(unsigned int i = 0; i < amount_; i ++)
        particles_.push_back(Particle());

    instance_data_ = new float[amount_ * 6];
}