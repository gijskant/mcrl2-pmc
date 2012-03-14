#ifndef GRAPH_H
#define GRAPH_H

#include <QString>
#include <QtOpenGL>

namespace Graph
{

struct Coord3D
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
    Coord3D(GLfloat x, GLfloat y, GLfloat z)
        : x(x), y(y), z(z) {}
    Coord3D() : x(0), y(0), z(0) {}
    Coord3D& operator+=(const Coord3D& a)
    {
        x += a.x;
        y += a.y;
        z += a.z;
        return *this;
    }
    Coord3D& operator-=(const Coord3D& a)
    {
        x -= a.x;
        y -= a.y;
        z -= a.z;
        return *this;
    }

    Coord3D& operator*=(float f)
    {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }

    Coord3D& operator/=(float f)
    {
        x /= f;
        y /= f;
        z /= f;
        return *this;
    }


    Coord3D operator+(const Coord3D& a) const
    {
        return Coord3D(a.x + x, a.y + y, a.z + z);
    }
    Coord3D operator-(const Coord3D& a) const
    {
        return Coord3D(x - a.x, y - a.y, z - a.z);
    }
    Coord3D operator*(GLfloat c) const
    {
        return Coord3D(c * x, c * y, c * z);
    }
    Coord3D operator-() const
    {
        return Coord3D(-x, -y, -z);
    }
    Coord3D operator/(GLfloat c) const
    {
        return Coord3D(x / c, y / c, z / c);
    }
    float size() const
    {
        return sqrt(x * x + y * y + z * z);
    }
    float dot(const Coord3D& a) const
    {
        return x * a.x + y * a.y + z * a.z;
    }
    Coord3D cross(const Coord3D& a) const
    {
        return Coord3D(
           y * a.z - z * a.y,
           z * a.x - x * a.z,
           x * a.y - y * a.x
        );
    }
    void clip(const Coord3D& min, const Coord3D& max)
    {
        if (x < min.x) x = min.x;
        if (x > max.x) x = max.x;
        if (y < min.y) y = min.y;
        if (y > max.y) y = max.y;
        if (z < min.z) z = min.z;
        if (z > max.z) z = max.z;
    }
    operator const GLfloat*() const { return &x; }
    bool operator==(const Coord3D& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }
    bool operator!=(const Coord3D& other) const
    {
        return !operator==(other);
    }
};

enum NodeType
{
    nt_node,
    nt_label,
    nt_handle
};

struct Edge
{
    size_t from;
    size_t to;
    float selected;
    Edge (size_t from, size_t to, float selected)
        : from(from), to(to), selected(selected) {}
};

struct Node
{
    Coord3D pos;
    unsigned anchored : 1;
    unsigned locked : 1;
    float selected;
};

struct LabelNode : public Node
{
    size_t labelindex;
};

namespace detail
{
    class GraphImplBase;
}

/**
  @brief: This is the internal data structure that LTSGraph operates on.

    In its implementation it uses the mCRL2 lts classes to represent the graphs,
    and augments it with further information. In particular, positions of labels
    and edge handles are stored as if they were nodes.
*/
class Graph{
private:
    detail::GraphImplBase* m_impl;
public:
    Graph();
    ~Graph();
    Edge edge(size_t index) const;
    Node& node(size_t index) const;
    Node& handle(size_t edge) const;
    LabelNode& label(size_t edge) const;
    const QString& labelstring(size_t labelindex) const;
    const std::map<size_t, QString*>::iterator labels() const;
    void clip(const Coord3D& min, const Coord3D& max);
    void selectEdge(size_t index, float amount);
    size_t edgeCount() const;
    size_t nodeCount() const;
    size_t labelCount() const;
    void load(const QString& filename, const Coord3D& min, const Coord3D& max);
};

}

#endif // GRAPH_H
