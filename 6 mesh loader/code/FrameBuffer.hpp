// Proyecto: Entrega Programaci�n Gr�fica UDIT 2024/25
// Autor: Jorge L Broncano Luque
// Licencia: Uso libre (2025)
// Descripci�n: Implementaci�n de conceptos b�sicos de programaci�n de gr�ficos
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

