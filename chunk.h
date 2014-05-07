#ifndef CHUNK_H
#define CHUNK_H

#include "drawable.h"

#include <boost/scoped_array.hpp>
using boost::scoped_array;

namespace Glube {

class Chunk
{
public:
    Chunk(std::size_t size);
    virtual ~Chunk();

    void gen(long ix, long iy, long iz);

    virtual void draw();
private:
    typedef unsigned char BlockType;
    BlockType getBlock(std::size_t x, std::size_t y, std::size_t z) const;
    void setBlock(std::size_t x, std::size_t y, std::size_t z, BlockType value);

    void buildQuads();

    std::size_t size;
    scoped_array<BlockType> blockData;
    GLuint vertexBuffer, normalBuffer;
    std::size_t quads;
};

}
#endif // CHUNK_H
