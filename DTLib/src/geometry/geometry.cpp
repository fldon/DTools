#include "geometry/geometry.h"
#include <DTools/MiscTools.h>

using namespace NS_dtools;
using namespace NS_dtools::NS_geometry;

/* SPHERE */
std::vector< double > Sphere::intersect_ray(const Point3 &origin,
                                          const Vector3d &direction) const
{
    std::vector<double> result;

    const Vector3d center_to_origin = origin - m_center;

    //these terms result from analytical solution of line intersection with sphere
    const double a = direction.dot(direction);
    const double b = 2 * center_to_origin.dot(direction);
    const double c = center_to_origin.dot(center_to_origin) - m_radius * m_radius;

    //Solve quadratic equation
    //t1,t2 = -b +- sqrt(b^2 - 4ac) / 2a
    const double discriminant = b*b - 4*a*c;
    if(discriminant < 0)
    {
        return result;
    }

    //t1 and t2 can be equal, but rounding errors make checking for that nonsensical
    const double t1 = (-b + sqrt(discriminant)) / (2*a);
    const double t2 = (-b - sqrt(discriminant)) / (2*a);
    result.push_back(t1);
    result.push_back(t2);

    return result;
}

Vector3d Sphere::get_surface_normal_at(const Point3 &surface_point) const
{
    const Vector3d center2point = surface_point - m_center;
    if(center2point.norm() < m_radius - EPSILON || center2point.norm() > m_radius + EPSILON)
    {
        throw NS_dtools::NS_misc::BaseOmegaException("Point is not on surface of sphere");
    }

    return (surface_point - m_center).normalized();
}

void Sphere::set_radius(double radius)
{
    if(radius < EPSILON)
    {
        throw NS_dtools::NS_misc::BaseOmegaException("Radius is negative!");
    }
    m_radius = radius;
}

double Sphere::get_radius() const
{
    return m_radius;
}

void Sphere::set_center(const Point3 &center)
{
    m_center = center;
}

const Point3& Sphere::get_center() const
{
    return m_center;
}
/* SPHERE */


/* TRIANGLE */
std::vector< double > Triangle::intersect_ray(const Vector3d &origin,
                                            const Vector3d &direction) const
{
    enum class INTERSECT_STRATEGY{GEOM_SLOW, GEOM_FAST, MT};
    constexpr INTERSECT_STRATEGY STRATEGY_TO_USE = INTERSECT_STRATEGY::GEOM_FAST;

    //Triangle points A,B,C
    //Points in triangle are 0A + b*AB + c*AC
    //Equality with line vector P + q*PQ gives:
    //b*AB + c*AC + q*QP = AP
    //Which can be put in the form matrix_A * (b,c,q) = AP, where matrix_A is given by the vectors
    //Solving for b,c,q gives the resulting intersection
    //Inequalities must be fulfilled: b>=0, c>=0 and b+c <=1 !
    //Otherwise, no valid intersection!

    //In the other functions, QP is D(irection) and P is O(rigin) and q is t

    //Problem with this approach: We solve the whole linear system before knowing whether the solution works or not
    //In the other approaches, there is an early exit

    switch(STRATEGY_TO_USE)
    {
    case INTERSECT_STRATEGY::GEOM_SLOW:
    {

        const Vector3d vec_AB = m_p2 - m_p1;
        const Vector3d vec_AC = m_p3 - m_p1;
        const Vector3d vec_AO = origin - m_p1;


        Eigen::Matrix3d matrix_A;
        matrix_A.col(0) = vec_AB;
        matrix_A.col(1) = vec_AC;
        matrix_A.col(2) = -direction;

        //result x is (b,c,q), with q being the potential t value we want, and b and c being u,v from the barycentr. coords u,v,w
        const Eigen::Vector3d x = matrix_A.fullPivLu().solve(vec_AO);
        const double b = x(0);
        const double c = x(1);
        const double t = x(2);


        //check for inequalities:
        const bool test_b = not ( (b < 0 && abs(b) > EPSILON) || (b > 1 && abs(b-1) > EPSILON) );
        const bool test_c = not ( (c < 0 && abs(c) > EPSILON) || (b + c > 1 && abs(b + c - 1) > EPSILON) );
        const bool test_t = t > EPSILON;
        const bool valid_intersect = test_b && test_c && test_t;

        std::vector<double> retvec;
        if(valid_intersect)
        {
            debug_assert(std::abs(1 - b - c) < EPSILON && get_baryctr_coords_at(origin + t * direction).has_value());
            retvec.push_back(t);
        }

        return retvec;
    }break;

        //TODO: This above gemoetric solution seems to give intersection points that fail the inside-out tests! Why?
        //if I explicitly perform the inside out test here and throw away the intersection if it fails, I see no visual difference...
        //Maybe literal edge cases?



        //geometric solution using barycentric coords:
        //Solution stems from equality of ray equation and plane equation
        //parameter d is the origin-plane distance used in the plane equation
    case INTERSECT_STRATEGY::GEOM_FAST:
    {
        const Vector3d surface_normal = get_surface_normal_at({});

        //check if ray and plane are parallel
        const double normal_dot_ray_direction = surface_normal.dot(direction);
        if(std::abs(normal_dot_ray_direction) < EPSILON)
        {
            return {}; //parallel: no intersect
        }

        //compute d parameter
        const double d = -surface_normal.dot(m_p1);

        //compute t
        const double t = -(surface_normal.dot(origin) + d) / normal_dot_ray_direction;

        //check if triangle is behind ray
        if(t < 0)
        {
            return {};
        }

        //Perform inside-out test to see if intersection point is in triangle
        if(not get_baryctr_coords_at(origin + t * direction).has_value())
        {
            return {};
        }

        const std::vector<double> resvec = {t};
        return resvec;
    }break;


        //Moeller-Trumbore-algorithm:
        //Uses cramers rule with the same equation as in the geometric slow solution
        //Calculates the solution bit by bit and aborts as soon as solution cannot be true anymore
    case INTERSECT_STRATEGY::MT:
    {
        const Point3 &b = m_p2;
        const Point3 &c = m_p3;
        const Point3 &a = m_p1;

        const Vector3d edge1 = b - a;
        const Vector3d edge2 = c - a;
        const Vector3d ray_cross_e2 = direction.cross(edge2);
        const double det = edge1.dot(ray_cross_e2);

        if (det > -EPSILON && det < EPSILON)
            return {};    // This ray is parallel to this triangle.

        const double inv_det = 1.0 / det;
        const Vector3d s = origin - a;
        const double u = inv_det * s.dot(ray_cross_e2);

        if ((u < 0 && abs(u) > EPSILON) || (u > 1 && abs(u-1) > EPSILON))
            return {};

        const Vector3d s_cross_e1 = s.cross(edge1);
        const double v = inv_det * direction.dot( s_cross_e1);

        if ((v < 0 && abs(v) > EPSILON) || (u + v > 1 && abs(u + v - 1) > EPSILON))
            return {};

        // At this stage we can compute t to find out where the intersection point is on the line.
        const double t = inv_det * edge2.dot(s_cross_e1);

        if (t > EPSILON) // ray intersection
        {
            return {t};
        }
        else        // This means that there is a line intersection but not a ray intersection.
        {
            return {};
        }
    }break;

    default:
        debug_stop();
        return {};
    }
}

/* TRIANGLE */
