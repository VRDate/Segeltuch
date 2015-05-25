//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("Segeltuch.res");
USERES("Additional.res");
USEFORM("F_Main.cpp", MainForm);
USEUNIT("Pof.cpp");
USEUNIT("convert_dxf.cpp");
USEUNIT("Global.cpp");
USEFORM("F_ImportDXF.cpp", DXFForm);
USEFORM("F_About.cpp", AboutForm);
USEFORM("F_VPView.cpp", VPForm);
USEUNIT("RenderGL.cpp");
USEUNIT("Geometry.cpp");
USEFORM("F_NewProj.cpp", NewProjForm);
USEUNIT("read_dxf.cpp");
USEUNIT("Draw.cpp");
USEFORM("F_Select.cpp", SelectForm);
USEUNIT("VPFiles.cpp");
USEUNIT("ImageFiles.cpp");
USEUNIT("convert_cob.cpp");
USEUNIT("MemBuf.cpp");
USEUNIT("make_pof.cpp");
USEFORM("F_Visible.cpp", VisForm);
USEFORM("F_MeshInfo.cpp", MeshInfoForm);
USEUNIT("convert_model.cpp");
USEFORM("F_Shield.cpp", ShieldForm);
USEFORM("F_ScaleMesh.cpp", ScaleMeshForm);
USEFORM("F_MapSel.cpp", MapSelForm);
USEFORM("F_CreateWeap.cpp", CreateWeapForm);
USEUNIT("read_3ds.cpp");
USEUNIT("read_cob.cpp");
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        Application->Initialize();
        Application->Title = "Segeltuch";
        Application->CreateForm(__classid(TMainForm), &MainForm);
        Application->CreateForm(__classid(TDXFForm), &DXFForm);
        Application->CreateForm(__classid(TAboutForm), &AboutForm);
        Application->CreateForm(__classid(TVPForm), &VPForm);
        Application->CreateForm(__classid(TNewProjForm), &NewProjForm);
        Application->CreateForm(__classid(TSelectForm), &SelectForm);
        Application->CreateForm(__classid(TVisForm), &VisForm);
        Application->CreateForm(__classid(TMeshInfoForm), &MeshInfoForm);
        Application->CreateForm(__classid(TShieldForm), &ShieldForm);
        Application->CreateForm(__classid(TScaleMeshForm), &ScaleMeshForm);
        Application->CreateForm(__classid(TMapSelForm), &MapSelForm);
        Application->CreateForm(__classid(TCreateWeapForm), &CreateWeapForm);
        Application->Run();
    }
    catch (Exception &exception)
    {
        Application->ShowException(&exception);
    }
    return 0;
}
//---------------------------------------------------------------------------
