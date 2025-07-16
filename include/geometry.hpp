#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <expected>
#include <format>
#include <numbers>
#include <optional>
#include <print>
#include <ranges>
#include <variant>
#include <vector>

namespace geometry {
/*
 * Добавьте к методам класса Point2D и Lines2DDyn все необходимые аттрибуты и спецификаторы
 * Важно: Возвращаемый тип и принимаемые аргументы менять не нужно
 */
struct Point2D {
    double x, y;

    constexpr Point2D() noexcept : x(0), y(0) {}
    constexpr Point2D(double x_, double y_) noexcept : x(x_), y(y_) {}

    // Comparison
    constexpr bool operator<(const Point2D &other) const noexcept { return x < other.x && y < other.y; }
    constexpr bool operator==(const Point2D &other) const noexcept { return x == other.x && y == other.y; }

    // Binary math operators
    constexpr Point2D operator+(const Point2D &other) const noexcept { return {x + other.x, y + other.y}; }
    constexpr Point2D operator-(const Point2D &other) const noexcept { return {x - other.x, y - other.y}; }
    constexpr Point2D operator*(double value) const noexcept { return {x * value, y * value}; }
    constexpr Point2D operator/(double value) const noexcept { return {x / value, y / value}; }

    // Binary geometry operations
    constexpr double Dot(const Point2D &other) const noexcept { return x * other.x + y * other.y; }
    constexpr double Cross(const Point2D &other) const noexcept { return x * other.y - y * other.x; }
    double Length() const noexcept { return std::sqrt(x * x + y * y); }
    double DistanceTo(const Point2D &other) const noexcept { return (*this - other).Length(); }

    Point2D Normalize() const noexcept {
        const double len = Length();
        return len > 0 ? Point2D{x / len, y / len} : Point2D{0, 0};
    }
};

template <size_t N>
struct Lines2D {
    std::array<double, N> x;
    std::array<double, N> y;
};

struct Lines2DDyn {
    std::vector<double> x;
    std::vector<double> y;

    // Reserve — меняет контейнер, может бросить bad_alloc, поэтому без noexcept
    void Reserve(size_t n) {
        x.reserve(n);
        y.reserve(n);
    }

    // PushBack — модифицирует, может бросать bad_alloc
    void PushBack(Point2D p) {
        x.push_back(p.x);
        y.push_back(p.y);
    }
    void PushBack(double px, double py) {
        x.push_back(px);
        y.push_back(py);
    }

    // Front возвращает копию, не модифицирует контейнер
    Point2D Front() const { return {x.front(), y.front()}; }
};

struct BoundingBox {
    double min_x, min_y, max_x, max_y;

    bool Overlaps(const BoundingBox &other) noexcept {
        if (max_x < other.min_x || other.max_x < min_x)
            return false;
        if (max_y < other.min_y || other.max_y < min_y)
            return false;
        return true;
    }
    double Width() const noexcept { return max_x - min_x; }
    double Height() const noexcept { return max_y - min_y; }
    Point2D Center() const { return {(min_x + max_x) * 0.5, (min_y + max_y) * 0.5}; }
};

using Vector2D = std::pair<double, double>;

struct Line {
    Point2D start, end;

    Vector2D Delta() const noexcept { return {end.x - start.x, end.y - start.y}; }
    double Length() const noexcept {
        auto dir = Delta();
        return std::hypot(dir.first, dir.second);
    }
    Vector2D Direction(bool normalize = false) const noexcept {
        auto [dx, dy] = Delta();
        double len = std::hypot(dx, dy);
        if (len < std::numeric_limits<double>::epsilon()) {
            return {0.0, 0.0};
        }
        return {dx / len, dy / len};
    }
    BoundingBox BoundBox() const {
        BoundingBox bb;
        bb.min_x = std::min(start.x, end.x);
        bb.max_x = std::max(start.x, end.x);
        bb.min_y = std::min(start.y, end.y);
        bb.max_y = std::max(start.y, end.y);
        return bb;
    }
    // double Height() const noexcept { return 0; }
    Point2D Center() const { return {(start.x + end.x) * 0.5, (start.y + end.y) * 0.5}; }
    std::vector<Point2D> Vertices() const {
        std::vector<Point2D> res;
        res.reserve(2);
        res.push_back(start);
        res.push_back(end);
        return res;
    }
    // std::vector<Line> Lines() const { return std::vector<Line>{}; }
};

struct Triangle {
    Point2D a, b, c;

    double Area() const noexcept { return 0; }
    double Height() const noexcept { return 0; }
    Point2D Center() const { return Point2D{}; }
    BoundingBox BoundBox() const { return BoundingBox{}; }
    std::vector<Point2D> Vertices() const { return std::vector<Point2D>{}; }
    std::vector<Line> Lines() const { return std::vector<Line>{}; }
};

struct Rectangle {
    Point2D bottom_left;
    double width, height;

    double Area() const noexcept { return 0; }
    double Height() const noexcept { return 0; }
    Point2D Center() const { return Point2D{}; }
    BoundingBox BoundBox() const { return BoundingBox{}; }
    std::vector<Point2D> Vertices() const { return std::vector<Point2D>{}; }
    std::vector<Line> Lines() const { return std::vector<Line>{}; }
};

struct RegularPolygon {
    Point2D center_p;
    double radius;
    int sides;

    constexpr RegularPolygon(Point2D center, double radius, int sides)
        : center_p(center), radius(radius), sides(sides) {}

    std::vector<Point2D> Vertices() const {
        std::vector<Point2D> points;
        points.reserve(sides);

        for (int i = 0; i < sides; ++i) {
            const double angle = 2 * std::numbers::pi * i / sides;
            points.emplace_back(center_p.x + radius * std::cos(angle), center_p.y + radius * std::sin(angle));
        }
        return points;
    }

    double Height() const noexcept { return 0; }
    Point2D Center() const { return Point2D{}; }
    BoundingBox BoundBox() const { return BoundingBox{}; }
    std::vector<Line> Lines() const { return std::vector<Line>{}; }
};

struct Circle {
    Point2D center_p;
    double radius;

    constexpr Circle(Point2D center, double radius) : center_p(center), radius(radius) {}

    BoundingBox BoundBox() const {
        return {center_p.x - radius, center_p.y - radius, center_p.x + radius, center_p.y + radius};
    }
    double Height() const { return center_p.y + radius; }
    Point2D Center() const { return center_p; }

    //
    // Должны быть сделана по аналогии с RegularPolygon::Vertices
    //
    std::vector<Point2D> Vertices(size_t N = 30) const { return {}; }
    Lines2DDyn Lines(size_t N = 100) const { return {}; }
};

class Polygon {
public:
    std::vector<Point2D> Vertices() const { return std::vector<Point2D>{}; }
    double Height() const noexcept { return 0; }
    Point2D Center() const { return Point2D{}; }
    BoundingBox BoundBox() const { return BoundingBox{}; }
    std::vector<Line> Lines() const { return std::vector<Line>{}; }

private:
    std::vector<Point2D> points_;
    BoundingBox bounding_box_;
};

using Shape = std::variant<Line, Triangle, Rectangle, RegularPolygon, Circle, Polygon>;

struct DummyClass {
    DummyClass(std::vector<Shape>) {}
};

enum class GeometryError { Unsupported, NoIntersection, InvalidInput, DegenrateCase, InsufficientPoints };

template <typename T>
using GeometryResult = std::expected<T, GeometryError>;

}  // namespace geometry

template <>
struct std::formatter<geometry::Point2D> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Point2D &p, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "({:.2f}, {:.2f})", p.x, p.y);
    }
};

template <>
struct std::formatter<std::vector<geometry::Point2D>> {
    bool use_new_line = false;

    constexpr auto parse(std::format_parse_context &ctx) {
        auto it = ctx.begin();

        /* ваш код здесь */

        return it;
    }

    template <typename FormatContext>
    auto format(const std::vector<geometry::Point2D> &v, FormatContext &ctx) const {

        /* ваш код здесь */
        return ctx.out();
    }
};

template <>
struct std::formatter<geometry::Line> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Line &l, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "Line({}, {})", l.start, l.end);
    }
};

template <>
struct std::formatter<geometry::Circle> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Circle &c, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "Circle(center={}, r={:.2f})", c.center_p, c.radius);
    }
};

template <>
struct std::formatter<geometry::Rectangle> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Rectangle &r, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "Rectangle(bottom_left={}, w={:.2f}, h={:.2f})", r.bottom_left, r.width,
                              r.height);
    }
};

template <>
struct std::formatter<geometry::RegularPolygon> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::RegularPolygon &p, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "RegularPolygon(center={}, r={:.2f}, sides={})", p.center_p, p.radius,
                              p.sides);
    }
};
template <>
struct std::formatter<geometry::Triangle> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Triangle &t, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "Triangle({}, {}, {})", t.a, t.b, t.c);
    }
};
template <>
struct std::formatter<geometry::Polygon> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Polygon &poly, FormatContext &ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "Polygon[{} points]: [", poly.Vertices().size());

        for (const auto &p : poly.Vertices()) {
            out = std::format_to(out, "{} ", p);
        }

        return std::format_to(out, "]");
    }
};
