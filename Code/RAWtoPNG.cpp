// Copyright(C) 2018 Cláudio Gomes

#include <stdint.h>
#include "lodepng.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <cstdlib>

using namespace std;

int main(int argc, char** argv) {
    // Checks input.
    if (argc != 4) {
	    std::cout << "Invalid command! Correct Usage: ./RAWtoPNG filename width height\n";
        return -1;
	}
    
    // Declare variables.
	char * fileName         = argv[1];
    uint32_t width          = atoi(argv[2]);
    uint32_t doubleWidth    = width << 1;
    uint32_t height         = atoi(argv[3]);
    uint32_t doubleHeight   = height << 1;
    uint32_t imageSize      = width * height;
    uint32_t quarterSize    = imageSize >> 2;

    // Check obvious invalid width and height dimensions.
    if (width < 2 || height < 2) {
		std::cout << "Invalid width or height dimensions! Should be higher than 1.\n";
        return -1;
	}
    
    // Tries to open file.
    ifstream rawImage (fileName, ifstream::binary);
    if (rawImage) {
        // Gets length of file.
        rawImage.seekg(0, rawImage.end);
        uint32_t length = rawImage.tellg();
        rawImage.seekg(0, rawImage.beg);

        // Error check of given imageSize.
        if (length < (imageSize * 1.5) || (imageSize * 1.5  - length) < -256) {
            std::cout << "Invalid width or height dimensions! Perhaps they are too big or too small?\n";
            return -1;
        }

        // Allocates memory and stores the file info into a buffer.
        uint8_t * bufferImage = new uint8_t[length];
        rawImage.read(reinterpret_cast<char*>(bufferImage), length);

        // Closes file.
        rawImage.close();

        // Array for storing the sensor information from the file.
        uint8_t * arrayImage = new uint8_t[imageSize];

        
        // Converting 12 bits buffer to 8 bits array.
        int j = 0;
        for (long long index = 0; index < length; index += 3) {
            arrayImage[j]   = bufferImage[index];
            arrayImage[j+1] = ((bufferImage[index+1] & 0x0F) << 4) | (bufferImage[index+2] >> 4);
            j+=2;
        }

        // Deletes bufferImage.
        delete[] bufferImage;
        
        // Arrays for the 4 color layers.
        uint8_t * REDImage      = new uint8_t[quarterSize];
        uint8_t * GREEN0Image   = new uint8_t[quarterSize];
        uint8_t * GREEN1Image   = new uint8_t[quarterSize];
        uint8_t * BLUEImage     = new uint8_t[quarterSize];
        
        // Getting the 4 color layers.
        int odd = 0, even = 0;
        for (long long index = 0; index < imageSize; index += 2) {
            if ((index % doubleWidth) < width) {
                REDImage[even]      = arrayImage[index];
                GREEN0Image[even]   = arrayImage[index + 1];
                even++;
            } else {
                GREEN1Image[odd]    = arrayImage[index];
                BLUEImage[odd]      = arrayImage[index + 1];
                odd++;
            }
        }
        
        // Array for color image.
        uint8_t * COLORImage = new uint8_t[imageSize * 3];

        // Debayering to a color array (Simple Linear Interpolation).
        /*long long colorIndex;
        for (long long index = 0; index < imageSize; index += 2) {
            if ((index % doubleWidth) < width) {
                COLORImage[colorIndex    ] = arrayImage[index];
                COLORImage[colorIndex + 1] = arrayImage[index + 1];
                COLORImage[colorIndex + 2] = arrayImage[index + width + 1];
                COLORImage[colorIndex + 3] = arrayImage[index];
                COLORImage[colorIndex + 4] = arrayImage[index + 1];
                COLORImage[colorIndex + 5] = arrayImage[index + width + 1];
            } else {
                COLORImage[colorIndex    ] = arrayImage[index - width];
                COLORImage[colorIndex + 1] = arrayImage[index];
                COLORImage[colorIndex + 2] = arrayImage[index + 1];
                COLORImage[colorIndex + 3] = arrayImage[index - width];
                COLORImage[colorIndex + 4] = arrayImage[index];
                COLORImage[colorIndex + 5] = arrayImage[index + 1];
            }
        }*/

        // Debayering to a color array (Bilinear Interpolation with 4 values).
        long long colorIndex;
        for (long long index = 0; index < imageSize; index += 2) {
            colorIndex = index * 3;
            if (index < doubleWidth || index > (imageSize - doubleWidth) || (index % width) < 2 || (index % width) > (width - 2)) {
                // Avoids accessing invalid or inadequate pixels.
                if ((index % doubleWidth) < width) {
                    // Red Pixel.
                    COLORImage[colorIndex    ] = arrayImage[index];
                    COLORImage[colorIndex + 1] = arrayImage[index + 1];
                    COLORImage[colorIndex + 2] = arrayImage[index + width + 1];
                    // Green0 Pixel.
                    COLORImage[colorIndex + 3] = arrayImage[index];
                    COLORImage[colorIndex + 4] = arrayImage[index + 1];
                    COLORImage[colorIndex + 5] = arrayImage[index + width + 1];
                } else {
                    // Green1 Pixel.
                    COLORImage[colorIndex    ] = arrayImage[index - width];
                    COLORImage[colorIndex + 1] = arrayImage[index];
                    COLORImage[colorIndex + 2] = arrayImage[index + 1];
                    // Blue Pixel.
                    COLORImage[colorIndex + 3] = arrayImage[index - width];
                    COLORImage[colorIndex + 4] = arrayImage[index];
                    COLORImage[colorIndex + 5] = arrayImage[index + 1];
                }
            } else {
                if ((index % doubleWidth) < width) {
                    // Red Pixel.
                    COLORImage[colorIndex    ] = arrayImage[index];
                    COLORImage[colorIndex + 1] = (arrayImage[index + 1] + arrayImage[index - 1] + arrayImage[index + doubleWidth + 1] + arrayImage[index - doubleWidth + 1]) >> 2;
                    COLORImage[colorIndex + 2] = (arrayImage[index + width + 1] + arrayImage[index - width + 1] + arrayImage[index + width + 3] + arrayImage[index + width - 1]) >> 2;
                    // Green0 Pixel.
                    COLORImage[colorIndex + 3] = (arrayImage[index] + arrayImage[index + 2] + arrayImage[index + doubleWidth] + arrayImage[index - doubleWidth]) >> 2;
                    COLORImage[colorIndex + 4] = arrayImage[index + 1];
                    COLORImage[colorIndex + 5] = (arrayImage[index + width + 1] + arrayImage[index - width + 1] + arrayImage[index + width + 3] + arrayImage[index + width - 1]) >> 2;
                } else {
                    // Green1 Pixel.
                    COLORImage[colorIndex    ] = (arrayImage[index - width] + arrayImage[index + width] + arrayImage[index - width + 2] + arrayImage[index - width - 2]) >> 2;
                    COLORImage[colorIndex + 1] = arrayImage[index];
                    COLORImage[colorIndex + 2] = (arrayImage[index + 1] + arrayImage[index - 1] + arrayImage[index + doubleWidth + 1] + arrayImage[index - doubleWidth + 1]) >> 2;
                    // Blue Pixel.
                    COLORImage[colorIndex + 3] = (arrayImage[index - width] + arrayImage[index + width] + arrayImage[index - width + 2] + arrayImage[index - width - 2]) >> 2;
                    COLORImage[colorIndex + 4] = (arrayImage[index] + arrayImage[index + 2] + arrayImage[index + doubleWidth] + arrayImage[index - doubleWidth]) >> 2;
                    COLORImage[colorIndex + 5] = arrayImage[index + 1];
                }
            }
        }
        
        // Saves color layers and color image to PNG files.
        std::cout << "Saving red channel...\n";
        lodepng::encode("layerRED.png", REDImage, width >> 1, height >> 1, LCT_GREY);

        std::cout << "Saving green0 channel...\n";
        lodepng::encode("layerGREEN0.png", GREEN0Image, width >> 1, height >> 1, LCT_GREY);

        std::cout << "Saving green1 channel...\n";
        lodepng::encode("layerGREEN1.png", GREEN1Image, width >> 1, height >> 1, LCT_GREY);

        std::cout << "Saving blue channel...\n";
        lodepng::encode("layerBLUE.png", BLUEImage, width >> 1, height >> 1, LCT_GREY);

        std::cout << "Saving color image...\n";
        lodepng::encode("layerCOLOR.png", COLORImage, width, height, LCT_RGB);

        // Success message.
        std::cout << "Raw image converted successfully!\n";
        return 0;

    } else {
        std::cout << "Couldn't open specified file!\n";
        return -1;
    }
}