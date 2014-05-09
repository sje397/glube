#ifndef CHUNK_H
#define CHUNK_H

#include "drawable.h"

#include <boost/scoped_array.hpp>
#include <boost/thread/mutex.hpp>
using boost::scoped_array;

#include <boost/thread.hpp>

#include <vector>
#include <functional>

namespace Glube {

class Chunk
{
public:
    Chunk(int size);
    virtual ~Chunk();

    virtual void draw();
    virtual void deleteBuffers();

    typedef unsigned char BlockType;
    virtual BlockType getBlock(int x, int y, int z);
    virtual void setBlock(int x, int y, int z, BlockType value);

protected:
    void assignRandom(long ix, long iy, long iz);
    void buildQuads();
    int size;

private:

    void copyDataToGPU();

    boost::mutex m_mutex;
    scoped_array<BlockType> blockData;
    bool blockDataReady;

    GLuint vertexBuffer, normalBuffer;
    std::vector<float> verts, normals;
    std::size_t quads;

};

}
#endif // CHUNK_H
