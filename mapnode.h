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

class MapNode: public Drawable, public Chunk
{
public:
    static const int NORTH = 0;
    static const int EAST = 1;
    static const int SOUTH = 2;
    static const int WEST = 3;

    MapNode(long x, long z, std::size_t chunkSize, MapNodeFactory &fact);
    virtual ~MapNode();
    void startBuild();
    void assignRandom();
    void build();

    void draw(QGLShaderProgram &shaderProg, const glm::mat4 &parentModelMatrix);
    void deleteBuffers();
    virtual Chunk::BlockType getBlock(int x, int y, int z);
    virtual void setBlock(int x, int y, int z, BlockType value);
    shared_ptr<MapNode> getNext(int direction);

    typedef QList<MapNode*> List;
    void findRecursive(const glm::vec3 &pos, float radius, List &nodeList);
private:
    long x, z;
    MapNodeFactory &factory;
    boost::mutex m_mutex;
    scoped_ptr<boost::thread> m_buildThread;
    bool built;
};

}
#endif // MAPNODE_H
