//---------------------------------------------------------------------------
#ifndef VPFilesH
#define VPFilesH

#include <vcl.h>
#include <comctrls.hpp>
#include <vector>

using namespace std;

struct VPEntry
{
    int offset;          // from beginning of file
    int size;
    char filename[32];   // Null-terminated string
    int timestamp;       // The time the file was last modified.

    int par;
    TTreeNode *node;
};

struct VPFile
{
    char signature[4];   // "VPVP"
    int version;         // "2" in all versions yet

    char filename[256];

    vector<VPEntry> entries;

    FILE *goto_entry(int n);
    bool read(char *fn);
};


//---------------------------------------------------------------------------
#endif
