// +-------------------------------------------< PREPROCESSING >--------------------------------------------+

#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

// +----------------------------------------------< INCLUDE >-----------------------------------------------+

#include <cassert>
#include <cstdint>
#include <cstdio>

// +------------------------------------------< TYPE DEFINITION >-------------------------------------------+

typedef uint8_t byte_t;

// +------------------------------------------< GLOBAL VARIABLE >-------------------------------------------+

static const size_t WIDTH  = 880;
static const size_t HEIGHT = 880;

byte_t inputImage[WIDTH * HEIGHT];
byte_t outputImage[WIDTH * HEIGHT];

// +---------------------------------------< HISTOGRAM EQUALIZATION >---------------------------------------+

byte_t* HistogramEqualization(byte_t* inputImage, byte_t* outputImage)
{
    assert(inputImage  != NULL);
    assert(outputImage != NULL);

    uint32_t histogram[256]    = { 0 };
    double   histogramCDF[256] = { 0.0 };

    for (unsigned int iy = 0; iy < HEIGHT; ++iy)
        for (unsigned int ix = 0; ix < WIDTH; ++ix)
            histogram[inputImage[iy * WIDTH + ix]]++;

    histogramCDF[0] = static_cast<double>(histogram[0]) / static_cast<double>(WIDTH * HEIGHT);
    for (int brightness = 1; brightness < 256; ++brightness)
        histogramCDF[brightness] = static_cast<double>(histogram[brightness]) / static_cast<double>(WIDTH * HEIGHT) + histogramCDF[brightness - 1];

    for (unsigned int iy = 0; iy < HEIGHT; ++iy)
        for (unsigned int ix = 0; ix < WIDTH; ++ix)
            outputImage[iy * WIDTH + ix] = static_cast<byte_t>(255 * histogramCDF[inputImage[iy * WIDTH + ix]] + 0.5);

    return outputImage;
}

// +------------------------------------------------< MAIN >------------------------------------------------+

int main(void)
{
    static const char* INPUT_RAW_FILE_NAME  = "chest.raw";
    static const char* OUTPUT_RAW_FILE_NAME = "chest_Equalization.raw";

    FILE* fileStream;

    fileStream = fopen(INPUT_RAW_FILE_NAME, "rb");
    fread(inputImage, sizeof(inputImage), 1, fileStream);
    fclose(fileStream);

    HistogramEqualization(inputImage, outputImage);

    fileStream = fopen(OUTPUT_RAW_FILE_NAME, "w+b");
    fwrite(outputImage, sizeof(outputImage), 1, fileStream);
    fclose(fileStream);

    return 0;
}

// +------------------------------------------------< END >-------------------------------------------------+