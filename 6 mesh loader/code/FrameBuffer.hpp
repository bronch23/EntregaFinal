#pragma once
#include <glad/glad.h>

class Framebuffer
{
private:
    GLuint framebuffer;
    GLuint texture;
    GLuint renderbuffer;

public:
    Framebuffer();
    ~Framebuffer();

    void initialize(int width, int height);
    void bind() const;
    void unbind() const;
    GLuint get_texture() const;
};

