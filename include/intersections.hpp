#pragma once
#include "geometry.hpp"
#include <cmath>
#include <optional>

namespace geometry::intersections {

/*
 * Класс для поиска пересечений между двумя фигурами
 *
 * Требуется организовать возможность нахождения пересечений только для следующих комбинаций фигур:
 *    - Line   & Line
 *    - Line   & Circle
 *    - Circle & Circle
 *
 * Для всех остальных требуется выбросить исключение std::logic_error
 */
class IntersectionVisitor {
public:
    using ReturnType = GeometryResult<Point2D>;
    ReturnType operator()(const Line &L1, const Line &L2) noexcept {
        Point2D P = L1.start;
        Point2D r = L1.end - L1.start;
        Point2D Q = L2.start;
        Point2D s = L2.end - L2.start;

        double rxs = r.Cross(s);
        Point2D QP = Q - P;
        double qpxr = QP.Cross(r);

        // Параллельны или коллинеарны
        if (std::abs(rxs) < std::numeric_limits<double>::epsilon()) {
            return std::unexpected(GeometryError::NoIntersection);
        }

        double t = QP.Cross(s) / rxs;
        double u = qpxr / rxs;

        if (t >= 0.0 && t <= 1.0 && u >= 0.0 && u <= 1.0) {
            return P + r * t;
        }
        return std::unexpected(GeometryError::NoIntersection);
    }
    ReturnType operator()(const Line &L, const Circle &C) {
        Point2D P = L.start;
        Point2D r = L.end - L.start;
        Point2D d = P - C.center_p;

        double a = r.Dot(r);
        double b = 2.0 * d.Dot(r);
        double c = d.Dot(d) - C.radius * C.radius;
        double disc = b * b - 4 * a * c;
        if (disc < 0.0) {
            return std::unexpected(GeometryError::NoIntersection);  // нет действительных корней
        }

        double sqrt_disc = std::sqrt(disc);
        double t1 = (-b - sqrt_disc) / (2 * a);
        double t2 = (-b + sqrt_disc) / (2 * a);

        double t = std::numeric_limits<double>::infinity();
        if (0.0 <= t1 && t1 <= 1.0)
            t = t1;
        else if (0.0 <= t2 && t2 <= 1.0)
            t = t2;

        if (t == std::numeric_limits<double>::infinity())
            return std::unexpected(GeometryError::DegenrateCase);

        return P + r * t;
    }
    ReturnType operator()(const Circle &C, const Line &L) noexcept { return (*this)(L, C); }
    ReturnType operator()(const Circle &C1, const Circle &C2) {
        Point2D d = C2.center_p - C1.center_p;
        double dist = std::hypot(d.x, d.y);
        double r0 = C1.radius, r1 = C2.radius;

        if (dist > r0 + r1 || dist < std::abs(r0 - r1) || dist == 0.0)
            return std::unexpected(GeometryError::NoIntersection);

        double a = (r0 * r0 - r1 * r1 + dist * dist) / (2 * dist);
        double h = std::sqrt(r0 * r0 - a * a);

        Point2D P2 = C1.center_p + d * (a / dist);
        Point2D offset{-d.y * (h / dist), d.x * (h / dist)};
        Point2D I1 = P2 + offset;
        return I1;
    }
    ReturnType operator()(const auto &, const auto &) {
        throw std::logic_error("Unsupported types");
        return std::unexpected(GeometryError::InvalidInput);
    }
};

inline GeometryResult<Point2D> GetIntersectPoint(const Shape &shape1, const Shape &shape2) {
    return std::visit(IntersectionVisitor{}, shape1, shape2);
}

}  // namespace geometry::intersections