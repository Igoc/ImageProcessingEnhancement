// +-------------------------------------------< PREPROCESSING >--------------------------------------------+

#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

// +----------------------------------------------< INCLUDE >-----------------------------------------------+

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>

// +------------------------------------------< TYPE DEFINITION >-------------------------------------------+

typedef uint8_t byte_t;

// +------------------------------------------< GLOBAL VARIABLE >-------------------------------------------+

static const size_t WIDTH  = 512;
static const size_t HEIGHT = 512;

byte_t inputImage[WIDTH * HEIGHT];
byte_t outputImage[WIDTH * HEIGHT];

// +------------------------------------------< SALT AND PEPPER >-------------------------------------------+

byte_t* CreateSaltAndPepperNoise(byte_t* inputImage, byte_t* outputImage, float ratio)
{
    assert(inputImage  != NULL);
    assert(outputImage != NULL);
    assert(ratio >= 0.0 && ratio <= 1.0);

    srand(static_cast<unsigned int>(time(NULL)));
    memcpy(outputImage, inputImage, WIDTH * HEIGHT);

    for (int index = 0; index < WIDTH * HEIGHT * ratio; ++index)
        outputImage[(rand() % HEIGHT) * WIDTH + (rand() % WIDTH)] = rand() % 2 * 255;

    return outputImage;
}

// +--------------------------------------------< MEDIAN BLUR >---------------------------------------------+

byte_t* SeparableMedianBlur(byte_t* inputImage, byte_t* outputImage, const int wsize)
{
    assert(inputImage  != NULL);
    assert(outputImage != NULL);
    assert(wsize % 2   == 1);

    byte_t*             interimImage = new byte_t[WIDTH * HEIGHT];
    std::vector<byte_t> filter(wsize, 0);

    memcpy(interimImage, inputImage, WIDTH * HEIGHT);
    memcpy(outputImage, inputImage, WIDTH * HEIGHT);

    for (int iy = 0; iy < HEIGHT; ++iy)
        for (int ix = wsize / 2; ix < WIDTH - wsize / 2; ++ix)
        {
            for (int iw = -wsize / 2; iw <= wsize / 2; ++iw)
                filter[iw + wsize / 2] = inputImage[iy * WIDTH + (ix + iw)];
            sort(filter.begin(), filter.end());

            interimImage[iy * WIDTH + ix] = filter[wsize / 2];
        }

    for (int ix = 0; ix < WIDTH; ++ix)
        for (int iy = wsize / 2; iy < HEIGHT - wsize / 2; ++iy)
        {
            for (int iw = -wsize / 2; iw <= wsize / 2; ++iw)
                filter[iw + wsize / 2] = interimImage[(iy + iw) * WIDTH + ix];
            sort(filter.begin(), filter.end());

            outputImage[iy * WIDTH + ix] = filter[wsize / 2];
        }

    delete[] interimImage;

    return outputImage;
}

// +------------------------------------------------< MAIN >------------------------------------------------+

int main(void)
{
    static const char* INPUT_RAW_FILE_NAME                   = "Lena.raw";
    static const char* OUTPUT_SALT_AND_PEPPER_RAW_FILE_NAME  = "Lena_SaltAndPepper.raw";
    static const char* OUTPUT_SEPARABLE_MEDIAN_RAW_FILE_NAME = "Lena_SeparableMedian.raw";

    FILE* fileStream;

    fileStream = fopen(INPUT_RAW_FILE_NAME, "rb");
    fread(inputImage, sizeof(inputImage), 1, fileStream);
    fclose(fileStream);

    CreateSaltAndPepperNoise(inputImage, inputImage, 0.05F);
    SeparableMedianBlur(inputImage, outputImage, 3);

    fileStream = fopen(OUTPUT_SALT_AND_PEPPER_RAW_FILE_NAME, "w+b");
    fwrite(inputImage, sizeof(inputImage), 1, fileStream);
    fclose(fileStream);

    fileStream = fopen(OUTPUT_SEPARABLE_MEDIAN_RAW_FILE_NAME, "w+b");
    fwrite(outputImage, sizeof(outputImage), 1, fileStream);
    fclose(fileStream);

    return 0;
}

// +------------------------------------------------< END >-------------------------------------------------+