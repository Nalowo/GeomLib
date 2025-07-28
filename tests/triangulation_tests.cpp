#include <gtest/gtest.h>
#include "geometry.hpp"
#include "triangulation.hpp"

using namespace geometry;
using namespace geometry::triangulation;

TEST(DelaunayTriangleTest, Circumcircle) {
    DelaunayTriangle t{{0,0},{1,0},{0,1}};
    auto center = t.Circumcenter();
    EXPECT_NEAR(center.x, 0.5, details::EPSILON);
    EXPECT_NEAR(center.y, 0.5, details::EPSILON);
    EXPECT_DOUBLE_EQ(t.Circumradius(), std::hypot(0.5,0.5));
}

TEST(DelaunayTriangleTest, ContainsPoint) {
    DelaunayTriangle t{{0,0},{2,0},{0,2}};
    EXPECT_TRUE(t.ContainsPoint({1,1}));
    EXPECT_FALSE(t.ContainsPoint({3,3}));
}

TEST(EdgeTest, OrderingAndEquality) {
    Edge e1{{0,0},{1,1}}, e2{{1,1},{0,0}};
    EXPECT_EQ(e1, e2);
    Edge a{{0,0},{2,0}}, b{{0,1},{1,1}};
    EXPECT_TRUE(a < b || b < a);
}

TEST(DelaunayTriangulationTest, MinimalPoints) {
    std::vector<Point2D> pts{{0,0},{1,0},{0,1}};
    auto r = DelaunayTriangulation(pts);
    ASSERT_TRUE(r.has_value());
    auto tris = *r;
    ASSERT_EQ(tris.size(), 1u);
    EXPECT_TRUE(tris[0] == DelaunayTriangle({0,0},{1,0},{0,1}));
}

TEST(DelaunayTriangulationTest, TooFewPoints) {
    std::vector<Point2D> pts{{0,0},{1,1}};
    auto r = DelaunayTriangulation(pts);
    EXPECT_FALSE(r.has_value());
    EXPECT_EQ(r.error(), GeometryError::InsufficientPoints);
}

TEST(DelaunayTriangulationTest, SquareSplit) {
    std::vector<Point2D> pts{{0,0},{1,0},{1,1},{0,1}};
    auto r = DelaunayTriangulation(pts);
    ASSERT_TRUE(r.has_value());
    auto tris = *r;
    EXPECT_EQ(tris.size(), 2u);
    std::set<std::pair<double,double>> verts;
    for (auto& t : tris)
        for (auto& p : t.vertices())
            verts.emplace(p.x,p.y);
    EXPECT_EQ(verts.size(), 4u);
    EXPECT_TRUE(verts.count({0,0}));
    EXPECT_TRUE(verts.count({1,0}));
    EXPECT_TRUE(verts.count({1,1}));
    EXPECT_TRUE(verts.count({0,1}));
}