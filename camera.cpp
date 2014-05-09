#include "camera.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Glube {

Camera::Camera():
    position(),
    yaw(0),
    pitch(0)
{
}

Camera::~Camera()
{

}

void Camera::setPosition(const glm::vec3& pos)
{
    position = pos;
}

void Camera::setYaw(float yaw_)
{
    yaw = yaw_;
}

void Camera::setPitch(float pitch_)
{
    pitch = pitch_;
}

void Camera::setView(const QGLShaderProgram &shaderProg)
{
    glm::mat4 yawMatrix = glm::rotate(glm::mat4(1.0f), -yaw, glm::vec3(0, 1, 0));
    glm::vec4 xax = glm::transpose(yawMatrix) * glm::vec4(1, 0, 0, 1);
    view = glm::rotate(yawMatrix, -pitch, glm::vec3(xax.x, xax.y, xax.z));
    view = glm::translate(view, -position);
    int vLoc = shaderProg.uniformLocation("viewMatrix");
    glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(view));
    int eyeLoc = shaderProg.uniformLocation("eyePosition");
    glUniform3fv(eyeLoc, 1, glm::value_ptr(position));
}

glm::mat4 Camera::viewMatrix() const
{
    return view;
}

glm::vec3 Camera::getPosition() const
{
    return position;
}

float Camera::getYaw() const
{
    return yaw;
}

float Camera::getPitch() const
{
    return pitch;
}

}
