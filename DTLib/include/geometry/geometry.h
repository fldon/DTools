#ifndef GEOMETRY_H
#define GEOMETRY_H
#include <DTools/debug.h>
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

/*!
 * \brief Triangle class with three points that are always ordered COUNTER-CLOCKWISE (assuming right-handed coordinate system)
 */
class Triangle: public iGeometryObject
{
public:
    constexpr Triangle(const Point3 &point1, const Point3 &point2, const Point3 &point3, bool flip_from_right_handed = false);

    [[nodiscard]] std::vector< double > intersect_ray(const Vector3d &origin,
                                                    const Vector3d &direction) const override;

    [[nodiscard]] Vector3d get_surface_normal_at(const Point3 &surface_point) const override;

private:
    [[nodiscard]] std::optional<Point3> get_vector_intersection(const Vector3d &vec) const;

    Point3 m_p1, m_p2, m_p3;
};

constexpr Triangle::Triangle(const Point3 &point1, const Point3 &point2, const Point3 &point3, bool flip_from_right_handed)
    :m_p1(point1), m_p2(point2), m_p3(point3)
{
    using std::swap;
    //TODO: also check that the three points actually form a plane, otherwise throw exception

    //if transforming a right handed coord system triangle into this left handed one, flip the x-coords
    if(flip_from_right_handed)
    {
        m_p1(0) = -m_p1(0);
        m_p2(0) = -m_p2(0);
        m_p3(0) = -m_p3(0);
    }

    //make sure that the three points are ordered counter-clockwise in respect to the z axis
    Vector3d vec_AB = m_p2 - m_p1;
    Vector3d vec_AC = m_p3 - m_p1;

    //if normal has negative z, flip points 2 and 3
    if(vec_AB.cross(vec_AC)(2) < 0)
    {
        swap(m_p2, m_p3);
    }
    debug_assert((m_p2 - m_p1).cross(m_p3 - m_p1)(2) >= 0);
}

} //NS_geometry


} //NS_dtools
#endif // GEOMETRY_H
