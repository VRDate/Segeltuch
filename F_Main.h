//---------------------------------------------------------------------------
#ifndef F_MainH
#define F_MainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <ComCtrls.hpp>
#include <ToolWin.hpp>
#include <Buttons.hpp>
#include <Dialogs.hpp>
#include <FileCtrl.hpp>
#include <Grids.hpp>

#include "Geometry.h"
#include <ActnList.hpp>
#include "DumbPanel.h"
#include <ImgList.hpp>

//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
    TMainMenu *mnuMain;
    TMenuItem *mnuFile;
    TOpenDialog *dlgOpen;
    TRichEdit *rchInfo;
    TSplitter *spltInfo;
    TMenuItem *mnuView;
        TMenuItem *mnuHelp;
    TMenuItem *mnuHelpAbout;
    TMenuItem *mnuFileExit;
    TMenuItem *mnuFileConvertDXF;
    TMenuItem *mnuFileN1;
    TMenuItem *mnuFileN2;
    TMenuItem *mnuFileNew;
    TMenuItem *mnuFileSavePOF;
    TMenuItem *mnuFileN3;
    TMenuItem *mnuFileN4;
    TMenuItem *mnuFileExportDXF;
    TMenuItem *mnuViewRefresh;
    TPanel *pnlClient;
    TPanel *pnlRight;
    TMenuItem *mnuTools;
    TMenuItem *mnuToolsAutoShield;
    TDumbPanel *pnlView;
    TMenuItem *mnuViewMode;
    TMenuItem *mnuViewModeWire;
    TMenuItem *mnuViewModeSolid;
    TMenuItem *mnuViewModeText;
    TSplitter *spltRight;
    TMenuItem *mnuHelpHomepage;
    TMenuItem *mnuHelpN1;
    TMenuItem *mnuFileConvertCOB;
    TMenuItem *mnuFileN5;
    TMenuItem *mnuEdit;
    TMenuItem *mnuEditSelAll;
    TMenuItem *mnuEditDeselAll;
    TMenuItem *mnuEditClearInfo;
    TMenuItem *mnuEditN1;
    TCoolBar *cobMain;
    TMenuItem *mnuViewLight;
    TMenuItem *mnuFileNewPOF;
    TPanel *pnlLeftDock;
    TPanel *pnlDisp;
    TActionList *actMain;
    TTreeView *tvwHier;
    TPanel *pnlDispOpts;
    TMenuItem *GenerateBSPHulls1;
    TImageList *imlMainLarge;
    TToolBar *tbrMain;
    TToolButton *ToolButton1;
    TStatusBar *stsMain;
    TAction *ConvPOFDXF;
    TToolButton *ToolButton2;
    TAction *OpenProj;
    TAction *SaveProj;
    TToolButton *ToolButton3;
    TToolButton *ToolButton4;
    TToolButton *ToolButton5;
    TAction *ShowHier;
    TAction *ModeMesh;
    TToolButton *ToolButton8;
    TAction *ModeFace;
    TAction *ModeVert;
    TToolButton *ToolButton9;
    TToolButton *ToolButton10;
    TToolButton *ToolButton11;
    TAction *ModeThrust;
    TToolButton *ToolButton12;
    TMenuItem *OpenProject1;
    TMenuItem *SaveProject1;
    TNotebook *nbkRight;
    TPanel *pnlMshClient;
    TLabel *lblMshNm;
    TLabel *lblMshOff;
    TLabel *lblMshProp;
    TSpeedButton *btnMshInvertNorms;
    TSpeedButton *btnMshDefText;
    TEdit *edtMshNm;
    TEdit *edtMshOff;
    TMemo *edtMshProp;
    TPanel *pnlMshTitle;
    TSpeedButton *btnMshInfo;
    TSpeedButton *btnMshDelete;
    TPanel *pnlFaceClient;
    TSpeedButton *btnFaceInvert;
    TPanel *pnlFaceTitle;
    TPanel *pnlThrustTop;
    TSpeedButton *btnNewThrust;
    TPanel *pnlThrustTitle;
    TAction *ImportDXF;
    TSpeedButton *btnFaceMap;
    TLabel *Label1;
    TComboBox *cboFaceMap;
    TSpeedButton *btnMshCenter;
    TPopupMenu *mnuHier;
    TMenuItem *mnuHierMergeChild;
    TToolButton *ToolButton13;
    TAction *ModeWeap;
    TPanel *pnlWeapClient;
    TPanel *pnlWeapTitle;
    TPanel *pnlVertClient;
    TPanel *pnlVertTitle;
        TLabel *Label2;
        TEdit *edtWeapPos;
        TLabel *Label3;
        TEdit *edtWeapNorm;
    TPrintDialog *dlgPrint;
    TAction *PrintProj;
    TToolButton *ToolButton14;
    TMenuItem *Print1;
    TCheckBox *cbxShowShields;
    TSpeedButton *btnMshScale;
    TLabel *Label4;
    TEdit *edtVertVal;
    TImageList *imlMainSmall;
    TEdit *edtWeapGroup;
    TEdit *edtWeapID;
    TLabel *Label5;
    TLabel *Label6;
    TRadioGroup *rgrpWeapType;
    TSpeedButton *btnFaceCreateWeap;
    TToolButton *ToolButton15;
    TAction *ModeSpecial;
    TPanel *pnlSpclClient;
    TPanel *pnlSpclTitle;
    TEdit *edtSpclName;
    TLabel *Label7;
    TMemo *mmoSpclProps;
    TLabel *Label8;
    TEdit *edtSpclPos;
    TLabel *Label9;
    TEdit *edtSpclRad;
    TLabel *Label10;
    TMenuItem *mnuFileImport;
    TMenuItem *ImportDXFasChildofCurrentNode1;
    TAction *Import3DS;
    TMenuItem *From3DS1;
    TSpeedButton *btnFaceReplaceMat;
    TLabel *lblThrust;
    TMemo *mmoThrustProps;
    TLabel *Label12;
    TLabel *Label13;
    TEdit *edtGlowPos;
    TLabel *Label14;
    TEdit *edtGlowNorm;
    TLabel *Label15;
    TEdit *edtGlowRad;
    TAction *ImportCOB;
    TMenuItem *FromCOB1;
    TMenuItem *mnuHierPromote;
    TMenuItem *mnuHierToggle;
    TPanel *pnlHierTitle;
    TSplitter *spltLeft;
    TMenuItem *N1;
    TMenuItem *mnuHierDelete;
    void __fastcall FormCreate(TObject *Sender);

    void __fastcall FormActivate(TObject *Sender);
    void __fastcall mnuHelpAboutClick(TObject *Sender);
    void __fastcall mnuFileExitClick(TObject *Sender);


    void __fastcall mnuFileNewClick(TObject *Sender);
    void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
    void __fastcall FormDestroy(TObject *Sender);
    void __fastcall mnuViewRefreshClick(TObject *Sender);
    void __fastcall mnuToolsAutoShieldClick(TObject *Sender);
    
    void __fastcall pnlViewResize(TObject *Sender);
    void __fastcall pnlViewPaint(TObject *Sender);
    void __fastcall mnuViewModeWireClick(TObject *Sender);
    void __fastcall mnuViewModeSolidClick(TObject *Sender);
    void __fastcall mnuViewModeTextClick(TObject *Sender);
    void __fastcall pnlViewMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    
    
    void __fastcall pnlViewKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall pnlViewKeyUp(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall pnlViewMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
    void __fastcall nbkRightPageChanged(TObject *Sender);
    void __fastcall mnuHelpHomepageClick(TObject *Sender);
    void __fastcall mnuFileConvertCOBClick(TObject *Sender);
    void __fastcall mnuFileSavePOFClick(TObject *Sender);
    void __fastcall mnuEditSelAllClick(TObject *Sender);
    void __fastcall mnuEditDeselAllClick(TObject *Sender);
    void __fastcall mnuEditClearInfoClick(TObject *Sender);
    void __fastcall mnuViewLightClick(TObject *Sender);
    void __fastcall mnuFileNewPOFClick(TObject *Sender);
    void __fastcall tvwHierChange(TObject *Sender, TTreeNode *Node);
    void __fastcall GenerateBSPHulls1Click(TObject *Sender);
    void __fastcall ConvPOFDXFExecute(TObject *Sender);
    void __fastcall OpenProjExecute(TObject *Sender);
    void __fastcall SaveProjExecute(TObject *Sender);
    void __fastcall ModeMeshExecute(TObject *Sender);
    void __fastcall ModeThrustExecute(TObject *Sender);
    void __fastcall ModeFaceExecute(TObject *Sender);
    void __fastcall ModeVertExecute(TObject *Sender);
    void __fastcall ModeMeshUpdate(TObject *Sender);
    void __fastcall ModeFaceUpdate(TObject *Sender);
    void __fastcall ModeVertUpdate(TObject *Sender);
    void __fastcall ModeThrustUpdate(TObject *Sender);
    void __fastcall btnMshInvertNormsClick(TObject *Sender);
    void __fastcall btnMshDefTextClick(TObject *Sender);
    void __fastcall edtMshNmChange(TObject *Sender);
    void __fastcall edtMshOffChange(TObject *Sender);
    void __fastcall edtMshPropChange(TObject *Sender);
    void __fastcall btnMshInfoClick(TObject *Sender);
    void __fastcall btnMshDeleteClick(TObject *Sender);
    void __fastcall btnFaceInvertClick(TObject *Sender);
    void __fastcall ImportDXFExecute(TObject *Sender);
    void __fastcall btnFaceMapClick(TObject *Sender);
    void __fastcall cboFaceMapChange(TObject *Sender);
    void __fastcall btnMshCenterClick(TObject *Sender);
    void __fastcall tvwHierDragOver(TObject *Sender, TObject *Source,
          int X, int Y, TDragState State, bool &Accept);
    void __fastcall tvwHierDragDrop(TObject *Sender, TObject *Source,
          int X, int Y);
    void __fastcall mnuHierMergeChildClick(TObject *Sender);
    void __fastcall ModeWeapExecute(TObject *Sender);
    void __fastcall PrintProjExecute(TObject *Sender);
    void __fastcall cbxShowShieldsClick(TObject *Sender);
    void __fastcall ModeWeapUpdate(TObject *Sender);
    void __fastcall btnMshScaleClick(TObject *Sender);
    void __fastcall rgrpWeapTypeClick(TObject *Sender);
    void __fastcall edtWeapGroupChange(TObject *Sender);
    void __fastcall edtWeapIDChange(TObject *Sender);
    void __fastcall edtWeapPosChange(TObject *Sender);
    void __fastcall edtWeapNormChange(TObject *Sender);
    void __fastcall pnlViewMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall btnFaceCreateWeapClick(TObject *Sender);
    void __fastcall ModeSpecialExecute(TObject *Sender);
    void __fastcall ModeSpecialUpdate(TObject *Sender);
    void __fastcall mmoSpclPropsChange(TObject *Sender);
    void __fastcall edtSpclNameChange(TObject *Sender);
    void __fastcall edtSpclPosChange(TObject *Sender);
    void __fastcall edtSpclRadChange(TObject *Sender);
    void __fastcall Import3DSExecute(TObject *Sender);
    void __fastcall btnFaceReplaceMatClick(TObject *Sender);
    void __fastcall edtVertValChange(TObject *Sender);
    void __fastcall btnNewThrustClick(TObject *Sender);
    void __fastcall mmoThrustPropsChange(TObject *Sender);
    void __fastcall edtGlowPosChange(TObject *Sender);
    void __fastcall edtGlowNormChange(TObject *Sender);
    void __fastcall edtGlowRadChange(TObject *Sender);
    void __fastcall ImportCOBExecute(TObject *Sender);
    void __fastcall mnuHierPromoteClick(TObject *Sender);
    void __fastcall mnuHierToggleClick(TObject *Sender);
    void __fastcall mnuHierDeleteClick(TObject *Sender);
private:	// User declarations
    void __fastcall AppMessage(TMsg &AMessage, bool &Handled);

public:		// User declarations
    __fastcall TMainForm(TComponent* Owner);
    void ClearView();
    void CleanupModel();
    bool DoOpen(AnsiString FileName);
    void SetTitle();
    void DoCommandLine();
    void DoRender();
    void PopulateTree();
    void UpdateTree();
    void AddMeshToTree(TTreeNode *pPar, int msh);

    bool GetMousePos(int &x, int &y);
    void SetMM(int mm);

    Vector CurPos;

    TPoint OpOrgXY;
    Vector OpOrg;
    TPoint LastSelXY;
    void Do_StartRotateCam(int x, int y);
    void Do_RotateCam(int x, int y);
    void Do_StartTranslateCam(int x, int y);
    void Do_TranslateCam(int x, int y);
    void Do_ToggleSel();
    void Do_Selection(int x, int y);
    void Do_AlignGridToSelection();
    void Do_RepositionCamera(ECameraView NewView, bool AlignGrid);
    void Do_StartTranslateSelection();
    void Do_TranslateSelection();
    void Do_InsertItem();
    void Do_DeleteSelection();

    void On_SelChanged();

    void TextTranslate(float du, float dv);
    void TextRotate(float ang);
    void TextScale(float val);

    bool Updating;
    void FillMeshInfo(int cnt);
    void FillFaceInfo(int cnt);
    void FillVertInfo(int cnt);
    void FillThrustInfo(int cnt);
    void FillWeaponInfo(int cnt);
    void FillSpecialInfo(int cnt);

    unsigned int BspDispList;

};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;

void ErrorMsg(char *fmt, ...);
void InfoMsg(char *fmt, ...);
//---------------------------------------------------------------------------
#endif
