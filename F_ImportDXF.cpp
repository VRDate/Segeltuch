//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "F_ImportDXF.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TDXFForm *DXFForm;
//---------------------------------------------------------------------------
__fastcall TDXFForm::TDXFForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void TDXFForm::Reset()
{
    lvwObjs->Items->Clear();
}
//---------------------------------------------------------------------------

void TDXFForm::FillList(pof_file *pof)
{
    int i;
    AnsiString s;
    TListItem *item;

    // add the LODs
    for (i=0; i<pof->ohdr.num_detaillevels; i++)
    {
        item = lvwObjs->Items->Add();
        item->Caption = pof->objs[pof->ohdr.sobj_detaillevels[i]].submodel_name.s;
        s = "LOD " + IntToStr(i);
        item->SubItems->Add(s);
        item->Data = (void*)pof->ohdr.sobj_detaillevels[i];
    }

    // add the Debris
    for (i=0; i<pof->ohdr.num_debris; i++)
    {
        item = lvwObjs->Items->Add();
        item->Caption = pof->objs[pof->ohdr.sobj_debris[i]].submodel_name.s;
        item->SubItems->Add("Debris");
        item->Data = (void*)pof->ohdr.sobj_debris[i];
    }
}
//---------------------------------------------------------------------------


