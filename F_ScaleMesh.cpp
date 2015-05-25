//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "F_ScaleMesh.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TScaleMeshForm *ScaleMeshForm;
//---------------------------------------------------------------------------
__fastcall TScaleMeshForm::TScaleMeshForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void TScaleMeshForm::Initialize(const std::vector<Mesh*> &mshs)
{
    Meshes = mshs;

    lvwMeshes->Items->Clear();
    for (size_t i=0; i < Meshes.size(); i++)
    {
        Meshes[i]->get_minmax();
        Vector size = Meshes[i]->max - Meshes[i]->min;
        TListItem *ListItem = lvwMeshes->Items->Add();
        ListItem->Caption = Meshes[i]->name;
        ListItem->SubItems->Add(AnsiString((double)size.x));
        ListItem->SubItems->Add(AnsiString((double)size.y));
        ListItem->SubItems->Add(AnsiString((double)size.z));
    }

    edtScaleFactor->Text = VecToStr(Vector(1.0f,1.0f,1.0f));
    rgpScaleSys->ItemIndex = 0;
    chkScaleOffs->Checked = true;
}
