#include "PostProcessor.h"

#include <fmt/core.h>

#include "Texture2D.h"
#include "Shader.h"

static const float vertices[] = {
    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f,  1.0f, 1.0f, 1.0f,
    -1.0f,  1.0f, 0.0f, 1.0f,

    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
    1.0f,  1.0f, 1.0f, 1.0f
};

static const float offset = 1.0f / 300.0f;
static const float offsets[9][2] = {
    { -offset,  offset  },  // top-left
    {  0.0f,    offset  },  // top-center
    {  offset,  offset  },  // top-right
    { -offset,  0.0f    },  // center-left
    {  0.0f,    0.0f    },  // center-center
    {  offset,  0.0f    },  // center - right
    { -offset, -offset  },  // bottom-left
    {  0.0f,   -offset  },  // bottom-center
    {  offset, -offset  }   // bottom-right
};

static const int edge_kernel[9] = {
    -1, -1, -1,
    -1,  8, -1,
    -1, -1, -1
};

static const float blur_kernel[9] = {
    1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
    2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
    1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f
};

PostProcessor::PostProcessor(const std::shared_ptr<Shader> shader,
                             int width, int height)
    : shader(shader)
    , width(width)
    , height(height) {
    glGenFramebuffers(1, &MSFBO);
    glGenFramebuffers(1, &FBO);
    glGenRenderbuffers(1, &RBO);

    glBindFramebuffer(GL_FRAMEBUFFER, MSFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGB,
                                     width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER, RBO);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
        GL_FRAMEBUFFER_COMPLETE) {
        fmt::print("ERROR::POSTPROCESSOR: "
                   "Failed to initialize MSFBO\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    TextureSource ts;
    ts.width = width;
    ts.height = height;
    texture = std::make_shared<Texture2D>(&ts);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, texture->ID, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
        GL_FRAMEBUFFER_COMPLETE) {
        fmt::print("ERROR::POSTPROCESSOR: "
                   "Failed to initialize FBO\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    initData();

    shader->use();
    shader->setVec2V("offsets", (const float*)offsets, 9);
    shader->setIntV("edge_kernel", edge_kernel, 9);
    shader->setFloatV("blur_kernel", blur_kernel, 9);
}

PostProcessor::~PostProcessor() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void PostProcessor::initData() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                 vertices, GL_STATIC_DRAW);

    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void PostProcessor::BeginRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, MSFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void PostProcessor::EndRender() {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, MSFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
    glBlitFramebuffer(0, 0, width, height,
                      0, 0, width, height,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::Render(float time) {
    shader->use();
    shader->setFloat("time", time);
    shader->setInt("confuse", confuse);
    shader->setInt("chaos", chaos);
    shader->setInt("shake", shake);

    shader->setTexture("scene", 0, texture);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
