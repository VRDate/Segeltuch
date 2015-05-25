//---------------------------------------------------------------------------

#ifndef F_ScaleMeshH
#define F_ScaleMeshH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>

#include "geometry.h"
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TScaleMeshForm : public TForm
{
__published:	// IDE-managed Components
    TListView *lvwMeshes;
    TStatusBar *StatusBar1;
    TBevel *Bevel1;
    TRadioGroup *rgpScaleSys;
    TCheckBox *chkScaleOffs;
    TEdit *edtScaleFactor;
    TLabel *Label1;
    TPanel *Panel1;
    TLabel *Label2;
    TBitBtn *BitBtn1;
    TBitBtn *BitBtn2;
private:	// User declarations
    std::vector<Mesh*> Meshes;
public:		// User declarations
    __fastcall TScaleMeshForm(TComponent* Owner);

    void Initialize(const std::vector<Mesh*> &mshs);
};
//---------------------------------------------------------------------------
extern PACKAGE TScaleMeshForm *ScaleMeshForm;
//---------------------------------------------------------------------------
#endif
