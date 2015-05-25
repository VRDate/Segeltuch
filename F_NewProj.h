//---------------------------------------------------------------------------
#ifndef F_NewProjH
#define F_NewProjH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TNewProjForm : public TForm
{
__published:	// IDE-managed Components
    TLabel *Label1;
    TEdit *edtName;
    TButton *Button1;
    TButton *Button2;
private:	// User declarations
public:		// User declarations
    __fastcall TNewProjForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TNewProjForm *NewProjForm;
//---------------------------------------------------------------------------
#endif
