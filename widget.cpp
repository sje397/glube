#include "widget.h"

#include <QTimer>
#include <QGLShader>
#include <QFile>
#include <QDebug>
#include <QCursor>
#include <QApplication>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <boost/shared_ptr.hpp>
using boost::shared_ptr;

const float UpdatePeriod = 0.02;
const float FoV = M_PI / 4;
const float RenderDistance = 400;
const float FogStart = 350;
const float LoadBufferDistance = 100;

const float SPEED = 20;
const float RSPEED = M_PI / 2;
const float MOUSE_RSPEED = M_PI / 2 / 200;
const float MAX_PITCH = M_PI / 2 * 0.9;
const float CHUNK_SIZE = 128;
const float GRAVITY = -10;
const float JETPACK = 20;

Widget::Widget(QWidget *parent) :
    QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer | QGL::Rgba ), parent),
    vao(),
    yawRate(0),
    jets(false),
    activeCam(0),
    nodeFactory(CHUNK_SIZE)
{
    //srand(QDateTime::currentMSecsSinceEpoch());
    srand(2);

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(updateGL()));
    t->setInterval(1000 * UpdatePeriod);
    t->start();

    setMouseTracking(true);
    setCursor( QCursor( Qt::BlankCursor ) );
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
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
    currentMapNode->build();

    cam[0].setPosition(glm::vec3(0, CHUNK_SIZE / 2.0f + 2, 0));

    shaderProg.setUniformValue("RenderDistance", RenderDistance);
    shaderProg.setUniformValue("FogStart", FogStart);
}

void Widget::resizeGL(int w, int h)
{
    if(w > 0 && h > 0)
    {
        glViewport(0, 0, (GLint)w, (GLint)h);
        glm::mat4 projectionMatrix = glm::perspective(FoV, static_cast<float>(w)/h, 0.1f, RenderDistance);
        int pLoc = shaderProg.uniformLocation("projectionMatrix");
        glUniformMatrix4fv(pLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
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

    // position
    float yaw = fmod(cam[activeCam].getYaw() + yawRate * RSPEED * UpdatePeriod, M_PI * 2);
    glm::mat4 yawMatrix = glm::rotate(glm::mat4(1.0f), -yaw, glm::vec3(0, 1, 0));

    glm::vec4 delta(motion * SPEED * UpdatePeriod, 1.0);
    delta = glm::transpose(yawMatrix) * delta;

    glm::vec3 newPos = cam[activeCam].getPosition();
    shared_ptr<Glube::MapNode> newMapNode = currentMapNode;
    //float newYVelocity = yVelocity;

    if(activeCam == 0) {
        newPos.x += delta.x;
        newPos.y = std::min((float)CHUNK_SIZE * 2, std::max(newPos.y + delta.y, 0.0f));
        newPos.z += delta.z;

        if(newPos.z > CHUNK_SIZE / 2) {
            newPos.z += -CHUNK_SIZE;
            newMapNode = newMapNode->getNext(Glube::MapNode::SOUTH);
        } else if(newPos.z < -CHUNK_SIZE / 2) {
            newPos.z += CHUNK_SIZE;
            newMapNode = newMapNode->getNext(Glube::MapNode::NORTH);
        } else if(newPos.x > CHUNK_SIZE / 2) {
            newPos.x += -CHUNK_SIZE;
            newMapNode = newMapNode->getNext(Glube::MapNode::EAST);
        } else if(newPos.x < -CHUNK_SIZE / 2) {
            newPos.x += CHUNK_SIZE;
            newMapNode = newMapNode->getNext(Glube::MapNode::WEST);
        }

        /*
        float accel = (GRAVITY + (jets ? JETPACK : 0)) * UpdatePeriod;
        newYVelocity = std::min(0.5f, std::max(newYVelocity + accel, -0.5f));
        newPos.y = std::min((float)CHUNK_SIZE, std::max(newPos.y + newYVelocity, 0.0f));

        // height 1.75, eyes at 1.5
        bool g1 = newMapNode->getBlock(newPos.x, newPos.y - 0.5f - 1.5f, newPos.z) == 0;
        bool g2 = newMapNode->getBlock(newPos.x, newPos.y - 0.5f - 1.5f + 1.75f / 2, newPos.z) == 0;
        bool g3 = newMapNode->getBlock(newPos.x, newPos.y - 0.5f + 0.25f, newPos.z) == 0;
        bool legal = g2 && g3;
        if(legal) {
            if(!g1 && newYVelocity < 0) {
                newPos.y = (int)(newPos.y - 0.5f - 1.5f) + 2.0f;
                newYVelocity = 0;
            }
            currentMapNode = newMapNode;
            position = newPos;
            yVelocity = newYVelocity;
        } else if(!g3 && newYVelocity > 0) {
            yVelocity = 0;
        }
        */
        currentMapNode = newMapNode;
    } else {
        newPos.x += delta.x;
        newPos.y += delta.y;
        newPos.z += delta.z;
    }
    // camera
    cam[activeCam].setPosition(newPos);
    cam[activeCam].setYaw(yaw);
    cam[activeCam].setView(shaderProg);

    // draw
    glm::mat4 modelMatrix(1.0f);
    glm::mat4 view = cam[0].viewMatrix();

    const float ChunkDiag = CHUNK_SIZE/2.0f * 1.414;

    Glube::MapNode::List nodes, filteredNodes;
    currentMapNode->findRecursive(glm::vec3(0, 0, 0), RenderDistance + LoadBufferDistance + ChunkDiag, nodes);
    foreach(Glube::MapNode* n, nodes) {
        n->startBuild();
        glm::vec4 camPos = view * glm::vec4(n->pos(), 1.0f);
        glm::vec2 cp(camPos.x, camPos.z);
        // additional angle based on distance
        double angle = atan2(ChunkDiag, glm::length(cp));
        if(glm::length(cp) < ChunkDiag
           || (glm::acos(glm::dot(glm::normalize(cp), glm::vec2(0, -1))) < FoV + angle))
        {
            filteredNodes.append(n);
        }
    }
    //qDebug() << "Drawing" << filteredNodes.size() << "nodes.";
    foreach(Glube::MapNode* n, filteredNodes) {
        n->draw(shaderProg, modelMatrix);
    }
    foreach(Glube::MapNode* n, visibleNodes) {
        if(!filteredNodes.contains(n)) {
            n->deleteBuffers();
        }
    }
    visibleNodes = nodes;
}

void Widget::keyPressEvent(QKeyEvent *e)
{
    if(!e->isAutoRepeat()) {
        switch(e->key()) {
            case Qt::Key_A: motion += glm::vec3(-1, 0, 0); break;
            case Qt::Key_D: motion += glm::vec3(1, 0, 0); break;
            case Qt::Key_W:
                motion += glm::vec3(0, 0, -1); break;
            case Qt::Key_S: motion += glm::vec3(0, 0, 1); break;

            case Qt::Key_Z: motion += glm::vec3(0, -1, 0); break;
            case Qt::Key_X: motion += glm::vec3(0, 1, 0); break;

            case Qt::Key_J: yawRate += 1; break;
            case Qt::Key_L: yawRate += -1; break;

            case Qt::Key_Space: jets = true; break;

            case Qt::Key_1: activeCam = 0; break;
            case Qt::Key_2: activeCam = 1; break;
            case Qt::Key_3: activeCam = 2; break;

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

            case Qt::Key_Space: jets = false; break;

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

        // only move pointer to centre on first update
        static bool first = true;
        if(!first) {
            cam[activeCam].setYaw(cam[activeCam].getYaw() - dx * MOUSE_RSPEED);
            cam[activeCam].setPitch(std::min(MAX_PITCH, std::max(-MAX_PITCH, cam[activeCam].getPitch() - dy * MOUSE_RSPEED)));
        } else {
            first = false;
        }
    }
}

void Widget::LoadShaders()
{
    shaderProg.addShaderFromSourceFile(QGLShader::Vertex, ":/shaders/vertex.shader");
    shaderProg.addShaderFromSourceFile(QGLShader::Fragment, ":/shaders/fragment.shader");
    shaderProg.link();
    shaderProg.bind();
}

