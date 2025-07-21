#include "geometry.hpp"
#include "triangulation.hpp"
#include <span>

namespace geometry::visualization {

void Draw(std::span<const geometry::Shape> shapes);

void Draw(std::span<const geometry::triangulation::DelaunayTriangle> triangles);

}  // namespace geometry::visualization
