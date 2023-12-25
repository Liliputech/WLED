#pragma once
#include "wled.h"
#include <PNGdec.h>
#include <AnimatedGIF.h>
#define BRIGHT_SHIFT 3
#define FX_MODE_POV_IMAGE 255
#define FX_MODE_2D_IMAGE  255
static const char _data_FX_MODE_POV_IMAGE[] PROGMEM = "POV Image@!;;;1";
static const char _data_FX_MODE_2D_IMAGE[] PROGMEM = "2D Image@!;;;2";
AnimatedGIF gif;
PNG png;
File f;

typedef struct file_tag
{
    int32_t iPos; // current file position
    int32_t iSize; // file size
    uint8_t *pData; // memory file pointer
    void * fHandle; // class pointer to File/SdFat or whatever you want
} IMGFILE;

void * openFile(const char *filename, int32_t *size) {
    f = WLED_FS.open(filename, "r");
    *size = f.size();
    return &f;
}

void closeFile(void *handle) {
    if (f) f.close();
}

int32_t readFile(IMGFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    int32_t iBytesRead;
    iBytesRead = iLen;
    File *f = static_cast<File *>(pFile->fHandle);
    // Note: If you read a file all the way to the last byte, seek() stops working
    if ((pFile->iSize - pFile->iPos) < iLen)
	iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
    if (iBytesRead <= 0)
	return 0;
    iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
    pFile->iPos = f->position();
    return iBytesRead;
}

int32_t readPNG(PNGFILE *pFile, uint8_t *pBuf, int32_t iLen)
{ return readFile((IMGFILE*) pFile, pBuf, iLen); }
    
int32_t readGIF(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{ return readFile((IMGFILE*) pFile, pBuf, iLen); }

int32_t seekFile(IMGFILE *pFile, int32_t iPosition)
{
    int i = micros();
    File *f = static_cast<File *>(pFile->fHandle);
    f->seek(iPosition);
    pFile->iPos = (int32_t)f->position();
    i = micros() - i;
    return pFile->iPos;
}

int32_t seekPNG(PNGFILE *pFile, int32_t iPos)
{ return seekFile((IMGFILE*) pFile, iPos); }
    
int32_t seekGIF(GIFFILE *pFile, int32_t iPos)
{ return seekFile((IMGFILE*) pFile, iPos); }

void pngPOV(PNGDRAW *pDraw) {
    uint16_t usPixels[SEGLEN];
    png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_LITTLE_ENDIAN, 0xffffffff);
    for(int x=0; x < SEGLEN; x++) {
	uint16_t color = usPixels[x];
	byte r = ((color >> 11) & 0x1F);
	byte g = ((color >> 5) & 0x3F);
	byte b = (color & 0x1F);
	SEGMENT.setPixelColor(x, RGBW32(r,g,b,0));
    }
    strip.show();
}

void png2D(PNGDRAW *pDraw) {
    uint16_t usPixels[pDraw->iWidth];
    for(int y=0; y < pDraw->iWidth; y++) {
	png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_LITTLE_ENDIAN, 0xffffffff);
	for(int x=0; x < SEGLEN; x++) {
	    uint16_t color = usPixels[x];
	    byte r = ((color >> 11) & 0x1F);
	    byte g = ((color >> 5) & 0x3F);
	    byte b = (color & 0x1F);
	    SEGMENT.setPixelColorXY(x, y, RGBW32(r,g,b,0));
	}
    }
    strip.show();
}

void gifPOV(GIFDRAW *pDraw) {
    uint8_t r, g, b, *s, *p, *pPal = (uint8_t *)pDraw->pPalette;
    int x, y = pDraw->iY + pDraw->y;
    s = pDraw->pPixels;
    if (pDraw->ucDisposalMethod == 2) {
	p = &pPal[pDraw->ucBackground * 3];
	r = p[0]; g = p[1]; b = p[2];
	for (x=0; x<pDraw->iWidth; x++)
	    if (s[x] == pDraw->ucTransparent)
		SEGMENT.setPixelColor(x, RGBW32(r, g, b, 0));
	pDraw->ucHasTransparency = 0;
    }


    if (pDraw->ucHasTransparency) {
	const uint8_t ucTransparent = pDraw->ucTransparent;
	for (x=0; x<pDraw->iWidth; x++)
	    if (s[x] != ucTransparent) {
		p = &pPal[s[x] * 3];
		SEGMENT.setPixelColor(x, RGBW32(p[0]>>BRIGHT_SHIFT, p[1]>>BRIGHT_SHIFT, p[2]>>BRIGHT_SHIFT, 0));
	    }
    }

    else
	for (x=0; x<pDraw->iWidth; x++) {
	    p = &pPal[s[x] * 3];
	    SEGMENT.setPixelColor(x, RGBW32(p[0], p[1], p[2], 0));
	}
    strip.show();
}

void gif2D(GIFDRAW *pDraw) {
    uint8_t r, g, b, *s, *p, *pPal = (uint8_t *)pDraw->pPalette;
    int x, y = pDraw->iY + pDraw->y;

    s = pDraw->pPixels;
    if (pDraw->ucDisposalMethod == 2) {
	p = &pPal[pDraw->ucBackground * 3];
	r = p[0] >> BRIGHT_SHIFT; g = p[1] >> BRIGHT_SHIFT; b = p[2] >> BRIGHT_SHIFT;
	for (x=0; x<pDraw->iWidth; x++)
	    if (s[x] == pDraw->ucTransparent)
	    SEGMENT.setPixelColorXY(x, y, RGBW32(r,g,b,0));
	pDraw->ucHasTransparency = 0;
    }

    if (pDraw->ucHasTransparency) {
	const uint8_t ucTransparent = pDraw->ucTransparent;
	for (x=0; x<pDraw->iWidth; x++)
	    if (s[x] != ucTransparent) {
		p = &pPal[s[x] * 3];
		r = p[0] >> BRIGHT_SHIFT; g = p[1] >> BRIGHT_SHIFT; b = p[2] >> BRIGHT_SHIFT;
		SEGMENT.setPixelColorXY(x, y, RGBW32(r,g,b,0));
	    }
    }

    else
	for (x=0; x<pDraw->iWidth; x++) {
	    p = &pPal[s[x] * 3];
	    r = p[0] >> BRIGHT_SHIFT; g = p[1] >> BRIGHT_SHIFT; b = p[2] >> BRIGHT_SHIFT;
	    SEGMENT.setPixelColorXY(x, y, RGBW32(r,g,b,0));
	}

    if (pDraw->y == pDraw->iHeight-1) // last line has been decoded, display the image
	strip.show();
}

uint16_t mode_pov_image(void) {
    const char * filepath = SEGMENT.name;
    int rc = png.open(filepath, openFile, closeFile, readPNG, seekPNG, pngPOV);
    if (rc == PNG_SUCCESS && png.getWidth() == SEGLEN) {
	rc = png.decode(NULL, 0);
	png.close();
	return FRAMETIME;
    }

    gif.begin(GIF_PALETTE_RGB888);
    rc = gif.open(filepath, openFile, closeFile, readGIF, seekGIF, gifPOV);
    if (rc && gif.getCanvasWidth() == SEGLEN) {
	while (gif.playFrame(true, NULL)) {}
	gif.close();
	return FRAMETIME;
    }
    return FRAMETIME;
}

uint16_t mode_2d_image(void) {
    const char * filepath = SEGMENT.name;
    int rc = png.open(filepath, openFile, closeFile, readPNG, seekPNG, png2D);
    if (rc == PNG_SUCCESS && png.getWidth() * png.getHeight() == SEGLEN) {
	rc = png.decode(NULL, 0);
	png.close();
	return FRAMETIME;
    }

    gif.begin(GIF_PALETTE_RGB888);
    rc = gif.open(filepath, openFile, closeFile, readGIF, seekGIF, gif2D);
    if (rc && gif.getCanvasWidth() * gif.getCanvasHeight() == SEGLEN) {
	while (gif.playFrame(true, NULL)) {}
	gif.close();
	return FRAMETIME;
    }
    return FRAMETIME;
}

class ImgDisplayUsermod : public Usermod
{
  protected:
	bool enabled = false; //WLEDMM
	bool initDone = false; //WLEDMM
	unsigned long lastTime = 0; //WLEDMM

  public:
    void setup() {
	strip.addEffect(FX_MODE_POV_IMAGE, &mode_pov_image, _data_FX_MODE_POV_IMAGE);
	strip.addEffect(FX_MODE_2D_IMAGE, &mode_2d_image, _data_FX_MODE_2D_IMAGE);
	initDone=true;
    }

    void loop() {
	if (!enabled || strip.isUpdating()) return;
	if (millis() - lastTime > 1000) {
	    lastTime = millis();
	}
    }

    void connected() {}
};
