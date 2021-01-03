#include "Utility.h"

#include <glad/glad.h>

#include <fmt/core.h>

void Utility::CheckGLError() {
    GLenum code;
    while ((code = glGetError()) != GL_NO_ERROR) {
        switch (code) {
        default:
            fmt::print("Unhandled GL Error {}\n", code);
            break;
        }
    }
}
