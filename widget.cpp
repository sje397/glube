#include "widget.h"

#include <QTimer>
#include <QGLShader>
#include <QFile>
#include <QDebug>

Widget::Widget(QWidget *parent) :
    QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer | QGL::Rgba), parent)
{
    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(updateGL()));
    t->setInterval(0);
    t->start();
}

Widget::~Widget()
{
    makeCurrent();
    glDeleteBuffers(1, &gridVertexbuffer);
    glDeleteVertexArrays(1, &vertexArrayID);
}

void Widget::initializeGL()
{
    LoadShaders();

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);

    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &gridVertexbuffer);

    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, gridVertexbuffer);

    const int squares = 200;
    const double side = 200.0, y = -1.0;
    double verts[(squares + 1) * 4 * 3];
    double *pos = verts;

    for(int i = 0; i <= squares; ++i)
    {
        //glVertex3d(i, -1, -100);
        pos[0] = side / squares * i - side / 2;
        pos[1] = y;
        pos[2] = -side / 2;

        pos += 3;
        //glVertex3d(i, -1, 100);
        pos[0] = side / squares * i - side / 2;
        pos[1] = y;
        pos[2] = side / 2;

        pos += 3;

        //glVertex3d(-100, -1, i);
        pos[0] = -side / 2;
        pos[1] = y;
        pos[2] = side / squares * i - side / 2;

        pos += 3;
        //glVertex3d(100, -1, i);
        pos[0] = side / 2;
        pos[1] = y;
        pos[2] = side / squares * i - side / 2;

        pos += 3;
    }
    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts, GL_STATIC_DRAW);
}

void Widget::resizeGL(int w, int h)
{
    glViewport(0, 0, (GLint)w, (GLint)h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void Widget::paintGL()
{
    static double rot = 0.0;
    glBindVertexArray(vertexArrayID);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    rot += 0.2;
    glRotated(rot, 0, 1, 0);
    glColor3d(0.0, 1.0, 0.0);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
       0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
       3,                  // size
       GL_DOUBLE,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)0            // array buffer offset
    );

    glBindBuffer(GL_ARRAY_BUFFER, gridVertexbuffer);
    // Draw the triangle !
    glDrawArrays(GL_LINES, 0, 800); // Starting from vertex 0; 3 vertices total -> 1 triangle

    glDisableVertexAttribArray(0);
    glPopMatrix();
}

void Widget::LoadShaders()
{
    shaderProg.addShaderFromSourceFile(QGLShader::Vertex, ":/shaders/vertex.shader");
    shaderProg.addShaderFromSourceFile(QGLShader::Fragment, ":/shaders/fragment.shader");
    shaderProg.link();
    shaderProg.bind();
}

