#include "convex_hull.hpp"
#include "geometry.hpp"
#include "intersections.hpp"
#include "queries.hpp"
#include "shape_utils.hpp"
#include "triangulation.hpp"
#include "visualization.hpp"

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
        return std::visit(Multilambda{// true для Line&Line, Line&Circle, Circle&Line, Circle&Circle
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
        auto lhsShapeIndex = others.GetIndex(shape).or_else([]() -> std::optional<size_t>
        {
            throw std::logic_error("Lhs shape index was`t found");
        });
        for (const auto& rShape : others.GetShapeContainer() | views::filter(supported)) {
            auto rhsShapeIndex = others.GetIndex(rShape);
            if (!rhsShapeIndex)
            {
                std::println("Rhs shape index was`t found");
                continue;
            }
            if (*lhsShapeIndex == *rhsShapeIndex)
                continue;
            geometry::intersections::GetIntersectPoint(shape, rShape).transform([&](Point2D p) 
            {
                std::println("Intersection at {} between {} and {}", p, *lhsShapeIndex, *rhsShapeIndex);
                return p;
            }).or_else([&](GeometryError err) -> std::expected<Point2D, GeometryError> 
            {
                std::println("Shape {} and {} have: {}", *lhsShapeIndex, *rhsShapeIndex, err);
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
}

void PerformShapeAnalysis(DummyClass shapes) {
    std::println("\n=== Shape Analysis ===");

    /*
     * Используйте ranges и созданные классы чтобы:
     *     - Найти все пересечения между фигурами используя метод Bounding Box
     *     - Найти самую высокую фигуру (чья высота наибольшая)
     *     - Вывести расстояние между любыми двумя фигурами, которые поддерживают данную функциональность
     */
}

void PerformExtraShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\n=== Shape Extra Analysis ===");

    /*
     * Используйте ranges и созданные классы чтобы:
     *     - Вывести 3 любые фигуры, которые находятся выше 50.0
     *     - Вывести фигуры с наименьшей и с наибольшей высотами
     */
}

int main() {
    utils::ShapeGenerator generator(-50.0, 50.0, 5.0, 25.0);
    std::vector<Shape> shapes = generator.GenerateShapes(15);
    
    std::println("Generated {} random shapes", shapes.size());

    // Выведите индекс каждой фигуры и её высоту
    for (const auto &[index, shape] : views::enumerate(shapes))
    {
        auto height = std::visit(Multilambda([](const auto& shape)
        {
            return shape.Height();
        }), shape);
        std::println("Shape index - {}, height: {}", index, std::abs(height));
    }

    Document doc(std::move(shapes));
    //
    // Вызываем разработанные функции
    //
    PrintAllIntersections(doc.GetShape(0), doc);

    PrintDistancesFromPointToShapes(Point2D{10.0, 10.0}, doc);

    PerformShapeAnalysis(shapes);

    PerformExtraShapeAnalysis(shapes);

    //
    // Рисуем все фигуры
    //
    // Важно: после изучения графика - нажмите Enter чтобы продолжить выполнение и построить 2ой график
    //
    geometry::visualization::Draw(doc.GetShapeContainer());

    //
    // Формируем список из вершин всех фигур
    //
    std::vector<Point2D> points;

    /* ваш код здесь */

    //
    // Находим список точек, для построения выпуклой оболочки - convex hull - алгоритмом Грэхема
    // Создаём из них объект класса `Polygon` и добавляем его в список shapes
    // Рисуем все фигуры
    //

    /* ваш код здесь */

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
    }
    return 0;
}