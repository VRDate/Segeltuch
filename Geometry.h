//---------------------------------------------------------------------------
#ifndef GeometryH
#define GeometryH

#include <vector> // dynamic array
#include <stdio.h>

#define GEO_MAXVERTS 32
#define GEO_MAGIC    0x46584446

#define MT_HULL   1
#define MT_DEBRIS 2

#define WT_UNDEF   0
#define WT_GUN     1
#define WT_MISSILE 2

struct ImgString
{
    AnsiString s;
    void *image;

    ImgString() : s(""), image(0)
    {}
    ImgString(const ImgString &rhs) : s(rhs.s), image(rhs.image)
    {}
    ~ImgString() // Note: We do NOT free the image!
    {}
};

struct Vector
{
    float x,y,z;

    float length() const;
    float length_sqrd() const { return (x*x+y*y+z*z); }
    Vector& normalize();
    void Rotate(float rx, float ry, float rz); // angles given in º (right-handed)

    static float dot(const Vector &v1, const Vector &v2);
    static void cross(Vector &r, const Vector &v1, const Vector &v2);
    Vector GetAlmostOrtho() const
    {
        if ((x >= y) && (x >= z))
            return Vector(0.0f, 1.0f, 0.0f);
        else if ((y >= x) && (y >= z))
            return Vector(0.0f, 0.0f, 1.0f);
        else
            return Vector(1.0f, 1.0f, 0.0f);
    }

    // Le Constructor
    Vector() : x(0.0f), y(0.0f), z(0.0f)
    {}
    Vector(const Vector &v) : x(v.x), y(v.y), z(v.z)
    {}
    explicit Vector(float a, float b, float c) : x(a), y(b), z(c)
    {}


    // Assignment
    Vector& operator=(const float s) { x=s; y=s; z=s; return *this; }
    Vector& operator=(const Vector &v) { x=v.x; y=v.y; z=v.z; return *this; }
    Vector& operator=(const float *v) { x=v[0]; y=v[1]; z=v[2]; return *this; }
    Vector& operator+=(const Vector &v) { x+=v.x; y+=v.y; z+=v.z; return *this; }
    Vector& operator-=(const Vector &v) { x-=v.x; y-=v.y; z-=v.z; return *this; }
    Vector& operator*=(const Vector &v) { x*=v.x; y*=v.y; z*=v.z; return *this; }
    Vector& operator*=(const float s) { x*=s; y*=s; z*=s; return *this; }
    Vector& operator/=(const float s) { x/=s; y/=s; z/=s; return *this; }

    // Math/Uniary
    Vector operator-() { Vector t; t.x = -x; t.y = -y; t.z = -z; return t; }
    friend Vector operator+(Vector v1, Vector v2) { Vector t; t.x = v1.x+v2.x; t.y = v1.y+v2.y; t.z = v1.z+v2.z; return t; }
    friend Vector operator-(Vector v1, Vector v2) { Vector t; t.x = v1.x-v2.x; t.y = v1.y-v2.y; t.z = v1.z-v2.z; return t; }
    friend Vector operator*(Vector v, float s) { Vector t; t.x = v.x*s; t.y = v.y*s; t.z = v.z*s; return t; }
    friend Vector operator*(float s, Vector v) { Vector t; t.x = v.x*s; t.y = v.y*s; t.z = v.z*s; return t; }
    friend Vector operator/(Vector v, float s) { Vector t; t.x = v.x/s; t.y = v.y/s; t.z = v.z/s; return t; }

    float max_val();
    float min_val();
};

struct Transformation {
    Vector Dir, Up, Right;
    Vector Pos;

    void ProjectPoint(const Vector &Pt, Vector &NewPt);
    void UnProjectPoint(const Vector &Pt, Vector &NewPt);
};

struct Glow
{
    Vector pos;
    Vector norm;
    float radius;

    void Write(FILE *f)
    {
        fwrite(&pos, sizeof(Vector), 1, f);
        fwrite(&norm, sizeof(Vector), 1, f);
        fwrite(&radius, 4, 1, f);
    }
    void Read(FILE *f)
    {
        fread(&pos, sizeof(Vector), 1, f);
        fread(&norm, sizeof(Vector), 1, f);
        fread(&radius, 4, 1, f);
    }
};

struct Thruster
{
    std::vector<Glow> glows;
    AnsiString Props;

    void Write(FILE *f);
    void Read(FILE *f);
};

struct TexVert
{
    float u,v;

    TexVert(float InU=0.0f, float InV=0.0f) : u(InU), v(InV)
    {}
};

class Weapon {
public:
    Vector Pos;
    Vector Normal;
    int Type;
    int Group;
    int ID;

    Weapon() : Type(WT_GUN), Group(0), ID(0)
    {}
    void Write(FILE *f)
    {
        fwrite(&Pos, sizeof(Vector), 1, f);
        fwrite(&Normal, sizeof(Vector), 1, f);
        fwrite(&Type, 4, 1, f);
        fwrite(&Group, 4, 1, f);
        fwrite(&ID, 4, 1, f);
    }
    void Read(FILE *f)
    {
        fread(&Pos, sizeof(Vector), 1, f);
        fread(&Normal, sizeof(Vector), 1, f);
        fread(&Type, 4, 1, f);
        fread(&Group, 4, 1, f);
        fread(&ID, 4, 1, f);
    }
};

//// Get rid of as soon as possible
struct WeaponGroup
{
    int type;
    std::vector<Weapon> weapons;

    WeaponGroup() : type(WT_UNDEF)
    {}

    void Read(FILE *f);
    void Write(FILE *f);
};
////

class Special
{
public:
    AnsiString Name;
    AnsiString Props;
    Vector     Pos;
    float      Radius;

    Special() : Name(""), Props(""), Pos(), Radius(0.0f)
    {}

    void Write(FILE *f)
    {
        int i = Name.Length();
        fwrite(&i, sizeof(i), 1, f);
        fwrite(Name.c_str(), i+1, 1, f);
        i = Props.Length();
        fwrite(&i, sizeof(i), 1, f);
        fwrite(Props.c_str(), i+1, 1, f);
        fwrite(&Pos, sizeof(Vector), 1, f);
        fwrite(&Radius, 4, 1, f);
    }
    void Read(FILE *f)
    {
        int i; char str[256];
        fread(&i, sizeof(i), 1, f);
        fread(str, i+1, 1, f);
        Name = str;
        fread(&i, sizeof(i), 1, f);
        fread(str, i+1, 1, f);
        Props = str;
        fread(&Pos, sizeof(Vector), 1, f);
        fread(&Radius, 4, 1, f);
    }
};

struct Face
{
    int texture;
    int nverts;
    int verts[GEO_MAXVERTS];
    TexVert texverts[GEO_MAXVERTS];
    Vector norm;
    Vector center;

    // for use during POF Conversion
    int norms[GEO_MAXVERTS];

    // fun texturing stuff
    void tex_rot(float ang);
    void tex_scale(float val);
};
typedef std::vector<Face*> FaceList;

struct EdgeReference {
    INT StartVert;
    INT EndVert;

};

struct Mesh
{
    // Modifiers
    int lod;       // index
    int parent;    // index
    char type;     // MT_*
    AnsiString name;
    AnsiString props;
    Vector offset; // offset from parent

    // Physical
    std::vector<Vector> verts;
    std::vector<Face> faces;

    // Size
    Vector min;
    Vector max;

    int find_vert(float px, float py, float pz);
    int add_vert(float px, float py, float pz, bool check);
    Vector center();

    int invert_face(int n);
    int calc_face_text(int n);

    void get_minmax();
    void calc_face_norms();
    void calc_face_centers();
    float get_surfacearea();
    float get_mass();
    float GetFaceArea(const Face &f);
    float CalcRadiusFromPoint(const Vector &pt);
    Vector GetVertNormal(const int Vert) const;
    void Triangulate();

    bool Write(FILE *f);
    bool Read(FILE *f);
    bool LoadFromDXF(const AnsiString &fn, bool NewName);
    void Clear();
};

struct Model
{
    std::vector<ImgString> mats;
    std::vector<Mesh> meshes;
    std::vector<Weapon> weapons;
    std::vector<Thruster> thrusters;
    std::vector<Special> Specials;
    Mesh shields;

    int nlods; // a helper to know how many LODS
    bool mod;  // modified
    AnsiString name;
    AnsiString filename;

    // Size
    Vector min;
    Vector max;
    double dMin, dMax;

    int find_mesh(const AnsiString nm, int lod);
    int add_mesh(const AnsiString &nm, int lod, char typ);
    int delete_mesh(int m);
    int get_count_of(char type);
    Vector get_mesh_off(int m, int parent);
    void change_mesh_off(int m, const Vector &off);
    size_t FindMaterial(const std::string &Mat);

    void get_minmax(bool with_shields=false);
    float CalcRadiusFromPoint(const Vector &pt, bool with_shields);
    void recalc_all();

    bool Write();
    bool Read();
    bool ReadDXF(AnsiString fn, int lod, char typ);
    bool AddMeshFromDXF(const AnsiString &fn, int parent, int lod, char type);
    bool AddMeshesFrom3DS(const AnsiString &fn, int parent, int lod, char type);
    bool AddMeshesFromCOB(const AnsiString &fn, int parent, int lod, char type);
    void Clear();

    bool MakePOF();
};

class BspHull
{
public:
    Vector min, max;
    std::vector<Face*> face_list;

    void split(BspHull *back, BspHull *front, Vector *norm, Vector *center) const;
    void split_n(BspHull *back, BspHull *front, Vector *norm, Vector *center,
                 char n) const;
    bool  is_point_inside(const Vector &p);
    bool  is_face_inside(Mesh *msh, Face *f);
    bool  is_face_partially_inside(Mesh *msh, Face *f);
    float calc_amount_face_is_inside(Mesh *msh, Face *f);

    bool addto_uniquelist(Mesh *msh, Face *f, std::vector<FaceList> &UniqueList);
};

//---------------------------------------------------------------------------
extern Model Proj;

// primitive builders
void GenSphere(Mesh *msh, Vector cent, Vector scal, int hdiv, int vdiv);

//helpers
Vector StrToVec(AnsiString s);
AnsiString VecToStr(Vector v);
extern Vector PlaneLineInter(const Vector &s, const Vector &d,
                             const Vector &n, const Vector &p);

extern int find_cocentric_face(Face &f, std::vector<FaceList> &UniqueList);

//---------------------------------------------------------------------------
#endif
