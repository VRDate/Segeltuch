//---------------------------------------------------------------------------
#include <vector>
#include <vcl.h>
#pragma hdrstop

#include "Global.h"
#include "F_Main.h"
#include "Geometry.h"

#include "F_CreateWeap.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "DumbPanel"
#pragma resource "*.dfm"
TCreateWeapForm *CreateWeapForm;

static bool UpdatingData = false;

//---------------------------------------------------------------------------
__fastcall TCreateWeapForm::TCreateWeapForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void TCreateWeapForm::Initialize(size_t mesh, size_t face)
{
    UpdatingData = true;

    iMesh = mesh;
    iFace = face;

    edtCount->Text = IntToStr(1);
    rgrpPattern->ItemIndex = 0;
    rgrpType->ItemIndex = 0;
    rgrpDir->ItemIndex = 0;
    edtDir->Text = "<0, 0, 1>";
    edtGroup->Text = "0";

    ProjectFace();
    CreateWeaponPoints(StrToInt(edtCount->Text));

    Width = (MainForm->Width*5)/6;
    Height = (MainForm->Height*5)/6;

    float scalex = float(View->Width)/((FaceMax-FaceMin).x*1.5f);
    float scaley = float(View->Height)/((FaceMax-FaceMin).y*1.5f);
    ViewScale  = min(scalex,scaley);
    ViewOffset = -FaceCenter;

    UpdatingData = false;
}
//---------------------------------------------------------------------------
void __fastcall TCreateWeapForm::btnOKClick(TObject *Sender)
{
    // Figureout the defaults
    int    wGroup = StrToInt(edtGroup->Text);
    int    wType  = rgrpType->ItemIndex + 1;
    int    wID    = -1;
    Vector wDir;
    switch (rgrpDir->ItemIndex)
    {
    case 0: // Normal
        wDir = Proj.meshes[iMesh].faces[iFace].norm;
        break;
    case 1: // Anti-normal
        wDir = -Proj.meshes[iMesh].faces[iFace].norm;
        break;
    case 2: // Specific
        wDir = StrToVec(edtDir->Text);
        break;
    }
    // Make sure we do not overwrite any IDs
    for (size_t i=0; i < Proj.weapons.size(); i++)
    {
        if (Proj.weapons[i].Group != wGroup) continue;
        if (Proj.weapons[i].Type != wType) continue;
        if (Proj.weapons[i].ID > wID)
        {
            wID++;
        }
    }

    // Create each weapon where it is supposed to be
    for (size_t i=0; i < Points.size(); i++)
    {
        Proj.weapons.push_back(Weapon());
        Proj.weapons.back().Normal = wDir;
        Proj.weapons.back().Type   = wType;
        Proj.weapons.back().Group  = wGroup;
        Proj.weapons.back().ID     = ++wID;
        FaceTrans.UnProjectPoint(Points[i], Proj.weapons.back().Pos);
    }

    Proj.mod = true;
}
//---------------------------------------------------------------------------
void __fastcall TCreateWeapForm::edtCountChange(TObject *Sender)
{
    if (UpdatingData) return;

    int Count;
    if (edtCount->Text.Length() < 1)
        Count = 1;
    else
        Count = StrToInt(edtCount->Text);
    if (Count < 1)
        Count = 1;

    if (rgrpPattern->ItemIndex == 0)
    {
    }

    CreateWeaponPoints(Count);
    View->Invalidate();
}
//---------------------------------------------------------------------------

struct PFactor {
    unsigned int A, B;
    PFactor() : A(0), B(0)
    {}
    PFactor(unsigned int InA, unsigned int InB) : A(InA), B(InB)
    {}
};
static void GetFactors(unsigned int Num, vector<PFactor> &Factors)
{
    Factors.clear();
    unsigned int j;
    for (unsigned int i=1; i <= Num; i++)
    {
        if ((Num % i) == 0)
        {
            Factors.push_back(PFactor(i, Num/i));
        }
    }
}
static size_t MatchRatioToFactor(float Ratio, const vector<PFactor> &Factors)
{
    float  Dist;
    float  CloseDist = fabs(Ratio - float(Factors[0].A)/float(Factors[0].B));
    size_t CloseIdx  = 0;
    for (size_t i=1; i<Factors.size(); i++)
    {
        Dist = fabs(Ratio - float(Factors[i].A)/float(Factors[i].B));
        if (Dist < CloseDist)
        {
            CloseDist = Dist;
            CloseIdx  = i;
        }
    }
    return CloseIdx;
}
void TCreateWeapForm::CreateWeaponPoints(size_t Count)
{
    float  SpanX = (FaceMax-FaceMin).x;
    float  SpanY = (FaceMax-FaceMin).y;

    Points.clear();
    Points.insert(Points.begin(), Count, Vector());

    if (rgrpPattern->ItemIndex == 0) // Even distribution
    {
        if (Count == 1)
        {
            Points[0] = FaceCenter;
            return;
        }

        size_t EvenCount = Count & (~1);

        vector<PFactor> Factors;
        GetFactors((unsigned int)EvenCount, Factors);
        size_t Factor = MatchRatioToFactor(fabs(SpanX/SpanY), Factors);
        int    NumX   = Factors[Factor].A;
        int    NumY   = Factors[Factor].B;
        //MessageBox(0,(IntToStr(NumX)+" "+IntToStr(NumY)).c_str(),"",MB_OK);
        float  DeltaX = SpanX / float(NumX+1);
        float  DeltaY = SpanY / float(NumY+1);
        float  X = FaceMin.x+DeltaX, Y;

        for (int i=0; i < NumX; i++)
        {
            Y = FaceMin.y+DeltaY;
            for (int j=0; j < NumY; j++)
            {
                Points[i*NumY + j].x = X;
                Points[i*NumY + j].y = Y;
                Y += DeltaY;
            }
            X += DeltaX;
        }

        if (EvenCount < Count)
        {
            Points.back().x = X;
            Points.back().y = FaceMin.y+DeltaY;
        }
    }
}
//---------------------------------------------------------------------------

void TCreateWeapForm::ProjectFace()
{
    FacePoints.clear();

    Face &f = Proj.meshes[iMesh].faces[iFace];
    if (f.nverts < 3) return;
    FacePoints.insert(FacePoints.begin(), f.nverts, Vector());

    FaceTrans.Dir = -f.norm;
    FaceTrans.Dir.normalize();
    FaceTrans.Up  = Proj.meshes[iMesh].verts[f.verts[1]] -
                    Proj.meshes[iMesh].verts[f.verts[0]];
    FaceTrans.Up.normalize();
    Vector::cross(FaceTrans.Right, FaceTrans.Dir, FaceTrans.Up);
    FaceTrans.Right.normalize();
    FaceTrans.Pos = Proj.meshes[iMesh].verts[f.verts[0]] /* + Proj.get_mesh_off(iMesh,-1) */;

    FaceTrans.ProjectPoint(Proj.meshes[iMesh].verts[f.verts[0]], FacePoints[0]);
    FaceMin = FacePoints[0];
    FaceMax = FacePoints[0];
    for (int i=1; i < f.nverts; i++)
    {
        FaceTrans.ProjectPoint(Proj.meshes[iMesh].verts[f.verts[i]], FacePoints[i]);
        if (FacePoints[i].x > FaceMax.x) FaceMax.x = FacePoints[i].x;
        if (FacePoints[i].y > FaceMax.y) FaceMax.y = FacePoints[i].y;
        if (FacePoints[i].x < FaceMin.x) FaceMin.x = FacePoints[i].x;
        if (FacePoints[i].y < FaceMin.y) FaceMin.y = FacePoints[i].y;
    }

    FaceCenter = ((FaceMin+FaceMax)*0.5f);
}
//---------------------------------------------------------------------------
void TCreateWeapForm::ProjectPoint(const Vector &Pt, int &X, int &Y)
{
    X = ViewCenter.x + int((Pt.x+ViewOffset.x)*ViewScale + 0.5f);
    Y = ViewCenter.y - int((Pt.y+ViewOffset.y)*ViewScale + 0.5f);
}
void __fastcall TCreateWeapForm::ViewPaint(TObject *Sender)
{
    size_t i;
    int    x, y;
    char   Text[30];

    // Draw the face
    View->Canvas->Pen->Color = clWindowText;
    View->Canvas->Pen->Width = 2;
    ProjectPoint(FacePoints.back(), x, y);
    View->Canvas->MoveTo(x,y);
    for (i=0; i<FacePoints.size(); i++)
    {
        //sprintf(Text, "%d", int(i));
        ProjectPoint(FacePoints[i], x, y);
        View->Canvas->LineTo(x,y);
        //View->Canvas->TextOut(x,y,Text);
    }

    // Draw the weapons
    View->Canvas->Pen->Color = clBtnFace;
    View->Canvas->Brush->Color = clHighlight;
    View->Canvas->Pen->Width = 1;
    for (i=0; i<Points.size(); i++)
    {
        ProjectPoint(Points[i], x, y);
        View->Canvas->Ellipse(x-6,y-6,x+6,y+6);
    }
}
//---------------------------------------------------------------------------
void __fastcall TCreateWeapForm::Panel4Resize(TObject *Sender)
{
    ViewCenter.x = View->Width/2;
    ViewCenter.y = View->Height/2;    
}
//---------------------------------------------------------------------------


