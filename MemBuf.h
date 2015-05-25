//---------------------------------------------------------------------------
#ifndef MemBufH
#define MemBufH

#include <vector>

class MemBuf
{
protected:
    std::vector<char> buf;
    size_t pos;

public:
    MemBuf(size_t sz);
    ~MemBuf();

    void reset();
    bool printf(char *fmt, ...);
    void write(const void *data, unsigned int len);
    void write(int i);
    void write(float f);
    void write(char c);
    void write(short s);

    inline size_t tell() { return pos; }
    size_t set(size_t p);
    int dump(FILE *file);
    int dump(MemBuf *mem);
};

//---------------------------------------------------------------------------
#endif
