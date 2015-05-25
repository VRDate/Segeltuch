//---------------------------------------------------------------------------
#ifndef PofH
#define PofH

#define POF_MAXSTRING    256
#define POF_MAXNORMS     32
#define POF_MAXPOLYVERTS 32

#include <vector>
#include <stdio.h>

//---------------------------------------------------------------------------
// Basic components of POFs
struct pof_vector {
    float x, y, z;
    float &operator [] (int n) { return *(&x + n); }
    operator float* () { return &x; }
    //operator const float *const () const { return &x; }
};

struct pof_string
{
    int length;
    char s[POF_MAXSTRING];

    bool read(FILE *file);
};

struct pof_tpolyvert
{
    short num;
    short normnum;
    float u,v;
};

struct pof_tpoly
{
    pof_vector normal;
    pof_vector center;
    float radius;
    int nverts;
    int tmap_num;

    pof_tpolyvert verts[POF_MAXPOLYVERTS];
};

struct pof_shield
{
    std::vector<pof_vector> verts;
    std::vector<pof_tpoly> faces;
};

// Headers and "chunks"
struct pof_fileheader
{
    int file_id;
    int version;

    bool read(FILE *file);
};

struct pof_chunkheader
{
    int chunk_id;
    int length;

    bool read(FILE *file);
};

struct pof_crosssection
{
    float depth;
    float radius;
};

struct pof_light
{
    pof_vector location;
    int type;
};

struct pof_objectheader  // HDR2
{
    float max_radius;        // maximum radius of entire ship
    int obj_flags;           // object flags. Bit 1 = Textures contain tiling

    int num_subobjects;      // number of subobjects

    pof_vector min_bounding;     // min bounding box point
    pof_vector max_bounding;     // max boundingbox point

    int num_detaillevels;    // number of detail levels
    int *sobj_detaillevels;  // subobject number for detail level I, 0 being highest.

    int num_debris;          // number of debris pieces that model explodes into
    int *sobj_debris;        // subobject number for debris piece i

// if version1903orhigher
    float mass;                 // see notes below
    pof_vector mass_center;         // center of mass
    float moment_inertia[3][3]; // moment of inertia
// endif

// if version2014orhigher
    int num_cross_sections;     // number of cross sections (used for exploding ship)
    pof_crosssection *cross_sections;
// endif

// if version2007orhigher
    int num_lights;             // number of precalculated muzzle flash lights
    pof_light *lights;
// endif

    pof_objectheader();
    ~pof_objectheader();
    bool read(FILE *file);
};

struct pof_txtr_chunk  // TXTR
{
    std::vector<pof_string> mats;

    bool read(FILE *file);
};

struct pof_sobj_chunk  // OBJ2
{
    int submodel_number;  // What submodel number this is.
    int submodel_parent;  // What submodel is this model's parent. Equal to -1 if none.
    pof_vector offset;        // Offset to from parent object <- Added 09/10/98
    float radius;         // radius of this subobject

    pof_vector geometric_center;
    pof_vector bounding_box_min_point;
    pof_vector bounding_box_max_point;

    pof_string submodel_name;
    pof_string properties;
    int movement_type;
    int movement_axis;

    int reserved;         // must be 0
    std::vector<char> bsp_data;             // contains actual polygons, etc.

    pof_sobj_chunk()
    {}
    ~pof_sobj_chunk()
    {}
    bool read(FILE *file);
};

struct pof_glow
{
    pof_vector pos;
    pof_vector norm;
    float radius;
};

struct pof_thruster
{
    std::vector<pof_glow> glows;
    pof_string properties;
};

struct pof_weapon_slot
{
    pof_vector point;
    pof_vector norm;
};

struct pof_weapon
{
    int type;
    std::vector<pof_weapon_slot> slots;
};

struct pof_special
{
    pof_string name;
    pof_string properties;
    pof_vector point;
    float      radius;
};

struct pof_file
{
    pof_fileheader   fhdr;
    pof_objectheader ohdr;
    pof_txtr_chunk   txtrs;
    pof_shield       shlds;
    std::vector<pof_sobj_chunk> objs;
    std::vector<pof_weapon> weapons;
    std::vector<pof_thruster> thrusters;
    std::vector<pof_special> specials;

    pof_file();
    ~pof_file();
    bool read(FILE *file, long start, long end);
};

struct pof_tvert
{
    float u,v;
};

struct pof_vertex
{
    pof_vector pt;
    pof_vector nms[POF_MAXNORMS];
    char nnms;
};





struct pof_mesh
{
    std::vector<pof_vertex> verts;
    std::vector<pof_tpoly> faces;

    pof_sobj_chunk *obj;

    pof_mesh();
    ~pof_mesh();
};

//---------------------------------------------------------------------------
void parse_bsp(char *bsp, int len, pof_mesh *mesh);

bool convert_dxf(pof_file *pof, char *filename);
bool convert_cob(pof_file *pof, char *filename);
bool convert_model(pof_file *pof);

//---------------------------------------------------------------------------
#endif
