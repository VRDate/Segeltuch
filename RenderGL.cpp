//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "RenderGL.h"
#include "Global.h"
#include "F_Main.h"
#include "Geometry.h"
#include "Draw.h"

// global objects
GLPort MainGL;
const double ppUnit = 8.0;

// constructor
GLPort::GLPort()
{
    Created = false;
    const float s = 2.5f;

    hPal = NULL;
    hRC = NULL;
    UseLight = true;

    ColBck.r=ColBck.g=ColBck.b=0.40f;
    ColBck.a=1.0f;

    ColHull[0].r = ColHull[0].g = 1.0f;
    ColHull[0].b = 1.0f;
    ColHull[0].a = 0.4f;
    ColHull[1] = ColHull[0];
    /*ColHull[1].r = ColHull[0].r/s;
    ColHull[1].g = ColHull[0].g/s;
    ColHull[1].b = ColHull[0].b/s;
    ColHull[1].a = ColHull[0].a;*/

    ColObj[0].r = ColObj[0].a = 1.0f;
    ColObj[0].g = 0.666667f;
    ColObj[0].b = 0.2745f;
    ColObj[1].r = ColObj[0].r/s;
    ColObj[1].g = ColObj[0].g/s;
    ColObj[1].b = ColObj[0].b/s;
    ColObj[1].a = ColObj[0].a;

    ColGrid.a = 1.0f;
    ColGrid.r = ColGrid.g = ColGrid.b = 0.0f;

    ColShld[0].r = ColShld[0].g=1.0f;
    ColShld[0].b = 0.3f;
    ColShld[0].a = 0.6f;
    ColShld[1].r = ColShld[0].r/s;
    ColShld[1].g = ColShld[0].g/s;
    ColShld[1].b = ColShld[0].b/s;
    ColShld[1].a = ColShld[0].a;

    ColSelSing[0].g = 1.0;
    ColSelSing[0].a = 0.5f;
    ColSelSing[0].r = ColSelSing[0].b=0.1f;
    ColSelSing[1].r = ColSelSing[0].r/s;
    ColSelSing[1].g = ColSelSing[0].g/s;
    ColSelSing[1].b = ColSelSing[0].b/s;
    ColSelSing[1].a = ColSelSing[0].a;

    ColSelMult[0].r = 1.0;
    ColSelMult[0].a = 1.5f;
    ColSelMult[0].g = ColSelMult[0].b = 0.1f;
    ColSelMult[1].r = ColSelMult[0].r/s;
    ColSelMult[1].g = ColSelMult[0].g/s;
    ColSelMult[1].b = ColSelMult[0].b/s;
    ColSelMult[1].a = ColSelMult[0].a;

    ColSelResl[0].r = ColSelResl[0].g = 1.0;
    ColSelResl[0].a = 1.0f;
    ColSelResl[0].b = 0.1f;
    ColSelResl[1].r = ColSelResl[0].r/s;
    ColSelResl[1].g = ColSelResl[0].g/s;
    ColSelResl[1].b = ColSelResl[0].b/s;
    ColSelResl[1].a = ColSelResl[0].a;

    ResetCamera();

    Active = false;
}

// destructor
GLPort::~GLPort()
{
    if (Created) Clear();
}

void GLPort::ResetCamera()
{
    // Fix the view
    MainForm->Do_RepositionCamera(CV_Top, true);
    CamScale = 1.0f;
}

// Just some quick helpers
void GLPort::Activate()
{
    if (Active) return;

    hDrawDC = GetDC(MainForm->pnlView->Handle);
    wglMakeCurrent(hDrawDC, hRC);

    Active = true;
}

void GLPort::Deactivate()
{
    if (!Active) return;

    wglMakeCurrent(hDrawDC, NULL);
    ReleaseDC(MainForm->pnlView->Handle, hDrawDC);
    hDrawDC = NULL;

    Active = false;
}

// looks a lot like Dave's :-)
bool GLPort::Initialize()
{
    PIXELFORMATDESCRIPTOR pfd;
    int npf;

    // need the DC later
    HDC dc = GetDC(MainForm->pnlView->Handle);

    // this is what we want who knows what we get
    memset(&pfd, 0, sizeof pfd);
    pfd.nSize      = sizeof pfd;
    pfd.nVersion   = 1;
    pfd.dwFlags    = /*PFD_SUPPORT_GDI|*/PFD_SUPPORT_OPENGL|
                     PFD_DOUBLEBUFFER|PFD_DRAW_TO_WINDOW;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    // set the pixel format for the window
    npf = ChoosePixelFormat(dc, &pfd);
    SetPixelFormat(dc, npf, &pfd);
    DescribePixelFormat(dc, npf, sizeof pfd, &pfd);
    if (pfd.dwFlags & PFD_NEED_PALETTE)
    {
        ErrorMsg("Segeltuch does not yet support 8-bit video modes.\n\n"
                 "Please increase your video depth to atleast 16-bit.");
        //return false;
        //for now, we allow the program to continue even though the
        //display will be very ugly....
    }
    if (!(pfd.dwFlags & PFD_DOUBLEBUFFER))
    {
        ErrorMsg("No available double buffering support.");
    }
    InfoMsg("Found graphics card with %d-bits of color resolution and a %d-bit depth buffer.",
        pfd.cColorBits, pfd.cDepthBits);

    // create the OpenGL interface
    hRC = wglCreateContext(dc);
    if (hRC == NULL)
    {
        ErrorBox("Unable to create OpenGL renderring context.");
        return false;
    }

    // start her up
    Activate();
    ResetCamera();

    // no more
    MainForm->pnlView->EraseBack = false;
    // Clear the screen initially
    glClearColor(ColBck.r, ColBck.g, ColBck.b, ColBck.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SwapBuffers(dc);

    // setup the new window
    ResetSize(false);

    // Our options
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glAlphaFunc(GL_GREATER, 0.5);
    glDisable(GL_ALPHA_TEST);

    // Utility Object
    qobj = gluNewQuadric();

    // clean up our stuff
    Deactivate();

    return (Created=true);
}

void GLPort::Clear()
{
    if (!Created) return;

    Activate();
    gluDeleteQuadric(qobj);
    Deactivate();

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);

    Created = false;
}

void GLPort::StartUp()
{
    if (Created)
        ErrorMsg("Tried to call StartUp() a second time.");

    if (!Initialize())
        ErrorMsg("Unable to create OpenGL interface.");

    InfoMsg("OpenGL device intitialized successfully.");
}

void GLPort::BeginDrawing()
{
    Activate();

    glDrawBuffer(GL_BACK); //default? who knows

    // Clear the screen
    glClearColor(ColBck.r, ColBck.g, ColBck.b, ColBck.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Move the scene around a bit
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(CamTrans.x, CamTrans.y, CamTrans.z);
    glRotatef(CamRot.x, 1.0f, 0.0f, 0.0f);
    glRotatef(CamRot.y, 0.0f, 1.0f, 0.0f);
    glRotatef(CamRot.z, 0.0f, 0.0f, 1.0f);

    grid.Draw();
}

void GLPort::EndDrawing()
{
    // get some screen properties so we can interrogate later
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);

    // Ensure all drawing is finished
    glPopMatrix();
    glFlush();
    SwapBuffers(hDrawDC);

    // Release control of the window
    Deactivate();
}

void GLPort::ResetSize(bool mkcur)
{
    if (mkcur)
    {
        Activate();
    }

    w = MainForm->pnlView->ClientWidth;
    h = MainForm->pnlView->ClientHeight;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // From JED
    double dpx = (double)w/ppUnit*CamScale;
    double dpy = (double)h/ppUnit*CamScale;
    glOrtho(-dpx/2.0, dpx/2.0, -dpy/2.0, dpy/2.0,
            Proj.dMax*100.0, Proj.dMin*100.0);
    glViewport(0, 0, w, h);

    if (mkcur)
    {
        Deactivate();
    }
}

void GLPort::SimplePick(int x, int y)
{
    // Do the windows stuff
    Activate();

    GLint viewport[4];

    glGetIntegerv(GL_VIEWPORT, viewport);

    glSelectBuffer(SELBUFSZ*4, (GLuint*)SelectBuf);
    glRenderMode(GL_SELECT);

    glInitNames();
    glPushName(0);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // create 5x5 pixel picking region near cursor location
    gluPickMatrix((GLdouble) x, (GLdouble) (viewport[3] - y),
                  5.0, 5.0, viewport);
    // From JED
    double dpx = (double)w/ppUnit*CamScale;
    double dpy = (double)h/ppUnit*CamScale;
    glOrtho(-dpx/2.0, dpx/2.0, -dpy/2.0, dpy/2.0,
            Proj.dMax*100.0, Proj.dMin*100.0);

    // Move the scene around a bit
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(CamTrans.x, CamTrans.y, CamTrans.z);
    glRotatef(CamRot.x, 1.0f, 0.0f, 0.0f);
    glRotatef(CamRot.y, 0.0f, 1.0f, 0.0f);
    glRotatef(CamRot.z, 0.0f, 0.0f, 1.0f);

    //////////////////////////////////
    MainForm->DoRender();
    //////////////////////////////////

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glFlush();

    nHits = (GLuint)glRenderMode(GL_RENDER);

    if (nHits > 0)
    {
        // Find the first buffer that isn't 0xFFFFFFFF
        for (GLuint h=0; h<nHits; h++)
        {
            Sels.cur = (unsigned int)SelectBuf[h].name;
            if ((int)Sels.cur != -1)
                break;
        }
        CurSel = 0;
    }
    else
    {
        Sels.cur = (unsigned int)(-1);
        CurSel = 0;
    }

    /*/////////////////////////////////////////
    unsigned int i;

    if (nHits>0)
    {
        AnsiString s = "nHits = " + IntToStr(nHits) + ": ";
        for (i=0;i<nHits;i++)
            s += IntToStr(SelectBuf[i].name) + " ";
        //InfoMsg(s.c_str());
    }
    else
    {
        //ErrorMsg("No Hits");
    }
    //////////////////////////////////////////*/

    Deactivate();
}

//---------------------------------------------------------------------------
#pragma package(smart_init)
