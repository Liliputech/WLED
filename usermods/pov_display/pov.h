#ifndef _POV_H
#define _POV_H
#include "bmpimage.h"
#ifdef WLED_ENABLE_GIF
#include "gifimage.h"
#endif


class POV {
    public:
        POV();
        
        /* Shows a column directly from image memory (for horizontal POV)
         * Uses image width/height/rowSize to compute pixel positions
         * (BMP images use padded BGR rows, GIF images a packed RGB buffer)
         */
        void showColumn(uint16_t colIndex);

        /* Reads from file an image and making it current image.
         * Dispatches on the file extension: .bmp always, .gif when the
         * firmware is built with WLED_ENABLE_GIF.
         */
        bool loadImage(const char * filename);

        /* True when the current image (BMP or GIF) is loaded */
        bool isLoaded();

        /* Width of the current image in pixels (number of POV columns) */
        int width();

        /* For animated GIFs, decodes the next frame into the image buffer.
         * No-op (returns true) for static images.
         */
        bool decodeNextFrame();

        char * getFilename();

    private:
        BMPimage image;
#ifdef WLED_ENABLE_GIF
        GIFimage gifImage;
        bool _useGif = false;
        void showColumnGif(uint16_t colIndex);
#endif
};



#endif
