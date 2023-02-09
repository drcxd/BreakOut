#ifndef __POSTPROCESSOR_H__
#define __POSTPROCESSOR_H__

#include <memory>

#include <glad/glad.h>

class Shader;
class Texture2D;

class PostProcessor {
public:

    PostProcessor(const Shader* shader,
                  int width, int height);
    ~PostProcessor();

    void BeginRender();
    void EndRender();

    void Render(float time);

    const Shader* shader;
    std::unique_ptr<Texture2D> texture = nullptr;
    int width, height;
    bool confuse = false;
    bool chaos = false;
    bool shake = false;

private:

    GLuint MSFBO = 0;
    GLuint FBO = 0;
    GLuint RBO = 0;
    GLuint VAO = 0;
    GLuint VBO = 0;

    void initData();
};
#endif
