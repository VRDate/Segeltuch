//---------------------------------------------------------------------------

#ifndef F_MapSelH
#define F_MapSelH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <FileCtrl.hpp>
//---------------------------------------------------------------------------
class TMapSelForm : public TForm
{
__published:	// IDE-managed Components
    TStatusBar *StatusBar1;
    TListView *lvwFiles;
    TPanel *Panel1;
    TBitBtn *btnOK;
    TBitBtn *btnCancel;
    TFileListBox *flbFiles;
    void __fastcall FormShow(TObject *Sender);
    void __fastcall btnOKClick(TObject *Sender);
    void __fastcall lvwFilesChange(TObject *Sender, TListItem *Item,
          TItemChange Change);
private:	// User declarations
public:		// User declarations
    AnsiString OldFileName;
    __fastcall TMapSelForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMapSelForm *MapSelForm;
//---------------------------------------------------------------------------
#endif
