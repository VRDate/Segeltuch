//---------------------------------------------------------------------------
#ifndef F_MeshInfoH
#define F_MeshInfoH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TMeshInfoForm : public TForm
{
__published:	// IDE-managed Components
    TButton *btnOK;
    TImageList *ilstIcons;
    TTreeView *tvwList;
    TButton *Button1;
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall Button1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TMeshInfoForm(TComponent* Owner);

    void FillList();
    void AddMesh(TTreeNode *ParItem, unsigned int n);
};
//---------------------------------------------------------------------------
extern PACKAGE TMeshInfoForm *MeshInfoForm;
//---------------------------------------------------------------------------
#endif
