//---------------------------------------------------------------------------
#ifndef F_VisibleH
#define F_VisibleH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TVisForm : public TForm
{
__published:	// IDE-managed Components
private:	// User declarations
public:		// User declarations
    __fastcall TVisForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TVisForm *VisForm;
//---------------------------------------------------------------------------
#endif
