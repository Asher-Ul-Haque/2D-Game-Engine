#pragma once
#include "defines.h"


// - - - | Vector | - - -


// - - - 2D Vector - - -

typedef union Vector2D
{
    float elements[2];
    struct 
    {
        union
        {
            float x, r, s, u;
        };
        union
        {
            float y, g, t, v;
        };
    };
} Vector2D;


// - - - 3D Vector - - -

typedef union Vector3D
{
    float elements[3];
    struct 
    {
        union
        {
            float x, r, s, u;
        };
        union
        {
            float y, g, t, v;
        };
        union
        {
            float z, b, p, w;
        };
    };
} Vector3D;


// - - - 4D Vector - - -

typedef union Vector4D
{
    float elements[4];
    struct 
    {
        union
        {
            float x, r, s;
        };
        union
        {
            float y, g, t;
        };
        union
        {
            float z, b, p;
        };
        union
        {
            float w, a, q;
        };
    };
} Vector4D;

typedef Vector4D Quaternion;


// - - - | Matrices | - - -


typedef union Matrix4
{
    float data[16];
    Vector4D rows[4];
} Matrix4;


// - - - | Vertices | - - -


typedef struct Vertex3D
{
    Vector3D position;
} Vertex3D;
