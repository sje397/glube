#include "mapnode.h"

#include <QDebug>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Glube {

MapNodeFactory::MapNodeFactory(std::size_t chunkSize_): chunkSize(chunkSize_), nodes()
{
}

shared_ptr<MapNode> MapNodeFactory::getMapNode(long x, long y)
{
    QString key = QString("%1x%2").arg(x).arg(y);
    std::map<QString, shared_ptr<MapNode> >::iterator i = nodes.find(key);
    if(i == nodes.end()) {
        qDebug() << "Creating map node (" << x << "," << y << ")";
        MapNode *node = new MapNode(x, y, chunkSize, *this);
        nodes[key].reset(node);
    }
    return nodes[key];
}

std::size_t MapNodeFactory::getChunkSize() const
{
    return chunkSize;
}


MapNode::MapNode(long x_, long z_, std::size_t chunkSize, MapNodeFactory& fact):
    Drawable(glm::vec3(x_ * chunkSize - chunkSize / 2.0f, 0, z_ * chunkSize - chunkSize / 2.0f)),
    Chunk(chunkSize),
    x(x_), z(z_),
    factory(fact),
    built(false)
{
}

MapNode::~MapNode()
{
    if(m_buildThread) {
        m_buildThread->join();
    }
}

void MapNode::startBuild()
{
    if(!built && !m_buildThread) {
        m_buildThread.reset(new boost::thread(boost::bind(&MapNode::build, this)));
    }
}

void MapNode::assignRandom()
{
    Chunk::assignRandom(x, 0, z);
}

void MapNode::build() {
    if(!built) {
        qDebug() << "Building (" << x << "," << z << ")";
        getNext(NORTH)->assignRandom();
        getNext(EAST)->assignRandom();
        getNext(SOUTH)->assignRandom();
        getNext(WEST)->assignRandom();
        assignRandom();
        buildQuads();
        built = true;
        qDebug() << "Built (" << x << "," << z << ")";
    }
}

void MapNode::draw(QGLShaderProgram& shaderProg, const glm::mat4& parentModelMatrix)
{
    Drawable::draw(shaderProg, parentModelMatrix);
    if(built) {
        Chunk::draw();
    } else {
        startBuild();
    }
}

void MapNode::deleteBuffers()
{
    Chunk::deleteBuffers();
}

Chunk::BlockType MapNode::getBlock(int x, int y, int z)
{
    int si = size/2;
    if(y < 0 || y >= size) return 0;
    MapNode* n = this;
    if(x < -si) {
        n = n->getNext(WEST).get();
        x += size;
    } else if(x >= si) {
        n = n->getNext(EAST).get();
        x -= size;
    }
    if(z < -si) {
        n = n->getNext(NORTH).get();
        z += size;
    } else if(z >= si) {
        n = n->getNext(SOUTH).get();
        z -= size;
    }
    return n->Chunk::getBlock(x, y, z);
}

void MapNode::setBlock(int x, int y, int z, BlockType value)
{
    Chunk::setBlock(x, y, z, value);
    built = false;
}

shared_ptr<MapNode> MapNode::getNext(int direction)
{
    switch(direction) {
    case NORTH: return factory.getMapNode(x, z - 1);
    case EAST: return factory.getMapNode(x + 1, z);
    case SOUTH: return factory.getMapNode(x, z + 1);
    case WEST: return factory.getMapNode(x - 1, z);
    }

    return shared_ptr<MapNode>();
}

void MapNode::findRecursive(const glm::vec3 &pos, float radius, List& nodeList)
{
    if(nodeList.contains(this)) return;
    if(radius < glm::length(pos)) return;
    nodeList.append(this);

    setPos(pos);

    for(int i = 0; i < 4; ++i) {
        MapNode *n = getNext(i).get();

        float d = static_cast<float>(factory.getChunkSize());
        switch(i) {
        case NORTH: n->findRecursive(pos + glm::vec3(0, 0, -d), radius, nodeList); break;
        case EAST: n->findRecursive(pos + glm::vec3(d, 0, 0), radius, nodeList); break;
        case SOUTH: n->findRecursive(pos + glm::vec3(0, 0, d), radius, nodeList); break;
        case WEST: n->findRecursive(pos + glm::vec3(-d, 0, 0), radius, nodeList); break;
        }
    }
}

}
