//---------------------------------------------------------------------------
#ifndef F_ImportDXFH
#define F_ImportDXFH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>

#include "pof.h"
//---------------------------------------------------------------------------
class TDXFForm : public TForm
{
__published:	// IDE-managed Components
    TButton *btnOK;
    TListView *lvwObjs;
    TLabel *Label1;
    
private:	// User declarations
public:		// User declarations
    __fastcall TDXFForm(TComponent* Owner);

    void Reset();
    void FillList(pof_file *pof);
};
//---------------------------------------------------------------------------
extern PACKAGE TDXFForm *DXFForm;
//---------------------------------------------------------------------------
#endif
