#pragma once
#include "defines.h"
#include "math_types.h"
#include "core/memory.h"


// - - - |  CONSTANTS AND GENERAL FUNCTIONS  | - - -


// - - - CONSTANTS
#define FORGE_PI 3.14159265358979323846f
#define FORGE_PI_2 2.0f * FORGE_PI
#define FORGE_PI_4 4.0f * FORGE_PI
#define FORGE_HALF_PI 0.5f * FORGE_PI
#define FORGE_QUARTER_PI 0.25f * FORGE_PI
#define FORGE_ONE_OVER_PI 1.0f / FORGE_PI
#define FORGE_ONE_OVER_TWO_PI 1.0f / FORGE_PI_2
#define FORGE_SQRT_2 1.41421356237309504880f
#define FORGE_SQRT_3 1.73205080756887729352f
#define FORGE_SQRT_1_OVER_2 0.70710678118654752440f
#define FORGE_SQRT_1_OVER_3 0.57735026918962576450f
#define FORGE_DEG_TO_RAD FORGE_PI / 180.0f
#define FORGE_RAD_TO_DEG 180.0f / FORGE_PI
#define FORGE_EPSILON 1.192092896e-07f
#define FORGE_EPSILON_SQUARED FORGE_EPSILON * FORGE_EPSILON
#define FORGE_INFINITY 1e30f;


// - - - Functions - - -

// - - - GENERAL FUNCTIONS
FORGE_API float forgeSin(float ANGLE);
FORGE_API float forgeCos(float ANGLE);
FORGE_API float forgeAcos(float ANGLE);
FORGE_API float forgeTan(float ANGLE);
FORGE_API float forgeSqrt(float VALUE);
FORGE_API float forgeAbs(float VALUE);

// - - - Utility functions
FORGE_INLINE bool isPowerOfTwo(unsigned int VALUE) 
{ 
    return ((VALUE & (VALUE - 1)) == 0) && (VALUE != 0); 
}

FORGE_API int forgeRandom();
FORGE_API int forgeRandomRange(int MIN, int MAX);
FORGE_API float forgeFloatRandom();
FORGE_API float forgeFloatRandomRange(float MIN, float MAX);


// - - - | Vectors | - - - 


// - - - Vector2D - - -

// - - - Creation
FORGE_INLINE Vector2D createVector2D(float X, float Y)
{
    Vector2D result = {};
    result.x = X;
    result.y = Y;
    //return (Vector2D){X, Y};
    return result;
}

// - - - Common Vectors
FORGE_INLINE Vector2D zeroVector2D()
{
    return createVector2D(0.0f, 0.0f);
    //return (Vector2D){0.0f, 0.0f};
}

FORGE_INLINE Vector2D oneVector2D()
{
    return createVector2D(1.0f, 1.0f);
    //return (Vector2D){1.0f, 1.0f};
}

FORGE_INLINE Vector2D upVector2D()
{
    return createVector2D(0.0f, 1.0f);
    //return (Vector2D){0.0f, 1.0f};
}

FORGE_INLINE Vector2D rightVector2D()
{
    return createVector2D(1.0f, 0.0f);
    //return (Vector2D){1.0f, 0.0f};
}

FORGE_INLINE Vector2D downVector2D()
{
    return createVector2D(0.0f, -1.0f);
    //return (Vector2D){0.0f, -1.0f};
}

FORGE_INLINE Vector2D leftVector2D()
{
    return createVector2D(-1.0f, 0.0f);
    //return (Vector2D){-1.0f, 0.0f};
}

// - - - Operations
FORGE_INLINE Vector2D addVector2D(Vector2D A, Vector2D B)
{
    return createVector2D(A.x + B.x, A.y + B.y);
    //return (Vector2D){A.x + B.x, A.y + B.y};
}

FORGE_INLINE Vector2D subtractVector2D(Vector2D A, Vector2D B)
{
    return createVector2D(A.x - B.x, A.y - B.y);
    //return (Vector2D){A.x - B.x, A.y - B.y};
}

FORGE_INLINE Vector2D multiplyVector2D(Vector2D A, Vector2D B)
{
    return createVector2D(A.x * B.x, A.y * B.y);
    //return (Vector2D){A.x * B.x, A.y * B.y};
}

FORGE_INLINE float dotProductVector2D(Vector2D A, Vector2D B)
{
    return A.x * B.x + A.y * B.y;
}

FORGE_INLINE Vector2D divideVector2D(Vector2D A, Vector2D B)
{
    return createVector2D(A.x / B.x, A.y / B.y);
    //return (Vector2D){A.x / B.x, A.y / B.y};
}

FORGE_INLINE Vector2D scaleVector2D(Vector2D A, float SCALAR)
{
    return createVector2D(A.x * SCALAR, A.y * SCALAR);
    //return (Vector2D){A.x * SCALAR, A.y * SCALAR};
}

FORGE_INLINE float lengthSquaredVector2D(Vector2D A)
{
    return A.x * A.x + A.y * A.y;
}

FORGE_INLINE float lengthVector2D(Vector2D A)
{
    return forgeSqrt(lengthSquaredVector2D(A));
}

FORGE_INLINE void normalizeVector2D(Vector2D* A)
{
    const float length = lengthVector2D(*A);
    A->x /= length;
    A->y /= length;
}

FORGE_INLINE Vector2D normalizedVector2D(Vector2D A)
{
    normalizeVector2D(&A);
    return A;
}

FORGE_INLINE bool compareVector2D(Vector2D A, Vector2D B, float TOLERANCE)
{
    if (forgeAbs(A.x - B.x) > TOLERANCE) 
    {
        return false;
    }
    if (forgeAbs(A.y - B.y) > TOLERANCE) 
    {
        return false;
    }
    return true;
}

FORGE_INLINE float distanceSquaredVector2D(Vector2D A, Vector2D B)
{
    return lengthSquaredVector2D(subtractVector2D(A, B));
}

FORGE_INLINE float distanceVector2D(Vector2D A, Vector2D B)
{
    return forgeSqrt(distanceSquaredVector2D(A, B));
}


// - - - Vector3D - - -

// - - - Creation
FORGE_INLINE Vector3D createVector3D(float X, float Y, float Z)
{
    Vector3D result = {};
    result.x = X;
    result.y = Y;
    result.z = Z;
    //return (Vector3D){X, Y, Z};
    return result;
}

// - - - Conversion
FORGE_INLINE Vector4D vector3DToVector4D(Vector3D A, float W)
{
    return (Vector4D){A.x, A.y, A.z, W};
}

// - - - Common Vectors
FORGE_INLINE Vector3D zeroVector3D()
{
    return createVector3D(0.0f, 0.0f, 0.0f);
    //return (Vector3D){0.0f, 0.0f, 0.0f};
}

FORGE_INLINE Vector3D oneVector3D()
{
    return createVector3D(1.0f, 1.0f, 1.0f);
    //return (Vector3D){1.0f, 1.0f, 1.0f};
}

FORGE_INLINE Vector3D upVector3D()
{
    return createVector3D(0.0f, 1.0f, 0.0f);
    //return (Vector3D){0.0f, 1.0f, 0.0f};
}

FORGE_INLINE Vector3D rightVector3D()
{
    return createVector3D(1.0f, 0.0f, 0.0f);
    //return (Vector3D){1.0f, 0.0f, 0.0f};
}

FORGE_INLINE Vector3D forwardVector3D()
{
    return createVector3D(0.0f, 0.0f, -1.0f);
    //return (Vector3D){0.0f, 0.0f, -1.0f};
}

FORGE_INLINE Vector3D downVector3D()
{
    return createVector3D(0.0f, -1.0f, 0.0f);
    //return (Vector3D){0.0f, -1.0f, 0.0f};
}

FORGE_INLINE Vector3D leftVector3D()
{
    return createVector3D(-1.0f, 0.0f, 0.0f);
    //return (Vector3D){-1.0f, 0.0f, 0.0f};
}

FORGE_INLINE Vector3D backVector3D()
{
    return createVector3D(0.0f, 0.0f, 1.0f);
    //return (Vector3D){0.0f, 0.0f, 1.0f};
}

// - - - Operations
FORGE_INLINE Vector3D addVector3D(Vector3D A, Vector3D B)
{
    return createVector3D(A.x + B.x, A.y + B.y, A.z + B.z);
    //return (Vector3D){A.x + B.x, A.y + B.y, A.z + B.z};
}

FORGE_INLINE Vector3D subtractVector3D(Vector3D A, Vector3D B)
{
    return createVector3D(A.x - B.x, A.y - B.y, A.z - B.z);
    //return (Vector3D){A.x - B.x, A.y - B.y, A.z - B.z};
}

FORGE_INLINE Vector3D multiplyVector3D(Vector3D A, Vector3D B)
{
    return createVector3D(A.x * B.x, A.y * B.y, A.z * B.z);
    //return (Vector3D){A.x * B.x, A.y * B.y, A.z * B.z};
}

FORGE_INLINE float dotProductVector3D(Vector3D A, Vector3D B)
{
    return A.x * B.x + A.y * B.y + A.z * B.z;
}

FORGE_INLINE Vector3D crossProductVector3D(Vector3D A, Vector3D B)
{
    return createVector3D(A.y * B.z - A.z * B.y, A.z * B.x - A.x * B.z, A.x * B.y - A.y * B.x);
    //return (Vector3D){A.y * B.z - A.z * B.y, A.z * B.x - A.x * B.z, A.x * B.y - A.y * B.x};
}

FORGE_INLINE Vector3D divideVector3D(Vector3D A, Vector3D B)
{
    return createVector3D(A.x / B.x, A.y / B.y, A.z / B.z);
    //return (Vector3D){A.x / B.x, A.y / B.y, A.z / B.z};
}

FORGE_INLINE Vector3D scaleVector3D(Vector3D A, float SCALAR)
{
    return createVector3D(A.x * SCALAR, A.y * SCALAR, A.z * SCALAR);
    //return (Vector3D){A.x * SCALAR, A.y * SCALAR, A.z * SCALAR};
}

FORGE_INLINE float lengthSquaredVector3D(Vector3D A)
{
    return A.x * A.x + A.y * A.y + A.z * A.z;
}

FORGE_INLINE float lengthVector3D(Vector3D A)
{
    return forgeSqrt(lengthSquaredVector3D(A));
}

FORGE_INLINE void normalizeVector3D(Vector3D* A)
{
    const float length = lengthVector3D(*A);
    A->x /= length;
    A->y /= length;
    A->z /= length;
}

FORGE_INLINE Vector3D normalizedVector3D(Vector3D A)
{
    normalizeVector3D(&A);
    return A;
}


// - - - Vector4D - - -

// - - - Creation
FORGE_INLINE Vector4D createVector4D(float X, float Y, float Z, float W)
{
    Vector4D result = {};
    result.x = X;
    result.y = Y;
    result.z = Z;
    result.w = W;
    //return (Vector4D){X, Y, Z, W};
    return result;
}

// - - - Conversion
FORGE_INLINE Vector3D vector4DToVector3D(Vector4D A)
{
    return createVector3D(A.x, A.y, A.z);
}

// - - - Common Vectors
FORGE_INLINE Vector4D zeroVector4D()
{
    return createVector4D(0.0f, 0.0f, 0.0f, 0.0f);
    //return (Vector4D){0.0f, 0.0f, 0.0f, 0.0f};
}

FORGE_INLINE Vector4D oneVector4D()
{
    return createVector4D(1.0f, 1.0f, 1.0f, 1.0f);
    //return (Vector4D){1.0f, 1.0f, 1.0f, 1.0f};
}

// - - - Operations
FORGE_INLINE Vector4D addVector4D(Vector4D A, Vector4D B)
{
    return createVector4D(A.x + B.x, A.y + B.y, A.z + B.z, A.w + B.w);
    //return (Vector4D){A.x + B.x, A.y + B.y, A.z + B.z, A.w + B.w};
}

FORGE_INLINE Vector4D subtractVector4D(Vector4D A, Vector4D B)
{
    return createVector4D(A.x - B.x, A.y - B.y, A.z - B.z, A.w - B.w);
    //return (Vector4D){A.x - B.x, A.y - B.y, A.z - B.z, A.w - B.w};
}

FORGE_INLINE Vector4D multiplyVector4D(Vector4D A, Vector4D B)
{
    return createVector4D(A.x * B.x, A.y * B.y, A.z * B.z, A.w * B.w);
    //return (Vector4D){A.x * B.x, A.y * B.y, A.z * B.z, A.w * B.w};
}

FORGE_INLINE float dotProductVector4D(Vector4D A, Vector4D B)
{
    return A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
}

FORGE_INLINE float dotProductWithFloats(float A_0, float A_1, float A_2, float A_3, float B_0, float B_1, float B_2, float B_3)
{
    return A_0 * B_0 + A_1 * B_1 + A_2 * B_2 + A_3 * B_3;
}

FORGE_INLINE Vector4D divideVector4D(Vector4D A, Vector4D B)
{
    return createVector4D(A.x / B.x, A.y / B.y, A.z / B.z, A.w / B.w);
    //return (Vector4D){A.x / B.x, A.y / B.y, A.z / B.z, A.w / B.w};
}

FORGE_INLINE Vector4D scaleVector4D(Vector4D A, float SCALAR)
{
    return createVector4D(A.x * SCALAR, A.y * SCALAR, A.z * SCALAR, A.w * SCALAR);
    //return (Vector4D){A.x * SCALAR, A.y * SCALAR, A.z * SCALAR, A.w * SCALAR};
}

FORGE_INLINE float lengthSquaredVector4D(Vector4D A)
{
    return A.x * A.x + A.y * A.y + A.z * A.z + A.w * A.w;
}

FORGE_INLINE float lengthVector4D(Vector4D A)
{
    return forgeSqrt(lengthSquaredVector4D(A));
}

FORGE_INLINE void normalizeVector4D(Vector4D* A)
{
    const float length = lengthVector4D(*A);
    A->x /= length;
    A->y /= length;
    A->z /= length;
    A->w /= length;
}

FORGE_INLINE Vector4D normalizedVector4D(Vector4D A)
{
    normalizeVector4D(&A);
    return A;
}


// - - - Matrices - - -


FORGE_INLINE Matrix4 identityMatrix4()
{
    Matrix4 matrix;
    forgeZeroMemory(matrix.data, sizeof(matrix.data));
    matrix.data[0] = 1.0f;
    matrix.data[5] = 1.0f;
    matrix.data[10] = 1.0f;
    matrix.data[15] = 1.0f;
    return matrix;
}

FORGE_INLINE Matrix4 multiplyMatrix4D(Matrix4 MAT_0, Matrix4 MAT_1)
{
    // John carmac the legend
    Matrix4 output = identityMatrix4();
    const float* m0Ptr = MAT_0.data;
    const float* m1Ptr = MAT_1.data;
    float* dstPtr = output.data;

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            *dstPtr =
                m0Ptr[0] * m1Ptr[0 + j] +
                m0Ptr[1] * m1Ptr[4 + j] +
                m0Ptr[2] * m1Ptr[8 + j] +
                m0Ptr[3] * m1Ptr[12 + j];
            ++dstPtr;
        }
        m0Ptr += 4;
    }
    return output;
}

FORGE_INLINE Matrix4 orthographicMatrix4(float LEFT, float RIGHT, float BOTTOM, float TOP, float NEAR_CLIP, float FAR_CLIP)
{
    Matrix4 output = identityMatrix4();

    float lr = 1.0f / (LEFT - RIGHT);
    float br = 1.0f / (BOTTOM - TOP);
    float nf = 1.0f / (NEAR_CLIP - FAR_CLIP);

    output.data[0] = -2.0f * lr;
    output.data[5] = -2.0f * br;
    output.data[10] = 2.0f * nf;

    output.data[12] = (LEFT + RIGHT) * lr;
    output.data[13] = (TOP + BOTTOM) * br;
    output.data[14] = (FAR_CLIP + NEAR_CLIP) * nf;

    return output;
}

FORGE_INLINE Matrix4 perspectiveMatrix(float FOV, float ASPECT_RATIO, float NEAR_CLIP, float FAR_CLIP)
{
    float tanHalfFOV = forgeTan(FOV * 0.5f);
    Matrix4 output;
    forgeZeroMemory(output.data, sizeof(output.data));

    output.data[0] = 1.0f / (ASPECT_RATIO * tanHalfFOV);
    output.data[5] = 1.0f / tanHalfFOV;
    output.data[10] = -((FAR_CLIP + NEAR_CLIP) / (FAR_CLIP - NEAR_CLIP));
    output.data[11] = -1.0f;
    output.data[14] = -((2.0f * FAR_CLIP * NEAR_CLIP) / (FAR_CLIP - NEAR_CLIP));

    return output;
}

FORGE_INLINE Matrix4 lookAtMatrix4(Vector3D POSITION, Vector3D TARGET, Vector3D UP)
{
    Matrix4 output;
    Vector3D zAxis;
    zAxis.x = TARGET.x - POSITION.x;
    zAxis.y = TARGET.y - POSITION.y;
    zAxis.z = TARGET.z - POSITION.z;

    zAxis = normalizedVector3D(zAxis);
    Vector3D xAxis = normalizedVector3D(crossProductVector3D(zAxis, UP));
    Vector3D yAxis = crossProductVector3D(xAxis, zAxis);

    output.data[0] = xAxis.x;
    output.data[1] = yAxis.x;
    output.data[2] = -zAxis.x;
    output.data[3] = 0;
    output.data[4] = xAxis.y;
    output.data[5] = yAxis.y;
    output.data[6] = -zAxis.y;
    output.data[7] = 0;
    output.data[8] = xAxis.z;
    output.data[9] = yAxis.z;
    output.data[10] = -zAxis.z;
    output.data[11] = 0;
    output.data[12] = -dotProductVector3D(xAxis, POSITION);
    output.data[13] = -dotProductVector3D(yAxis, POSITION);
    output.data[14] = dotProductVector3D(zAxis, POSITION);
    output.data[15] = -1.0f;

    return output;
}

FORGE_INLINE Matrix4 inverseMatrix4(Matrix4 MATRIX)
{ 
    const float* m = MATRIX.data;

    float t0 = m[10] * m[15];
    float t1 = m[14] * m[11];
    float t2 = m[6] * m[15];
    float t3 = m[14] * m[7];
    float t4 = m[6] * m[11];
    float t5 = m[10] * m[7];
    float t6 = m[2] * m[15];
    float t7 = m[14] * m[3];
    float t8 = m[2] * m[11];
    float t9 = m[10] * m[3];
    float t10 = m[2] * m[7];
    float t11 = m[6] * m[3];
    float t12 = m[8] * m[13];
    float t13 = m[12] * m[9];
    float t14 = m[4] * m[13];
    float t15 = m[12] * m[5];
    float t16 = m[4] * m[9];
    float t17 = m[8] * m[5];
    float t18 = m[0] * m[13];
    float t19 = m[12] * m[1];
    float t20 = m[0] * m[9];
    float t21 = m[8] * m[1];
    float t22 = m[0] * m[5];
    float t23 = m[4] * m[1];

    Matrix4 output;
    float* o = output.data;

    o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) - (t1 * m[5] + t2 * m[9] + t5 * m[13]);
    o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) - (t0 * m[1] + t7 * m[9] + t8 * m[13]);
    o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
    o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) - (t4 * m[1] + t9 * m[5] + t10 * m[9]);

    float d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

    o[0] = d * o[0];
    o[1] = d * o[1];
    o[2] = d * o[2];
    o[3] = d * o[3];
    o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) - (t0 * m[4] + t3 * m[8] + t4 * m[12]));
    o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) - (t1 * m[0] + t6 * m[8] + t9 * m[12]));
    o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) - (t2 * m[0] + t7 * m[4] + t10 * m[12]));
    o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) - (t5 * m[0] + t8 * m[4] + t11 * m[8]));
    o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) - (t13 * m[7] + t14 * m[11] + t17 * m[15]));
    o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) - (t12 * m[3] + t19 * m[11] + t20 * m[15]));
    o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) - (t15 * m[3] + t18 * m[7] + t23 * m[15]));
    o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) - (t16 * m[3] + t21 * m[7] + t22 * m[11]));
    o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) - (t16 * m[14] + t12 * m[6] + t15 * m[10]));
    o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
    o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) - (t22 * m[14] + t14 * m[2] + t19 * m[6]));
    o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) - (t20 * m[6] + t23 * m[10] + t17 * m[2]));

    return output;
}

FORGE_INLINE Matrix4 transposeMatrix4(Matrix4 MATRIX)
{
    Matrix4 output = identityMatrix4();

    output.data[0] = MATRIX.data[0];
    output.data[1] = MATRIX.data[4];
    output.data[2] = MATRIX.data[8];
    output.data[3] = MATRIX.data[12];
    output.data[4] = MATRIX.data[1];
    output.data[5] = MATRIX.data[5];
    output.data[6] = MATRIX.data[9];
    output.data[7] = MATRIX.data[13];
    output.data[8] = MATRIX.data[2];
    output.data[9] = MATRIX.data[6];
    output.data[10] = MATRIX.data[10];
    output.data[11] = MATRIX.data[14];
    output.data[12] = MATRIX.data[3];
    output.data[13] = MATRIX.data[7];
    output.data[14] = MATRIX.data[11];
    output.data[15] = MATRIX.data[15];

    return output;
}

FORGE_INLINE Matrix4 translationMatrix4(Vector3D POSITION)
{
    Matrix4 output = identityMatrix4();

    output.data[12] = POSITION.x;
    output.data[13] = POSITION.y;
    output.data[14] = POSITION.z;

    return output;
}

FORGE_INLINE Matrix4 scaleMatrix4(Vector3D SCALE)
{
    Matrix4 output = identityMatrix4();

    output.data[0] = SCALE.x;
    output.data[5] = SCALE.y;
    output.data[10] = SCALE.z;

    return output;
}

FORGE_INLINE Matrix4 eulerXMatrix4(float ANGLE)
{
    Matrix4 output = identityMatrix4();
    float c = forgeCos(ANGLE);
    float s = forgeSin(ANGLE);

    output.data[5] = c;
    output.data[6] = s;
    output.data[9] = -s;
    output.data[10] = c;

    return output;
}

FORGE_INLINE Matrix4 eulerYMatrix4(float ANGLE)
{
    Matrix4 output = identityMatrix4();
    float c = forgeCos(ANGLE);
    float s = forgeSin(ANGLE);

    output.data[0] = c;
    output.data[2] = s;
    output.data[8] = -s;
    output.data[10] = c;

    return output;
}

FORGE_INLINE Matrix4 eulerZMatrix4(float ANGLE)
{
    Matrix4 output = identityMatrix4();
    float c = forgeCos(ANGLE);
    float s = forgeSin(ANGLE);

    output.data[0] = c;
    output.data[1] = s;
    output.data[4] = -s;
    output.data[5] = c;

    return output;
}

FORGE_INLINE Matrix4 eulerXYZMatrix4(float X, float Y, float Z)
{
    Matrix4 resultX = eulerXMatrix4(X);
    Matrix4 resultY = eulerYMatrix4(Y);
    Matrix4 resultZ = eulerZMatrix4(Z);

    Matrix4 output = multiplyMatrix4D(resultX, resultY);
    output = multiplyMatrix4D(output, resultZ);

    return output;
}

FORGE_INLINE Vector3D forwardMatrix4(Matrix4 MATRIX)
{
    Vector3D forward;

    forward.x = -MATRIX.data[2];
    forward.y = -MATRIX.data[6];
    forward.z = -MATRIX.data[10];
    normalizeVector3D(&forward);

    return forward;
}
FORGE_INLINE Vector3D backwardMatrix4(Matrix4 MATRIX)
{
    Vector3D backward;

    backward.x = MATRIX.data[2];
    backward.y = MATRIX.data[6];
    backward.z = MATRIX.data[10];
    normalizeVector3D(&backward);

    return backward;
}

FORGE_INLINE Vector3D upwardMatrix4(Matrix4 MATRIX)
{
    Vector3D upward;

    upward.x = MATRIX.data[1];
    upward.y = MATRIX.data[5];
    upward.z = MATRIX.data[9];
    normalizeVector3D(&upward);

    return upward;
}

FORGE_INLINE Vector3D downwardMatrix4(Matrix4 MATRIX)
{
    Vector3D downward;

    downward.x = -MATRIX.data[1];
    downward.y = -MATRIX.data[5];
    downward.z = -MATRIX.data[9];
    normalizeVector3D(&downward);

    return downward;
}

FORGE_INLINE Vector3D rightwardMatrix4(Matrix4 MATRIX)
{
    Vector3D rightward;

    rightward.x = MATRIX.data[0];
    rightward.y = MATRIX.data[4];
    rightward.z = MATRIX.data[8];
    normalizeVector3D(&rightward);

    return rightward;
}

FORGE_INLINE Vector3D leftwardMatrix4(Matrix4 MATRIX)
{
    Vector3D leftward;

    leftward.x = MATRIX.data[0];
    leftward.y = MATRIX.data[4];
    leftward.z = MATRIX.data[8];
    normalizeVector3D(&leftward);

    return leftward;
}


// - - - | Quaternion | - - -


FORGE_INLINE Quaternion identityQuaternion()
{
    return (Quaternion){0, 0, 0, 1.0f};
}

FORGE_INLINE float normalQuaternion(Quaternion QUAT)
{
    return forgeSqrt(
        QUAT.x * QUAT.x + 
        QUAT.y * QUAT.y + 
        QUAT.z * QUAT.z + 
        QUAT.w * QUAT.w);
}

FORGE_INLINE Quaternion normalizeQuaternion(Quaternion QUAT)
{
    float normal = normalQuaternion(QUAT);
    return (Quaternion){
        QUAT.x / normal,
        QUAT.y / normal,
        QUAT.z / normal,
        QUAT.w / normal};
}

FORGE_INLINE Quaternion conjugateQuaternion(Quaternion QUAT)
{
    return (Quaternion){
        -QUAT.x,
        -QUAT.y,
        -QUAT.z,
        QUAT.w,
    };
}

FORGE_INLINE Quaternion inverseQuaternion(Quaternion QUAT)
{
    return normalizeQuaternion(conjugateQuaternion(QUAT));
}

FORGE_INLINE Quaternion multiplyQuaternion(Quaternion QUAT_0, Quaternion QUAT_1)
{
    Quaternion output;

    output.x = QUAT_0.x * QUAT_1.w +
               QUAT_0.y * QUAT_1.z -
               QUAT_0.z * QUAT_1.y +
               QUAT_0.w * QUAT_1.x;
    
    output.y = -QUAT_0.x * QUAT_1.z +
               QUAT_0.y * QUAT_1.w +
               QUAT_0.z * QUAT_1.x +
               QUAT_0.w * QUAT_1.y;

    output.z = QUAT_0.x * QUAT_1.y -
               QUAT_0.y * QUAT_1.x +
               QUAT_0.z * QUAT_1.w +
               QUAT_0.w * QUAT_1.z;
    
    output.w = -QUAT_0.x * QUAT_1.x -
               QUAT_0.y * QUAT_1.y -
               QUAT_0.z * QUAT_1.z +
               QUAT_0.w * QUAT_1.w;

    return output;
}

FORGE_INLINE float dotProductQuaternion(Quaternion QUAT_0, Quaternion QUAT_1)
{
    return QUAT_0.x * QUAT_1.x +
           QUAT_0.y * QUAT_1.y + 
           QUAT_0.z * QUAT_1.z + 
           QUAT_0.w * QUAT_1.w; 
}

FORGE_INLINE Matrix4 quaternionToMatrix4(Quaternion QUAT)
{
    Matrix4 output = identityMatrix4();
    //https://stackoverflow.com/questions/1556260/convert-quaternion-rotation-to-rotation-matrix
    Quaternion n = normalizeQuaternion(QUAT);

    output.data[0] = 1.0f - 2.0f * n.y * n.y - 2.0f * n.z * n.z;
    output.data[1] = 2.0f * n.x * n.y - 2.0f * n.z * n.w;
    output.data[2] = 2.0f * n.x * n.z - 2.0f * n.z * n.w;

    output.data[4] = 2.0f * n.x * n.y + 2.0f * n.z * n.w;
    output.data[5] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.z * n.z;
    output.data[6] = 2.0f * n.y * n.z - 2.0f * n.x * n.w;

    output.data[8] = 2.0f * n.x * n.z - 2.0f * n.y * n.w;
    output.data[9] = 2.0f * n.y * n.z + 2.0f * n.x * n.w;
    output.data[10] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.y * n.y;

    return output;
}

FORGE_INLINE Matrix4 quaternionToRotationMatrix(Quaternion QUAT, Vector3D CENTER)
{
    Matrix4 output;

    float* o = output.data;
    o[0] = (QUAT.x * QUAT.x) - (QUAT.y * QUAT.y) - (QUAT.z * QUAT.z) + (QUAT.w * QUAT.w);
    o[1] = 2.0f * ((QUAT.x * QUAT.y) + (QUAT.z * QUAT.w));
    o[2] = 2.0f * ((QUAT.x * QUAT.z) - (QUAT.y * QUAT.w));
    o[3] = CENTER.x - CENTER.x * o[0] - CENTER.y * o[1] - CENTER.z * o[2];

    o[4] = 2.0f * ((QUAT.x * QUAT.y) - (QUAT.z * QUAT.w));
    o[5] = -(QUAT.x * QUAT.x) + (QUAT.y * QUAT.y) - (QUAT.z * QUAT.z) + (QUAT.w * QUAT.w);
    o[6] = 2.0f * ((QUAT.y * QUAT.z) + (QUAT.x * QUAT.w));
    o[7] = CENTER.y - CENTER.x * o[4] - CENTER.y * o[5] - CENTER.z * o[6];

    o[8] = 2.0f * ((QUAT.x * QUAT.z) + (QUAT.y * QUAT.w));
    o[9] = 2.0f * ((QUAT.y * QUAT.z) - (QUAT.x * QUAT.w));
    o[10] = -(QUAT.x * QUAT.x) - (QUAT.y * QUAT.y) + (QUAT.z * QUAT.z) + (QUAT.w * QUAT.w);
    o[11] = CENTER.z - CENTER.x * o[8] - CENTER.y * o[9] - CENTER.z * o[10];

    o[12] = 0.0f;
    o[13] = 0.0f;
    o[14] = 0.0f;
    o[15] = 1.0f;
    return output;
}

FORGE_INLINE Quaternion quaternionFromAxisAngle(Vector3D AXIS, float ANGLE, bool NORMALIZE)
{
    const float halfAngle = 0.5f * ANGLE;
    float s = forgeSin(halfAngle); 
    float c = forgeCos(halfAngle);

    Quaternion q = (Quaternion){s * AXIS.x, s * AXIS.y, s * AXIS.z, c};
    if (NORMALIZE) 
    {
        return normalizeQuaternion(q);
    }
    return q;
}

FORGE_INLINE Quaternion quaternionSlerp(Quaternion QUAT_0, Quaternion QUAT_1, float PERCENTAGE)
{
    Quaternion output;
    // Source: https://en.wikipedia.org/wiki/Slerp
    // Only unit quaternions are valid rotations.
    // Normalize to avoid undefined behavior.
    
    Quaternion v0 = normalizeQuaternion(QUAT_0);
    Quaternion v1 = normalizeQuaternion(QUAT_1);

    float dot = dotProductQuaternion(v0, v1);

    if (dot < 0.0f)
    {
        v1.x = -v1.x;
        v1.y = -v1.y;
        v1.z = -v1.z;
        v1.w = -v1.w;
        dot = -dot;
    }

    const float DOT_THRESHOLD = 0.9995f;
    if (dot > DOT_THRESHOLD)
    {
        // If the inputs are too close for comfort, linearly interpolate
        // and normalize the result.
        output = (Quaternion)
            {
                v0.x + ((v1.x - v0.x) * PERCENTAGE),
                v0.y + ((v1.y - v0.y) * PERCENTAGE),
                v0.z + ((v1.z - v0.z) * PERCENTAGE),
                v0.w + ((v1.w - v0.w) * PERCENTAGE)};
            
        return normalizeQuaternion(output);
    }

    // Since dot is in range [0, DOT_THRESHOLD], acos is safe
    float theta0 = forgeAcos(dot);          // theta0 = angle between input vectors
    float theta = theta0 * PERCENTAGE;  // theta = angle between v0 and result
    float sinTheta = forgeSin(theta);       // compute this value only once
    float sinTheta0 = forgeSin(theta0);   // compute this value only once

    float s0 = forgeCos(theta) - dot * sinTheta / sinTheta0;  // == sin(theta0 - theta) / sin(theta0)
    float s1 = sinTheta / sinTheta0;

    return (Quaternion){
        (v0.x * s0) + (v1.x * s1),
        (v0.y * s0) + (v1.y * s1),
        (v0.z * s0) + (v1.z * s1),
        (v0.w * s0) + (v1.w * s1)};
}

FORGE_INLINE float degreeToRadian(float DEGREES)
{
    return DEGREES * FORGE_DEG_TO_RAD;
}

FORGE_INLINE float radiansToDegree(float RADIANS)
{
    return RADIANS * FORGE_RAD_TO_DEG;
}
