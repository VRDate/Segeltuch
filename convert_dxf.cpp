#include <vcl.h>
#include <stdio.h>
#include <string.h>
#pragma hdrstop

#include "Global.h"
#include "pof.h"
#include "F_ImportDXF.h"
#include "F_Main.h"

#define _ws(s,f) fwrite(s, 1, strlen(s), f)
void dxf_write_face(FILE *file, pof_mesh *msh, int n, const char *name)
{
    int i, c;
    char str[256];
    pof_tpoly *f = &msh->faces[n];

    // DXFs only support 3 and 4 vert faces so we must break up
    // this face into those components

    int num4 = (f->nverts - 2)/2;
    int num3 = (f->nverts & 1);

    // Write the quads
    for (i=0; i<num4; i++)
    {
        sprintf(str, "3DFACE\n 8\n%s\n", name);
        _ws(str, file);

        c = i*2+1;
        sprintf(str, "10\n%.6f\n20\n%.6f\n30\n%.6f\n",
           msh->verts[f->verts[c].num].pt[0],
           msh->verts[f->verts[c].num].pt[1],
           msh->verts[f->verts[c].num].pt[2]);
        _ws(str, file);
        sprintf(str, "11\n%.6f\n21\n%.6f\n31\n%.6f\n",
           msh->verts[f->verts[c+1].num].pt[0],
           msh->verts[f->verts[c+1].num].pt[1],
           msh->verts[f->verts[c+1].num].pt[2]);
        _ws(str, file);
        sprintf(str, "12\n%.6f\n22\n%.6f\n32\n%.6f\n",
           msh->verts[f->verts[c+2].num].pt[0],
           msh->verts[f->verts[c+2].num].pt[1],
           msh->verts[f->verts[c+2].num].pt[2]);
        _ws(str, file);
        sprintf(str, "13\n%.6f\n23\n%.6f\n33\n%.6f\n",
           msh->verts[f->verts[0].num].pt[0],
           msh->verts[f->verts[0].num].pt[1],
           msh->verts[f->verts[0].num].pt[2]);
        _ws(str, file);

        sprintf(str, "0\n");
        _ws(str, file);
    }

    // Write the tris
    if (num3)
    {
        sprintf(str, "3DFACE\n 8\n%s\n", name);
        _ws(str, file);

        c = num4*2+1;
        sprintf(str, "10\n%.6f\n20\n%.6f\n30\n%.6f\n",
           msh->verts[f->verts[c].num].pt[0],
           msh->verts[f->verts[c].num].pt[1],
           msh->verts[f->verts[c].num].pt[2]);
        _ws(str, file);
        sprintf(str, "11\n%.6f\n21\n%.6f\n31\n%.6f\n",
           msh->verts[f->verts[c+1].num].pt[0],
           msh->verts[f->verts[c+1].num].pt[1],
           msh->verts[f->verts[c+1].num].pt[2]);
        _ws(str, file);
        sprintf(str, "12\n%.6f\n22\n%.6f\n32\n%.6f\n",
           msh->verts[f->verts[0].num].pt[0],
           msh->verts[f->verts[0].num].pt[1],
           msh->verts[f->verts[0].num].pt[2]);
        _ws(str, file);
        sprintf(str, "13\n%.6f\n23\n%.6f\n33\n%.6f\n",
           msh->verts[f->verts[0].num].pt[0],
           msh->verts[f->verts[0].num].pt[1],
           msh->verts[f->verts[0].num].pt[2]);
        _ws(str, file);

        sprintf(str, "0\n");
        _ws(str, file);
    }
}

void dxf_output_mesh(FILE *file, int nm, pof_mesh *meshes, pof_file *pof)
{
    int j;

    for (j=0; j<(int)meshes[nm].faces.size(); j++)
    {
        dxf_write_face(file, &meshes[nm], j, pof->objs[nm].submodel_name.s);
    }

    // output its children also
    for (j=0; j<pof->ohdr.num_subobjects; j++)
    {
        if (pof->objs[j].submodel_parent == nm)
            dxf_output_mesh(file, j, meshes, pof);
    }
}

void dxf_offset_mesh(int nm, pof_mesh *meshes, pof_file *pof, const pof_vector off)
{
    int j;
    pof_vector newoff;

    newoff.x = off.x + pof->objs[nm].offset[0];
    newoff.y = off.y + pof->objs[nm].offset[1];
    newoff.z = off.z + pof->objs[nm].offset[2];

    for (j=0; j<(int)meshes[nm].verts.size(); j++)
    {
        meshes[nm].verts[j].pt[0] += newoff[0];
        meshes[nm].verts[j].pt[1] += newoff[1];
        meshes[nm].verts[j].pt[2] += newoff[2];
    }

    // output its children also
    for (j=0; j<pof->ohdr.num_subobjects; j++)
    {
        if (pof->objs[j].submodel_parent == nm)
            dxf_offset_mesh(j, meshes, pof, newoff);
    }
}

bool convert_dxf(pof_file *pof, char *filename)
{
    int i;

    FILE *ofile = fopen(filename, "w");
    if (ofile == NULL)
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

    char buf[256];

    // Introduce the file
    strcpy(buf, "0\nSECTION\n2\nENTITIES\n0\n");
    _ws(buf, ofile);

    // Figureout excatly what the user wants to output
    DXFForm->Reset();
    DXFForm->FillList(pof);
    DXFForm->Caption = "DXF Conversion Options";
    DXFForm->ShowModal();

    // Output the meshes
    if (DXFForm->lvwObjs->SelCount > 0)
    {
        int o;
        TListItem *item = DXFForm->lvwObjs->Selected;
        pof_vector v0 = {0.0f, 0.0f, 0.0f};
        TItemStates sel;
        sel << isSelected;

        o = (int)item->Data;
        dxf_offset_mesh(o, meshes, pof, v0);
        dxf_output_mesh(ofile, o, meshes, pof);

        for (i=1; i<DXFForm->lvwObjs->SelCount; i++)
        {
            item = DXFForm->lvwObjs->GetNextItem(item, sdAll, sel);
            o = (int)item->Data;
            dxf_offset_mesh(o, meshes, pof, v0);
            dxf_output_mesh(ofile, o, meshes, pof);
        }
    }

    // Kill the meshes
    delete[] meshes;

    // End the file
    strcpy(buf, "ENDSEC\n 0\nEOF\n");
    _ws(buf, ofile);
    fclose(ofile);

    InfoMsg("Finished Writing '%s'.", filename);

    return true;
}
#undef _ws