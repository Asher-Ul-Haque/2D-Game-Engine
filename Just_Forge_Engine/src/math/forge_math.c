#include "forge_math.h"
#include "platform/platform.h"
#include <math.h>
#include <stdlib.h> 

static bool randomlySeeded = false;

/*
 * Note that these are here so no need to include <math.h> everywhre
*/

float forgeSin(float ANGLE)
{
    return sinf(ANGLE);
}

float forgeCos(float ANGLE)
{
    return cosf(ANGLE);
}

float forgeTan(float ANGLE)
{
    return tanf(ANGLE);
}

float forgeAcos(float ANGLE)
{
    return acosf(ANGLE);
}

float forgeAbs(float VALUE)
{
    return fabsf(VALUE);
}

float forgeSqrt(float VALUE)
{
    return sqrtf(VALUE);
}

int forgeRandom()
{
    if (!randomlySeeded)
    {
        srand((unsigned int)platformGetTime());
        randomlySeeded = true;
    }
    return rand();
}

int forgeRandomRange(int MIN, int MAX)
{
    if (!randomlySeeded)
    {
        srand((unsigned int)platformGetTime());
        randomlySeeded = true;
    }
    return (rand() % (MAX - MIN + 1)) + MIN;
}

float forgeFloatRandom()
{
    return (float)forgeRandom() / (float)RAND_MAX;
}

float forgeFloatRandomRange(float MIN, float MAX)
{
    return ((float)forgeRandom() / ((float)RAND_MAX / (MAX - MIN))) + MIN;
}
