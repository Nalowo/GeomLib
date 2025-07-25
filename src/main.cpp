#include "convex_hull.hpp"
#include "geometry.hpp"
#include "intersections.hpp"
#include "queries.hpp"
#include "shape_utils.hpp"
#include "triangulation.hpp"
#include "visualization.hpp"
#include "details.hpp"

#include <algorithm>
#include <print>
#include <ranges>

using namespace geometry;

namespace rng = std::ranges;
namespace views = std::ranges::views;

void PrintAllIntersections(const Shape &shape, const geometry::Document& others) {
    std::println("\n=== Intersections ===");

    /*
     * Используйте ranges чтобы оставить только фигуры,
     * поддерживающие возможность находить пересечения между собой
     *
     * Затем примените монадический интерфейс для обработки результатов:
     *     - Пересечение найдено в точке A между фигурами B и C
     *     - Фигуры B и C не пересекаются
     */

    auto supported = [&shape](const auto& rhs) {
        return std::visit(details::Multilambda{// true для Line&Line, Line&Circle, Circle&Line, Circle&Circle
                                     [](const Line &, const Line &) { return true; },
                                     [](const Line &, const Circle &) { return true; },
                                     [](const Circle &, const Line &) { return true; },
                                     [](const Circle &, const Circle &) { return true; },
                                     // все прочие — false
                                     [](auto &&, auto &&) { return false; }},
                          shape, rhs);
    };

    if (!std::holds_alternative<geometry::Circle>(shape) 
        && !std::holds_alternative<geometry::Line>(shape))
    {
        std::println("Lhs shape type unsupported for search intersections");
        return;
    }

    try
    {
        auto lhsShapeIndex = others.GetIndex(shape).value();
        for (const auto& rShape : others.GetShapeContainer() | views::filter(supported)) {
            auto rhsShapeIndex = others.GetIndex(rShape).value();
            if (lhsShapeIndex == rhsShapeIndex)
                continue;
            geometry::intersections::GetIntersectPoint(shape, rShape).transform([&](Point2D p) 
            {
                std::println("Intersection at {} between {} and {}", p, lhsShapeIndex, rhsShapeIndex);
                return p;
            }).or_else([&](GeometryError err) -> std::expected<Point2D, GeometryError> 
            {
                std::println("Shape {} and {} have: {}", lhsShapeIndex, rhsShapeIndex, err);
                return std::unexpected(err);
            });
        }
    }
    catch (const std::exception& e)
    {
        std::println("Errore: {}", e.what());
    }
}

void PrintDistancesFromPointToShapes(Point2D p, const geometry::Document& shapes) {
    std::println("\n=== Distance from Point Test ===");
    std::println("Testing point: {} ", p);

    /*
     * Используйте ranges чтобы выбрать любые 5 фигур из списка.
     * Затем найдите расстояния от заданной точки до всех выбранных фигур.
     * Выведите результат в формате "Расстояние от точки P до фигуры S равно D"
     */
    
    try
    {
        auto sv = shapes.GetShapeContainer() | views::take(std::min(shapes.size(), size_t(5)));
        for (const auto& shape : sv)
        {
            auto shapeIndex = shapes.GetIndex(shape);
            if (!shapeIndex)
            {
                std::println("Rhs shape index not found");
                continue;
            }

            std::println("Distance from {} to shape {} - {:.2f}", p, *shapeIndex, 
            geometry::queries::PointToShapeDistanceVisitor{}(p, shape));
        }
    }
    catch (const std::exception& e)
    {
        std::println("Errore: {}", e.what());
    }
}

void PerformShapeAnalysis(const geometry::Document& shapes) {
    std::println("\n=== Shape Analysis ===");

    /*
     * Используйте ranges и созданные классы чтобы:
     *     - Найти все пересечения между фигурами используя метод Bounding Box
     *     - Найти самую высокую фигуру (чья высота наибольшая)
     *     - Вывести расстояние между любыми двумя фигурами, которые поддерживают данную функциональность
     */

    try
    {
        auto collisions = geometry::utils::FindAllCollisions(shapes);
        rng::for_each(collisions, [&shapes](const auto& pair)
        {
            const auto& [l, r] = pair;
            auto lIndex = shapes.GetIndex(l.get()).value();
            auto rIndex = shapes.GetIndex(r.get()).value();
            if (lIndex && rIndex)
            {
                std::println("Collision between: {} {}", lIndex, rIndex);
            }
        });
    }
    catch (const std::exception& e)
    {
        std::println("Errore: {}", e.what());
    }

    std::println("Max heigth is {:.2f}", geometry::utils::FindHighestShape(shapes).value());

    try
    {
        auto indexed = shapes | std::views::enumerate;
        auto resultView = views::cartesian_product(indexed, indexed)
            | std::views::filter([](auto const& tup)
            {
                auto const& [l, r] = tup;
                return std::get<0>(l) < std::get<0>(r);
            })
            | views::transform([](auto const &tup) {
                const auto& [l, r] = tup;
                auto const &a = std::get<1>(l);
                auto const &b = std::get<1>(r);
                return std::tuple{
                    queries::DistanceBetweenShapes(a,b),
                    std::cref(a),
                    std::cref(b)
                };
            })
            | views::filter([](auto const &t) {
                return std::get<0>(t).has_value();
            }) | views::take(1);; 
        for (auto const & [dopt, a, b] : resultView) {
            auto lIndx = shapes.GetIndex(a).value();
            auto rIndx = shapes.GetIndex(b).value();
            std::println("First distance between {} and {} is {:.2f}", lIndx, rIndx, dopt.value());
        }
    }
    catch (const std::exception& e)
    {
        std::println("Errore: {}", e.what());
    }
}

void PerformExtraShapeAnalysis(const geometry::Document& shapes) {
    std::println("\n=== Shape Extra Analysis ===");
    /*
     * Используйте ranges и созданные классы чтобы:
     *     - Вывести 3 любые фигуры, которые находятся выше 50.0
     *     - Вывести фигуры с наименьшей и с наибольшей высотами
     */

    constexpr double MaxHeight = 50.0;
    auto getHeight = [](auto &obj) { return std::visit([](auto &shape) { return shape.Height(); }, obj); };
    auto res = shapes | views::filter([&](auto &shape) 
    { 
        return getHeight(shape) > MaxHeight; 
    }) | views::take(3);

    rng::for_each(res, [&](const auto &obj) 
    {
        std::println("{} is above {:.2f}", shapes.GetIndex(obj).value(), MaxHeight);
    });

    auto less = [getHeight](const auto& lhs, const auto& rhs)
    {
        return getHeight(lhs) < getHeight(rhs);
    };

    try
    {
        auto max_it = std::ranges::max_element(shapes, less);
        auto min_it = std::ranges::min_element(shapes, less);
        std::println("Min height - {} = {:.2f}, Max height - {} = {:.2f}",
            shapes.GetIndex(*min_it).value(), getHeight(*min_it),
            shapes.GetIndex(*max_it).value(), getHeight(*max_it));
    }
    catch(const std::exception& e)
    {
        std::println("Errore: {}", e.what());
    }
}

int main() {
    utils::ShapeGenerator generator(-50.0, 50.0, 5.0, 25.0);
    std::vector<Shape> shapes = generator.GenerateShapes(15);
    
    std::println("Generated {} random shapes", shapes.size());

    // Выведите индекс каждой фигуры и её высоту
    for (const auto &[index, shape] : views::enumerate(shapes))
    {
        auto height = std::visit(details::Multilambda([](const auto& shape)
        {
            return shape.Height();
        }), shape);
        std::println("Shape index - {}, height: {:.2f}", index, std::abs(height));
    }

    Document doc(std::move(shapes));
    //
    // Вызываем разработанные функции
    //
    PrintAllIntersections(doc.GetShape(0), doc);

    PrintDistancesFromPointToShapes(Point2D{10.0, 10.0}, doc);

    PerformShapeAnalysis(doc);

    PerformExtraShapeAnalysis(doc);

    //
    // Рисуем все фигуры
    //
    // Важно: после изучения графика - нажмите Enter чтобы продолжить выполнение и построить 2ой график
    //
    geometry::visualization::Draw(doc);

    //
    // Формируем список из вершин всех фигур
    //
    std::vector<Point2D> points;
    for (const auto& shape : doc) {
        std::visit([&points](const auto& s) {
            auto verts = s.Vertices();
            points.insert(points.end(), verts.begin(), verts.end());
        }, shape);
    }

    //
    // Находим список точек, для построения выпуклой оболочки - convex hull - алгоритмом Грэхема
    // Создаём из них объект класса `Polygon` и добавляем его в список shapes
    // Рисуем все фигуры
    //

    // Вычисление выпуклой оболочки
    if (auto hullResult = geometry::convex_hull::GrahamScan(std::move(points))) {
        doc.AddShape(geometry::Polygon(std::move(*hullResult)));
        geometry::visualization::Draw(doc);
    } else {
        std::println("Failed to compute convex hull: {}", hullResult.error());
    }

    //
    // после изучения графика - нажмите Enter чтобы продолжить выполнение и построить 3ий график
    //

    {
        std::vector<Point2D> points = {{0, 0}, {10, 0}, {5, 8}, {15, 5}, {2, 12}};

        //
        // Используйте список точек points или свой, чтобы
        // выполнить алгоритм триангуляции Делоне алгоритмом Боуэра-Ватсона
        //
        // После успешного завершения алгоритма - выведите результат для проверки
        // используя geometry::visualization::Draw
        //

        if (auto tri_result = geometry::triangulation::DelaunayTriangulation(points)) {
            geometry::visualization::Draw(*tri_result);
        } else {
            std::println("Failed to compute Dela delaunayTriangulationunay triangulation: {}", tri_result.error());
        }
    }
    return 0;
}