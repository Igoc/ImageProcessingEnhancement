// +-------------------------------------------< PREPROCESSING >--------------------------------------------+

#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

// +----------------------------------------------< INCLUDE >-----------------------------------------------+

#include <cassert>
#include <cinttypes>
#include <cstdio>

// +------------------------------------------< TYPE DEFINITION >-------------------------------------------+

typedef uint8_t  byte_t;
typedef uint32_t lbyte_t;

// +------------------------------------------< GLOBAL VARIABLE >-------------------------------------------+

static const size_t WIDTH  = 512;
static const size_t HEIGHT = 512;

byte_t inputImage[WIDTH * HEIGHT];
byte_t zeroInterpolatorImage[4 * WIDTH * HEIGHT];
byte_t firstInterpolatorImage[4 * WIDTH * HEIGHT];

// +--------------------------------------------< INTERPOLATOR >--------------------------------------------+

byte_t* ZeroOrderInterpolator(byte_t* inputImage, byte_t* outputImage, const int magnification)
{
    assert(inputImage  != NULL);
    assert(outputImage != NULL);

    for (int iy = 0; iy < magnification * HEIGHT; ++iy)
        for (int ix = 0; ix < magnification * WIDTH; ++ix)
            outputImage[iy * (magnification * WIDTH) + ix] = inputImage[(iy / magnification) * WIDTH + (ix / magnification)];

    return outputImage;
}

byte_t* FirstOrderInterpolator(byte_t* inputImage, byte_t* outputImage)
{
    assert(inputImage  != NULL);
    assert(outputImage != NULL);

    lbyte_t interimData;

    for (int iy = 0; iy < 2 * HEIGHT; ++iy)
        for (int ix = 0; ix < 2 * WIDTH; ++ix)
            outputImage[iy * (2 * WIDTH) + ix] = inputImage[(iy / 2) * WIDTH + (ix / 2)];

    for (int iy = 0; iy < 2 * HEIGHT; iy += 2)
        for (int ix = 1; ix < 2 * WIDTH; ix += 2)
        {
            interimData = outputImage[iy * (2 * WIDTH) + (ix - 1)] + outputImage[iy * (2 * WIDTH) + (ix + 1)];
            outputImage[iy * (2 * WIDTH) + ix] = static_cast<byte_t>(interimData / 2.0F + 0.5F);
        }

    for (int iy = 1; iy < 2 * HEIGHT; iy += 2)
        for (int ix = 0; ix < 2 * WIDTH; ix += 2)
        {
            interimData = outputImage[(iy - 1) * (2 * WIDTH) + ix] + outputImage[(iy + 1) * (2 * WIDTH) + ix];
            outputImage[iy * (2 * WIDTH) + ix] = static_cast<byte_t>(interimData / 2.0F + 0.5F);
        }

    return outputImage;
}

// +------------------------------------------------< MAIN >------------------------------------------------+

int main(void)
{
    static const char* INPUT_RAW_FILE_NAME                     = "Lena.raw";
    static const char* OUTPUT_ZERO_INTERPOLATOR_RAW_FILE_NAME  = "Lena_ZeroInterpolator.raw";
    static const char* OUTPUT_FIRST_INTERPOLATOR_RAW_FILE_NAME = "Lena_FirstInterpolator.raw";

    FILE* fileStream;

    fileStream = fopen(INPUT_RAW_FILE_NAME, "rb");
    fread(inputImage, sizeof(inputImage), 1, fileStream);
    fclose(fileStream);

    ZeroOrderInterpolator(inputImage, zeroInterpolatorImage, 2);
    FirstOrderInterpolator(inputImage, firstInterpolatorImage);

    fileStream = fopen(OUTPUT_ZERO_INTERPOLATOR_RAW_FILE_NAME, "w+b");
    fwrite(zeroInterpolatorImage, sizeof(zeroInterpolatorImage), 1, fileStream);
    fclose(fileStream);

    fileStream = fopen(OUTPUT_FIRST_INTERPOLATOR_RAW_FILE_NAME, "w+b");
    fwrite(firstInterpolatorImage, sizeof(firstInterpolatorImage), 1, fileStream);
    fclose(fileStream);

    return 0;
}

// +------------------------------------------------< END >-------------------------------------------------+