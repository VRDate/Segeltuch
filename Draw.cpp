//---------------------------------------------------------------------------
#include <vcl.h>
#include <vector>
#pragma hdrstop

#include "Global.h"
#include "RenderGL.h"
#include "Geometry.h"
#include "F_Main.h"
#include "ImageFiles.h"

#include "Draw.h"

using namespace std;

const GLfloat NORM_LENGTH = 5.0f;

void DrawRadius(const Vector &P, float Rad)
{
    const int     NumSegs = 30;
    static Vector Segs[NumSegs+1];
    static bool   Initialized = false;
    int           i;

    if (!Initialized)
    {
        float SegArcLen = TwoPi/float(NumSegs);
        for (i=0; i <= NumSegs; i++)
        {
            Segs[i].x = cos(float(i)*SegArcLen);
            Segs[i].y = sin(float(i)*SegArcLen);
        }
        Initialized = true;
    }

    glLineWidth(1.5);
    // Draw the circle on X, Y, and Z planes
    glBegin(GL_LINES);
        for (i=0; i < NumSegs; i++)
        {
            glVertex3f(P.x+0.0f, P.y+Rad*Segs[i].x,   P.z+Rad*Segs[i].y);
            glVertex3f(P.x+0.0f, P.y+Rad*Segs[i+1].x, P.z+Rad*Segs[i+1].y);
        }
    glEnd();
    glBegin(GL_LINES);
        for (i=0; i < NumSegs; i++)
        {
            glVertex3f(P.x+Rad*Segs[i].x,   P.y+0.0f, P.z+Rad*Segs[i].y);
            glVertex3f(P.x+Rad*Segs[i+1].x, P.y+0.0f, P.z+Rad*Segs[i+1].y);
        }
    glEnd();
    glBegin(GL_LINES);
        for (i=0; i < NumSegs; i++)
        {
            glVertex3f(P.x+Rad*Segs[i].x,   P.y+Rad*Segs[i].y,   P.z+0.0f);
            glVertex3f(P.x+Rad*Segs[i+1].x, P.y+Rad*Segs[i+1].y, P.z+0.0f);
        }
    glEnd();
    glLineWidth(1.0);
}

void DrawFaceSel(int nm, int nf, Vector n_off)
{
    int k;
    Face *f;
    GLint cull;
    glGetIntegerv(GL_CULL_FACE_MODE, &cull);

    // Give this face a color
    if (cull == GL_BACK)
        glColor4f(MainGL.Fore.r, MainGL.Fore.g, MainGL.Fore.b, MainGL.Fore.a);
    else
        glColor4f(MainGL.Back.r, MainGL.Back.g, MainGL.Back.b, MainGL.Back.a);

    f = &Proj.meshes[nm].faces[nf];

    // Give a name to this face
    switch (ViewObjs)
    {
    case VO_FACE:
        glLoadName(nm + (nf<<16));
        break;
    case VO_MESH:
        glLoadName(nm);
        break;
    default:
        glLoadName(-1);
        break;
    }

    // Actually draw it
    if (f->nverts == 3)
        glBegin(GL_TRIANGLES);
    else if (f->nverts == 4)
        glBegin(GL_QUADS);
    else
        glBegin(GL_POLYGON);
    // We give everything to OpenGL "backwards" to make it happy
    glNormal3f(-f->norm.x, -f->norm.y, -f->norm.z);
    for (k=f->nverts-1; k>=0; k--)
    {
        glVertex3f(Proj.meshes[nm].verts[f->verts[k]].x + n_off.x,
                   Proj.meshes[nm].verts[f->verts[k]].y + n_off.y,
                   Proj.meshes[nm].verts[f->verts[k]].z + n_off.z);
    }
    glEnd(); // POLYGON
}

void DrawMeshFacesSel(int nm, Vector n_off, bool PickColor)
{
    unsigned int j;
    int k;
    Face *f;
    GLint cull;
    glGetIntegerv(GL_CULL_FACE_MODE, &cull);

    // Give this face a color
    if (PickColor)
    {
        if (cull == GL_BACK)
            glColor4f(MainGL.Fore.r, MainGL.Fore.g, MainGL.Fore.b, MainGL.Fore.a);
        else
            glColor4f(MainGL.Back.r, MainGL.Back.g, MainGL.Back.b, MainGL.Back.a);
    }

    // Draw this mesh
    for (j=0; j<Proj.meshes[nm].faces.size(); j++)
    {
        f = &Proj.meshes[nm].faces[j];

        // Give a name to this face
        switch (ViewObjs)
        {
        case VO_FACE:
            glLoadName(nm + (j<<16));
            break;
        case VO_MESH:
            glLoadName(nm);
            break;
        default:
            glLoadName(-1);
            break;
        }

        // Actually draw it
        if (f->nverts == 3)
            glBegin(GL_TRIANGLES);
        else if (f->nverts == 4)
            glBegin(GL_QUADS);
        else
            glBegin(GL_POLYGON);
        // We give everything to OpenGL "backwards" to make it happy
        glNormal3f(-f->norm.x, -f->norm.y, -f->norm.z);
        for (k=f->nverts-1; k>=0; k--)
        {
            glVertex3f(Proj.meshes[nm].verts[f->verts[k]].x + n_off.x,
                Proj.meshes[nm].verts[f->verts[k]].y + n_off.y,
                Proj.meshes[nm].verts[f->verts[k]].z + n_off.z);
        }
        glEnd(); // POLYGON
    }
}

void DrawMeshFacesNorm(int nm, Vector n_off, bool PickColor)
{
    unsigned int j;
    int k;
    Face *f;
    GLint cull;
    glGetIntegerv(GL_CULL_FACE_MODE, &cull);

    // Give this face a color
    if (PickColor)
    {
        if (cull == GL_BACK)
            glColor4f(MainGL.Fore.r, MainGL.Fore.g, MainGL.Fore.b, MainGL.Fore.a);
        else
            glColor4f(MainGL.Back.r, MainGL.Back.g, MainGL.Back.b, MainGL.Back.a);
    }

    // Draw this mesh's faces
    for (j=0; j<Proj.meshes[nm].faces.size(); j++)
    {
        // do not draw selected faces
        if (ViewObjs==VO_FACE)
            if (Sels.is_sel(nm + (j<<16))) continue;

        f = &Proj.meshes[nm].faces[j];

        // Give a name to this face
        switch (ViewObjs)
        {
        case VO_FACE:
            glLoadName(nm + (j<<16));
            break;
        case VO_MESH:
            glLoadName(nm);
            break;
        default:
            glLoadName(-1);
            break;
        }

        // Actually draw it
        if (f->nverts == 3)
            glBegin(GL_TRIANGLES);
        else if (f->nverts == 4)
            glBegin(GL_QUADS);
        else
            glBegin(GL_POLYGON);
        // We give everything to OpenGL "backwards" to make it happy
        glNormal3f(-f->norm.x, -f->norm.y, -f->norm.z);
        for (k=f->nverts-1; k>=0; k--)
        {
            glVertex3f(Proj.meshes[nm].verts[f->verts[k]].x + n_off.x,
                Proj.meshes[nm].verts[f->verts[k]].y + n_off.y,
                Proj.meshes[nm].verts[f->verts[k]].z + n_off.z);
        }
        glEnd(); // POLYGON
    }
}

void DrawMeshVerts(int nm, Vector n_off)
{
    unsigned int j;

    GLboolean elight = glIsEnabled(GL_LIGHTING);
    glDisable(GL_LIGHTING);

    static GLfloat OldColor[4];

    glGetFloatv(GL_CURRENT_COLOR, OldColor);

    glColor3f(MainGL.ColObj[0].r, MainGL.ColObj[0].g, MainGL.ColObj[0].b);
    glPointSize(6.0f);

    // Draw this mesh
    for (j=0; j<Proj.meshes[nm].verts.size(); j++)
    {
        // Give a name to this face
        switch (ViewObjs)
        {
        case VO_VERT:
            glLoadName(nm + (j<<16));
            break;
        case VO_MESH:
            glLoadName(nm);
            break;
        default:
            glLoadName(-1);
            break;
        }

        // Actually draw it
        glBegin(GL_POINTS);
        glVertex3f(Proj.meshes[nm].verts[j].x + n_off.x,
                   Proj.meshes[nm].verts[j].y + n_off.y,
                   Proj.meshes[nm].verts[j].z + n_off.z);
        glEnd(); // POINTS
    }

    glPointSize(1.0f);
    glColor4fv(OldColor);
    if (elight)
    {
        glEnable(GL_LIGHTING);
    }
}

void DrawMeshHier(int nm, Vector offset, bool AutoColor = true)
{
    unsigned int j;

    if ((nm>=0) && (nm<(int)Proj.meshes.size()))
    {
        MainGL.Fore = MainGL.ColHull[0];
        MainGL.Back = MainGL.ColHull[1];

        if (ViewObjs == VO_MESH)
        {
            int fin = Sels.find_sel(nm);
            if ((fin >= 0) && ((int)Sels.cur != nm))
            {
                MainGL.Fore = MainGL.ColSelMult[0];
                MainGL.Back = MainGL.ColSelMult[1];
            }
            else if ((fin >= 0) && ((int)Sels.cur == nm))
            {
                MainGL.Fore = MainGL.ColSelResl[0];
                MainGL.Back = MainGL.ColSelResl[1];
            }
            else if ((fin < 0) && ((int)Sels.cur == nm))
            {
                MainGL.Fore = MainGL.ColSelSing[0];
                MainGL.Back = MainGL.ColSelSing[1];
            }
        }

        // choose the proper rendering system
        if ((ViewObjs == VO_MESH) && (Sels.is_sel(nm)))
            DrawMeshFacesSel(nm, offset, AutoColor);
        else
            DrawMeshFacesNorm(nm, offset, AutoColor);

        if (ViewObjs == VO_VERT) DrawMeshVerts(nm, offset);
    }

    // Draw the children
    for (j=0; j<Proj.meshes.size(); j++)
    {
        if (Proj.meshes[j].parent == nm)
        {
            DrawMeshHier(j, offset + Proj.meshes[j].offset, AutoColor);
        }
    }
}

void DrawMeshesWire()
{
    Vector v0;
    unsigned int i;
    int fin, hin;
    Vector off;

    glDisable(GL_LIGHTING);

    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
	glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_BLEND);
    glShadeModel(GL_FLAT);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(1.0, 1.0);

    // Draw the front faces with the light color
    glCullFace(GL_BACK);
    DrawMeshHier(ViewNode, v0);

    // draw selected objects
    // note that the selected mesh has already been drawn in the correct color
    switch (ViewObjs)
    {
    case VO_FACE:
        // selected group
        MainGL.Fore = MainGL.ColSelMult[0];
        MainGL.Back = MainGL.ColSelMult[1];
        for (i=0; i<Sels.size(); i++)
        {
            if (Sels[i] == Sels.cur) continue;

            fin = 0xFFFF & (int)Sels[i]; // get the mesh
            hin = (int)(Sels[i] >> 16);  // get the face

            off = Proj.get_mesh_off(fin, ViewNode);
            glCullFace(GL_BACK);
            DrawFaceSel(fin, hin, off);
        }
        // current face
        fin = 0xFFFF & (int)Sels.cur; // get the mesh
        hin = (int)(Sels.cur >> 16);  // get the face
        if (fin >= (int)Proj.meshes.size()) break;
        if (Sels.find_sel(Sels.cur) >= 0)
        {
            MainGL.Fore = MainGL.ColSelResl[0];
            MainGL.Back = MainGL.ColSelResl[1];
        }
        else
        {
            MainGL.Fore = MainGL.ColSelSing[0];
            MainGL.Back = MainGL.ColSelSing[1];
        }
        off = Proj.get_mesh_off(fin, ViewNode);
        glCullFace(GL_FRONT);
        DrawFaceSel(fin, hin, off);
        glCullFace(GL_BACK);
        DrawFaceSel(fin, hin, off);
        break;
    }
}

void DrawMeshesSolid()
{
    Vector v0;
    int face;
    Vector off;

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);

    DrawMeshHier(ViewNode, v0);

    // draw selected objects
    // note that the selected mesh has already been drawn
    switch (ViewObjs)
    {
    case VO_FACE:
        for (size_t mm=0; mm<Proj.meshes.size(); mm++)
        {
            off = Proj.get_mesh_off(mm, ViewNode);
            MainGL.Fore = MainGL.ColSelMult[0];
            // selected group
            for (size_t i=0; i<Sels.size(); i++)
            {
                if ((Sels[i] & 0xFFFF) != mm) continue;
                if (Sels[i] == Sels.cur) continue;
                face = int(Sels[i] >> 16);  // get the face
                DrawFaceSel(mm, face, off);
            }
            // current face
            if ((int(Sels.cur) != -1) && ((Sels.cur & 0xFFFF) == mm))
            {
                face = int(Sels.cur >> 16);  // get the face
                if (Sels.find_sel(Sels.cur) >= 0)
                {
                    MainGL.Fore = MainGL.ColSelResl[0];
                }
                else
                {
                    MainGL.Fore = MainGL.ColSelSing[0];
                }
                DrawFaceSel(mm, face, off);
            }
        }
        break;
    }

    glColor4f(0.9,0.9,0.9,1);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(0.0, 0.0);
    //glEnable(GL_LINE_SMOOTH);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    //glLineWidth(1.5);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
    glShadeModel(GL_FLAT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    DrawMeshHier(ViewNode, v0, false);
    //glLineWidth(1.0);
    //glDisable(GL_BLEND);
    //glDisable(GL_LINE_SMOOTH);
}

void DrawTextMeshFacesNorm(int nm, Vector n_off)
{
    unsigned int j;
    int k;
    Face *f;
    OGLTexture *textr;

    // Draw this mesh
    for (j=0; j<Proj.meshes[nm].faces.size(); j++)
    {
        f = &Proj.meshes[nm].faces[j];

        // Make sure this face's texture map is loaded
        if ((f->texture >= 0) && (f->texture < int(Proj.mats.size())))
        {
            textr = (OGLTexture*)Proj.mats.at(f->texture).image;
            if (textr == 0)
            {
                textr = new OGLTexture;
                if (!textr->LoadFromPCX(MapsDir + Proj.mats[f->texture].s + ".pcx"))
                {
                    textr->LoadFromPCX(MapsDir + "default.pcx");
                }
                Proj.mats.at(f->texture).image = textr;
            }
            textr->SetCurrent();
        }

        // Give a name to this face
        switch (ViewObjs)
        {
        case VO_FACE:
            glLoadName(nm + (j<<16));
            break;
        case VO_MESH:
            glLoadName(nm);
            break;
        default:
            glLoadName(-1);
            break;
        }

        // Actually draw it
        glBegin(GL_POLYGON);
        // We give everything to OpenGL "backwards" to make it happy
        glNormal3f(-f->norm.x, -f->norm.y, -f->norm.z);
        for (k=f->nverts-1; k>=0; k--)
        {
            glTexCoord2f(f->texverts[k].u, f->texverts[k].v);
            glVertex3f(Proj.meshes[nm].verts[f->verts[k]].x + n_off.x,
                       Proj.meshes[nm].verts[f->verts[k]].y + n_off.y,
                       Proj.meshes[nm].verts[f->verts[k]].z + n_off.z);

        }
        glEnd(); // POLYGON
    }
}

void DrawTextMeshHier(int nm, Vector offset)
{
    unsigned int j;

    glEnable(GL_TEXTURE_2D);

    if ((nm>=0) && (nm<(int)Proj.meshes.size()))
    {
        // choose the proper rendering system
        DrawTextMeshFacesNorm(nm, offset);

        if (ViewObjs == VO_VERT) DrawMeshVerts(nm, offset);
    }

    // Draw the children
    for (j=0; j<Proj.meshes.size(); j++)
    {
        if (Proj.meshes[j].parent == nm)
        {
            DrawTextMeshHier(j, offset + Proj.meshes[j].offset);
        }
    }

    glDisable(GL_TEXTURE_2D);
}

void DrawTextFaceSel(int nm, int nf, Vector n_off)
{
    int k;
    Face *f;

    glColor4f(MainGL.Fore.r, MainGL.Fore.g, MainGL.Fore.b, MainGL.Fore.a);

    f = &Proj.meshes[nm].faces[nf];

    // Give a name to this face
    switch (ViewObjs)
    {
    case VO_FACE:
        glLoadName(nm + (nf<<16));
        break;
    case VO_MESH:
        glLoadName(nm);
        break;
    default:
        glLoadName(-1);
        break;
    }

    // Actually draw it
    glBegin(GL_POLYGON);
    // We give everything to OpenGL "backwards" to make it happy
    glNormal3f(-f->norm.x, -f->norm.y, -f->norm.z);
    for (k=f->nverts-1; k>=0; k--)
    {
        glVertex3f(Proj.meshes[nm].verts[f->verts[k]].x + n_off.x,
                   Proj.meshes[nm].verts[f->verts[k]].y + n_off.y,
                   Proj.meshes[nm].verts[f->verts[k]].z + n_off.z);
    }
    glEnd(); // POLYGON
}

void DrawMeshesTextured()
{
    Vector v0;
    Vector off;
    int face;

    glDisable(GL_LIGHTING);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);

    glPolygonMode(GL_FRONT, GL_FILL);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);

    DrawTextMeshHier(ViewNode, v0);

    // draw selected objects
    // note that the selected mesh has already been drawn
    glPolygonMode(GL_FRONT, GL_LINE);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(0.0, 0.0);
    glDisable(GL_LIGHTING);
    switch (ViewObjs)
    {
    case VO_FACE:
        for (size_t mm=0; mm<Proj.meshes.size(); mm++)
        {
            off = Proj.get_mesh_off(mm, ViewNode);
            MainGL.Fore = MainGL.ColSelMult[0];
            // selected group
            for (size_t i=0; i<Sels.size(); i++)
            {
                if ((Sels[i] & 0xFFFF) != mm) continue;
                if (Sels[i] == Sels.cur) continue;
                face = int(Sels[i] >> 16);  // get the face
                DrawTextFaceSel(mm, face, off);
            }
            // current face
            if ((int(Sels.cur) != -1) && ((Sels.cur & 0xFFFF) == mm))
            {
                face = int(Sels.cur >> 16);  // get the face
                if (Sels.find_sel(Sels.cur) >= 0)
                {
                    MainGL.Fore = MainGL.ColSelResl[0];
                }
                else
                {
                    MainGL.Fore = MainGL.ColSelSing[0];
                }
                DrawTextFaceSel(mm, face, off);
            }
        }
        break;
    }
}

void CALLBACK errorCallback(/*GLenum errorCode*/)
{
    const GLubyte *estring;
    estring = gluErrorString(glGetError());
    ErrorMsg("Quadric Error: %s", estring);
}

void DrawCircle(GLUquadricObj *qobj, Vector pos, Vector norm, float radius)
{
    //radius *= 50.0;
    //InfoMsg("DrawCircle(<%.1f,%.1f,%.1f>, %.3f)", pos.x,pos.y,pos.z,radius);
    //glPushMatrix();
    //    glLoadIdentity();
    //    glTranslatef(pos.x, pos.y, pos.z);
    //    gluSphere(qobj, radius, 10, 10);
    //glPopMatrix();
}

void DrawThrusters()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_COLOR_MATERIAL);
    glPointSize(6.0);

    unsigned int name;

    for (size_t t=0; t < Proj.thrusters.size(); t++)
    {
        for (size_t g=0; g<Proj.thrusters[t].glows.size(); g++)
        {
            // Determine its name
            name  = (g << 16) | (t & 0xFFFF);
            glLoadName(name);

            // Determine the color
            if (Sels.find_sel(name) >= 0)
            {
                if (name == Sels.cur)
                    glColor4f(MainGL.ColSelResl[0].r,
                              MainGL.ColSelResl[0].g,
                              MainGL.ColSelResl[0].b,
                              MainGL.ColSelResl[0].a);
                else
                    glColor4f(MainGL.ColSelMult[0].r,
                              MainGL.ColSelMult[0].g,
                              MainGL.ColSelMult[0].b,
                              MainGL.ColSelMult[0].a);
            }
            else if (name == Sels.cur)
            {
                glColor4f(MainGL.ColSelSing[0].r,
                          MainGL.ColSelSing[0].g,
                          MainGL.ColSelSing[0].b,
                          MainGL.ColSelSing[0].a);
            }
            else
            {
                glColor4f(MainGL.ColObj[0].r,
                          MainGL.ColObj[0].g,
                          MainGL.ColObj[0].b,
                          MainGL.ColObj[0].a);
            }

            // Draw it
            Glow &gl = Proj.thrusters[t].glows[g];
            glBegin(GL_POINTS);
                glVertex3f(gl.pos.x, gl.pos.y, gl.pos.z);
            glEnd();
            glBegin(GL_LINES);
                glVertex3f(gl.pos.x, gl.pos.y, gl.pos.z);
                glVertex3f(gl.pos.x + gl.norm.x*NORM_LENGTH,
                           gl.pos.y + gl.norm.y*NORM_LENGTH,
                           gl.pos.z + gl.norm.z*NORM_LENGTH);
            glEnd();
            DrawRadius(gl.pos, gl.radius);
        }
    }

}

void Grid::Draw()
{
    int i,j,n;
    float x,y,z;
    Vector v1,v2;

    float halfs = size / 2.0;

    /****NEW
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
    glDisable(GL_DEPTH_TEST);
	glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glShadeModel(GL_FLAT);
    ****/
    glFrontFace(GL_CCW);
    glShadeModel(GL_FLAT);
    glDisable(GL_BLEND);
    /****/

    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glColor4f(MainGL.ColGrid.r, MainGL.ColGrid.g, MainGL.ColGrid.b, 1.0);
    glPointSize(1.0);

    n = (int)(size/dot+0.5)/2 - 1;
    for (i = -n; i <= n; i++)
    {
        for (j = -n; j <= n; j++)
        {
            x = pos.x + (i*dot*right.x) + (j*dot*dir.x);
            y = pos.y + (i*dot*right.y) + (j*dot*dir.y);
            z = pos.z + (i*dot*right.z) + (j*dot*dir.z);

            //glLoadName(-1);
            glBegin(GL_POINTS);
            glVertex3f(x, y, z);
            glEnd();
        }
    }

    //glColor4f(col.x*0.7, col.y*0.7, col.z*0.7, 1.0);

    n = (int)(size/line+0.5)/2;
    for (i = -n; i <= n; i++)
    {
        v1.x = pos.x + (i*line*right.x) + (halfs*dir.x);
        v1.y = pos.y + (i*line*right.y) + (halfs*dir.y);
        v1.z = pos.z + (i*line*right.z) + (halfs*dir.z);

        v2.x = pos.x + (i*line*right.x) + (-halfs*dir.x);
        v2.y = pos.y + (i*line*right.y) + (-halfs*dir.y);
        v2.z = pos.z + (i*line*right.z) + (-halfs*dir.z);

        //glLoadName(-1);
        glBegin(GL_LINES);
        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
        glEnd();
    }

    for (i = -n; i <= n; i++)
    {
        v1.x = pos.x + (halfs*right.x) + (i*line*dir.x);
        v1.y = pos.y + (halfs*right.y) + (i*line*dir.y);
        v1.z = pos.z + (halfs*right.z) + (i*line*dir.z);

        v2.x = pos.x + (i*line*dir.x) + (-halfs*right.x);
        v2.y = pos.y + (i*line*dir.y) + (-halfs*right.y);
        v2.z = pos.z + (i*line*dir.z) + (-halfs*right.z);

        //glLoadName(-1);
        glBegin(GL_LINES);
        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
        glEnd();
    }
}



void DrawSpecials()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_COLOR_MATERIAL);
    glPointSize(6.0);

    unsigned int name;

    vector<Special>::iterator i = Proj.Specials.begin();
    for (; i != Proj.Specials.end(); i++)
    {
        // Determine its name
        name  = i - Proj.Specials.begin();
        glLoadName(name);

        // Determine the color
        if (Sels.find_sel(name) >= 0)
        {
            if (name == Sels.cur)
                glColor4f(MainGL.ColSelResl[0].r,
                          MainGL.ColSelResl[0].g,
                          MainGL.ColSelResl[0].b,
                          MainGL.ColSelResl[0].a);
            else
                glColor4f(MainGL.ColSelMult[0].r,
                          MainGL.ColSelMult[0].g,
                          MainGL.ColSelMult[0].b,
                          MainGL.ColSelMult[0].a);
        }
        else if (name == Sels.cur)
        {
            glColor4f(MainGL.ColSelSing[0].r,
                      MainGL.ColSelSing[0].g,
                      MainGL.ColSelSing[0].b,
                      MainGL.ColSelSing[0].a);
        }
        else
        {
            glColor4f(MainGL.ColObj[0].r,
                      MainGL.ColObj[0].g,
                      MainGL.ColObj[0].b,
                      MainGL.ColObj[0].a);
        }

        // Draw it
        glBegin(GL_POINTS);
            glVertex3f((*i).Pos.x, (*i).Pos.y, (*i).Pos.z);
        glEnd();
        DrawRadius((*i).Pos, (*i).Radius);
    }
}

void DrawWeapons()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_COLOR_MATERIAL);
    glPointSize(6.0);

    unsigned int name;

    vector<Weapon>::iterator i = Proj.weapons.begin();
    for (; i != Proj.weapons.end(); i++)
    {
        // Determine its name
        name  = i - Proj.weapons.begin();
        glLoadName(name);

        // Determine the color
        if (Sels.find_sel(name) >= 0)
        {
            if (name == Sels.cur)
                glColor4f(MainGL.ColSelResl[0].r,
                          MainGL.ColSelResl[0].g,
                          MainGL.ColSelResl[0].b,
                          MainGL.ColSelResl[0].a);
            else
                glColor4f(MainGL.ColSelMult[0].r,
                          MainGL.ColSelMult[0].g,
                          MainGL.ColSelMult[0].b,
                          MainGL.ColSelMult[0].a);
        }
        else if (name == Sels.cur)
        {
            glColor4f(MainGL.ColSelSing[0].r,
                      MainGL.ColSelSing[0].g,
                      MainGL.ColSelSing[0].b,
                      MainGL.ColSelSing[0].a);
        }
        else
        {
            if ((*i).Type == WT_GUN)
                glColor4f(MainGL.ColObj[0].r,
                      MainGL.ColObj[0].g,
                      MainGL.ColObj[0].b,
                      MainGL.ColObj[0].a);
            else
                glColor4f(MainGL.ColObj[0].b,
                      MainGL.ColObj[0].g,
                      MainGL.ColObj[0].r,
                      MainGL.ColObj[0].a);
        }

        // Draw it
        glBegin(GL_POINTS);
            glVertex3f((*i).Pos.x, (*i).Pos.y, (*i).Pos.z);
        glEnd();
        glBegin(GL_LINES);
            glVertex3f((*i).Pos.x,  (*i).Pos.y, (*i).Pos.z);
            glVertex3f((*i).Pos.x + (*i).Normal.x*NORM_LENGTH,
                       (*i).Pos.y + (*i).Normal.y*NORM_LENGTH,
                       (*i).Pos.z + (*i).Normal.z*NORM_LENGTH);
        glEnd();
    }
}
//---------------------------------------------------------------------------
static void DrawShieldMesh(Mesh &msh)
{
    glColor4f(MainGL.ColShld[0].r, MainGL.ColShld[0].g, MainGL.ColShld[0].b,
        MainGL.ColShld[0].a);

    // Draw this mesh
    for (size_t j=0; j<msh.faces.size(); j++)
    {
        Face &f = msh.faces[j];
        //glLoadName(-1); // Questionable whether this should be here

        // Actually draw it
        glBegin(GL_TRIANGLES);
            glNormal3f(-f.norm.x, -f.norm.y, -f.norm.z);
            for (int k=2; k>=0; k--)
            {
                glVertex3f(msh.verts[f.verts[k]].x,
                           msh.verts[f.verts[k]].y,
                           msh.verts[f.verts[k]].z);
            }
        glEnd();
    }
}
void DrawShields()
{
    if (MainGL.UseLight)
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
    }
    else
    {
        glDisable(GL_LIGHTING);
    }

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_FLAT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    DrawShieldMesh(Proj.shields);
}
void DrawCursor(const Vector &Pos)
{
    glDisable(GL_LIGHTING);

    glColor3f(0.5f, 0.5f, 1.0f);
    glPointSize(6.0f);

    glBegin(GL_POINTS);
        glVertex3f(Pos.x, Pos.y, Pos.z);
    glEnd();
}

//---------------------------------------------------------------------------
#pragma package(smart_init)
