#include "geometry.hpp"
#include "queries.hpp"
#include <gtest/gtest.h>
#include <cmath>

using namespace geometry;
using namespace geometry::queries;

class DistanceToPointTests : public ::testing::Test {
protected:
    Line line{{0, 0}, {4, 0}};
    Circle circle{{0, 0}, 2.0};
    Triangle triangle{{0, 0}, {4, 0}, {0, 3}};
    Rectangle rect{{1, 1}, 2, 2};
    RegularPolygon hex{{0, 0}, 2.0, 6};
    Polygon poly{{{0,0},{2,0},{2,2},{0,2}}};
};

TEST_F(DistanceToPointTests, LineAbove) {
    EXPECT_DOUBLE_EQ(DistanceToPoint(line, {2, 3}), 3.0);
}

TEST_F(DistanceToPointTests, LineEndpoint) {
    EXPECT_DOUBLE_EQ(DistanceToPoint(line, {5, 4}), std::hypot(1,4));
}

TEST_F(DistanceToPointTests, LineOnSegment) {
    EXPECT_DOUBLE_EQ(DistanceToPoint(line, {2, 0}), 0.0);
}

TEST_F(DistanceToPointTests, CircleOutside) {
    EXPECT_DOUBLE_EQ(DistanceToPoint(circle, {3, 0}), 1.0);
    EXPECT_DOUBLE_EQ(DistanceToPoint(circle, {0, -4}), 2.0);
}

TEST_F(DistanceToPointTests, CircleInside) {
    EXPECT_DOUBLE_EQ(DistanceToPoint(circle, {1, 1}), 0.0);
}

TEST_F(DistanceToPointTests, TriangleInside) {
    EXPECT_DOUBLE_EQ(DistanceToPoint(triangle, {1, 1}), 0.0);
}

TEST_F(DistanceToPointTests, TriangleOutside) {
    EXPECT_DOUBLE_EQ(DistanceToPoint(triangle, {2, -1}), 1.0);
}

TEST_F(DistanceToPointTests, RectangleCorner) {
    EXPECT_DOUBLE_EQ(DistanceToPoint(rect, {0, 0}), std::hypot(1,1));
}

TEST_F(DistanceToPointTests, RectangleEdge) {
    EXPECT_DOUBLE_EQ(DistanceToPoint(rect, {2, 1}), 0.0);
}

TEST_F(DistanceToPointTests, RegularPolygonVertex) {
    EXPECT_DOUBLE_EQ(DistanceToPoint(hex, {2, 0}), 0.0);
}

TEST_F(DistanceToPointTests, RegularPolygonOutside) {
    EXPECT_NEAR(DistanceToPoint(hex, {3, 0}), 1.0, 1e-10);
}

TEST_F(DistanceToPointTests, PolygonInside) {
    EXPECT_DOUBLE_EQ(DistanceToPoint(poly, {1, 1}), 0.0);
}

TEST_F(DistanceToPointTests, PolygonOutside) {
    EXPECT_DOUBLE_EQ(DistanceToPoint(poly, {3, 1}), 1.0);
}

class DistanceBetweenShapesTests : public ::testing::Test {
protected:
    Line line1{{0, 0}, {4, 0}};
    Line line2{{2, 1}, {2, 3}};
    Circle c1{{0, 0}, 1.0};
    Circle c2{{3, 0}, 1.0};
    Rectangle rect{{0, 0}, 2, 2};
};

TEST_F(DistanceBetweenShapesTests, LineLineIntersect) {
    auto d = DistanceBetweenShapes(line1, Line{{1, -1},{1,1}});
    ASSERT_TRUE(d.has_value());
    EXPECT_DOUBLE_EQ(*d, 0);
}

TEST_F(DistanceBetweenShapesTests, LineLineZero) {
    auto d = DistanceBetweenShapes(line1, Line{{0,0},{2,0}});
    ASSERT_TRUE(d.has_value());
    EXPECT_DOUBLE_EQ(*d, 0.0);
}

TEST_F(DistanceBetweenShapesTests, CircleCircleSeparate) {
    auto d = DistanceBetweenShapes(c1, c2);
    ASSERT_TRUE(d.has_value());
    EXPECT_DOUBLE_EQ(*d, 1.0);
}

TEST_F(DistanceBetweenShapesTests, CircleCircleTouchInside) {
    auto d = DistanceBetweenShapes(c1, Circle{{0,0},2.0});
    ASSERT_TRUE(d.has_value());
    EXPECT_DOUBLE_EQ(*d, 0.0);
}

TEST_F(DistanceBetweenShapesTests, UnsupportedLineCircle) {
    auto d = DistanceBetweenShapes(line1, c1);
    EXPECT_FALSE(d.has_value());
}

TEST_F(DistanceBetweenShapesTests, RectanglePoint) {
    Point2D p{3,3};
    auto d = DistanceBetweenShapes(p, rect);
    ASSERT_TRUE(d.has_value());
    EXPECT_DOUBLE_EQ(*d, std::hypot(1,1));
}

TEST_F(DistanceBetweenShapesTests, PointCircle) {
    auto d = DistanceBetweenShapes(Point2D{0,3}, c1);
    ASSERT_TRUE(d.has_value());
    EXPECT_DOUBLE_EQ(*d, 2.0);
}
