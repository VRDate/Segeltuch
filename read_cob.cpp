#include <vcl.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <fstream>
#pragma hdrstop

#include "Global.h"
#include "Geometry.h"
#include "F_Main.h"
#include "F_Select.h"

#define F_HOLE		0x08

using namespace std;

#pragma pack(push, 1)

struct COBHeader {
	char Name[9];
	char Version[6];
	char Format;
	char Endian[2];
	char Blank[13];
	char Newline;
};

struct COBChunk {
	unsigned long Type;
	short Major;
	short Minor;
	long Chunkid;
	long Parentid;
	long Size;
};

#pragma pack(pop)

static string ParseString(FILE *fp)
{
    static char big_string[256];
    short len;
    fread(&len, 2, 1, fp);
    fread(big_string, 1, len, fp);
    big_string[len] = '\0';
    return big_string;
}

struct COBLocalAxes {
    Vector Center;
    Vector LocalX;
    Vector LocalY;
    Vector LocalZ;

    bool Parse(FILE *fp)
    {
        fread(&Center, 4, 3, fp);
        fread(&LocalX, 4, 3, fp);
        fread(&LocalY, 4, 3, fp);
        fread(&LocalZ, 4, 3, fp);
        return true;
    }
};

struct COBCurPos {
    float Row1[4];
    float Row2[4];
    float Row3[4];
    float Row4[4];

    bool Parse(FILE *fp)
    {
        fread(Row1, 4, 4, fp);
        fread(Row2, 4, 4, fp);
        fread(Row3, 4, 4, fp);
        Row4[0] = Row4[1] = Row4[2] = 0.0f;
        Row4[3] = 1.0f;
        return true;
    }
};

struct COBMaterial {
    short Number;
    char  Shader, Facet, Autofacet;
    float Red, Green, Blue, Opacity;
    float Ambient, Specular, Hilight, Refraction;
    TexVert Offset;
    TexVert Repeat;
    char    Flags;
    string  Pathname;
    long    ParentID;

    size_t MatIdx; // Index into model's list

    bool Parse(COBChunk &Chunk, FILE *fp);
};

bool COBMaterial::Parse(COBChunk &Chunk, FILE *fp)
{
    // This chunk is variably sized...
    long StartPos = ftell(fp);

    fread(&Number, 2, 1, fp);
    fread(&Shader, 1, 1, fp);
    fread(&Facet, 1, 1, fp);
    fread(&Autofacet, 1, 1, fp);
    fread(&Red, 4, 1, fp);
    fread(&Green, 4, 1, fp);
    fread(&Blue, 4, 1, fp);
    fread(&Opacity, 4, 1, fp);
    fread(&Ambient, 4, 1, fp);
    fread(&Specular, 4, 1, fp);
    fread(&Hilight, 4, 1, fp);
    fread(&Refraction, 4, 1, fp);

    Offset.u = Offset.v = 0.0f;
    Repeat.u = Repeat.v = 0.0f;
    Flags = 0;
    MatIdx = 0;
    ParentID = Chunk.Parentid;

    while ((ftell(fp)-StartPos) < Chunk.Size)
    {
        unsigned short Identifier;
        char   DumbChar;
        float  DumbFloat;
        string DumbString;

        fread(&Identifier, 2, 1, fp);

        switch (Identifier)
        {
        case 'e:':
            fread(&DumbChar, 1, 1, fp);
            DumbString = ParseString(fp);
            break;
        case 't:':
            fread(&Flags, 1, 1, fp);
            Pathname = ParseString(fp);
            fread(&Offset.u, 4, 1, fp);
            fread(&Offset.v, 4, 1, fp);
            fread(&Repeat.u, 4, 1, fp);
            fread(&Repeat.v, 4, 1, fp);
            break;
        case 'b:':
            fread(&DumbChar, 1, 1, fp);
            DumbString = ParseString(fp);
            fread(&DumbFloat, 4, 1, fp);
            fread(&DumbFloat, 4, 1, fp);
            fread(&DumbFloat, 4, 1, fp);
            fread(&DumbFloat, 4, 1, fp);
            fread(&DumbFloat, 4, 1, fp);
            break;
        default:
            ErrorMsg("Unknown COB Material Type: 0x%X", int(Identifier));
            break;
        }
    }

    if (Pathname.size() < 1)
    {
        Pathname = "default";
    }
    else
    {
        // Strip the name of path info and file extensions
        size_t pos = Pathname.rfind('\\');
        if (pos != string::npos)
        {
            Pathname.erase(0, pos+1);
        }
        pos = Pathname.find('.');
        if (pos != string::npos)
        {
            Pathname.erase(Pathname.begin()+pos, Pathname.end());
        }
    }

    //InfoMsg("Material ID: 0x%X, Parent: 0x%X, Number: %d", Chunk.Chunkid, Chunk.Parentid, int(Number));

    return true;
}

struct COBMeshInfo {
    long ParentID;
    long ChunkID;

    COBMeshInfo() : ParentID(0), ChunkID(0)
    {}
};

struct COBGroup {
    COBChunk Chunk;
    short DupeCount;
    string ObjName;
    COBLocalAxes LocalAxes;
    COBCurPos CurPos;
    size_t MeshIndex;
};

static int                  mParent;
static int                  mLOD;
static char                 mType;
static Model               *Mod;
static vector<TexVert>      MapCoords;
static vector<COBMaterial>  Mats;
static vector<COBMeshInfo>  MeshInfos;
static vector<COBGroup>     Groups;
static const float          COBAutoScale = 20.0f;

static bool ParseGroup(COBChunk &Chunk, FILE *fp)
{
    Groups.push_back(COBGroup());

    // Read the NAME's DUPECOUNT
    fread(&Groups.back().DupeCount, 2, 1, fp);
    // Read the NAME
    Groups.back().ObjName = ParseString(fp);
    if (Groups.back().ObjName.size() < 1)
    {
        Groups.back().ObjName = "cob_noname";
    }
    // Read the LOCAL AXES
    Groups.back().LocalAxes.Parse(fp);
    // Read the CURRENT POSITION
    Groups.back().CurPos.Parse(fp);

    // Remember the parent hierarchy
    Groups.back().Chunk = Chunk;

    // Create the proper mesh
    Mod->meshes.push_back(Mesh());
    Mod->meshes.back().name     = Groups.back().ObjName.c_str();
    Mod->meshes.back().offset.x = -Groups.back().LocalAxes.Center.y * COBAutoScale;
    Mod->meshes.back().offset.y = Groups.back().LocalAxes.Center.z * COBAutoScale;
    Mod->meshes.back().offset.z = Groups.back().LocalAxes.Center.x * COBAutoScale;
    Mod->meshes.back().lod      = mLOD;
    Mod->meshes.back().type     = mType;
    Groups.back().MeshIndex     = Mod->meshes.size() - 1;
    if (Chunk.Parentid == 0)
    {
        Mod->meshes.back().parent   = mParent;
        if ((mParent == -1) && (mType == MT_HULL))
        {
            Mod->nlods++;
            mLOD++;
        }
    }
    else
    {
        // Find its real parent
        for (size_t i=0; i<Groups.size(); i++)
        {
            if (Groups[i].Chunk.Chunkid == Groups.back().Chunk.Parentid)
            {
                Mod->meshes.back().parent  = Groups[i].MeshIndex;
                Mod->meshes.back().lod     = Mod->meshes[Groups[i].MeshIndex].lod;
                Mod->meshes.back().offset -= Mod->get_mesh_off(Groups[i].MeshIndex, -1);
            }
        }
    }
    Mod->meshes.back().verts.clear();
    Mod->meshes.back().faces.clear();

    // Need to later correlate materials with these
    MeshInfos.push_back(COBMeshInfo());

    //InfoMsg("Grou ID: 0x%X, Parent: 0x%X, Name: %s", Chunk.Chunkid, Chunk.Parentid, Groups.back().ObjName.c_str());
    return true;
}

static bool ParsePolygonalData(COBChunk &Chunk, FILE *fp)
{
    long StartPos = ftell(fp);

    // Find this geometry's parent group
    size_t GroupIndex = size_t(-1);
    size_t CurMesh    = size_t(-1);
    int    VertsRefStart = 0;
    for (size_t i=0; i<Groups.size(); i++)
    {
        if (Groups[i].Chunk.Chunkid == Chunk.Parentid)
        {
            GroupIndex = i;
            CurMesh = Groups[i].MeshIndex;
            VertsRefStart = Mod->meshes[CurMesh].verts.size();
        }
    }
    // SKip it if it doesn't have a group parent
    if (GroupIndex == size_t(-1))
    {
        fseek(fp, Chunk.Size, SEEK_CUR);
        return false;
    }

    if (MeshInfos[CurMesh].ParentID == 0)
    {
        MeshInfos[CurMesh].ParentID = Chunk.Parentid;
        MeshInfos[CurMesh].ChunkID = Chunk.Chunkid;
    }

    // Read the NAME's DUPECOUNT
    short DupeCount;
    fread(&DupeCount, 2, 1, fp);
    // Read the NAME
    string ObjName = ParseString(fp);
    if (ObjName.size() < 1) ObjName = "cob_noname";
    // Read the LOCAL AXES
    COBLocalAxes LocalAxes;
    LocalAxes.Parse(fp);
    // Read the CURRENT POSITION
    COBCurPos CurPos;
    CurPos.Parse(fp);

    //InfoMsg("PolH ID: 0x%X, Parent: 0x%X, Name: %s", Chunk.Chunkid, Chunk.Parentid, ObjName.c_str());

    // Load the vertices
    long l;
    fread(&l, 4, 1, fp);
    Mod->meshes[CurMesh].verts.insert(Mod->meshes[CurMesh].verts.end(), l, Vector());
    float v[3];
    for (size_t i=size_t(VertsRefStart); i < Mod->meshes[CurMesh].verts.size(); i++)
    {
        fread(v, 4, 3, fp);
        Mod->meshes[CurMesh].verts[i].x = -v[0]*CurPos.Row2[0] -
                                          v[1]*CurPos.Row2[1] -
                                          v[2]*CurPos.Row2[2] -
                                          CurPos.Row2[3];
        Mod->meshes[CurMesh].verts[i].y = v[0]*CurPos.Row3[0] +
                                          v[1]*CurPos.Row3[1] +
                                          v[2]*CurPos.Row3[2] +
                                          CurPos.Row3[3];
        Mod->meshes[CurMesh].verts[i].z = v[0]*CurPos.Row1[0] +
                                          v[1]*CurPos.Row1[1] +
                                          v[2]*CurPos.Row1[2] +
                                          CurPos.Row1[3];

        Mod->meshes[CurMesh].verts[i] *= COBAutoScale;
        Mod->meshes[CurMesh].verts[i] -= Mod->get_mesh_off(CurMesh, -1);
    }

    // Load the texture vertices
    fread(&l, 4, 1, fp);
    MapCoords.clear();
    MapCoords.insert(MapCoords.end(), l, TexVert());
    for (size_t i=0; i < MapCoords.size(); i++)
    {
        fread(&MapCoords[i].u, 4, 1, fp);
        fread(&MapCoords[i].v, 4, 1, fp);
        MapCoords[i].v *= -1.0f;
    }

    // Load the faces
    fread(&l, 4, 1, fp);
    for (size_t i=0; i < size_t(l); i++)
    {
        char Flags;
        fread(&Flags, 1, 1, fp);

        Mod->meshes[CurMesh].faces.push_back(Face());

        // Number of verts and material
        short s;
        fread(&s, 2, 1, fp);
        Mod->meshes[CurMesh].faces.back().nverts = s;
        if (Flags & F_HOLE)
        {
            Mod->meshes[CurMesh].faces.back().texture = 0;
        }
        else
        {
            fread(&s, 2, 1, fp);
            Mod->meshes[CurMesh].faces.back().texture = s;
        }

        // Each vertex
        for (int k=0; k < Mod->meshes[CurMesh].faces.back().nverts; k++)
        {
            long ll;
            fread(&ll, 4, 1, fp);
            Mod->meshes[CurMesh].faces.back().verts[k] = ll + VertsRefStart;
            fread(&ll, 4, 1, fp);
            Mod->meshes[CurMesh].faces.back().texverts[k] = MapCoords[ll];
        }

        // Erase any "holes"
        //if (Flags & F_HOLE)
        //{
        //    Mod->meshes[CurMesh].faces.pop_back();
        //}
    }

    if (Chunk.Minor >= 6)
    {
        long DrawFlag;
        short Radiosity;
        fread(&DrawFlag, 4, 1, fp);
        fread(&Radiosity, 2, 1, fp);
    }

    // C:\OldHarddrive\TrueSpace CD\DATA\OBJECTS\TRICYCLE.COB
    long Size = ftell(fp) - StartPos;
    if (Size != Chunk.Size)
    {
        ErrorMsg("COB Read Error: %d vs. %d", Size, Chunk.Size);
        fseek(fp, StartPos+Chunk.Size, SEEK_SET);
        return false;
    }

    return true;
}

static bool ParseMaterial(COBChunk &Chunk, FILE *fp)
{
    float dummy;

    // Load the new material
    Mats.push_back(COBMaterial());
    Mats.back().Parse(Chunk, fp);

    // Locate names
    Mats.back().MatIdx = Mod->FindMaterial(Mats.back().Pathname);
    if (int(Mats.back().MatIdx) == -1)
    {
        Mats.back().MatIdx = Mod->mats.size();
        Mod->mats.push_back(ImgString());
        Mod->mats.back().s = Mats.back().Pathname.c_str();
        Mod->mats.back().image = 0;
    }

    return true;
}

bool Model::AddMeshesFromCOB(const AnsiString &fn, int parent, int lod, char type)
{
    FILE *fp = fopen(fn.c_str(), "rb");
    if (fp == 0)
    {
        ErrorMsg("Could not create input stream to '%s'", fn.c_str());
        return false;
    }

    // Read in the header making sure this is an OK format
    COBHeader Header;
    fread(&Header, sizeof(COBHeader), 1, fp);
    if (Header.Format != 'B')
    {
        ErrorMsg("Only binary COBs may be loaded!");
        fclose(fp);
        return false;
    }

    // Setup the global variables so everything is OK
    mParent = parent;
    mLOD = lod;
    mType = type;
    Mod = this;
    Mats.clear();
    MeshInfos.clear();
    Groups.clear();

    // Remeber the index of the first mesh added
    size_t FirstMesh = meshes.size();

    // Read in each chunk an process it if we know how
    bool Done = false;
    COBChunk Chunk;
    while (!Done)
    {
        if (fread(&Chunk, sizeof(COBChunk), 1, fp) < 1)
        {
            Done = true;
        }
        else
        {
            switch (Chunk.Type)
            {
            case 'Grou':
                ParseGroup(Chunk, fp);
                break;
            case 'PolH':
                ParsePolygonalData(Chunk, fp);
                break;
            case 'Mat1':
                ParseMaterial(Chunk, fp);
                break;
            case 'END ':
                Done = true;
                break;
            default:
                if (Chunk.Size != -1)
                {
                    fseek(fp, Chunk.Size, SEEK_CUR);
                }
                else
                {
                    ErrorMsg("Found an unknown COB Chunk with an unknown size!");
                    Done = true;
                }
                break;
            }
        }
    }

    // Assign all the new mats to the new faces, and transform the texture
    // coordinates
    for (size_t i=FirstMesh; i < meshes.size(); i++)
    {
        for (size_t f=0; f < meshes[i].faces.size(); f++)
        {
            for (size_t m=0; m < Mats.size(); m++)
            {
                if ((meshes[i].faces[f].texture == Mats[m].Number) &&
                    (MeshInfos[i-FirstMesh].ChunkID == Mats[m].ParentID))
                {
                    meshes[i].faces[f].texture = Mats[m].MatIdx;
                    for (int k=0; k < meshes[i].faces[f].nverts; k++)
                    {
                        if (Mats[m].Repeat.u != 0.0f)
                        {
                            meshes[i].faces[f].texverts[k].u *= Mats[m].Repeat.u;
                        }
                        if (Mats[m].Repeat.v != 0.0f)
                        {
                            meshes[i].faces[f].texverts[k].v *= Mats[m].Repeat.v;
                        }
                        meshes[i].faces[f].texverts[k].u -= Mats[m].Offset.u;
                        meshes[i].faces[f].texverts[k].v += Mats[m].Offset.v;
                    }
                }
            } // for each cob material
        } // for each face in that mesh
    } // for each new mesh

    fclose(fp);

    InfoMsg("Finished reading '%s'", fn.c_str());

    return true;
}
