#ifndef WIDGET_H
#define WIDGET_H

#define GL_GLEXT_PROTOTYPES 1
#include <QGLWidget>
#include <QGLShaderProgram>

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

private:
    void LoadShaders();

    QGLShaderProgram shaderProg;
    GLuint vertexArrayID;
    GLuint gridVertexbuffer;
};

#endif // WIDGET_H
