//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "F_MeshInfo.h"

#include "global.h"
#include "Geometry.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMeshInfoForm *MeshInfoForm;
//---------------------------------------------------------------------------
__fastcall TMeshInfoForm::TMeshInfoForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void TMeshInfoForm::AddMesh(TTreeNode *ParItem, unsigned int n)
{
    Mesh *msh = &Proj.meshes[n];

    TTreeNode *NewItem;
    TTreeNode *Child;

    NewItem = tvwList->Items->AddChild(ParItem, IntToStr((int)n) + AnsiString(": ") + msh->name);
    NewItem->ImageIndex = 0;

    Child = tvwList->Items->AddChild(NewItem, AnsiString("nVerts = ") +
        IntToStr((int)msh->verts.size()));
    Child->ImageIndex = 1;
    Child->SelectedIndex = 1;
    Child = tvwList->Items->AddChild(NewItem,  AnsiString("nFaces = ") +
        IntToStr((int)msh->faces.size()));
    Child->ImageIndex = 1;
    Child->SelectedIndex = 1;
    Child = tvwList->Items->AddChild(NewItem,  AnsiString("Mass = ") +
        FormatFloat(",#########0.00", msh->get_mass()));
    Child->ImageIndex = 1;
    Child->SelectedIndex = 1;
}

void TMeshInfoForm::FillList()
{
    unsigned int i;

    tvwList->Items->Clear();
    tvwList->Items->BeginUpdate();

    for (i=0; i<Proj.meshes.size(); i++)
    {
        if (!Sels.is_sel(i)) continue;

        AddMesh(NULL, i);
    }
    tvwList->FullExpand();
    tvwList->Items->EndUpdate();
}
//---------------------------------------------------------------------------

void __fastcall TMeshInfoForm::FormCreate(TObject *Sender)
{
    TLoadResources res;
    res << lrMap3DColors;
    if (!ilstIcons->GetResource(rtBitmap, "BMP_MSHINFO", 16, res, clTeal))
    {
        MessageBox(NULL,"Unable to load images.","Eror",MB_OK);
    }
}
//---------------------------------------------------------------------------


void __fastcall TMeshInfoForm::Button1Click(TObject *Sender)
{
    AnsiString fn = ChangeFileExt(Proj.filename, "_MeshInfo.txt");
    tvwList->SaveToFile(fn);
}
//---------------------------------------------------------------------------

