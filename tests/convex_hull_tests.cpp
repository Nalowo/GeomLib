#include "convex_hull.hpp"
#include "details.hpp"
#include "geometry.hpp"
#include <gtest/gtest.h>

using namespace geometry;
using namespace geometry::convex_hull;

static bool same_set(std::vector<Point2D> a, std::vector<Point2D> b) {
    if (a.size() != b.size())
        return false;
    auto cmp = [](const Point2D &p1, const Point2D &p2) {
        if (p1.x != p2.x)
            return p1.x < p2.x;
        return p1.y < p2.y;
    };
    std::sort(a.begin(), a.end(), cmp);
    std::sort(b.begin(), b.end(), cmp);
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::abs(a[i].x - b[i].x) > details::EPSILON || std::abs(a[i].y - b[i].y) > details::EPSILON)
            return false;
    }
    return true;
}

TEST(CrossProductTest, SimpleTurns) {
    EXPECT_LT(CrossProduct({0, 0}, {1, 0}, {1, 1}), 0);
    EXPECT_GT(CrossProduct({0, 0}, {1, 1}, {2, 0}), 0);
}

TEST(GrahamScanTest, TooFewPoints) {
    std::vector<Point2D> pts{{0, 0}, {1, 1}};
    auto r = GrahamScan(pts);
    EXPECT_FALSE(r.has_value());
    EXPECT_EQ(r.error(), GeometryError::InsufficientPoints);
}

TEST(GrahamScanTest, TriangleHull) {
    std::vector<Point2D> pts{{0, 0}, {1, 0}, {0, 1}};
    auto r = GrahamScan(pts);
    ASSERT_TRUE(r.has_value());
    std::vector<Point2D> exp{{0, 0}, {1, 0}, {0, 1}};
    EXPECT_TRUE(same_set(*r, exp));
}

TEST(GrahamScanTest, SquareWithInnerPoint) {
    std::vector<Point2D> pts{{0, 0}, {2, 0}, {2, 2}, {0, 2}, {1, 1}};
    auto r = GrahamScan(pts);
    ASSERT_TRUE(r.has_value());
    std::vector<Point2D> exp{{0, 0}, {2, 0}, {2, 2}, {0, 2}};
    EXPECT_TRUE(same_set(*r, exp));
}

TEST(StackForGrahamScanTest, PushPopExtract) {
    StackForGrahamScan st;
    st.Reserve(3);
    st.Push({0, 0});
    st.Push({1, 0});
    st.Push({1, 1});
    EXPECT_EQ(st.Size(), 3u);
    st.Pop();
    EXPECT_TRUE((st.Top() == Point2D{1, 0}));
    auto hull = std::move(st).Extract();
    ASSERT_EQ(hull.size(), 2u);
    EXPECT_EQ(hull[0], (Point2D{0, 0}));
    EXPECT_EQ(hull[1], (Point2D{1, 0}));
}