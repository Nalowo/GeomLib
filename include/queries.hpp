#pragma once
#include "geometry.hpp"
#include "intersections.hpp"
#include <algorithm>
#include <optional>
#include <variant>

namespace geometry::queries {

template <class... Ts>
struct Multilambda : Ts... {
    using Ts::operator()...;
};

inline double DistanceToPoint(const Shape &shape, const Point2D &point);

/*
 * Класс для поиска расстояния от точки до фигуры
 *
 * Требуется организовать возможность нахождения расстояния для всех возможных фигур типа-суммы Shape
 */
struct PointToShapeDistanceVisitor {
    double operator()(const Point2D &p, const Shape &shape) { return DistanceToPoint(shape, p); }
};

/*
 * Класс для поиска расстояния между двумя фигурами
 *
 * Требуется организовать возможность нахождения расстояния только для следующих комбинаций фигур:
 *    - Any    & Point
 *    - Line   & Line
 *    - Circle & Circle
 *
 * Для всех остальных требуется вернуть пустое значение
 */
struct ShapeToShapeDistanceVisitor {
    std::optional<double> operator()(const Line &L, const Point2D &p) const noexcept { return DistanceToPoint(L, p); }
    std::optional<double> operator()(const Point2D &p, const Line &L) const noexcept { return DistanceToPoint(L, p); }
    std::optional<double> operator()(const Triangle &T, const Point2D &p) const noexcept {
        return DistanceToPoint(T, p);
    }
    std::optional<double> operator()(const Point2D &p, const Triangle &T) const noexcept {
        return DistanceToPoint(T, p);
    }
    std::optional<double> operator()(const Rectangle &R, const Point2D &p) const noexcept {
        return DistanceToPoint(R, p);
    }
    std::optional<double> operator()(const Point2D &p, const Rectangle &R) const noexcept {
        return DistanceToPoint(R, p);
    }
    std::optional<double> operator()(const RegularPolygon &P, const Point2D &p) const noexcept {
        return DistanceToPoint(P, p);
    }
    std::optional<double> operator()(const Point2D &p, const RegularPolygon &P) const noexcept {
        return DistanceToPoint(P, p);
    }
    std::optional<double> operator()(const Circle &C, const Point2D &p) const noexcept { return DistanceToPoint(C, p); }
    std::optional<double> operator()(const Point2D &p, const Circle &C) const noexcept { return DistanceToPoint(C, p); }
    std::optional<double> operator()(const Polygon &Poly, const Point2D &p) const noexcept {
        return DistanceToPoint(Poly, p);
    }
    std::optional<double> operator()(const Point2D &p, const Polygon &Poly) const noexcept {
        return DistanceToPoint(Poly, p);
    }

    std::optional<double> operator()(const Line &a, const Line &b) const noexcept {
        if (intersections::IntersectionVisitor{}(a, b).has_value())
            return 0.0;
        double d1 = DistanceToPoint(a, b.start);
        double d2 = DistanceToPoint(a, b.end);
        double d3 = DistanceToPoint(b, a.start);
        double d4 = DistanceToPoint(b, a.end);
        return std::min({d1, d2, d3, d4});
    }

    std::optional<double> operator()(const Circle &c1, const Circle &c2) const noexcept {
        double d = c1.center_p.DistanceTo(c2.center_p);
        double gap = d - (c1.radius + c2.radius);
        return gap > 0.0 ? gap : 0.0;
    }

    std::optional<double> operator()(const auto& a, const auto& b) const noexcept {
        return std::nullopt;
    }
};  // end class ShapeToShapeDistanceVisitor

inline double pointSegmentDistance(const Point2D &p, const Line &seg) noexcept {
    Point2D v = seg.end - seg.start;
    Point2D w = p - seg.start;
    double c1 = w.Dot(v);
    if (c1 <= 0.0) {
        return p.DistanceTo(seg.start);
    }
    double c2 = v.Dot(v);
    if (c2 <= c1) {
        return p.DistanceTo(seg.end);
    }
    double t = c1 / c2;
    Point2D proj = seg.start + v * t;
    return p.DistanceTo(proj);
}

inline bool pointInPolygon(const Point2D &pt, const std::vector<Point2D> &verts) noexcept {
    bool inside = false;
    size_t n = verts.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        const auto &vi = verts[i];
        const auto &vj = verts[j];
        bool intersect =
            ((vi.y > pt.y) != (vj.y > pt.y)) && (pt.x < (vj.x - vi.x) * (pt.y - vi.y) / (vj.y - vi.y) + vi.x);
        if (intersect)
            inside = !inside;
    }
    return inside;
}

/*
 * Функции-помощники
 */
inline double DistanceToPoint(const Shape &shape, const Point2D &point) {
    return std::visit(Multilambda{[&](const Line &L) -> double { return pointSegmentDistance(point, L); },
                                  [&](const Triangle &T) -> double {
                                      auto verts = T.Vertices();
                                      if (pointInPolygon(point, verts))
                                          return 0.0;
                                      double d0 = pointSegmentDistance(point, Line{verts[0], verts[1]});
                                      double d1 = pointSegmentDistance(point, Line{verts[1], verts[2]});
                                      double d2 = pointSegmentDistance(point, Line{verts[2], verts[0]});
                                      return std::min({d0, d1, d2});
                                  },
                                  [&](const Rectangle &R) -> double {
                                      auto verts = R.Vertices();
                                      if (pointInPolygon(point, verts))
                                          return 0.0;
                                      double best = std::numeric_limits<double>::infinity();
                                      for (size_t i = 0; i < verts.size(); ++i) {
                                          Line e{verts[i], verts[(i + 1) % verts.size()]};
                                          best = std::min(best, pointSegmentDistance(point, e));
                                      }
                                      return best;
                                  },
                                  [&](const RegularPolygon &P) -> double {
                                      auto verts = P.Vertices();
                                      if (pointInPolygon(point, verts))
                                          return 0.0;
                                      double best = std::numeric_limits<double>::infinity();
                                      for (size_t i = 0; i < verts.size(); ++i) {
                                          Line e{verts[i], verts[(i + 1) % verts.size()]};
                                          best = std::min(best, pointSegmentDistance(point, e));
                                      }
                                      return best;
                                  },
                                  [&](const Circle &C) -> double {
                                      double d = point.DistanceTo(C.center_p);
                                      return d <= C.radius ? 0.0 : (d - C.radius);
                                  },
                                  [&](const Polygon &poly) -> double {
                                      auto verts = poly.Vertices();
                                      if (pointInPolygon(point, verts))
                                          return 0.0;

                                      double best = std::numeric_limits<double>::infinity();
                                      size_t n = verts.size();
                                      for (size_t i = 0; i < n; ++i) {
                                          Line edge{verts[i], verts[(i + 1) % n]};
                                          best = std::min(best, pointSegmentDistance(point, edge));
                                      }
                                      return best;
                                  }},
                      shape);
}

inline BoundingBox GetBoundBox(const Shape &shape) {
    return std::visit([](const auto &s) { return s.BoundBox(); }, shape);
}

inline double GetHeight(const Shape &shape) {
    return std::visit([](const auto &s) { return s.Height(); }, shape);
}

inline bool BoundingBoxesOverlap(const Shape &shape1, const Shape &shape2) {
    return std::visit([](const auto &lhs, const auto &rhs)
    {
        return GetBoundBox(lhs).Overlaps(GetBoundBox(rhs));
    }, shape1, shape2);
}

std::optional<double> DistanceBetweenShapes(const Shape &shape1, const Shape &shape2) {
    return std::visit(ShapeToShapeDistanceVisitor{}, shape1, shape2);
}

}  // namespace geometry::queries