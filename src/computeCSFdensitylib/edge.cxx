#include "edge.h"

Edge::Edge(): m_u(-1), m_v(-1), m_boundary(false) {}
Edge::Edge(vtkIdType s, vtkIdType e): m_u(s), m_v(e), m_boundary(false), m_axisAligned(0) {}
Edge::Edge(vtkIdType s, vtkIdType e, bool b): m_u(s), m_v(e), m_boundary(b), m_axisAligned(0) {}
Edge::Edge(Edge& e): m_u(e.u()), m_v(e.v()), m_boundary(e.boundary()), m_axisAligned(0) {}

vtkIdType Edge::u() const{
    return m_u;
}

vtkIdType Edge::v() const{
    return m_v;
}

bool Edge::boundary() const{
    return m_boundary;
}

size_t Edge::axisAligned() const{
    return m_axisAligned;
}

bool Edge::operator==(const Edge& e) const {
    return (m_u == e.u() && m_v == e.v()) || (m_u == e.v() && m_v == e.u());
}

bool Edge::operator<(const Edge& e) const {
    return (m_u == e.u() ? m_v < e.v() : m_u < e.u());
}

bool Edge::operator<=(const Edge& e) const {
    return (m_u == e.u() ? m_v <= e.v() : m_u <= e.u());
}

void Edge::operator=(const Edge& e) {
    m_u = e.u();
    m_v = e.v();
    m_boundary = e.boundary();
    m_axisAligned = e.axisAligned();
}
