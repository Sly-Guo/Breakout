#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <map>

#include "shader.h"
#include "glm.hpp"

// struct Character {
//     unsigned int texture_id_;  // 字形纹理的ID
//     glm::ivec2 size_;  // 字形大小
//     glm::ivec2 bearing_;  // 从基准线到字形左部/顶部的偏移量
//     unsigned int advance_;  // 原点距下一个字形原点的距离
// };

// 位图字符
struct Character {
    glm::vec2 texcoords_;  // 字形左下角坐标
    glm::vec2 offset_;  // 字形右上角距离左下角的偏移
    glm::ivec2 size_;  // 字形大小
    glm::ivec2 bearing_;  // 从基准线到字形左部/顶部的偏移量
    unsigned int advance_;  // 原点距下一个字形原点的距离
};

class TextRenderer
{
private:
    unsigned int vao_, vbo_;
public:
    unsigned int texture_id_;  // 记录大的位图纹理的id

    std::map<char, Character> characters_;
    Shader shader_;
    TextRenderer(Shader& shader);

    // 加载指定字体
    void Load(std::string font, unsigned font_size);
    // 使用预加载的字体渲染一个文本
    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1.0f));
};

#endif