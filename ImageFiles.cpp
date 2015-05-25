//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "Global.h"
#include "ImageFiles.h"
#include "F_Main.h"

// from PCX specification
int encget(int *pbyt, int *pcnt, FILE *fid)
{
    int i;

    *pcnt = 1;        // assume a "run" length of one
    if (EOF == (i = getc(fid)))
        return (EOF);
    if (0xC0 == (0xC0 & i))
    {
        *pcnt = 0x3F & i;
        if (EOF == (i = getc(fid)))
            return (EOF);
    }
    *pbyt = i;

    return (0);
}

bool ScalePow2(int width, int height, unsigned char *o_image,
               unsigned char **n_image, int *xSize2, int *ySize2)
{
    GLint glMaxTexDim;
    double xPow2, yPow2;
    int ixPow2, iyPow2;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTexDim);
    glMaxTexDim = min(256, glMaxTexDim);

    if (width <= glMaxTexDim)
        xPow2 = log((double)width) / log(2.0);
    else
        xPow2 = log((double)glMaxTexDim) / log(2.0);

    if (height <= glMaxTexDim)
        yPow2 = log((double)height) / log(2.0);
    else
        yPow2 = log((double)glMaxTexDim) / log(2.0);

    ixPow2 = (int)xPow2;
    iyPow2 = (int)yPow2;

    if (xPow2 != (double)ixPow2)
        ixPow2++;
    if (yPow2 != (double)iyPow2)
        iyPow2++;

    *xSize2 = 1 << ixPow2;
    *ySize2 = 1 << iyPow2;

    *n_image = new unsigned char[(*xSize2)*(*ySize2)*3];
    if (!(*n_image)) return false;

    gluScaleImage(GL_RGB, width, height,
                  GL_UNSIGNED_BYTE, o_image,
                  *xSize2, *ySize2, GL_UNSIGNED_BYTE,
                  *n_image);

    return true;
}

bool OGLTexture::LoadFromPCX(AnsiString FileName)
{
    int i;
    int l, lsize, chr, cnt;
    PCXHeader hdr;
    unsigned char pal[768];
    unsigned char *tbufr, *temp_buf;
    unsigned char *ibufr, *image_buf;

    // Clear the error buffer
    glGetError();

    // open the file
    FILE *fp = fopen(FileName.c_str(), "rb");
    if (!fp)
    {
        ErrorMsg("Could not open '%s' for binary reading.", FileName.c_str());
        return false;
    }

    // read the header
    fread(&hdr, sizeof (PCXHeader), 1, fp);
    width = hdr.BytesPerLine;
    height = (1 + hdr.yMax - hdr.yMin);

    // read the palette
    fseek(fp, -769L, SEEK_END);
    if (fgetc(fp) != 0xC)
    {
        ErrorMsg("No palette was found in '%s'.", FileName.c_str());
        return false;
    }
    fread(pal, 1, 768, fp);

    // read the image data
    // from PCX specification / slight mods
    temp_buf = new unsigned char[width*height];
    tbufr = temp_buf;
    fseek(fp, 128, SEEK_SET);
    l = 0;
    lsize = (int)hdr.BytesPerLine * hdr.NPlanes * height;
    while (l  < lsize)             // increment by cnt below
    {
        if (EOF == encget(&chr, &cnt, fp))
            break;
        for (i = 0; i < cnt; i++)
            *tbufr++ = (unsigned char)chr;
        l += cnt;
    }
    fclose(fp);

    // turn this indexed image into a 24-bit RGB image
    tbufr = temp_buf;
    image_buf = new unsigned char[width*height*3];
    ibufr = image_buf;
    while (tbufr < (temp_buf + lsize))
    {
        i = *tbufr * 3;
        *ibufr++ = pal[i];
        *ibufr++ = pal[i+1];
        *ibufr++ = pal[i+2];
        tbufr++;
    }

    // the trick is to now detect if the image is not a
    // power of 2.  If it is not, then we must rescale
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if ((fmod(log((float)width)/log(2.0f), 1.0f) > 0.000001f) ||
        (fmod(log((float)height)/log(2.0f), 1.0f) > 0.000001f))
    {
        unsigned char *new_buf;
        int nw, nh;

        if (ScalePow2(width, height, image_buf, &new_buf, &nw, &nh))
        {
            delete[] image_buf;
            image_buf = new_buf;
            width = nw;
            height = nh;
        }
        else
            ErrorMsg("Could not resize image to a power of 2.");
    }

    // Now that we have the buffer, let us generate the OGL texture
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_2D, name);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height,
                 0, GL_RGB, GL_UNSIGNED_BYTE, image_buf);

    if (glGetError() != GL_NO_ERROR)
    {
        ErrorMsg("Unable to load PCX into OpenGL memory.");
    }

    glEnable(GL_TEXTURE_2D);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // a little cleanup
    delete[] temp_buf;
    delete[] image_buf;

    return true;
}

bool OGLTexture::SetCurrent()
{
    glBindTexture(GL_TEXTURE_2D, name);

    // Clear errors
    glGetError();

    if (glGetError() != GL_NO_ERROR)
    {
        ErrorMsg("Unable to bind texture %u", name);
        return false;
    }

    return true;
}

OGLTexture::~OGLTexture()
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &name);

    if (glGetError() != GL_NO_ERROR)
    {
        ErrorMsg("Unable to bind and destroy texture %u", name);
    }
}

void WriteBMPHeader(FILE *ofile, int width, int height, char *pal)
{
    BITMAPINFOHEADER bih;
    BITMAPFILEHEADER bfh;

    int width4;
    if ((width & 3)==0)
        width4 = width;
    else
        width4 = ((width & 0xFFFFFFFC) + 4);

    memset(&bfh, 0, sizeof(BITMAPFILEHEADER));
    bfh.bfType      = 0x4D42; // 'BM'
    bfh.bfOffBits   = sizeof(bfh)+sizeof(bih)+(256*4); // +pal
    bfh.bfReserved1 = 0;
    bfh.bfReserved2 = 0;
    bfh.bfSize      = bfh.bfOffBits + (height*width4);

    memset(&bih, 0, sizeof(BITMAPINFOHEADER));
    bih.biSize     = sizeof(BITMAPINFOHEADER);
    bih.biWidth    = width;
    bih.biHeight   = height;
    bih.biCompression = BI_RGB;
    bih.biPlanes   = 1;
    bih.biBitCount = 8;
    bih.biClrUsed  = 256;
    bih.biClrImportant = 256;
    bih.biSizeImage = width4*height;
    bih.biXPelsPerMeter = bih.biYPelsPerMeter = 2384;

    fwrite(&bfh, sizeof(bfh), 1, ofile);
    fwrite(&bih, sizeof(bih), 1, ofile);

    // write the palette as 4 entries not 3
    char c0 = 0;
    char *pi = pal;
    for (int c=0; c<256; c++)
    {
        fwrite(&pi[2], 1, 1, ofile); // blue
        fwrite(&pi[1], 1, 1, ofile); // green
        fwrite(&pi[0], 1, 1, ofile); // red
        fwrite(&c0, 1, 1, ofile);
        pi += 3;
    }
}

bool ConvertPCXtoBMP(AnsiString FileIn, AnsiString FileOut)
{
    int i;
    int l, lsize, chr, cnt;
    PCXHeader hdr;
    char *ibufr, *image_buf;
    char *pal;
    int width, height;

    // open the file
    FILE *f_pcx = fopen(FileIn.c_str(), "rb");
    if (!f_pcx)
    {
        ErrorMsg("Could not open '%s' for binary reading.", FileIn.c_str());
        return false;
    }

    // read the header
    fread(&hdr, sizeof (PCXHeader), 1, f_pcx);
    width = hdr.BytesPerLine;
    height = (1 + hdr.yMax - hdr.yMin);

    // read the palette
    fseek(f_pcx, -769L, SEEK_END);
    if (fgetc(f_pcx) != 0xC)
    {
        ErrorMsg("No palette was found in '%s'.", FileIn.c_str());
        return false;
    }
    pal = new char[768];
    fread(pal, 1, 768, f_pcx);

    // read the image data
    // from PCX specification / slight mods
    image_buf = new char[width*height];
    ibufr = image_buf;
    fseek(f_pcx, 128, SEEK_SET);
    l = 0;
    lsize = (int)hdr.BytesPerLine * hdr.NPlanes * height;
    while (l  < lsize)             // increment by cnt below
    {
        if (EOF == encget(&chr, &cnt, f_pcx))
            break;
        for (i = 0; i < cnt; i++)
            *ibufr++ = (unsigned char)chr;
        l += cnt;
    }
    fclose(f_pcx);

    // calculate a new width as a multiple of 4
    int width4;
    if ((width & 3)==0)
        width4 = width;
    else
        width4 = ((width & 0xFFFFFFFC) + 4);

    // Start the fun
    // open the file
    FILE *f_bmp = fopen(FileOut.c_str(), "wb");
    if (!f_bmp)
    {
        ErrorMsg("Could not open '%s' for binary reading.", FileOut.c_str());
        return false;
    }
    WriteBMPHeader(f_bmp, width, height, pal);

    // write the real data
    char *pline = new char[width4];
    for (i=(height-1); i>=0; i--)
    {
        memset(pline, 0, width4);
        memcpy(pline, &image_buf[i*width], width);
        fwrite(pline, width4, 1, f_bmp);
    }

    // End
    fclose(f_bmp);
    delete[] pal;
    delete[] image_buf;
    delete[] pline;

    return true;
}

/*************************************************/
// ptr = packed data to unpack
// frame = where to store unpacked data to
// size = total number of unpacked pixels requested
// pal_translate = color translation lookup table (NULL if no palette translation desired)
#define PACKER_CODE                 0xEE    // Use'd by PACKING_METHOD_RLE
#define PACKING_METHOD_RLE          0       // Hoffoss's RLE format
#define PACKING_METHOD_RLE_KEY      1       // Hoffoss's key frame RLE format
#define PACKING_METHOD_STD_RLE	    2       // Standard RLE format (high bit is count)
#define PACKING_METHOD_STD_RLE_KEY  3       // Standard RLE format key frame
#define STD_RLE_CODE                0x80
int packer_code = PACKER_CODE;
int transparent_code = 254;
// unpack a pixel given the passed index and the anim_instance's palette, return bytes stuffed
int unpack_pixel(unsigned char *data, unsigned char pix, int aabitmap, int bpp)
{
	unsigned char bit_8;// = 0;
	// to remove warnings
	aabitmap;
	if(bpp != 8){
		return 0;
	}
	bit_8 = pix;
	*data = bit_8;
	return sizeof(unsigned char);
}
// unpack a pixel given the passed index and the anim_instance's palette, return bytes stuffed
int unpack_pixel_count(unsigned char *data, unsigned char pix, int count, int aabitmap, int bpp)
{
	int idx;
	unsigned char bit_8;// = 0;
	// to remove warnings
	aabitmap;
	if(bpp != 8){
		return 0;
	}
	bit_8 = pix;
	// stuff the pixel
	for(idx=0; idx<count; idx++){
		*(data + idx) = bit_8;
	}
	return sizeof(unsigned char) * count;
}
unsigned char *unpack_frame(unsigned char *ptr, unsigned char *frame, int size, unsigned char *pal_translate, int aabitmap, int bpp)
{
	int	xlate_pal, value, count;// = 0;
	int stuffed;
	int pixel_size = (bpp == 16) ? 2 : 1;

	if ( pal_translate == NULL ) {
		xlate_pal = 0;
	}
	else {
		xlate_pal = 1;
	}

	if (*ptr == PACKING_METHOD_RLE_KEY) {  // key frame, Hoffoss's RLE format
		ptr++;
		while (size > 0) {
			value = *ptr++;
			if (value != packer_code) {
				if ( xlate_pal ){
					stuffed = unpack_pixel(frame, pal_translate[value], aabitmap, bpp);
				} else {
					stuffed = unpack_pixel(frame, (unsigned char)value, aabitmap, bpp);
				}
				frame += stuffed;
				size--;
			} else {
				count = *ptr++;
				if (count < 2){
					value = packer_code;
				} else {
					value = *ptr++;
				}

				if (++count > size){
					count = size;
				}

				if ( xlate_pal ){
					stuffed = unpack_pixel_count(frame, pal_translate[value], count, aabitmap, bpp);
				} else {					
					stuffed = unpack_pixel_count(frame, (unsigned char)value, count, aabitmap, bpp);
				}

				frame += stuffed;
				size -= count;
			}
		}
	}
	else if ( *ptr == PACKING_METHOD_STD_RLE_KEY) {	// key frame, with high bit as count
		ptr++;
		while (size > 0) {
			value = *ptr++;
			if ( !(value & STD_RLE_CODE) ) {
				if ( xlate_pal ){
					stuffed = unpack_pixel(frame, pal_translate[value], aabitmap, bpp);
				} else {
					stuffed = unpack_pixel(frame, (unsigned char)value, aabitmap, bpp);
				}

				frame += stuffed;
				size--;
			} else {
				count = value & (~STD_RLE_CODE);
				value = *ptr++;

				size -= count;
//              Assert(size >= 0);

				if ( xlate_pal ){
					stuffed = unpack_pixel_count(frame, pal_translate[value], count, aabitmap, bpp);
				} else {
					stuffed = unpack_pixel_count(frame, (unsigned char)value, count, aabitmap, bpp);
				}

				frame += stuffed;
			}
		}
	}
	else if (*ptr == PACKING_METHOD_RLE) {  // normal frame, Hoffoss's RLE format
		ptr++;
		while (size > 0) {
			value = *ptr++;
			if (value != packer_code) {
				if (value != transparent_code) {
					if ( xlate_pal ){
						stuffed = unpack_pixel(frame, pal_translate[value], aabitmap, bpp);
					} else {
						stuffed = unpack_pixel(frame, (unsigned char)value, aabitmap, bpp);
					}
				} else {
					// temporary pixel
					stuffed = pixel_size;
				}

				frame += stuffed;
				size--;
			} else {
				count = *ptr++;
				if (count < 2){
					value = packer_code;
				} else {
					value = *ptr++;
				}

				if (++count > size){
					count = size;
				}

				size -= count;
//		        Assert(size >= 0);

				if (value != transparent_code ) {
					if ( xlate_pal ) {
						stuffed = unpack_pixel_count(frame, pal_translate[value], count, aabitmap, bpp);
					} else {
						stuffed = unpack_pixel_count(frame, (unsigned char)value, count, aabitmap, bpp);
					}
				} else {
					stuffed = count * pixel_size;
				}

				frame += stuffed;
			}
		}

	}
	else if ( *ptr == PACKING_METHOD_STD_RLE) {	// normal frame, with high bit as count
		ptr++;
		while (size > 0) {
			value = *ptr++;
			if ( !(value & STD_RLE_CODE) ) {
				if (value != transparent_code) {
					if ( xlate_pal ){
						stuffed = unpack_pixel(frame, pal_translate[value], aabitmap, bpp);
					} else {
						stuffed = unpack_pixel(frame, (unsigned char)value, aabitmap, bpp);
					}
				} else {
					stuffed = pixel_size;
				}

				frame += stuffed;
				size--;
			} else {
				count = value & (~STD_RLE_CODE);
				value = *ptr++;

				size -= count;
//      		Assert(size >= 0);

				if (value != transparent_code) {
					if ( xlate_pal ){
						stuffed = unpack_pixel_count(frame, pal_translate[value], count, aabitmap, bpp);
					} else {
						stuffed = unpack_pixel_count(frame, (unsigned char)value, count, aabitmap, bpp);
					}
				} else {
					stuffed = pixel_size * count;
				}

				frame += stuffed;
			}
		}
	}
	else {
		ErrorMsg("Unknown packing method in ANI file.");
	}

	return ptr;
}
/*************************************************/

//---------------------------------------------------------------------------
#pragma package(smart_init)
