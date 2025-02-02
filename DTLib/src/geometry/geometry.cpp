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
/* SPHERE */


/* TRIANGLE */
std::vector< double > Triangle::intersect_ray(const Vector3d &origin,
                                              const Vector3d &direction) const
{
    //TODO: Why does the moeller-trumbore work but my simple approach does not???
    //TODO: also, how does the moller-trumbore work and how does it differ from my approach?


    //Triangle points A,B,C
    //Points in triangle are 0A + b*AB + c*AC
    //Equality with line vector P + q*PQ gives:
    //b*AB + c*AC + q*QP = AP
    //Which can be put in the form matrix_A * (b,c,q) = AP, where matrix_A is given by the vectors
    //Solving for b,c,q gives the resulting intersection
    //Inequalities must be fulfilled: b>=0, c>=0 and b+c <=1 !
    //Otherwise, no valid intersection!

    //In the other functions, Q is D(irection) and P is O(rigin) and q is t

    constexpr double epsilon = EPSILON;

    /* If I add this check to my own version, the problem disappears. But why can I not check this later, after getting the wrong solution???
    {

    Point3 b = m_p2;
    Point3 c = m_p3;
    Point3 a = m_p1;

    Vector3d edge1 = b - a;
    Vector3d edge2 = c - a;
    Vector3d ray_cross_e2 = direction.cross(edge2);
    double det = edge1.dot(ray_cross_e2);

    if (det > -epsilon && det < epsilon)
        return {};    // This ray is parallel to this triangle.

    double inv_det = 1.0 / det;
    Vector3d s = origin - a;
    double u = inv_det * s.dot(ray_cross_e2);

    if ((u < 0 && abs(u) > epsilon) || (u > 1 && abs(u-1) > epsilon))
        return {};
    }
*/
    /* It seems like this solution falls apart when there is no real solution.
     * The value-check does not work because the values for b,c,t are themselves okay,
     * but the solution they symbolize is simply wrong
     * Maybe Eigen just produces garbage when a system has no solution?
    const Vector3d vec_AB = m_p2 - m_p1;
    const Vector3d vec_AC = m_p3 - m_p1;
    const Vector3d vec_DO = origin - direction;
    const Vector3d vec_AO = origin - m_p1;


    Eigen::Matrix3d matrix_A;
    matrix_A.col(0) = vec_AB;
    matrix_A.col(1) = vec_AC;
    matrix_A.col(2) = vec_DO;


    //result x is (b,c,q), with q being the potential t value we want
    const Eigen::Vector3d x = matrix_A.colPivHouseholderQr().solve(vec_AO);
    const double b = x(0);
    const double c = x(1);
    const double t = x(2);


    //check for inequalities:
    //bool test = (b < 0 && abs(b) > epsilon) || (b > 1 && abs(b-1) > epsilon);
    //const bool valid_intersect = b >= 0 && c >= 0 && b + c <= 1 && t >= 0 && test;
    const bool test_b = not ( (b < 0 && abs(b) > epsilon) || (b > 1 && abs(b-1) > epsilon) );
    const bool test_c = not ( (c < 0 && abs(c) > epsilon) || (b + c > 1 && abs(b + c - 1) > epsilon) );
    const bool test_t = t > epsilon;
    const bool valid_intersect = test_b && test_c && test_t;

    std::vector<double> retvec;
    if(valid_intersect)
    {
        retvec.push_back(t);
    }

    const Point3 alleged_intersect_point = origin + t* direction;
    if( valid_intersect && alleged_intersect_point(2) < 0 )
    {
        int SOMETHINGWENTWRONG = 5;
    }

    return retvec;
*/


    //Moeller-Trumbore-algorithm:
    //TODO: try to understand why this algorithm works
    const Point3 &b = m_p2;
    const Point3 &c = m_p3;
    const Point3 &a = m_p1;

    const Vector3d edge1 = b - a;
    const Vector3d edge2 = c - a;
    const Vector3d ray_cross_e2 = direction.cross(edge2);
    const double det = edge1.dot(ray_cross_e2);

    if (det > -epsilon && det < epsilon)
        return {};    // This ray is parallel to this triangle.

    const double inv_det = 1.0 / det;
    const Vector3d s = origin - a;
    const double u = inv_det * s.dot(ray_cross_e2);

    if ((u < 0 && abs(u) > epsilon) || (u > 1 && abs(u-1) > epsilon))
        return {};

    const Vector3d s_cross_e1 = s.cross(edge1);
    const double v = inv_det * direction.dot( s_cross_e1);

    if ((v < 0 && abs(v) > epsilon) || (u + v > 1 && abs(u + v - 1) > epsilon))
        return {};

    // At this stage we can compute t to find out where the intersection point is on the line.
    const double t = inv_det * edge2.dot(s_cross_e1);

    if (t > epsilon) // ray intersection
    {
        return {t};
    }
    else        // This means that there is a line intersection but not a ray intersection.
    {
        return {};
    }

}
/* TRIANGLE */
