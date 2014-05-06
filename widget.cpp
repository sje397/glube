#include "widget.h"

#include <QTimer>
#include <QGLShader>
#include <QFile>
#include <QDebug>
#include <QCursor>
#include <QApplication>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

const float FoV = M_PI / 4;
const float SPEED = 0.5;
const float RSPEED = M_PI / 200;
const float MOUSE_RSPEED = M_PI / 200 / 10;
const float MAX_PITCH = M_PI / 2 * 0.9;

Widget::Widget(QWidget *parent) :
    QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer | QGL::Rgba ), parent),
    yaw(0),
    yawRate(0),
    pitch(0),
    pitchRate(0)
{
    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(updateGL()));
    t->setInterval(0);
    t->start();

    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    setMouseTracking(true);
    setCursor( QCursor( Qt::BlankCursor ) );
}

Widget::~Widget()
{
    makeCurrent();
    glDeleteBuffers(1, &gridVertexBuffer);
    glDeleteBuffers(1, &gridIndexBuffer);
    glDeleteVertexArrays(1, &vertexArrayID);
}

void Widget::initializeGL()
{
    LoadShaders();

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);

    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    const int squares = 1000;
    const float side = 1000.0;
    float y = 0;
    double *verts = new double[(squares + 1) * (squares + 1) * 3];
    double *pos = verts;
    unsigned int *indices = new unsigned int[squares * squares * 8];
    unsigned int index = 0;
    for(int i = 0; i <= squares; ++i)
    {
        for(int j = 0; j <= squares; ++j)
        {
            //glVertex3d(i, -1, -100);
            pos[0] = side / squares * i - side / 2;
            pos[1] = 0;
            pos[2] = side / squares * j - side / 2;

            //if(sqrt(pos[0]*pos[0]+pos[2]*pos[2] < 200)) {
                pos[1] = random() / (float)RAND_MAX - 0.5;
            //}
            pos += 3;

            if(i < squares && j < squares) {
                int v = (i * squares + j) * 8;
                indices[v] = index;
                indices[v + 1] = index + 1;
                indices[v + 2] = index + 1;
                indices[v + 3] = index + 1 + (squares + 1);
                indices[v + 4] = index + 1 + (squares + 1);
                indices[v + 5] = index + (squares + 1);
                indices[v + 6] = index + (squares + 1);
                indices[v + 7] = index;
            }
            index++;
        }
    }

//    {
//        int v = (500 * 480 + 10) * 8;
//        qDebug() << indices[v] << indices[v + 1] << indices[v + 2] << indices[v + 3] << indices[v + 4] << indices[v + 5] << indices[v + 6] << indices[v + 7];
//        for(int i = 0; i < 8; ++i) {
//            qDebug() << "(" << verts[indices[v + i] * 3] << ", " << verts[indices[v + i] * 3 + 1] << ", " << verts[indices[v + i] * 3 + 2] << ")";
//        }
//    }

    glGenBuffers(1, &gridVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, gridVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, (squares + 1) * (squares + 1) * 3 * sizeof(double), verts, GL_STATIC_DRAW);

    glGenBuffers(1, &gridIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, squares * squares * 8 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    delete[] verts;
    delete[] indices;

    position = glm::vec3(0, 2, 0);
}

void Widget::resizeGL(int w, int h)
{
    if(w > 0 && h > 0)
    {
        glViewport(0, 0, (GLint)w, (GLint)h);
        glm::mat4 projectionMatrix = glm::perspective(FoV, static_cast<float>(w)/h, 1.0f, 600.0f);
        int pLoc = shaderProg.uniformLocation("projectionMatrix");
        glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

        QCursor::setPos(mapToGlobal(QPoint(w / 2, h / 2)));
    }
}

void Widget::paintGL()
{
    static int clock = 0;
    static const int CLOCK_MAX = 1024;
    clock = (clock + 1) % CLOCK_MAX;

    float l = sin(clock / (float)CLOCK_MAX * M_PI * 2) / 2 + 0.5;
    int clockLoc = shaderProg.uniformLocation("clock");
    glUniform1f(clockLoc, l);

    glClearColor(135/255.0 * l, 196/255.0 * l, 250 / 255.0 * l, 1.0);

    glBindVertexArray(vertexArrayID);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //camera
    yaw = fmod(yaw + yawRate * RSPEED, M_PI * 2);
    cam = glm::rotate(glm::mat4(1.0f), -yaw, glm::vec3(0, 1, 0));
    glm::vec4 delta(motion * SPEED, 1.0);
    delta = glm::transpose(cam) * delta;
    position.x += delta.x;
    position.y += delta.y;
    position.z += delta.z;
    glm::vec4 xax = glm::transpose(cam) * glm::vec4(1, 0, 0, 1);
    cam = glm::rotate(cam, -pitch, glm::vec3(xax.x, xax.y, xax.z));
    cam = glm::translate(cam, -position);
    int vLoc = shaderProg.uniformLocation("viewMatrix");
    glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(cam));
    int eyeLoc = shaderProg.uniformLocation("eyePosition");
    glUniform3fv(eyeLoc, 1, glm::value_ptr(position));

    glm::mat4 modelMatrix;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0));
    int mLoc = shaderProg.uniformLocation("modelMatrix");
    glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, gridVertexBuffer);
    glVertexAttribPointer(
       0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
       3,                  // size
       GL_DOUBLE,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)0            // array buffer offset
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridIndexBuffer);
    glDrawElements(GL_LINES, 1000 * 1000 * 8, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
}

void Widget::keyPressEvent(QKeyEvent *e)
{
    if(!e->isAutoRepeat()) {
        switch(e->key()) {
            case Qt::Key_A: motion += glm::vec3(-1, 0, 0); break;
            case Qt::Key_D: motion += glm::vec3(1, 0, 0); break;
            case Qt::Key_W: motion += glm::vec3(0, 0, -1); break;
            case Qt::Key_S: motion += glm::vec3(0, 0, 1); break;

            case Qt::Key_Z: motion += glm::vec3(0, -1, 0); break;
            case Qt::Key_X: motion += glm::vec3(0, 1, 0); break;

            case Qt::Key_J: yawRate += 1; break;
            case Qt::Key_L: yawRate += -1; break;
            default: QGLWidget::keyPressEvent(e); break;
        }
    } else {
        QGLWidget::keyPressEvent(e);
    }
}

void Widget::keyReleaseEvent(QKeyEvent *e)
{
    if(!e->isAutoRepeat()) {
        switch(e->key()) {
            case Qt::Key_A: motion += glm::vec3(1, 0, 0); break;
            case Qt::Key_D: motion += glm::vec3(-1, 0, 0); break;
            case Qt::Key_W: motion += glm::vec3(0, 0, 1); break;
            case Qt::Key_S: motion += glm::vec3(0, 0, -1); break;

            case Qt::Key_Z: motion += glm::vec3(0, 1, 0); break;
            case Qt::Key_X: motion += glm::vec3(0, -1, 0); break;

            case Qt::Key_J: yawRate += -1; break;
            case Qt::Key_L: yawRate += 1; break;
            default: QGLWidget::keyReleaseEvent(e); break;
        }
    } else {
        QGLWidget::keyReleaseEvent(e);
    }
}

void Widget::mouseMoveEvent(QMouseEvent *e)
{
    int w = width(), h = height();
    if(w > 0 && h > 0) {
        QPoint pos = mapFromGlobal(e->globalPos());
        float dx = pos.x() - w / 2, dy = pos.y() - h / 2;
        QCursor::setPos(mapToGlobal(QPoint(w / 2, h / 2)));
        yaw += -dx * MOUSE_RSPEED;
        pitch = std::min(MAX_PITCH, std::max(-MAX_PITCH, pitch - dy * MOUSE_RSPEED));
    }
}

void Widget::LoadShaders()
{
    shaderProg.addShaderFromSourceFile(QGLShader::Vertex, ":/shaders/vertex.shader");
    shaderProg.addShaderFromSourceFile(QGLShader::Fragment, ":/shaders/fragment.shader");
    shaderProg.link();
    shaderProg.bind();
}

