#include "Bitmap.h"
#include "SRAM/VRAM.h"


extern VRAM graphics;
bool Bitmap::ParseFileHeader(File* bmpFile) {

    Serial.print("Parsing file header.. ");
    uint16_t read = read16(*bmpFile);
	if (read == 0x4D42 || read == 0x5089) { // BMP signature
		Serial.print(F("File size: ")); Serial.println(read32(*bmpFile));
		(void)read32(*bmpFile); // Read & ignore creator bytes
		bmpImageoffset = read32(*bmpFile); // Start of image data
		Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
		// Read DIB header
		Serial.print(F("Header size: ")); Serial.println(read32(*bmpFile));
		bmpWidth = read32(*bmpFile);
		bmpHeight = read32(*bmpFile);
		if (read16(*bmpFile) == 1) { // # planes -- must be '1'
			bmpDepth = read16(*bmpFile); // bits per pixel
			Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
			return true;
		}
	} else{
        Serial.print("Invalid BMP file");
    }
	return false;
}

Bitmap::Bitmap()
{
	
}

BitmapFileInfo Bitmap::getFileInfo(char* filename)
{
	if ((bmpFile = SD.open(filename)) == NULL) {
		Serial.println(F("File not found"));
		return BitmapFileInfo(0,0,0);
	}
	if (ParseFileHeader(&bmpFile)) {
		//printf("Bitmap (%d,%d) offset: %d\r\n", bmpWidth, bmpHeight, bmpImageoffset);		
	}
	else
		printf("error parsing file %s",filename);
	
	
	return BitmapFileInfo(bmpWidth, bmpHeight, bmpImageoffset);
}

bool Bitmap::drawFile(const char* filename, int x, int y, uint16_t bg_color)
{
	uint32_t rowSize;               // Not always = bmpWidth; may have padding
	 // pixel in buffer (R+G+B+A per pixel is biggest, although we might only use 24 bit or 16 bit/2byte)
	  // pixel out buffer (16-bit per pixel)
	
    char buf[256];
	boolean  goodBmp = false;       // Set to true on valid header parse
	boolean  flip = true;        // BMP is stored bottom-to-top
	uint16_t      w, h, row, col;
	uint32_t  r, g, b, a;
	uint32_t pos = 0, startTime = millis();
	uint8_t  lcdidx = 0;
	//boolean  first = true;
	/*uint16_t red_mask = 0xF800;
	uint16_t green_mask = 0x7E0;
	uint16_t blue_mask = 0x1F;
	uint8_t red_value = 0;
	uint8_t green_value = 0;
	uint8_t blue_value = 0;*/

	unsigned long initialStartTime = 0,
		loadTime = 0, drawTime = 0, loadTimeTotal = 0, drawTimeTotal = 0, calcTime = 0, calcTimeTotal = 0;

	if ((x >= graphics.settings.screenWidth) || (y >= graphics.settings.screenHeight)) return false;

	Serial.println();
	//Serial.print(F("Loading image '"));
	//Serial.print(filename);
	//Serial.println('\'');
	// Open requested file on SD card
	bool parsed = false;
	if (bmpFile.size() > 0) parsed = true;
	else {
		if ((bmpFile = SD.open(filename)) == NULL) {
			Serial.println(F("Error drawing image: File not found"));
			return false;
		}
		parsed = ParseFileHeader(&bmpFile);
	}

	if (!parsed)  return false;
    int buffSize = bmpWidth * (bmpDepth / 8);
    uint8_t  sdbuffer[buffSize];
    uint8_t lcdbuffer[buffSize];
    uint16_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
	if ((bmpDepth == 16 || bmpDepth == 24 || bmpDepth == 32 || bmpDepth == 8)) {
		uint8_t compress = read32(bmpFile);
		if (compress != 0)// 0 = uncompressed
			printf("Compression flag should be zero. Instead: %d\r\n", compress);

		byte hi, low;
		
		goodBmp = true; // Supported BMP format -- proceed!
		//printf("Drawing BMP file with size of %d x %d and depth of %d at (%d, %d)\r\n", bmpWidth, bmpHeight, bmpDepth, x, y);
		rowSize = (bmpWidth * (bmpDepth / 8) + (bmpDepth / 8)) & ~3;

		uint8_t red_value = 0;
		uint8_t green_value = 0;
		uint8_t blue_value = 0;

		if (bmpDepth == 32) {
			uint16_t pixel = 0;// _tft->readPixel(1, 1);
			uint16_t red_mask = 0xF800;
			uint16_t green_mask = 0x7E0;
			uint16_t blue_mask = 0x1F;

			red_value = (pixel & red_mask) >> 11;
			green_value = (pixel & green_mask) >> 5;
			blue_value = (pixel & blue_mask);

			rowSize = bmpWidth * (bmpDepth / 8);
			//printf("Blending with backgorund color %d, %d, %d\r\n", red_value, green_value, blue_value);
		}

		// BMP rows are padded (if needed) to 4-byte boundary

		// If bmpHeight is negative, image is in top-down order.
		// This is not canon but has been observed in the wild.
		if (bmpHeight < 0) {
			bmpHeight = -bmpHeight;
			flip = false;
		}

		// Crop area to be loaded
		w = bmpWidth;
		h = bmpHeight;
		// if ((x + w - 1) >= graphics.settings.screenHeight)  w = graphics.settings.screenWidth - x;
		// if ((y + h - 1) >= graphics.settings.screenHeight) h = graphics.settings.screenHeight - y;

		////override with passed values
		//w = scaleW;
		//h = scaleH;

		// Set TFT address window to clipped image bounds
		//_tft->setAddrWindow(x, y, x + w - 1, y + h - 1);

		for (row = 0; row < min(h,graphics.settings.screenHeight); row++) { 
			
			if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
				pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
			else     // Bitmap is stored top-to-bottom
				pos = bmpImageoffset + row * rowSize;
			if (bmpFile.position() != pos) { // Need seek?
				bmpFile.seek(pos);
				buffidx = buffSize; // Force buffer reload
				//Serial.print("  *  FORCE RELOAD  *  ");

			}
            lcdidx = 0;		
			startTime = millis();
			bmpFile.read(sdbuffer, buffSize);
			buffidx = 0; // Set index to beginning
			loadTime += (millis() - startTime);
			startTime = millis();

			for (col = 0; col < w; col++) { // For each column...
											// Time to read more pixel data?	
               
				// Convert pixel from BMP to TFT format
				switch (bmpDepth) {
                case 8:
                    lcdbuffer[lcdidx++] = sdbuffer[buffidx++];
                    break;
				case 16:
					hi = sdbuffer[buffidx++];
					low = sdbuffer[buffidx++];
					lcdbuffer[lcdidx++] = (r >> 1) << 5 | (g >> 2) << 2 | b >>1;
					break;
				case 24:
					b = sdbuffer[buffidx++];
					g = sdbuffer[buffidx++];
					r = sdbuffer[buffidx++];
					lcdbuffer[lcdidx++] =(r >> 2) << 5 | (g >> 2) << 2 | b >>1;
					break;
				case 32:
					a = sdbuffer[buffidx + 3]; //read alpha to determine if we should read colors
					if (a & 0x80) {
						b = sdbuffer[buffidx++];
						g = sdbuffer[buffidx++];
						r = sdbuffer[buffidx++];
						buffidx++; //a
						lcdbuffer[lcdidx++] = (r >> 2) << 5 | (g >> 2) << 2 | b >>1;
					}
					else {
						//lcdbuffer[lcdidx++] = bg_color;
						buffidx += 4;
					}

					break;
				default:
					printf("Unknown BMP depth %d in file %s", bmpDepth, filename);
					return false;
				}
                //graphics.drawPixel(col,row, (r >> 2) << 5 | (g >> 2) << 2 | b >>1);
				//_tft->setTextColor(_tft->color565(r, g, b));
				
			} // end pixel
            if (lcdidx > 0) {
                //calculate pixel ratio
                double pixelRatio = (double)w / (double)graphics.settings.screenWidth;
                sprintf(buf,  "Image to screen pixel ratio: %2.3lf",  pixelRatio);
                Serial.println(buf);
                //above 1 if the screen is smaller than image
                if(pixelRatio > 1){
                    //collapse, calculating approximate middle
                    for(int idx = 0; idx < graphics.settings.screenWidth; idx++){
                        int sourcePixel = int(((double)idx)*pixelRatio);
                        
                        
                        //get neighbors based on pixel ratio, adjust color to match original pixel's neighbors
                        int offsetLeftIdx = (int)round(sourcePixel - pixelRatio);
                        int offsetRightIdx = (int)round(sourcePixel + pixelRatio);

                        if(offsetLeftIdx < 0){
                            offsetRightIdx -= offsetLeftIdx;
                            offsetLeftIdx = 0;
                        }
                        byte color = lcdbuffer[sourcePixel];
                        for(int calcIdx = offsetLeftIdx; calcIdx < offsetRightIdx; calcIdx++){
                            // int distance = abs(sourcePixel - calcIdx);
                            // double weight = ((pixelRatio - distance) *100 / 110);
                            // sprintf(buf, 
                            //     "Adjusting color %d in pixel (%d,%d) from pixel (%d,%d) with distance %d and weight %2.3lf to pixel (%d,%d)", 
                            //     color,
                            //     sourcePixel, row,
                            //     calcIdx, row,
                            //     distance,
                            //     weight,
                            //     idx, row
                            // );
                            // Serial.println(buf);
                            color = VRAM::mulitplyColors(color, lcdbuffer[calcIdx]);
                            //color += (lcdbuffer[calcIdx] * weight);
                        }
                        lcdbuffer[idx] = color;

                    }
                    graphics.drawBuffer(0, row, graphics.settings.screenWidth, 1, lcdbuffer);
                }
                else{
                    graphics.drawBuffer(0, row, w, 1, lcdbuffer);
                    
                }
            drawTime += millis() - startTime;
			//_tft->pushColors(lcdbuffer, lcdidx, first);
		}
		} // end scanline
        calcTime += millis() - startTime;
        
			// Write any remaining data to LCD
		
		Serial.print(bmpDepth); Serial.println(" bit - ");
		Serial.print("Read File: "); Serial.print(loadTime); Serial.println("ms.");
		Serial.print("Draw Bytes: "); Serial.print(drawTime); Serial.println("ms.");
		Serial.print("Calc Time: "); Serial.print(calcTime); Serial.println("ms.");
		Serial.print("Total Time: "); Serial.print(millis() - initialStartTime); Serial.println("ms.");
	} // end goodBmp

	bmpFile.close();
	if (!goodBmp) Serial.println(F("BMP format not recognized."));


}