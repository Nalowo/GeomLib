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

    [[nodiscard]] bool Overlaps(const BoundingBox &other) noexcept {
        if (max_x < other.min_x || other.max_x < min_x)
            return false;
        if (max_y < other.min_y || other.max_y < min_y)
            return false;
        return true;
    }
    [[nodiscard]] double Width() const noexcept { return max_x - min_x; }
    [[nodiscard]] double Height() const noexcept { return max_y - min_y; }
    [[nodiscard]] Point2D Center() const noexcept { return {(min_x + max_x) * 0.5, (min_y + max_y) * 0.5}; }
};

using Vector2D = std::pair<double, double>;

struct Line {
    Point2D start, end;

    [[nodiscard]] Vector2D Delta() const noexcept { return {end.x - start.x, end.y - start.y}; }
    [[nodiscard]] double Length() const noexcept {
        auto dir = Delta();
        return std::hypot(dir.first, dir.second);
    }
    [[nodiscard]] Vector2D Direction(bool normalize = false) const noexcept {
        auto [dx, dy] = Delta();
        double len = std::hypot(dx, dy);
        if (len < std::numeric_limits<double>::epsilon())
            return {0.0, 0.0};
        return normalize ? Vector2D{dx / len, dy / len} : Vector2D{dx, dy};
    }
    [[nodiscard]] BoundingBox BoundBox() const noexcept {
        return {std::min(start.x, end.x), std::min(start.y, end.y), std::max(start.x, end.x), std::max(start.y, end.y)};
    }
    [[nodiscard]] Point2D Center() const noexcept { return {(start.x + end.x) * 0.5, (start.y + end.y) * 0.5}; }
    [[nodiscard]] std::vector<Point2D> Vertices() const { return {start, end}; }
    [[nodiscard]] double Height() const noexcept { return std::abs(end.y - start.y); }
    [[nodiscard]] Lines2D<2> Lines() const noexcept {
        Lines2D<2> out;
        out.x = {start.x, end.x};
        out.y = {start.y, end.y};
        return out;
    }
};

struct Triangle {
    Point2D a, b, c;

    [[nodiscard]] double Area() const noexcept {
        double cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
        return std::abs(cross) * 0.5;
    }
    [[nodiscard]] double Height() const noexcept {
        double base = std::hypot(b.x - a.x, b.y - a.y);
        return base > 0.0 ? (2.0 * Area() / base) : 0.0;
    }
    [[nodiscard]] Point2D Center() const noexcept {
        double lenA = std::hypot(b.x - c.x, b.y - c.y);  // сторона BC, противоположная A
        double lenB = std::hypot(c.x - a.x, c.y - a.y);  // сторона CA, противоположная B
        double lenC = std::hypot(a.x - b.x, a.y - b.y);  // сторона AB, противоположная C

        double sum = lenA + lenB + lenC;
        if (sum == 0.0)
            return Point2D{};

        return Point2D{(lenA * a.x + lenB * b.x + lenC * c.x) / sum, (lenA * a.y + lenB * b.y + lenC * c.y) / sum};
    }
    [[nodiscard]] BoundingBox BoundBox() const noexcept {
        double min_x = std::min({a.x, b.x, c.x});
        double max_x = std::max({a.x, b.x, c.x});
        double min_y = std::min({a.y, b.y, c.y});
        double max_y = std::max({a.y, b.y, c.y});

        return BoundingBox{min_x, min_y, max_x, max_y};
    }
    [[nodiscard]] std::vector<Point2D> Vertices() const { return {a, b, c}; }
    Lines2D<4> Lines() const noexcept {
        Lines2D<4> out;
        out.x = {a.x, b.x, c.x, a.x};
        out.y = {a.y, b.y, c.y, a.y};
        return out;
    }
};

struct Rectangle {
    Point2D bottom_left;
    double width, height;

    [[nodiscard]] double Area() const noexcept { return width * height; }
    [[nodiscard]] double Height() const noexcept { return height; }
    [[nodiscard]] Point2D Center() const noexcept {
        return {bottom_left.x + width * 0.5, bottom_left.y + height * 0.5};
    }
    [[nodiscard]] BoundingBox BoundBox() const noexcept {
        return {
            bottom_left.x,          // min_x
            bottom_left.y,          // min_y
            bottom_left.x + width,  // max_x
            bottom_left.y + height  // max_y
        };
    }
    [[nodiscard]] std::vector<Point2D> Vertices() const {
        Point2D bl = bottom_left;
        Point2D br = {bottom_left.x + width, bottom_left.y};
        Point2D tr = {bottom_left.x + width, bottom_left.y + height};
        Point2D tl = {bottom_left.x, bottom_left.y + height};
        return {bl, br, tr, tl};
    }
    [[nodiscard]] Lines2D<5> Lines() const noexcept {
        Lines2D<5> out;

        const double x0 = bottom_left.x;
        const double y0 = bottom_left.y;
        const double x1 = x0 + width;
        const double y1 = y0 + height;

        // порядок: bl, br, tr, tl, bl
        out.x = {x0, x0, x1, x1, x0};
        out.y = {y0, y1, y1, y0, y0};

        return out;
    }
};

struct RegularPolygon {
    Point2D center_p;
    double radius;
    int sides;

    constexpr RegularPolygon(Point2D center, double radius, int sides) noexcept
        : center_p(center), radius(radius), sides(sides) {}

    [[nodiscard]] std::vector<Point2D> Vertices() const {
        std::vector<Point2D> points;
        points.reserve(sides);

        for (int i = 0; i < sides; ++i) {
            const double angle = 2 * std::numbers::pi * i / sides;
            points.emplace_back(center_p.x + radius * std::cos(angle), center_p.y + radius * std::sin(angle));
        }
        return points;
    }

    [[nodiscard]] double Height() const noexcept { return 2.0 * radius; }
    [[nodiscard]] Point2D Center() const noexcept { return center_p; }
    [[nodiscard]] BoundingBox BoundBox() const noexcept {
        return {center_p.x - radius, center_p.y - radius, center_p.x + radius, center_p.y + radius};
    }
    [[nodiscard]] Lines2DDyn Lines(size_t /*unused*/ = 0) const {
        Lines2DDyn out;
        // у нас ровно sides ребер, но чтобы замкнуть контур, возьмём sides+1 точку
        out.Reserve(sides + 1);
        for (int i = 0; i <= sides; ++i) {
            double ang = 2 * std::numbers::pi * double(i) / double(sides);
            out.PushBack(center_p.x + radius * std::cos(ang), center_p.y + radius * std::sin(ang));
        }
        return out;
    }
};

struct Circle {
    Point2D center_p;
    double radius;

    [[nodiscard]] constexpr Circle(Point2D center, double radius) noexcept : center_p(center), radius(radius) {}

    [[nodiscard]] BoundingBox BoundBox() const noexcept {
        return {center_p.x - radius, center_p.y - radius, center_p.x + radius, center_p.y + radius};
    }
    [[nodiscard]] double Height() const noexcept { return center_p.y + radius; }
    [[nodiscard]] Point2D Center() const noexcept { return center_p; }

    [[nodiscard]] std::vector<Point2D> Vertices(size_t N = 30) const {
        std::vector<Point2D> pts;
        pts.reserve(N);
        for (size_t i = 0; i < N; ++i) {
            double ang = 2 * std::numbers::pi * double(i) / double(N);
            pts.emplace_back(center_p.x + radius * std::cos(ang), center_p.y + radius * std::sin(ang));
        }
        return pts;
    }
    [[nodiscard]] Lines2DDyn Lines(size_t N = 100) const {
        Lines2DDyn out;
        out.Reserve(N + 1);  // +1, чтобы замкнуть контур
        for (size_t i = 0; i <= N; ++i) {
            double ang = 2 * std::numbers::pi * double(i) / double(N);
            out.PushBack(center_p.x + radius * std::cos(ang), center_p.y + radius * std::sin(ang));
        }
        return out;
    }
};

class Polygon {
public:
    [[nodiscard]] std::vector<Point2D> Vertices() const noexcept { return points_; }
    [[nodiscard]] double Height() const noexcept { return bounding_box_.max_y - bounding_box_.min_y; }
    [[nodiscard]] Point2D Center() const noexcept {
        if (points_.empty())
            return Point2D{};
        double sum_x = 0, sum_y = 0;
        for (const auto &p : points_) {
            sum_x += p.x;
            sum_y += p.y;
        }
        return Point2D{sum_x / points_.size(), sum_y / points_.size()};
    }
    [[nodiscard]] BoundingBox BoundBox() const noexcept { return bounding_box_; }
    [[nodiscard]] Lines2DDyn Lines() const {
        Lines2DDyn lines;
        lines.Reserve(points_.size() + 1);

        for (const auto &p : points_)
            lines.PushBack(p);

        if (!points_.empty())
            lines.PushBack(points_.front());

        return lines;
    }

    template <typename Vec>
    void SetPoints(Vec &&pts) noexcept(std::is_rvalue_reference_v<Vec&&>) {
        points_ = std::forward<Vec>(pts);
        UpdateBoundingBox();
    }

private:
    void UpdateBoundingBox() noexcept {
        if (points_.empty()) {
            bounding_box_ = BoundingBox{};
            return;
        }

        double min_x = points_[0].x, max_x = points_[0].x;
        double min_y = points_[0].y, max_y = points_[0].y;
        for (const auto &p : points_) {
            if (p.x < min_x)
                min_x = p.x;
            if (p.x > max_x)
                max_x = p.x;
            if (p.y < min_y)
                min_y = p.y;
            if (p.y > max_y)
                max_y = p.y;
        }

        bounding_box_ = BoundingBox{min_x, min_y, max_x, max_y};
    }

    std::vector<Point2D> points_;
    BoundingBox bounding_box_;
};

using Shape = std::variant<Line, Triangle, Rectangle, RegularPolygon, Circle, Polygon>;

enum class GeometryError { Unsupported, NoIntersection, InvalidInput, DegenrateCase, InsufficientPoints };

template <typename T>
using GeometryResult = std::expected<T, GeometryError>;

struct DummyClass {
    DummyClass(std::vector<Shape>) {}
};

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

    constexpr auto parse(format_parse_context &ctx) {
        auto it = ctx.begin();
        auto end = ctx.end();

        constexpr std::string_view tag = "new_line";
        if (std::distance(it, end) >= static_cast<ptrdiff_t>(tag.size()) && std::string_view(&*it, tag.size()) == tag) {
            use_new_line = true;
            it += tag.size();
        }
        while (it != end && *it != '}') {
            ++it;
        }
        return it;
    }

    template <typename FormatContext>
    auto format(const std::vector<geometry::Point2D> &v, FormatContext &ctx) const {
        auto out = ctx.out();
        if (!use_new_line) {
            out = std::format_to(out, "[");
            for (size_t i = 0; i < v.size(); ++i) {
                out = std::format_to(out, "{}", v[i]);
                if (i + 1 < v.size())
                    out = std::format_to(out, ", ");
            }
            out = std::format_to(out, "]");
        } else {
            out = std::format_to(out, "[\n");
            for (size_t i = 0; i < v.size(); ++i) {
                out = std::format_to(out, "\t{}\n", v[i]);
            }
            out = std::format_to(out, "]");
        }
        return out;
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
