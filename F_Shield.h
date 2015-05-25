//---------------------------------------------------------------------------

#ifndef F_ShieldH
#define F_ShieldH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>

#include "geometry.h"
#include <ComCtrls.hpp>
#include <Dialogs.hpp>

//---------------------------------------------------------------------------
class TShieldForm : public TForm
{
__published:	// IDE-managed Components
    TRadioButton *rdoImport;
    TRadioButton *rdoGen;
    TPanel *pnlGenerate;
    TPanel *pnlImport;
    TButton *Button1;
    TPanel *pnlTop;
    TPanel *pnlSide;
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Label3;
    TLabel *Label4;
    TEdit *edtSize;
    TButton *btnOK;
    TButton *btnCancel;
    TPaintBox *boxTop;
    TPaintBox *boxSide;
    TLabel *Label5;
    TEdit *edtCenter;
    TMemo *mmoDim;
    TTrackBar *tbrZoom;
    TLabel *Label6;
    TLabel *Label7;
    TEdit *edtNumDiv;
    TUpDown *UpDown1;
    TLabel *lblDXFName;
    TOpenDialog *dlgOpen;
    void __fastcall edtSizeChange(TObject *Sender);
    void __fastcall boxTopPaint(TObject *Sender);
    void __fastcall boxSidePaint(TObject *Sender);
    void __fastcall edtCenterChange(TObject *Sender);
    void __fastcall tbrZoomChange(TObject *Sender);
    void __fastcall Button1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TShieldForm(TComponent* Owner);

    Vector Min, Max;
    Vector Size, Center;

    void Init(Vector min, Vector max);
};
//---------------------------------------------------------------------------
extern PACKAGE TShieldForm *ShieldForm;
//---------------------------------------------------------------------------
#endif
