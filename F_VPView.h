//---------------------------------------------------------------------------
#ifndef F_VPViewH
#define F_VPViewH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <ToolWin.hpp>

#include "VPFiles.h"
#include <ImgList.hpp>
//---------------------------------------------------------------------------
class TVPForm : public TForm
{
__published:	// IDE-managed Components
    TPanel *pnlBottom;
    TPanel *pnlClient;
    TTreeView *tvwFolders;
    TSplitter *spltMain;
    TListView *lvwFiles;
    TButton *btnOK;
    TButton *btnCancel;
    TImageList *ilMain;
    TStatusBar *stsMain;
    void __fastcall btnOKClick(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall tvwFoldersChange(TObject *Sender, TTreeNode *Node);
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall lvwFilesSelectItem(TObject *Sender, TListItem *Item,
          bool Selected);
    void __fastcall lvwFilesDblClick(TObject *Sender);
    
private:	// User declarations
public:		// User declarations
    __fastcall TVPForm(TComponent* Owner);

    AnsiString FileName;
    VPFile vp_file;
    DWORD fldr_img;

    long fstart, fend;
    AnsiString fname;

    void ListFolders();
    void AddFolder(int ient);
    void ListFiles(int fldr);
};
//---------------------------------------------------------------------------
extern PACKAGE TVPForm *VPForm;
//---------------------------------------------------------------------------
#endif
