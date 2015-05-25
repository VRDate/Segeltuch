//---------------------------------------------------------------------------
#include <vcl.h>
#include <math.h>
#include <stdio.h>
#pragma hdrstop

#include "Geometry.h"
#include "Global.h"
#include "F_Main.h"

#define DRAD 0.0174533f

#define TEXT_ID       1
#define SHLD_ID       2
#define THRUST_ID     3
#define OLD_WEAP_ID   4
#define NEW_WEAP_ID   5
#define SPCL_ID       6

using namespace std;

Model Proj;

void Transformation::ProjectPoint(const Vector &Pt, Vector &NewPt)
{
    Vector Temp = Pt - Pos;

    NewPt.x = Vector::dot(Temp, Right);
    NewPt.y = Vector::dot(Temp, Up);
    NewPt.z = 0.0f;
}

void Transformation::UnProjectPoint(const Vector &Pt, Vector &NewPt)
{
    NewPt = Up*Pt.y + Right*Pt.x + Pos;
}

bool BspHull::is_face_inside(Mesh *msh, Face *f)
{
    // Who knows...
    if (f->nverts < 1) return false;

    // For now, we check the center of the polygon and test that
    Vector p;
    for (int k=0; k<f->nverts; k++)
        p += msh->verts.at(f->verts[k]);
    p *= (1.0f / (float)f->nverts);

    if ((p.x > max.x) || (p.x < min.x)) return false;
    if ((p.y > max.y) || (p.y < min.y)) return false;
    if ((p.z > max.z) || (p.z < min.z)) return false;

    return true;
}

bool BspHull::is_face_partially_inside(Mesh *msh, Face *f)
{
    // This function should be more advanced as to check for edge intersections
    // with this bounding box.  But for now, we just go with vertices.
    for (int k=0; k<f->nverts; k++)
    {
        if (is_point_inside(msh->verts.at(f->verts[k])))
        {
            return true;
        }
    }

    return false;
}

/*------------------------------------------------------------------------------
  Returns a value [0..1] representing how much of a face is inside this volume.
  A value of 0 means nothing, and a value of 1 means all.  Intermediate values
  are expected.
------------------------------------------------------------------------------*/
float BspHull::calc_amount_face_is_inside(Mesh *msh, Face *f)
{
    // This function should be more advanced as to check for edge intersections,
    // and hence area intersections, and then use the total area to find the
    // fractional amount.
    /*if (f->nverts < 1) return 0.0;

    int count = 0;
    for (int k=0; k<f->nverts; k++)
    {
        if (is_point_inside(msh->verts.at(f->verts[k])))
        {
            count++;
        }
    }

    return ((float)count / (float)f->nverts);*/

    if (is_point_inside(f->center)) return 1.0f;
    return 0.0f;
}

bool BspHull::is_point_inside(const Vector &p)
{
    if ((p.x > max.x) || (p.x < min.x)) return false;
    if ((p.y > max.y) || (p.y < min.y)) return false;
    if ((p.z > max.z) || (p.z < min.z)) return false;

    return true;
}

void BspHull::split(BspHull *back, BspHull *front, Vector *norm, Vector *center) const
{
    Vector dims = (max - min);

    *center = (max + min)*(0.5f);
    *norm = 0.0f;
    back->min = min; back->max = max;
    front->min = min; front->max = max;

    // Find the longest dimension of the bounding box and splits that axis in
    // half.
    if ((dims.x >= dims.y) && (dims.x >= dims.z))
    {
        norm->x = 1.0f;
        back->max.x = center->x;
        front->min.x = center->x;
    }
    else if (dims.y >= dims.z)
    {
        norm->y = 1.0f;
        back->max.y = center->y;
        front->min.y = center->y;
    }
    else
    {
        norm->z = 1.0f;
        back->max.z = center->z;
        front->min.z = center->z;
    }
}

// Allows user to select the normal
void BspHull::split_n(BspHull *back, BspHull *front, Vector *norm, Vector *center,
                      char n) const
{
    Vector dims = (max - min);

    *center = (max + min)*(0.5f);
    *norm = 0.0f;
    back->min = min; back->max = max;
    front->min = min; front->max = max;

    // Find the major axis and split accross it
    if (n == 0)
    {
        norm->x = 1.0f;
        back->max.x = center->x;
        front->min.x = center->x;
    }
    else if (n == 1)
    {
        norm->y = 1.0f;
        back->max.y = center->y;
        front->min.y = center->y;
    }
    else
    {
        norm->z = 1.0f;
        back->max.z = center->z;
        front->min.z = center->z;
    }
}

bool BspHull::addto_uniquelist(Mesh *msh, Face *f, vector<FaceList> &UniqueList)
{
    if (!is_face_inside(msh, &(*f))) return false;

    int conc = find_cocentric_face(*f, UniqueList);

    if (conc >= 0)
    {
        UniqueList[conc].push_back(&(*f));
    }
    else
    {
        UniqueList.push_back(FaceList());
        UniqueList.back().push_back(&(*f));
    }

    return true;
}


float Vector::length() const
{
    return sqrt(x*x + y*y + z*z);
}

float Vector::max_val()
{
    float a = x>y?x:y;
    return (a>z?a:z);
}

float Vector::min_val()
{
    float a = x<y?x:y;
    return (a<z?a:z);
}

// Misc
Vector& Vector::normalize()
{
    float len = length();

    if (len < 1e-6f)
        len = 1.0f;
    else
        len = 1.0f/len;

    x *= len;
    y *= len;
    z *= len;

    return *this;
}

float Vector::dot(const Vector &v1, const Vector &v2)
{
    return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}

void Vector::cross(Vector &r, const Vector &v1, const Vector &v2)
{
    r.x = v1.y*v2.z - v1.z*v2.y;
    r.y = v1.z*v2.x - v1.x*v2.z;
    r.z = v1.x*v2.y - v1.y*v2.x;
}

Vector PlaneLineInter(const Vector &s, const Vector &d,
                      const Vector &n, const Vector &p)
{
    //from Kenny Hoff
    //s: position of ray
    //d: direction of ray
    //t: time until ray intersects plane
    //n: normal of plane
    //p: point on the plane
    //D: distance along normal to plane
    float D = Vector::dot(n, p);

    float denom = Vector::dot(n, d);
    if (fabs(denom) < 1e-6) return p;

    float t = (D - Vector::dot(n, s))/denom;
    return (s + d*t);
}

void Mesh::Clear()
{
    lod     = 0;
    name    = "";
    max    *= 0.0f;
    min    *= 0.0f;
    offset *= 0.0f;
    props   = "";
    type    = MT_HULL;

    faces.clear();
    verts.clear();
}

void Model::Clear()
{
    nlods     = 0;
    mod       = false;
    name      = "";
    filename  = "";
    max      *= 0.0f;
    min      *= 0.0f;
    dMin = dMax = 0.0f;

    meshes.clear();
    mats.clear();
    shields.Clear();
    thrusters.clear();
    weapons.clear();
    Specials.clear();
}

bool Model::Write()
{
    if (filename.Length() == 0)
    {
        ErrorMsg("Project has no filename.");
        return false;
    }
    FILE *f = fopen(filename.c_str(), "wb");
    if (f == NULL)
    {
        ErrorMsg("Could not open %s for binary writing.", filename.c_str());
        return false;
    }

    int n = meshes.size();
    int i;

    // Header
    i = GEO_MAGIC;
    fwrite(&i, sizeof i, 1, f);
    fwrite(&n, sizeof n, 1, f);
    fwrite(&nlods, sizeof nlods, 1, f);

    // Info
    for (i=0; i<n; i++)
    {
        meshes[i].Write(f);
    }

    // Texturing
    i = TEXT_ID;
    fwrite(&i, sizeof i, 1, f);
    i = mats.size();
    fwrite(&i, sizeof i, 1, f);
    for (i=0; i<(int)mats.size(); i++)
    {
        n = mats[i].s.Length();
        fwrite(&n, sizeof n, 1, f);
        fwrite(mats[i].s.c_str(), n+1, 1, f);
    }

    // Shields
    if (shields.faces.size() > 0)
    {
        i = SHLD_ID;
        fwrite(&i, 4, 1, f);
        shields.Write(f);
    }

    if (weapons.size() > 0)
    {
        i = NEW_WEAP_ID;
        fwrite(&i, 4, 1, f);
        i = (int)weapons.size();
        fwrite(&i, 4, 1, f);
        for (i=0; i<(int)weapons.size(); i++)
        {
            weapons[i].Write(f);
        }
    }

    if (Specials.size() > 0)
    {
        i = SPCL_ID;
        fwrite(&i, 4, 1, f);
        i = (int)Specials.size();
        fwrite(&i, 4, 1, f);
        for (i=0; i<(int)Specials.size(); i++)
        {
            Specials[i].Write(f);
        }
    }

    if (thrusters.size() > 0)
    {
        i = THRUST_ID;
        fwrite(&i, 4, 1, f);
        i = (int)thrusters.size();
        fwrite(&i, 4, 1, f);
        for (i=0; i<(int)thrusters.size(); i++)
        {
            thrusters[i].Write(f);
        }
    }

    fclose(f);
    return true;
}

bool Model::Read()
{
    int chunk;
    FILE *f;
    ImgString ist;

    if (filename.Length() == 0)
    {
        ErrorMsg("Project has no filename.");
        return false;
    }
    f = fopen(filename.c_str(), "rb");
    if (f == NULL)
    {
        ErrorMsg("Could not open %s for binary reading.", filename.c_str());
        return false;
    }

    int nmeshes;
    Mesh *m;

    // Header
    fread(&nmeshes, sizeof nmeshes, 1, f);
    if (nmeshes != GEO_MAGIC)
    {
        ErrorMsg("This is not a Segeltuch Project file");
        return false;
    }
    fread(&nmeshes, sizeof nmeshes, 1, f);
    fread(&nlods, sizeof nlods, 1, f);

    // Info
    meshes.insert(meshes.end(), nmeshes, Mesh());
    for (m=meshes.begin(); m != meshes.end(); m++)
    {
        m->Read(f);
    }

    // Additional resources
    int ntexts, i;
    char str[128];
    int nt, ng;
    Thruster *trstr;
    Glow *glow;
    // Process them
    while (fread(&chunk, 4, 1, f))
    {
        switch (chunk)
        {
        case TEXT_ID:
            fread(&ntexts, 4, 1, f);
            //InfoMsg("%d Textures found", ntexts);
            while (ntexts)
            {
                fread(&i, 4, 1, f);
                fread(str, i+1, 1, f);
                ist.s = str; ist.image = 0;
                mats.push_back(ist);
                ntexts--;
            }
            break;
        case SHLD_ID:
            shields.Read(f);
            break;
        case OLD_WEAP_ID:  /////// REMOVE AS SOON AS POSSIBLE ////////
            {
            fread(&i, 4, 1, f);
            WeaponGroup wg;
            int group_gun = 0, group_mis = 0, group;
            while (i > 0)
            {
                wg.Read(f); i--;
                if (wg.type == WT_GUN)
                    group = group_gun++;
                else
                    group = group_mis++;
                size_t wi = weapons.size();
                weapons.insert(weapons.end(), wg.weapons.size(), Weapon());
                for (size_t w=0; w < wg.weapons.size(); w++, wi++)
                {
                    weapons[wi].Type  = wg.type;
                    weapons[wi].ID    = w;
                    weapons[wi].Group = group;
                    weapons[wi].Pos   = wg.weapons[w].Pos;
                    weapons[wi].Normal= wg.weapons[w].Normal;
                }
            }
            }                           ///////////////
            break;
        case NEW_WEAP_ID:
            fread(&i, 4, 1, f);
            weapons.clear();
            weapons.insert(weapons.end(), i, Weapon());
            for (int w=0; w < i; w++)
            {
                weapons[w].Read(f);
            }
            break;
        case THRUST_ID:
            fread(&i, 4, 1, f);
            thrusters.clear();
            thrusters.insert(thrusters.begin(), i, Thruster());
            for (int t=0; t < i; t++)
            {
                thrusters[t].Read(f);
            }
            break;
        case SPCL_ID:
            fread(&i, 4, 1, f);
            Specials.clear();
            Specials.insert(Specials.begin(), i, Special());
            for (int s=0; s < i; s++)
            {
                Specials[s].Read(f);
            }
            break;
        default:
            ErrorMsg("Unrecognized chunk (ID=0x%X) in project file", chunk);
            fclose(f);
            return false;
        }
    }

    fclose(f);
    return true;
}

bool Mesh::Write(FILE *file)
{
    int nverts = verts.size();
    int nfaces = faces.size();
    int i,j;

    // Save header info
    fwrite(&lod, sizeof lod, 1, file);
    fwrite(&parent, sizeof parent, 1, file);
    fwrite(&type, sizeof type, 1, file);
    i = name.Length();
    fwrite(&i, sizeof i, 1, file);
    fwrite(name.c_str(), i+1, 1, file);
    fwrite(&offset, sizeof offset, 1, file);
    i = props.Length();
    fwrite(&i, sizeof i, 1, file);
    fwrite(props.c_str(), i+1, 1, file);

    // Save vertices
    fwrite(&nverts, sizeof nverts, 1, file);
    for (i=0; i<nverts; i++)
    {
        fwrite(&verts[i], sizeof(Vector), 1, file);
    }

    // Save faces
    fwrite(&nfaces, sizeof nfaces, 1, file);
    for (i=0; i<nfaces; i++)
    {
        fwrite(&faces[i].texture, sizeof(int), 1, file);
        fwrite(&faces[i].nverts, sizeof(int), 1, file);
        for (j=0; j<faces[i].nverts; j++)
        {
            fwrite(&faces[i].verts[j], sizeof(int), 1, file);
            fwrite(&faces[i].texverts[j], sizeof(TexVert), 1, file);
        }
    }

    return true;
}

bool Mesh::Read(FILE *file)
{
    int nverts;
    int nfaces;
    Vector *v;
    Face *f;
    int i;
    char str[256];

    // Load header info
    fread(&lod, sizeof lod, 1, file);
    fread(&parent, sizeof parent, 1, file);
    fread(&type, sizeof type, 1, file);
    fread(&i, sizeof i, 1, file);
    fread(str, i+1, 1, file);
    name = str;
    fread(&offset, sizeof offset, 1, file);
    fread(&i, sizeof i, 1, file);
    fread(str, i+1, 1, file);
    props = str;

    // Load vertices
    fread(&nverts, sizeof nverts, 1, file);
    verts.reserve(nverts); // ensure capacity size
    while (nverts)
    {
        v = verts.insert(verts.end(), Vector());
        fread(v, sizeof(Vector), 1, file);
        nverts--;
    }

    // Load faces
    fread(&nfaces, sizeof nfaces, 1, file);
    faces.reserve(nfaces); // ensure capacity size
    while (nfaces)
    {
        f = faces.insert(faces.end(), Face());
        fread(&f->texture, sizeof(int), 1, file);
        fread(&f->nverts, sizeof(int), 1, file);
        for (i=0; i<f->nverts; i++)
        {
            fread(&f->verts[i], sizeof(int), 1, file);
            fread(&f->texverts[i], sizeof(TexVert), 1, file);
        }
        nfaces--;
    }

    calc_face_norms();

    return true;
}

int Mesh::find_vert(float px, float py, float pz)
{
    int n=-1;
    unsigned int i;

    for (i=0; i<verts.size(); i++)
    {
        if ((fabs(px-verts[i].x) < 1e-4f) &&
            (fabs(py-verts[i].y) < 1e-4f) &&
            (fabs(pz-verts[i].z) < 1e-4f))
        {
            n = i;
        }
    }

    return n;
}

int Mesh::add_vert(float px, float py, float pz, bool check)
{
    Vector &v = *verts.insert(verts.end(), Vector());
    v.x = px;
    v.y = py;
    v.z = pz;

    return (verts.size()-1);
}

int Model::find_mesh(const AnsiString nm, int lod)
{
    // if lod >= 0, then only check that lod
    // else check all meshes

    int n=-1;
    unsigned int i;

    for (i=0; i<meshes.size(); i++)
    {
        if ((lod>=0) && (meshes[i].lod != lod)) continue;
        if (nm == meshes[i].name)
        {
            n = i;
        }
    }

    return n;
}

int Model::add_mesh(const AnsiString &nm, int lod, char typ)
{
    Mesh &m = *meshes.insert(meshes.end(), Mesh());

    m.name = nm;
    m.lod = lod;
    m.type = typ;

    return (meshes.size()-1);
}

void Mesh::get_minmax()
{
    max.x = max.y = max.z = -999999.0f;
    min.x = min.y = min.z =  999999.0f;

    unsigned int i;
    for (i=0; i<verts.size(); i++)
    {
        if (verts[i].x > max.x) max.x = verts[i].x;
        if (verts[i].y > max.y) max.y = verts[i].y;
        if (verts[i].z > max.z) max.z = verts[i].z;

        if (verts[i].x < min.x) min.x = verts[i].x;
        if (verts[i].y < min.y) min.y = verts[i].y;
        if (verts[i].z < min.z) min.z = verts[i].z;
    }
}

void Model::get_minmax(bool with_shields)
{
    max.x = max.y = max.z = -999999.0f;
    min.x = min.y = min.z =  999999.0f;

    Vector in, ax;

    unsigned int i;
    for (i=0; i<meshes.size(); i++)
    {
        meshes[i].get_minmax();
        in = ax = get_mesh_off(i, -1);
        in += meshes[i].min;
        ax += meshes[i].max;

        if (ax.x > max.x) max.x = ax.x;
        if (ax.y > max.y) max.y = ax.y;
        if (ax.z > max.z) max.z = ax.z;

        if (in.x < min.x) min.x = in.x;
        if (in.y < min.y) min.y = in.y;
        if (in.z < min.z) min.z = in.z;
    }

    if (with_shields == true)
    {
        shields.get_minmax();
        in = shields.min;
        ax = shields.max;

        if (ax.x > max.x) max.x = ax.x;
        if (ax.y > max.y) max.y = ax.y;
        if (ax.z > max.z) max.z = ax.z;

        if (in.x < min.x) min.x = in.x;
        if (in.y < min.y) min.y = in.y;
        if (in.z < min.z) min.z = in.z;
    }
}
//---------------------------------------------------------------------------

int Model::get_count_of(char type)
{
    int n=0;
    std::vector<Mesh>::iterator i;

    for (i=meshes.begin(); i!=meshes.end(); i++)
    {
        if (i->type == type) n++;
    }

    return n;
}

//---------------------------------------------------------------------------
void GenSphere(Mesh *msh, Vector cent, Vector scal, int hdiv, int vdiv)
{
    // Init our math crap
	int horzLoop=hdiv;
	int vertLoop=vdiv;
	int lastFace=horzLoop*vertLoop;
	float dTheta=360.0f/(float)horzLoop;
	float halfTheta=dTheta/2.0f;
	float Theta=halfTheta;
	float dPhi=180.0f/(float)vertLoop;
	float halfPhi=dPhi/2.0f;
	float Phi=halfPhi;
	float sinPhi, cosPhi;

	// Init our process data
	Face *face;
	int horz=1;
	int vert=1;
	int nface;

    Vector verts[4];
    int vs[4], v;

	for (nface=0;nface<lastFace;++nface)
	{
		sinPhi = sin((Phi+halfPhi)*DRAD);
		cosPhi = cos((Phi+halfPhi)*DRAD);

        verts[0].x = sinPhi*cos((Theta-halfTheta)*DRAD);
        verts[0].y = sinPhi*sin((Theta-halfTheta)*DRAD);
		verts[0].z = cosPhi;
        verts[0] *= scal;
        verts[0] += cent;

		verts[1].x = sinPhi*cos((Theta+halfTheta)*DRAD);
        verts[1].y = sinPhi*sin((Theta+halfTheta)*DRAD);
        verts[1].z = cosPhi;
        verts[1] *= scal;
        verts[1] += cent;

		sinPhi = sin((Phi-halfPhi)*DRAD);
		cosPhi = cos((Phi-halfPhi)*DRAD);

		verts[2].x = sinPhi*cos((Theta+halfTheta)*DRAD);
        verts[2].y = sinPhi*sin((Theta+halfTheta)*DRAD);
        verts[2].z = cosPhi;
        verts[2] *= scal;
        verts[2] += cent;

		verts[3].x = sinPhi*cos((Theta-halfTheta)*DRAD);
        verts[3].y = sinPhi*sin((Theta-halfTheta)*DRAD);
        verts[3].z = cosPhi;
        verts[3] *= scal;
        verts[3] += cent;

        for (v=0; v<4; v++)
        {
//            InfoMsg("   v[%d] = <%g, %g, %g>", v, verts[v].x, verts[v].y, verts[v].z);
            vs[v] = msh->find_vert(verts[v].x, verts[v].y, verts[v].z);
            if (vs[v] < 0)
                vs[v] = msh->add_vert(verts[v].x, verts[v].y, verts[v].z, false);
        }

        //InfoMsg("f%d: %d, %d, %d, %d", nface, vs[0], vs[1], vs[2], vs[3]);

        // We have to watch for end-caps
        if (vs[0] == vs[1])
        {
            face = msh->faces.insert(msh->faces.end(), Face());
            face->texture = 0;
            face->nverts = 3;
            face->verts[0] = vs[1];
            face->verts[1] = vs[2];
            face->verts[2] = vs[3];
//            InfoMsg("Added Face (%d, %d, %d)", vs[1], vs[2], vs[3]);
        }
        else if (vs[2] == vs[3])
        {
            face = msh->faces.insert(msh->faces.end(), Face());
            face->texture = 0;
            face->nverts = 3;
            face->verts[0] = vs[0];
            face->verts[1] = vs[1];
            face->verts[2] = vs[2];
//            InfoMsg("Added Face (%d, %d, %d)", vs[0], vs[1], vs[2]);
        }
        else
        {
            face = msh->faces.insert(msh->faces.end(), Face());
            face->texture = 0;
            face->nverts = 3;
            face->verts[0] = vs[0];
            face->verts[1] = vs[1];
            face->verts[2] = vs[2];
//            InfoMsg("Added Face (%d, %d, %d)", vs[0], vs[1], vs[2]);
            face = msh->faces.insert(msh->faces.end(), Face());
            face->texture = 0;
            face->nverts = 3;
            face->verts[0] = vs[0];
            face->verts[1] = vs[2];
            face->verts[2] = vs[3];
//            InfoMsg("Added Face (%d, %d, %d)", vs[0], vs[2], vs[3]);
        }

		Theta += dTheta;
		horz++;
		if (horz>horzLoop)
		{
			horz = 1;
			vert++;
			Theta = halfTheta;
			Phi += dPhi;
		}
	}

    //ErrorMsg("f: %u, v: %u", msh->faces.size(), msh->verts.size());
}
//---------------------------------------------------------------------------

void Mesh::calc_face_norms()
{
    vector<Face>::iterator f;
    Vector u,v;

    for (f=faces.begin(); f!=faces.end(); f++)
    {
        u = verts[f->verts[1]] - verts[f->verts[0]];
        v = verts[f->verts[2]] - verts[f->verts[0]];
        Vector::cross(f->norm, u, v);

        f->norm.normalize();
    }
}

void Mesh::calc_face_centers()
{
    vector<Face>::iterator f;
    Vector c;
    int k;

    for (f=faces.begin(); f!=faces.end(); f++)
    {
        if ((*f).nverts < 1) continue;
        (*f).center = verts[(*f).verts[0]];

        for (k=1; k < (*f).nverts; k++)
        {
            (*f).center += verts[(*f).verts[k]];
        }

        (*f).center *= (1.0 / (float)(*f).nverts);
    }
}
//---------------------------------------------------------------------------
float Mesh::get_surfacearea()
{
    float area = 0.0f;

    vector<Face>::iterator f;
    for (f=faces.begin(); f!=faces.end(); f++)
    {
        area += GetFaceArea(*f);
    }

    return area;
}

float Mesh::GetFaceArea(const Face &f)
{
    static Vector u,v,c;
    float area = 0.0f;

    // For each face, break it into triangles and find the area of each of
    // those triangles.
    for (int j=1, k=2; j < (f.nverts-1); j++, k++)
    {
        u = verts[f.verts[0]] - verts[f.verts[j]];
        v = verts[f.verts[k]] - verts[f.verts[j]];
        Vector::cross(c, u, v);

        area += (c.length()/2.0f); // area = |UxV|/2
    }

    return area;
}

float Mesh::get_mass()
{
    // The divied by 2.5278 is experimental and only approximate.  Who knows what
    // the true algorithm looks like.
    return (get_surfacearea() / 2.5278f);
}
//---------------------------------------------------------------------------

void Model::recalc_all()
{
    get_minmax();

    dMin = min.min_val();
    dMax = max.max_val();

    vector<Mesh>::iterator m;
    for (m=meshes.begin(); m!=meshes.end(); m++)
    {
        m->calc_face_norms();
    }
}
//---------------------------------------------------------------------------

Vector StrToVec(AnsiString s)
{
    if (s.Length() < 1) return Vector();
    
    char str[64];
    char *p;
    Vector v;

    strcpy(str, s.c_str());

    p = strtok(str, "<> ,");
    if (p) v.x = atof(p);

    p = strtok(NULL, "<> ,");
    if (p) v.y = atof(p);

    p = strtok(NULL, "<> ,");
    if (p) v.z = atof(p);

    return v;
}
//---------------------------------------------------------------------------

Vector Model::get_mesh_off(int m, int parent)
{
    if (parent == m) return Vector();

    //int sz = (int)meshes.size();

    Vector off = meshes[m].offset;
    int par = meshes[m].parent;

    while ((par != -1) && (par != parent) && (par < (int)meshes.size()))
    {
        if (par == meshes[par].parent)
            break;
        if (meshes[par].parent != -1)
        {
            if (par == meshes[meshes[par].parent].parent)
            break;
        }

        off += meshes[par].offset;
        par  = meshes[par].parent;
    }

    return off;
}
//---------------------------------------------------------------------------

int Mesh::invert_face(int n)
{
    // This also changes the texture vertices
    Face *f = &faces[n];
    int oldverts[GEO_MAXVERTS];
    TexVert ttmp[GEO_MAXVERTS];
    int nv = f->nverts;

    memcpy(oldverts, f->verts, 4*nv);
    memcpy(ttmp, f->texverts, sizeof(TexVert)*nv);
    while (nv > 0)
    {
        f->verts[f->nverts-nv] = oldverts[nv-1];
        f->texverts[f->nverts-nv] = ttmp[nv-1];
        nv--;
    }

    // Re-calc its normal
    Vector u = verts[f->verts[0]] - verts[f->verts[1]];
    Vector v = verts[f->verts[2]] - verts[f->verts[1]];
    Vector::cross(f->norm, v, u);
    f->norm.normalize();

    return 1;
}
//---------------------------------------------------------------------------
// returns the old center of the mesh
Vector Mesh::center()
{
    if (verts.size() < 1) return Vector();

    get_minmax();
    Vector cen = (min + max) * 0.5f;

    vector<Vector>::iterator i = verts.begin();
    for (; i != verts.end(); i++)
    {
        *i -= cen;
    }

    return cen;
}
//---------------------------------------------------------------------------
void Model::change_mesh_off(int m, const Vector &off)
{
    if (m < 0) return;
    if (m >= (int)meshes.size()) return;

    // Move this mesh
    meshes[m].offset += off;
}
//---------------------------------------------------------------------------

void Face::tex_rot(float ang)
{
    int i;
    TexVert v0 = texverts[0];
    TexVert v,n;
    float ca, sa;

    ca = cos(ang*DRAD);
    sa = sin(ang*DRAD);

    //[ cos(a)*x-sin(a)*y]
    //[ sin(a)*x+cos(a)*y]

    for (i=1; i<nverts; i++)
    {
        v.u = texverts[i].u - v0.u;
        v.v = texverts[i].v - v0.v;

        n.u = ca*v.u - sa*v.v + v0.u;
        n.v = sa*v.u + ca*v.v + v0.v;

        texverts[i] = n;
    }
}
//---------------------------------------------------------------------------

void Face::tex_scale(float val)
{
    int i;
    TexVert v0 = texverts[0];
    TexVert v,n;

    for (i=1; i<nverts; i++)
    {
        v.u = texverts[i].u - v0.u;
        v.v = texverts[i].v - v0.v;

        n.u = v.u*val + v0.u;
        n.v = v.v*val + v0.v;

        texverts[i] = n;
    }
}
//---------------------------------------------------------------------------

int Mesh::calc_face_text(int n)
{
    const float ppUnit = 1.0f/8.0f;//16.0f; // pixels per unit

    Face *f = &faces.at(n);
    Vector u,v;

    // calc axi
    u = verts.at(f->verts[1]) - verts.at(f->verts[0]);
    u.normalize();
    Vector::cross(v, faces.at(n).norm, u);

    // set each vert's tex
    f->texverts[0].u = f->texverts[0].v = 0.0f;
    Vector vx = verts[f->verts[0]];
    TexVert tv = f->texverts[0];
    for (int i=0; i<f->nverts; i++)
    {
        f->texverts[i].u = tv.u+Vector::dot(u, verts[f->verts[i]]-vx)*ppUnit;
        f->texverts[i].v = tv.v+Vector::dot(v, verts[f->verts[i]]-vx)*ppUnit;
        //InfoMsg("Face %d, Coordinate %d == (%f, %f)", n, i, f->texverts[i].u,f->texverts[i].v);
    }

    return 1;
}
//---------------------------------------------------------------------------

AnsiString VecToStr(Vector v)
{
    AnsiString s = "<"  + FormatFloat("####0.#####", v.x) +
                   ", " + FormatFloat("####0.#####", v.y) +
                   ", " + FormatFloat("####0.#####", v.z) + ">";
    return s;
}
//---------------------------------------------------------------------------

int Model::delete_mesh(int m)
{
    if ((m < 0) || (m >= (int)meshes.size()))
    {
        ErrorMsg("Attempted to delete inexistant mesh %d", m);
        return 0;
    }

    // we do not break the chain by assigning all children to
    // this mesh's parent
    int par = meshes[m].parent;
    unsigned int i;

    meshes.erase(&meshes[m]);

    for (i=0; i<meshes.size(); i++)
    {
        if (meshes[i].parent == m) meshes[i].parent = par;
    }

    return 1;
}
//---------------------------------------------------------------------------

int find_cocentric_face(Face &f, vector<FaceList> &List)
{
    static int i;
    Vector d;

    for (i=0; i < (int)List.size(); i++)
    {
        d = f.center - List[i].at(0)->center;
        if (d.length_sqrd() < 0.0001)
            return i;
    }
    return -1;
}

//---------------------------------------------------------------------------
void WeaponGroup::Write(FILE *f)
{
    /*fwrite(&type, 4, 1, f);
    int i = int(weapons.size());
    fwrite(&i, 4, 1, f);
    for (i=0; i < int(weapons.size()); i++)
    {
        fwrite(&weapons[i].pos, sizeof(Vector), 1, f);
        fwrite(&weapons[i].norm, sizeof(Vector), 1, f);
    } */
}
void WeaponGroup::Read(FILE *f)
{
    weapons.clear();

    fread(&type, 4, 1, f);
    int num;
    fread(&num, 4, 1, f);
    while (num > 0)
    {
        weapons.push_back(Weapon());
        fread(&weapons.back().Pos, sizeof(Vector), 1, f);
        fread(&weapons.back().Normal, sizeof(Vector), 1, f);
        num--;
    }
}
//---------------------------------------------------------------------------
void Vector::Rotate(float rx, float ry, float rz)
{
    float cosa, sina;
    float dx, dy, dz;

    /* Around X */
    cosa = cos(rx*PiOver180);
    sina = sin(rx*PiOver180);
    dx   = x;
    dy   = cosa*y - sina*z;
    dz   = sina*y + cosa*z;

    /* Around Y */
    cosa = cos(ry*PiOver180);
    sina = sin(ry*PiOver180);
    x    = cosa*dx + sina*dz;
    y    = dy;
    z    = -sina*dx + cosa*dz;

    /* Around Z */
    cosa = cos(rz*PiOver180);
    sina = sin(rz*PiOver180);
    dx   = cosa*x - sina*y;
    dy   = sina*x + cosa*y;
    dz   = z;

    x = dx;
    y = dy;
    z = dz;
}
//---------------------------------------------------------------------------
float Mesh::CalcRadiusFromPoint(const Vector &pt)
{
    if (verts.size() < 1) return 0.0f;

    float rad = (verts[0] - pt).length();
    float temp;

    for (size_t i=1; i < verts.size(); i++)
    {
        temp = (verts[i] - pt).length();
        if (temp > rad)
            rad = temp;
    }

    return rad;
}
//---------------------------------------------------------------------------
float Model::CalcRadiusFromPoint(const Vector &pt, bool with_shields)
{
    float rad = 0.0f;
    float temp;

    for (size_t m=0; m < meshes.size(); m++)
    {
        Vector off = get_mesh_off(m, -1);
        for (size_t v=0; v < meshes[m].verts.size(); v++)
        {
            temp = (meshes[m].verts[v] + off - pt).length();
            if (temp > rad)
                rad = temp;
        }
    }

    if (with_shields == true)
    {
        for (size_t v=0; v < shields.verts.size(); v++)
        {
            temp = (shields.verts[v] - pt).length();
            if (temp > rad)
                rad = temp;
        }
    }

    return rad;
}
//---------------------------------------------------------------------------
Vector Mesh::GetVertNormal(const int Vert) const
{
    Vector norm;
    int count = 0;

    for (size_t i=0; i < faces.size(); i++)
    {
        for (int k=0; k < faces[i].nverts; k++)
        {
            if (k == Vert)
            {
                norm += faces[i].norm;
                count++;
            }
        }
    }

    if (count == 0) return Vector();

    norm /= float(count);
    return norm;
}
//---------------------------------------------------------------------------
size_t Model::FindMaterial(const std::string &Mat)
{
    for (size_t i=0; i < mats.size(); i++)
    {
        if (stricmp(Mat.c_str(), mats[i].s.c_str()) == 0)
            return i;
    }
    return (size_t(-1));
}
//---------------------------------------------------------------------------
void Mesh::Triangulate()
{
    size_t f = 0;
    while (f < faces.size())
    {
        if (faces[f].nverts < 4)
        {
            f++;
        }
        else
        {
            for (int i=1; i < (faces[f].nverts-1); i++)
            {
                faces.push_back(faces[f]);
                faces.back().nverts = 3;
                faces.back().verts[0] = faces[f].verts[0];
                faces.back().verts[1] = faces[f].verts[i];
                faces.back().verts[2] = faces[f].verts[i+1];
                faces.back().texverts[0] = faces[f].texverts[0];
                faces.back().texverts[1] = faces[f].texverts[i];
                faces.back().texverts[2] = faces[f].texverts[i+1];
            }
            faces.erase(faces.begin()+f);
        }
    }
}
//---------------------------------------------------------------------------
void Thruster::Write(FILE *f)
{
    // Number of glows
    size_t ng = glows.size();
    fwrite(&ng, 4, 1, f);

    // Properties
    int i = Props.Length();
    fwrite(&i, sizeof i, 1, f);
    fwrite(Props.c_str(), i+1, 1, f);

    // GLows
    for (size_t g=0; g < ng; g++)
    {
        glows[g].Write(f);
    }
}
//---------------------------------------------------------------------------
void Thruster::Read(FILE *f)
{
    static char str[256];

    size_t ng;
    fread(&ng, 4, 1, f);

    int i;
    fread(&i, sizeof i, 1, f);
    fread(str, i+1, 1, f);
    Props = str;

    glows.clear();
    glows.insert(glows.begin(), ng, Glow());
    for (size_t g=0; g < ng; g++)
    {
        glows[g].Read(f);
    }
}
//---------------------------------------------------------------------------

#pragma package(smart_init)
