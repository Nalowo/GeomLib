#pragma once
#include "details.hpp"
#include "geometry.hpp"
#include <algorithm>
#include <format>
#include <set>
#include <vector>

namespace geometry::triangulation {

struct DelaunayTriangle {
    Point2D a, b, c;

    DelaunayTriangle(Point2D a, Point2D b, Point2D c) : a(a), b(b), c(c) {}

    bool ContainsPoint(const Point2D &p) const {
        Point2D center = Circumcenter();
        double radius = Circumradius();
        return center.DistanceTo(p) <= radius + details::EPSILON;
    }

    Point2D Circumcenter() const {
        double d = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
        if (std::abs(d) < details::EPSILON) {
            return {(a.x + b.x + c.x) / 3, (a.y + b.y + c.y) / 3};
        }

        double ux = ((a.x * a.x + a.y * a.y) * (b.y - c.y) + (b.x * b.x + b.y * b.y) * (c.y - a.y) +
                     (c.x * c.x + c.y * c.y) * (a.y - b.y)) /
                    d;

        double uy = ((a.x * a.x + a.y * a.y) * (c.x - b.x) + (b.x * b.x + b.y * b.y) * (a.x - c.x) +
                     (c.x * c.x + c.y * c.y) * (b.x - a.x)) /
                    d;

        return {ux, uy};
    }

    double Circumradius() const {
        Point2D center = Circumcenter();
        return center.DistanceTo(a);
    }

    bool SharesEdge(const DelaunayTriangle &other) const {
        std::vector<Point2D> this_points = {a, b, c};
        std::vector<Point2D> other_points = {other.a, other.b, other.c};

        int shared_count = 0;
        for (const Point2D &p1 : this_points) {
            for (const Point2D &p2 : other_points) {
                if (std::abs(p1.x - p2.x) < details::EPSILON && std::abs(p1.y - p2.y) < details::EPSILON) {
                    shared_count++;
                    break;
                }
            }
        }

        return shared_count == 2;
    }

    bool operator==(const DelaunayTriangle &o) const noexcept {
        std::array<Point2D, 3> va{a, b, c}, vb{o.a, o.b, o.c};
        auto cmp = [](const Point2D &p1, const Point2D &p2) {
            if (std::abs(p1.x - p2.x) > details::EPSILON)
                return p1.x < p2.x;
            return p1.y < p2.y;
        };
        std::sort(va.begin(), va.end(), cmp);
        std::sort(vb.begin(), vb.end(), cmp);
        return va[0] == vb[0] && va[1] == vb[1] && va[2] == vb[2];
    }

    std::vector<Point2D> vertices() const { return {a, b, c}; }
};

struct Edge {
    Point2D p1, p2;

    Edge(Point2D p1, Point2D p2) : p1(p1), p2(p2) {
        if (p1.x > p2.x || (p1.x == p2.x && p1.y > p2.y)) {
            std::swap(this->p1, this->p2);
        }
    }

    bool operator<(const Edge &other) const {
        if (std::abs(p1.x - other.p1.x) > details::EPSILON)
            return p1.x < other.p1.x;
        if (std::abs(p1.y - other.p1.y) > details::EPSILON)
            return p1.y < other.p1.y;
        if (std::abs(p2.x - other.p2.x) > details::EPSILON)
            return p2.x < other.p2.x;
        return p2.y < other.p2.y;
    }

    bool operator==(const Edge &other) const {
        return std::abs(p1.x - other.p1.x) < details::EPSILON && std::abs(p1.y - other.p1.y) < details::EPSILON &&
               std::abs(p2.x - other.p2.x) < details::EPSILON && std::abs(p2.y - other.p2.y) < details::EPSILON;
    }
};

inline GeometryResult<std::vector<DelaunayTriangle>> DelaunayTriangulation(std::span<const Point2D> points) {
    if (points.size() < 3) {
        return std::unexpected(GeometryError::InsufficientPoints);
    }

    double min_x = points[0].x, max_x = points[0].x;
    double min_y = points[0].y, max_y = points[0].y;
    for (const auto &p : points) {
        min_x = std::min(min_x, p.x);
        max_x = std::max(max_x, p.x);
        min_y = std::min(min_y, p.y);
        max_y = std::max(max_y, p.y);
    }
    double dx = max_x - min_x;
    double dy = max_y - min_y;
    double max_d = std::max(dx, dy) * 2;

    Point2D super1(min_x - max_d, min_y - max_d);
    Point2D super2(max_x + max_d, min_y - max_d);
    Point2D super3((min_x + max_x) / 2, max_y + max_d);
    std::vector<DelaunayTriangle> triangulation{DelaunayTriangle(super1, super2, super3)};

    for (const auto &p : points) {
        std::vector<DelaunayTriangle> bad_triangles;
        for (const auto &tri : triangulation) {
            if (tri.ContainsPoint(p)) {
                bad_triangles.push_back(tri);
            }
        }

        std::set<Edge> polygon;
        for (const auto &tri : bad_triangles) {
            auto verts = tri.vertices();
            Edge edges[3] = {{verts[0], verts[1]}, {verts[1], verts[2]}, {verts[2], verts[0]}};
            for (const auto &edge : edges) {
                if (auto it = polygon.find(edge); it == polygon.end()) {
                    polygon.insert(edge);
                } else {
                    polygon.erase(it);
                }
            }
        }

        triangulation.erase(std::remove_if(triangulation.begin(), triangulation.end(),
                                           [&bad_triangles](const DelaunayTriangle &tri) {
                                               return std::find(bad_triangles.begin(), bad_triangles.end(), tri) !=
                                                      bad_triangles.end();
                                           }),
                            triangulation.end());

        for (const auto &edge : polygon) {
            triangulation.emplace_back(edge.p1, edge.p2, p);
        }
    }

    triangulation.erase(std::remove_if(triangulation.begin(), triangulation.end(),
                                       [&](const DelaunayTriangle &tri) {
                                           auto verts = tri.vertices();
                                           for (auto const &v : verts) {
                                               bool isSuper1 = std::abs(v.x - super1.x) < details::EPSILON &&
                                                               std::abs(v.y - super1.y) < details::EPSILON;
                                               bool isSuper2 = std::abs(v.x - super2.x) < details::EPSILON &&
                                                               std::abs(v.y - super2.y) < details::EPSILON;
                                               bool isSuper3 = std::abs(v.x - super3.x) < details::EPSILON &&
                                                               std::abs(v.y - super3.y) < details::EPSILON;
                                               if (isSuper1 || isSuper2 || isSuper3)
                                                   return true;
                                           }
                                           return false;
                                       }),
                        triangulation.end());

    return triangulation;
}
}  // namespace geometry::triangulation

template <>
struct std::formatter<geometry::triangulation::DelaunayTriangle> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::triangulation::DelaunayTriangle &t, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "DelaunayTriangle({}, {}, {})", t.a, t.b, t.c);
    }
};
