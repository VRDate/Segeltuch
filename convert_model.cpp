//---------------------------------------------------------------------------
#include <vcl.h>
#include <math.h>
#include <stdio.h>
#pragma hdrstop

#include "Geometry.h"
#include "Global.h"
#include "F_Main.h"
#include "pof.h"

using namespace std;

static pof_mesh *meshes;

void add_mesh(pof_file *pof, int nm, int lod, char type)
{
    int nv,nf,k;
    Mesh *m;
    Face *f;
    Vector *v;

    m = Proj.meshes.insert(Proj.meshes.end(), Mesh());

    m->lod = lod;
    m->name = pof->objs[nm].submodel_name.s; m->name.Unique();
    m->props = pof->objs[nm].properties.s; m->props.Unique();
    m->parent = pof->objs[nm].submodel_parent;
    m->offset.x = pof->objs[nm].offset[0];
    m->offset.y = pof->objs[nm].offset[1];
    m->offset.z = pof->objs[nm].offset[2];
    m->type = type;

    for (nv=0; nv<(int)meshes[nm].verts.size(); nv++)
    {
        v = m->verts.insert(m->verts.end(), Vector());
        v->x = meshes[nm].verts[nv].pt[0];
        v->y = meshes[nm].verts[nv].pt[1];
        v->z = meshes[nm].verts[nv].pt[2];
    }

    for (nf=0; nf<(int)meshes[nm].faces.size(); nf++)
    {
        f = m->faces.insert(m->faces.end(), Face());
        f->texture = meshes[nm].faces[nf].tmap_num;
        f->nverts = meshes[nm].faces[nf].nverts;
        for (k=0; k<f->nverts; k++)
        {
            f->verts[k] = (int)meshes[nm].faces[nf].verts[k].num;
            f->texverts[k].u = meshes[nm].faces[nf].verts[k].u;
            f->texverts[k].v = meshes[nm].faces[nf].verts[k].v;

            //InfoMsg("<%.4f, %.4f>", f->texverts[k].u, f->texverts[k].v);
        }
    }

    // Add any children
    for (k=0; k<pof->ohdr.num_subobjects; k++)
    {
        if (pof->objs[k].submodel_parent == nm)
            add_mesh(pof, k, lod, type);
    }
}

bool convert_model(pof_file *pof)
{
    int nm, nf, nv, k;
    Mesh *m;
    Face *f;
    Vector *v;
    ImgString *s;
    Glow *g;
    Thruster *t;

    // Read the file and build meshes
    meshes = new pof_mesh[pof->ohdr.num_subobjects];
    for (nm=0; nm<pof->ohdr.num_subobjects; nm++)
    {
        meshes[nm].obj = &pof->objs[nm];
        parse_bsp(&pof->objs[nm].bsp_data[0], pof->objs[nm].bsp_data.size(), &meshes[nm]);
    }
    InfoMsg("Finished Interpretting POF.");

    // Put materials into the project
    for (nm=0; nm<int(pof->txtrs.mats.size()); nm++)
    {
        s = Proj.mats.insert(Proj.mats.end(), ImgString());
        (*s).s = pof->txtrs.mats[nm].s;
        (*s).s.Unique();
    }

    // Put Geometry into the project
    Proj.nlods = pof->ohdr.num_detaillevels;
    for (nm=0; nm<pof->ohdr.num_detaillevels; nm++)
    {
        add_mesh(pof, pof->ohdr.sobj_detaillevels[nm], nm, MT_HULL);
    }
    for (nm=0; nm<pof->ohdr.num_debris; nm++)
    {
        add_mesh(pof, pof->ohdr.sobj_debris[nm], -1, MT_DEBRIS);
    }
    delete[] meshes;

    // Thrusters
    for (nm=0; nm<(int)pof->thrusters.size(); nm++)
    {
        t = Proj.thrusters.insert(Proj.thrusters.end(), Thruster());
        (*t).Props = pof->thrusters[nm].properties.s;
        for (k=0; k<(int)pof->thrusters[nm].glows.size(); k++)
        {
            g = t->glows.insert(t->glows.end(), Glow());
            g->pos.x = pof->thrusters[nm].glows[k].pos[0];
            g->pos.y = pof->thrusters[nm].glows[k].pos[1];
            g->pos.z = pof->thrusters[nm].glows[k].pos[2];
            g->norm.x = pof->thrusters[nm].glows[k].norm[0];
            g->norm.y = pof->thrusters[nm].glows[k].norm[1];
            g->norm.z = pof->thrusters[nm].glows[k].norm[2];
            g->radius = pof->thrusters[nm].glows[k].radius;
        }
    }

    // Weapons
    Proj.weapons.clear();
    int wGunGrp = 0;
    int wMisGrp = 0;
    int wGroup;
    for (nm=0; nm<(int)pof->weapons.size(); nm++)
    {
        // Determine the group number
        if (pof->weapons[nm].type == WT_GUN)
            wGroup = wGunGrp++;
        else
            wGroup = wMisGrp++;
        for (k=0; k<(int)pof->weapons[nm].slots.size(); k++)
        {
            Proj.weapons.push_back(Weapon());
            Proj.weapons.back().Pos.x = pof->weapons[nm].slots[k].point.x;
            Proj.weapons.back().Pos.y = pof->weapons[nm].slots[k].point.y;
            Proj.weapons.back().Pos.z = pof->weapons[nm].slots[k].point.z;
            Proj.weapons.back().Normal.x = pof->weapons[nm].slots[k].norm.x;
            Proj.weapons.back().Normal.y = pof->weapons[nm].slots[k].norm.y;
            Proj.weapons.back().Normal.z = pof->weapons[nm].slots[k].norm.z;
            Proj.weapons.back().Type  = pof->weapons[nm].type;
            Proj.weapons.back().Group = wGroup;
            Proj.weapons.back().ID    = k;
        }
    }

    // Specials
    Proj.Specials.clear();
    Proj.Specials.insert(Proj.Specials.begin(), pof->specials.size(), Special());
    for (nm=0; nm < (int)pof->specials.size(); nm++)
    {
        Proj.Specials[nm].Name   = pof->specials[nm].name.s;
        Proj.Specials[nm].Props  = pof->specials[nm].properties.s;
        Proj.Specials[nm].Pos.x  = pof->specials[nm].point.x;
        Proj.Specials[nm].Pos.y  = pof->specials[nm].point.y;
        Proj.Specials[nm].Pos.z  = pof->specials[nm].point.z;
        Proj.Specials[nm].Radius = pof->specials[nm].radius;
    }

    // Shields
    for (nv=0; nv<(int)pof->shlds.verts.size(); nv++)
    {
        Vector &w = *Proj.shields.verts.insert(Proj.shields.verts.end(), Vector());
        w.x = pof->shlds.verts[nv].x;
        w.y = pof->shlds.verts[nv].y;
        w.z = pof->shlds.verts[nv].z;
    }

    for (nf=0; nf<(int)pof->shlds.faces.size(); nf++)
    {
        Face &g = *Proj.shields.faces.insert(Proj.shields.faces.end(), Face());
        g.texture = 0;
        g.nverts = 3;
        g.norm.x = pof->shlds.faces[nf].normal.x;
        g.norm.y = pof->shlds.faces[nf].normal.y;
        g.norm.z = pof->shlds.faces[nf].normal.z;
        for (k=0; k<3; k++)
        {
            g.verts[k] = (int)pof->shlds.faces[nf].verts[k].num;
        }

        // Diplay info
        //InfoMsg("(<%g, %g, %g>, <%g, %g, %g>, <%g, %g, %g>)*<%g, %g, %g>",
        //    Proj.shields.verts[g.verts[0]].x,Proj.shields.verts[g.verts[0]].y,Proj.shields.verts[g.verts[0]].z,
        //    Proj.shields.verts[g.verts[1]].x,Proj.shields.verts[g.verts[1]].y,Proj.shields.verts[g.verts[1]].z,
        //    Proj.shields.verts[g.verts[2]].x,Proj.shields.verts[g.verts[2]].y,Proj.shields.verts[g.verts[2]].z,
        //    g.norm.x, g.norm.y, g.norm.z);
    }

    return true;
}


//---------------------------------------------------------------------------

#pragma package(smart_init)
