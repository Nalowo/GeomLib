#include "convex_hull.hpp"
#include <algorithm>
#include "details.hpp"

namespace geometry::convex_hull {

double CrossProduct(Point2D p1, Point2D middle, Point2D p2) noexcept {
    auto new_p1 = p1 - middle;
    auto new_p2 = p2 - middle;
    return new_p1.Cross(new_p2);
}

GeometryResult<std::vector<Point2D>> GrahamScan(std::vector<Point2D> points) {
    if (points.size() < 3)
        return std::unexpected{GeometryError::InsufficientPoints};

    auto it = std::min_element(points.begin(), points.end(), [](const Point2D &a, const Point2D &b) {
        return a.y < b.y || (a.y == b.y && a.x < b.x);
    });
    Point2D pStart = *it;
    std::swap(points[0], *it);

    std::ranges::sort(points.begin() + 1, points.end(), [pStart](const Point2D& a, const Point2D& b) {
        Point2D va = a - pStart, vb = b - pStart;
        double cross = va.Cross(vb);
        if (std::abs(cross) < details::EPSILON)
            return va.Length() < vb.Length();
        return cross > 0;
    });
    
    StackForGrahamScan stack;
    stack.Reserve(points.size());
    stack.Push(points[0]);
    stack.Push(points[1]);
    for (size_t i = 2; i < points.size(); ++i) 
    {
        while (stack.Size() >= 2 && CrossProduct(stack.Top(), stack.NextToTop(), points[i]) <= 0) {
            stack.Pop();
        }
        stack.Push(points[i]);
    }
    return stack.Extract();
}

}  // namespace geometry::convex_hull