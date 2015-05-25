//---------------------------------------------------------------------------
#ifndef ImageFilesH
#define ImageFilesH

#include <gl/gl.h>
#include <gl/glu.h>

struct PCXHeader
{
    char Manufacturer; // Constant Flag, 10 = ZSoft .pcx
    char Version;      // Vrsion information
                       //     0 = Version 2.5 of PC Paintbrush
                       //     2 = Version 2.8 w/palette information
                       //     3 = Version 2.8 w/o palette information
                       //     4 = PC Paintbrush for Windows(Plus for
                       //         Windows uses Ver 5)
                       //     5 = Version 3.0 and > of PC Paintbrush
                       //         and PC Paintbrush +, includes
                       //         Publisher's Paintbrush . Includes
                       //         24-bit .PCX files
    char Encoding;     // 1 = .PCX run length encoding
    char BitsPerPixel; // Number of bits to represent a pixel
                       //     (per Plane) - 1, 2, 4, or 8
    short xMin;
    short yMin;
    short xMax;
    short yMax;
    short HDpi;        // Horizontal Resolution of image in DPI*
    short VDpi;        // Vertical Resolution of image in DPI*
    char Colormap[48]; // Color palette setting, see text
    char Reserved;     // Should be set to 0.
    char NPlanes;      // Number of color planes
    short BytesPerLine;// Number of bytes to allocate for a scanline
                       //     plane.  MUST be an EVEN number.  Do NOT
                       //     calculate from Xmax-Xmin.
    short PaletteInfo; // How to interpret palette- 1 = Color/BW,
                       //     2 = Grayscale (ignored in PB IV/ IV +)
    short HscreenSize; // Horizontal screen size in pixels. New field
                       //     found only in PB IV/IV Plus
    short VscreenSize; // Vertical screen size in pixels. New field
                       //     found only in PB IV/IV Plus
    char   Filler[54]; // Blank to fill out 128 byte header.  Set all
                       //     bytes to 0
};

struct OGLTexture
{
    int width, height;
    float su, sv;
    GLuint name;

    bool LoadFromPCX(AnsiString FileName);
    bool SetCurrent();

    ~OGLTexture();
};

bool ConvertPCXtoBMP(AnsiString FileIn, AnsiString FileOut);

//---------------------------------------------------------------------------
#endif
