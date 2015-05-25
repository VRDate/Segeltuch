//---------------------------------------------------------------------------
#include <vcl.h>
#include <shellapi.h>
#pragma hdrstop

#include "Global.h"
#include "F_VPView.h"
#include "F_Main.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TVPForm *VPForm;
//---------------------------------------------------------------------------
__fastcall TVPForm::TVPForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TVPForm::btnOKClick(TObject *Sender)
{
    //
}
//---------------------------------------------------------------------------

void __fastcall TVPForm::FormShow(TObject *Sender)
{
    Caption = FileName;
    vp_file.entries.clear();

    if (!vp_file.read(FileName.c_str()))
    {
        ErrorMsg("Could not process %s", FileName.c_str());
        ModalResult = mrCancel;
        return;
    }

    stsMain->Panels->Items[0]->Text = IntToStr(vp_file.entries.size()) + " Entries Found.";

    tvwFolders->Items->Clear();
    ListFolders();
}
//---------------------------------------------------------------------------

void TVPForm::ListFolders()
{
    unsigned int i;

    for (i=0; i<vp_file.entries.size(); i++)
    {
        AddFolder((int)i);
    }
    tvwFolders->FullExpand();
}
//---------------------------------------------------------------------------

void TVPForm::AddFolder(int ient)
{
    // Figureout where on the tree to put this
    VPEntry *ent = &vp_file.entries[ient];
    TTreeNode *par;

    if (ent->par >= 0)
        par = vp_file.entries[ent->par].node;
    else
        par = NULL;

    // forget about all the ".."'s
    if ((ent->size == 0) &&
        (ent->filename[0] == '.') &&
        (ent->filename[1] == '.'))
        return;
    // forget about all files
    if (ent->size > 0) return;

    // select image
    //int i = (ent->length == 0)?0:2;
    //int si = (ent->length == 0)?0:2;//1:2;

    ent->node = tvwFolders->Items->AddChild(par, ent->filename);
    ent->node->Data = (void*)ient;
    ent->node->ImageIndex = fldr_img;
    ent->node->SelectedIndex = fldr_img;
}

void __fastcall TVPForm::tvwFoldersChange(TObject *Sender, TTreeNode *Node)
{
    int i = (int)Node->Data;

    ListFiles(i);
}
//---------------------------------------------------------------------------

void TVPForm::ListFiles(int fldr)
{
    AnsiString s;
    unsigned int i;
    TListItem *item;
    SHFILEINFO IconInfo;
//    int t;

    lvwFiles->Items->BeginUpdate();
    lvwFiles->Items->Clear();

    for (i=0; i<vp_file.entries.size(); i++)
    {
        if (vp_file.entries[i].par != fldr) continue;
        if (vp_file.entries[i].size <= 0) continue;

        item = lvwFiles->Items->Add();
        item->Caption = vp_file.entries[i].filename;
        item->Data = (void*)i;

        s = FormatFloat(",######", (double)vp_file.entries[i].size/1024.0);
        s += "KB";
        item->SubItems->Add(s);

        // Get Image
        //s = "*" + ExtractFileExt(vp_file.entries[i].filename);
        //SHGetFileInfo(s.c_str(), 0, &IconInfo, sizeof(IconInfo),
        //    SHGFI_ICON | SHGFI_SHELLICONSIZE | SHGFI_SYSICONINDEX);
        item->ImageIndex = IconInfo.iIcon;

        /*t = vp_file.entries[i].timestamp;// >> 16;
        s = IntToStr(t & 0x1F + 1) + "/" +      //day
            IntToStr((t>>5) & 0xF + 1) + "/" +  //mon
            IntToStr((t>>9) & 0x7F + 1980);       //yr
        item->SubItems->Add(s);*/
    }

    lvwFiles->Items->EndUpdate();
}
//---------------------------------------------------------------------------

void __fastcall TVPForm::FormCreate(TObject *Sender)
{
    SHFILEINFO IconInfo;

    // Bind ilMain to the system image list.
    ilMain->ShareImages = true;
    ilMain->Handle = SHGetFileInfo("", 0, &IconInfo, sizeof(IconInfo),
        SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX);
    tvwFolders->Images = ilMain;
    //lvwFiles->SmallImages = ilMain;

    // Get Folder Icon
    char winpath[MAX_PATH];
    GetWindowsDirectory(winpath, MAX_PATH);
    SHGetFileInfo(winpath, 0, &IconInfo, sizeof(IconInfo),
        SHGFI_ICON | SHGFI_SHELLICONSIZE | SHGFI_SYSICONINDEX);
    fldr_img = IconInfo.iIcon;

    // Make this window a sizeable dialog frame
    LONG Style;// = GetWindowLong(Handle, GWL_STYLE);
    Style = WS_POPUPWINDOW|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|
            WS_DLGFRAME|WS_THICKFRAME|WS_OVERLAPPED|DS_3DLOOK|DS_MODALFRAME;
    SetWindowLong(Handle, GWL_STYLE, Style);
    Style = GetWindowLong(Handle, GWL_EXSTYLE);
    Style |= WS_EX_DLGMODALFRAME|WS_EX_WINDOWEDGE;
    SetWindowLong(Handle, GWL_EXSTYLE, Style);
}
//---------------------------------------------------------------------------


void __fastcall TVPForm::lvwFilesSelectItem(TObject *Sender,
      TListItem *Item, bool Selected)
{
    fstart=fend=0;
    stsMain->Panels->Items[1]->Text = "";

    if (!lvwFiles->Selected) return;

    int i = (int)lvwFiles->Selected->Data;
    if ((i<0) || (i>=(int)vp_file.entries.size())) return;

    fstart = vp_file.entries[i].offset;
    fend = vp_file.entries[i].offset + vp_file.entries[i].size;
    fname = vp_file.entries[i].filename;

    stsMain->Panels->Items[1]->Text = IntToStr(fstart) + " to " + IntToStr(fend);
}
//---------------------------------------------------------------------------

void __fastcall TVPForm::lvwFilesDblClick(TObject *Sender)
{
    btnOKClick(0);
    ModalResult = mrOk;    
}
//---------------------------------------------------------------------------

