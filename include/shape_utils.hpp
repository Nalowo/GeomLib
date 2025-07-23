#pragma once
#include "geometry.hpp"
#include "queries.hpp"
#include <print>
#include <random>
#include <ranges>
#include <utility>
#include <vector>

namespace geometry::utils {

class ShapeGenerator {
public:
    ShapeGenerator(double min_coord = -100.0, double max_coord = 100.0, double min_size = 1.0, double max_size = 20.0)
        // : gen(std::random_device{}()) , coord_dist(min_coord, max_coord), size_dist(min_size, max_size), sides_dist(3, 12), type_dist(0, 4) {
        : gen(20), coord_dist(min_coord, max_coord), size_dist(min_size, max_size), sides_dist(3, 12), type_dist(0, 4) {
    }

    Shape GenerateRandomShape() {
        Point2D center{coord_dist(gen), coord_dist(gen)};
        double size = size_dist(gen);

        switch (type_dist(gen)) {
        case 0: {
            Point2D end{center.x + size, center.y + size};
            return Line{center, end};
        }
        case 1: {
            Point2D a{center.x, center.y};
            Point2D b{center.x + size, center.y};
            Point2D c{center.x + size / 2, center.y + size};
            return Triangle{a, b, c};
        }
        case 2: {
            return Rectangle{center, size, size * 0.8};
        }
        case 3: {
            int sides = sides_dist(gen);
            return RegularPolygon{center, size, sides};
        }
        case 4: {
            return Circle{center, size};
        }
        }
        return Circle{center, size};
    }

    std::vector<Shape> GenerateShapes(size_t count) {
        std::vector<Shape> shapes;
        shapes.reserve(count);

        for (auto _ : std::views::iota(0u, count)) {
            shapes.push_back(GenerateRandomShape());
        }

        return shapes;
    }

private:
    std::mt19937 gen;
    std::uniform_real_distribution<double> coord_dist;
    std::uniform_real_distribution<double> size_dist;
    std::uniform_int_distribution<int> sides_dist;
    std::uniform_int_distribution<int> type_dist;
};

auto FindAllCollisions(std::span<const Shape> shapes) {
    using ShapeRef     = std::reference_wrapper<const geometry::Shape>;
    using RetType      = std::vector<std::pair<ShapeRef, ShapeRef>>;
    /*
     * Используйте библиотеку ranges, чтобы найти все коллизии между фигурами
     *
     * Важно: использование ручной итерации по фигурам не разрешается
     *
     * Также используйте наиболее эффективный метод добавления объектов в collisions
     */
    auto indexed = shapes | std::views::enumerate;
    auto collisionsView = std::views::cartesian_product(indexed, indexed) 
    | std::views::filter([](auto const& tup){
            // только i < j, чтобы не дублировать и не сравнивать с собой
            auto const& [l, r] = tup;
            return std::get<0>(l) < std::get<0>(r);
    }) | std::views::filter([](auto const& tup){
            const auto& [l, r] = tup;
            return queries::BoundingBoxesOverlap(std::get<1>(l), std::get<1>(r));
    }) | std::views::transform([](auto const& tup){
            auto const& [l, r] = tup;
            const geometry::Shape& s1 = std::get<1>(l);
            const geometry::Shape& s2 = std::get<1>(r);
            return std::pair<ShapeRef,ShapeRef>{
                std::cref(s1), std::cref(s2)
            };
    });

    return collisionsView | std::ranges::to<RetType>();
}

std::optional<double> FindHighestShape(std::span<const Shape> shapes) {

    /*
     * Используйте библиотеку ranges, чтобы найти самую высокую фигуру
     *
     * Важно: использование ручной итерации по фигурам не разрешается
     */

    auto maxElem = std::ranges::max_element(shapes, {}, geometry::queries::GetHeight);
    if (maxElem == shapes.end()) {
        return std::nullopt;
    }
    return geometry::queries::GetHeight(*maxElem);
}

}  // namespace geometry::utils