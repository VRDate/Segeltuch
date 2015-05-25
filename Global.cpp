//---------------------------------------------------------------------------
#include <vcl.h>
#include <vector>
#include <cmath>
#pragma hdrstop

#include "Global.h"

using namespace std;

//---------------------------------------------------------------------------

/************* Program stuff *************/
int num_errors=0;
int num_warnings=0;
AnsiString AppDir;
AnsiString ModelsDir;
AnsiString MapsDir;
AnsiString ProjDir;
AnsiString AppTitle = "Segeltuch v0.3";
AnsiString PrjExt = ".fprj";
AnsiString PofFilter = "FreeSpace POF Files (*.pof, *.vp)|*.pof;*.vp|All Files (*.*)|*.*";
AnsiString DxfFilter = "AutoCAD R12 DXF Files (*.dxf)|*.dxf|All Files (*.*)|*.*";
AnsiString PrjFilter = "Segeltuch Project Files (*.fprj)|*.fprj|All Files (*.*)|*.*";

void ErrorBox(const char *str)
{
    MessageBox(NULL, str, "Error", MB_OK|MB_ICONERROR);
    num_errors++;
}
void WarningBox(const char *str)
{
    MessageBox(NULL, str, "Warning", MB_OK|MB_ICONINFORMATION);
    num_warnings++;
}

/************* Status stuff *************/
int ViewNode;
int ViewMode;
int ViewFill;
int ViewObjs;
int MouseMode;
SelVector Sels;

// Texturing Options
float TexTransRate = DEFAULT_TEXT_TRANS_RATE;
float TexRotateRate = 1.5f;
float TexScaleRate = 1.01f;

const float PiOver180 = 4.0*std::atan(1.0)/180.0;
const float TwoPi     = 8.0*std::atan(1.0);

// Slection Stuff
int SelVector::find_sel(unsigned int v)
{
    unsigned int i;

    for (i=0; i<size(); i++)
        if (at(i) == v) return (int)i;

    return -1;
}

bool SelVector::is_sel(unsigned int v)
{
    unsigned int i;

    for (i=0; i<size(); i++)
        if (at(i) == v) return true;
    if (cur == v) return true;

    return false;
}

int SelVector::count_sel()
{
    int n = (int)size(); // permanent
    if (cur != (unsigned int)(-1)) // current
    {
        n++;
        // do not count it if it is on the stack
        if (find_sel(cur) >= 0) n--;
    }

    return n;
}

unsigned int SelVector::get_single()
{
    if (cur != (unsigned int)(-1))
        return cur;

    if (size() > 0) return back();

    return (unsigned int)(-1);
}

//---------------------------------------------------------------------------


#pragma package(smart_init)
