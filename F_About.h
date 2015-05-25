//---------------------------------------------------------------------------
#ifndef F_AboutH
#define F_AboutH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
//---------------------------------------------------------------------------
class TAboutForm : public TForm
{
__published:	// IDE-managed Components
    TImage *Image1;
    TLabel *lblTitle;
    TLabel *Label2;
    TMemo *Memo1;
    TLabel *Label3;
    TButton *btnOK;
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall Image1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TAboutForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TAboutForm *AboutForm;
//---------------------------------------------------------------------------
#endif
