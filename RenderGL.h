//---------------------------------------------------------------------------
#ifndef RenderGLH
#define RenderGLH

#include <gl/gl.h>
#include <gl/glu.h>

#include "Grid.h"

#define SELBUFSZ 256

extern const double ppUnit; 

struct GLColor
{
    GLclampf r,g,b,a;
};

struct GLVec
{
    GLfloat x,y,z;
};

struct GLSelection
{
    GLuint  numhits;
    GLfloat z1, z2;
    GLuint  name;
};

struct GLPort
{
    HPALETTE hPal;
    HGLRC hRC;
    HDC hDrawDC;
    bool Created;

    bool Active;
    void Activate();
    void Deactivate();

    int w,h;
    GLColor ColBck;
    GLColor ColGrid;
    GLColor ColHull[2];
    GLColor ColObj[2];
    GLColor ColShld[2];
    GLColor ColSelSing[2];
    GLColor ColSelMult[2];
    GLColor ColSelResl[2];
    GLColor Fore, Back;
    GLVec CamTrans,CamRot;
    GLfloat CamScale;
    bool UseLight;

    GLSelection SelectBuf[SELBUFSZ];
    GLuint nHits;
    GLuint CurSel;

    GLUquadricObj *qobj;

    bool Initialize();
    void Clear();
    void StartUp();
    void ResetCamera();

    void ResetSize(bool mkcur);
    void BeginDrawing();
    void EndDrawing();

    void SimplePick(int x, int y);

    Grid grid;

    // Stuff so we can unproject later on
    GLint viewport[4];
    GLdouble mvmatrix[16], projmatrix[16];

    GLPort();
    ~GLPort();
};

extern GLPort MainGL;



//---------------------------------------------------------------------------
#endif
