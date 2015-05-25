//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "Global.h"
#include "Pof.h"
#include "Geometry.h"
#include "F_Main.h"

#define _rd(o,f) fread(&o, sizeof o, 1, f)

bool pof_string::read(FILE *file)
{
    _rd(length, file);

    if (length >= POF_MAXSTRING)
        ErrorMsg("String too long in POF file.");

    fread(s, 1, length, file);
    s[length] = '\0';

    return true;
}

bool pof_fileheader::read(FILE *file)
{
    _rd(file_id, file);
    _rd(version, file);

    return true;
}

pof_objectheader::pof_objectheader()
{
    num_detaillevels = 0;
    num_debris = 0;
    num_cross_sections = 0;
}

pof_objectheader::~pof_objectheader()
{
    if (num_detaillevels > 0)
        delete[] sobj_detaillevels;
    if (num_debris > 0)
        delete[] sobj_debris;
    if (num_cross_sections > 0)
        delete[] cross_sections;
}

bool pof_txtr_chunk::read(FILE *file)
{
    long Count;
    _rd(Count, file);

    mats.clear();
    mats.insert(mats.end(), Count, pof_string());

    for (long i=0; i<Count; i++)
    {
        mats[i].read(file);
        //InfoMsg("%d: %s", i, mats[i].s);
    }

    return true;
}

// This can be used only with FreeSpace2 HDR2 chunks
bool pof_objectheader::read(FILE *file)
{
    int i;

    // Basic stuff
    _rd(max_radius, file);
    _rd(obj_flags, file);

    _rd(num_subobjects, file);

    fread(min_bounding, sizeof(float), 3, file);
    fread(max_bounding, sizeof(float), 3, file);

     //InfoMsg("PofModel: <%g, %g, %g>-<%g, %g, %g>, r = %g",
     //   min_bounding.x, min_bounding.y, min_bounding.z,
     //   max_bounding.x, max_bounding.y, max_bounding.z,
     //   max_radius);

    // Main subobjects that define LODs
    _rd(num_detaillevels, file);
    if (num_detaillevels > 0)
    {
        sobj_detaillevels = new int[num_detaillevels];
        for (i=0; i<num_detaillevels; i++)
            _rd(sobj_detaillevels[i], file);
    }

    // Debris - the peices the ship will break into
    _rd(num_debris, file);
    if (num_debris > 0)
    {
        sobj_debris = new int[num_debris];
        for (i=0; i<num_debris; i++)
            _rd(sobj_debris[i], file);
    }

    // Physical properties of ship
    _rd(mass, file);
    //InfoMsg("mass = %f", mass);
    fread(mass_center, sizeof(float), 3, file);
    fread(moment_inertia[0], sizeof(float), 3, file);
    fread(moment_inertia[1], sizeof(float), 3, file);
    fread(moment_inertia[2], sizeof(float), 3, file);

    //InfoMsg("Mass: %f", mass);
    //InfoMsg("MassCenter: <%f, %f, %f>", mass_center[0], mass_center[1], mass_center[2]);
    //InfoMsg("MomentIntertia0: <%f, %f, %f>", moment_inertia[0][0], moment_inertia[0][1], moment_inertia[0][2]);
    //InfoMsg("MomentIntertia1: <%f, %f, %f>", moment_inertia[1][0], moment_inertia[1][1], moment_inertia[1][2]);
    //InfoMsg("MomentIntertia2: <%f, %f, %f>", moment_inertia[2][0], moment_inertia[2][1], moment_inertia[2][2]);

    // More about blowing up
    _rd(num_cross_sections, file);
    if (num_cross_sections > 0)
    {
        cross_sections = new pof_crosssection[num_cross_sections];
        for (i=0; i<num_cross_sections; i++)
        {
            _rd(cross_sections[i].depth, file);
            _rd(cross_sections[i].radius, file);
        }
    }

    // Pre-calculated light flashes?
    _rd(num_lights, file);
    if (num_lights > 0)
    {
        lights = new pof_light[num_lights];
        for (i=0; i<num_lights; i++)
        {
            fread(lights[i].location, sizeof(float), 3, file);
            _rd(lights[i].type, file);
        }
    }

    return true;
}

bool pof_sobj_chunk::read(FILE *file) // OBJ2
{
    _rd(submodel_number, file);
    _rd(radius, file);

    _rd(submodel_parent, file);
    fread(offset, sizeof(float), 3, file);

    fread(geometric_center, sizeof(float), 3, file);
    fread(bounding_box_min_point, sizeof(float), 3, file);
    fread(bounding_box_max_point, sizeof(float), 3, file);

    submodel_name.read(file);
    properties.read(file);

    //InfoMsg("pof-%s: <%g, %g, %g>-<%g, %g, %g>, c = <%g, %g, %g>, r = %g",
    //    submodel_name.s,
    //    bounding_box_min_point.x, bounding_box_min_point.y, bounding_box_min_point.z,
    //    bounding_box_max_point.x, bounding_box_max_point.y, bounding_box_max_point.z,
    //    geometric_center.x, geometric_center.y, geometric_center.z,
    //    radius);

    //InfoMsg("%d: %s, %s", submodel_number, submodel_name.s, properties.s);

    _rd(movement_type, file);
    _rd(movement_axis, file);

//    InfoMsg("MovType: %d, MovAxis: %d", movement_type, movement_axis);

    _rd(reserved, file);

    int bsp_data_size;
    _rd(bsp_data_size, file);
    bsp_data.clear();
    if (bsp_data_size > 0)
    {
        bsp_data.insert(bsp_data.end(), bsp_data_size, 0);
        fread(&bsp_data[0], 1, bsp_data_size, file);
    }

    return true;
}

pof_file::pof_file()
{
}

pof_file::~pof_file()
{
}

bool pof_file::read(FILE *file, long start, long end)
{
    pof_chunkheader ch;

    // reset the pointer just in case
    fseek(file, start, SEEK_SET);

    // read the file header
    fhdr.read(file);
    if (fhdr.file_id != 'PSPO')
    {
        ErrorMsg("Invalid POF file ID: 0x%X", fhdr.file_id);
        return false;
    }
    InfoMsg("Version %d", fhdr.version);

    // read all the chunks
    memset(&ohdr, 0, sizeof ohdr);
    while (ftell(file) < end)
    {
        fread(&ch, sizeof ch, 1, file);

        if (ch.chunk_id == 'HDR2')
        {
            ohdr.read(file);
        }
        else if (ch.chunk_id == 'OBJ2')
        {
            objs.push_back(pof_sobj_chunk());
            objs.back().read(file);
        }
        else if (ch.chunk_id == 'TXTR')
        {
            txtrs.read(file);
        }
        else if (ch.chunk_id == 'FUEL')
        {
            int num, ng;
            _rd(num, file);
            //if ((unsigned int)num > 0xFFFF)
            //{
            //    fseek(file, ch.length - 4, SEEK_CUR);
            //}
            while (num > 0)
            {
                thrusters.push_back(pof_thruster());
                _rd(ng, file);
                if (fhdr.version >= 2117)
                {
                    thrusters.back().properties.read(file);
                }
                else
                {
                    thrusters.back().properties.s[0] = '\0';
                }
                while (ng > 0)
                {
                    thrusters.back().glows.push_back(pof_glow());
                    fread(thrusters.back().glows.back().pos, sizeof(float), 3, file);
                    fread(thrusters.back().glows.back().norm, sizeof(float), 3, file);
                    _rd(thrusters.back().glows.back().radius, file);
                    ng--;
                }
                num--;
            }
        }
        else if (ch.chunk_id == 'GPNT')
        {
            int num_guns;
            _rd(num_guns, file);
            while (num_guns-- > 0)
            {
                weapons.push_back(pof_weapon());
                weapons.back().type = WT_GUN;
                int num_slots;
                _rd(num_slots, file);
                while (num_slots-- > 0)
                {
                    weapons.back().slots.push_back(pof_weapon_slot());
                    pof_weapon_slot &slot = weapons.back().slots.back();
                    fread(slot.point, sizeof(float), 3, file);
                    fread(slot.norm, sizeof(float), 3, file);
                }
            }
        }
        else if (ch.chunk_id == 'MPNT')
        {
            int num_guns;
            _rd(num_guns, file);
            while (num_guns-- > 0)
            {
                weapons.push_back(pof_weapon());
                weapons.back().type = WT_MISSILE;
                int num_slots;
                _rd(num_slots, file);
                while (num_slots-- > 0)
                {
                    weapons.back().slots.push_back(pof_weapon_slot());
                    pof_weapon_slot &slot = weapons.back().slots.back();
                    fread(slot.point, sizeof(float), 3, file);
                    fread(slot.norm, sizeof(float), 3, file);
                }
            }
        }
        else if (ch.chunk_id == 'SHLD')
        {
            // Vertices
            int num;
            _rd(num, file);
            pof_vector v;
            while (num-- > 0)
            {
                shlds.verts.push_back(v);
                fread(&shlds.verts.back(), 4, 3, file);
            }
            // Faces
            _rd(num, file);
            while (num-- > 0)
            {
                shlds.faces.push_back(pof_tpoly());
                fread(shlds.faces.back().normal, 4, 3, file);
                fread(&shlds.faces.back().verts[0].num, 4, 1, file);
                fread(&shlds.faces.back().verts[1].num, 4, 1, file);
                fread(&shlds.faces.back().verts[2].num, 4, 1, file);
                shlds.faces.back().nverts = 3;
                fseek(file, 12, SEEK_CUR); // skip the neighbors
            }
        }
        else if (ch.chunk_id == 'SPCL')
        {
            int num;
            _rd(num, file);
            specials.clear();
            specials.insert(specials.begin(), num, pof_special());
            for (int i=0; i < num; i++)
            {
                specials[i].name.read(file);
                //InfoMsg("SPCLName: %s", ss.s);
                specials[i].properties.read(file);
                //InfoMsg("SPCLProperties: %s", ss.s);
                fread(&specials[i].point, 4, 3, file);
                //InfoMsg("SPCLPoint: <%g, %g, %g>", v.x,v.y,v.z);
                fread(&specials[i].radius, 4, 1, file);
                //InfoMsg("SPCLRadius: %g", r);
            }
        }
        else
        {
            fseek(file, ch.length, SEEK_CUR);
        }
            InfoMsg("Unknown POF Block Type %c%c%c%c",
                 *((char*)(&ch.chunk_id)+0),
                 *((char*)(&ch.chunk_id)+1),
                 *((char*)(&ch.chunk_id)+2),
                 *((char*)(&ch.chunk_id)+3));

    }

    // A little check
    if (objs.size() != size_t(ohdr.num_subobjects))
    {
        ErrorMsg("Not all objects defined in file.  %d vs. %d", objs.size(), ohdr.num_subobjects);
    }

    return true;
}

#undef _rd

void parse_bsp(char *bsp, int len, pof_mesh *mesh)
{
    char *p = bsp;
    char *end = &bsp[len];
    pof_chunkheader ch;
    int i;
    char j;
    AnsiString s;

    //InfoMsg("Data for %s", mesh->obj->submodel_name.s);

    while (p < end)
    {
        memcpy(&ch, p, sizeof ch);

        if (ch.chunk_id == 1)
        {
            int nverts;
            pof_vertex *v;
            int offset;
            int nnorms;
            memcpy(&nverts, &p[8], 4);
            memcpy(&nnorms, &p[12], 4);
            memcpy(&offset, &p[16], 4);

            // create and read the verts
            mesh->verts.insert(mesh->verts.end(), nverts, pof_vertex());

            for (i=0; i<nverts; i++)
            {
                memcpy(&mesh->verts[i].nnms, &p[20+i], 1);
                // TESTED: the correct num of norms is read here
            }

            for (i=0; i<nverts; i++)
            {
                v = &mesh->verts[i];
                memcpy(v->pt, &p[offset], 12);

                offset += 12;
                for (j=0; j<v->nnms; j++)
                {
                    memcpy(v->nms[j], &p[offset], 12);
                    offset += 12;
                }
            }
        }
        else if (ch.chunk_id == 3) // textured face
        {
            pof_tpoly *f = mesh->faces.insert(mesh->faces.end(), pof_tpoly());

            memcpy(f->normal, &p[8], 12);
            memcpy(f->center, &p[20], 12);
            memcpy(&f->radius, &p[32], 4);
            //InfoMsg("pof: c = <%g, %g, %g>, r = %g", f->center.x, f->center.y, f->center.z, f->radius);
            memcpy(&f->nverts, &p[36], 4);
            memcpy(&f->tmap_num, &p[40], 4);
            for (i=0; i<f->nverts; i++)
            {
                memcpy(&f->verts[i], &p[44+i*12], 12);
                //InfoMsg("%d: %d", (int)mesh->faces.size(), f->verts[i].num);
                //ErrorMsg("{%.4f, %.4f}", f->verts[i].u, f->verts[i].v);
            }

            /*Vector nn, v, w;
            v.x = mesh->verts[f->verts[0].num].pt[0] - mesh->verts[f->verts[1].num].pt[0];
            v.y = mesh->verts[f->verts[0].num].pt[1] - mesh->verts[f->verts[1].num].pt[1];
            v.z = mesh->verts[f->verts[0].num].pt[2] - mesh->verts[f->verts[1].num].pt[2];
            w.x = mesh->verts[f->verts[2].num].pt[0] - mesh->verts[f->verts[1].num].pt[0];
            w.y = mesh->verts[f->verts[2].num].pt[1] - mesh->verts[f->verts[1].num].pt[1];
            w.z = mesh->verts[f->verts[2].num].pt[2] - mesh->verts[f->verts[1].num].pt[2];
            Vector::cross(nn, w, v);
            nn.normalize();

            //InfoMsg("n = <%f, %f, %f>, nn = <%f, %f, %f>",
            //    f->normal[0], f->normal[1], f->normal[2],
            //    nn.x, nn.y, nn.z);*/
            //InfoMsg("Added %d-vert Face", f->nverts);
        }
        /*else if (ch.chunk_id == 4) // BSP Node
        {
            // This is a volume of space defined by two points,
            // min and max.  This space is divided by a plane
            // that transverses the volumes longest axis.
            int front, back, onlin, prel, postl;
            pof_vector v;
            v[0] = *(float*)(p+8);
            v[1] = *(float*)(p+12);
            v[2] = *(float*)(p+16);
            front = *(int*)(p+36);
            back  = *(int*)(p+40);
            prel  = *(int*)(p+44);
            postl = *(int*)(p+48);
            onlin = *(int*)(p+52);
            int fv=-1, bv=-1, ov=-1, pre=-1, post=-1;
            if (front>0)
                fv = *(int*)(p+front);
            if (back>0)
                bv = *(int*)(p+back);
            if (onlin>0)
                ov = *(int*)(p+onlin);
            if (prel>0)
                pre = *(int*)(p+prel);
            if (postl>0)
                post = *(int*)(p+postl);
            //InfoMsg("%5d: <%.2f, %.2f, %.2f>  *front = %d, *back = %d, *onlin = %d, *prel = %d, *postl = %d.",
            //    (int)(p-bsp), v[0], v[1], v[2], fv, bv, ov, pre, post);
        }*/
        else
        {
            //InfoMsg("Type: %d, Len: %d, Off: %d", (int)ch.chunk_id[0], ch.length, p-bsp);
        }

        p += ch.length;
        // watch for end of tree
        if (ch.chunk_id == 0) p += 8;
    }
}

pof_mesh::pof_mesh()
{
}

pof_mesh::~pof_mesh()
{
    verts.clear();
    faces.clear();
}

//---------------------------------------------------------------------------
#pragma package(smart_init)
