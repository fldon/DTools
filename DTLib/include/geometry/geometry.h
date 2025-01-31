#ifndef GEOMETRY_H
#define GEOMETRY_H
#include <eigen3/Eigen/Eigen>

namespace NS_dtools
{

namespace NS_geometry
{

using Eigen::Matrix3i;
using Eigen::Vector3d;
using Eigen::Matrix2i;
using Eigen::Vector2i;

using Point2 = Eigen::Vector2i;
using Point3 = Eigen::Vector3d;

constexpr double EPSILON = 0.0001; //std::numeric_limits<double>::epsilon();

constexpr double EPSILON_T_MIN = EPSILON; //Minimal tiny t so that we do not cast shadows from our origin point itself


class iGeometryObject
{
public:
    [[nodiscard]] virtual std::vector<double> intersect_ray(const Vector3d &origin,
                                                    const Vector3d &direction) const = 0;

    [[nodiscard]] virtual Vector3d get_surface_normal_at(const Point3 &surface_point) const = 0;

};


class Plane : public iGeometryObject
{
public:
    Plane(const Vector3d &vec_1, const Vector3d &vec_2);
    [[nodiscard]] virtual std::vector<double> intersect_ray(const Vector3d &origin,
                                                            const Vector3d &direction) const override;

    [[nodiscard]] virtual Vector3d get_surface_normal_at(const Point3 &surface_point) const override;

    [[nodiscard]] bool get_is_point_in_plane(const Point3 &p, double epsilon = EPSILON) const;
    [[nodiscard]] std::optional<Point3> get_vector_intersection(const Vector3d &vec) const;
private:
    Vector3d m_vec1, m_vec2;
};




class Sphere : public iGeometryObject
{
public:
    constexpr Sphere(const Point3 &center, double radius);

    [[nodiscard]] std::vector< double > intersect_ray(const Vector3d &origin,
                                                    const Vector3d &direction) const override;

    [[nodiscard]] Vector3d get_surface_normal_at(const Point3 &surface_point) const override;

private:
    double m_radius;
    Point3 m_center;
};

constexpr Sphere::Sphere(const Point3 &center, double radius)
    :m_radius(radius), m_center(center)
{
}

class Triangle: public iGeometryObject
{
public:
    constexpr Triangle(const Point3 &point1, const Point3 &point2, const Point3 &point3);

    [[nodiscard]] std::vector< double > intersect_ray(const Vector3d &origin,
                                                    const Vector3d &direction) const override;

    [[nodiscard]] Vector3d get_surface_normal_at(const Point3 &surface_point) const override;

private:
    [[nodiscard]] std::optional<Point3> get_vector_intersection(const Vector3d &vec) const;

    Point3 m_p1, m_p2, m_p3;
};

constexpr Triangle::Triangle(const Point3 &point1, const Point3 &point2, const Point3 &point3)
    :m_p1(point1), m_p2(point2), m_p3(point3)
{
}

} //NS_geometry


} //NS_dtools
#endif // GEOMETRY_H
