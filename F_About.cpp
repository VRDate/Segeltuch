//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "F_About.h"
#include "Global.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAboutForm *AboutForm;
//---------------------------------------------------------------------------
__fastcall TAboutForm::TAboutForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TAboutForm::FormCreate(TObject *Sender)
{
    lblTitle->Caption = AppTitle; 
}
//---------------------------------------------------------------------------

void __fastcall TAboutForm::Image1Click(TObject *Sender)
{
    ShellExecute(Handle, "open", "http://www.code-alliance.com",NULL, NULL, 0);
}
//---------------------------------------------------------------------------

