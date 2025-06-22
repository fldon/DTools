#ifndef GEOMETRY_H
#define GEOMETRY_H
#include <DTools/debug.h>
#include <DTools/MiscTools.h>
#include <eigen3/Eigen/Eigen>

namespace NS_dtools
{

namespace NS_geometry
{

using Eigen::Matrix3i;
using Eigen::Vector3d;
using Eigen::Matrix2i;
using Eigen::Vector2i;

//TODO: Point2 should be renamed to Point2i, this is too specific!
using Point2i = Eigen::Vector2i;
using Point3 = Eigen::Vector3d;

constexpr double EPSILON = 0.0001; //std::numeric_limits<double>::epsilon();

constexpr double EPSILON_T_MIN = EPSILON; //Minimal tiny t so that we do not cast shadows from our origin point itself

/*GENERAL FUNCTIONS AND CONVERSIONS*/

/*!
 * \brief returns a vector/point in cartesian coords corresponding to the supplied spherical coords.
 * Assumes that 0 < theta < pi and 0 < phi < 2 pi.
 * Uses convention that "z is up" . Might have to convert the resulting vector if this differs
 * \param radius
 * \param theta
 * \param phi
 */
constexpr Vector3d spherical_to_cartesian(double radius, double theta, double phi)
{
    const double x = radius * sin(theta) * cos(phi);
    const double y = radius * sin(theta) * sin(phi);
    const double z = radius * cos(theta);
    return Vector3d{x,y,z};
}


/*INVIDIDUAL GEOMETRY OBJECTS*/
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
 * \brief Triangle class with three points
 */
class Triangle: public iGeometryObject
{
public:
    constexpr Triangle(const Point3 &point1,
                       const Point3 &point2,
                       const Point3 &point3,
                       bool flip_from_mirror_coord_system = false);

    [[nodiscard]] std::vector< double > intersect_ray(const Vector3d &origin,
                                                    const Vector3d &direction) const override;

    [[nodiscard]] constexpr Vector3d get_surface_normal_at(const Point3 &surface_point) const override;

    [[nodiscard]] constexpr std::optional< std::tuple<double, double, double> >  get_baryctr_coords_at(const Point3 &point) const;

    [[nodiscard]] constexpr bool is_point_in_triangle(const Point3 &point) const;

private:
    [[nodiscard]] std::optional<Point3> get_vector_intersection(const Vector3d &vec) const;

    Point3 m_p1, m_p2, m_p3;
};

constexpr Triangle::Triangle(const Point3 &point1,
                             const Point3 &point2,
                             const Point3 &point3,
                             bool flip_from_mirror_coord_system)
    :m_p1(point1), m_p2(point2), m_p3(point3)
{
    using std::swap;
    //TODO: also check that the three points actually form a plane within Epsilon accuracy, otherwise throw exception

    //if transforming a right handed coord system triangle into this left handed one, flip the x-coords
    if(flip_from_mirror_coord_system)
    {
        m_p1(0) = -m_p1(0);
        m_p2(0) = -m_p2(0);
        m_p3(0) = -m_p3(0);

        //according to scratchapixeL: also re-order the points to p3,p2,p1 instead of p1,p2,p2
        swap(m_p1, m_p3);
    }
}

constexpr Vector3d Triangle::get_surface_normal_at(const Point3&) const
{
    //There is only one surface normal, regardless of where we are - defined by system handedness / winding
    const Vector3d vec_AB = m_p2 - m_p1;
    const Vector3d vec_AC = m_p3 - m_p1;

    const Vector3d surface_normal = vec_AB.cross(vec_AC);

    return surface_normal;
}

/*!
 * \brief function to compute barycentric coordinates of Point p in triangle (checking inside_out test for validity)
 * Returns empty optional if Point p is not in triangle
 * \param point
 * \return
 */
constexpr std::optional< std::tuple<double, double, double> >  Triangle::get_baryctr_coords_at(const Point3 &point) const
{
    const Vector3d surface_normal = get_surface_normal_at(point);
    //perform inside_out tests: if failed, return empty optional

    //P = u*A + v*B + w*C where w = 1-u-v
    //This is opposed to e.g. moeller-trumber definition of P = w*A + u*B + v*C
    //This is important in any calculation where the ordering becomes relevant, e.g. intersections using moeller-trumbore

    //Test u (BCP)
    const Vector3d vec_BP = point - m_p2;
    const Vector3d vec_BC = m_p3 - m_p2;
    const Vector3d BCP_normal = vec_BC.cross(vec_BP);

    double u = surface_normal.dot(BCP_normal);
    if(u < 0 && std::abs(u) > EPSILON)
    {
        return {};
    }

    //Test v (CAP)
    const Vector3d vec_CP = point - m_p3;
    const Vector3d vec_CA = m_p1 - m_p3;
    const Vector3d CAP_normal = vec_CA.cross(vec_CP);

    double v = surface_normal.dot(CAP_normal);
    if(v < 0 && std::abs(v) > EPSILON)
    {
        return {};
    }

    //Test w (ABP)
    const Vector3d vec_AP = point - m_p1;
    const Vector3d vec_AB = m_p2 - m_p1;
    const Vector3d ABP_normal = vec_AB.cross(vec_AP);

    double w = surface_normal.dot(ABP_normal);
    if(w < 0 && std::abs(w) > EPSILON)
    {
        return {};
    }

    //divide uvw by N^2
    const double denominator_uvw = surface_normal.dot(surface_normal);

    u /= denominator_uvw;
    v /= denominator_uvw;
    w /= denominator_uvw;

    if(not (std::abs(1 - u - v - w) < EPSILON && u > -EPSILON && v > -EPSILON && w > -EPSILON ) ) //u,v,w must be >= 0 each and total sum must be 1
    {
        debug_stop();
        return {};
    }

    //Boundary check: clamp u,v,w?

    u = NS_misc::clamp_d(u,0,1);
    v = NS_misc::clamp_d(v,0,1);
    w = NS_misc::clamp_d(w,0,1);

    return std::make_tuple(u,v,w);
}

constexpr bool Triangle::is_point_in_triangle(const Point3 &point) const
{
    return get_baryctr_coords_at(point).has_value();
}


} //NS_geometry


} //NS_dtools
#endif // GEOMETRY_H
