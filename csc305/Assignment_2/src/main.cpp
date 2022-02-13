/*
 * Briana Johnson
 * CSC 305, Spring 2022
 */

// C++ include
#include <iostream>
#include <string>
#include <vector>

// Utilities for the Assignment
#include "utils.h"

// Image writing library
#define STB_IMAGE_WRITE_IMPLEMENTATION // Do not include this line twice in your project!
#include "stb_image_write.h"

// Shortcut to avoid Eigen:: everywhere, DO NOT USE IN .h
using namespace Eigen;

void raytrace_sphere()
{
    std::cout << "Simple ray tracer, one sphere with orthographic projection" << std::endl;

    const std::string filename("sphere_orthographic.png");  // decide where the final product goes
    MatrixXd C = MatrixXd::Zero(800, 800);                  // Store the color (C)
    MatrixXd A = MatrixXd::Zero(800, 800);                  // Store the alpha mask (A)

    const Vector3d camera_origin(0, 0, 3);                  // set the camera origin
    const Vector3d camera_view_direction(0, 0, -1);         // set the camera view direction

    // The camera is orthographic, pointing in the direction -z and covering the
    // unit square (-1,1) in x and y
    const Vector3d image_origin(-1, 1, 1);                  // change z to -z, set x to -1, set y to 1
    const Vector3d x_displacement(2.0 / C.cols(), 0, 0);    // setting up the colour grid for the camera
    const Vector3d y_displacement(0, -2.0 / C.rows(), 0);   // setting up the colour grid for the camera

    // Single light source
    const Vector3d light_position(-1, 1, 1);                // set up where our light source is

    for (unsigned i = 0; i < C.cols(); ++i)
    {
        for (unsigned j = 0; j < C.rows(); ++j)
        {
            // the pixel itself is at the image origin plus the offset of the outer loop * the colour + "" for inner loop
            const Vector3d pixel_center = image_origin + double(i) * x_displacement + double(j) * y_displacement;

            // Prepare the ray
            const Vector3d ray_origin = pixel_center;
            const Vector3d ray_direction = camera_view_direction;

            // Intersect with the sphere
            // NOTE: this is a special case of a sphere centered in the origin and for orthographic rays aligned with the z axis
            Vector2d ray_on_xy(ray_origin(0), ray_origin(1));
            const double sphere_radius = 0.9;

            if (ray_on_xy.norm() < sphere_radius)
            {
                // The ray hit the sphere, compute the exact intersection point
                Vector3d ray_intersection(
                    ray_on_xy(0), ray_on_xy(1),
                    sqrt(sphere_radius * sphere_radius - ray_on_xy.squaredNorm()));

                // Compute normal at the intersection point
                Vector3d ray_normal = ray_intersection.normalized();

                // Simple diffuse model
                C(i, j) = (light_position - ray_intersection).normalized().transpose() * ray_normal;

                // Clamp to zero
                C(i, j) = std::max(C(i, j), 0.);

                // Disable the alpha mask for this pixel
                A(i, j) = 1;
            }
        }
    }

    // Save to png
    write_matrix_to_png(C, C, C, A, filename);
}

void raytrace_parallelogram()
{
    std::cout << "Simple ray tracer, one parallelogram with orthographic projection" << std::endl;

    const std::string filename("plane_orthographic.png");
    MatrixXd C = MatrixXd::Zero(800, 800);                  // Store the color (C)
    MatrixXd A = MatrixXd::Zero(800, 800);                  // Store the alpha mask (A)

    const Vector3d camera_origin(0, 0, 3);                  // Set camera origin
    const Vector3d camera_view_direction(0, 0, -1);         // Set camera view dir

    // The camera is orthographic, pointing in the direction -z and covering the unit square (-1,1) in x and y
    const Vector3d image_origin(-1, 1, 1);
    const Vector3d x_displacement(2.0 / C.cols(), 0, 0);
    const Vector3d y_displacement(0, -2.0 / C.rows(), 0);

    // Parallelogram parameters    
    const Vector3d pgram_origin(-0.2, -0.4, 0);
    const Vector3d pgram_u(0, 0.31, -4);
    const Vector3d pgram_v(0.6, 0.28, 0);
    
    // Single light source
    const Vector3d light_position(-1, 1, 1);

    for (unsigned i = 0; i < C.cols(); ++i)
    {
        for (unsigned j = 0; j < C.rows(); ++j)
        {
            const Vector3d pixel_center = image_origin + double(i) * x_displacement + double(j) * y_displacement;

            // Prepare the ray
            const Vector3d ray_origin = pixel_center;
            const Vector3d ray_direction = camera_view_direction;

            // Check if the ray intersects with the parallelogram
            Matrix3d B;
            B.col(0) = -pgram_u;
            B.col(1) = -pgram_v;
            B.col(2) = ray_direction;

            Vector3d b = pgram_origin - ray_origin;
            Vector3d k = B.inverse() * b;

            if (k(2) >= 0 && k(0) <= 1 && k(0) >= 0 && k(1) >= 0 && k(1) <= 1)
            {
                // Compute intersection point
                Vector3d ray_intersection = ray_origin + (k(2) * ray_direction);

                // Compute normal at the intersection point
                Vector3d ray_normal = (pgram_v.cross(pgram_u)).normalized();

                // Simple diffuse model
                C(i, j) = (light_position - ray_intersection).normalized().transpose() * ray_normal;

                // Clamp to zero
                C(i, j) = std::max(C(i, j), 0.);

                // Disable the alpha mask for this pixel
                A(i, j) = 1;
            }
        }
    }

    // Save to png
    write_matrix_to_png(C, C, C, A, filename);
}

void raytrace_perspective()
{
    std::cout << "Simple ray tracer, one parallelogram with perspective projection" << std::endl;

    const std::string filename("plane_perspective.png");
    MatrixXd C = MatrixXd::Zero(800, 800); // Store the color
    MatrixXd A = MatrixXd::Zero(800, 800); // Store the alpha mask

    const Vector3d camera_origin(0, 0, 3);
    const Vector3d camera_view_direction(0, 0, -1);

    // The camera is perspective, pointing in the direction -z and covering the unit square (-1,1) in x and y
    const Vector3d image_origin(-1, 1, 1);
    const Vector3d x_displacement(2.0 / C.cols(), 0, 0);
    const Vector3d y_displacement(0, -2.0 / C.rows(), 0);

    // TODO: Parameters of the parallelogram (position of the lower-left corner + two sides)
    const Vector3d pgram_origin(-0.4, -0.8, 0);
    const Vector3d pgram_u(0, 0.9, -9);
    const Vector3d pgram_v(0.9, 0.3, 0);

    // Single light source
    const Vector3d light_position(-1, 1, 1);

    for (unsigned i = 0; i < C.cols(); ++i)
    {
        for (unsigned j = 0; j < C.rows(); ++j)
        {
            const Vector3d pixel_center = image_origin + double(i) * x_displacement + double(j) * y_displacement;

            // Prepare the ray (origin point and direction)
            const Vector3d ray_origin = pixel_center;            
            const Vector3d ray_direction = pixel_center - camera_origin;


            // Check if the ray intersects with the parallelogram
            Matrix3d B;
            B.col(0) = -pgram_u;
            B.col(1) = -pgram_v;
            B.col(2) = ray_direction;

            Vector3d b = pgram_origin - ray_origin;
            Vector3d k = B.inverse() * b;

            if (k(2) >= 0 && k(0) <= 1 && k(0) >= 0 && k(1) >= 0 && k(1) <= 1)
            {
                // Compute intersection point
                Vector3d ray_intersection = ray_origin + (k(2) * ray_direction);

                // Compute normal at the intersection point
                Vector3d ray_normal = (pgram_v.cross(pgram_u)).normalized();

                // Simple diffuse model
                C(i, j) = (light_position - ray_intersection).normalized().transpose() * ray_normal;

                // Clamp to zero
                C(i, j) = std::max(C(i, j), 0.);

                // Disable the alpha mask for this pixel
                A(i, j) = 1;
            }
        }
    }

    // Save to png
    write_matrix_to_png(C, C, C, A, filename);
}

void raytrace_shading()
{
    std::cout << "Simple ray tracer, one sphere with different shading" << std::endl;

    const std::string filename("shading.png");
    //MatrixXd C = MatrixXd::Zero(800, 800); // Store the color
    MatrixXd R = MatrixXd::Zero(800, 800); // Store the color
    MatrixXd G = MatrixXd::Zero(800, 800); // Store the color
    MatrixXd B = MatrixXd::Zero(800, 800); // Store the color
    MatrixXd A = MatrixXd::Zero(800, 800); // Store the alpha mask

    const Vector3d camera_origin(0, 0, 3);
    const Vector3d camera_view_direction(0, 0, -1);

    // The camera is perspective, pointing in the direction -z and covering the unit square (-1,1) in x and y
    const Vector3d image_origin(-1, 1, 1);
    const Vector3d x_displacement(2.0 / A.cols(), 0, 0);
    const Vector3d y_displacement(0, -2.0 / A.rows(), 0);

    // Sphere setup
    const Vector3d sphere_center(0, 0, 0);
    const double sphere_radius = 0.9;

    // Material params
    const Vector3d diffuse_color(4, 4, 0);
    const double specular_exponent = 2;
    const Vector3d specular_color(1, 0, 1);


    // Single light source
    const Vector3d light_position(-1, 1, 1);
    double ambient = 0;

    for (unsigned i = 0; i < R.cols(); ++i)
    {
        for (unsigned j = 0; j < R.rows(); ++j)
        {
            const Vector3d pixel_center = image_origin + double(i) * x_displacement + double(j) * y_displacement;

            // Prepare the ray
            const Vector3d ray_origin = pixel_center;
            const Vector3d ray_direction = camera_view_direction;

            // Intersect with the sphere
            // NOTE: this is a special case of a sphere centered in the origin and for orthographic rays aligned with the z axis
            Vector2d ray_on_xy(ray_origin(0), ray_origin(1));
            const double sphere_radius = 0.9;

            if (ray_on_xy.norm() < sphere_radius)
            {
                // The ray hit the sphere, compute the exact intersection point
                Vector3d ray_intersection(
                    ray_on_xy(0), ray_on_xy(1),
                    sqrt(sphere_radius * sphere_radius - ray_on_xy.squaredNorm()));

                // Compute normal at the intersection point
                Vector3d ray_normal = ray_intersection.normalized();

                // Add shading parameter here
                const double diffuse = (light_position - ray_intersection).normalized().dot(ray_normal);
                const double specular = pow((light_position - ray_intersection).normalized().dot(ray_normal),specular_exponent);

                // Shading
                R(i,j) = ambient + diffuse_color(0) * diffuse + specular_color(0) * specular;
                G(i,j) = ambient + diffuse_color(1) * diffuse + specular_color(1) * specular;
                B(i,j) = ambient + diffuse_color(2) * diffuse + specular_color(2) * specular;

                // Clamp to zero
                R(i, j) = std::max(R(i, j), 0.);
                G(i, j) = std::max(G(i, j), 0.);
                B(i, j) = std::max(B(i, j), 0.);

                // Disable the alpha mask for this pixel
                A(i, j) = 1;
            }
        }
    }

    // Save to png
    write_matrix_to_png(R, G, B, A, filename);
}

int main()
{
    raytrace_sphere();
    raytrace_parallelogram();
    raytrace_perspective();
    raytrace_shading();

    return 0;
}
