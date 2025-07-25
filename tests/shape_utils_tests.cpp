// tests/shape_utils_tests.cpp
#include "geometry.hpp"
#include "shape_utils.hpp"
#include "queries.hpp"
#include <gtest/gtest.h>

using namespace geometry;
using namespace geometry::utils;
using geometry::queries::BoundingBoxesOverlap;

class ShapeUtilsTests : public ::testing::Test {
protected:
    Circle c1{{0.0, 0.0}, 1.0};             // bbox [-1,-1]..[1,1]
    Circle c2{{1.5, 0.0}, 1.0};             // bbox [0.5,-1]..[2.5,1] перекрытие c1
    Circle c3{{5.0, 5.0}, 1.0};             // далеко

    Rectangle r1{{0.0, 0.0}, 2.0, 2.0};     // bbox [0,0]..[2,2]
    Rectangle r2{{1.0, 1.0}, 2.0, 2.0};     // bbox [1,1]..[3,3], перекрытие r1
    Rectangle r3{{10.0,10.0},1.0,1.0};      // далеко

    Line l1{{0.0,0.0},{2.0,2.0}};           // bbox [0,0]..[2,2]
    Line l2{{2.0,0.0},{0.0,2.0}};           // bbox [0,0]..[2,2], перекрытие l1
    Line l3{{10.0,10.0},{11.0,11.0}};       // далеко

    std::vector<Shape> allShapes;
    std::vector<Shape> someShapes;

    void SetUp() override {
        allShapes = { c1, c2, c3, r1, r2, r3, l1, l2, l3 };
        someShapes = { c1, c2, c3 };
    }
};

TEST_F(ShapeUtilsTests, FindAllCollisions_Empty) {
    std::vector<Shape> empty;
    auto col = FindAllCollisions(empty);
    EXPECT_TRUE(col.empty());
}

TEST_F(ShapeUtilsTests, FindAllCollisions_Single) {
    std::vector<Shape> single = { c1 };
    auto col = FindAllCollisions(single);
    EXPECT_TRUE(col.empty());
}

TEST_F(ShapeUtilsTests, FindAllCollisions_TwoOverlap) {
    std::vector<Shape> shapes = { c1, c2 };
    auto col = FindAllCollisions(shapes);
    ASSERT_EQ(col.size(), 1);
    const auto &p = col[0];
    EXPECT_TRUE(BoundingBoxesOverlap(p.first.get(), p.second.get()));
}

TEST_F(ShapeUtilsTests, FindAllCollisions_TwoNonOverlap) {
    std::vector<Shape> shapes = { c1, c3 };
    auto col = FindAllCollisions(shapes);
    EXPECT_TRUE(col.empty());
}

TEST_F(ShapeUtilsTests, FindAllCollisions_MixedShapes) {
    std::vector<Shape> shapes = { c1, r1, l1 };
    auto col = FindAllCollisions(shapes);
    ASSERT_EQ(col.size(), 3);
    for (auto &pr : col) {
        EXPECT_NE(&pr.first.get(), &pr.second.get());
        EXPECT_TRUE(BoundingBoxesOverlap(pr.first.get(), pr.second.get()));
    }
}

TEST_F(ShapeUtilsTests, FindHighestShape_Empty) {
    std::vector<Shape> empty;
    auto h = FindHighestShape(empty);
    EXPECT_FALSE(h.has_value());
}

TEST_F(ShapeUtilsTests, FindHighestShape_Single) {
    std::vector<Shape> single = { r1 };
    auto h = FindHighestShape(single);
    ASSERT_TRUE(h.has_value());
    EXPECT_DOUBLE_EQ(*h, r1.Height());
}

TEST_F(ShapeUtilsTests, FindHighestShape_Multiple) {
    Triangle tri{{-1,0},{1,0},{0,5}};
    std::vector<Shape> shapes = { c1, r1, tri, l1 };
    auto h = FindHighestShape(shapes);
    ASSERT_TRUE(h.has_value());
    EXPECT_DOUBLE_EQ(*h, tri.Height());
}

TEST_F(ShapeUtilsTests, FindHighestShape_Ties) {
    Rectangle rA{{0,0}, 1, 4}, rB{{0,0},2,2};
    std::vector<Shape> shapes = { rA, rB };
    auto h = FindHighestShape(shapes);
    ASSERT_TRUE(h.has_value());
    EXPECT_DOUBLE_EQ(*h, 4.0);
}