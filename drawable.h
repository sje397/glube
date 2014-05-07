#ifndef DRAWABLE_H
#define DRAWABLE_H

#define GL_GLEXT_PROTOTYPES 1
#include <QGLShaderProgram>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace Glube {

class Drawable
{
public:
    Drawable();
    Drawable(const glm::vec3 &position);
    virtual ~Drawable();

    virtual void draw(QGLShaderProgram &shaderProg, const glm::mat4 &parentModelMatrix);

    virtual glm::vec3 pos() const;
    virtual void setPos(const glm::vec3 &p);
protected:
    glm::vec3 position;
};

}

#endif // DRAWABLE_H
