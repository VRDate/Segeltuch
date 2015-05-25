//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Global.h"
#include "F_Shield.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TShieldForm *ShieldForm;
//---------------------------------------------------------------------------
__fastcall TShieldForm::TShieldForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TShieldForm::edtSizeChange(TObject *Sender)
{
    Size = StrToVec(edtSize->Text);

    boxTop->Invalidate();
    boxSide->Invalidate();
}
//---------------------------------------------------------------------------
void TShieldForm::Init(Vector min, Vector max)
{
    Min = min;
    Max = max;

    Center = (Min+Max) * 0.5f;
    Size   = (Max-Center) * 1.2f;

    mmoDim->Lines->Clear();
    mmoDim->Lines->Add(VecToStr(Min));
    mmoDim->Lines->Add(VecToStr(Max));
    edtSize->Text   = VecToStr(Size);
    edtCenter->Text = VecToStr(Center);

    tbrZoomChange(0);
}
//---------------------------------------------------------------------------
static float Padding = 1.0;
void __fastcall TShieldForm::boxTopPaint(TObject *Sender)
{
    TPoint cen;
    cen.x = boxTop->Width / 2;
    cen.y = boxTop->Height / 2;

    Vector bigger = (Max - Min) * Padding;
    float  ppu    = float(boxTop->Height) / (bigger.z);

    TRect rct;

    boxTop->Canvas->Brush->Style = bsClear;

    rct.left   = cen.x + (int)((-Size.x + Center.x)*ppu);
    rct.right  = cen.x + (int)(( Size.x + Center.x)*ppu);
    rct.top    = cen.y - (int)((-Size.z + Center.z)*ppu);
    rct.bottom = cen.y - (int)(( Size.z + Center.z)*ppu);
    boxTop->Canvas->Pen->Color = clYellow;
    boxTop->Canvas->Ellipse(rct);

    rct.left   = cen.x + (int)((Min.x)*ppu);
    rct.right  = cen.x + (int)((Max.x)*ppu);
    rct.top    = cen.y - (int)((Min.z)*ppu);
    rct.bottom = cen.y - (int)((Max.z)*ppu);
    boxTop->Canvas->Pen->Color = clWhite;
    boxTop->Canvas->Rectangle(rct);
}
void __fastcall TShieldForm::boxSidePaint(TObject *Sender)
{
    TPoint cen;
    cen.x = boxSide->Width / 2;
    cen.y = boxSide->Height / 2;

    Vector bigger = (Max - Min) * Padding;
    float  ppu    = float(boxSide->Height) / (bigger.y);

    TRect rct;

    boxSide->Canvas->Brush->Style = bsClear;

    rct.left   = cen.x + (int)((-Size.z + Center.z)*ppu);
    rct.right  = cen.x + (int)(( Size.z + Center.z)*ppu);
    rct.top    = cen.y - (int)((-Size.y + Center.y)*ppu);
    rct.bottom = cen.y - (int)(( Size.y + Center.y)*ppu);
    boxSide->Canvas->Pen->Color = clYellow;
    boxSide->Canvas->Ellipse(rct);

    rct.left   = cen.x + (int)((Min.z)*ppu);
    rct.right  = cen.x + (int)((Max.z)*ppu);
    rct.top    = cen.y - (int)((Min.y)*ppu);
    rct.bottom = cen.y - (int)((Max.y)*ppu);
    boxSide->Canvas->Pen->Color = clWhite;
    boxSide->Canvas->Rectangle(rct);
}
//---------------------------------------------------------------------------

void __fastcall TShieldForm::edtCenterChange(TObject *Sender)
{
    Center = StrToVec(edtCenter->Text);

    boxTop->Invalidate();
    boxSide->Invalidate();
}
//---------------------------------------------------------------------------

void __fastcall TShieldForm::tbrZoomChange(TObject *Sender)
{
    Padding = 5.0f * float(tbrZoom->Max-tbrZoom->Position)/float(tbrZoom->Max);
    if (Padding < 0.00001f) Padding = 0.00001f;

    boxTop->Invalidate();
    boxSide->Invalidate();
}
//---------------------------------------------------------------------------


void __fastcall TShieldForm::Button1Click(TObject *Sender)
{
    dlgOpen->InitialDir = ProjDir;
    dlgOpen->FileName = "";
    if (!dlgOpen->Execute()) return;

    rdoImport->Checked = true;
    lblDXFName->Caption = dlgOpen->FileName;
}
//---------------------------------------------------------------------------

