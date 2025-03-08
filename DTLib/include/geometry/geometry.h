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
    enum class TR_WINDING {RIGHTHAND_CCW, LEFTHAND_CW};

    constexpr Triangle(const Point3 &point1,
                       const Point3 &point2,
                       const Point3 &point3,
                       TR_WINDING winding,
                       bool flip_from_mirror_coord_system = false);

    [[nodiscard]] std::vector< double > intersect_ray(const Vector3d &origin,
                                                    const Vector3d &direction) const override;

    [[nodiscard]] constexpr Vector3d get_surface_normal_at(const Point3 &surface_point) const override;

    [[nodiscard]] constexpr std::optional< std::tuple<double, double, double> >  get_baryctr_coords_at(const Point3 &point) const;

    [[nodiscard]] constexpr bool is_point_in_triangle(const Point3 &point) const;

    //Did p2 and p3 have to be swapped from their order in the ctor to fulfill the winding requirement?
    [[nodiscard]] constexpr bool get_p2_p3_were_swapped() const noexcept {return had_to_swap_p2_p3_for_winding;}

private:
    [[nodiscard]] std::optional<Point3> get_vector_intersection(const Vector3d &vec) const;

    Point3 m_p1, m_p2, m_p3;
    TR_WINDING m_winding{TR_WINDING::RIGHTHAND_CCW};
    bool had_to_swap_p2_p3_for_winding{false};
};

constexpr Triangle::Triangle(const Point3 &point1,
                             const Point3 &point2,
                             const Point3 &point3,
                             TR_WINDING winding,
                             bool flip_from_mirror_coord_system)
    :m_p1(point1), m_p2(point2), m_p3(point3), m_winding(winding)
{
    using std::swap;
    //TODO: also check that the three points actually form a plane within Epsilon accuracy, otherwise throw exception

    //if transforming a right handed coord system triangle into this left handed one, flip the x-coords
    if(flip_from_mirror_coord_system)
    {
        m_p1(0) = -m_p1(0);
        m_p2(0) = -m_p2(0);
        m_p3(0) = -m_p3(0);
    }

    /*As a test: don't reorder, instead handle windedness completely when getting the normal
     * This assumes that the windedness can be described completely by its effects on the normal direction*/
    //make sure that the three points are ordered counter-clockwise in respect to the z axis
    const Vector3d vec_AB = m_p2 - m_p1;
    const Vector3d vec_AC = m_p3 - m_p1;

    //if normal has negative z, flip points 2 and 3
    //This makes all calculaitons less complicated, but is not ideal for the user of this class...
    if(vec_AB.cross(vec_AC)(2) < 0)
    {
        had_to_swap_p2_p3_for_winding = true;
        swap(m_p2, m_p3);

    }
    debug_assert((m_p2 - m_p1).cross(m_p3 - m_p1)(2) >= 0);

}

constexpr Vector3d Triangle::get_surface_normal_at(const Point3&) const
{
    //There is only one surface normal, regardless of where we are - defined by system handedness / winding
    const Vector3d vec_AB = m_p2 - m_p1;
    const Vector3d vec_AC = m_p3 - m_p1;

    const Vector3d surface_normal = vec_AB.cross(vec_AC);

    debug_assert(surface_normal(2) >= 0);

    const bool has_to_flip_z =  m_winding == TR_WINDING::LEFTHAND_CW;

    return surface_normal * (has_to_flip_z? -1 : 1);
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

    if(not (std::abs(1 - u - v - w) < EPSILON) )
    {
        debug_stop();
        return {};
    }

    return std::make_tuple(u,v,w);
}

constexpr bool Triangle::is_point_in_triangle(const Point3 &point) const
{
    return get_baryctr_coords_at(point).has_value();
}


} //NS_geometry


} //NS_dtools
#endif // GEOMETRY_H
