//---------------------------------------------------------------------------
#include <vcl.h>
#include <cstdio>
#pragma hdrstop

#include "global.h"
#include "pof.h"
#include "geometry.h"
#include "F_Main.h"
#include "MemBuf.h"

using namespace std;

static unsigned int bsp_off;
static size_t       min_faces_per_hull = 4;

void write_id(char *id, FILE *file)
{
    fwrite(id, 1, 4, file);
}
void write_int(int i, FILE *file)
{
    fwrite(&i, 4, 1, file);
}
void write_float(float f, FILE *file)
{
    fwrite(&f, 4, 1, file);
}

void write_txtr(Model *mdl, FILE *file)
{
    unsigned int i;

    // Build the chunk (is 64 a safe number for maximum string length?)
    MemBuf buf(0x1000);

    // textures count
    buf.write((int)mdl->mats.size());

    // list each texture
    for (i=0; i<mdl->mats.size(); i++)
    {
        buf.write(mdl->mats[i].s.Length());
        buf.write(mdl->mats[i].s.c_str(), mdl->mats[i].s.Length());
    }

    // Write the Chunk
    write_id("TXTR", file);
    write_int((int)buf.tell(), file);
    buf.dump(file);
}

void write_hdr2(Model *mdl, FILE *file)
{
    unsigned int i;
    int j;

    // Build the chunk
    MemBuf buf(0x1000);

    // get and write the radius
    float rad = mdl->CalcRadiusFromPoint(Vector(0.0f,0.0f,0.0f), true);
    buf.write(rad);

    // Flags
    buf.write(1);

    // Number of sub objects
    j = mdl->get_count_of(MT_HULL) + mdl->get_count_of(MT_DEBRIS);
    buf.write(j);

    // Bounding Box
    buf.write(&mdl->min, 12);
    buf.write(&mdl->max, 12);

    //InfoMsg("Model: <%g, %g, %g>-<%g, %g, %g>, r = %g",
    //    mdl->min.x, mdl->min.y, mdl->min.z,
    //    mdl->max.x, mdl->max.y, mdl->max.z,
    //    rad);

    // Detail Level Parents
    buf.write(mdl->nlods);
    for (j=0; j<mdl->nlods; j++)
    {
        // scan through all meshes trying to find a hull with
        // no parent and the correct LOD number
        for (i=0; i<mdl->meshes.size(); i++)
        {
            if ((mdl->meshes[i].lod == j) &&
                (mdl->meshes[i].parent == -1) &&
                (mdl->meshes[i].type == MT_HULL))
                buf.write((int)i);
        }
    }

    // Debris
    int nDebris = mdl->get_count_of(MT_DEBRIS);
    buf.write(nDebris);
    for (i=0; i<mdl->meshes.size(); i++)
    {
        if ((mdl->meshes[i].type == MT_DEBRIS) &&
            (mdl->meshes[i].parent == -1))
            buf.write((int)i);
    }

    // Physical Properties of 1st LOD
    float mass = 0.0f;
    for (i=0; i<mdl->meshes.size(); i++)
    {
        if ((mdl->meshes[i].lod == 0) &&
            (mdl->meshes[i].type == MT_HULL))
            mass += mdl->meshes[i].get_mass();
    }
    buf.write(mass);
    Vector cen = (mdl->max + mdl->min)*0.5;
    buf.write(&cen, 12);
    ////////////////////////////////////////////////////
    // Moment of Inertia
    cen = 0.0f;
    buf.write(&cen, 12);
    buf.write(&cen, 12);
    buf.write(&cen, 12);

    // Cross Sections - none
    int temp = 0;
    buf.write(temp);

    // Precalculated Lights - none
    temp = 0;
    buf.write(temp);

    // Write the Chunk
    write_id("HDR2", file);
    write_int((int)buf.tell(), file);
    buf.dump(file);
}

void gen_bsp_defpoints(Mesh *msh, MemBuf *bsp)
{
    unsigned int i, j;
    int k;

    // Each vertex contains a list of the normals of all the polygons that use it.

    vector<pof_vertex> vs;
    pof_vertex *pv;
    int n_norms=0; // global count of normals

    for (i=0; i<msh->verts.size(); i++)
    {
        pv = vs.insert(vs.end(), pof_vertex());

        // Geometric point
        pv->pt.x = msh->verts[i].x;
        pv->pt.y = msh->verts[i].y;
        pv->pt.z = msh->verts[i].z;

        // Normals list

        // Advanced Smoothing:  If the angle between the polygon an its neighbor
        // is greater than 60º, then the edge is smoothed.  If it is not, then
        // it is flat shaded. BUT, it doesn't do this yet....

        pv->nnms = 0;  // number
        for (j=0; j<msh->faces.size(); j++)
        {
            for (k=0; k<msh->faces[j].nverts; k++)
            {
                if ((msh->faces[j].verts[k] == (int)i) &&
                    (pv->nnms < POF_MAXNORMS))
                {
                    int num_neighbors = 1;
                    pv->nms[pv->nnms].x = msh->faces[j].norm.x;
                    pv->nms[pv->nnms].y = msh->faces[j].norm.y;
                    pv->nms[pv->nnms].z = msh->faces[j].norm.z;

                    for (size_t of=0; of < msh->faces.size(); of++)
                    {
                        for (int ofk=0; ofk < msh->faces[of].nverts; ofk++)
                        {
                            if (msh->faces[of].verts[ofk] == msh->faces[j].verts[k])
                            {
                                pv->nms[pv->nnms].x += msh->faces[of].norm.x;
                                pv->nms[pv->nnms].y += msh->faces[of].norm.y;
                                pv->nms[pv->nnms].z += msh->faces[of].norm.z;
                                num_neighbors++;
                            }
                        }
                    }

                    pv->nms[pv->nnms].x /= (float)num_neighbors;
                    pv->nms[pv->nnms].y /= (float)num_neighbors;
                    pv->nms[pv->nnms].z /= (float)num_neighbors;

                    msh->faces[j].norms[k] = n_norms; //pv->nnms;
                    pv->nnms++;
                    n_norms++;
                } // if this vertex is used by this face
            } // for each vertex in the face
        } // for each face
    } // for each vertex

    // We now have a good list, simply have to write the
    // properly formatted chunk...
    MemBuf buf(0x1000);

    buf.write((int)vs.size());
    buf.write(n_norms);
    // offset to vertex data
    buf.write(20+(int)vs.size());
    // normal counts
    for (i=0; i<vs.size(); i++)
        buf.write((char)vs[i].nnms);
    // geometry
    for (i=0; i<vs.size(); i++)
    {
        buf.write(vs[i].pt.x);
        buf.write(vs[i].pt.y);
        buf.write(vs[i].pt.z);
        for (k=0; k<(int)vs[i].nnms; k++)
        {
            buf.write(vs[i].nms[k].x);
            buf.write(vs[i].nms[k].y);
            buf.write(vs[i].nms[k].z);
        }
    }

    // Chunk Header
    bsp->write(1);                 // Chunk ID
    bsp->write((int)buf.tell()+8); // Size
    buf.dump(bsp);                 // Data
}

unsigned int gen_bsp_null(MemBuf *bsp)
{
    unsigned int pos_mem = bsp->tell();

    bsp->write(0);
    bsp->write(0);

    return pos_mem;
}

unsigned int gen_bsp_boundbox(Mesh *msh, Face *f, MemBuf *bsp)
{
    MemBuf buf(0x40);
    int i;
    unsigned int pos_mem = bsp->tell();
    Vector min, max;

    min = msh->verts[f->verts[0]];
    max = msh->verts[f->verts[0]];

    // get the extents of this single face
    Vector c;
    for (i=1; i<f->nverts; i++)
    {
        if (msh->verts[f->verts[i]].x > max.x) max.x = msh->verts[f->verts[i]].x;
        if (msh->verts[f->verts[i]].y > max.y) max.y = msh->verts[f->verts[i]].y;
        if (msh->verts[f->verts[i]].z > max.z) max.z = msh->verts[f->verts[i]].z;
        if (msh->verts[f->verts[i]].x < min.x) min.x = msh->verts[f->verts[i]].x;
        if (msh->verts[f->verts[i]].y < min.y) min.y = msh->verts[f->verts[i]].y;
        if (msh->verts[f->verts[i]].z < min.z) min.z = msh->verts[f->verts[i]].z;
    }

    buf.write(&min, 12);
    buf.write(&max, 12);

    // Chunk Header
    bsp->write(5);                 // Chunk ID
    bsp->write((int)buf.tell()+8); // Size
    buf.dump(bsp);                 // Data

    return pos_mem;
}

unsigned int gen_bsp_face3(Mesh *msh, Face *f, MemBuf *bsp)
{
    // We now have a good list, simply have to write the
    // properly formatted chunk...
    MemBuf buf(0x300);
    int i;
    unsigned int pos_mem = bsp->tell();

    buf.write(&f->norm, 12);

    // get and write the center
    Vector c;
    for (i=0; i<f->nverts; i++)
        c += msh->verts[f->verts[i]];
    c *= (1.0f/(float)f->nverts);
    buf.write(&c, 12);

    // get and write the radius
    float r=0.0;
    float l;
    for (i=0; i<f->nverts; i++)
    {
        l = (msh->verts[f->verts[i]] - c).length();
        if (l > r) r = l;
    }
    buf.write(r);
//    InfoMsg("c = <%g, %g, %g>, r = %g", c.x,c.y,c.z,r);

    // everything else
    buf.write(f->nverts);
    buf.write(f->texture);
    for (i=0; i<f->nverts; i++)
    {
        buf.write((short)f->verts[i]);
        buf.write((short)f->norms[i]);
        buf.write(f->texverts[i].u);
        buf.write(f->texverts[i].v);
    }

    // Chunk Header
    bsp->write(3);                 // Chunk ID
    bsp->write((int)buf.tell()+8); // Size
    buf.dump(bsp);                 // Data

    return pos_mem;
}

// This function will either output a face, a null, or a sortnorm depending on
// the number of faces found in the given volume and whether there are still
// faces that need to be written.
unsigned int gen_bsp_node(const BspHull &main_hull, Mesh *msh, MemBuf *bsp)
{
    //InfoMsg("There are %d faces in this volume.", main_hull.face_list.size());
    
    BspHull front, back;
    Vector norm, center;
    unsigned int pos_front, pos_back, pos_pre, pos_post, pos_onl;
    unsigned int pos_mem;

    // Remeber where exactly this data is being placed in the big mess of bsp
    // data.  This is a pretty important number!
    pos_mem = bsp->tell();

    // Instead of doing this blind split, a histogram should be built to
    // determine which axis really needs splitting.
    main_hull.split(&back, &front, &norm, &center);

    // Now we take all the faces contained in main_hull and distribute them to
    // its two new children.
    for (size_t i=0; i < main_hull.face_list.size(); i++)
    {
        if (front.calc_amount_face_is_inside(msh, main_hull.face_list[i]) >=
            back.calc_amount_face_is_inside(msh, main_hull.face_list[i]))
        {
            front.face_list.push_back(main_hull.face_list[i]);
        }
        else
        {
            back.face_list.push_back(main_hull.face_list[i]);
        }
    }

    // There are three instances where we will just write all the faces contained
    // in this block:
    // 1) When the volume contains the less than minimum number of faces that a
    //    volume is allowed to have when it is split.
    // 2) When there are no faces contained in the front sub-volume
    // 3) When there are no faces contained in the back sub-volume
    if ((main_hull.face_list.size() < min_faces_per_hull) ||
        (front.face_list.size() == 0) ||
        (back.face_list.size() == 0))
    {
        for (size_t i=0; i < main_hull.face_list.size(); i++)
        {
            gen_bsp_boundbox(msh, main_hull.face_list[i], bsp);
            gen_bsp_face3(msh, main_hull.face_list[i], bsp);
        }
        gen_bsp_null(bsp);

        return pos_mem;
    }

    // We must have made the dreadful decision to build a sortnorm.  Here we go..
    // Fill sortnorm in with everything we know thus far, we will later come back
    // and change values
    MemBuf buf(180);               // this is 2x more than we need (paranoia)
    buf.write(&norm, 12);          // plane normal
    buf.write(&center, 12);        // plane point
    buf.write(0);                  // reserved
    buf.write(0);                  // front
    buf.write(0);                  // back
    buf.write(0);                  // prelist
    buf.write(0);                  // postlist
    buf.write(0);                  // on_the_line
    buf.write(&main_hull.min, 12);  // min point
    buf.write(&main_hull.max, 12);  // max point
    bsp->write(4);                 // Chunk ID
    bsp->write((int)buf.tell()+8); // size
    buf.dump(bsp);                 // data

    // Write a termination node right after this sortnorm
    gen_bsp_null(bsp);

    // If there are lots of unique faces then continue
    pos_front  = gen_bsp_node(front, msh, bsp) - pos_mem;
    pos_back   = gen_bsp_node(back, msh, bsp)  - pos_mem;

    // Pre and Post, what the hell are these things??
    pos_pre  = gen_bsp_null(bsp) - pos_mem;
    pos_post = gen_bsp_null(bsp) - pos_mem;
    pos_onl  = gen_bsp_null(bsp) - pos_mem;

    // Fill the sortnum struct with our new found info
    unsigned int pos_old = bsp->set(pos_mem+8);
    bsp->write(&norm, 12);          // Plane normal
    bsp->write(&center, 12);        // Center of plane
    bsp->write((int)0);
    bsp->write((int)pos_front);
    bsp->write((int)pos_back);
    bsp->write((int)pos_pre);
    bsp->write((int)pos_post);
    bsp->write((int)pos_onl);       // On-line

    // We shall now artificially resize the bounding volume so that it
    // encompasses all of the faces whose centers are in it.
    Vector new_min = main_hull.min, new_max = main_hull.max;
    for (size_t i=0; i < main_hull.face_list.size(); i++)
    {
        for (int k=0; k < main_hull.face_list[i]->nverts; k++)
        {
            // resuse the center variable
            center = msh->verts[main_hull.face_list[i]->verts[k]];
            if (center.x > new_max.x) new_max.x = center.x;
            if (center.y > new_max.y) new_max.y = center.y;
            if (center.z > new_max.z) new_max.z = center.z;
            if (center.x < new_min.x) new_min.x = center.x;
            if (center.y < new_min.y) new_min.y = center.y;
            if (center.z < new_min.z) new_min.z = center.z;
        }
    }
    bsp->write(&new_min, 12);       // Bounding Volume min coordinate
    bsp->write(&new_max, 12);       // Bounding Volume min coordinate
    bsp->set(pos_old);

    return pos_mem;
}

void gen_bsp(Mesh *msh, MemBuf *bsp)
{
    const Vector gamma(0.01f, 0.01f, 0.01f);

    // Write the vertices and vert normals
    gen_bsp_defpoints(msh, bsp);

    // Just in case we will want them
    msh->calc_face_centers();

    // Create the main volume
    BspHull hull;
    hull.min = msh->min - gamma;
    hull.max = msh->max + gamma;
    for (size_t i=0; i<msh->faces.size(); i++)
    {
        hull.face_list.push_back(&msh->faces[i]);
    }

    // Record the location of all this mayham
    bsp_off = bsp->tell();

    // Tell the user because this could take a few...
    //InfoMsg("Generating BSP Tree for %s w/%u Faces...",
    //        msh->name.c_str(), msh->faces.size());

    // Go and do the dirty work
    //min_faces_per_hull = msh->faces.size()/8;
    //if (min_faces_per_hull < 8) min_faces_per_hull = 8;
    min_faces_per_hull = 2;
    gen_bsp_node(hull, msh, bsp);

    // EOF marker
    gen_bsp_null(bsp);
}

void write_obj2(Model *mdl, int nm, FILE *file)
{
    Mesh *msh = &(mdl->meshes[nm]);

    // Build the chunk
    MemBuf buf(0x1000);

    // Number/Radius/Parent/Offset/Center
    buf.write(nm);
    float rad = msh->CalcRadiusFromPoint(Vector(0.0f,0.0f,0.0f));
    buf.write(rad);
    buf.write(msh->parent);
    buf.write(&msh->offset, 12);

    Vector cen = (msh->max + msh->min)*0.5f;
    buf.write(&cen, 12);

    // Bounding Box
    buf.write(&msh->min, 12);
    buf.write(&msh->max, 12);

    //InfoMsg("%s: <%g, %g, %g>-<%g, %g, %g>, c = <%g, %g, %g>, r = %g",
    //    msh->name.c_str(),
    //    msh->min.x, msh->min.y, msh->min.z,
    //    msh->max.x, msh->max.y, msh->max.z,
    //    cen.x, cen.y, cen.z,
    //    rad);

    // Name/Props
    buf.write(msh->name.Length());
    buf.write(msh->name.c_str(), msh->name.Length());
    buf.write(msh->props.Length());
    buf.write(msh->props.c_str(), msh->props.Length());

    // Movement Type/Axi/Reserved
    buf.write(-1);
    buf.write(-1);
    buf.write(0);

    // BSP
    MemBuf bsp(0x1000);
    gen_bsp(msh, &bsp);
    buf.write((int)bsp.tell());
    bsp.dump(&buf);

    // Write the Chunk
    write_id("OBJ2", file);
    write_int((int)buf.tell(), file);
    buf.dump(file);
}

static void find_tri_neighbors(Mesh &msh, int face, int *neighbors)
{
    neighbors[0] = neighbors[1] = neighbors[2] = 0;

    // For each edge on the face, find a corresponding edge on some other face
    int ia, ib, va, vb, ic, id, vc, vd;
    for (ia = 0; ia <= 2; ia++)
    {
        // Get the proper indexes
        ib = (ia+1) % 3;

        // Get the proper edge values
        va = msh.faces[face].verts[ia];
        vb = msh.faces[face].verts[ib];

        // cycle through all the faces until a similar edge is found
        for (size_t n=0; n < msh.faces.size(); n++)
        {
            if (int(n) == face) continue;

            for (ic = 0; ic <= 2; ic++)
            {
                id = (ic+1) % 3;
                vc = msh.faces[n].verts[ic];
                vd = msh.faces[n].verts[id];

                if ((vc==va && vd==vb) ||
                    (vc==vb && vd==va))
                {
                    neighbors[ia] = int(n);
                }
            }  // for each edge
        } // for every other face
    } // for each edge
}

void write_shld(Model *mdl, FILE *file)
{
    MemBuf buf(0x1000);

    // First the vertices
    buf.write((int)mdl->shields.verts.size());
    for (size_t i=0; i<mdl->shields.verts.size(); i++)
    {
        buf.write(&mdl->shields.verts[i], 12);
    }

    // Now the faces
    int n[3];
    buf.write((int)mdl->shields.faces.size());
    for (size_t i=0; i<mdl->shields.faces.size(); i++)
    {
        buf.write(&mdl->shields.faces[i].norm, 12);
        buf.write((int)mdl->shields.faces[i].verts[0]);
        buf.write((int)mdl->shields.faces[i].verts[1]);
        buf.write((int)mdl->shields.faces[i].verts[2]);
        find_tri_neighbors(mdl->shields, i, n);
        buf.write(n[0]); // neighbor 1
        buf.write(n[1]); //          2
        buf.write(n[2]); //          3
    }

    write_id("SHLD", file);
    write_int((int)buf.tell(), file);
    buf.dump(file);
}

void write_weapon_group(WeaponGroup &grp, MemBuf &buf)
{
    size_t NumWeaps = grp.weapons.size();

    buf.write((int)NumWeaps);

    size_t weap; int difference;
    for (size_t i=0; i < NumWeaps; i++)
    {
        // find the weapon that is closest to (or larger than) 'i'
        difference = 999999999; //big
        for (size_t j=0; j < grp.weapons.size(); j++)
        {
            if (grp.weapons[j].ID < (int)i) continue; // shouldn't happen
            if ((grp.weapons[j].ID - (int)i) < difference)
            {
                weap = j;
                difference = (grp.weapons[j].ID - (int)i);
            }
        }
        buf.write(&grp.weapons[weap].Pos, 12);
        buf.write(&grp.weapons[weap].Normal, 12);

        // remove it so that it is not considered in the future
        grp.weapons.erase(grp.weapons.begin()+weap);
    }
}

int imax(int a, int b) { return ((a>=b) ? a : b); }
int imin(int a, int b) { return ((a<=b) ? a : b); }

void write_weapons(Model *mdl, FILE *file)
{
    size_t i, num;

    // First, count the number of gun and missile groups that exist
    int num_ggrps = 0;
    int num_mgrps = 0;
    for (i=0; i < mdl->weapons.size(); i++)
    {
        if (mdl->weapons[i].Type != WT_GUN) continue;
        num_ggrps = imax(mdl->weapons[i].Group+1, num_ggrps);
    }
    for (i=0; i < mdl->weapons.size(); i++)
    {
        if (mdl->weapons[i].Type != WT_MISSILE) continue;
        num_mgrps = imax(mdl->weapons[i].Group+1, num_mgrps);
    }

    // Now, add each weapon to its appropriate group
    vector<WeaponGroup> guns;
    vector<WeaponGroup> missiles;
    guns.insert(guns.begin(), num_ggrps, WeaponGroup());
    missiles.insert(missiles.begin(), num_mgrps, WeaponGroup());
    for (i=0; i < mdl->weapons.size(); i++)
    {
        if (mdl->weapons[i].Type == WT_GUN)
        {
            guns[mdl->weapons[i].Group].weapons.push_back(mdl->weapons[i]);
        }
        else if (mdl->weapons[i].Type == WT_MISSILE)
        {
            missiles[mdl->weapons[i].Group].weapons.push_back(mdl->weapons[i]);
        }
    }

    // Leave it up to the child function to sort the actual weapon IDs...

    MemBuf buf(0x1000);

    // First the guns
    buf.reset();
    if (guns.size() > 0)
    {
        buf.write((int)guns.size());
        for (size_t i=0; i < guns.size(); i++)
        {
            write_weapon_group(guns[i], buf);
        }
        write_id("GPNT", file);
        write_int((int)buf.tell(), file);
        buf.dump(file);
    }

    // Now, the missiles
    buf.reset();
    if (missiles.size() > 0)
    {
        buf.write((int)missiles.size());
        for (size_t i=0; i < missiles.size(); i++)
        {
            write_weapon_group(missiles[i], buf);
        }
        write_id("MPNT", file);
        write_int((int)buf.tell(), file);
        buf.dump(file);
    }
}

void write_specials(Model *mdl, FILE *file)
{
    MemBuf buf(0x1000);
    buf.reset();

    buf.write((int)mdl->Specials.size());
    for (size_t i=0; i < mdl->Specials.size(); i++)
    {
        buf.write(mdl->Specials[i].Name.Length());
        buf.write(mdl->Specials[i].Name.c_str(), mdl->Specials[i].Name.Length());
        buf.write(mdl->Specials[i].Props.Length());
        buf.write(mdl->Specials[i].Props.c_str(), mdl->Specials[i].Props.Length());
        buf.write(&mdl->Specials[i].Pos, 12);
        buf.write(mdl->Specials[i].Radius);
    }
    write_id("SPCL", file);
    write_int((int)buf.tell(), file);
    buf.dump(file);
}

void write_thrusters(Model *mdl, FILE *file)
{
    MemBuf buf(0x1000);
    buf.reset();

    buf.write((int)mdl->thrusters.size());

    for (size_t t=0; t < mdl->thrusters.size(); t++)
    {
        buf.write((int)mdl->thrusters[t].glows.size());
        buf.write(mdl->thrusters[t].Props.Length());
        buf.write(mdl->thrusters[t].Props.c_str(), mdl->thrusters[t].Props.Length());
        for (size_t g=0; g < mdl->thrusters[t].glows.size(); g++)
        {
            buf.write(&mdl->thrusters[t].glows[g].pos, 12);
            buf.write(&mdl->thrusters[t].glows[g].norm, 12);
            buf.write(mdl->thrusters[t].glows[g].radius);
        }
    }
    write_id("FUEL", file);
    write_int((int)buf.tell(), file);
    buf.dump(file);
}

bool Model::MakePOF()
{
    unsigned int i;
    AnsiString fn = ModelsDir + ExtractFileName(filename);
    fn = ChangeFileExt(fn, ".pof");

    FILE *ofile = fopen(fn.c_str(), "wb");
    if (!ofile)
    {
        ErrorMsg("Could not open '%s' for writing.", fn.c_str());
        return false;
    }

    // Make sure everything is calculated (including shields)
    get_minmax(true);

    // The header
    write_id("PSPO", ofile);
    write_int(2117, ofile);

    // texture names chunk
    write_txtr(this, ofile);

    // objects header
    write_hdr2(this, ofile);

    // sub objects
    for (i=0; i<meshes.size(); i++)
    {
        write_obj2(this, (int)i, ofile);
    }

    // Good old shields
    if (shields.faces.size() > 0)
    {
        write_shld(this, ofile);
    }

    // Hey, this is a a combat game isn't it???
    write_weapons(this, ofile);

    // "Special" -- Well, Laa dee daa!
    if (Specials.size() > 0)
    {
        write_specials(this, ofile);
    }

    // Thrusters -- Pure eye candy, you've got to love that
    if (thrusters.size() > 0)
    {
        write_thrusters(this, ofile);
    }

    fclose(ofile);

    return true;
}

//---------------------------------------------------------------------------
#pragma package(smart_init)
