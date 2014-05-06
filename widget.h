#ifndef WIDGET_H
#define WIDGET_H

#define GL_GLEXT_PROTOTYPES 1
#include <QGLWidget>
#include <QGLShaderProgram>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <QKeyEvent>
#include <QMouseEvent>

class Widget : public QGLWidget
{
    Q_OBJECT
public:
    explicit Widget(QWidget *parent = 0);
    virtual ~Widget();
signals:

public slots:

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

private:
    void LoadShaders();

    QGLShaderProgram shaderProg;
    GLuint vertexArrayID;
    GLuint gridVertexBuffer, gridIndexBuffer;

    glm::mat4 cam;
    glm::vec3 motion, position;
    float yaw, yawRate, pitch, pitchRate;
};

#endif // WIDGET_H
