#ifndef EDGE_H
#define EDGE_H

#include <iostream>

#include <vtkType.h>

class Edge
{
public:
    Edge();
    Edge(vtkIdType s, vtkIdType e);
    Edge(vtkIdType s, vtkIdType e, bool b);
    Edge(Edge& e);

    vtkIdType u() const;
    vtkIdType v() const;
    bool boundary() const;
    size_t axisAligned() const;

    bool operator==(const Edge& e) const;
    bool operator<(const Edge& e) const;
    bool operator<=(const Edge& e) const;

    void operator=(const Edge& e);

private:
    vtkIdType m_u;
    vtkIdType m_v;
    bool m_boundary;
    size_t m_axisAligned;
};

#endif // EDGE_H
