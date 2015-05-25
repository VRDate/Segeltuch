//---------------------------------------------------------------------------
#ifndef F_SelectH
#define F_SelectH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TSelectForm : public TForm
{
__published:	// IDE-managed Components
    TLabel *lblText;
    TButton *btnOK;
    TButton *btnCancel;
    TListBox *lstList;
private:	// User declarations
public:		// User declarations
    __fastcall TSelectForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TSelectForm *SelectForm;
//---------------------------------------------------------------------------
#endif
