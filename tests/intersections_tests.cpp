#include "details.hpp"
#include "geometry.hpp"
#include "intersections.hpp"
#include <gtest/gtest.h>

using namespace geometry;
using namespace geometry::intersections;

class LineLineIntersection : public ::testing::Test {
protected:
    Line h{{0, 0}, {4, 0}};       // горизонтальный
    Line v{{2, -2}, {2, 2}};      // вертикальный
    Line d1{{1, 1}, {3, 3}};      // диагональ вверх
    Line d2{{1, 3}, {3, 1}};      // диагональ вниз
    Line p{{0, 1}, {4, 1}};       // параллельный
};

TEST_F(LineLineIntersection, Perpendicular) {
    auto r = GetIntersectPoint(h, v);
    ASSERT_TRUE(r.has_value());
    EXPECT_DOUBLE_EQ(r->x, 2.0);
    EXPECT_DOUBLE_EQ(r->y, 0.0);
}

TEST_F(LineLineIntersection, Diagonals) {
    auto r = GetIntersectPoint(d1, d2);
    ASSERT_TRUE(r.has_value());
    EXPECT_DOUBLE_EQ(r->x, 2.0);
    EXPECT_DOUBLE_EQ(r->y, 2.0);
}

TEST_F(LineLineIntersection, ParallelNoIntersect) {
    auto r = GetIntersectPoint(h, p);
    EXPECT_FALSE(r.has_value());
}

TEST_F(LineLineIntersection, BorderTouch) {
    Line e{{2, 0}, {2, 2}};
    auto r = GetIntersectPoint(h, e);
    ASSERT_TRUE(r.has_value());
    EXPECT_DOUBLE_EQ(r->x, 2.0);
    EXPECT_DOUBLE_EQ(r->y, 0.0);
}

TEST_F(LineLineIntersection, OutsideSegments) {
    Line far{{5, 5}, {6, 6}};
    auto r = GetIntersectPoint(h, far);
    EXPECT_FALSE(r.has_value());
}

TEST_F(LineLineIntersection, CollinearOverlap) {
    Line sub{{1, 0}, {3, 0}};
    auto r = GetIntersectPoint(h, sub);
    EXPECT_FALSE(r.has_value());
}

class LineCircleIntersection : public ::testing::Test {
protected:
    Circle c{{0,0}, 1.0};
    Line h{{-2,0}, {2,0}};
    Line t{{1,-1}, {1,1}};      // касательная
    Line o{{3,0}, {4,0}};       // вне
    Line ch{{-0.5,0}, {0.5,0}}; // хорда внутри
    Line d{{-1,-1}, {1,1}};     // диагональ
};

TEST_F(LineCircleIntersection, HorizontalTwoIntersections_YieldsFirst) {
    auto r = GetIntersectPoint(h, c);
    ASSERT_TRUE(r.has_value());
    EXPECT_DOUBLE_EQ(r->x, -1.0);
    EXPECT_DOUBLE_EQ(r->y,  0.0);
}

TEST_F(LineCircleIntersection, TangentOnePoint) {
    auto r = GetIntersectPoint(t, c);
    ASSERT_TRUE(r.has_value());
    EXPECT_DOUBLE_EQ(r->x, 1.0);
    EXPECT_DOUBLE_EQ(r->y, 0.0);
}

TEST_F(LineCircleIntersection, NoIntersection) {
    EXPECT_FALSE(GetIntersectPoint(o, c).has_value());
}

TEST_F(LineCircleIntersection, ChordInside_NoIntersection) {
    EXPECT_FALSE(GetIntersectPoint(ch, c).has_value());
}

class CircleCircleIntersection : public ::testing::Test {
protected:
    Circle c0{{0,0}, 2.0};
    Circle c1{{3,0}, 2.0};   // пересекаются двухточечно
    Circle c2{{4,0}, 2.0};   // касаются извне
    Circle c3{{0,0}, 1.0};   // внутреннее касание
    Circle c4{{5,0}, 1.0};   // раздельные
    Circle c5{{0,0}, 2.0};   // совпадающие центры
};

TEST_F(CircleCircleIntersection, TwoPoints_YieldsFirst) {
    auto r = GetIntersectPoint(c0, c1);
    ASSERT_TRUE(r.has_value());
    EXPECT_NEAR(r->x, 1.5, 1e-6);
    EXPECT_NEAR(r->y,  std::sqrt(4 - 1.5*1.5), 1e-6);
}

TEST_F(CircleCircleIntersection, ExternalTangent) {
    auto r = GetIntersectPoint(c0, c2);
    ASSERT_TRUE(r.has_value());
    EXPECT_NEAR(r->x, 2.0, 1e-6);
    EXPECT_NEAR(r->y, 0.0, 1e-6);
}

TEST_F(CircleCircleIntersection, NoIntersection) {
    EXPECT_FALSE(GetIntersectPoint(c0, c4).has_value());
}

TEST_F(CircleCircleIntersection, Concentric_NoIntersection) {
    EXPECT_FALSE(GetIntersectPoint(c0, c5).has_value());
}

TEST(IntersectionUnsupported, PolygonCircle) {
    Polygon poly({{0,0},{1,0},{0,1}});
    Circle cir({0,0},1.0);
    EXPECT_THROW(GetIntersectPoint(poly, cir), std::logic_error);
}