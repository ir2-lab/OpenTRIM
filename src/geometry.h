#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#include <algorithm>
#include <limits>

#include <unsupported/Eigen/AlignedVector3>

/**
 * \defgroup Geometry Geometry
 *
 * \brief 3D vectors and spatial grid.
 *
 * @{
 *
 * The geometric features of the simulation are defined on a 3D rectangular grid
 * represented by the class grid3D.
 *
 * Space is divided into rectangular cells.
 *
 * The grid3D class offers functions to check if a particular
 * 3D point is within the grid, to which cell it belongs, etc
 *
 * A grid3D comprises of three objects of class grid1D representing
 * the grid in each of the axes.
 *
 * @ingroup MC
 *
 * @}
 */


/**
 * @brief The class vector3 represents a 3D vector.
 *
 * It is based on Eigen::AlignedVector3 with float as the basic type,
 * so that the code can
 * benefit from SSE cpu instructions where possible. For example,
 * vector addition can be performed in one cpu intstruction.
 *
 * @ingroup Geometry
 */
typedef Eigen::AlignedVector3<float> vector3;

/**
 * @brief The class ivector3 represents a 3D index vector.
 *
 * It is based on Eigen::AlignedVector3 with integer as the basic type.
 * Thus, the code can
 * benefit from SSE cpu instructions where possible. For example,
 * vector addition can be performed in one cpu intstruction.
 *
 * Index vectors are used to specify a cell in the 3D rectangular grid.
 *
 * @ingroup Geometry
 */
typedef Eigen::AlignedVector3<int> ivector3;

/**
 * @brief A 1D range based on Eigen::AlignedBox1f
 * @ingroup Geometry
 */
typedef Eigen::AlignedBox1f box1D;

/**
 * @brief A 1D index range based on Eigen::AlignedBox1i
 * @ingroup Geometry
 */
typedef Eigen::AlignedBox1i ibox1D;

/**
 * @brief A rectangular 3D range based on Eigen::AlignedBox3f
 * @ingroup Geometry
 */
typedef Eigen::AlignedBox3f box3D;

/**
 * @brief The grid1D class represents a 1D spatial partition
 *
 * It is essentially an array of \f$ N \f$ monotonically increasing
 * points, \f$ x_0,x_1,...,x_{N-1} \f$, which divides the region
 * between the 1st and the Nth point in \f$ N-1 \f$ cells.
 *
 * By convention \f$ x_0 = 0\f$ and \f$ x_{N-1} = w\f$, where \f$w\f$
 * is the width of the grid.
 *
 * The cells do not have to be of equal width, however, currently only
 * equidistant grids have been employed.
 *
 * @ingroup Geometry
 *
 */
class grid1D : public std::vector<float> {

    typedef std::vector<float> vec_t;

    float w_, dx_;
    bool equispaced_;
    bool periodic_;

public:
    /// Default constructor creates an empty grid
    grid1D() : vec_t(), w_(0), dx_(0),
        equispaced_(false), periodic_(false)
    {}
    /// Copy constructor
    grid1D(const grid1D& g) : vec_t(g),
        w_(g.w_), dx_(g.dx_),
        equispaced_(g.equispaced_),
        periodic_(g.periodic_)
    {}

    /// Returns the total width \f$ x_{N-1}-x_0 \f$
    float w() const { return w_; }
    /// Returns cell width (if equispaced)
    float dx() const { return dx_; }
    /// Returns true if all cells have the same width
    bool equispaced() const { return equispaced_; }
    /// Returns true if the grid has periodic boundary conditions
    bool periodic() const { return periodic_; }
    /// Set periodic boundary conditions on or off depebing on the value of b
    void setPeriodic(bool b) { periodic_ = b; }

    /// Set the grid to the values of the array x
    void set(const vec_t& x) {
        vec_t::assign(x.begin(),x.end());
        w_ = back() - front();
        dx_ = 0;
        float x0 = front();
        front() = 0.f;
        for(int i=1; i<size(); i++) at(i) += x0;
        back() = w_;
        equispaced_ = false;
    }
    /// Create an equidistant grid from x0 to x1 divided in n cells
    void set(float w, int n) {
        resize(n+1);
        front() = 0.f;
        w_ = w;
        dx_ = w/n;
        for(int i=1; i<=n; i++) at(i) = i*dx_;
        back() = w;
        equispaced_ = true;
    }

    /// Returns true if x is within the grid region
    bool contains(const float& x) const {
        return (x>=0.f) && (x<w_);
    }

    /**
     * @brief Returns true if x is within the grid region, anticipating periodic boundary conditions
     *
     * If periodic boundary conditions are on and x is outside the grid, then x will be
     * brought within the range by adding or subtracting an integer multiple of the
     * grid period (width). The function will return
     * true.
     *
     * @param x is the posistion to check
     * @return true if x is within the grid
     */
    bool contains_with_bc(float& x) const {
        if (x<0.f) {
            if (periodic_) {
                do x += w_; while (x<0.f);
                if (x == w_)
                    x = std::nextafter(w_, std::numeric_limits<float>::lowest());
                assert(x<w_);
                return true;
            } else return false;
        } else if (x>=w_) {
            if (periodic_) {
                do x -= w_; while (x>=w_);
                assert(x>=0.f);
                return true;
            } else return false;
        } else return true;
    }
    void apply_bc(float& x) const {
        if (periodic_) {
            if (x<0.f) {
                do x += w_; while (x<0.f);
                if (x == w_)
                    x = std::nextafter(w_, std::numeric_limits<float>::lowest());
                assert(x<w_);
            } else if (x>=w_) {
                do x -= w_; while (x>=w_);
                assert(x>=0.f);
            }
        }
    }

    /// Returns true if x is inside the i-th cell, \f$ x_i \leq x < x_{i+1} \f$
    bool contains(int i, const float& x) const {
        return (x>=at(i)) && (x<at(i+1));
    }

    /// Returns the cell index i for which \f$ x_i \leq x < x_{i+1} \f$.
    /// Should be called only if grid1D::contains(x) returns true
    int pos2cell(const float& x) const {
        assert(contains(x));
        if (size()==2) return 0;
        if (equispaced_) {
            return std::floor(x/dx_);
        }
        else
            return std::upper_bound(begin(), end(), x) - begin() - 1;
    }

    /**
     * @brief Return the range of cell indexes that are within the spatial range b
     *
     * The function returns the range of cells that have their centers
     * within the interval defined by b.
     *
     *
     * @param b the range as a box1D
     * @return the range of cells as an ibox1D
     */
    ibox1D range(const box1D& b) const
    {
        int i1(0), i2(size()-2);
        float x1 = 0.5*(at(i1)+at(i1+1));
        float x2 = 0.5*(at(i2)+at(i2+1));
        while (i1<i2 && !b.contains(box1D::VectorType(x1))) {
            i1++; x1 = 0.5*(at(i1)+at(i1+1));
        }
        while (i2>i1 && !b.contains(box1D::VectorType(x2))) {
            i2--; x2 = 0.5*(at(i2)+at(i2+1));
        }
        return ibox1D(i1,i2);
    }

};


/**
 * @brief The grid3D class represents a 3D rectangular grid.
 *
 * It comprises of three grid1D objects, one for each dimension,
 * which can be accessed by the funtions grid3D::x(), grid3D::y() and
 * grid3D::z().
 *
 * @ingroup Geometry
 */
class grid3D
{
    grid1D x_, y_, z_;
    box3D box_;

    void calcBox()
    {
        box_.min() = box3D::VectorType(0.f,0.f,0.f);
        box_.max() = box3D::VectorType(x_.w(),y_.w(),z_.w());
    }

public:


    typedef enum {
        X=0,
        Y=1,
        Z=2
    } Axis;

    /// Default constructor creates empty grid
    grid3D()
    {
        x_.set(1.f,2);
        y_.set(1.f,2);
        z_.set(1.f,2);
        calcBox();
    }
    /// Copy constructor
    grid3D(const grid3D& g) :
        x_(g.x_), y_(g.y_), z_(g.z_), box_(g.box_)
    {}

    /// Set the x-axis grid to an equidistant partition of n cells
    void setX(float w, int n, bool periodic)
    { x_.set(w,n); calcBox(); x_.setPeriodic(periodic); }
    /// Set the y-axis grid to an equidistant partition of n cells
    void setY(float w, int n, bool periodic)
    { y_.set(w,n); calcBox(); y_.setPeriodic(periodic); }
    /// Set the z-axis grid to an equidistant partition of n cells
    void setZ(float w, int n, bool periodic)
    { z_.set(w,n); calcBox(); z_.setPeriodic(periodic); }

    /// Return the x-axis grid
    const grid1D& x() const { return x_; }
    /// Return the y-axis grid
    const grid1D& y() const { return y_; }
    /// Return the z-axis grid
    const grid1D& z() const { return z_; }
    /// Return the rectangular box containing the whole 3D grid
    const box3D& box() const { return box_; }

    /// Return the i-th cell rectangular box
    box3D box(const ivector3& i) const {
        vector3 X0(x_[i.x()],y_[i.y()],z_[i.z()]);
        vector3 X1(x_[i.x()+1],y_[i.y()+1],z_[i.z()+1]);
        return box3D(X0, X1);
    }

    /// Return true if v is within the grid
    bool contains(const vector3& v) const {
        return x_.contains(v.x()) &&
               y_.contains(v.y()) &&
               z_.contains(v.z());
    }
    /// Return true if v is within the grid, adjusting v for periodic boundaries
    bool contains_with_bc(vector3& v) const {
        return x_.contains_with_bc(v.x()) &&
               y_.contains_with_bc(v.y()) &&
               z_.contains_with_bc(v.z());
    }
    /// Return true if v is within the cell with indexes i
    bool contains(const ivector3& i, const vector3& v) const {
        return x_.contains(i.x(),v.x()) &&
            y_.contains(i.y(),v.y()) &&
            z_.contains(i.z(),v.z());
    }
    /// Return the cell index for v; call only if contains()==true
    ivector3 pos2cell(const vector3& v) const
    {
        assert(contains(v));
        return ivector3(x_.pos2cell(v.x()),
                        y_.pos2cell(v.y()),
                        z_.pos2cell(v.z()));
    }
    void apply_bc(vector3& v) const
    {
        x_.apply_bc(v.x());
        y_.apply_bc(v.y());
        z_.apply_bc(v.z());
    }

    /// Return the id of the i-th cell
    int cellid(const ivector3& i) const {
        return (i.x()*(y_.size()-1) + i.y())*(z_.size()-1) + i.z();
    }

    static bool isNull(const ivector3& i)  {
        return (i.x()<0) || (i.y()<0) || (i.z()<0);
    }

    static ivector3 nullcell() {
        return ivector3(-1,-1,-1);
    }
    /// Returns the total number of cells
    int ncells() const {
        return (x_.size()-1)*
               (y_.size()-1)*
               (z_.size()-1);
    }
    /// Returns the total volume
    float volume() const {
        return x_.w() * y_.w() * z_.w();
    }
};

/**
 * @brief Find the distance traveled by a particle until it crosses a rectangular box boundary
 * 
 * The algorithm assumes that particle is inside a rectangular box b.
 *
 * b.min() is the vector at the box origin and b.max() is the opposite corner.
 *
 * For each dimension the equation for the distance \f$ \ell \f$ to the boundary is
 * (e.g. for x)
 * 
 * \f[
 * \ell_x = (x_{max} - x) / n_x, \quad  \mbox{if} \; n_x > 0
 * \f]
 * 
 * OR
 *
 * \f[
 * \ell_x = (x_{min} - x) / n_x, \quad  \mbox{if} \; n_x < 0
 * \f]
 * 
 * where \f$ n_x \f$ is the x direction cosine
 *
 * Finally \f$ \ell = \mbox{min}\left[ \ell_x, \ell_y, \ell_z \right] \f$.
 * 
 * @param b   Rectangular box
 * @param pos particle position
 * @param dir particle direction
 * @return float 
 * 
 * @ingroup Geometry
 */
inline
float distance2boundary(const box3D& b, const vector3& pos, const vector3& dir)
{
    assert(b.contains(pos));

    vector3 d = b.sizes();
    if (dir[0]<0.f) d[0]=0.f;
    if (dir[1]<0.f) d[1]=0.f;
    if (dir[2]<0.f) d[2]=0.f;
    d -= pos;
    d += b.min();
    d.array() /= dir.array();
    return d.minCoeff();

//    vector3 d;
//    d.x() = dir.x()>0 ? b.max().x() - pos.x() : b.min().x() - pos.x();
//    d.y() = dir.y()>0 ? b.max().y() - pos.y() : b.min().y() - pos.y();
//    d.z() = dir.z()>0 ? b.max().z() - pos.z() : b.min().z() - pos.z();
//    d.array() /= dir.array();
//    return d.minCoeff();

    // (Slightly ~1%) Slower implementations

    // float d = std::numeric_limits<float>::infinity(), d1;
    // d1 = (dir.x()>0 ? b.max().x() - pos.x() : b.min().x() - pos.x()) / dir.x();
    // d = std::min(d, d1);
    // d1 = (dir.y()>0 ? b.max().y() - pos.y() : b.min().y() - pos.y()) / dir.y();
    // d = std::min(d, d1);
    // d1 = (dir.z()>0 ? b.max().z() - pos.z() : b.min().z() - pos.z()) / dir.z();
    // d = std::min(d, d1);
    // return d;

    // vector3 d = b.max() - pos, d1 = b.min() - pos;
    // d.array() /= dir.array();
    // d1.array() /= dir.array();
    // d = d.cwiseMax(d1);
    // return d.minCoeff();

    // vector3 L = b.sizes();
    // for(int i=0; i<3; i++) if (dir[i]<0.f) L[i]=0.f;
    // L -= pos;
    // L += b.min();
    // L.array() /= dir.array();
    // return L.minCoeff();
}

/**
 * @brief Propagates a particle to the cell boundary
 *
 * The algorithm assumes that the particle is inside a rectangular box b.
 *
 * First, the distance \f$d\f$ is found to the boundary that the particle will
 * cross, similarly to \ref distance2boundary().
 *
 * If the particle crosses the boundary perpendicular to, e.g., the \f$x\f$-axis, then
 * its \f$x\f$ coordinate is updated to
 *
 * \f[
 * x' = x_{max} + \delta, \quad  \mbox{if} \; n_x > 0
 * \f]
 *
 * OR
 *
 * \f[
 * x' = x_{min} - \delta, \quad  \mbox{if} \; n_x < 0
 * \f]
 *
 * where \f$(x_{min},x_{max})\f$ are the cell boundaries along the \f$x\f$-axis and 
 * \f$\delta\f$ is a small number so that the new position is guaratied to lie
 * outside of the box. This is accomplished by using the std::nextafter() function of
 * the std. math library.
 *
 * The other two coordinates are updated normally by
 * \f[
 * y' = y + n_y\,d, \quad  z' = z + n_z\,d
 * \f]
 *
 * @param b   Rectangular box
 * @param pos particle position vector
 * @param dir particle direction vector
 * @return float distance travelled to the box boundary
 *
 * @ingroup Geometry
 */
inline
float bring2boundary(const box3D& b, vector3& pos, const vector3& dir)
{
    assert(b.contains(pos));

    float d(std::numeric_limits<float>::infinity()),
        d1;
    int imax = -1;
    if (dir.x() != 0.0f) {
        d1 = dir.x()>0 ? b.max().x() - pos.x() : b.min().x() - pos.x();
        d1 /= dir.x();
        if (d1 < d) { d = d1; imax = 0; }
    }
    if (dir.y() != 0.0f) {
        d1 = dir.y()>0 ? b.max().y() - pos.y() : b.min().y() - pos.y();
        d1 /= dir.y();
        if (d1 < d) { d = d1; imax = 1; }
    }
    if (dir.z() != 0.0f) {
        d1 = dir.z()>0 ? b.max().z() - pos.z() : b.min().z() - pos.z();
        d1 /= dir.z();
        if (d1 < d) { d = d1; imax = 2; }
    }

    switch (imax) {
    case 0:
        pos.x() = (dir.x()>0) ? std::nextafter(b.max().x(), std::numeric_limits<float>::max()) :
                      std::nextafter(b.min().x(), std::numeric_limits<float>::lowest());
        pos.y() += dir.y()*d;
        pos.z() += dir.z()*d;
        break;
    case 1:
        pos.y() = (dir.y()>0) ? std::nextafter(b.max().y(), std::numeric_limits<float>::max()) :
                      std::nextafter(b.min().y(), std::numeric_limits<float>::lowest());
        pos.x() += dir.x()*d;
        pos.z() += dir.z()*d;
        break;
    case 2:
        pos.z() = (dir.z()>0) ? std::nextafter(b.max().z(), std::numeric_limits<float>::max()) :
                      std::nextafter(b.min().z(), std::numeric_limits<float>::lowest());
        pos.y() += dir.y()*d;
        pos.x() += dir.x()*d;
        break;
    default:
        assert(0);
    }

    return d;
}

/**
 * @brief Rotates the direction vector of a particle according to the scattering angles \f$ (\theta,\phi) \f$
 * 
 * The 1st argument represents the particle's direction vector \f$ \mathbf{m} \f$.
 * 
 * The 2nd argument is a vector
 * \f[
 * \mathbf{n} = (\cos\phi\,\sin\theta, \sin\phi\,\sin\theta,\cos\theta)
 * \f]
 * where \f$ (\theta,\phi) \f$ are the polar and azimuthal scattering
 * angles.
 * 
 * The new direction after scattering is obtained by the well-known result
 * \f{eqnarray*}{
 * m'_x &=& m_x\, n_z + \frac{m_x\, m_z\, n_x - m_y\, n_y}{\sqrt{1-m_z^2}} \\
 * m'_y &=& m_y\, n_z + \frac{m_y\, m_z\, n_x - m_x\, n_y}{\sqrt{1-m_z^2}}  \\
 * m'_z &=& m_z\, n_z - n_x\, \sqrt{1-m_z^2}
 * \f}
 * 
 * \f$\mathbf{m'}\f$ is again normalized to minimize round-off errors.
 * 
 * @param m direction vector
 * @param n deflection vector
 * 
 * @ingroup Geometry
 */
inline
void deflect_vector(vector3 &m, const vector3 &n)
{
    // if the ion moves parallel to the z-axis
    // then the new direction is n
    float smz = 1-m.z()*m.z();
    if (smz <= 0.f) {
        m = n;
        return;
    }

    vector3 m1 = m;
    smz = std::sqrt(smz);
    m.x() = m1.x()*n.z() + (m1.x()*m1.z()*n.x() - m1.y()*n.y())/smz;
    m.y() = m1.y()*n.z() + (m1.y()*m1.z()*n.x() + m1.x()*n.y())/smz;
    m.z() = m1.z()*n.z() - n.x()*smz;

    // normalize
    m.normalize();

}

#endif // GEOMETRY_H
