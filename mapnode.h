#ifndef MAPNODE_H
#define MAPNODE_H

#include "drawable.h"
#include "chunk.h"

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
using boost::scoped_ptr;
using boost::shared_ptr;

#include <map>

namespace Glube {

class MapNode;

class MapNodeFactory
{
public:
    MapNodeFactory(std::size_t chunkSize);
    shared_ptr<MapNode> getMapNode(long x, long y);
    std::size_t getChunkSize() const;
private:
    std::size_t chunkSize;
    std::map<QString, shared_ptr<MapNode> > nodes;
};

class MapNode: public Drawable
{
public:
    static const int NORTH = 0;
    static const int EAST = 1;
    static const int SOUTH = 2;
    static const int WEST = 3;

    MapNode(long x, long y, std::size_t chunkSize, MapNodeFactory &fact);
    virtual ~MapNode();

    void draw(QGLShaderProgram &shaderProg, const glm::mat4 &parentModelMatrix);
    shared_ptr<MapNode> getNext(int direction);

    void findRecursive(const glm::vec3 &pos, float radius, QList<MapNode*> &nodeList);
private:
    long x, y;
    MapNodeFactory &factory;
    scoped_ptr<Chunk> chunk;
};

}
#endif // MAPNODE_H
