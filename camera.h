#ifndef CAMERA_H
#define CAMERA_H

#define GL_GLEXT_PROTOTYPES 1
#include <QGLShaderProgram>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace Glube {

class Camera
{
public:
    Camera();
    virtual ~Camera();

    void setPosition(const glm::vec3 &pos);
    void setYaw(float yaw);
    void setPitch(float pitch);
    void setView(const QGLShaderProgram &shaderProg);
    glm::mat4 viewMatrix() const;
    glm::vec3 getPosition() const;
    float getYaw() const;
    float getPitch() const;
private:
    glm::vec3 position;
    float yaw, pitch;
    glm::mat4 view;
};

}
#endif // CAMERA_H
