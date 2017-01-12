#include "png_wrapper.h"
#include <png.h>
#include <stdlib.h>
#include <stdint.h>

/* Attempts to save PNG to file; returns 0 on success, non-zero on error. */
int save_png_to_file(RGBBitmap *bitmap, const char *path) {
    FILE *fp = fopen(path, "wb");
    if (fp == NULL) {
        return(-1);
    }
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_byte **row_pointers = NULL;

    if (fp == NULL) return -1;

    /* Initialize the write struct. */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        fclose(fp);
        return -1;
    }

    /* Initialize the info struct. */
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        return -1;
    }
    
    png_set_sRGB(png_ptr, info_ptr, 0);

    /* Set up error handling. */
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return -1;
    }

    /* Set image attributes. */
    png_set_IHDR(png_ptr,
                 info_ptr,
                 (png_uint_32)bitmap->width,
                 (png_uint_32)bitmap->height,
                 8,
                 PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    /* Initialize rows of PNG. */
    row_pointers = (png_byte **)png_malloc(png_ptr, bitmap->height * sizeof(png_bytep));
    for (size_t y = 0; y < bitmap->height; ++y) {
        row_pointers[y] = (png_bytep)bitmap->pixels + bitmap->bytewidth * y;
    }

    /* Actually write the image data. */
    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    /* Cleanup. */
    png_free(png_ptr, row_pointers);

    /* Finish writing. */
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    return 0;
}
