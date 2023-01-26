// +-------------------------------------------< PREPROCESSING >--------------------------------------------+

#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

// +----------------------------------------------< INCLUDE >-----------------------------------------------+

#include <Windows.h>

#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstring>

// +------------------------------------------< TYPE DEFINITION >-------------------------------------------+

typedef uint8_t  byte_t;
typedef uint32_t lbyte_t;

// +------------------------------------------< GLOBAL VARIABLE >-------------------------------------------+

static const size_t WIDTH  = 512;
static const size_t HEIGHT = 512;

byte_t  inputImage[WIDTH * HEIGHT];
lbyte_t integralImage[WIDTH * HEIGHT];
byte_t  integralSpatialAveragingImage[WIDTH * HEIGHT];
byte_t  outputImage[WIDTH * HEIGHT];

// +-------------------------------------------< AVERAGING BLUR >-------------------------------------------+

byte_t CalculateIntegralWindowAverage(lbyte_t* integralImage, POINT center, SIZE wsize)
{
    assert(center.x >= wsize.cx / 2 && center.x < WIDTH - wsize.cx / 2);
    assert(center.y >= wsize.cy / 2 && center.y < HEIGHT - wsize.cy / 2);
    assert(wsize.cx % 2 == 1);
    assert(wsize.cy % 2 == 1);

    long integralSum = integralImage[(center.y + wsize.cy / 2) * WIDTH + (center.x + wsize.cx / 2)];

    if (center.x > wsize.cx / 2)
        integralSum -= integralImage[(center.y + wsize.cy / 2) * WIDTH + (center.x - wsize.cx / 2 - 1)];
    if (center.y > wsize.cy / 2)
        integralSum -= integralImage[(center.y - wsize.cy / 2 - 1) * WIDTH + (center.x + wsize.cx / 2)];
    if (center.x > wsize.cx / 2 && center.y > wsize.cy / 2)
        integralSum += integralImage[(center.y - wsize.cy / 2 - 1) * WIDTH + (center.x - wsize.cx / 2 - 1)];

    return static_cast<byte_t>(integralSum / (wsize.cx * wsize.cy) + 0.5);
}

lbyte_t* CreateIntegralImage(byte_t* inputImage, lbyte_t* integralImage)
{
    assert(inputImage    != NULL);
    assert(integralImage != NULL);

    for (int iy = 0; iy < HEIGHT; ++iy)
        integralImage[iy * WIDTH] = inputImage[iy * WIDTH];

    for (int iy = 0; iy < HEIGHT; ++iy)
        for (int ix = 1; ix < WIDTH; ++ix)
            integralImage[iy * WIDTH + ix] = inputImage[iy * WIDTH + ix] + integralImage[iy * WIDTH + (ix - 1)];

    for (int ix = 0; ix < WIDTH; ++ix)
        for (int iy = 1; iy < HEIGHT; ++iy)
            integralImage[iy * WIDTH + ix] += integralImage[(iy - 1) * WIDTH + ix];

    return integralImage;
}

byte_t* IntegralAveragingBlur(byte_t* inputImage, lbyte_t* integralImage, byte_t* outputImage, const int wsize)
{
    assert(inputImage    != NULL);
    assert(integralImage != NULL);
    assert(outputImage   != NULL);

    memcpy(outputImage, inputImage, WIDTH * HEIGHT);

    for (int iy = wsize / 2; iy < HEIGHT - wsize / 2; ++iy)
        for (int ix = wsize / 2; ix < WIDTH - wsize / 2; ++ix)
            outputImage[iy * WIDTH + ix] = CalculateIntegralWindowAverage(integralImage, { ix, iy }, { wsize, wsize });

    return outputImage;
}

// +------------------------------------------< UNSHARP MASKING >-------------------------------------------+

byte_t Clipping(lbyte_t brightness)
{
    if (brightness > 255)
        return static_cast<byte_t>(255);
    else if (brightness < 0)
        return static_cast<byte_t>(0);

    return static_cast<byte_t>(brightness);
}

byte_t* UnsharpMasking(byte_t* inputImage, byte_t* blurImage, byte_t* outputImage, const float lambda = 0.3F)
{
    assert(inputImage  != NULL);
    assert(blurImage   != NULL);
    assert(outputImage != NULL);
    assert(lambda >= 0.25F && lambda <= 0.33F);

    for (int iy = 0; iy < HEIGHT; ++iy)
        for (int ix = 0; ix < WIDTH; ++ix)
            outputImage[iy * WIDTH + ix] = Clipping(inputImage[iy * WIDTH + ix] + lambda * (inputImage[iy * WIDTH + ix] - blurImage[iy * WIDTH + ix]));

    return outputImage;
}

// +------------------------------------------------< MAIN >------------------------------------------------+

int main(void)
{
    static const char* INPUT_RAW_FILE_NAME  = "Pentagon.raw";
    static const char* OUTPUT_RAW_FILE_NAME = "Pentagon_UnsharpMasking.raw";

    FILE* fileStream;

    fileStream = fopen(INPUT_RAW_FILE_NAME, "rb");
    fread(inputImage, sizeof(inputImage), 1, fileStream);
    fclose(fileStream);

    CreateIntegralImage(inputImage, integralImage);
    IntegralAveragingBlur(inputImage, integralImage, integralSpatialAveragingImage, 5);
    UnsharpMasking(inputImage, integralSpatialAveragingImage, outputImage, 3.0F);

    fileStream = fopen(OUTPUT_RAW_FILE_NAME, "w+b");
    fwrite(outputImage, sizeof(outputImage), 1, fileStream);
    fclose(fileStream);

    return 0;
}

// +------------------------------------------------< END >-------------------------------------------------+