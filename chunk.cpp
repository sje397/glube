#include "chunk.h"
#include "simplex.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include <QDebug>

namespace Glube {

Chunk::Chunk(std::size_t size_):
    size(size_),
    blockData(new unsigned char[size_ * size_ * size_]),
    vertexBuffer(0),
    normalBuffer(0),
    quads(0)
{
    for(std::size_t i = 0; i < size * size * size; ++i) {
        blockData[i] = 0;
    }

    glGenBuffers(1, &vertexBuffer);
    glGenBuffers(1, &normalBuffer);
}

Chunk::~Chunk() {
    glDeleteBuffers(1, &normalBuffer);
    glDeleteBuffers(1, &vertexBuffer);
}

void Chunk::gen(long ix, long iy, long iz)
{
    const int hs = size/2;
    //const float r = size/2;
    for(int x = -hs; x < hs; ++x) {
        for(int y = -hs; y < hs; ++y) {
            for(int z = -hs; z < hs; ++z) {
                if(y > 0) setBlock(x, y, z, 0);
                else
                {
                    const float n = simplex_noise(3, x / (float)hs/2 + ix, y / (float)hs/2 + iy, z / (float)hs/2 + iz);
                    BlockType block = n > 2.8f ? 0 : 1;
                    setBlock(x, y, z, block);
                }
            }
        }
    }
}

void Chunk::draw() {
    if(quads == 0) {
        buildQuads();
    }

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
       1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
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

Chunk::BlockType Chunk::getBlock(std::size_t x, std::size_t y, std::size_t z) const
{
    return blockData[x + size/2 + (y + size/2) * size + (z + size/2) * size * size];
}

void Chunk::setBlock(std::size_t x, std::size_t y, std::size_t z, Chunk::BlockType value)
{
    blockData[x + size/2 + (y + size/2) * size + (z + size/2) * size * size] = value;
    quads = 0;
}


#define push(v, x, y, z) v.push_back(x); v.push_back(y); v.push_back(z);
void Chunk::buildQuads()
{
    quads = 0;
    std::vector<float> verts, normals;
    const int hs = size/2;
    for(int x = -hs; x < hs; ++x)
    {
        for(int y = -hs; y < hs; ++y)
        {
            for(int z = -hs; z < hs; ++z)
            {
                BlockType block = getBlock(x, y, z);
                if(block) {
                    if(x == -hs || (x > -hs && !getBlock(x - 1, y, z))) {
                        //left
                        push(verts, x - 0.5, y - 0.5, z - 0.5);
                        push(verts, x - 0.5, y + 0.5, z - 0.5);
                        push(verts, x - 0.5, y + 0.5, z + 0.5);
                        push(verts, x - 0.5, y - 0.5, z + 0.5);

                        push(normals, -1, 0, 0);
                        push(normals, -1, 0, 0);
                        push(normals, -1, 0, 0);
                        push(normals, -1, 0, 0);
                        quads++;
                    }
                    if(x == hs - 1 || (x < hs - 1 && !getBlock(x + 1, y, z))) {
                        //right
                        push(verts, x + 0.5, y - 0.5, z - 0.5);
                        push(verts, x + 0.5, y + 0.5, z - 0.5);
                        push(verts, x + 0.5, y + 0.5, z + 0.5);
                        push(verts, x + 0.5, y - 0.5, z + 0.5);

                        push(normals, 1, 0, 0);
                        push(normals, 1, 0, 0);
                        push(normals, 1, 0, 0);
                        push(normals, 1, 0, 0);
                        quads++;
                    }
                    if(z == -hs || (z > -hs && !getBlock(x, y, z - 1))) {
                        //forward
                        push(verts, x - 0.5, y - 0.5, z - 0.5);
                        push(verts, x + 0.5, y - 0.5, z - 0.5);
                        push(verts, x + 0.5, y + 0.5, z - 0.5);
                        push(verts, x - 0.5, y + 0.5, z - 0.5);

                        push(normals, 0, 0, -1);
                        push(normals, 0, 0, -1);
                        push(normals, 0, 0, -1);
                        push(normals, 0, 0, -1);
                        quads++;
                    }
                    if(z == hs - 1 || (z < hs - 1 && !getBlock(x, y, z + 1))) {
                        //back
                        push(verts, x - 0.5, y - 0.5, z + 0.5);
                        push(verts, x + 0.5, y - 0.5, z + 0.5);
                        push(verts, x + 0.5, y + 0.5, z + 0.5);
                        push(verts, x - 0.5, y + 0.5, z + 0.5);

                        push(normals, 0, 0, 1);
                        push(normals, 0, 0, 1);
                        push(normals, 0, 0, 1);
                        push(normals, 0, 0, 1);
                        quads++;
                    }
                    if(y == -hs || (y > -hs && !getBlock(x, y - 1, z))) {
                        //up
                        push(verts, x - 0.5, y - 0.5, z - 0.5);
                        push(verts, x + 0.5, y - 0.5, z - 0.5);
                        push(verts, x + 0.5, y - 0.5, z + 0.5);
                        push(verts, x - 0.5, y - 0.5, z + 0.5);

                        push(normals, 0, -1, 0);
                        push(normals, 0, -1, 0);
                        push(normals, 0, -1, 0);
                        push(normals, 0, -1, 0);
                        quads++;
                    }
                    if(y == hs - 1 || (y < hs - 1 && !getBlock(x, y + 1, z))) {
                        //down
                        push(verts, x - 0.5, y + 0.5, z - 0.5);
                        push(verts, x + 0.5, y + 0.5, z - 0.5);
                        push(verts, x + 0.5, y + 0.5, z + 0.5);
                        push(verts, x - 0.5, y + 0.5, z + 0.5);

                        push(normals, 0, 1, 0);
                        push(normals, 0, 1, 0);
                        push(normals, 0, 1, 0);
                        push(normals, 0, 1, 0);
                        quads++;
                    }
                }
            }
        }
    }

    qDebug() << "Quads:" << quads << ", verts" << verts.size();

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), &verts[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);
}

}
