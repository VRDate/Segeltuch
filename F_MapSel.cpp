//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "global.h"
#include "geometry.h"
#include "F_MapSel.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMapSelForm *MapSelForm;
//---------------------------------------------------------------------------
__fastcall TMapSelForm::TMapSelForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TMapSelForm::FormShow(TObject *Sender)
{
    flbFiles->ApplyFilePath(MapsDir);

    lvwFiles->Items->BeginUpdate();
    lvwFiles->Items->Clear();
    lvwFiles->Selected = 0;
    TListItem *li;
    for (int i=0; i < flbFiles->Items->Count; i++)
    {
        li = lvwFiles->Items->Add();
        li->Caption = flbFiles->Items->Strings[i];
        if (flbFiles->Items->Strings[i].AnsiCompareIC(OldFileName) == 0)
        {
            lvwFiles->Selected = li;
        }
    }
    lvwFiles->Items->EndUpdate();
}
//---------------------------------------------------------------------------

void __fastcall TMapSelForm::btnOKClick(TObject *Sender)
{
//
}
//---------------------------------------------------------------------------

void __fastcall TMapSelForm::lvwFilesChange(TObject *Sender,
      TListItem *Item, TItemChange Change)
{
//
}
//---------------------------------------------------------------------------

