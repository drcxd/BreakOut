#include "Shader.h"

#include <sstream>
#include <cassert>

#include <fmt/core.h>

#include "Texture2D.h"

const char* ERROR_LOG_FMT = "Shader {} compilation failed!\n{}\n";

Shader::Shader(ShaderSource* vert, ShaderSource* frag,
               ShaderSource* geom) {
    assert(vert->code);
    assert(frag->code);
    GLuint vertex, fragment, geometry;
    int success;
    char infoLog[512];

    vertex = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource( vertex, 1, &vert->code, NULL );
    glCompileShader( vertex );
    glGetShaderiv( vertex, GL_COMPILE_STATUS, &success );
    if ( !success ) {
        glGetShaderInfoLog( vertex, 512, NULL, infoLog );
        fmt::print(ERROR_LOG_FMT, vert->name, infoLog);
    }

    fragment = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( fragment, 1, &frag->code, NULL );
    glCompileShader( fragment );
    glGetShaderiv( fragment, GL_COMPILE_STATUS, &success );
    if ( !success ) {
        glGetShaderInfoLog( fragment, 512, NULL, infoLog );
        fmt::print(ERROR_LOG_FMT, frag->name, infoLog);
    }

    if (geom) {
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &geom->code, NULL);
        glCompileShader(geometry);
        glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(geometry, 512, NULL, infoLog);
            fmt::print(ERROR_LOG_FMT, geom->name, infoLog);
        }
    }

    ID = glCreateProgram();
    glAttachShader( ID, vertex );
    glAttachShader( ID, fragment );
    if (geom) {
        glAttachShader(ID, geometry);
    }
    glLinkProgram( ID );

    glGetProgramiv( ID, GL_LINK_STATUS, &success );
    if ( !success ) {
        glGetProgramInfoLog( ID, 512, NULL, infoLog );
        fmt::print("Shader linking failed!\n{}\n", infoLog);
    }

    glDetachShader(ID, vertex);
    glDetachShader(ID, fragment);

    glDeleteShader( vertex );
    glDeleteShader( fragment );
    if (geom) {
        glDetachShader(ID, geometry);
        glDeleteShader(geometry);
    }
}

Shader::~Shader() {
    glDeleteProgram(ID);
}
void Shader::use() {
    glUseProgram(ID);
}

void Shader::setBool( const std::string& name, bool value ) const {
    glUniform1i( glGetUniformLocation( ID, name.c_str() ), ( int )value );
}

void Shader::setInt( const std::string& name, int value ) const {
    glUniform1i( glGetUniformLocation( ID, name.c_str() ), value );
}

void Shader::setFloat( const std::string& name, float value ) const {
    glUniform1f( glGetUniformLocation( ID, name.c_str() ), value );
}

void Shader::setMat4( const std::string& name, const glm::mat4& value ) const {
    glUniformMatrix4fv( glGetUniformLocation( ID, name.c_str() ), 1, GL_FALSE, glm::value_ptr( value ) );
}

void Shader::setVec3( const std::string& name, const glm::vec3& value ) const {
    glUniform3fv( glGetUniformLocation( ID, name.c_str() ), 1, glm::value_ptr( value ) );
}

void Shader::setVec3( const std::string& name, float x, float y, float z) const {
    glUniform3f( glGetUniformLocation( ID, name.c_str() ), x, y, z );
}

void Shader::setTexture(const std::string& name, int value,
                        const std::shared_ptr<Texture2D>& tex) {
    glActiveTexture(GL_TEXTURE0 + value);
    glBindTexture(GL_TEXTURE_2D, tex->ID);
    setInt(name, value);
}

void Shader::setVec2(const std::string& name,
                     const glm::vec2& value) const {
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1,
                 glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name,
                     const glm::vec4& value) const {
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1,
                 glm::value_ptr(value));
}

void Shader::setFloatV(const std::string& name,
                       const float* values, int num) const {
    glUniform1fv(glGetUniformLocation(ID, name.c_str()), num,
                 values);
}

void Shader::setVec2V(const std::string& name,
                      const float* values, int num) const {
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), num,
                 values);
}

void Shader::setIntV(const std::string& name,
                     const int* values, int num) const {
    glUniform1iv(glGetUniformLocation(ID, name.c_str()), num,
                 values);
}