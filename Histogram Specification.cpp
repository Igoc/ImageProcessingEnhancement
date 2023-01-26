// +-------------------------------------------< PREPROCESSING >--------------------------------------------+

#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

// +----------------------------------------------< INCLUDE >-----------------------------------------------+

#include <cassert>
#include <cinttypes>
#include <cstdio>

// +------------------------------------------< TYPE DEFINITION >-------------------------------------------+

typedef uint8_t byte_t;

// +---------------------------------------< BRIGHTNESS OF INTEREST >---------------------------------------+

enum class BOI : uint8_t
{
    DARKNESS  = 0,
    LIGHTNESS = 1
};

// +------------------------------------------< GLOBAL VARIABLE >-------------------------------------------+

static const size_t WIDTH  = 256;
static const size_t HEIGHT = 256;

byte_t inputImage[WIDTH * HEIGHT];
byte_t outputImage[WIDTH * HEIGHT];

// +--------------------------------------< HISTOGRAM SPECIFICATION >---------------------------------------+

float* CreateDesiredCDF(float* desiredCDF, BOI boi, const byte_t maxBrightness = 255)
{
    assert(desiredCDF != NULL);

    byte_t desiredValue = (boi == BOI::DARKNESS) ? (0) : (255);
    float  curvature    = 1.0F / static_cast<float>(maxBrightness * maxBrightness);

    for (int brightness = 0; brightness < maxBrightness + 1; ++brightness)
        desiredCDF[brightness] = static_cast<float>((desiredValue - brightness) * (desiredValue - brightness)) / static_cast<float>(maxBrightness * maxBrightness);

    return desiredCDF;
}

byte_t* HistogramSpecification(byte_t* inputImage, byte_t* outputImage, BOI boi = BOI::DARKNESS, const byte_t maxBrightness = 255)
{
    assert(inputImage  != NULL);
    assert(outputImage != NULL);

    uint32_t histogram[256]     = { 0 };
    float    inputImageCDF[256] = { 0.0F };
    float    desiredCDF[256]    = { 0.0F };

    for (unsigned int iy = 0; iy < HEIGHT; ++iy)
        for (unsigned int ix = 0; ix < WIDTH; ++ix)
            histogram[inputImage[iy * WIDTH + ix]]++;

    for (int brightness = 0; brightness < maxBrightness + 1; ++brightness)
        inputImageCDF[brightness] = static_cast<float>(histogram[brightness]) / static_cast<float>(WIDTH * HEIGHT) + ((brightness > 0) ? (inputImageCDF[brightness - 1]) : (0));
    CreateDesiredCDF(desiredCDF, boi);

    for (unsigned int iy = 0; iy < HEIGHT; ++iy)
        for (unsigned int ix = 0; ix < WIDTH; ++ix)
            for (int brightness = 0; brightness < maxBrightness + 1; ++brightness)
                if (inputImageCDF[inputImage[iy * WIDTH + ix]] <= desiredCDF[brightness])
                {
                    outputImage[iy * WIDTH + ix] = brightness;
                    break;
                }

    return outputImage;
}

// +------------------------------------------------< MAIN >------------------------------------------------+

int main(void)
{
    static const char* INPUT_RAW_FILE_NAME  = "Room.raw";
    static const char* OUTPUT_RAW_FILE_NAME = "Room_Specification.raw";

    FILE* fileStream;

    fileStream = fopen(INPUT_RAW_FILE_NAME, "rb");
    fread(inputImage, sizeof(inputImage), 1, fileStream);
    fclose(fileStream);

    HistogramSpecification(inputImage, outputImage);

    fileStream = fopen(OUTPUT_RAW_FILE_NAME, "w+b");
    fwrite(outputImage, sizeof(outputImage), 1, fileStream);
    fclose(fileStream);

    return 0;
}

// +------------------------------------------------< END >-------------------------------------------------+