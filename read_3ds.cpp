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

using namespace std;

static int     mParent;
static int     mLOD;
static char    mType;
static size_t  CurMesh;
static Model  *Mod;
static vector<TexVert> MapCoords;

static string ParseCStr(FILE *fp)
{
    string str;
    int    ch;
    while ((ch = fgetc(fp)) != EOF && ch != '\0')
        str.push_back(ch);
    return str;
}

static bool ParseNamedObject(FILE *fp)
{
    string ObjName = ParseCStr(fp);

    Mod->meshes.push_back(Mesh());
    Mod->meshes.back().name   = ObjName.c_str();
    Mod->meshes.back().offset = Vector();
    Mod->meshes.back().lod    = mLOD;
    Mod->meshes.back().type   = mType;
    Mod->meshes.back().parent = mParent;

    CurMesh = Mod->meshes.size() - 1;
    MapCoords.clear();

    if ((mParent == -1) && (mType == MT_HULL))
    {
        Mod->nlods++;
        mLOD++;
    }

    return true;
}

static bool ReadChunkHeader(FILE *fp, unsigned short *pID, unsigned long *pLen)
{
    fread(pID, sizeof(*pID), 1, fp);
    fread(pLen, sizeof(*pLen), 1, fp);
    return true;
}

static bool ParseVertexArray(FILE *fp)
{
    short NumVertices;
    fread(&NumVertices, sizeof(NumVertices), 1, fp);

    Mod->meshes[CurMesh].verts.insert(Mod->meshes[CurMesh].verts.begin(),
                                      size_t(NumVertices), Vector());
    for (size_t i=0; i < size_t(NumVertices); i++)
    {
        float v[3];
        fread(v, sizeof(v[0]), 3, fp);
        
        Mod->meshes[CurMesh].verts[i] = v;
    }

    return true;
}

static bool ParseFaceArray(FILE *fp)
{
    short NumFaces;
    fread(&NumFaces, sizeof(NumFaces), 1, fp);

    struct ExtFace {
        short V0, V1, V2, Flags;
    };

    Mod->meshes[CurMesh].faces.insert(Mod->meshes[CurMesh].faces.begin(),
                                      size_t(NumFaces), Face());
    for (size_t i=0; i < size_t(NumFaces); i++)
    {
        ExtFace XFace;

        fread(&XFace, sizeof(XFace), 1, fp);

        Mod->meshes[CurMesh].faces[i].nverts = 3;
        Mod->meshes[CurMesh].faces[i].verts[0] = XFace.V0;
        Mod->meshes[CurMesh].faces[i].verts[1] = XFace.V1;
        Mod->meshes[CurMesh].faces[i].verts[2] = XFace.V2;
        if (MapCoords.size() > 0)
        {
            Mod->meshes[CurMesh].faces[i].texverts[0] = MapCoords[XFace.V0];
            Mod->meshes[CurMesh].faces[i].texverts[1] = MapCoords[XFace.V1];
            Mod->meshes[CurMesh].faces[i].texverts[2] = MapCoords[XFace.V2];
        }
        else
        {
            Mod->meshes[CurMesh].faces[i].texverts[0] = TexVert(1.0f, 1.0f);
            Mod->meshes[CurMesh].faces[i].texverts[1] = TexVert(0.0f, 1.0f);
            Mod->meshes[CurMesh].faces[i].texverts[2] = TexVert(0.0f, 0.0f);
        }
        Mod->meshes[CurMesh].faces[i].texture = 0;
    }

    return true;
}

static bool ParseFaceMaterial(FILE *fp)
{
    string MaterialName = ParseCStr(fp);
    size_t MatIdx = Mod->FindMaterial(MaterialName);

    if (int(MatIdx) == -1)
    {
        Mod->mats.push_back(ImgString());
        Mod->mats.back().s = MaterialName.c_str();
        MatIdx = Mod->mats.size()-1;
    }

    short FaceNum, NumFaces;
    fread(&NumFaces, sizeof(NumFaces), 1, fp);
    for (size_t i=0; i < size_t(NumFaces); i++)
    {
        fread(&FaceNum, sizeof(FaceNum), 1, fp);

        Mod->meshes[CurMesh].faces[FaceNum].texture = int(MatIdx);
    }

    return true;
}

static bool ParseMapCoords(FILE *fp)
{
    short NumCoords;
    fread(&NumCoords, sizeof(NumCoords), 1, fp);

    float  U, V;
    size_t f;
    int    k;

    MapCoords.insert(MapCoords.begin(), size_t(NumCoords), TexVert());
    for (size_t i=0; i < size_t(NumCoords); i++)
    {
        fread(&U, sizeof(U), 1, fp);
        fread(&V, sizeof(V), 1, fp);

        MapCoords[i].u = U;
        MapCoords[i].v = 1.0f - V;
    }

    return true;
}

static bool ParseChunk(FILE *fp)
{
    unsigned long  StartPos = ftell(fp);
    unsigned short ID;
    unsigned long  Len;

    ReadChunkHeader(fp, &ID, &Len);

    switch (ID)
    {
    case 0x3d3d:
    case 0x4100:
    case 0x4d4d:
        while (ftell(fp) - StartPos < Len)
            if (!ParseChunk(fp))
                break;
        break;

    case 0x4000:
        if (ParseNamedObject(fp))
            while (ftell(fp) - StartPos < Len)
                if (!ParseChunk(fp))
                    break;
        break;

    case 0x4110:
        ParseVertexArray(fp);
        break;

    case 0x4120:
        if (ParseFaceArray(fp))
            while (ftell(fp) - StartPos < Len)
                if (!ParseChunk(fp))
                    break;
        break;

    case 0x4130:
        ParseFaceMaterial(fp);
        break;

    case 0x4140:
        ParseMapCoords(fp);
        break;

    default:
        fseek(fp, Len-(sizeof(ID))-(sizeof(Len)), SEEK_CUR);
        break;
    }

    unsigned long ChunkRead = ftell(fp) - StartPos;
    if (ChunkRead < Len)
        fseek(fp, Len-ChunkRead, SEEK_CUR);

    return true;
}

bool Model::AddMeshesFrom3DS(const AnsiString &fn, int parent, int lod, char type)
{
    FILE *fp = fopen(fn.c_str(), "rb");
    if (fp == 0)
    {
        ErrorMsg("Unable to gain an input stream to %s", fn.c_str());
        return false;
    }

    mParent = parent;
    mLOD = lod;
    mType = type;
    Mod = this;
    CurMesh = size_t(-1);
    ParseChunk(fp);
    fclose(fp);

    return true;
}
