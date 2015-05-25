//---------------------------------------------------------------------------
#include <vcl.h>
#include <cstdio>
#pragma hdrstop

#include "global.h"
#include "pof.h"
#include "geometry.h"
#include "F_Main.h"
#include "MemBuf.h"

const Vector gamma(0.00f, 0.00f, 0.00f);
unsigned int bsp_off;

using namespace std;

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

    // Build the chunk
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
    Vector center = (mdl->min + mdl->max) * 0.5f;
    float rad = (mdl->min - center).length();
    if ((mdl->max - center).length() > rad)
        rad = (mdl->max - center).length();
    buf.write(rad);

    // Flags
    buf.write(1);

    // Number of sub objects
    j = mdl->get_count_of(MT_HULL) + mdl->get_count_of(MT_DEBRIS);
    buf.write(j);

    // Bounding Box
    buf.write(&mdl->min, 12);
    buf.write(&mdl->max, 12);

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
    buf.write(mdl->get_count_of(MT_DEBRIS));
    for (i=0; i<mdl->meshes.size(); i++)
    {
        if (mdl->meshes[i].type == MT_DEBRIS)
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
    Vector cen = (mdl->max - mdl->min)*0.5;
    buf.write(&cen, 12);
    ////////////////////////////////////////////////////
    // Moment of Inertia
    cen = 0.0f;
    buf.write(&cen, 12);
    buf.write(&cen, 12);
    buf.write(&cen, 12);

    // Cross Sections - none
    buf.write(0);

    // Precalculated Lights - none
    buf.write(0);

    // Write the Chunk
    write_id("HDR2", file);
    write_int((int)buf.tell(), file);
    buf.dump(file);
}

static void reset_mesh_faces(Mesh *msh)
{
    for (size_t i=0; i<msh->faces.size(); i++)
    {
        msh->faces[i].num_includes = 0;
    }
}
static int count_mesh_lone_faces(Mesh *msh)
{
    int count = 0;
    for (size_t i=0; i<msh->faces.size(); i++)
    {
        if (msh->faces[i].num_includes < 1)
            count++;
    }
    return count;
}

void gen_bsp_defpoints(Mesh *msh, MemBuf *bsp)
{
    unsigned int i, j;
    int k;

    // Each vertex contains a list of the normals of all
    // the polygons that use it.  God knows why, but we have
    // to make this list.

    // copy the nice model verts into ugly POF verts
    vector<pof_vertex> vs;
    pof_vertex *pv;
    int n_norms=0; // global count of normals

    for (i=0; i<msh->verts.size(); i++)
    {
        pv = vs.insert(vs.end(), pof_vertex());

        // geometric point
        pv->pt[0] = msh->verts[i].x;
        pv->pt[1] = msh->verts[i].y;
        pv->pt[2] = msh->verts[i].z;

        // normals list
        pv->nnms = 0;  // number
        for (j=0; j<msh->faces.size(); j++)
        {
            for (k=0; k<msh->faces[j].nverts; k++)
            {
                if ((msh->faces[j].verts[k] == (int)i) &&
                    (pv->nnms < POF_MAXNORMS))
                {
                    msh->faces[j].norms[k] = pv->nnms;

                    pv->nms[pv->nnms][0] = msh->faces[j].norm.x;
                    pv->nms[pv->nnms][1] = msh->faces[j].norm.y;
                    pv->nms[pv->nnms][2] = msh->faces[j].norm.z;

                    pv->nnms++;
                    n_norms++;
                }
            }
        }
    }

    // We now have a good list, simply have to write the
    // properly formatted chunk...
    MemBuf buf(0x8000);

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
        buf.write(vs[i].pt[0]);
        buf.write(vs[i].pt[1]);
        buf.write(vs[i].pt[2]);
        for (k=0; k<(int)vs[i].nnms; k++)
        {
            buf.write(vs[i].nms[k][0]);
            buf.write(vs[i].nms[k][1]);
            buf.write(vs[i].nms[k][2]);
        }
    }

    // Chunk Header
    bsp->write(1);               //id
    bsp->write((int)buf.tell()+8); //size
    buf.dump(bsp);               //data
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

    min -= gamma;
    max += gamma;

    buf.write(&min, 12);
    buf.write(&max, 12);

    // Chunk Header
    bsp->write(5);                 // Chunk ID
    bsp->write((int)buf.tell()+8); // size
    buf.dump(bsp);                 // data

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
    bsp->write((int)buf.tell()+8); // size
    buf.dump(bsp);                 // data

    return pos_mem;
}

// This function will either output a face, a null, or a sortnorm depending on
// the number of unique faces found in the given volume
unsigned int gen_bsp_node(BspHull MainHull, Mesh *msh, MemBuf *bsp,
                          vector<FaceList> &Cocentrics)
{
    BspHull front, back;
    Vector norm, center;
    vector<FaceList> front_list, back_list;
    vector<Face>::iterator f;
    unsigned int pos_front, pos_back, pos_pre, pos_post, pos_onl;
    unsigned int pos_mem;

    // Remeber where exactly this data is being placed in the big mess of bsp
    // data.  This is a pretty important number!
    pos_mem = bsp->tell();

    // Count the number of faces in this volume that are unique, but also keep
    // a record of all those that are concentric.
    front_list.clear();
    for (f = msh->faces.begin(); f != msh->faces.end(); f++)
    {
        MainHull.addto_uniquelist(msh, &(*f), front_list);
    }
    // if there are no lists, then generate a null
    if (front_list.size() == 0)
    {
        gen_bsp_null(bsp);
        return pos_mem;
    }
    // if there is only one collection of unique faces, then output them
    else if (front_list.size() <= 2)
    {
        vector<FaceList>::iterator fl = front_list.begin();

        for (; fl != front_list.end(); fl++)
        {
            vector<Face*>::iterator pf = (*fl).begin();
            for (; pf != (*fl).end(); pf++)
            {
                gen_bsp_boundbox(msh, *pf, bsp);
                gen_bsp_face3(msh, *pf, bsp);
            }
        }
        gen_bsp_null(bsp);

        return pos_mem;
    }

    // There must be more than 1 unique faces in this volume, therefore we
    // must create a sortnorm

    // fill it in with everything we know thus far, we will later come back
    // and changed values
    MemBuf buf(0x100);             // this is 9x more than we need
    buf.write(&norm, 12);          // plane normal
    buf.write(&center, 12);        // plane point
    buf.write(0);                  // reserved
    buf.write(0);                  // front
    buf.write(0);                  // back
    buf.write(0);                  // prelist
    buf.write(0);                  // postlist
    buf.write(0);                  // on_the_line
    buf.write(&MainHull.min, 12);  // min point
    buf.write(&MainHull.max, 12);  // max point
    bsp->write(4);                 // Chunk ID
    bsp->write((int)buf.tell()+8); // size
    buf.dump(bsp);                 // data

    // To build the best tree, we try to balance the back side and the front
    // side.  This means testing each combination until the best is found!
    unsigned int diff[3];
    unsigned int best_diff;

    for (int ndiff=0; ndiff<3; ndiff++)
    {
        MainHull.split_n(&back, &front, &norm, &center, ndiff);
        // Count the number of faces in each volume and keep track
        // of cocentric faces
        front_list.clear();
        back_list.clear();
        for (f = msh->faces.begin(); f != msh->faces.end(); f++)
        {
            // Try to add this face to either of the lists
            if (!front.addto_uniquelist(msh, &(*f), front_list))
                back.addto_uniquelist(msh, &(*f), back_list);
        }
        if (front_list.size() >= back_list.size())
            diff[ndiff] = front_list.size() - back_list.size();
        else
            diff[ndiff] = back_list.size() - front_list.size();
    }

    // The normal with the smallest difference will be chosen
    best_diff = 0;
    if (diff[1] < diff[best_diff]) best_diff = 1;
    if (diff[2] < diff[best_diff]) best_diff = 2;

    // Now do the real split and polygon selection
    MainHull.split_n(&back, &front, &norm, &center, best_diff);
    front_list.clear();
    back_list.clear();
    for (f = msh->faces.begin(); f != msh->faces.end(); f++)
    {
        // Try to add this face to either of the lists
        if (!front.addto_uniquelist(msh, &(*f), front_list))
            back.addto_uniquelist(msh, &(*f), back_list);
    }

    // If there are lots of unique faces then continue
    pos_front = gen_bsp_node(front, msh, bsp, Cocentrics);
    pos_front -= pos_mem;
    pos_back = gen_bsp_node(back, msh, bsp, Cocentrics);
    pos_back -= pos_mem;

    // Pre and Post, what the hell are these things??
    pos_pre = 0;//gen_bsp_null(bsp);
    pos_post = 0;//gen_bsp_null(bsp);
    pos_onl = 0;//gen_bsp_null(bsp);

    // Fill the sortnum struct with our new found info
    unsigned int pos_old = bsp->set(pos_mem+8);
    bsp->write(&norm, 12);          // plane normal
    bsp->write(&center, 12);        // center of plane
    bsp->write((int)0);
    bsp->write((int)pos_front);
    bsp->write((int)pos_back);
    bsp->write((int)pos_pre);
    bsp->write((int)pos_post);
    bsp->write((int)pos_onl); // onlin
    bsp->set(pos_old);

    return pos_mem;
}

void gen_bsp(Mesh *msh, MemBuf *bsp)
{
    gen_bsp_defpoints(msh, bsp);

    msh->calc_face_centers();
    reset_mesh_faces(msh);

    // Create the main volume
    BspHull hull;
    hull.min = msh->min - gamma;
    hull.max = msh->max + gamma;

    // A list to  be used for later optimization
    vector<FaceList> ConcList;

    bsp_off = bsp->tell();

    InfoMsg("Generating BSP Tree for %s w/%u Faces...",
            msh->name.c_str(), msh->faces.size());
    gen_bsp_node(hull, msh, bsp, ConcList);

    gen_bsp_null(bsp);
}

void write_obj2(Model *mdl, int nm, FILE *file)
{
    Mesh *msh = &mdl->meshes[nm];

    // Build the chunk
    MemBuf buf(0xFFFFF);

    // Number/Radius/Parent/Offset
    buf.write(nm);
    float rad = msh->min.length();
    if (msh->max.length() > rad) rad = msh->max.length();
    buf.write(rad);
    buf.write(msh->parent);
    buf.write(&msh->offset, 12);

    // Bounding Box
    Vector cen = (msh->max - msh->min)*0.5f;
    buf.write(&cen, 12);
    buf.write(&msh->min, 12);
    buf.write(&msh->max, 12);

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
    MemBuf bsp(0xFFFFF);
    gen_bsp(msh, &bsp);
    buf.write((int)bsp.tell());
    bsp.dump(&buf);

    // Write the Chunk
    write_id("OBJ2", file);
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

    // Make sure everything is calculated
    get_minmax();

    // The header
    write_id("PSPO", ofile);
    write_int(2116, ofile);

    // texture names chunk
    write_txtr(this, ofile);

    // objects header
    write_hdr2(this, ofile);

    // sub objects
    for (i=0; i<meshes.size(); i++)
    {
        write_obj2(this, (int)i, ofile);
    }

    fclose(ofile);

    return true;
}

//---------------------------------------------------------------------------
#pragma package(smart_init)
