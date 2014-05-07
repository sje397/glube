#ifndef VAO_H
#define VAO_H

#define GL_GLEXT_PROTOTYPES 1
#include <QtOpenGL>

namespace Glube {

class VAO
{
public:
    VAO();
    virtual ~VAO();

    void allocate();
    void bind();

private:
    GLuint vertexArrayID;
};

}

#endif // VAO_H
