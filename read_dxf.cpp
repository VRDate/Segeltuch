#include <vcl.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <fstream>
#pragma hdrstop

#include "Global.h"
#include "Geometry.h"
#include "F_Main.h"
#include "F_Select.h"

using namespace std;

bool Model::ReadDXF(AnsiString fn, int lod, char typ)
{
    ifstream file(fn.c_str());
    if (file.bad())
    {
        ErrorMsg("Unable to gain an input stream to %s", fn.c_str());
        return false;
    }

    char buf[256];
    char mname[64];
    int nmesh;
    int j;
    float v[3];

    // Later, we will hav to picka parent for all these
    // meshes, so we need to keep a list of all new ones
    // NOT ANYMORE
    TStringList *new_meshes = new TStringList;

    while (!file.getline(buf,255).rdstate())
    {
        if (buf[0] == 0) continue;
        char* p = strtok(buf, " ");
        if (p==NULL) continue;

        if (stricmp(p, "3dface") == 0)
        {
            // eat the " 8"
            file.getline(buf,255);

            // get the mesh to which we add this face
            file.getline(buf,255);
            memset(mname, 0, 64);
            strcpy(mname, strtok(buf, " "));
            //InfoMsg(mname);
            nmesh = find_mesh(mname, lod);
            if (nmesh < 0)
            {
                nmesh = add_mesh(mname, lod, typ);
                meshes[nmesh].offset.x = 0.0f;
                meshes[nmesh].offset.y = 0.0f;
                meshes[nmesh].offset.z = 0.0f;
                meshes[nmesh].parent = ViewNode;

                new_meshes->AddObject(mname, (TObject*)nmesh);
            }

            // create the face and load it
            Face *f = meshes[nmesh].faces.insert(meshes[nmesh].faces.end(), Face());
            f->nverts = 4;
            f->texture = 0;
            for (j=0; j<4; j++)
            {
                file.getline(buf,255); // eat the id
                file.getline(buf,255); // value
                v[0] = atof(strtok(buf, " "));
                file.getline(buf,255); // eat the id
                file.getline(buf,255); // value
                v[1] = atof(strtok(buf, " "));
                file.getline(buf,255); // eat the id
                file.getline(buf,255); // value
                v[2] = atof(strtok(buf, " "));

                f->verts[j] = meshes[nmesh].find_vert(v[0], v[1], v[2]);
                if (f->verts[j] < 0)
                    f->verts[j] = meshes[nmesh].add_vert(v[0], v[1], v[2], false);
            }
            if (f->verts[2] == f->verts[3]) f->nverts = 3;
        }
    }

    // Assign some default face texture vertices
    for (nmesh=0; nmesh<new_meshes->Count; nmesh++)
    {
        for (j=0; j<(int)meshes[(int)new_meshes->Objects[nmesh]].faces.size(); j++)
            meshes[(int)new_meshes->Objects[nmesh]].calc_face_text(j);
    }

    // Now we have to establish the hierarchy.  Bring up an interactive display
    /*SelectForm->Caption = "Select Parent Mesh";
    SelectForm->lblText->Caption = "Meshes found in DXF file:";
    SelectForm->btnCancel->Visible = false;
    SelectForm->lstList->Items->Clear();
    for (j=0; j<new_meshes->Count; j++)
    {
        SelectForm->lstList->Items->Add(new_meshes->Strings[j]);
    }
    SelectForm->lstList->ItemIndex = 0;
    SelectForm->ShowModal();
    int par_itm = (int)new_meshes->Objects[SelectForm->lstList->ItemIndex];
    for (j=0; j<new_meshes->Count; j++)
    {
        if ((int)new_meshes->Objects[j] == par_itm) continue;
        meshes[(int)new_meshes->Objects[j]].parent = par_itm;
    }*/

    delete new_meshes;

    return true;
}

bool Model::AddMeshFromDXF(const AnsiString &fn, int parent, int lod, char type)
{
    ifstream file(fn.c_str());
    if (file.bad())
    {
        ErrorMsg("Unable to gain an input stream to %s", fn.c_str());
        return false;
    }

    AnsiString last_mesh_name;
    static char buf[256];
    float v[3];

    // Some will say it is simplistic to just merge all the meshes together, but
    // that process is not a result of coding simplicity.  I decided it will be
    // easier on the user if there are simple rules that are followed.  One of
    // these rules is that when using this function, only one mesh should be
    // created.  In the future, it may be more functional if we allow the user to
    // selected which of the meshes to merge (and henceforth) which to totally
    // disregard.
    meshes.push_back(Mesh());
    meshes.back().name    = "";
    meshes.back().parent  = parent;
    meshes.back().lod     = lod;
    meshes.back().type    = type;
    meshes.back().offset *= 0.0f;
    int nmesh = (int)(meshes.size()-1);

    // An idiot simple tokenizer.  Does it even deserve that title?
    while (!file.getline(buf,255).rdstate())
    {
        if (buf[0] == 0) continue;
        char* p = strtok(buf, " ");
        if (p == 0) continue;

        if (stricmp(p, "3dface") == 0)
        {
            // eat the " 8"
            file.getline(buf,255);

            // get the mesh name to which we add this face
            file.getline(buf,255);
            last_mesh_name = strtok(buf, " ");

            // create the face and load it
            meshes[nmesh].faces.push_back(Face());
            Face &f = meshes[nmesh].faces.back();
            f.nverts = 4;
            f.texture = 0;
            for (int j=0; j<4; j++)
            {
                file.getline(buf,255); // eat the id
                file.getline(buf,255); // value
                v[0] = atof(strtok(buf, " "));
                file.getline(buf,255); // eat the id
                file.getline(buf,255); // value
                v[1] = atof(strtok(buf, " "));
                file.getline(buf,255); // eat the id
                file.getline(buf,255); // value
                v[2] = atof(strtok(buf, " "));

                f.verts[j] = meshes[nmesh].find_vert(v[0], v[1], v[2]);
                if (f.verts[j] < 0)
                    f.verts[j] = meshes[nmesh].add_vert(v[0], v[1], v[2], false);
            }
            // handle triangles
            if (f.verts[2] == f.verts[3]) f.nverts = 3;
        }
    }

    // Calculate all that good stuff
    meshes[nmesh].calc_face_norms();
    meshes[nmesh].get_minmax();

    // Assign some default face texture vertices
    for (int j=0; j<(int)meshes[nmesh].faces.size(); j++)
    {
        meshes[nmesh].calc_face_text(j);
    }

    // What's in a name??
    meshes[nmesh].name = last_mesh_name;
    if (meshes[nmesh].name.Length() < 1)
        meshes[nmesh].name = "unnamed";

    return true;
}


bool Mesh::LoadFromDXF(const AnsiString &fn, bool NewName)
{
    ifstream file(fn.c_str());
    if (file.rdstate())
    {
        ErrorMsg("Unable to gain an input stream to %s", fn.c_str());
        return false;
    }

    AnsiString last_mesh_name;
    static char buf[256];
    float v[3];

    // An idiot simple tokenizer.  Does it even deserve that title?
    while (!file.getline(buf,255).eof())
    {
        if (buf[0] == 0) continue;
        char* p = strtok(buf, " ");
        if (p == 0) continue;

        if (stricmp(p, "3dface") == 0)
        {
            // eat the " 8"
            file.getline(buf,255);

            // get the mesh name to which we add this face
            file.getline(buf,255);
            last_mesh_name = strtok(buf, " ");

            // create the face and load it
            faces.push_back(Face());
            Face &f = faces.back();
            f.nverts = 4;
            f.texture = 0;
            for (int j=0; j<4; j++)
            {
                file.getline(buf,255); // eat the id
                file.getline(buf,255); // value
                v[0] = atof(strtok(buf, " "));
                file.getline(buf,255); // eat the id
                file.getline(buf,255); // value
                v[1] = atof(strtok(buf, " "));
                file.getline(buf,255); // eat the id
                file.getline(buf,255); // value
                v[2] = atof(strtok(buf, " "));

                f.verts[j] = find_vert(v[0], v[1], v[2]);
                if (f.verts[j] < 0)
                    f.verts[j] = add_vert(v[0], v[1], v[2], false);
            }
            // handle triangles
            if (f.verts[2] == f.verts[3]) f.nverts = 3;
        }
    }

    // Calculate all that good stuff
    calc_face_norms();
    get_minmax();

    // Assign some default face texture vertices
    for (int j=0; j<(int)faces.size(); j++)
    {
        calc_face_text(j);
    }

    // What's in a name??
    if (NewName)
    {
        if (last_mesh_name.Length() < 1)
            name = "unnamed";
        else
            name = last_mesh_name;
    }

    return true;
}

