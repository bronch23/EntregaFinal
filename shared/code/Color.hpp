// Proyecto: Entrega Programaci�n Gr�fica UDIT 2024/25
// Autor: Jorge L Broncano Luque
// Licencia: Uso libre (2025)
// Descripci�n: Implementaci�n de conceptos b�sicos de programaci�n de gr�ficos

#pragma once

#include <cstdint>

namespace udit
{

    using Monochrome8 = uint8_t;

    union Rgba8888
    {
        enum { RED, GREEN, BLUE, ALPHA };

        uint32_t value;
        uint8_t  components[4];
    };

}
