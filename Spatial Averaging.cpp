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

// +---------------------------------------------< CHECK TIME >---------------------------------------------+

#define CHECK_TIME_START(start, freq)            { if (QueryPerformanceFrequency((LARGE_INTEGER*)&freq)) QueryPerformanceCounter((LARGE_INTEGER*)&start); }
#define CHECK_TIME_END(start, freq, elapsedTime) { __int64 end; QueryPerformanceCounter((LARGE_INTEGER*)&end); elapsedTime = ((float)end - (float)start) / (float)freq * 1000.0F; }

// +------------------------------------------< TYPE DEFINITION >-------------------------------------------+

typedef uint8_t  byte_t;
typedef uint32_t lbyte_t;

// +------------------------------------------< GLOBAL VARIABLE >-------------------------------------------+

static const size_t WIDTH  = 3136;
static const size_t HEIGHT = 2199;

byte_t  inputImage[WIDTH * HEIGHT];
byte_t  spatialAveragingImage[WIDTH * HEIGHT];
byte_t  separableSpatialAveragingImage[WIDTH * HEIGHT];
lbyte_t integralImage[WIDTH * HEIGHT];
byte_t  normalizationIntegralImage[WIDTH * HEIGHT];
byte_t  integralSpatialAveragingImage[WIDTH * HEIGHT];

// +-------------------------------------------< AVERAGING BLUR >-------------------------------------------+

byte_t CalculatePixelWindowAverage(byte_t* image, POINT center, SIZE wsize)
{
    assert(image != NULL);
    assert(center.x >= wsize.cx / 2 && center.x < WIDTH - wsize.cx / 2);
    assert(center.y >= wsize.cy / 2 && center.y < HEIGHT - wsize.cy / 2);
    assert(wsize.cx % 2 == 1);
    assert(wsize.cy % 2 == 1);

    long pixelSum = 0;

    for (int wy = -wsize.cy / 2; wy <= wsize.cy / 2; ++wy)
        for (int wx = -wsize.cx / 2; wx <= wsize.cx / 2; ++wx)
            pixelSum += inputImage[(center.y + wy) * WIDTH + (center.x + wx)];

    return static_cast<byte_t>(pixelSum / (wsize.cx * wsize.cy) + 0.5);
}

byte_t CalculateIntegralWindowAverage(lbyte_t* integralImage, POINT center, SIZE wsize)
{
    assert(integralImage != NULL);
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

byte_t* NormalizationIntegralImage(lbyte_t* integralImage, byte_t* normalizationIntegralImage)
{
    assert(integralImage              != NULL);
    assert(normalizationIntegralImage != NULL);

    for (int iy = 0; iy < HEIGHT; ++iy)
        for (int ix = 0; ix < WIDTH; ++ix)
            normalizationIntegralImage[iy * WIDTH + ix] = static_cast<float>(integralImage[iy * WIDTH + ix]) / static_cast<float>(integralImage[WIDTH * HEIGHT - 1]) * 255;

    return normalizationIntegralImage;
}

byte_t* AveragingBlur(byte_t* inputImage, byte_t* outputImage, const int wsize)
{
    assert(inputImage  != NULL);
    assert(outputImage != NULL);
    assert(wsize % 2   == 1);

    memcpy(outputImage, inputImage, WIDTH * HEIGHT);

    for (int iy = wsize / 2; iy < HEIGHT - wsize / 2; ++iy)
        for (int ix = wsize / 2; ix < WIDTH - wsize / 2; ++ix)
            outputImage[iy * WIDTH + ix] = CalculatePixelWindowAverage(inputImage, { ix, iy }, { wsize, wsize });

    return outputImage;
}

byte_t* SeparableAveragingBlur(byte_t* inputImage, byte_t* outputImage, const int wsize)
{
    assert(inputImage  != NULL);
    assert(outputImage != NULL);
    assert(wsize % 2   == 1);

    byte_t* interimImage = new byte_t[WIDTH * HEIGHT];

    memcpy(interimImage, inputImage, WIDTH * HEIGHT);
    memcpy(outputImage, inputImage, WIDTH * HEIGHT);

    for (int iy = wsize / 2; iy < HEIGHT - wsize / 2; ++iy)
        for (int ix = wsize / 2; ix < WIDTH - wsize / 2; ++ix)
            interimImage[iy * WIDTH + ix] = CalculatePixelWindowAverage(inputImage, { ix, iy }, { 1, wsize });

    for (int iy = wsize / 2; iy < HEIGHT - wsize / 2; ++iy)
        for (int ix = wsize / 2; ix < WIDTH - wsize / 2; ++ix)
            outputImage[iy * WIDTH + ix] = CalculatePixelWindowAverage(interimImage, { ix, iy }, { wsize, 1 });

    delete[] interimImage;

    return outputImage;
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

// +------------------------------------------------< MAIN >------------------------------------------------+

int main(void)
{
    static const char* INPUT_RAW_FILE_NAME                = "Snow.raw";
    static const char* OUTPUT_AVG_RAW_FILE_NAME           = "Snow_Avg.raw";
    static const char* OUTPUT_SEPARABLE_AVG_RAW_FILE_NAME = "Snow_SeparableAvg.raw";
    static const char* OUTPUT_INTEGRAL_RAW_FILE_NAME      = "Snow_Integral.raw";
    static const char* OUTPUT_INTEGRAL_AVG_RAW_FILE_NAME  = "Snow_IntegralAvg.raw";

    FILE*   fileStream;
    int64_t startTime, frequency;
    float   elapsedTime;

    fileStream = fopen(INPUT_RAW_FILE_NAME, "rb");
    fread(inputImage, sizeof(inputImage), 1, fileStream);
    fclose(fileStream);

    CHECK_TIME_START(startTime, frequency);
    AveragingBlur(inputImage, spatialAveragingImage, 21);
    CHECK_TIME_END(startTime, frequency, elapsedTime);
    printf("[Averaging Blur] %fms\n", elapsedTime);

    CHECK_TIME_START(startTime, frequency);
    SeparableAveragingBlur(inputImage, separableSpatialAveragingImage, 21);
    CHECK_TIME_END(startTime, frequency, elapsedTime);
    printf("[Separable Averaging Blur] %fms\n", elapsedTime);

    CreateIntegralImage(inputImage, integralImage);
    NormalizationIntegralImage(integralImage, normalizationIntegralImage);

    CHECK_TIME_START(startTime, frequency);
    IntegralAveragingBlur(inputImage, integralImage, integralSpatialAveragingImage, 21);
    CHECK_TIME_END(startTime, frequency, elapsedTime);
    printf("[Integral Averaging Blur] %fms\n", elapsedTime);

    fileStream = fopen(OUTPUT_AVG_RAW_FILE_NAME, "w+b");
    fwrite(spatialAveragingImage, sizeof(spatialAveragingImage), 1, fileStream);
    fclose(fileStream);

    fileStream = fopen(OUTPUT_SEPARABLE_AVG_RAW_FILE_NAME, "w+b");
    fwrite(separableSpatialAveragingImage, sizeof(separableSpatialAveragingImage), 1, fileStream);
    fclose(fileStream);

    fileStream = fopen(OUTPUT_INTEGRAL_RAW_FILE_NAME, "w+b");
    fwrite(normalizationIntegralImage, sizeof(normalizationIntegralImage), 1, fileStream);
    fclose(fileStream);

    fileStream = fopen(OUTPUT_INTEGRAL_AVG_RAW_FILE_NAME, "w+b");
    fwrite(integralSpatialAveragingImage, sizeof(integralSpatialAveragingImage), 1, fileStream);
    fclose(fileStream);

    return 0;
}

// +------------------------------------------------< END >-------------------------------------------------+