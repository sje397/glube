#include "drawable.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Glube {

Drawable::Drawable()
{
}

Drawable::Drawable(const glm::vec3& pos):
    position(pos)
{

}

Drawable::~Drawable()
{
}

void Drawable::draw(QGLShaderProgram &shaderProg, const glm::mat4 &parentModelMatrix)
{
    glm::mat4 modelMatrix = glm::translate(parentModelMatrix, position);
    int mLoc = shaderProg.uniformLocation("modelMatrix");
    glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
}

glm::vec3 Drawable::pos() const
{
    return position;
}

void Drawable::setPos(const glm::vec3& p)
{
    position = p;
}

}
