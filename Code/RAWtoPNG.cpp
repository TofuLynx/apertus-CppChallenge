// Copyright(C) 2018 Cláudio Gomes

#include <stdint.h>
#include "lodepng.h"
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

int main(int argc, char** argv) {
    
    // Checks input.
    if (argc != 2)
	{
		printf("Invalid command! Are you sure you wrote the filename correctly?\n");
		return -1;
	}
    
	char * fileName = argv[1];
    
    // Tries to open file.
    ifstream rawImage (fileName, ifstream::binary);
    if (rawImage) {
        // Gets length of file.
        rawImage.seekg(0, rawImage.end);
        uint32_t length = rawImage.tellg();
        rawImage.seekg(0, rawImage.beg);

        // Allocates memory and stores the file info into a buffer.
        uint8_t * bufferImage = new uint8_t[length];
        rawImage.read(reinterpret_cast<char*>(bufferImage), length);

        // Closes file.
        rawImage.close();

        // Array for storing the sensor information from the file.
        uint8_t * arrayImage = new uint8_t[4096*3072];

        // Converting 12 bits buffer to 8 bits array.
        int j = 0;
        for (long long index = 0; index < length; index += 3) {
            arrayImage[j] = bufferImage[index];
            arrayImage[j+1] = ((bufferImage[index+1] & 0x0F) << 4) | (bufferImage[index+2] >> 4);
            j+=2;
        }
        
        // Deletes bufferImage.
        delete[] bufferImage;
        
        // Arrays for the 4 color layers.
        uint8_t * REDImage = new uint8_t[2048*1536];
        uint8_t * GREEN0Image = new uint8_t[2048*1536];
        uint8_t * GREEN1Image = new uint8_t[2048*1536];
        uint8_t * BLUEImage = new uint8_t[2048*1536];
        
        // Getting the 4 color layers.
        int odd = 0, even = 0;
        for (long long index = 0; index < 4096*3072; index += 2) {
            if (index%8192 < 4096) {
                REDImage[even] = arrayImage[index];
                GREEN0Image[even] = arrayImage[index + 1];
                even++;
            } else {
                GREEN1Image[odd] = arrayImage[index];
                BLUEImage[odd] = arrayImage[index + 1];
                odd++;
            }
        }
        
        // Array for color image.
        uint8_t * COLORImage = new uint8_t[4096*3072*3];

        // Debayering to a color array (Simple Linear Interpolation).
        /*for (long long index = 0; index < 4096*3072; index += 2) {
            if (index%8192 < 4096) {
                COLORImage[index*3] = arrayImage[index];
                COLORImage[index*3+1] = arrayImage[index + 1];
                COLORImage[index*3+2] = arrayImage[index + 4097];
                COLORImage[index*3+3] = arrayImage[index];
                COLORImage[index*3+4] = arrayImage[index + 1];
                COLORImage[index*3+5] = arrayImage[index + 4097];
            } else {
                COLORImage[index*3] = arrayImage[index - 4096];
                COLORImage[index*3+1] = arrayImage[index];
                COLORImage[index*3+2] = arrayImage[index + 1];
                COLORImage[index*3+3] = arrayImage[index - 4096];
                COLORImage[index*3+4] = arrayImage[index];
                COLORImage[index*3+5] = arrayImage[index + 1];
            }
        }*/

        // Debayering to a color array (Bilinear Interpolation with 4 values).
        for (long long index = 0; index < 4096*3072; index += 2) {
            if (index < 8192 || index > 4096*3070 || index%4096 < 2 || index%4096 > 4094) {
                // Avoids accessing invalid or inadequate pixels.
                if (index%8192 < 4096) {
                    // Red Pixel.
                    COLORImage[index*3] = arrayImage[index];
                    COLORImage[index*3+1] = arrayImage[index + 1];
                    COLORImage[index*3+2] = arrayImage[index + 4097];
                    // Green0 Pixel.
                    COLORImage[index*3+3] = arrayImage[index];
                    COLORImage[index*3+4] = arrayImage[index + 1];
                    COLORImage[index*3+5] = arrayImage[index + 4097];
                } else {
                    // Green1 Pixel.
                    COLORImage[index*3] = arrayImage[index - 4096];
                    COLORImage[index*3+1] = arrayImage[index];
                    COLORImage[index*3+2] = arrayImage[index + 1];
                    // Blue Pixel.
                    COLORImage[index*3+3] = arrayImage[index - 4096];
                    COLORImage[index*3+4] = arrayImage[index];
                    COLORImage[index*3+5] = arrayImage[index + 1];
                }
            } else {
                if (index%8192 < 4096) {
                    // Red Pixel.
                    COLORImage[index*3] = arrayImage[index];
                    COLORImage[index*3+1] = (arrayImage[index + 1] + arrayImage[index - 1] + arrayImage[index + 8193] + arrayImage[index - 8191]) >> 2;
                    COLORImage[index*3+2] = (arrayImage[index + 4097] + arrayImage[index - 4095] + arrayImage[index + 4099] + arrayImage[index + 4095]) >> 2;
                    // Green0 Pixel.
                    COLORImage[index*3+3] = (arrayImage[index] + arrayImage[index + 2] + arrayImage[index + 8192] + arrayImage[index - 8192]) >> 2;
                    COLORImage[index*3+4] = arrayImage[index + 1];
                    COLORImage[index*3+5] = (arrayImage[index + 4097] + arrayImage[index - 4095] + arrayImage[index + 4099] + arrayImage[index + 4095]) >> 2;
                } else {
                    // Green1 Pixel.
                    COLORImage[index*3] = (arrayImage[index - 4096] + arrayImage[index + 4096] + arrayImage[index - 4094] + arrayImage[index - 4098]) >> 2;
                    COLORImage[index*3+1] = arrayImage[index];
                    COLORImage[index*3+2] = (arrayImage[index + 1] + arrayImage[index - 1] + arrayImage[index + 8193] + arrayImage[index - 8191]) >> 2;
                    // Blue Pixel.
                    COLORImage[index*3+3] = (arrayImage[index - 4096] + arrayImage[index + 4096] + arrayImage[index - 4094] + arrayImage[index - 4098]) >> 2;
                    COLORImage[index*3+4] = (arrayImage[index] + arrayImage[index + 2] + arrayImage[index + 8192] + arrayImage[index - 8192]) >> 2;
                    COLORImage[index*3+5] = arrayImage[index + 1];
                }
            }
        }

        
        // Saves layers to PNG files.
        std::cout << lodepng::encode("layerRED.png", REDImage, 2048,  1536, LCT_GREY) << endl;
        std::cout << lodepng::encode("layerGREEN0.png", GREEN0Image, 2048,  1536, LCT_GREY) << endl;
        std::cout << lodepng::encode("layerGREEN1.png", GREEN1Image, 2048,  1536, LCT_GREY) << endl;
        std::cout << lodepng::encode("layerBLUE.png", BLUEImage, 2048,  1536, LCT_GREY) << endl;
        std::cout << lodepng::encode("layerCOLOR.png", COLORImage, 4096,  3072, LCT_RGB) << endl;

        return 0;
    } else {
        std::cout << "Couldn't open specified file!\n";
        return -1;
    }
}
