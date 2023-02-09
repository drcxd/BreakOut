#ifndef __SHADER_H__
#define __SHADER_H__

#include <string>
#include <memory>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

class Texture2D;

// Hold information used to compile shader
struct ShaderSource {
    const char* name; // For debug convenience
    const char* code;
};

class Shader {
public:
    // the program ID
    GLuint ID;

    // constructor reads and builds the shader
    Shader(ShaderSource* vert, ShaderSource* frag,
           ShaderSource* geom = nullptr);
    ~Shader();
    // use/active the shader
    void use() const;
    // utility uniform functions
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setMat4(const std::string& name,
                 const glm::mat4& value) const;
    void setVec3(const std::string& name,
                 const glm::vec3& value) const;
    void setVec3(const std::string& name,
                 float x, float y, float z) const;
    void setVec2(const std::string& name,
                 const glm::vec2& value) const;
    void setVec4(const std::string& name,
                 const glm::vec4& value) const;
    void setTexture(const std::string& name, int value,
                    const Texture2D* tex) const;
    void setFloatV(const std::string& name,
                   const float* values, int num) const;
    void setVec2V(const std::string& name,
                  const float* values, int num) const;
    void setIntV(const std::string& name,
                 const int* values, int num) const;
};

#endif
