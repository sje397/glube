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
const float CHUNK_SIZE = 128;

Widget::Widget(QWidget *parent) :
    QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer | QGL::Rgba ), parent),
    vao(),
    yaw(0),
    yawRate(0),
    pitch(0),
    pitchRate(0),
    nodeFactory(CHUNK_SIZE)
{
    //srand(QDateTime::currentMSecsSinceEpoch());
    srand(2);

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
    makeCurrent(); // so context is current for vao/chunk etc destructors
}

void Widget::initializeGL()
{
    LoadShaders();

    vao.allocate();

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    currentMapNode = nodeFactory.getMapNode(0, 0);

    position = glm::vec3(0, CHUNK_SIZE/2, 0);
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
    //glClearColor(1.0, 1.0, 1.0, 1.0);

    vao.bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //camera
    yaw = fmod(yaw + yawRate * RSPEED, M_PI * 2);
    cam = glm::rotate(glm::mat4(1.0f), -yaw, glm::vec3(0, 1, 0));
    glm::vec4 delta(motion * SPEED, 1.0);
    delta = glm::transpose(cam) * delta;
    position.x += delta.x;
    //position.y += delta.y;
    position.y = std::min(CHUNK_SIZE / 2.0f, std::max(position.y + delta.y, -CHUNK_SIZE / 2.0f));
    position.z += delta.z;

    if(position.z > CHUNK_SIZE / 2) {
        position.z += -CHUNK_SIZE;
        currentMapNode = currentMapNode->getNext(Glube::MapNode::SOUTH);
    } else if(position.z < -CHUNK_SIZE / 2) {
        position.z += CHUNK_SIZE;
        currentMapNode = currentMapNode->getNext(Glube::MapNode::NORTH);
    } else if(position.x > CHUNK_SIZE / 2) {
        position.x += -CHUNK_SIZE;
        currentMapNode = currentMapNode->getNext(Glube::MapNode::EAST);
    } else if(position.x < -CHUNK_SIZE / 2) {
        position.x += CHUNK_SIZE;
        currentMapNode = currentMapNode->getNext(Glube::MapNode::WEST);
    }

    glm::vec4 xax = glm::transpose(cam) * glm::vec4(1, 0, 0, 1);
    cam = glm::rotate(cam, -pitch, glm::vec3(xax.x, xax.y, xax.z));
    cam = glm::translate(cam, -position);
    int vLoc = shaderProg.uniformLocation("viewMatrix");
    glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(cam));
    int eyeLoc = shaderProg.uniformLocation("eyePosition");
    glUniform3fv(eyeLoc, 1, glm::value_ptr(position));

    glm::mat4 modelMatrix(1.0f);

    QList<Glube::MapNode*> nodes;
    currentMapNode->findRecursive(glm::vec3(0, 0, 0), 2.5f * CHUNK_SIZE, nodes);
    foreach(Glube::MapNode *n, nodes) {
        glm::vec4 camPos = cam * glm::vec4(n->pos(), 1.0f);
        if(camPos.z < CHUNK_SIZE/2)
            n->draw(shaderProg, modelMatrix);
    }
    //currentMapNode->draw(shaderProg, modelMatrix);
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

