//---------------------------------------------------------------------------
#ifndef GlobalH
#define GlobalH

#include <vector>

using namespace std;

//---------------------------------------------------------------------------

/************* Program stuff *************/
extern int num_errors;
extern int num_warnings;
extern AnsiString AppDir;
extern AnsiString ModelsDir;
extern AnsiString MapsDir;
extern AnsiString ProjDir;
extern AnsiString AppTitle;
extern AnsiString PrjExt;
extern AnsiString PofFilter;
extern AnsiString DxfFilter;
extern AnsiString PrjFilter;
void ErrorBox(const char *str);
void WarningBox(const char *str);

/************* Program stuff *************/
#define VM_ORTHO 1
#define VM_PERSP 2

#define VF_WIRE  1
#define VF_SOLID 2
#define VF_TEXT  4

#define VO_MESH    0x01
#define VO_FACE    0x02
#define VO_VERT    0x04
#define VO_WEAP    0x08
#define VO_THRUST  0x10
#define VO_SPCL    0x20

#define MM_SELECT   1
#define MM_ROTCAM   2
#define MM_TRNCAM   4
#define MM_DRAG     8
#define MM_TRNSEL   8

extern int ViewNode;
extern int ViewMode;
extern int ViewFill;
extern int ViewObjs;
extern int MouseMode;

extern const float PiOver180;
extern const float TwoPi;

// Texturing Options
#define DEFAULT_TEXT_TRANS_RATE 0.005
extern float TexTransRate;
extern float TexRotateRate;
extern float TexScaleRate;

// Camera View Position
enum ECameraView {
    CV_Top = 1,
    CV_Front,
    CV_Left,
    CV_Bottom,
    CV_Back,
    CV_Right,
    CV_Selection
};

struct SelVector : public vector<unsigned int>
{
    unsigned int cur;
    
    int find_sel(unsigned int v);
    bool is_sel(unsigned int v);
    inline void flush() { clear(); cur=(unsigned int)(-1); }
    int count_sel();
    unsigned int get_single();
};
extern SelVector Sels;

//---------------------------------------------------------------------------
#endif
