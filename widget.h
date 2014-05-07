#ifndef WIDGET_H
#define WIDGET_H

#define GL_GLEXT_PROTOTYPES 1
#include <QGLWidget>
#include <QGLShaderProgram>

#include <QKeyEvent>
#include <QMouseEvent>

#include "mapnode.h"
#include "vao.h"

#include <memory>

#include <boost/shared_ptr.hpp>

using boost::shared_ptr;

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
    Glube::VAO vao;
    glm::mat4 cam;
    glm::vec3 motion, position;
    float yaw, yawRate, pitch, pitchRate;

    Glube::MapNodeFactory nodeFactory;
    shared_ptr<Glube::MapNode> currentMapNode;
};

#endif // WIDGET_H
