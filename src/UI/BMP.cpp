#include <SD.h>
#include "SRAM/VRAM.h"
#define SD_CS 10 
File workingFile;
uint8_t bytes[54];

extern VRAM graphics;
void saveFileHeader(int byteLength);
void saveInfoHeader(int height, int width);
void saveBitmapData();
void saveBMP();

void saveBMP()
{
	/*try
	{*/
	int i = 0;
	char fileName[9];
	while (i < 64000) {
		sprintf(fileName, "%i.bmp", i);
		if (!SD.exists(fileName))
			break;
	}
	workingFile = SD.open(fileName, FILE_WRITE);

	int byteLength = 54 + 1 * (graphics.settings.screenHeight * graphics.settings.screenWidth); //1 byte per pixel

	saveFileHeader(byteLength);
	saveInfoHeader(graphics.settings.screenHeight, graphics.settings.screenWidth);
	workingFile.write(bytes, 54);
	saveBitmapData();

	workingFile.close();
	/*}
	catch(String s)
	{
	tft.print(s);
	}*/
}

void saveFileHeader(int byteLength)
{
	bytes[0] = (byte)'B';
	bytes[1] = (byte)'M';

	bytes[5] = (byte)byteLength;
	bytes[4] = (byte)(byteLength >> 8);
	bytes[3] = (byte)(byteLength >> 16);
	bytes[2] = (byte)(byteLength >> 24);

	//data offset
	bytes[10] = 54;
}

void saveInfoHeader(int height, int width)
{
	bytes[14] = 40;

	bytes[18] = (byte)width;
	bytes[19] = (byte)(width >> 8);
	bytes[20] = (byte)(width >> 16);
	bytes[21] = (byte)(width >> 24);

	bytes[22] = (byte)height;
	bytes[23] = (byte)(height >> 8);
	bytes[24] = (byte)(height >> 16);
	bytes[25] = (byte)(height >> 24);

	bytes[26] = 1;

	bytes[28] = 24;
}

void saveBitmapData() {
	for (int y = 0; y <= graphics.settings.screenHeight; y++) {
		for (int x = 0; x <= graphics.settings.screenWidth; x++) {
			uint8_t rgbVal = graphics.readPixel(x, y);
//			workingFile.write((const uint8_t)(rgbVal >> 8));
			workingFile.write((const uint8_t)(rgbVal & 0xFF));
		}
	}
}

