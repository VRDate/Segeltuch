//---------------------------------------------------------------------------

#ifndef F_CreateWeapH
#define F_CreateWeapH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "DumbPanel.h"
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <ToolWin.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TCreateWeapForm : public TForm
{
__published:	// IDE-managed Components
    TStatusBar *stsMain;
    TPanel *Panel1;
    TSplitter *Splitter1;
    TPanel *Panel2;
    TToolBar *tbrMain;
    TPanel *Panel3;
    TBitBtn *btnOK;
    TBitBtn *btnCancel;
    TEdit *edtCount;
    TLabel *Label1;
    TRadioGroup *rgrpType;
    TRadioGroup *rgrpDir;
    TEdit *edtDir;
    TRadioGroup *rgrpPattern;
    TPanel *Panel4;
    TPaintBox *View;
    TLabel *Label2;
    TEdit *edtGroup;
    void __fastcall btnOKClick(TObject *Sender);
    void __fastcall edtCountChange(TObject *Sender);
    void __fastcall ViewPaint(TObject *Sender);
    void __fastcall Panel4Resize(TObject *Sender);
private:	// User declarations
    size_t              iMesh, iFace;
    std::vector<Vector> Points;
    std::vector<Vector> FacePoints;
    TPoint              ViewCenter;
    float               ViewScale;
    Vector              ViewOffset;
    Transformation      FaceTrans;
    Vector              FaceMax;        //2D
    Vector              FaceMin;        //2D
    Vector              FaceCenter;     //2D

    void ProjectFace();
    void ProjectPoint(const Vector &Pt, int &X, int &Y);
    void CreateWeaponPoints(size_t Count);

public:		// User declarations
    __fastcall TCreateWeapForm(TComponent* Owner);

    void Initialize(size_t mesh, size_t face);
};
//---------------------------------------------------------------------------
extern PACKAGE TCreateWeapForm *CreateWeapForm;
//---------------------------------------------------------------------------
#endif
