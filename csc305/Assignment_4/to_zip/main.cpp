////////////////////////////////////////////////////////////////////////////////
// C++ include
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <Eigen/Core>

// Utilities for the Assignment
#include "utils.h"

// Image writing library
#define STB_IMAGE_WRITE_IMPLEMENTATION // Do not include this line twice in your project!
#include "stb_image_write.h"

// Shortcut to avoid Eigen:: everywhere, DO NOT USE IN .h
using namespace Eigen;

////////////////////////////////////////////////////////////////////////////////
// Class to store tree
////////////////////////////////////////////////////////////////////////////////
class AABBTree
{
public:
    class Node
    {
    public:
        AlignedBox3d bbox;
        int parent;   // Index of the parent node (-1 for root)
        int left;     // Index of the left child (-1 for a leaf)
        int right;    // Index of the right child (-1 for a leaf)
        int triangle; // Index of the node triangle (-1 for internal nodes)
    };

    std::vector<Node> nodes;
    int root;

    AABBTree() = default;                           // Default empty constructor
    AABBTree(const MatrixXd &V, const MatrixXi &F); // Build a BVH from an existing mesh
};

////////////////////////////////////////////////////////////////////////////////
// Scene setup, global variables
////////////////////////////////////////////////////////////////////////////////

const std::string data_dir = DATA_DIR;
const std::string filename("raytrace.png");
const std::string mesh_filename(data_dir + "bunny.off");


//Camera settings
const double focal_length = 12;
const double field_of_view = 0.7854; //45 degrees
const double image_z = 5;
const bool is_perspective = true;
const Vector3d camera_position(0, -0.2, 3);

//Maximum number of recursive calls
const int max_bounce = 5;

// Triangle Mesh
MatrixXd vertices; // n x 3 matrix (n points)
MatrixXi facets;   // m x 3 matrix (m triangles)
AABBTree bvh;

// Objects
std::vector<Vector3d> sphere_centers;
std::vector<double> sphere_radii;
std::vector<Matrix3d> parallelograms;

// Material for the object, same material for all objects
const Vector4d obj_ambient_color(0.0, 0.5, 0.0, 0);
const Vector4d obj_diffuse_color(0.5, 0.5, 0.5, 0);
const Vector4d obj_specular_color(0.2, 0.2, 0.2, 0);
const double obj_specular_exponent = 256.0;
const Vector4d obj_reflection_color(0.7, 0.7, 0.7, 0);
const Vector4d obj_refraction_color(0.7, 0.7, 0.7, 0);


// Precomputed (or otherwise) gradient vectors at each grid node
const int grid_size = 20;
std::vector<std::vector<Vector2d>> grid;

// Lights
std::vector<Vector3d> light_positions;
std::vector<Vector4d> light_colors;
// Ambient light
const Vector4d ambient_light(0.2, 0.2, 0.2, 0);

// Fills the different arrays
void setup_scene()
{
    // Loads file
    std::ifstream in(mesh_filename);
    std::string token;
    in >> token;
    int nv, nf, ne;
    in >> nv >> nf >> ne;
    vertices.resize(nv, 3);
    facets.resize(nf, 3);
    for (int i = 0; i < nv; ++i)
    {
        in >> vertices(i, 0) >> vertices(i, 1) >> vertices(i, 2);
    }
    for (int i = 0; i < nf; ++i)
    {
        int s;
        in >> s >> facets(i, 0) >> facets(i, 1) >> facets(i, 2);
        assert(s == 3);
    }

    // setup tree
    bvh = AABBTree(vertices, facets);

    //Spheres
    sphere_centers.emplace_back(10, 0, 1);
    sphere_radii.emplace_back(1);

    sphere_centers.emplace_back(7, 0.05, -1);
    sphere_radii.emplace_back(1);

    sphere_centers.emplace_back(1, 0.2, -1);
    sphere_radii.emplace_back(1);

    sphere_centers.emplace_back(-2, 0.4, 1);
    sphere_radii.emplace_back(1);


    //parallelograms
    parallelograms.emplace_back();
    parallelograms.back() << -100, 100, -100,
        -1.25, 0, -1.2,
        -100, -100, 100;


    // Lights
    light_positions.emplace_back(8, 8, 0);
    light_colors.emplace_back(16, 16, 16, 0);

    light_positions.emplace_back(6, -8, 0);
    light_colors.emplace_back(16, 16, 16, 0);

    light_positions.emplace_back(4, 8, 0);
    light_colors.emplace_back(16, 16, 16, 0);

    light_positions.emplace_back(2, -8, 0);
    light_colors.emplace_back(16, 16, 16, 0);

    light_positions.emplace_back(0, 8, 0);
    light_colors.emplace_back(16, 16, 16, 0);

    light_positions.emplace_back(-2, -8, 0);
    light_colors.emplace_back(16, 16, 16, 0);

    light_positions.emplace_back(-4, 8, 0);
    light_colors.emplace_back(16, 16, 16, 0);
}

//We need to make this function visible
Vector4d shoot_ray(const Vector3d &ray_origin, const Vector3d &ray_direction, int max_bounce);

////////////////////////////////////////////////////////////////////////////////
// BVH Code
////////////////////////////////////////////////////////////////////////////////

AlignedBox3d bbox_from_triangle(const Vector3d &a, const Vector3d &b, const Vector3d &c)
{
    AlignedBox3d box;
    box.extend(a);
    box.extend(b);
    box.extend(c);
    return box;
}

Vector3d get_barycentric(const int &index, MatrixXd cur_matrix, const MatrixXd &V, const MatrixXi &F)
{
    /*-- fetch a,b,c parameters of the triangle --*/
    Vector3d a = V.row(F.row(index).transpose()[0]).transpose();
    Vector3d b = V.row(F.row(index).transpose()[1]).transpose();
    Vector3d c = V.row(F.row(index).transpose()[2]).transpose();

    /*-- set vectors of the triangle --*/
    Vector3d v0 = b - a;
    Vector3d v1 = c - a;
    Vector3d v2 = cur_matrix.row(index).transpose() - a;

    /*-- compute, set, and return barycentric coordinates --*/
    double denom = v0(0) * v1(1) - v1(0) * v0(1);
    double v = (v2(0) * v1(1) - v1(0) * v2(1)) / denom;
    double w = (v0(0) * v2(1) - v2(0) * v0(1)) / denom;
    double u = 1.0f - v - w;
    Vector3d barycentr(u, v, w);
    return barycentr;
}

bool compare_head(const VectorXd &lhs, const VectorXd &rhs)
{
    return lhs(0) < rhs(0);
}

MatrixXd sort_rows(MatrixXd cur_matrix)
{
    std::vector<Eigen::VectorXd> cur_vec;
    for (int i = 0; i < cur_matrix.rows(); ++i)
    {
        cur_vec.push_back(cur_matrix.row(i)); // pushes value passed to the back of the vector
    }

    std::sort(cur_vec.begin(), cur_vec.end(), &compare_head);
    for (int i = 0; i < cur_matrix.rows(); ++i)
    {
        cur_matrix.row(i) = cur_vec[i];
    }
    return cur_matrix;
}
/*
bool compare_bary(int i, int j)
{
    return (centroids(i,max_axis) < centroids(j,max_axis));
}*/

// bol = beginning of list
// eol = end of list

int build_tree_rec(int bol, int eol, int parent, std::vector<int> nodeVec, MatrixXd centroids, const MatrixXd &V, const MatrixXi &F)
{
    AABBTree::Node cur;

    // -- leaf node
    if ((eol - bol) == 1)
    {
        cur.triangle = bol;
        cur.parent = parent;
        cur.right = -1;
        cur.left = -1;

        // -- fetch a,b,c parameters of the triangle
        Vector3d a = V.row(F.row(bol).transpose()[0]).transpose();
        Vector3d b = V.row(F.row(bol).transpose()[1]).transpose();
        Vector3d c = V.row(F.row(bol).transpose()[2]).transpose();
        cur.bbox = bbox_from_triangle(a, b, c);
        return nodeVec[bol]; // box id, this sets the parent

    } else {

        // -- set coordinates of current box
        AlignedBox3d temp_box;
        for (int k = bol; k < eol; k++) {
            temp_box.extend(get_barycentric(k, centroids, V, F)); // come back to this
        }

        // -- find additional parameters of current box for ray trace
        Vector3d diagon = temp_box.diagonal();
        int max_axis;
        if ((temp_box.max().x() - temp_box.min().x()) > (temp_box.max().y() - temp_box.min().y()) 
            && (temp_box.max().x() - temp_box.min().x()) > (temp_box.max().z() - temp_box.min().z())) {
            max_axis = 0;
        } else if ((temp_box.max().y() - temp_box.min().y()) > (temp_box.max().z() - temp_box.min().z())) {
            max_axis = 1;
        } else {
            max_axis = 2;
        }

        // -- sort list of triangles
        //std::sort(nodeVec.begin(), nodeVec.end(), &compare_bary); // come back

        // -- recursively call build_tree_rec while making list smaller
        int temp_m = (eol - bol) / 2 + bol;
        AABBTree::Node parent_node; 
        // parent_node.triangle = i;
        // cur_node.bbox = 
        // bvh.nodes.push_back(cur_node);

        size_t middle = nodeVec.size() / 2;
        std::vector<int>::const_iterator middleIter(nodeVec.cbegin());
        std::advance(middleIter, middle);

        std::vector<int> leftHalf(nodeVec.cbegin(), middleIter);
        std::vector<int> rightHalf(middleIter, nodeVec.end());

        cur.left = build_tree_rec(bol, temp_m, parent, leftHalf, centroids, V, F);
        cur.right = build_tree_rec(temp_m, eol, parent, rightHalf, centroids, V, F);

        // -- set new values & return node
        parent_node.left = cur.left;
        parent_node.right = cur.right;
        parent_node.bbox = temp_box;
        cur.parent = parent;
        return parent;
    }
} 
/*
Please note: I spent 2 days straight trying to get this code to work and 
spent hours looking at pseudocode and trying to debug my issues. 
The code was not working, so I had to comment out the section 
that calls these functions. Sorry for the inconvenience.
*/

AABBTree::AABBTree(const MatrixXd &V, const MatrixXi &F)
{
    // Compute the centroids of all the triangles in the input mesh
    MatrixXd centroids(F.rows(), V.cols());
    centroids.setZero();
    for (int i = 0; i < F.rows(); ++i)
    {
        for (int k = 0; k < F.cols(); ++k)
        {
            centroids.row(i) += V.row(F(i, k));
        }
        centroids.row(i) /= F.cols();
    }
    
    // Step 0: sort centroids
    sort_rows(centroids);
    
    // Step 1: split input set into 2 sets
    std::vector<int> s1;
    std::vector<int> s2;

    for (int g = 0; g < centroids.rows(); ++g)
    {
        if (g <= centroids.rows() / 2) {
            s1.push_back(g);
        } else {
            s2.push_back(g);
        }
    } 
    std::sort(s1.begin(), s1.end(), [&](int a, int b) {
        return centroids.row(a).x() < centroids.row(b).x();
    });
    
    std::sort(s2.begin(), s2.end(), [&](int a, int b) {
        return centroids.row(a).x() < centroids.row(b).x();
    });

    
    // Step 2: recursively build subtree T1 
    int t1;
    t1 = build_tree_rec(0, s1.size(), -1, s1, centroids, V, F);
    
    // Step 3: recursively build subtree T2
    int t2;
    t2 = build_tree_rec(0, s2.size(), -1, s2, centroids, V, F);

    // Step 4: Merge root of T1 and T2
    AABBTree::Node root_node; 
    root_node.left = t1;
    root_node.right = t2; 
    // bvh.nodes(0) = root_node;
} 

////////////////////////////////////////////////////////////////////////////////
// Intersection code
////////////////////////////////////////////////////////////////////////////////

double ray_triangle_intersection(const Vector3d &ray_origin, const Vector3d &ray_direction, const Vector3d &a, const Vector3d &b, const Vector3d &c, Vector3d &p, Vector3d &N)
{
    // TODO
    // Compute whether the ray intersects the given triangle.
    // If you have done the parallelogram case, this should be very similar to it.
    const Vector3d pgram_u = b - a;
    const Vector3d pgram_v = c - a;

    double t = -1;

    // Check if the ray intersects with the parallelogram
    Matrix3d C;
    C.col(0) = -pgram_u;
    C.col(1) = -pgram_v;
    C.col(2) = ray_direction;

    Vector3d f = a - ray_origin;
    Vector3d k = C.inverse() * f;
    // t >= 0 && u
    if (k(2) >= 0 && k(0) <= 1 && k(0) >= 0 && (k(0) + k(1)) <= 1 && k(1) >= 0 && k(1) <= 1)
    {
        // Compute intersection point
        p = ray_origin + (k(2) * ray_direction);

        // Compute normal at the intersection point
        N = (pgram_v.cross(pgram_u)).normalized();
        return k(2);
    }
    return -1;
}

bool ray_box_intersection(const Vector3d &ray_origin, const Vector3d &ray_direction, const AlignedBox3d &box)
{
    // TODO
    // Compute whether the ray intersects the given box.
    // we are not testing with the real surface here anyway.
    int txmin = (box.min()[0] - ray_origin(0)) / ray_direction(0);
    int txmax = (box.max()[0] - ray_origin(0)) / ray_direction(0);
    int tymin = (box.min()[1] - ray_origin(1)) / ray_direction(1);
    int tymax = (box.max()[1] - ray_origin(1)) / ray_direction(1);

    if ((txmin > tymax) || (tymin > txmax))
    {
        return false;
    }
    return true;
}

//Compute the intersection between a ray and a sphere, return -1 if no intersection
double ray_sphere_intersection(const Vector3d &ray_origin, const Vector3d &ray_direction, int index, Vector3d &p, Vector3d &N)
{

    const Vector3d sphere_center = sphere_centers[index];
    const double sphere_radius = sphere_radii[index];
    
    // Step 1: set up temporary t
    double t = -1;

    // Step 2: solve quadratic eqn
    double a = ray_direction.dot(ray_direction);
    double b = 2*ray_origin.dot(ray_direction) - 2*ray_direction.dot(sphere_center);
    double c = (sphere_center-ray_origin).dot(sphere_center-ray_origin) - sphere_radius * sphere_radius;

    double delta = b * b - 4 * a * c;

    // Step 3: check to see if the intersection point hit anything
    if (delta < 0)
    {
        return t;
    }
    else
    {
        // Step 4: solve for t
        if (((-b - sqrt(delta)) / (2 * a)) >= 0) {
            t = (-b - sqrt(delta)) / (2 * a);
        } else {
            t = (-b + sqrt(delta)) / (2 * a);
        }

        
        // Step 5: solve p(t) = o + td
        
        p(0) = ray_origin(0) + t * ray_direction(0);
        p(1) = ray_origin(1) + t * ray_direction(1);
        p(2) = ray_origin(2) + t * ray_direction(2);

        // Step 6: solve for normal
        N = (p - sphere_center).normalized();

        // Step 7: return t
        return t;
    }

    return -1;
}

//Compute the intersection between a ray and a paralleogram, return -1 if no intersection
double ray_parallelogram_intersection(const Vector3d &ray_origin, const Vector3d &ray_direction, int index, Vector3d &p, Vector3d &N)
{

    const Vector3d pgram_origin = parallelograms[index].col(0);
    const Vector3d A = parallelograms[index].col(1);
    const Vector3d B = parallelograms[index].col(2);
    const Vector3d pgram_u = A - pgram_origin;
    const Vector3d pgram_v = B - pgram_origin;

    double t = -1;

    // Check if the ray intersects with the parallelogram
    Matrix3d C;
    C.col(0) = -pgram_u;
    C.col(1) = -pgram_v;
    C.col(2) = ray_direction;

    Vector3d b = pgram_origin - ray_origin;
    Vector3d k = C.inverse() * b;
    
    if (k(2) >= 0 && k(0) <= 1 && k(0) >= 0 && k(1) >= 0 && k(1) <= 1)
    {
        // Compute intersection point
        p = ray_origin + (k(2) * ray_direction);

        // Compute normal at the intersection point
        N = (pgram_v.cross(pgram_u)).normalized();
        return k(2);
    }
    return -1;
}

/*
// returns a list of t's
std::vector<int> tree_traversal(const Vector3d &ray_origin, const Vector3d &ray_direction) {
    AABBTree::Node cur = bvh.nodes[0];
    if ()
    {
        if (ray_box_intersection(ray_origin, ray_direction, bvh.nodes[i].bbox));
        {
            return true;
        }
        if (bvh.root == NULL) {
            ;
        } else if (bvh.nodes[i].left == NULL) {
            if (bvh.nodes[i].right == NULL) {
                ;
            }
            
        } else if 
    }
}
*/

// Finds the closest intersecting object returns its index
// In case of intersection it writes into p and N (intersection point and normals)
bool find_nearest_object(const Vector3d &ray_origin, const Vector3d &ray_direction, Vector3d &p, Vector3d &N)
{
    Vector3d tmp_p, tmp_N;

    // TODO
    // Method (1): Traverse every triangle and return the closest hit.
    
    int closest_index = -1;
    double closest_t = std::numeric_limits<double>::max(); //closest t is "+ infinity"

    /*--- dodeca/bunny --*/
    for (int i = 0; i < facets.rows(); ++i)
    {
        Vector3d a = vertices.row(facets.row(i).transpose()[0]).transpose();
        Vector3d b = vertices.row(facets.row(i).transpose()[1]).transpose();
        Vector3d c = vertices.row(facets.row(i).transpose()[2]).transpose();


        //returns t and writes on tmp_p and tmp_N
        const double t = ray_triangle_intersection(ray_origin, ray_direction, a, b, c, tmp_p, tmp_N);
        // std::cout << "after t is assigned" << std::endl;

        //We have intersection
        if (t >= 0)
        {
            //The point is before our current closest t
            if (t < closest_t)
            {
                closest_index = i;
                closest_t = t;
                p = tmp_p;
                N = -tmp_N;
            }
        }

    }

    /*--- sphere ---*/
    for (int i = 0; i < sphere_centers.size(); ++i)
    {
        //returns t and writes on tmp_p and tmp_N
        const double t = ray_sphere_intersection(ray_origin, ray_direction, i, tmp_p, tmp_N);
        //We have intersection
        if (t >= 0)
        {
            //The point is before our current closest t
            if (t < closest_t)
            {
                closest_index = i;
                closest_t = t;
                p = tmp_p;
                N = tmp_N;
            }
        }
    }

    /*--- parallelogram ---*/
    for (int i = 0; i < parallelograms.size(); ++i)
    {
        //returns t and writes on tmp_p and tmp_N
        const double t = ray_parallelogram_intersection(ray_origin, ray_direction, i, tmp_p, tmp_N);
        //We have intersection
        if (t >= 0)
        {
            //The point is before our current closest t
            if (t < closest_t)
            {
                closest_index = sphere_centers.size() + i;
                closest_t = t;
                p = tmp_p;
                N = tmp_N;
            }
        }
    }
/*
    // Method (2): Traverse the BVH tree and test the intersection with
    //             triangles at the leaf nodes that intersect the input ray.
    std::vector<int> list_of_t = tree_traversal(ray_origin, ray_direction);
 
    for (int i = 0; i < list_of_t.size(); ++i)
    {
        Vector3d a = vertices.row(facets.row(i).transpose()[0]).transpose();
        Vector3d b = vertices.row(facets.row(i).transpose()[1]).transpose();
        Vector3d c = vertices.row(facets.row(i).transpose()[2]).transpose();


        //returns t and writes on tmp_p and tmp_N
        const double t = ray_triangle_intersection(ray_origin, ray_direction, a, b, c, tmp_p, tmp_N);
        // std::cout << "after t is assigned" << std::endl;

        //We have intersection
        if (t >= 0)
        {
            //The point is before our current closest t
            if (t < closest_t)
            {
                closest_index = i;
                closest_t = t;
                p = tmp_p;
                N = tmp_N;
            }
        }

    }
*/

    if (closest_index > -1) {
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
// Raytracer code
////////////////////////////////////////////////////////////////////////////////
double calculate_distance(const Vector3d &origin, const Vector3d &point) {
    return sqrt((point(0)-origin(0))*(point(0)-origin(0))
            +(point(1)-origin(1))*(point(1)-origin(1))
            +(point(2)-origin(2))*(point(2)-origin(2)));
}

//Checks if the light is visible
bool is_light_visible(const Vector3d &ray_origin, const Vector3d &ray_direction, const Vector3d &light_position)
{
    // cast a ray from each point to the light
    //   if you intersect with something before reaching the light, then it is a shadow area
    //   and light should not contribute to its colour
    Vector3d p, N;
    Vector3d temp;

    // Find out if the ray is reaching the light
    bool closest = find_nearest_object(ray_origin + (1e-6) * ray_direction.normalized(), ray_direction, p, N);

    int p_dist = (ray_origin-p).norm();
    int light_dist = (ray_origin-light_position).norm();

    if (closest == true && p_dist < light_dist) {
        return false;       // light is hidden
    }
    return true;            // light is visible
}

Vector4d shoot_ray(const Vector3d &ray_origin, const Vector3d &ray_direction, int max_bounce)
{
    // Intersection point and normal, these are output of find_nearest_object
    Vector3d p, N;
    Vector4d spec_color;
    Vector4d reflection_color(0, 0, 0, 0);
    Vector4d refraction_color(0, 0, 0, 0);


    const bool nearest_object = find_nearest_object(ray_origin, ray_direction, p, N);

    if (!nearest_object)
    {
        // Return a transparent color
        return Vector4d(0, 0, 0, 0);
    }

    // Ambient light contribution
    const Vector4d ambient_color = obj_ambient_color.array() * ambient_light.array();

    // Punctual lights contribution (direct lighting)
    Vector4d lights_color(0, 0, 0, 0);
    for (int i = 0; i < light_positions.size(); ++i)
    {
        const Vector3d &light_position = light_positions[i];
        const Vector4d &light_color = light_colors[i];

        const Vector3d Li = (light_position - p).normalized();
        const Vector3d Vi = (camera_position - p).normalized();
        const Vector3d Hi = (Vi + Li).normalized();
        
        if (is_light_visible(p,Li,light_position) == false){
           continue;
        } else {
            Vector4d diff_color = obj_diffuse_color;
            Vector4d spec_color = obj_specular_color;

            // Diffuse contribution
            const Vector4d diffuse = diff_color * std::max(Li.dot(N), 0.0);

            // Specular contribution, use obj_specular_color

            const Vector4d specular = spec_color * pow(std::max(Hi.dot(N),0.0),obj_specular_exponent);

            // Attenuate lights according to the squared distance to the lights
            const Vector3d D = light_position - p;
            lights_color += (diffuse + specular).cwiseProduct(light_color) / D.squaredNorm();
        }

    }
    
    Vector4d refl_color = obj_reflection_color;
    
    Vector4d black_space(0, 0, 0, 0);

    if (refl_color != black_space && max_bounce >= 0){
        max_bounce = max_bounce - 1;
        const Vector3d refl_origin = p;
        const Vector3d refl_direction = ray_direction - 2 * (ray_direction.dot(N))* N;
        reflection_color = (shoot_ray(refl_origin + (1e-6) * refl_direction, refl_direction.normalized(), max_bounce));
        reflection_color(0) *= refl_color(0);
        reflection_color(1) *= refl_color(1);
        reflection_color(2) *= refl_color(2);
        reflection_color(3) *= refl_color(3);

    } 

    // Rendering equation
    Vector4d C = ambient_color + lights_color + reflection_color + refraction_color;

    //Set alpha to 1
    C(3) = 1;

    return C;
}

////////////////////////////////////////////////////////////////////////////////

void raytrace_scene()
{
    std::cout << "Simple ray tracer." << std::endl;

    int w = 640;
    int h = 480;
    MatrixXd R = MatrixXd::Zero(w, h);
    MatrixXd G = MatrixXd::Zero(w, h);
    MatrixXd B = MatrixXd::Zero(w, h);
    MatrixXd A = MatrixXd::Zero(w, h); // Store the alpha mask

    // The camera always points in the direction -z
    // The sensor grid is at a distance 'focal_length' from the camera center,
    // and covers an viewing angle given by 'field_of_view'.
    double aspect_ratio = double(w) / double(h);
    // TODO
    double image_y = focal_length * tan(field_of_view / 2);
    double image_x = image_y * aspect_ratio;

    // The pixel grid through which we shoot rays is at a distance 'focal_length'
    const Vector3d image_origin(-image_x, image_y, camera_position[2] - focal_length);
    const Vector3d x_displacement(2.0 / w * image_x, 0, 0);
    const Vector3d y_displacement(0, -2.0 / h * image_y, 0);

    for (unsigned i = 0; i < w; ++i)
    {
        for (unsigned j = 0; j < h; ++j)
        {
            const Vector3d pixel_center = image_origin + (i + 0.5) * x_displacement + (j + 0.5) * y_displacement;

            // Prepare the ray
            Vector3d ray_origin;
            Vector3d ray_direction;

            if (is_perspective)
            {
                // Perspective camera
                ray_origin = camera_position;
                ray_direction = (pixel_center - camera_position).normalized();
            }
            else
            {
                // Orthographic camera
                ray_origin = pixel_center;
                ray_direction = Vector3d(0, 0, -1);
            }

            const Vector4d C = shoot_ray(ray_origin, ray_direction, max_bounce);
            R(i, j) = C(0);
            G(i, j) = C(1);
            B(i, j) = C(2);
            A(i, j) = C(3);
        }
    }

    // Save to png
    write_matrix_to_png(R, G, B, A, filename);
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    setup_scene();

    raytrace_scene();
    return 0;
}
