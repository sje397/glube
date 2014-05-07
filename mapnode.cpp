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


MapNode::MapNode(long x_, long y_, std::size_t chunkSize, MapNodeFactory& fact):
    x(x_), y(y_),
    factory(fact),
    chunk(new Chunk(chunkSize))
{
    chunk->gen(x, 0, -y);
}

MapNode::~MapNode()
{
}

void MapNode::draw(QGLShaderProgram& shaderProg, const glm::mat4& parentModelMatrix)
{
    Drawable::draw(shaderProg, parentModelMatrix);
    chunk->draw();
}

shared_ptr<MapNode> MapNode::getNext(int direction)
{
    switch(direction) {
    case NORTH: return factory.getMapNode(x, y + 1);
    case EAST: return factory.getMapNode(x + 1, y);
    case SOUTH: return factory.getMapNode(x, y - 1);
    case WEST: return factory.getMapNode(x - 1, y);
    }

    return shared_ptr<MapNode>();
}

void MapNode::findRecursive(const glm::vec3 &pos, float radius, QList<MapNode*>& nodeList)
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
