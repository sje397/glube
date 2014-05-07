#include "vao.h"

namespace Glube {

VAO::VAO():
    vertexArrayID(0)
{
}

VAO::~VAO()
{
    if(vertexArrayID) {
        glDeleteVertexArrays(1, &vertexArrayID);
    }
}

void VAO::allocate()
{
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);
}

void VAO::bind()
{
    glBindVertexArray(vertexArrayID);
}

}
