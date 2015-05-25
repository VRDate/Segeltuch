//---------------------------------------------------------------------------
#include <vcl.h>
#include <vector>
#include <cmath>
#pragma hdrstop

#include "Global.h"
#include "pof.h"
#include "F_ImportDXF.h"
#include "F_Main.h"
#include "MemBuf.h"

using namespace std;

struct CobVert
{
    float x,y,z;
};

struct CobFace
{
    int mat;
    int nverts;
    int verts[POF_MAXPOLYVERTS];
    int texverts[POF_MAXPOLYVERTS];
};


// A couple globals
int cur_id;
// We are saving as one COB object, so we must create
// one huge list of vertices, texverts, faces, and mats
vector<AnsiString> mats;
vector<CobFace> faces;
vector<CobVert> verts;
vector<CobVert> texverts;

int cob_add_mat(AnsiString name)
{
    unsigned int i;
    int n = -1;

    for (i=0; i<mats.size(); i++)
    {
        if (mats[i] == name)
            n = (int)i;
    }

    if (n >= 0) return n;

    mats.push_back(name);
    return ((int)mats.size() - 1);
}

int cob_add_vert(vector<CobVert> *list, CobVert v)
{
    unsigned int i;
    int n = -1;

    for (i=0; i<list->size(); i++)
    {
        if ((fabs(v.x-list->at(i).x) < 0.000001) &&
            (fabs(v.y-list->at(i).y) < 0.000001) &&
            (fabs(v.z-list->at(i).z) < 0.000001))
            n = (int)i;
    }

    // found alike vert, no need to add another
    if (n >= 0) return n;

    // add it and return its index
    list->push_back(v);
    return ((int)list->size() - 1);
}

void cob_add_mesh(int nm, pof_mesh *meshes, pof_file *pof)
{
    int j, k;
    pof_mesh *m = &meshes[nm];

    CobFace f;
    CobVert v;
    AnsiString mat;

    // add each face, meanwhile adding other stuff
    for (j=0; j<(int)m->faces.size(); j++)
    {
        f.mat = cob_add_mat(pof->txtrs.mats[m->faces[j].tmap_num].s);
        f.nverts = m->faces[j].nverts;

        for (k=0; k<f.nverts; k++)
        {
            // add this geo vert
            v.x = m->verts[m->faces[j].verts[k].num].pt[0];
            v.y = m->verts[m->faces[j].verts[k].num].pt[1];
            v.z = m->verts[m->faces[j].verts[k].num].pt[2];
            f.verts[k] = cob_add_vert(&verts, v);

            // add this texture vert
            v.x = m->faces[j].verts[k].u;
            v.y = 1.0-m->faces[j].verts[k].v;
            //ErrorMsg("<%.4f, %.4f>", v.x,v.y);
            v.z = 0.0f;
            f.texverts[k] = cob_add_vert(&texverts, v);
        }

        faces.push_back(f);
    }

    // add its children also
    for (j=0; j<pof->ohdr.num_subobjects; j++)
    {
        if (pof->objs[j].submodel_parent == nm)
            cob_add_mesh(j, meshes, pof);
    }
}

void cob_offset_mesh(int nm, pof_mesh *meshes, pof_file *pof, const pof_vector off)
{
    int j;
    pof_vector newoff;

    newoff[0] = off.x + pof->objs[nm].offset.x;
    newoff[1] = off.y + pof->objs[nm].offset.y;
    newoff[2] = off.z + pof->objs[nm].offset.z;

    for (j=0; j<(int)meshes[nm].verts.size(); j++)
    {
        meshes[nm].verts[j].pt[0] += newoff[0];
        meshes[nm].verts[j].pt[1] += newoff[1];
        meshes[nm].verts[j].pt[2] += newoff[2];
    }

    // offset its children also
    for (j=0; j<pof->ohdr.num_subobjects; j++)
    {
        if (pof->objs[j].submodel_parent == nm)
            cob_offset_mesh(j, meshes, pof, newoff);
    }
}

void cob_output(FILE *ofile)
{
    unsigned int j;
    int k;
    MemBuf buf(0x100000); // allocate 1mb

    // Write all data to the buffer then print the header and data...
    // header
    buf.printf("\nName FreeSpaceModel");
    buf.printf("\ncenter 0 0 0");
    buf.printf("\nx axis 1 0 0");
    buf.printf("\ny axis 0 1 0");
    buf.printf("\nz axis 0 0 1");
    buf.printf("\nTransform");
    buf.printf("\n1 0 0 0");
    buf.printf("\n0 1 0 0");
    buf.printf("\n0 0 1 0");
    buf.printf("\n0 0 0 1");
    // vertices
    buf.printf("\nWorld Vertices %u", verts.size());
    for (j=0; j<verts.size(); j++)
    {
        buf.printf("\n%f %f %f", verts[j].x, verts[j].y, verts[j].z);
    }
    // texture vertices
    buf.printf("\nTexture Vertices %u", texverts.size());
    for (j=0; j<texverts.size(); j++)
    {
        buf.printf("\n%f %f", texverts[j].x, texverts[j].y);
    }
    // faces
    buf.printf("\nFaces %u", faces.size());
    for (j=0; j<faces.size(); j++)
    {
        buf.printf("\nFace verts %d flags %d mat %d\n",
            faces[j].nverts, 0, faces[j].mat);
        for (k=0; k<faces[j].nverts; k++)
        {
            buf.printf("<%d,%d> ",
                faces[j].verts[k], faces[j].texverts[k]);
        }
    }
    buf.printf("\n"); //end

    // object header
    int par_id = cur_id;
    fprintf(ofile, "PolH V0.04 Id %d Parent %d Size %u",
        cur_id, 0, buf.tell());
    cur_id++;
    buf.dump(ofile);

    // units
    buf.reset();
    buf.printf("\nUnits 2\n");
    fprintf(ofile, "Unit V0.01 Id %d Parent %d Size %u",
        cur_id, par_id, buf.tell());
    cur_id++;
    buf.dump(ofile);

    // materials
    for (j=0; j<mats.size(); j++)
    {
        buf.reset();
        buf.printf("\nmat# %d", j);
        buf.printf("\nshader: phong  facet: auto40");
        buf.printf("\nrgb 0.407843,0.631373,0.870588");
        buf.printf("\nalpha 1  ka 0.39  ks 0.43  exp 0.43  ior 1");
        buf.printf("\ntexture: %s", (MapsDir + mats[j] + ".bmp").c_str());
        buf.printf("\noffset 0,0  repeats 1,1  flags 1\n");
        fprintf(ofile, "Mat1 V0.05 Id %d Parent %d Size %u",
            cur_id, par_id, buf.tell());
        cur_id++;
        buf.dump(ofile);
    }
}

bool convert_cob(pof_file *pof, char *filename)
{
    int i;

    FILE *ofile = fopen(filename, "wb");
    if (!ofile)
    {
        ErrorMsg("Could not open '%s' for writing.", filename);
        return false;
    }

    // Read the file and build meshes
    pof_mesh *meshes = new pof_mesh[pof->ohdr.num_subobjects];
    for (i=0; i<pof->ohdr.num_subobjects; i++)
    {
        meshes[i].obj = &pof->objs[i];
        parse_bsp(&pof->objs[i].bsp_data[0], pof->objs[i].bsp_data.size(), &meshes[i]);
    }
    InfoMsg("Finished Interpretting POF.");

    // Figureout excatly what the user wants to output
    DXFForm->Reset();
    DXFForm->FillList(pof);
    DXFForm->Caption = "COB Conversion Options";
    DXFForm->ShowModal();

    // ASCII COB header
    fprintf(ofile, "Caligari V00.01ALH             \n");

    cur_id = 13841364;
    faces.clear();
    verts.clear();
    texverts.clear();
    mats.clear();

    // Write the mesh groups to the master list
    if (DXFForm->lvwObjs->SelCount > 0)
    {
        int o;
        TListItem *item = DXFForm->lvwObjs->Selected;
        pof_vector v0 = {0.0f, 0.0f, 0.0f};
        TItemStates sel;
        sel << isSelected;

        o = (int)item->Data;
        cob_offset_mesh(o, meshes, pof, v0);
        cob_add_mesh(o, meshes, pof);

        for (i=1; i<DXFForm->lvwObjs->SelCount; i++)
        {
            item = DXFForm->lvwObjs->GetNextItem(item, sdAll, sel);
            o = (int)item->Data;
            cob_offset_mesh(o, meshes, pof, v0);
            cob_add_mesh(o, meshes, pof);
        }
    }

    // Kill the meshes
    delete[] meshes;

    // Write the file
    cob_output(ofile);

    // End the file
    fprintf(ofile, "END  V1.00 Id 0 Parent 0 Size        0");
    fclose(ofile);

    InfoMsg("Finished Writing '%s'.", filename);

    return true;
}

//---------------------------------------------------------------------------
#pragma package(smart_init)
