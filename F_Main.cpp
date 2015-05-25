//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#include <Printers.hpp>
#include <ShellAPI.h>
#include <fstream>
#pragma hdrstop

#include "Global.h"
#include "F_Main.h"
#include "Pof.h"
#include "F_About.h"
#include "RenderGL.h"
#include "Geometry.h"
#include "F_NewProj.h"
#include "Draw.h"
#include "F_VPView.h"
#include "ImageFiles.h"
#include "F_MeshInfo.h"
#include "F_Shield.h"
#include "F_ScaleMesh.h"
#include "F_MapSel.h"
#include "F_CreateWeap.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "DumbPanel"
#pragma resource "*.dfm"

TMainForm     *MainForm;
std::ofstream  LogFile;

//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
    : TForm(Owner), BspDispList(0)
{
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::mnuFileConvertCOBClick(TObject *Sender)
{
    FILE *f;
    long start, end;
    pof_file pof;
    AnsiString filename;

    dlgOpen->InitialDir = ModelsDir;
    dlgOpen->Filter = PofFilter;
    if (!dlgOpen->Execute()) return;

    // Find out what kind of file this is and take the appropriate action
    AnsiString ext = ExtractFileExt(dlgOpen->FileName).LowerCase();
    if (ext == ".pof")
    {
        f = fopen(dlgOpen->FileName.c_str(), "rb");
        if (f == NULL)
        {
            ErrorMsg("Could not open \"%s\".", dlgOpen->FileName.c_str());
            return;
        }

        //determine the length of the file
        start = 0;
        fseek(f, 0, SEEK_END);
        end = ftell(f);
        fseek(f, 0, SEEK_SET);

        filename = ProjDir + ExtractFileName(dlgOpen->FileName);
    }
    else if (ext == ".vp")
    {
        VPForm->FileName = dlgOpen->FileName;
        if (VPForm->ShowModal() != mrOk) return;

        f = fopen(dlgOpen->FileName.c_str(), "rb");
        if (f == NULL)
        {
            ErrorMsg("Could not open \"%s\".", dlgOpen->FileName.c_str());
            return;
        }

        start = VPForm->fstart;
        end = VPForm->fend;

        filename = ProjDir + VPForm->fname;
    }
    else
    {
        ErrorMsg("Unknown file type.");
        return;
    }

    // read the file
    if (!pof.read(f, start, end)) return;

    // Write the new dxf file
    filename = ChangeFileExt(filename, ".cob");
    convert_cob(&pof, filename.c_str());
    fclose(f);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormCreate(TObject *Sender)
{
    Application->OnMessage = AppMessage;

    Caption = AppTitle;

    AppDir = ExtractFilePath(Application->ExeName);
    ModelsDir = AppDir + "models\\";
    MapsDir = AppDir + "maps\\";
    ProjDir = AppDir + "projects\\";

    if (!DirectoryExists(ModelsDir))
    {
        InfoMsg("Models directory missing.  Segeltuch will create \"%s\"", ModelsDir.c_str());
        CreateDir(ModelsDir);
    }
    if (!DirectoryExists(MapsDir))
    {
        InfoMsg("Maps directory missing.  Segeltuch will create \"%s\"", MapsDir.c_str());
        CreateDir(MapsDir);
    }
    if (!DirectoryExists(ProjDir))
    {
        InfoMsg("Project directory missing.  Segeltuch will create \"%s\"", ProjDir.c_str());
        CreateDir(ProjDir);
    }

    // Logging stuff
    LogFile.open((AppDir + "log.txt").c_str());

    // Storage stuff
    CleanupModel();
    ViewNode = -1;
    Updating = false;

    // OpenGL stuff
    ViewMode = VM_ORTHO;
    ViewFill = VF_SOLID;
    ViewObjs = VO_MESH;
    SetMM(MM_SELECT);
    nbkRight->ActivePage = "Mesh";
    nbkRightPageChanged(this);
    MainGL.UseLight = true;
    MainGL.Clear();

    DoCommandLine();
}

void TMainForm::ClearView()
{
    MainGL.Clear();
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::FormActivate(TObject *Sender)
{
    pnlView->SetFocus();
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::mnuHelpAboutClick(TObject *Sender)
{
    AboutForm->ShowModal();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mnuFileExitClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------

void ErrorMsg(char *fmt, ...)
{
    MainForm->rchInfo->SelStart=0;
    MainForm->rchInfo->SelLength=0;
    MainForm->rchInfo->SelAttributes->Color = clRed;

    static char buffer[256];
    buffer[0] = '!';
    buffer[1] = ' ';
    va_list argptr;
	va_start(argptr, fmt);
	vsprintf(&buffer[2], fmt, argptr);
	va_end(argptr);

	MainForm->rchInfo->Lines->Insert(0, buffer);
    LogFile << buffer << endl;
}

void InfoMsg(char *fmt, ...)
{
    MainForm->rchInfo->SelStart=0;
    MainForm->rchInfo->SelLength=0;
    MainForm->rchInfo->SelAttributes->Color = clNavy;

    static char buffer[256];
    buffer[0] = '>';
    buffer[1] = ' ';
    va_list argptr;
	va_start(argptr, fmt);
	vsprintf(&buffer[0], fmt, argptr);
	va_end(argptr);

	MainForm->rchInfo->Lines->Insert(0, buffer);
    LogFile << buffer << endl;
}

void TMainForm::CleanupModel()
{
    Sels.flush();
    
    if (Proj.mod)
    {
        AnsiString s;
        s = Proj.name + " has been modified.  Would you like to save it?";
        if (Application->MessageBox(s.c_str(), AppTitle.c_str(), MB_YESNO|MB_ICONQUESTION)
            == IDYES)
        {
            Proj.Write();
        }
    }

    // Free any texture resources
    MainGL.Activate();
    for (unsigned int i=0; i<Proj.mats.size(); i++)
    {
        if (Proj.mats[i].image)
            delete ((OGLTexture*)Proj.mats[i].image);
    }
    MainGL.Deactivate();

    // Free the geometry
    Proj.Clear();
}

//---------------------------------------------------------------------------
void TMainForm::SetTitle()
{
    Caption = Proj.name + " - " + AppTitle;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mnuFileNewClick(TObject *Sender)
{
    CleanupModel();
    MainGL.Clear();

    if (NewProjForm->ShowModal() != IDOK) return;

    Proj.name = NewProjForm->edtName->Text;
    Proj.filename = ProjDir + Proj.name + PrjExt;
    if (FileExists(Proj.filename))
    {
        ErrorMsg("%s already exists as a project.", Proj.name.c_str());
        CleanupModel();
        return;
    }
    SetTitle();

    // Add the grande default image
    ImgString ist; ist.s = "default"; ist.image = 0;
    Proj.mats.push_back(ist);

    Proj.Write();
    MainGL.StartUp();
    Proj.recalc_all();
    MainGL.ResetSize(true);

    PopulateTree();
}

bool TMainForm::DoOpen(AnsiString FileName)
{
    CleanupModel();
    MainGL.Clear();

    Proj.name = ExtractFileName(FileName);
    Proj.name.SetLength(Proj.name.AnsiPos(PrjExt)-1);
    Proj.filename = FileName;
    SetTitle();

    if (!Proj.Read())
    {
        ErrorMsg("Unable to load project file.");
        return false;
    }

    MainGL.StartUp();
    Proj.recalc_all();
    MainGL.ResetSize(true);

    PopulateTree();

    return true;
}


void __fastcall TMainForm::FormCloseQuery(TObject *Sender, bool &CanClose)
{
    CleanupModel();
    CanClose = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormDestroy(TObject *Sender)
{
    MainGL.Clear();
    LogFile.close();
}
//---------------------------------------------------------------------------
void TMainForm::DoCommandLine()
{
    if (ParamCount() > 0)
    {
        AnsiString fname;

        //char str[1025];
        //GetLongPathName(ParamStr(1).c_str(), str, 1024);
        //fname = str;

        fname = ParamStr(1);
        fname.Unique();

        //::MessageBox(0,fname.c_str(),"",MB_OK);

        if (ExtractFileExt(fname) == PrjExt)
        {
            DoOpen(fname);
        }
        else
        {
            ErrorMsg("Only Segeltuch projects may be opened through the shell."); 
        }
    }
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void TMainForm::DoRender()
{
    switch (ViewFill)
    {
    case VF_WIRE: DrawMeshesWire(); break;
    case VF_SOLID: DrawMeshesSolid(); break;
    case VF_TEXT: DrawMeshesTextured(); break;
    }

    if (ViewObjs == VO_WEAP)
    {
        DrawWeapons();
    }
    else if (ViewObjs == VO_SPCL)
    {
        DrawSpecials();
    }
    else if (ViewObjs == VO_THRUST)
    {
        DrawThrusters();
    }

    if (cbxShowShields->Checked == true)
    {
        DrawShields();
    }

    //DrawCursor(CurPos);
//    if (BspDispList != 0)
//    {
//        glCallList(BspDispList);
//    }
}
//---------------------------------------------------------------------------


void TMainForm::SetMM(int mm)
{
    MouseMode = mm;
}
//---------------------------------------------------------------------------

bool TMainForm::GetMousePos(int &x, int &y)
{
    TPoint pt;

    GetCursorPos(&pt);
    pt = pnlView->ScreenToClient(pt);
    x = pt.x; y = pt.y;

    if (!PtInRect((tagRECT*)&ClientRect, pt)) return false;
    return true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mnuViewRefreshClick(TObject *Sender)
{
    pnlView->SetFocus();
    pnlView->Invalidate();
}
//---------------------------------------------------------------------------

void TMainForm::AddMeshToTree(TTreeNode *pPar, int msh)
{
    unsigned int i;

    // Add the mesh
    AnsiString name = Proj.meshes[msh].name;
    if (Proj.meshes[msh].type == MT_HULL && Proj.meshes[msh].parent == -1)
    {
        name += " (LOD " + IntToStr(Proj.meshes[msh].lod) + ")";
    }
    else if (Proj.meshes[msh].type == MT_DEBRIS)
    {
        name += " (Debris)";
    }

    TTreeNode *node = tvwHier->Items->AddChild(pPar, name);
    node->Data = (void*)msh;

    // Add children
    for (i=0; i<Proj.meshes.size(); i++)
    {
        if (Proj.meshes[i].parent == msh) AddMeshToTree(node, i);
    }
}

void TMainForm::PopulateTree()
{
    size_t i;

    // Add the main "Model" heading
    tvwHier->Items->Clear();
    TTreeNode *node = tvwHier->Items->Add(NULL, Proj.name);
    node->Data = (void*)-1;

    // Make it bold
    TVITEM tvi;
    tvi.mask = TVIF_STATE;
    tvi.hItem = node->ItemId;
    tvi.state = TVIS_BOLD;
    tvi.stateMask = TVIS_BOLD;
    TreeView_SetItem(node->Handle, &tvi);

    // The LODs First
    for (int j=0; j<Proj.nlods; j++)
    {
        // scan through all meshes trying to find a hull with
        // no parent and the correct LOD number
        for (i=0; i<Proj.meshes.size(); i++)
        {
            if ((Proj.meshes[i].lod == j) &&
                (Proj.meshes[i].parent == -1) &&
                (Proj.meshes[i].type == MT_HULL))
            {
                AddMeshToTree(node, i);
            }
        }
    }

    // the Debris go last (last and definately least)
    for (i=0; i<Proj.meshes.size(); i++)
    {
        if ((Proj.meshes[i].parent == -1)
            && (Proj.meshes[i].type == MT_DEBRIS))
        {
            AddMeshToTree(node, i);
        }
    }

    tvwHier->FullExpand();
    tvwHier->Selected = node;
}

void TMainForm::UpdateTree()
{
    Updating = true;

    void *selec = tvwHier->Selected->Data;
    int i;

    PopulateTree();

    // select the old one
    for (i=0; i<tvwHier->Items->Count; i++)
        if (tvwHier->Items->Item[0]->Data == selec);
            tvwHier->Selected = tvwHier->Items->Item[0];

    Updating = false;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::mnuToolsAutoShieldClick(TObject *Sender)
{
    static char quest[] = "Shields already exist for this project.  "
                 "Would you like to replace them with a new set?";
    if (Proj.shields.faces.size() > 0)
    {
        if (::MessageBox(0,quest,AppTitle.c_str(),MB_YESNO|MB_ICONQUESTION) == IDNO)
        {
            return;
        }
    }

    Proj.get_minmax();

    ShieldForm->Init(Proj.min, Proj.max);
    if (ShieldForm->ShowModal() != IDOK) return;

    // Prepare the old mesh
    Proj.shields.Clear();
    Proj.shields.name = "shields";
    Proj.mod = true;

    // Check how to create them
    if (ShieldForm->rdoGen->Checked == true)
    {
        int ndiv = StrToInt(ShieldForm->edtNumDiv->Text.c_str());
        GenSphere(&Proj.shields, ShieldForm->Center, ShieldForm->Size,
                  ndiv, ndiv);
        Proj.shields.calc_face_norms();

        //Face &g = Proj.shields.faces[0];
        //InfoMsg("(<%g, %g, %g>, <%g, %g, %g>, <%g, %g, %g>)*<%g, %g, %g>",
        //    Proj.shields.verts[g.verts[0]].x,Proj.shields.verts[g.verts[0]].y,Proj.shields.verts[g.verts[0]].z,
        //    Proj.shields.verts[g.verts[1]].x,Proj.shields.verts[g.verts[1]].y,Proj.shields.verts[g.verts[1]].z,
        //    Proj.shields.verts[g.verts[2]].x,Proj.shields.verts[g.verts[2]].y,Proj.shields.verts[g.verts[2]].z,
        //    g.norm.x, g.norm.y, g.norm.z);
    }
    else if (ShieldForm->rdoImport->Checked == true)
    {
        if (!Proj.shields.LoadFromDXF(ShieldForm->lblDXFName->Caption, false))
        {
            ErrorMsg("Error loading DXF");
            return;
        }
        Proj.shields.Triangulate();
    }

    cbxShowShields->Checked = true;
    InfoMsg("Shields created successfully with %d triangles and %d verts.",
            (int)Proj.shields.faces.size(), (int)Proj.shields.verts.size());
}
//---------------------------------------------------------------------------




void __fastcall TMainForm::pnlViewResize(TObject *Sender)
{
    if (MainGL.Created)
    {
        MainGL.ResetSize(true);
    }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::pnlViewPaint(TObject *Sender)
{
    if (MainGL.Created)
    {
        if (!pnlView->Visible) return;
        MainGL.BeginDrawing();
        DoRender();
        MainGL.EndDrawing();
    }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mnuViewModeWireClick(TObject *Sender)
{
    mnuViewModeWire->Checked = true;
    ViewFill = VF_WIRE;
    pnlView->Invalidate();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mnuViewModeSolidClick(TObject *Sender)
{
    mnuViewModeSolid->Checked = true;
    ViewFill = VF_SOLID;
    pnlView->Invalidate();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mnuViewModeTextClick(TObject *Sender)
{
    mnuViewModeText->Checked = true;
    ViewFill = VF_TEXT;
    pnlView->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::pnlViewMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    pnlView->SetFocus();
    if (!MainGL.Created) return;

    if (Shift.Contains(ssCtrl))
    {
        switch (MouseMode)
        {
        case MM_SELECT:
            Do_StartTranslateSelection();
            SetMM(MM_TRNSEL);
            break;
        }
    }
    else
    {
        switch (MouseMode)
        {
        case MM_SELECT:
            Do_Selection(X, Y);
            break;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::AppMessage(TMsg &AMessage, bool &Handled)
{
    if ((AMessage.message == WM_KEYDOWN) && (ActiveControl == pnlView) &&
        (this->Active))
    {
        WORD Key = 0;
        if (AMessage.wParam == VK_UP)
        {
            Key = VK_UP;
        }
        else if (AMessage.wParam == VK_DOWN)
        {
            Key = VK_DOWN;
        }
        else if (AMessage.wParam == VK_LEFT)
        {
            Key = VK_LEFT;
        }
        else if (AMessage.wParam == VK_RIGHT)
        {
            Key = VK_RIGHT;
        }
        else
        {
            return;
        }

        pnlViewKeyDown(0, Key, TShiftState());
        Handled = true;
    }
}

void __fastcall TMainForm::pnlViewKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    if (MainGL.Created == false) return;
    
    int x,y;

    Vector nx(MainGL.CamScale, 0.0f,            0.0f           );
    Vector ny(0.0f,            MainGL.CamScale, 0.0f           );
    Vector nz(0.0f,            0.0f,            MainGL.CamScale);
    nx.Rotate(MainGL.CamRot.x, MainGL.CamRot.y, MainGL.CamRot.z);
    ny.Rotate(MainGL.CamRot.x, MainGL.CamRot.y, MainGL.CamRot.z);
    nz.Rotate(MainGL.CamRot.x, MainGL.CamRot.y, MainGL.CamRot.z);

    if (Shift.Contains(ssAlt)) // Alternate
    {
        switch (Key)
        {
        case 0xBC:
            if (ViewObjs!=VO_FACE) break;
            TextScale(1.0f/TexScaleRate); break;
        case 0xBE:
            if (ViewObjs!=VO_FACE) break;
            TextScale(TexScaleRate); break;
        }
    }
    else if (Shift.Contains(ssShift)) // Shift
    {
        switch (Key)
        {
        case 0xBC: //comma
            if (ViewObjs!=VO_FACE) break;
            TextTranslate(0.0f, -TexTransRate); break;
        case 0xBE: //period
            if (ViewObjs!=VO_FACE) break;
            TextTranslate(0.0f, TexTransRate); break;
        case '1':
            Do_RepositionCamera(CV_Top, true);
            pnlView->Invalidate();
            break;
        case '2':
            Do_RepositionCamera(CV_Front, true);
            pnlView->Invalidate();
            break;
        case '3':
            Do_RepositionCamera(CV_Left, true);
            pnlView->Invalidate();
            break;
        case '4':
            Do_RepositionCamera(CV_Bottom, true);
            pnlView->Invalidate();
            break;
        case '5':
            Do_RepositionCamera(CV_Back, true);
            pnlView->Invalidate();
            break;
        case '6':
            Do_RepositionCamera(CV_Right, true);
            pnlView->Invalidate();
            break;
        case 'S':
            Do_AlignGridToSelection();
            break;
        }
    }
    else if (Shift.Contains(ssCtrl)) // Control
    {
        switch (Key)
        {
        case 0xBC:
            if (ViewObjs!=VO_FACE) break;
            TextRotate(-TexRotateRate); break;
        case 0xBE:
            if (ViewObjs!=VO_FACE) break;
            TextRotate(TexRotateRate); break;
        }
    }
    else // Nothing
    {
        switch (Key)
        {
        case VK_ADD:
            MainGL.CamScale /= 1.15f;
            MainGL.ResetSize(true);
            pnlView->Invalidate();
            break;
        case VK_SUBTRACT:
            MainGL.CamScale *= 1.15f;
            MainGL.ResetSize(true);
            pnlView->Invalidate();
            break;
        case VK_BACK:
            Sels.flush();
            On_SelChanged();
            break;
        case VK_SPACE:
            Do_ToggleSel();
            break;
        case VK_UP:
            MainGL.CamTrans.x += ny.x;
            MainGL.CamTrans.y += ny.y;
            MainGL.CamTrans.z += ny.z;
            pnlView->Invalidate();
            break;
        case VK_DOWN:
            MainGL.CamTrans.x -= ny.x;
            MainGL.CamTrans.y -= ny.y;
            MainGL.CamTrans.z -= ny.z;
            pnlView->Invalidate();
            break;
        case VK_LEFT:
            MainGL.CamTrans.x += nx.x;
            MainGL.CamTrans.y += nx.y;
            MainGL.CamTrans.z += nx.z;
            pnlView->Invalidate();
            break;
        case VK_RIGHT:
            MainGL.CamTrans.x -= nx.x;
            MainGL.CamTrans.y -= nx.y;
            MainGL.CamTrans.z -= nx.z;
            pnlView->Invalidate();
            break;
        case VK_PRIOR:
            MainGL.CamTrans.x += nz.x;
            MainGL.CamTrans.y += nz.y;
            MainGL.CamTrans.z += nz.z;
            pnlView->Invalidate();
            break;
        case VK_NEXT:
            MainGL.CamTrans.x -= nz.x;
            MainGL.CamTrans.y -= nz.y;
            MainGL.CamTrans.z -= nz.z;
            pnlView->Invalidate();
            break;
        case VK_INSERT:
            Do_InsertItem();
            break;
        case VK_DELETE:
            Do_DeleteSelection();
            break;
        case '1':
            Do_RepositionCamera(CV_Top, false);
            pnlView->Invalidate();
            break;
        case '2':
            Do_RepositionCamera(CV_Front, false);
            pnlView->Invalidate();
            break;
        case '3':
            Do_RepositionCamera(CV_Left, false);
            pnlView->Invalidate();
            break;
        case '4':
            Do_RepositionCamera(CV_Bottom, false);
            pnlView->Invalidate();
            break;
        case '5':
            Do_RepositionCamera(CV_Back, false);
            pnlView->Invalidate();
            break;
        case '6':
            Do_RepositionCamera(CV_Right, false);
            pnlView->Invalidate();
            break;
        case 'R':
            if (GetMousePos(x, y))
            {
                Do_StartRotateCam(x, y);
                SetMM(MM_ROTCAM);
            }
            break;
        case 'T':
            if (GetMousePos(x, y))
            {
                Do_StartTranslateCam(x, y);
                SetMM(MM_TRNCAM);
            }
            break;
        case 0xBC:
            if (ViewObjs!=VO_FACE) break;
            TextTranslate(-TexTransRate, 0.0f); break;
        case 0xBE:
            if (ViewObjs!=VO_FACE) break;
            TextTranslate(TexTransRate, 0.0f); break;
        }
    }

    Key = 0;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::pnlViewKeyUp(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
    switch (Key)
    {
    case 'R':
        SetMM(MM_SELECT);
        ReleaseCapture();
        break;
    case 'T':
        SetMM(MM_SELECT);
        ReleaseCapture();
        break;
    }
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::pnlViewMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
    // Get the 3D coordinates for this position
    if (MainGL.Created)
    {
        GLdouble nx=0,ny=0,nz=0,fx=0,fy=0,fz=0;
        GLint gly = MainGL.viewport[3] - (GLint)Y - 1;
        gluUnProject((GLdouble)X, (GLdouble)gly, 0.0,
                     MainGL.mvmatrix, MainGL.projmatrix, MainGL.viewport,
                     &nx, &ny, &nz);
        gluUnProject((GLdouble)X, (GLdouble)gly, 1.0,
                     MainGL.mvmatrix, MainGL.projmatrix, MainGL.viewport,
                     &fx, &fy, &fz);
        Vector pos(nx, ny, nz);
        Vector dir(fx-nx, fy-ny, fz-nz);
        dir.normalize();
        CurPos = PlaneLineInter(pos, dir,
                                MainGL.grid.GetNormal(), MainGL.grid.GetPos());
        AnsiString s;
        //s.printf("(%.2f, %.2f, %.2f)", CurPos.x, CurPos.y, CurPos.z);
        s.printf("(%g, %g, %g)", CurPos.x, CurPos.y, CurPos.z);
        stsMain->Panels->Items[1]->Text = s;

        //pnlView->Invalidate();
    }
    else
    {
        stsMain->Panels->Items[1]->Text = "";
    }

    switch (MouseMode)
    {
    case MM_ROTCAM:
        Do_RotateCam(X, Y);
        break;
    case MM_TRNCAM:
        Do_TranslateCam(X, Y);
        break;
    case MM_TRNSEL:
        Do_TranslateSelection();
        break;
    }
}
//---------------------------------------------------------------------------
static void TranslateSelWeapons(const Vector &Delta)
{
    for (size_t i=0; i<Sels.size(); i++)
    {
        size_t weap = (size_t)Sels[i];
        Proj.weapons[weap].Pos += Delta;
    }
    if ((int(Sels.find_sel(Sels.cur)) < 0) && (int(Sels.cur) >= 0))
    {
        size_t weap = (size_t)Sels.cur;
        Proj.weapons[weap].Pos += Delta;
    }
}
static void TranslateSelSpecials(const Vector &Delta)
{
    for (size_t i=0; i<Sels.size(); i++)
    {
        Proj.Specials[Sels[i]].Pos += Delta;
    }
    if ((int(Sels.find_sel(Sels.cur)) < 0) && (int(Sels.cur) >= 0)) // not on stack
    {
        Proj.Specials[Sels.cur].Pos += Delta;
    }
}
static void TranslateSelMeshes(const Vector &Delta)
{
    // Change everything except for the main node
    for (size_t i=0; i<Sels.size(); i++)
    {
        if (int(Sels[i]) == ViewNode) continue;
        Proj.meshes[Sels[i]].offset += Delta;
    }
    if ((int(Sels.find_sel(Sels.cur)) < 0) && (int(Sels.cur) != ViewNode) && (int(Sels.cur) >= 0))
    {
        Proj.meshes[Sels.cur].offset += Delta;
    }
}
static void TranslateSelThrusters(const Vector &Delta)
{
    for (size_t i=0; i<Sels.size(); i++)
    {
        Proj.thrusters[Sels[i]&0xFFFF].glows[Sels[i]>>16].pos += Delta;
    }
    if ((int(Sels.find_sel(Sels.cur)) < 0) && (int(Sels.cur) >= 0))
    {
        Proj.thrusters[Sels.cur&0xFFFF].glows[Sels.cur>>16].pos += Delta;
    }
}
void TMainForm::Do_StartTranslateSelection()
{
    if (MouseMode == MM_TRNSEL) return;

    //InfoMsg("Starting Translation!");

    OpOrg = CurPos;

    SetCaptureControl(pnlView);
}
void TMainForm::Do_TranslateSelection()
{
    Vector Delta = CurPos - OpOrg;
    OpOrg = CurPos;

    switch (ViewObjs)
    {
    case VO_MESH:
        TranslateSelMeshes(Delta);
        pnlView->Invalidate();
        break;
    case VO_WEAP:
        TranslateSelWeapons(Delta);
        pnlView->Invalidate();
        break;
    case VO_SPCL:
        TranslateSelSpecials(Delta);
        pnlView->Invalidate();
        break;
    case VO_THRUST:
        TranslateSelThrusters(Delta);
        pnlView->Invalidate();
        break;
    default:
        return;
    }

    Proj.mod = true;
    pnlView->Invalidate();
}
//---------------------------------------------------------------------------
void TMainForm::Do_StartRotateCam(int x, int y)
{
    if (MouseMode == MM_ROTCAM) return;

    OpOrgXY.x = x;
    OpOrgXY.y = y;
    OpOrg.x = MainGL.CamRot.x;
    OpOrg.y = MainGL.CamRot.y;
    OpOrg.z = MainGL.CamRot.z;

    SetCaptureControl(pnlView);
}

void TMainForm::Do_RotateCam(int x, int y)
{
    int dx = x - OpOrgXY.x;
    int dy = y - OpOrgXY.y;

    MainGL.CamRot.x = (float)(OpOrg.x + (double)dy);
    MainGL.CamRot.y = (float)(OpOrg.y + (double)dx);

    pnlView->Invalidate();
}
//---------------------------------------------------------------------------
void TMainForm::Do_StartTranslateCam(int x, int y)
{
    if (MouseMode == MM_TRNCAM) return;

    OpOrgXY.x = x;
    OpOrgXY.y = y;

    SetCaptureControl(pnlView);
}

void TMainForm::Do_TranslateCam(int x, int y)
{
    /*int dx = x - OpOrgXY.x;
    int dy = y - OpOrgXY.y;

    MainGL.CamTrans.x = (float)(OpOrg.x + (double)dy);
    MainGL.CamTrans.y = (float)(OpOrg.y + (double)dx);

    pnlView->Invalidate();*/
}
//---------------------------------------------------------------------------

void TMainForm::Do_ToggleSel()
{
    // if nothing is selected, then do do anything
    if (Sels.get_single() == (unsigned int)(-1)) return;

    // first, see if the current sel is on the stack
    int i = Sels.find_sel(Sels.cur);

    // it is, so we must remove it
    if (i >= 0)
    {
        Sels.erase(&Sels[i]);
    }
    else
    {
        Sels.push_back(Sels.cur);
    }

    On_SelChanged();
}
//---------------------------------------------------------------------------

void TMainForm::Do_Selection(int x, int y)
{
    if ((LastSelXY.x == x) && (LastSelXY.y == y))
    {
        MainGL.CurSel++;
        if (MainGL.CurSel >= MainGL.nHits) MainGL.CurSel = 0;
        if (MainGL.nHits > 0)
        {
            Sels.cur = MainGL.SelectBuf[MainGL.CurSel].name;
        }
        else
        {
            Sels.cur = (unsigned int)(-1);
        }
    }
    else
    {
        MainGL.SimplePick(x, y);
    }

    LastSelXY.x = x;
    LastSelXY.y = y;

    On_SelChanged();
}
//---------------------------------------------------------------------------

void TMainForm::On_SelChanged()
{
    pnlView->Invalidate();

    int count = Sels.count_sel();
    stsMain->Panels->Items[0]->Text = IntToStr(count) + " Selected";

    switch (ViewObjs)
    {
    case VO_MESH:
        FillMeshInfo(count);
        break;
    case VO_FACE:
        FillFaceInfo(count);
        // Try to adjust the texture translation rate
        TexTransRate = DEFAULT_TEXT_TRANS_RATE;
        if (Sels.get_single() != (unsigned int)(-1))
        {
            Face &f = Proj.meshes[Sels.get_single() & 0xFFFF].faces[Sels.get_single() >> 16];
            if (f.texture >= 0)
            {
                if (Proj.mats[f.texture].image)
                {
                    OGLTexture *t = ((OGLTexture*)Proj.mats[f.texture].image);
                    TexTransRate = (1.0f / float(max(t->width, t->height)))/2.0f;
                }
            }
        }
        break;
    case VO_VERT:
        FillVertInfo(count);
        break;
    case VO_WEAP:
        FillWeaponInfo(count);
        break;
    case VO_THRUST:
        FillThrustInfo(count);
        break;
    case VO_SPCL:
        FillSpecialInfo(count);
        break;
    }
}
//---------------------------------------------------------------------------

// Adds a string with CRs or LFs to a multiline memo
void SetMemoText(TMemo *mmo, AnsiString text)
{
    char *buf, *p;
    mmo->Lines->Clear();

    buf = new char[text.Length()+1];
    strcpy(buf, text.c_str());

    p = strtok(buf, "\r\n");
    while (p)
    {
        mmo->Lines->Add(p);
        p = strtok(NULL, "\r\n");
    }

    delete[] buf;
}

void TMainForm::FillMeshInfo(int cnt)
{
    Updating = true;

    int sm = (int)Sels.get_single();

    if (cnt < 1)
    {
        edtMshNm->Text = "";
        edtMshNm->Enabled = false;
        edtMshNm->Color = clBtnFace;
        edtMshOff->Text = "";
        edtMshOff->Enabled = false;
        edtMshOff->Color = clBtnFace;
        edtMshProp->Text = "";
        edtMshProp->Enabled = false;
        edtMshProp->Color = clBtnFace;
        btnMshInvertNorms->Enabled = false;
        btnMshDefText->Enabled = false;
        btnMshDelete->Enabled = false;
        btnMshInfo->Enabled = false;
        btnMshCenter->Enabled = false;
        btnMshScale->Enabled = false;
        Updating = false;
        return;
    }

    if (cnt > 1)
    {
        edtMshNm->Text = "";
        edtMshNm->Enabled = false;
        edtMshNm->Color = clBtnFace;
    }
    else
    {
        edtMshNm->Text = Proj.meshes[sm].name;
        edtMshNm->Enabled = true;
        edtMshNm->Color = clWindow;
    }

    edtMshOff->Text = VecToStr(Proj.meshes[sm].offset);
    edtMshOff->Enabled = true;
    edtMshOff->Color = clWindow;
    SetMemoText(edtMshProp, Proj.meshes[sm].props);
    edtMshProp->Enabled = true;
    edtMshProp->Color = clWindow;
    btnMshInvertNorms->Enabled = true;
    btnMshDefText->Enabled = true;
    btnMshDelete->Enabled = true;
    btnMshInfo->Enabled = true;
    btnMshCenter->Enabled = true;
    btnMshScale->Enabled = true;

    Updating = false;
}
void TMainForm::FillWeaponInfo(int cnt)
{
    Updating = true;

    int single = (int)Sels.get_single();

    if (cnt < 1 || single < 0)
    {
        edtWeapGroup->Text = "";
        edtWeapGroup->Enabled = false;
        edtWeapGroup->Color = clBtnFace;
        edtWeapID->Text = "";
        edtWeapID->Enabled = false;
        edtWeapID->Color = clBtnFace;
        edtWeapPos->Text = "";
        edtWeapPos->Enabled = false;
        edtWeapPos->Color = clBtnFace;
        edtWeapNorm->Text = "";
        edtWeapNorm->Enabled = false;
        edtWeapNorm->Color = clBtnFace;
        rgrpWeapType->Enabled = false;
        rgrpWeapType->ItemIndex = -1;
        Updating = false;
        return;
    }

    if (cnt > 1)
    {
        edtWeapID->Text = "";
        edtWeapID->Enabled = false;
        edtWeapID->Color = clBtnFace;
    }
    else
    {
        edtWeapID->Text = IntToStr(Proj.weapons[single].ID);
        edtWeapID->Enabled = true;
        edtWeapID->Color = clWindow;
    }

    edtWeapGroup->Text = IntToStr(Proj.weapons[single].Group);
    edtWeapGroup->Enabled = true;
    edtWeapGroup->Color = clWindow;
    rgrpWeapType->ItemIndex = Proj.weapons[single].Type - 1;
    rgrpWeapType->Enabled = true;
    edtWeapPos->Text = VecToStr(Proj.weapons[single].Pos);
    edtWeapPos->Enabled = true;
    edtWeapPos->Color = clWindow;
    edtWeapNorm->Text = VecToStr(Proj.weapons[single].Normal);
    edtWeapNorm->Enabled = true;
    edtWeapNorm->Color = clWindow;

    Updating = false;
}
void TMainForm::FillSpecialInfo(int cnt)
{
    Updating = true;

    int single = (int)Sels.get_single();

    if (cnt < 1 || single < 0)
    {
        edtSpclName->Text = "";
        edtSpclName->Enabled = false;
        edtSpclName->Color = clBtnFace;
        mmoSpclProps->Text = "";
        mmoSpclProps->Enabled = false;
        mmoSpclProps->Color = clBtnFace;
        edtSpclPos->Text = "";
        edtSpclPos->Enabled = false;
        edtSpclPos->Color = clBtnFace;
        edtSpclRad->Text = "";
        edtSpclRad->Enabled = false;
        edtSpclRad->Color = clBtnFace;
        Updating = false;
        return;
    }

    edtSpclName->Text = Proj.Specials[single].Name;
    edtSpclName->Enabled = true;
    edtSpclName->Color = clWindow;
    SetMemoText(mmoSpclProps, Proj.Specials[single].Props);
    mmoSpclProps->Enabled = true;
    mmoSpclProps->Color = clWindow;
    edtSpclPos->Text = VecToStr(Proj.Specials[single].Pos);
    edtSpclPos->Enabled = true;
    edtSpclPos->Color = clWindow;
    edtSpclRad->Text = FloatToStr(Proj.Specials[single].Radius);
    edtSpclRad->Enabled = true;
    edtSpclRad->Color = clWindow;

    Updating = false;
}
void TMainForm::FillFaceInfo(int cnt)
{
    Updating = true;

    unsigned int face = (unsigned int)Sels.get_single();

    if (cnt < 1)
    {
        cboFaceMap->ItemIndex = -1;
        cboFaceMap->Enabled = false;
        cboFaceMap->Color = clBtnFace;
        btnFaceMap->Enabled = false;
        btnFaceInvert->Enabled = false;
        btnFaceCreateWeap->Enabled = false;
        btnFaceReplaceMat->Enabled = false;
        Updating = false;
        return;
    }

    if ((int)face == -1) return;

    unsigned int mesh = face & 0xFFFF;
    face >>= 16;
    cboFaceMap->ItemIndex = Proj.meshes[mesh].faces[face].texture;
    cboFaceMap->Enabled = true;
    cboFaceMap->Color = clWindow;
    btnFaceMap->Enabled = true;
    btnFaceInvert->Enabled = true;
    btnFaceReplaceMat->Enabled = true;
    if (cnt == 1)
    {
        btnFaceCreateWeap->Enabled = true;
    }
    else
    {
        btnFaceCreateWeap->Enabled = false;
    }

    Updating = false;
}
void TMainForm::FillVertInfo(int cnt)
{
    Updating = true;

    int vert = (int)Sels.get_single();
    int mesh = vert & 0xFFFF;
    vert >>= 16;

    if (cnt < 1 || (mesh == -1))
    {
        edtVertVal->Text = "";
        edtVertVal->Enabled = false;
        edtVertVal->Color = clBtnFace;
        Updating = false;
        return;
    }

    edtVertVal->Text = VecToStr(Proj.meshes[mesh].verts[vert]);
    edtVertVal->Enabled = true;
    edtVertVal->Color = clWindow;

    Updating = false;
}
void TMainForm::FillThrustInfo(int cnt)
{
    Updating = true;

    int single = (int)Sels.get_single();

    if (cnt < 1 || single < 0)
    {
        mmoThrustProps->Text = "";
        mmoThrustProps->Enabled = false;
        mmoThrustProps->Color = clBtnFace;
        edtGlowPos->Text = "";
        edtGlowPos->Enabled = false;
        edtGlowPos->Color = clBtnFace;
        edtGlowRad->Text = "";
        edtGlowRad->Enabled = false;
        edtGlowRad->Color = clBtnFace;
        edtGlowNorm->Text = "";
        edtGlowNorm->Enabled = false;
        edtGlowNorm->Color = clBtnFace;
        lblThrust->Caption = "No Thrusters Selected";
        Updating = false;
        return;
    }

    lblThrust->Caption = "Thruster " + IntToStr(single&0xFFFF) + " Properties:";
    SetMemoText(mmoThrustProps, Proj.thrusters[single&0xFFFF].Props);
    mmoThrustProps->Enabled = true;
    mmoThrustProps->Color = clWindow;
    edtGlowPos->Text = VecToStr(Proj.thrusters[single&0xFFFF].glows[single>>16].pos);
    edtGlowPos->Enabled = true;
    edtGlowPos->Color = clWindow;
    edtGlowRad->Text = FloatToStr(Proj.thrusters[single&0xFFFF].glows[single>>16].radius);
    edtGlowRad->Enabled = true;
    edtGlowRad->Color = clWindow;
    edtGlowNorm->Text = VecToStr(Proj.thrusters[single&0xFFFF].glows[single>>16].norm);
    edtGlowNorm->Enabled = true;
    edtGlowNorm->Color = clWindow;

    Updating = false;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::nbkRightPageChanged(TObject *Sender)
{
    On_SelChanged();
}
//---------------------------------------------------------------------------




int __fastcall IntCompSB(void * Item1, void * Item2) //small to big
{
    return ((int)Item1 - (int)Item2);
}
int __fastcall IntCompBS(void * Item1, void * Item2) //big to small
{
    return ((int)Item2 - (int)Item1);
}


void __fastcall TMainForm::mnuHelpHomepageClick(TObject *Sender)
{
    if (ShellExecute(Handle, "open",
        "http://freespace.volitionwatch.com/segultuch/",
        NULL, NULL, 0) <= (void*)32)
    {
        ErrorMsg("Could not execute Home Page.");
    }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mnuFileSavePOFClick(TObject *Sender)
{
    TCursor Save_Cursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;    // Show hourglass cursor
    try
    {
        Proj.MakePOF();
    }
    __finally
    {
        Screen->Cursor = Save_Cursor; // always restore the cursor
        InfoMsg("Finished writing POF file.");
    }
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::mnuEditSelAllClick(TObject *Sender)
{
    unsigned int i, j;

    Sels.flush();

    switch (ViewObjs)
    {
    case VO_MESH:
        for (i=0; i<Proj.meshes.size(); i++)
            Sels.push_back(i);

        break;
    case VO_FACE:
        for (i=0; i<Proj.meshes.size(); i++)
            for (j=0; j<Proj.meshes[i].faces.size(); j++)
                Sels.push_back(i + (j<<16));
        break;
    case VO_VERT:
        for (i=0; i<Proj.meshes.size(); i++)
            for (j=0; j<Proj.meshes[i].verts.size(); j++)
                Sels.push_back(i + (j<<16));
        break;
    }

    On_SelChanged();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mnuEditDeselAllClick(TObject *Sender)
{
    Sels.flush();
    On_SelChanged();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mnuEditClearInfoClick(TObject *Sender)
{
    MainForm->rchInfo->Clear();
}
//---------------------------------------------------------------------------





void TMainForm::TextTranslate(float du, float dv)
{
    int mesh;
    Face *f;
    int i,j;

    // invert all the permanent faces
    for (i=0; i<(int)Sels.size(); i++)
    {
        mesh = (int)(Sels[i] & 0xFFFF);
        f = &Proj.meshes[mesh].faces[Sels[i] >> 16];
        for (j=0; j<f->nverts; j++)
        {
            f->texverts[j].u += du;
            f->texverts[j].v += dv;
        }
    }

    // invert the current mesh if it is not on the stack
    if (((int)Sels.cur != -1) && (Sels.find_sel(Sels.cur) < 0))
    {
        mesh = (int)(Sels.cur & 0xFFFF);
        f = &Proj.meshes[mesh].faces[Sels.cur >> 16];
        for (j=0; j<f->nverts; j++)
        {
            f->texverts[j].u += du;
            f->texverts[j].v += dv;
        }
    }

    Proj.mod = true;
    pnlView->Invalidate();
}
//---------------------------------------------------------------------------

void TMainForm::TextRotate(float ang)
{
    int mesh;
    Face *f;
    int i;

    // invert all the permanent faces
    for (i=0; i<(int)Sels.size(); i++)
    {
        mesh = (int)(Sels[i] & 0xFFFF);
        f = &Proj.meshes[mesh].faces[Sels[i] >> 16];
        f->tex_rot(ang);
    }

    // invert the current mesh if it is not on the stack
    if (((int)Sels.cur != -1) && (Sels.find_sel(Sels.cur) < 0))
    {
        mesh = (int)(Sels.cur & 0xFFFF);
        f = &Proj.meshes[mesh].faces[Sels.cur >> 16];
        f->tex_rot(ang);
    }

    Proj.mod = true;
    pnlView->Invalidate();
}
//---------------------------------------------------------------------------

void TMainForm::TextScale(float val)
{
    int mesh;
    Face *f;
    int i;

    // all the permanent faces
    for (i=0; i<(int)Sels.size(); i++)
    {
        mesh = (int)(Sels[i] & 0xFFFF);
        f = &Proj.meshes[mesh].faces[Sels[i] >> 16];
        f->tex_scale(val);
    }

    // the current mesh if it is not on the stack
    if (((int)Sels.cur != -1) && (Sels.find_sel(Sels.cur) < 0))
    {
        mesh = (int)(Sels.cur & 0xFFFF);
        f = &Proj.meshes[mesh].faces[Sels.cur >> 16];
        f->tex_scale(val);
    }

    Proj.mod = true;
    pnlView->Invalidate();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mnuViewLightClick(TObject *Sender)
{
    MainGL.UseLight = !MainGL.UseLight;
    mnuViewLight->Checked = MainGL.UseLight;
    pnlView->Invalidate();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mnuFileNewPOFClick(TObject *Sender)
{
    CleanupModel();
    MainGL.Clear();

    FILE *f;
    long start, end;
    pof_file pof;

    // Open the file (container or model)
    dlgOpen->InitialDir = ModelsDir;
    dlgOpen->Filter = PofFilter;
    if (!dlgOpen->Execute()) return;

    // Find out what kind of file this is and take the appropriate action
    AnsiString ext = ExtractFileExt(dlgOpen->FileName).LowerCase();
    if (ext == ".pof")
    {
        f = fopen(dlgOpen->FileName.c_str(), "rb");
        if (f == NULL)
        {
            ErrorMsg("Could not open \"%s\".", dlgOpen->FileName.c_str());
            return;
        }

        //determine the length of the file
        start = 0;
        fseek(f, 0, SEEK_END);
        end = ftell(f);
        fseek(f, 0, SEEK_SET);

        Proj.name = ExtractFileName(dlgOpen->FileName);
    }
    else if (ext == ".vp")
    {
        VPForm->FileName = dlgOpen->FileName;
        if (VPForm->ShowModal() != mrOk) return;

        f = fopen(dlgOpen->FileName.c_str(), "rb");
        if (f == NULL)
        {
            ErrorMsg("Could not open \"%s\".", dlgOpen->FileName.c_str());
            return;
        }

        start = VPForm->fstart;
        end = VPForm->fend;

        Proj.name = VPForm->fname;
    }
    else
    {
        ErrorMsg("Unknown file type.");
        return;
    }

    // read the file
    if (!pof.read(f, start, end))
    {
        fclose(f);
        return;
    }
    fclose(f);

    // Write the new name
    Proj.name = ChangeFileExt(Proj.name, "");
    Proj.filename = ProjDir + Proj.name + PrjExt;
    SetTitle();

    // Convert the data types
    convert_model(&pof);

    Proj.Write();
    MainGL.StartUp();
    Proj.recalc_all();
    MainGL.ResetSize(true);

    PopulateTree();
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::tvwHierChange(TObject *Sender, TTreeNode *Node)
{
    if (Updating) return;

    ViewNode = (int)tvwHier->Selected->Data;
    Sels.flush();
    On_SelChanged();
}
//---------------------------------------------------------------------------
const Vector gamma(0.01f, 0.01f, 0.01f);

void DrawBspHull(BspHull hull, Vector &off)
{
    hull.min += off;
    hull.max += off;
                        
    glBegin(GL_QUADS);
        glVertex3f(hull.min.x, hull.min.y, hull.min.z);
        glVertex3f(hull.min.x, hull.max.y, hull.min.z);
        glVertex3f(hull.max.x, hull.max.y, hull.min.z);
        glVertex3f(hull.max.x, hull.min.y, hull.min.z);
    glEnd();
    glBegin(GL_QUADS);
        glVertex3f(hull.min.x, hull.min.y, hull.max.z);
        glVertex3f(hull.min.x, hull.max.y, hull.max.z);
        glVertex3f(hull.max.x, hull.max.y, hull.max.z);
        glVertex3f(hull.max.x, hull.min.y, hull.max.z);
    glEnd();
    glBegin(GL_QUADS);
        glVertex3f(hull.min.x, hull.min.y, hull.max.z);
        glVertex3f(hull.min.x, hull.max.y, hull.max.z);
        glVertex3f(hull.min.x, hull.max.y, hull.min.z);
        glVertex3f(hull.min.x, hull.min.y, hull.min.z);
    glEnd();
    glBegin(GL_QUADS);
        glVertex3f(hull.max.x, hull.min.y, hull.max.z);
        glVertex3f(hull.max.x, hull.max.y, hull.max.z);
        glVertex3f(hull.max.x, hull.max.y, hull.min.z);
        glVertex3f(hull.max.x, hull.min.y, hull.min.z);
    glEnd();
}

void sim_sortnorm(BspHull MainHull, Mesh *msh, Vector &off,
                  vector<FaceList> &Cocentrics)
{
    BspHull front, back;
    Vector norm, center;
    vector<FaceList> front_list, back_list;
    vector<Face>::iterator f;
    unsigned int pos_front, pos_back, pos_pre, pos_post, pos_onl;
    unsigned int pos_mem;

    // Draw this hull
    DrawBspHull(MainHull, off);

    // Count the number of lists in the Concentrics list that are in this
    // volume.
    front_list.clear();
    for (f = msh->faces.begin(); f != msh->faces.end(); f++)
    {
        MainHull.addto_uniquelist(msh, &(*f), front_list);
    }
    if (front_list.size() == 0)
    {
        return;
    }
    else if (front_list.size() == 1)
    {
        return;
    }

    // This hull has more than 1 unique faces, therefore we must
    // split the hull into two equal volumes and test them
    MainHull.split(&back, &front, &norm, &center);

    // Count the number of faces in each volume and keep track
    // of cocentric faces
    front_list.clear();
    back_list.clear();
    for (f = msh->faces.begin(); f != msh->faces.end(); f++)
    {
        // Try to add this face to either of the lists
        if (!back.addto_uniquelist(msh, &(*f), back_list))
            front.addto_uniquelist(msh, &(*f), front_list);
    }

    // If there are lots of unique faces then continue
    sim_sortnorm(back, msh, off, Cocentrics);
    sim_sortnorm(front, msh, off, Cocentrics);
}

void __fastcall TMainForm::GenerateBSPHulls1Click(TObject *Sender)
{
    if (!pnlView->Visible) return;
    if (!MainGL.Created) return;
    if (ViewObjs != VO_MESH) return;
    if ((int)Sels.cur == -1) return;

    MainGL.Activate();

    BspDispList = glGenLists(1);
    glNewList(BspDispList, GL_COMPILE);

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_CULL_FACE);

    glColor4f(1.0, 1.0, 1.0, 1.0);

    // Prepare the mesh
    int nm = 0xFFFF & (int)Sels.cur;
    Proj.get_minmax();
    Vector off = Proj.get_mesh_off(nm, -1);
    Mesh *msh = &Proj.meshes[nm];
    msh->calc_face_centers();

    // Get a list of all faces and their cocentrics
    BspHull MainHull;
    MainHull.min = msh->min - gamma;
    MainHull.max = msh->max + gamma;
    vector<FaceList> CocentricsList;
    vector<Face>::iterator f;
    for (f = msh->faces.begin(); f != msh->faces.end(); f++)
    {
        MainHull.addto_uniquelist(msh, &(*f), CocentricsList);
    }
    sim_sortnorm(MainHull, msh, off, CocentricsList);

    glEndList();

    MainGL.Deactivate();

    pnlView->Invalidate();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ConvPOFDXFExecute(TObject *Sender)
{
    FILE *f;
    pof_file pof;
    long start, end;
    AnsiString filename;

    // Open the file (container or model)
    dlgOpen->InitialDir = ModelsDir;
    dlgOpen->Filter = PofFilter;
    if (!dlgOpen->Execute()) return;

    // Find out what kind of file this is and take the appropriate action
    AnsiString ext = ExtractFileExt(dlgOpen->FileName).LowerCase();
    if (ext == ".pof")
    {
        f = fopen(dlgOpen->FileName.c_str(), "rb");
        if (f == NULL)
        {
            ErrorMsg("Could not open \"%s\".", dlgOpen->FileName.c_str());
            return;
        }

        //determine the length of the file
        start = 0;
        fseek(f, 0, SEEK_END);
        end = ftell(f);
        fseek(f, 0, SEEK_SET);

        filename = ProjDir + ExtractFileName(dlgOpen->FileName);
    }
    else if (ext == ".vp")
    {
        VPForm->FileName = dlgOpen->FileName;
        if (VPForm->ShowModal() != mrOk) return;

        f = fopen(dlgOpen->FileName.c_str(), "rb");
        if (f == NULL)
        {
            ErrorMsg("Could not open \"%s\".", dlgOpen->FileName.c_str());
            return;
        }

        start = VPForm->fstart;
        end = VPForm->fend;

        filename = ProjDir + VPForm->fname;
    }
    else
    {
        ErrorMsg("Unknown file type.");
        return;
    }

    // read the file
    if (!pof.read(f, start, end)) return;

    // Write the new dxf file
    filename = ChangeFileExt(filename, ".dxf");
    convert_dxf(&pof, filename.c_str());
    fclose(f);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::OpenProjExecute(TObject *Sender)
{
    dlgOpen->InitialDir = ProjDir;
    dlgOpen->Filter = PrjFilter;
    if (!dlgOpen->Execute()) return;

    DoOpen(dlgOpen->FileName);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SaveProjExecute(TObject *Sender)
{
    Proj.Write();
    Proj.mod = false;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ModeMeshExecute(TObject *Sender)
{
    if (ViewObjs != VO_MESH)
    {
        ViewObjs = VO_MESH;
        Sels.flush();
    }
    pnlView->PopupMenu = 0;
    nbkRight->ActivePage = "Mesh";
}
//---------------------------------------------------------------------------



void __fastcall TMainForm::ModeThrustExecute(TObject *Sender)
{
    if (ViewObjs != VO_THRUST)
    {
        ViewObjs = VO_THRUST;
        Sels.flush();
    }
    pnlView->PopupMenu = 0;
    nbkRight->ActivePage = "Thrusters";
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ModeFaceExecute(TObject *Sender)
{
    if (ViewObjs != VO_FACE)
    {
        ViewObjs = VO_FACE;
        Sels.flush();
    }
    // Fill the List in with all active MAPs
    cboFaceMap->Items->Clear();
    vector<ImgString>::iterator i = Proj.mats.begin();
    for (; i != Proj.mats.end(); i++)
    {
        cboFaceMap->Items->Add((*i).s);
    }
    pnlView->PopupMenu = 0;//mnuFace;
    nbkRight->ActivePage = "Face";
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ModeVertExecute(TObject *Sender)
{
    if (ViewObjs != VO_VERT)
    {
        ViewObjs = VO_VERT;
        Sels.flush();
    }
    pnlView->PopupMenu = 0;
    nbkRight->ActivePage = "Vertex";

}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ModeMeshUpdate(TObject *Sender)
{
    ModeMesh->Checked = ViewObjs == VO_MESH;    
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ModeFaceUpdate(TObject *Sender)
{
    ModeFace->Checked = ViewObjs == VO_FACE;    
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ModeVertUpdate(TObject *Sender)
{
    ModeVert->Checked = ViewObjs == VO_VERT;    
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ModeThrustUpdate(TObject *Sender)
{
    ModeThrust->Checked = ViewObjs == VO_THRUST;    
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::btnMshInvertNormsClick(TObject *Sender)
{
    int mesh;
    int i,j;

    // invert all the permanent faces
    for (i=0; i<(int)Sels.size(); i++)
    {
        mesh = (int)Sels[i];
        for (j=0; j<(int)Proj.meshes[mesh].faces.size(); j++)
            Proj.meshes[mesh].invert_face(j);
        Proj.meshes[mesh].calc_face_norms();
    }

    // invert the current mesh if it is not on the stack
    if (Sels.find_sel(Sels.cur) < 0)
    {
        mesh = (int)Sels.cur;
        for (j=0; j<(int)Proj.meshes[mesh].faces.size(); j++)
            Proj.meshes[mesh].invert_face(j);
        Proj.meshes[mesh].calc_face_norms();
    }

    Proj.mod = true;
    pnlView->Invalidate();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::btnMshDefTextClick(TObject *Sender)
{
    if (Application->MessageBox(
    "Are you sure you want to overwrite existing texture vertices?",
    Application->Title.c_str(), MB_YESNO|MB_ICONQUESTION) != IDYES)
    {
        return;
    }

    int mesh;
    int i,j;

    // calc all the permanent faces
    for (i=0; i<(int)Sels.size(); i++)
    {
        mesh = (int)Sels[i];
        for (j=0; j<(int)Proj.meshes[mesh].faces.size(); j++)
            Proj.meshes[mesh].calc_face_text(j);
    }

    // calc the current mesh if it is not on the stack
    if ((Sels.find_sel(Sels.cur) < 0) && (int(Sels.cur) > 0)) 
    {
        mesh = (int)Sels.cur;
        for (j=0; j<(int)Proj.meshes[mesh].faces.size(); j++)
            Proj.meshes[mesh].calc_face_text(j);
    }

    Proj.mod = true;
    pnlView->Invalidate();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::edtMshNmChange(TObject *Sender)
{
    if (Updating) return;

    int sm = (int)Sels.get_single();

    Proj.meshes[sm].name = edtMshNm->Text;

    Proj.mod = true;
    UpdateTree();    
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::edtMshOffChange(TObject *Sender)
{
    if (Updating) return;

    int i;
    int mesh;

    for (i=0; i<(int)Sels.size(); i++)
    {
        mesh = (int)Sels[i];
        Proj.meshes[mesh].offset = StrToVec(edtMshOff->Text);
    }
    if ((int)Sels.find_sel(Sels.cur) < 0) // not on stack
    {
        mesh = (int)Sels.cur;
        Proj.meshes[mesh].offset = StrToVec(edtMshOff->Text);
    }

    Proj.mod = true;
    pnlView->Invalidate();    
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::edtMshPropChange(TObject *Sender)
{
    if (Updating) return;

    int i;
    int mesh;

    AnsiString text;
    text = "";
    for (i=0; i<edtMshProp->Lines->Count; i++)
    {
        if (edtMshProp->Lines->Strings[i].Length() < 1) continue;
        if (i != 0) text += "\n";
        text += edtMshProp->Lines->Strings[i];
    }

    for (i=0; i<(int)Sels.size(); i++)
    {
        mesh = (int)Sels[i];
        Proj.meshes[mesh].props = text;
    }
    if ((int)Sels.find_sel(Sels.cur) < 0) // not on stack
    {
        mesh = (int)Sels.cur;
        Proj.meshes[mesh].props = text;
    }

    Proj.mod = true;
    
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::btnMshInfoClick(TObject *Sender)
{
        MeshInfoForm->FillList();
    MeshInfoForm->ShowModal();
    
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::btnMshDeleteClick(TObject *Sender)
{
    if (Application->MessageBox("Are you sure you want to delete all selected meshes?",
        "FreeDXF", MB_YESNO|MB_ICONQUESTION) != IDYES) return;

    // make a list of all the meshes
    TList *SelList = new TList;

    int i;

    for (i=0; i<(int)Sels.size(); i++)
    {
        SelList->Add((void*)Sels[i]);
    }
    if (Sels.find_sel(Sels.cur) < 0) // not on stack
    {
        SelList->Add((void*)Sels.cur);
    }

    SelList->Sort(IntCompBS);

    for (i=0; i<SelList->Count; i++)
    {
        Proj.delete_mesh((int)SelList->Items[i]);
    }

    delete SelList;

    Proj.mod = true;
    Sels.flush();
    pnlView->Invalidate();
    UpdateTree();
    
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::btnFaceInvertClick(TObject *Sender)
{
    int mesh;
    int i;

    // invert all the permanent faces
    for (i=0; i<(int)Sels.size(); i++)
    {
        mesh = (int)(Sels[i] & 0xFFFF);
        Proj.meshes[mesh].invert_face((int)Sels[i]>>16);
    }

    // invert the current mesh if it is not on the stack
    if (Sels.find_sel(Sels.cur) < 0)
    {
        mesh = (int)(Sels.cur & 0xFFFF);
        Proj.meshes[mesh].invert_face((int)Sels.cur>>16);
    }

    InfoMsg("Inverted Selected Faces.");
    Proj.mod = true;
    pnlView->Invalidate();
    
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::ImportDXFExecute(TObject *Sender)
{
    dlgOpen->InitialDir = ProjDir;
    dlgOpen->Filter = DxfFilter;
    if (!dlgOpen->Execute()) return;

    // If they are adding this node to root, then we must ask whether it is
    // hull or debris.  Otherwise, it inherits its type from its parent.
    if ((ViewNode == -1) || (ViewNode >= (int)Proj.meshes.size()))
    {
        static char quest[] = { "Does this DXF represent a new level of detail?  If you "
            "answer No, then it will be added as a piece of debris." };
        if (::MessageBox(0, quest, AppTitle.c_str(), MB_YESNO|MB_ICONQUESTION) == IDYES)
        {
            if (Proj.AddMeshFromDXF(dlgOpen->FileName, -1, Proj.nlods, MT_HULL))
            {
                Proj.nlods++;
            }
        }
        else
        {
            Proj.AddMeshFromDXF(dlgOpen->FileName, -1, 0, MT_DEBRIS);
        }
    }
    else
    {
        Proj.AddMeshFromDXF(dlgOpen->FileName, ViewNode,
            Proj.meshes[ViewNode].lod,
            Proj.meshes[ViewNode].type);
    }

    // Add a new level of detail
    Proj.mod = true;
    Proj.recalc_all();
    MainGL.ResetSize(true);

    PopulateTree();
}
//---------------------------------------------------------------------------
bool RemoveMapIfNotUsed(int old_map)
{
    int num_using_old_map = 0;
    
    for (size_t i=0; i < Proj.meshes.size(); i++)
    {
        for (size_t j=0; j < Proj.meshes[i].faces.size(); j++)
        {
            if (Proj.meshes[i].faces[j].texture == old_map)
            {
                num_using_old_map++;
            }
        }
    }

    if (num_using_old_map == 0)
    {
        // simply remove the map, and update all the faces with new indexes
        if (Proj.mats[old_map].image != 0)
        {
            MainGL.Activate();
            delete ((OGLTexture*)Proj.mats[old_map].image);
            MainGL.Deactivate();
            Proj.mats[old_map].image = 0;
        }
        Proj.mats.erase(Proj.mats.begin()+old_map);

        for (size_t i=0; i < Proj.meshes.size(); i++)
        {
            for (size_t j=0; j < Proj.meshes[i].faces.size(); j++)
            {
                if (Proj.meshes[i].faces[j].texture > old_map)
                {
                    Proj.meshes[i].faces[j].texture--;
                }
            }
        }

        return true;
    }
    return false;
}

bool RemoveUnusedMaps()
{
    int count = 0; // number of maps removed
    int i = 0;

    while (i < (int)Proj.mats.size())
    {
        if (RemoveMapIfNotUsed(i) == true)
        {
            count++;
        }
        else
        {
            i++;
        }
    }

    return (count > 0);
}

void __fastcall TMainForm::btnFaceMapClick(TObject *Sender)
{
    if (cboFaceMap->ItemIndex >= 0)
    {
        MapSelForm->OldFileName =
            cboFaceMap->Items->Strings[cboFaceMap->ItemIndex] + ".pcx";
    }
    else
    {
        MapSelForm->OldFileName = "";
    }

    if (MapSelForm->ShowModal() != IDOK) return;
    if (MapSelForm->lvwFiles->Selected == 0) return;

    AnsiString chosen_map =
        ChangeFileExt(MapSelForm->lvwFiles->Selected->Caption, "");

    // This function should first check if the texture file is already included
    // in the model.  If it is, then simply select it:
    for (size_t i=0; i < Proj.mats.size(); i++)
    {
        if (Proj.mats[i].s.AnsiCompareIC(chosen_map) == 0)
        {
            // SIMULATE the user simply selecting a different file
            cboFaceMap->ItemIndex = (int)i;
            cboFaceMapChange(0);
            return;
        }
    }

    // Now, we must be adding a new map.  If this is the case, we must simply add
    // it to the map list and point the selected faces to it.
    // ADD the new map
    Proj.mats.push_back(ImgString());
    Proj.mats.back().s = chosen_map;
    // UPDATE the list
    ModeFaceExecute(0);
    // ASSIGN the map to the faces
    cboFaceMap->ItemIndex = cboFaceMap->Items->Count - 1;
    cboFaceMapChange(0);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::cboFaceMapChange(TObject *Sender)
{
    if (Updating) return;
    if (cboFaceMap->ItemIndex < 0) return;

    int i;
    unsigned int mesh, face;

    for (i=0; i<(int)Sels.size(); i++)
    {
        mesh = Sels[i] & 0xFFFF;
        face = Sels[i] >> 16;
        Proj.meshes[mesh].faces[face].texture = cboFaceMap->ItemIndex;
    }
    if ((int)Sels.find_sel(Sels.cur) < 0) // not on stack
    {
        mesh = Sels.cur & 0xFFFF;
        face = Sels.cur >> 16;
        Proj.meshes[mesh].faces[face].texture = cboFaceMap->ItemIndex;
    }

    if (RemoveUnusedMaps() == true)
    {
        // UPDATE the editor
        ModeFaceExecute(0);
        FillFaceInfo(Sels.count_sel());
    }

    Proj.mod = true;
    pnlView->Invalidate();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::btnMshCenterClick(TObject *Sender)
{
    // This process first centers the mesh and then adds to its offset the
    // negative of the amount by which it had to be moved to be centered.  Then
    // visit each child and add that value to their offset.
    // This is done because we are not just changing the offset, we are actually
    // changing the model's geometry and have to tell the children.
    // We only go one child deep (do not traverse the hierarchy) because it is
    // asssumed that the grandchildren will move appropriately wih their parents
    // (the children).
    int i, mesh;
    Vector off;

    // center all the permanent meshes
    for (i=0; i<(int)Sels.size(); i++)
    {
        mesh = (int)Sels[i];
        off = Proj.meshes[mesh].center();
        Proj.change_mesh_off(mesh, off);

        for (int j=0; j<(int)Proj.meshes.size(); j++)
        {
            if (Proj.meshes[j].parent == mesh)
                Proj.change_mesh_off(j, -off);
        }
    }

    // invert the current mesh if it is not on the stack
    if (Sels.find_sel(Sels.cur) < 0)
    {
        mesh = (int)Sels.cur;
        off = Proj.meshes[mesh].center();
        Proj.change_mesh_off(mesh, off);

        for (int j=0; j<(int)Proj.meshes.size(); j++)
        {
            if (Proj.meshes[j].parent == mesh)
                Proj.change_mesh_off(j, -off);
        }
    }

    Proj.mod = true;
    On_SelChanged();
    pnlView->Invalidate();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::tvwHierDragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
    Accept = Source->ClassNameIs("TTreeView");
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::tvwHierDragDrop(TObject *Sender,
      TObject *Source, int X, int Y)
{
    // This will make the dragging node the (most significant) child of the
    // dropped node.  Arcaine?  oui, oui!
    if (!Source->ClassNameIs("TTreeView")) return;
    TTreeView *tv = (TTreeView*)Sender;
    if (tv->Selected == NULL) return;

    TTreeNode *dragItem = tv->Selected;
    TTreeNode *dropItem = tv->GetNodeAt(X, Y);

    if (dropItem == NULL) return;

    // We do not allow the model to become a child of one of its meshes..neat
    // idea however...how is the model any differnt than just an empty mesh?
    if (int(dragItem->Data) < 0) return;

    // It is important not to mess things up here, we must match LODs and what not.
    size_t  IDrag = size_t(dragItem->Data);
    size_t  IDrop = size_t(dropItem->Data);
    Mesh   &MDrag = Proj.meshes[IDrag];
    Vector  ODrag = Proj.get_mesh_off(int(IDrag), -1);

    // First, check if nothing is being done
    if (IDrag == IDrop)
    {
        return;
    }
    if (MDrag.parent == int(IDrop))
    {
        return;
    }

    // Now we must remove this mesh if it was an LOD
    if ((MDrag.parent < 0) && (MDrag.type == MT_HULL))
    {
        Proj.nlods--;
        for (size_t m=0; m < Proj.meshes.size(); m++)
        {
            if (Proj.meshes[m].lod > MDrag.lod)
            {
                Proj.meshes[m].lod--;
            }
        }
    }

    // Next, check if it is promoted to an LOD
    if (IDrop == size_t(-1))
    {
        MDrag.parent = -1;
        MDrag.offset = ODrag;
        if (MDrag.type == MT_HULL)
        {
            MDrag.lod = Proj.nlods;
            Proj.nlods++;
        }
    }
    else
    {
        MDrag.parent = int(IDrop);
        MDrag.offset = ODrag - Proj.get_mesh_off(int(IDrop), -1);
        MDrag.type   = Proj.meshes[IDrop].type;
        MDrag.lod    = Proj.meshes[IDrop].lod;
    }

    Proj.mod = true;
    UpdateTree();
    tvwHierChange(0,0);
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::mnuHierMergeChildClick(TObject *Sender)
{
    if (ViewNode < 0) return;
    if (ViewNode >= (int)Proj.meshes.size()) return;

    TCursor Save_Cursor = Screen->Cursor;
    Screen->Cursor = crHourGlass;    // Show hourglass cursor
    try
    {

    Mesh &master = Proj.meshes[ViewNode];

    // add [m] to master
    size_t im = 0;
    while (im < Proj.meshes.size())
    {
        Mesh &m = Proj.meshes[im];

        // If this isn't a child, then skip it
        if (m.parent != ViewNode)
        {
            im++;
            continue;
        }

        // Add each face along with its verts to the master mesh
        vector<Face>::iterator f = m.faces.begin();
        for (; f != m.faces.end(); f++)
        {
            master.faces.push_back(*f);
            for (int j=0; j<(*f).nverts; j++)
            {
                Vector v = m.verts[(*f).verts[j]] + m.offset;
                master.faces.back().verts[j] = master.find_vert(v.x, v.y, v.z);
                if (master.faces.back().verts[j] == -1)
                {
                    master.faces.back().verts[j] = master.add_vert(v.x, v.y, v.z, false);
                }
            }
        }

        // The children of this child must now point to the master mesh
        for (size_t jm=0; jm < Proj.meshes.size(); jm++)
        {
            if (Proj.meshes[jm].parent == int(im))
            {
                Proj.meshes[jm].parent = ViewNode;
            }
        }

        // Now we can discard the mesh
        Proj.meshes.erase(Proj.meshes.begin() + im);
    }

    }
    __finally
    {
        Screen->Cursor = Save_Cursor; // always restore the cursor
    }

    Proj.mod = true;
    Sels.flush();
    UpdateTree();
    tvwHierChange(0,0);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ModeWeapExecute(TObject *Sender)
{
    if (ViewObjs != VO_WEAP)
    {
        ViewObjs = VO_WEAP;
        Sels.flush();
    }
    pnlView->PopupMenu = 0;
    nbkRight->ActivePage = "Weapons";
}
//---------------------------------------------------------------------------



void __fastcall TMainForm::PrintProjExecute(TObject *Sender)
{
    dlgPrint->Options.Clear();
    dlgPrint->Options << poSelection;
    dlgPrint->FromPage = 1;
    dlgPrint->MinPage = 1;
    dlgPrint->ToPage = 1;
    dlgPrint->MaxPage = 1;

    if (dlgPrint->Execute())
    {
        TPrinter *Prntr = Printer();
        Prntr->BeginDoc();

        Prntr->Canvas->Ellipse(0, 0, 200, 200);

        Prntr->EndDoc();
    }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::cbxShowShieldsClick(TObject *Sender)
{
    pnlView->Invalidate();    
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ModeWeapUpdate(TObject *Sender)
{
    ModeWeap->Checked = ViewObjs == VO_WEAP;    
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::btnMshScaleClick(TObject *Sender)
{
    std::vector<Mesh*> mshs;

    for (int i=0; i<(int)Sels.size(); i++)
    {
        mshs.push_back(&Proj.meshes[(int)Sels[i]]);
    }
    if ((Sels.find_sel(Sels.cur) < 0) && (int(Sels.cur) != -1))
    {
        mshs.push_back(&Proj.meshes[(int)Sels.cur]);
    }

    if (mshs.size() < 1) return;

    ScaleMeshForm->Initialize(mshs);
    if (ScaleMeshForm->ShowModal() != IDOK) return;

    // Now the dirty work.  Note, we will _not_ auto center the mesh.
    for (size_t i=0; i<mshs.size(); i++)
    {
        Vector scale = StrToVec(ScaleMeshForm->edtScaleFactor->Text);

        if (ScaleMeshForm->rgpScaleSys->ItemIndex == 1) // Absolute
        {
            Vector dim = mshs[i]->max - mshs[i]->min;
            scale.x /= dim.x;
            scale.y /= dim.y;
            scale.z /= dim.z;
        }
        if (scale.x == 0.0f) scale.x = 1.0f;
        if (scale.y == 0.0f) scale.y = 1.0f;
        if (scale.z == 0.0f) scale.z = 1.0f;

        for (size_t j=0; j<mshs[i]->verts.size(); j++)
        {
            mshs[i]->verts[j].x *= scale.x;
            mshs[i]->verts[j].y *= scale.y;
            mshs[i]->verts[j].z *= scale.z;
        }

        // Locate all the immediate children and scale their offsets.
        if (ScaleMeshForm->chkScaleOffs->Checked == true)
        {
            for (size_t nm=0; nm<Proj.meshes.size(); nm++)
            {
                if (Proj.meshes[nm].parent < 0) continue;
                if (&Proj.meshes[Proj.meshes[nm].parent] == mshs[i])
                {
                     Proj.meshes[nm].offset.x *= scale.x;
                     Proj.meshes[nm].offset.y *= scale.y;
                     Proj.meshes[nm].offset.z *= scale.z;
                }
            }
        }
    }

    Proj.mod = true;
}
//---------------------------------------------------------------------------
void TMainForm::Do_AlignGridToSelection()
{
    unsigned int single = Sels.get_single();

    // don't do anythign if nothing is selected
    if (single == (unsigned int)(-1)) return;

    unsigned int n;
    Mesh *msh;

    switch (ViewObjs)
    {
    case VO_MESH:
        //msh = &Proj.meshes[single & 0xFFFF];
        //MainGL.grid.SetPosition(msh->GetCenter());
        break;
    case VO_FACE:
        msh = &Proj.meshes[single & 0xFFFF];
        n   = single >> 16;
        MainGL.grid.SetPosition(msh->verts[msh->faces[n].verts[0]]);
        MainGL.grid.SetNormal(msh->faces[n].norm);
        MainGL.grid.SetRight(msh->verts[msh->faces[n].verts[1]] -
                             msh->verts[msh->faces[n].verts[0]]);
        break;
    case VO_VERT:
        msh = &Proj.meshes[single & 0xFFFF];
        n   = single >> 16;
        MainGL.grid.SetPosition(msh->verts[n]);
        MainGL.grid.SetNormal(msh->GetVertNormal(n));
        MainGL.grid.SetRight(msh->GetVertNormal(n).GetAlmostOrtho());
        break;
    case VO_WEAP:
        MainGL.grid.SetPosition(Proj.weapons[single].Pos);
        MainGL.grid.SetNormal(Proj.weapons[single].Normal);
        MainGL.grid.SetRight(Proj.weapons[single].Normal.GetAlmostOrtho());
        break;
    case VO_THRUST:
        break;
    case VO_SPCL:
        MainGL.grid.SetPosition(Proj.Specials[single].Pos);
        break;
    }

    pnlView->Invalidate();
}
//---------------------------------------------------------------------------
void TMainForm::Do_RepositionCamera(ECameraView NewView, bool AlignGrid)
{
    // Move the camera to wherever it should be
    switch (NewView)
    {
    case CV_Top:
        MainGL.CamTrans.x = MainGL.CamTrans.y = MainGL.CamTrans.z = 0.0f;
        MainGL.CamRot.y = MainGL.CamRot.z = 0.0f;
        MainGL.CamRot.x = -90.0f;
        break;
    case CV_Front:
        MainGL.CamTrans.x = MainGL.CamTrans.y = MainGL.CamTrans.z = 0.0f;
        MainGL.CamRot.x = MainGL.CamRot.z = 0.0f;
        MainGL.CamRot.y = 180.0f;
        break;
    case CV_Left:
        MainGL.CamTrans.x = MainGL.CamTrans.y = MainGL.CamTrans.z = 0.0f;
        MainGL.CamRot.x = MainGL.CamRot.z = 0.0f;
        MainGL.CamRot.y = -90.0f;
        break;
    case CV_Bottom:
        MainGL.CamTrans.x = MainGL.CamTrans.y = MainGL.CamTrans.z = 0.0f;
        MainGL.CamRot.y = MainGL.CamRot.z = 0.0f;
        MainGL.CamRot.x = 90.0f;
        break;
    case CV_Back:
        MainGL.CamTrans.x = MainGL.CamTrans.y = MainGL.CamTrans.z = 0.0f;
        MainGL.CamRot.x = MainGL.CamRot.y = MainGL.CamRot.z = 0.0f;
        break;
    case CV_Right:
        MainGL.CamTrans.x = MainGL.CamTrans.y = MainGL.CamTrans.z = 0.0f;
        MainGL.CamRot.x = MainGL.CamRot.z = 0.0f;
        MainGL.CamRot.y = 90.0f;
        break;
    case CV_Selection:
        break;
    };

    if (!AlignGrid) return;

    // Move the grid to be ligned up with the camera
    MainGL.grid.SetPosition(Vector());
    switch (NewView)
    {
    case CV_Top:
        MainGL.grid.SetNormal(Vector(0.0f, 1.0f, 0.0f));
        MainGL.grid.SetRight(Vector(0.0f, 0.0f, 1.0f));
        break;
    case CV_Front:
        MainGL.grid.SetNormal(Vector(0.0f, 0.0f, 1.0f));
        MainGL.grid.SetRight(Vector(1.0f, 0.0f, 0.0f));
        break;
    case CV_Left:
        MainGL.grid.SetNormal(Vector(-1.0f, 0.0f, 0.0f));
        MainGL.grid.SetRight(Vector(0.0f, -1.0f, 0.0f));
        break;
    case CV_Bottom:
        MainGL.grid.SetNormal(Vector(0.0f, -1.0f, 0.0f));
        MainGL.grid.SetRight(Vector(0.0f, 0.0f, -1.0f));
        break;
    case CV_Back:
        MainGL.grid.SetNormal(Vector(0.0f, 0.0f, -1.0f));
        MainGL.grid.SetRight(Vector(-1.0f, 0.0f, 0.0f));
        break;
    case CV_Right:
        MainGL.grid.SetNormal(Vector(1.0f, 0.0f, 0.0f));
        MainGL.grid.SetRight(Vector(0.0f, 1.0f, 0.0f));
        break;
    case CV_Selection:
        Do_AlignGridToSelection();
        break;
    };
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::rgrpWeapTypeClick(TObject *Sender)
{
    if (Updating) return;

    int i;

    int Val = rgrpWeapType->ItemIndex + 1;
    for (i=0; i<(int)Sels.size(); i++)
    {
        Proj.weapons[Sels[i]].Type = Val;
    }
    if ((int)Sels.find_sel(Sels.cur) < 0) // not on stack
    {
        Proj.weapons[Sels.cur].Type = Val;
    }

    pnlView->Invalidate();
    Proj.mod = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::edtWeapGroupChange(TObject *Sender)
{
    if (Updating) return;

    int i;

    int Val = StrToInt(edtWeapGroup->Text);
    for (i=0; i<(int)Sels.size(); i++)
    {
        Proj.weapons[Sels[i]].Group = Val;
    }
    if ((int)Sels.find_sel(Sels.cur) < 0) // not on stack
    {
        Proj.weapons[Sels.cur].Group = Val;
    }

    pnlView->Invalidate();
    Proj.mod = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::edtWeapIDChange(TObject *Sender)
{
    if (Updating) return;

    int i;

    int Val = StrToInt(edtWeapID->Text);
    for (i=0; i<(int)Sels.size(); i++)
    {
        Proj.weapons[Sels[i]].ID = Val;
    }
    if ((int)Sels.find_sel(Sels.cur) < 0) // not on stack
    {
        Proj.weapons[Sels.cur].ID = Val;
    }

    pnlView->Invalidate();
    Proj.mod = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::edtWeapPosChange(TObject *Sender)
{
    if (Updating) return;

    int i;

    Vector Val = StrToVec(edtWeapPos->Text);
    for (i=0; i<(int)Sels.size(); i++)
    {
        Proj.weapons[Sels[i]].Pos = Val;
    }
    if ((int)Sels.find_sel(Sels.cur) < 0) // not on stack
    {
        Proj.weapons[Sels.cur].Pos = Val;
    }

    pnlView->Invalidate();
    Proj.mod = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::edtWeapNormChange(TObject *Sender)
{
    if (Updating) return;

    int i;

    Vector Val = StrToVec(edtWeapNorm->Text);
    for (i=0; i<(int)Sels.size(); i++)
    {
        Proj.weapons[Sels[i]].Normal = Val;
    }
    if ((int)Sels.find_sel(Sels.cur) < 0) // not on stack
    {
        Proj.weapons[Sels.cur].Normal = Val;
    }

    pnlView->Invalidate();
    Proj.mod = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::pnlViewMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
    switch (MouseMode)
    {
    case MM_TRNSEL:
        SetMM(MM_SELECT);
        ReleaseCapture();
        On_SelChanged();
        break;
    }
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::btnFaceCreateWeapClick(TObject *Sender)
{
    unsigned int s = Sels.get_single();

    CreateWeapForm->Initialize(s&0xFFFF,s>>16);
    if (CreateWeapForm->ShowModal() == mrOk)
    {
        ModeWeapExecute(0);
    }
    pnlView->Invalidate();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ModeSpecialExecute(TObject *Sender)
{
    if (ViewObjs != VO_SPCL)
    {
        ViewObjs = VO_SPCL;
        Sels.flush();
    }
    pnlView->PopupMenu = 0;
    nbkRight->ActivePage = "Special";
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ModeSpecialUpdate(TObject *Sender)
{
    ModeSpecial->Checked = (ViewObjs == VO_SPCL);    
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mmoSpclPropsChange(TObject *Sender)
{
    if (Updating) return;

    int i;

    AnsiString text = "";
    for (i=0; i<mmoSpclProps->Lines->Count; i++)
    {
        if (mmoSpclProps->Lines->Strings[i].Length() < 1) continue;
        if (i != 0) text += "\n";
        text += mmoSpclProps->Lines->Strings[i];
    }

    for (i=0; i<(int)Sels.size(); i++)
    {
        Proj.Specials[Sels[i]].Props = text;
    }
    if ((int)Sels.find_sel(Sels.cur) < 0) // not on stack
    {
        Proj.Specials[Sels.cur].Props = text;
    }

    Proj.mod = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::edtSpclNameChange(TObject *Sender)
{
    if (Updating) return;

    int i;

    for (i=0; i<(int)Sels.size(); i++)
    {
        Proj.Specials[Sels[i]].Name = edtSpclName->Text;
    }
    if ((int)Sels.find_sel(Sels.cur) < 0) // not on stack
    {
        Proj.Specials[Sels.cur].Name = edtSpclName->Text;
    }

    Proj.mod = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::edtSpclPosChange(TObject *Sender)
{
    if (Updating) return;

    int i;

    Vector Val = StrToVec(edtSpclPos->Text);
    for (i=0; i<(int)Sels.size(); i++)
    {
        Proj.Specials[Sels[i]].Pos = Val;
    }
    if ((int)Sels.find_sel(Sels.cur) < 0) // not on stack
    {
        Proj.Specials[Sels.cur].Pos = Val;
    }

    pnlView->Invalidate();
    Proj.mod = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::edtSpclRadChange(TObject *Sender)
{
    if (Updating) return;

    int i;

    float Val = StrToFloat(edtSpclRad->Text);
    for (i=0; i<(int)Sels.size(); i++)
    {
        Proj.Specials[Sels[i]].Radius = Val;
    }
    if ((int(Sels.cur)>=0) && int(Sels.find_sel(Sels.cur)) < 0)
    {
        Proj.Specials[Sels.cur].Radius = Val;
    }

    pnlView->Invalidate();
    Proj.mod = true;
}
//---------------------------------------------------------------------------

void TMainForm::Do_InsertItem()
{
    size_t single;

    switch (ViewObjs)
    {
    case VO_SPCL:
        Proj.Specials.push_back(Special());
        Proj.Specials.back().Name   = "$untitled";
        Proj.Specials.back().Props  = "$none";
        Proj.Specials.back().Pos    = CurPos;
        Proj.Specials.back().Radius = 5.0;
        Sels.cur = Proj.Specials.size()-1;
        break;
    case VO_THRUST:
        // Add a glow to the currently selected thruster
        single = Sels.get_single();
        if (int(single) >= 0)
        {
            size_t t = single & 0xFFFF;
            size_t g = single >> 16;
            Proj.thrusters[t].glows.push_back(Proj.thrusters[t].glows[g]);
            Proj.thrusters[t].glows.back().pos = CurPos;
            Sels.cur = ((Proj.thrusters[t].glows.size()-1)<<16) | t;
        }
        else
        {
            ErrorMsg("In order to add a glow, you must first select the thruster to which it will be added.");
            return;
        }
        break;
    }

    pnlView->Invalidate();
    Proj.mod = true;
}
//---------------------------------------------------------------------------
static void DeleteSelectedSpecials()
{
    // Get a sorted list of the selection
    vector<size_t> del_list;
    for (size_t i=0; i<Sels.size(); i++)
        del_list.push_back(Sels[i]);
    if ((int(Sels.cur)>=0) && int(Sels.find_sel(Sels.cur)) < 0)
    {
        del_list.push_back(Sels.cur);
    }
    sort(del_list.begin(), del_list.end());

    for (size_t i=del_list.size(); i > 0; i--)
    {
        Proj.Specials.erase(Proj.Specials.begin()+del_list[i-1]);
    }

    Sels.flush();
}
static void DeleteSelectedWeapons()
{
    // Get a sorted list of the selection
    vector<size_t> del_list;
    for (size_t i=0; i<Sels.size(); i++)
        del_list.push_back(Sels[i]);
    if ((int(Sels.cur)>=0) && int(Sels.find_sel(Sels.cur)) < 0)
    {
        del_list.push_back(Sels.cur);
    }
    sort(del_list.begin(), del_list.end());

    for (size_t i=del_list.size(); i > 0; i--)
    {
        Proj.weapons.erase(Proj.weapons.begin()+del_list[i-1]);
    }

    Sels.flush();
}
static void DeleteSelectedThrusters()
{
    size_t single = Sels.get_single();
    if ((int(single) > 0) && (Sels.size() < 1))
    {
        size_t t = single & 0xFFFF;
        size_t g = single >> 16;
        Proj.thrusters[t].glows.erase(Proj.thrusters[t].glows.begin()+g);
        // Delete the thruster if there aren't anymore glows
        if (Proj.thrusters[t].glows.size() < 1)
        {
            Proj.thrusters.erase(Proj.thrusters.begin()+t);
        }
    }
    else
    {
        ErrorMsg("You may only delete one glow at a time.");
    }

    Sels.flush();
}
void TMainForm::Do_DeleteSelection()
{
    switch (ViewObjs)
    {
    case VO_WEAP:
        DeleteSelectedWeapons();
        break;
    case VO_SPCL:
        DeleteSelectedSpecials();
        break;
    case VO_THRUST:
        DeleteSelectedThrusters();
        break;
    }

    On_SelChanged();
    pnlView->Invalidate();
    Proj.mod = true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Import3DSExecute(TObject *Sender)
{
    dlgOpen->InitialDir = ProjDir;
    dlgOpen->Filter = "3D Studio Files (*.3ds)|*.3ds|All Files (*.*)|*.*";
    dlgOpen->FileName = "";
    if (!dlgOpen->Execute()) return;

    // If they are adding this node to root, then we must ask whether it is
    // hull or debris.  Otherwise, it inherits its type from its parent.
    if ((ViewNode == -1) || (ViewNode >= (int)Proj.meshes.size()))
    {
        static char quest[] = { "Does this 3DS represent a new level of detail?  If you "
            "answer No, then it will be added as a piece of debris." };
        if (::MessageBox(0, quest, AppTitle.c_str(), MB_YESNO|MB_ICONQUESTION) == IDYES)
        {
            if (Proj.AddMeshesFrom3DS(dlgOpen->FileName, -1, Proj.nlods, MT_HULL))
            {
                // The read function will increase Proj.nlods as necessary
            }
        }
        else
        {
            Proj.AddMeshesFrom3DS(dlgOpen->FileName, -1, 0, MT_DEBRIS);
        }
    }
    else
    {
        Proj.AddMeshesFrom3DS(dlgOpen->FileName, ViewNode,
            Proj.meshes[ViewNode].lod,
            Proj.meshes[ViewNode].type);
    }

    // Add a new level of detail
    Proj.mod = true;
    Proj.recalc_all();
    MainGL.ResetSize(true);

    PopulateTree();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::btnFaceReplaceMatClick(TObject *Sender)
{
    if (cboFaceMap->ItemIndex >= 0)
    {
        MapSelForm->OldFileName =
            cboFaceMap->Items->Strings[cboFaceMap->ItemIndex] + ".pcx";
    }
    else
    {
        MapSelForm->OldFileName = "";
    }
    if (MapSelForm->ShowModal() != IDOK) return;
    if (MapSelForm->lvwFiles->Selected == 0) return;

    // Get info about new map
    AnsiString chosen_map =
        ChangeFileExt(MapSelForm->lvwFiles->Selected->Caption, "");
    size_t MapIdx = Proj.FindMaterial(chosen_map.c_str());

    // Free the old image
    MainGL.Activate();
    if (Proj.mats[cboFaceMap->ItemIndex].image != 0)
        delete ((OGLTexture*)Proj.mats[cboFaceMap->ItemIndex].image);
    MainGL.Deactivate();
    Proj.mats[cboFaceMap->ItemIndex].image = 0;

    // Take necessary action
    if (int(MapIdx) == -1)
    {
        Proj.mats[cboFaceMap->ItemIndex].s = chosen_map;
    }
    else
    {
        // Point the faces to the new mat
        for (size_t m=0; m < Proj.meshes.size(); m++)
        {
            for (size_t f=0; f < Proj.meshes[m].faces.size(); f++)
            {
                if (Proj.meshes[m].faces[f].texture == cboFaceMap->ItemIndex)
                    Proj.meshes[m].faces[f].texture = int(MapIdx);
            }
        }
        RemoveUnusedMaps();
    }

    ModeFaceExecute(0);
    On_SelChanged();
    pnlView->Invalidate();
}
/*
Error: Out of memory.  Try closing down other applications, increasing your
virtual memory size, or installing more physical RAM.

File:C:\projects\freespace2\code\GlobalIncs\WinDebug.cpp
Line: 1121

Call stack:
------------------------------------------------------------------
------------------------------------------------------------------
*/
//---------------------------------------------------------------------------

void __fastcall TMainForm::edtVertValChange(TObject *Sender)
{
    if (Updating) return;

    int i;

    Vector Val = StrToVec(edtVertVal->Text);
    for (i=0; i<(int)Sels.size(); i++)
    {
        Proj.meshes[Sels[i]&0xFFFF].verts[Sels[i]>>16] = Val;
    }
    if ((int)Sels.find_sel(Sels.cur) < 0) // not on stack
    {
        Proj.meshes[Sels.cur&0xFFFF].verts[Sels.cur>>16] = Val;
    }

    pnlView->Invalidate();
    Proj.mod = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::btnNewThrustClick(TObject *Sender)
{
    Proj.thrusters.push_back(Thruster());
    Proj.thrusters.back().Props = "";

    Proj.thrusters.back().glows.push_back(Glow());
    Proj.thrusters.back().glows.back().pos    = MainGL.grid.GetPos();
    Proj.thrusters.back().glows.back().norm   = MainGL.grid.GetNormal();
    Proj.thrusters.back().glows.back().radius = 5.0f;

    Proj.mod = true;
    pnlView->Invalidate();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mmoThrustPropsChange(TObject *Sender)
{
    if (Updating) return;

    AnsiString Val = "";
    for (int i=0; i<mmoThrustProps->Lines->Count; i++)
    {
        if (mmoThrustProps->Lines->Strings[i].Length() < 1) continue;
        if (i != 0) Val += "\n";
        Val += mmoThrustProps->Lines->Strings[i];
    }

    for (size_t i=0; i<Sels.size(); i++)
    {
        Proj.thrusters[Sels[i]&0xFFFF].Props = Val;
    }
    if (int(Sels.find_sel(Sels.cur) < 0) && (int(Sels.cur) >= 0))
    {
        Proj.thrusters[Sels.cur&0xFFFF].Props = Val;
    }

    Proj.mod = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::edtGlowPosChange(TObject *Sender)
{
    if (Updating) return;

    Vector Val = StrToVec(edtGlowPos->Text);

    for (size_t i=0; i<Sels.size(); i++)
    {
        Proj.thrusters[Sels[i]&0xFFFF].glows[Sels[i]>>16].pos = Val;
    }
    if (int(Sels.find_sel(Sels.cur) < 0) && (int(Sels.cur) >= 0))
    {
        Proj.thrusters[Sels.cur&0xFFFF].glows[Sels.cur>>16].pos = Val;
    }

    pnlView->Invalidate();
    Proj.mod = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::edtGlowNormChange(TObject *Sender)
{
    if (Updating) return;

    Vector Val = StrToVec(edtGlowNorm->Text);

    for (size_t i=0; i<Sels.size(); i++)
    {
        Proj.thrusters[Sels[i]&0xFFFF].glows[Sels[i]>>16].norm = Val;
    }
    if (int(Sels.find_sel(Sels.cur) < 0) && (int(Sels.cur) >= 0))
    {
        Proj.thrusters[Sels.cur&0xFFFF].glows[Sels.cur>>16].norm = Val;
    }

    pnlView->Invalidate();
    Proj.mod = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::edtGlowRadChange(TObject *Sender)
{
    if (Updating) return;

    float Val = StrToFloat(edtGlowRad->Text);

    for (size_t i=0; i<Sels.size(); i++)
    {
        Proj.thrusters[Sels[i]&0xFFFF].glows[Sels[i]>>16].radius = Val;
    }
    if (int(Sels.find_sel(Sels.cur) < 0) && (int(Sels.cur) >= 0))
    {
        Proj.thrusters[Sels.cur&0xFFFF].glows[Sels.cur>>16].radius = Val;
    }

    pnlView->Invalidate();
    Proj.mod = true;
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::ImportCOBExecute(TObject *Sender)
{
    dlgOpen->InitialDir = ProjDir;
    dlgOpen->Filter = "trueSpace COB Files (*.cob)|*.cob|All Files (*.*)|*.*";
    dlgOpen->FileName = "";
    if (!dlgOpen->Execute()) return;

    // If they are adding this node to root, then we must ask whether it is
    // hull or debris.  Otherwise, it inherits its type from its parent.
    if ((ViewNode == -1) || (ViewNode >= (int)Proj.meshes.size()))
    {
        static char quest[] = { "Does this COB represent a new level of detail?  If you "
            "answer No, then it will be added as a piece of debris." };
        if (::MessageBox(0, quest, AppTitle.c_str(), MB_YESNO|MB_ICONQUESTION) == IDYES)
        {
            if (Proj.AddMeshesFromCOB(dlgOpen->FileName, -1, Proj.nlods, MT_HULL))
            {
                // The read function will increase Proj.nlods as necessary
            }
        }
        else
        {
            Proj.AddMeshesFromCOB(dlgOpen->FileName, -1, 0, MT_DEBRIS);
        }
    }
    else
    {
        Proj.AddMeshesFromCOB(dlgOpen->FileName, ViewNode,
            Proj.meshes[ViewNode].lod,
            Proj.meshes[ViewNode].type);
    }

    // Add a new level of detail
    Proj.mod = true;
    Proj.recalc_all();
    MainGL.ResetSize(true);

    PopulateTree();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mnuHierPromoteClick(TObject *Sender)
{
    if (ViewNode < 0) return;
    if (ViewNode >= (int)Proj.meshes.size()) return;

    Mesh &m = Proj.meshes[ViewNode];
    if (Proj.nlods < 2) return;
    if (m.parent >= 0) return;
    if (m.type != MT_HULL) return;
    if (m.lod == 0) return;

    // reset LOD index
    for (size_t i=0; i < Proj.meshes.size(); i++)
    {
        if ((Proj.meshes[i].type == MT_HULL) &&
            (Proj.meshes[i].parent == -1) &&
            (Proj.meshes[i].lod == (m.lod-1)))
        {
            Proj.meshes[i].lod++;
        }
    }

    m.lod--;

    Proj.mod = true;
    Sels.flush();
    UpdateTree();
    tvwHierChange(0,0);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mnuHierToggleClick(TObject *Sender)
{
    if (ViewNode < 0) return;
    if (ViewNode >= (int)Proj.meshes.size()) return;

    // This function only works with root nodes
    Mesh &m = Proj.meshes[ViewNode];
    if (m.parent >= 0) return;

    if (m.type == MT_HULL)
    {
        m.type = MT_DEBRIS;

        // reset LOD indexes
        for (size_t i=0; i < Proj.meshes.size(); i++)
        {
            if ((Proj.meshes[i].type == MT_HULL) &&
                (Proj.meshes[i].parent == -1) &&
                (Proj.meshes[i].lod >= m.lod))
            {
                Proj.meshes[i].lod--;
            }
        }

        m.lod = 0;
        Proj.nlods--;
    }
    else
    {
        m.type = MT_HULL;
        m.lod  = Proj.nlods;
        Proj.nlods++;
    }

    Proj.mod = true;
    Sels.flush();
    UpdateTree();
    tvwHierChange(0,0);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::mnuHierDeleteClick(TObject *Sender)
{
///    
}
//---------------------------------------------------------------------------

