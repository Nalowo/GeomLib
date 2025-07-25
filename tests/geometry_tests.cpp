#include "details.hpp"
#include "geometry.hpp"
#include <gtest/gtest.h>

using namespace geometry;

TEST(Point2DTest, ConstructorsAndAccess) {
    Point2D p1;
    EXPECT_DOUBLE_EQ(p1.x, 0.0);
    EXPECT_DOUBLE_EQ(p1.y, 0.0);

    Point2D p2{3.5, -2.5};
    EXPECT_DOUBLE_EQ(p2.x, 3.5);
    EXPECT_DOUBLE_EQ(p2.y, -2.5);
}

TEST(Point2DTest, OperatorsAndMath) {
    Point2D a{1, 2}, b{3, 4};
    auto sum = a + b;
    EXPECT_EQ(sum, (Point2D{4, 6}));
    auto diff = b - a;
    EXPECT_EQ(diff, (Point2D{2, 2}));
    auto scaled = a * 2.0;
    EXPECT_EQ(scaled, (Point2D{2, 4}));
    auto divd = b / 2.0;
    EXPECT_EQ(divd, (Point2D{1.5, 2.0}));

    EXPECT_DOUBLE_EQ(a.Dot(b), 11.0);
    EXPECT_DOUBLE_EQ(a.Cross(b), -2.0);
    EXPECT_DOUBLE_EQ(a.Length(), std::hypot(1, 2));
    EXPECT_DOUBLE_EQ(a.DistanceTo(b), std::hypot(2, 2));

    auto norm = Point2D{3, 4}.Normalize();
    EXPECT_NEAR(norm.x, 0.6, details::EPSILON);
    EXPECT_NEAR(norm.y, 0.8, details::EPSILON);

    EXPECT_TRUE((Point2D{0, 0} == Point2D{0, 0}));
    EXPECT_FALSE((Point2D{0, 1} < Point2D{1, 0}));
}

TEST(Lines2DDynTest, ReservePushBackFront) {
    Lines2DDyn lines;
    lines.Reserve(3);
    EXPECT_GE(lines.x.capacity(), 3u);
    EXPECT_GE(lines.y.capacity(), 3u);

    lines.PushBack(Point2D{1, 1});
    lines.PushBack(2.0, 3.0);
    EXPECT_EQ(lines.x.size(), 2u);
    EXPECT_EQ(lines.y.size(), 2u);

    auto front = lines.Front();
    EXPECT_EQ(front, (Point2D{1, 1}));
}

TEST(BoundingBoxTest, Metrics) {
    BoundingBox bb{1, 2, 4, 6};
    EXPECT_DOUBLE_EQ(bb.Width(), 3.0);
    EXPECT_DOUBLE_EQ(bb.Height(), 4.0);
    auto c = bb.Center();
    EXPECT_DOUBLE_EQ(c.x, 2.5);
    EXPECT_DOUBLE_EQ(c.y, 4.0);
}

TEST(BoundingBoxTest, Overlaps) {
    BoundingBox a{0, 0, 2, 2}, b{1, 1, 3, 3}, c{3, 3, 4, 4};
    EXPECT_TRUE(a.Overlaps(b));
    EXPECT_TRUE(b.Overlaps(a));
    EXPECT_FALSE(a.Overlaps(c));
}

class LineTest : public ::testing::Test {
protected:
    Line horiz{{0, 0}, {5, 0}};
    Line vert{{0, 0}, {0, 5}};
    Line diag{{1, 1}, {4, 5}};
};

TEST_F(LineTest, DeltaLength) {
    auto [dx, dy] = horiz.Delta();
    EXPECT_DOUBLE_EQ(dx, 5.0);
    EXPECT_DOUBLE_EQ(dy, 0.0);
    EXPECT_DOUBLE_EQ(horiz.Length(), 5.0);
}

TEST_F(LineTest, Direction) {
    auto d = vert.Direction(true);
    EXPECT_NEAR(d.first, 0.0, details::EPSILON);
    EXPECT_NEAR(d.second, 1.0, details::EPSILON);
}

TEST_F(LineTest, BoundBoxCenter) {
    auto bb = diag.BoundBox();
    EXPECT_DOUBLE_EQ(bb.min_x, 1.0);
    EXPECT_DOUBLE_EQ(bb.max_y, 5.0);
    auto c = diag.Center();
    EXPECT_DOUBLE_EQ(c.x, 2.5);
    EXPECT_DOUBLE_EQ(c.y, 3.0);
}

TEST_F(LineTest, VerticesLines) {
    auto verts = horiz.Vertices();
    ASSERT_EQ(verts.size(), 2u);
    EXPECT_TRUE((verts[0] == Point2D{0, 0}));
    auto l2 = horiz.Lines();
    EXPECT_EQ(l2.x[1], 5);
    EXPECT_EQ(l2.y[1], 0);
}

class TriangleTest : public ::testing::Test {
protected:
    Triangle t{{0, 0}, {3, 0}, {0, 4}};
};

TEST_F(TriangleTest, AreaHeightCenter) {
    EXPECT_DOUBLE_EQ(t.Area(), 6.0);
    EXPECT_DOUBLE_EQ(t.Height(), 4.0);
    auto c = t.Center();
    EXPECT_DOUBLE_EQ(c.x, (std::hypot(3, 4) * 0 + std::hypot(4, 0) * 3 + std::hypot(0, 3) * 0) /
                              (std::hypot(3, 4) + std::hypot(4, 0) + std::hypot(0, 3)));
}

TEST_F(TriangleTest, BoundBoxVerticesLines) {
    auto bb = t.BoundBox();
    EXPECT_EQ(bb.min_x, 0);
    auto v = t.Vertices();
    EXPECT_EQ(v.size(), 3u);
    auto l2 = t.Lines();
    EXPECT_EQ(l2.x.size(), 4u);
}

class RectangleTest : public ::testing::Test {
protected:
    Rectangle r{{1, 2}, 3, 4};
};

TEST_F(RectangleTest, AreaHeightCenterBoundBox) {
    EXPECT_DOUBLE_EQ(r.Area(), 12.0);
    EXPECT_DOUBLE_EQ(r.Height(), 4.0);
    auto c = r.Center();
    EXPECT_DOUBLE_EQ(c.x, 2.5);
    EXPECT_DOUBLE_EQ(c.y, 4.0);
    auto bb = r.BoundBox();
    EXPECT_DOUBLE_EQ(bb.max_x, 4.0);
}

TEST_F(RectangleTest, VerticesLines) {
    auto v = r.Vertices();
    EXPECT_EQ(v.front(), (Point2D{1, 2}));
    auto l = r.Lines();
    EXPECT_EQ(l.x.size(), 5u);
}

class RegularPolygonTest : public ::testing::Test {
protected:
    RegularPolygon p{{0, 0}, 1.0, 4};
};

TEST_F(RegularPolygonTest, VerticesCountHeightBoundBox) {
    auto v = p.Vertices();
    EXPECT_EQ(v.size(), 4u);
    EXPECT_DOUBLE_EQ(p.Height(), 2.0);
    auto bb = p.BoundBox();
    EXPECT_DOUBLE_EQ(bb.min_x, -1.0);
}

class CircleTest : public ::testing::Test {
protected:
    Circle c{{1, 1}, 2.0};
};

TEST_F(CircleTest, VerticesLinesHeightBoundBox) {
    auto v = c.Vertices(10);
    EXPECT_EQ(v.size(), 10u);
    auto l = c.Lines(20);
    EXPECT_EQ(l.x.size(), 21u);
    EXPECT_DOUBLE_EQ(c.Height(), 2.0 + 1.0);
    auto bb = c.BoundBox();
    EXPECT_DOUBLE_EQ(bb.min_y, -1.0);
}

class PolygonTest : public ::testing::Test {
protected:
    Polygon poly;
    void SetUp() override {
        std::vector<Point2D> pts{{0, 0}, {2, 0}, {2, 2}, {0, 2}};
        poly.SetPoints(pts);
    }
};

TEST_F(PolygonTest, VerticesHeightCenterBoundBox) {
    auto v = poly.Vertices();
    EXPECT_EQ(v.size(), 4u);
    EXPECT_DOUBLE_EQ(poly.Height(), 2.0);
    auto c = poly.Center();
    EXPECT_DOUBLE_EQ(c.x, 1.0);
    auto bb = poly.BoundBox();
    EXPECT_DOUBLE_EQ(bb.max_x, 2.0);
}

TEST_F(PolygonTest, LinesClosure) {
    auto l = poly.Lines();
    EXPECT_EQ(l.x.front(), l.x.back());
    EXPECT_EQ(l.y.front(), l.y.back());
}

TEST(DocumentTest, AddAndIterate) {
    std::vector<Shape> shapes;
    shapes.push_back(Line{{0, 0}, {1, 1}});
    shapes.push_back(Circle{{0, 0}, 1});
    Document doc(std::move(shapes));

    auto it = doc.begin();
    EXPECT_EQ(doc.GetIndex(*it).value(), 0u);
    ++it;
    EXPECT_EQ(doc.GetIndex(*it).value(), 1u);

    size_t idx = 0;
    for (auto const &s : doc) {
        EXPECT_EQ(doc.GetIndex(s).value(), idx++);
    }
    EXPECT_EQ(idx, 2u);
}