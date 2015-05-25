//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#include <stdarg.h>
#pragma hdrstop

#include "MemBuf.h"
#include "Global.h"
#include "F_Main.h"

MemBuf::MemBuf(size_t sz)
{
    buf.clear();
    buf.reserve(sz);
    pos = 0;
}

MemBuf::~MemBuf()
{
}

void MemBuf::reset()
{
    buf.clear();
    pos = 0;
}

// returns the old position
size_t MemBuf::set(size_t p)
{
    size_t r = pos;
    // do nothing if trying to seek ahead of mem
    if (p < buf.size())
    {
        pos = p;
    }
    else
    {
        pos = buf.size();
    }
    return r;
}

bool MemBuf::printf(char *fmt, ...)
{
    static char tempbuf[1024*4];

    va_list argptr;
	va_start(argptr, fmt);
	int sz = vsprintf(tempbuf, fmt, argptr);
    va_end(argptr);

    if (sz == EOF) return false;

    write(tempbuf, sz);

    return true;
}

void MemBuf::write(int i)
{
    write(&i, 4);
}
void MemBuf::write(float f)
{
    write(&f, 4);
}
void MemBuf::write(char c)
{
    write(&c, 1);
}
void MemBuf::write(short s)
{
    write(&s, 2);
}
void MemBuf::write(const void *data, size_t len)
{
    int more = int(pos) - int(buf.size()) + int(len);
    if (more > 0)
    {
        buf.insert(buf.end(), size_t(more), 0);
    }

    memcpy(&buf[pos], data, len);
    pos += len;
}
int MemBuf::dump(FILE *file)
{
    return fwrite(&buf[0], 1, buf.size(), file);
}
int MemBuf::dump(MemBuf *mem)
{
    mem->write(&buf[0], buf.size());
    return (int)pos;
}

//---------------------------------------------------------------------------
#pragma package(smart_init)
