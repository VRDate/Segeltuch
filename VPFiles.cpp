//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "VPFiles.h"

FILE *VPFile::goto_entry(int n)
{
    if (n >= (int)entries.size()) return NULL;
    if (entries[n].filename[0] == '.' &&
        entries[n].filename[1] == '.') return NULL;

    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;

    fseek(f, entries[n].offset, SEEK_SET);

    return f; //calling function must close 'f'
}
//---------------------------------------------------------------------------

bool VPFile::read(char *fn)
{
    int offset, n_ents;
    VPEntry *ent;

    strcpy(filename, fn);
    FILE *f = fopen(filename, "rb");
    if (!f) return false;

    // read the header
    fread(signature, 1, 4, f);   // "VPVP"
    fread(&version, 4, 1, f);    // "2" in all versions yet
    fread(&offset, 4, 1, f);
    fread(&n_ents, 4, 1, f);
    fseek(f, offset, SEEK_SET);
    while (n_ents)
    {
        ent = entries.insert(entries.end(), VPEntry());
        fread(&ent->offset, 4, 1, f);
        fread(&ent->size, 4, 1, f);
        fread(&ent->filename, 1, 32, f);
        fread(&ent->timestamp, 4, 1, f);
        n_ents--;
    }
    fclose(f);

    if (entries.size() < 1) return true;

    // We must now create the hierarchy by assigning parents
    unsigned int i;
    // first one is major folder
    int curpar = 0;
    entries[0].par = -1;

    for (i=1; i<entries.size(); i++)
    {
        entries[i].par = curpar;

        // is this a directory?
        if (entries[i].size == 0)
        {
            if ((entries[i].filename[0] == '.') &&
                (entries[i].filename[1] == '.'))
            {
                if (entries[curpar].par >= 0)
                    curpar = entries[curpar].par;
            }
            else
            {
                curpar = i;
            }
        }
    }

    return true;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#pragma package(smart_init)
