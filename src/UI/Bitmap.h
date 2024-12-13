#ifndef _VMC_UI_BITMAP_H
#define _VMC_UI__BITMAP_H


//#include "SD-master\SD.h"
#include "SD.h"
//#include "SPFD5408_TFT_eSPI.h"
#define BUFFPIXEL 432

typedef struct BitmapFileInfo {
	uint16_t width;
	uint16_t height;
	uint16_t offset;
	BitmapFileInfo(uint16_t w, uint16_t h, uint16_t o) {
		width = w;
		height = h;
		offset = o;
	}
};


class Bitmap
{
public:
	Bitmap();
	BitmapFileInfo getFileInfo(char* filename);
	bool drawFile(const char* filename, int x, int y, uint16_t bg_color = 0x0000);


protected:
	bool ParseFileHeader(File* bmpFile);

	uint16_t bmpWidth, bmpHeight;   // W+H in pixels
	uint8_t  bmpDepth;              // Bit depth (currently must be 24)
	uint16_t bmpImageoffset;        // Start of image data in file
	//uint16_t bmpBitsPerPixel;		//bits per pixel (24 is RGB, 32 is RGBA)

	File     bmpFile;


private:
	// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

	uint16_t read16(File f) {
		uint16_t result;
		((uint8_t*)& result)[0] = f.read(); // LSB
		((uint8_t*)& result)[1] = f.read(); // MSB
		return result;
	}

	uint32_t read32(File f) {
		uint32_t result;
		((uint8_t*)& result)[0] = f.read(); // LSB
		((uint8_t*)& result)[1] = f.read();
		((uint8_t*)& result)[2] = f.read();
		((uint8_t*)& result)[3] = f.read(); // MSB
		return result;
	}
};

#endif