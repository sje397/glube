#include "chunk.h"
#include "simplex.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <boost/thread/locks.hpp>

#include <QDebug>

namespace Glube {

Chunk::Chunk(int size_):
    size(size_),
    blockData(new unsigned char[size_ * size_ * size_]),
    blockDataReady(false),
    vertexBuffer(0),
    normalBuffer(0),
    quads(0)
{
}

Chunk::~Chunk() {
    deleteBuffers();
}

void Chunk::draw() {
    if(quads != 0) {
        copyDataToGPU();

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(
           0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
           3,                  // size
           GL_FLOAT,           // type
           GL_FALSE,           // normalized?
           0,                  // stride
           (void*)0            // array buffer offset
        );
        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
        glVertexAttribPointer(
           1,                  // attribute 1. No particular reason for 0, but must match the layout in the shader.
           3,                  // size
           GL_FLOAT,           // type
           GL_FALSE,           // normalized?
           0,                  // stride
           (void*)0            // array buffer offset
        );

        glDrawArrays(GL_QUADS, 0, quads * 4);

        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
    }
}

void Chunk::deleteBuffers()
{
    if(normalBuffer && vertexBuffer) {
        if(glIsBuffer(normalBuffer)) glDeleteBuffers(1, &normalBuffer);
        if(glIsBuffer(vertexBuffer)) glDeleteBuffers(1, &vertexBuffer);
        normalBuffer = 0;
        vertexBuffer = 0;
    }
}

Chunk::BlockType Chunk::getBlock(int x, int y, int z)
{
    boost::mutex::scoped_lock lock(m_mutex);
    return blockData[x + size/2 + y * size + (z + size/2) * size * size];
}

void Chunk::setBlock(int x, int y, int z, Chunk::BlockType value)
{
    blockData[x + size/2 + y * size + (z + size/2) * size * size] = value;
    deleteBuffers();
}

void Chunk::assignRandom(long ix, long iy, long iz)
{
    boost::mutex::scoped_lock lock(m_mutex);
    if(!blockDataReady) {
        qDebug() << "Generating block data for (" << ix << "," << iy << "," << iz << ")";
        const int hs = size/2;
        for(int x = -hs; x < hs; ++x) {
            for(int y = 0; y < size; ++y) {
                for(int z = -hs; z < hs; ++z) {
                    const float n = simplex_noise(1, x / (float)hs/2 + ix + 500, y / (float)size + iy + 500, z / (float)hs/2 + iz + 500);
                    //const float hf = (y < size/2 ? 1 : 1 - (y - size/2) / (float)(size / 2));
                    const float hf = pow((1 - y/(float)size) * 2, 2);
                    BlockType block = n * hf > 1.0f ? 0 : 1;
                    setBlock(x, y, z, block);

                    sched_yield();
                }
            }
        }
        blockDataReady = true;
        qDebug() << "Generated block data for (" << ix << "," << iy << "," << iz << ")";
    }
}


#define push(v, x, y, z) v.push_back(x); v.push_back(y); v.push_back(z);
void Chunk::buildQuads()
{
    if(!blockDataReady)
        return;

    std::vector<float> rVerts, rNormals;
    std::size_t rQuads = 0;
    const int hs = size/2;
    for(int x = -hs; x < hs; ++x)
    {
        for(int y = 0; y < size; ++y)
        {
            for(int z = -hs; z < hs; ++z)
            {
                BlockType block = getBlock(x, y, z);
                if(block) {
                    if(!getBlock(x - 1, y, z)) {
                        //left
                        push(rVerts, x - 0.5, y - 0.5, z - 0.5);
                        push(rVerts, x - 0.5, y + 0.5, z - 0.5);
                        push(rVerts, x - 0.5, y + 0.5, z + 0.5);
                        push(rVerts, x - 0.5, y - 0.5, z + 0.5);

                        push(rNormals, -1, 0, 0);
                        push(rNormals, -1, 0, 0);
                        push(rNormals, -1, 0, 0);
                        push(rNormals, -1, 0, 0);
                        rQuads++;
                    }
                    if(!getBlock(x + 1, y, z)) {
                        //right
                        push(rVerts, x + 0.5, y - 0.5, z - 0.5);
                        push(rVerts, x + 0.5, y + 0.5, z - 0.5);
                        push(rVerts, x + 0.5, y + 0.5, z + 0.5);
                        push(rVerts, x + 0.5, y - 0.5, z + 0.5);

                        push(rNormals, 1, 0, 0);
                        push(rNormals, 1, 0, 0);
                        push(rNormals, 1, 0, 0);
                        push(rNormals, 1, 0, 0);
                        rQuads++;
                    }
                    if(!getBlock(x, y, z - 1)) {
                        //forward
                        push(rVerts, x - 0.5, y - 0.5, z - 0.5);
                        push(rVerts, x + 0.5, y - 0.5, z - 0.5);
                        push(rVerts, x + 0.5, y + 0.5, z - 0.5);
                        push(rVerts, x - 0.5, y + 0.5, z - 0.5);

                        push(rNormals, 0, 0, -1);
                        push(rNormals, 0, 0, -1);
                        push(rNormals, 0, 0, -1);
                        push(rNormals, 0, 0, -1);
                        rQuads++;
                    }
                    if(!getBlock(x, y, z + 1)) {
                        //back
                        push(rVerts, x - 0.5, y - 0.5, z + 0.5);
                        push(rVerts, x + 0.5, y - 0.5, z + 0.5);
                        push(rVerts, x + 0.5, y + 0.5, z + 0.5);
                        push(rVerts, x - 0.5, y + 0.5, z + 0.5);

                        push(rNormals, 0, 0, 1);
                        push(rNormals, 0, 0, 1);
                        push(rNormals, 0, 0, 1);
                        push(rNormals, 0, 0, 1);
                        rQuads++;
                    }
                    if(y > 0 && !getBlock(x, y - 1, z)) {
                        //up
                        push(rVerts, x - 0.5, y - 0.5, z - 0.5);
                        push(rVerts, x + 0.5, y - 0.5, z - 0.5);
                        push(rVerts, x + 0.5, y - 0.5, z + 0.5);
                        push(rVerts, x - 0.5, y - 0.5, z + 0.5);

                        push(rNormals, 0, -1, 0);
                        push(rNormals, 0, -1, 0);
                        push(rNormals, 0, -1, 0);
                        push(rNormals, 0, -1, 0);
                        rQuads++;
                    }
                    if(y < size - 1 && !getBlock(x, y + 1, z)) {
                        //down
                        push(rVerts, x - 0.5, y + 0.5, z - 0.5);
                        push(rVerts, x + 0.5, y + 0.5, z - 0.5);
                        push(rVerts, x + 0.5, y + 0.5, z + 0.5);
                        push(rVerts, x - 0.5, y + 0.5, z + 0.5);

                        push(rNormals, 0, 1, 0);
                        push(rNormals, 0, 1, 0);
                        push(rNormals, 0, 1, 0);
                        push(rNormals, 0, 1, 0);
                        rQuads++;
                    }
                    sched_yield();
                }
            }
        }
    }

    qDebug() << "Quads:" << rQuads << ", verts" << rVerts.size();

    {
        verts = rVerts;
        normals = rNormals;
        quads = rQuads;
    }

}

void Chunk::copyDataToGPU()
{
    if(quads > 0 && (!vertexBuffer || !normalBuffer)) {
        if(!vertexBuffer) glGenBuffers(1, &vertexBuffer);
        if(!normalBuffer) glGenBuffers(1, &normalBuffer);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), &verts[0], GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_DYNAMIC_DRAW);
    }
}

}
