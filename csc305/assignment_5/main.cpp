// C++ include
#include <iostream>
#include <string>
#include <vector>
#include <limits>


// Utilities for the Assignment
#include "raster.h"

#include <gif.h>
#include <fstream>

#include <Eigen/Geometry>
// Image writing library
#define STB_IMAGE_WRITE_IMPLEMENTATION // Do not include this line twice in your project!
#include "stb_image_write.h"

using namespace std;
using namespace Eigen;

//Image height
const int H = 480;

//Camera settings
const float near_plane = 1.5; // AKA focal length
const float far_plane = near_plane * 100;
const float field_of_view = 0.7854; // 45 degrees
const float aspect_ratio = 1.5;
const bool is_perspective = false;
const Vector3f camera_position(0, 0, 3);
const Vector3f camera_gaze(0, 0, -1);
const Vector3f camera_top(0, 1, 0);

//Object
const std::string data_dir = DATA_DIR;
const std::string mesh_filename(data_dir + "bunny.off");
MatrixXf vertices; // n x 3 matrix (n points)
MatrixXi facets;   // m x 3 matrix (m triangles)

//Material for the object
const Vector3f obj_diffuse_color(0.5, 0.5, 0.5);
const Vector3f obj_specular_color(0.2, 0.2, 0.2);
const float obj_specular_exponent = 256.0;

//Lights
std::vector<Vector3f> light_positions;
std::vector<Vector3f> light_colors;
//Ambient light
const Vector3f ambient_light(0.3, 0.3, 0.3);

//Fills the different arrays
void setup_scene()
{
    //Loads file
    std::ifstream in(mesh_filename);
    if (!in.good())
    {
        std::cerr << "Invalid file " << mesh_filename << std::endl;
        exit(1);
    }
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

    //Lights
    light_positions.emplace_back(8, 8, 0);
    light_colors.emplace_back(16, 16, 16);

    light_positions.emplace_back(6, -8, 0);
    light_colors.emplace_back(16, 16, 16);

    light_positions.emplace_back(4, 8, 0);
    light_colors.emplace_back(16, 16, 16);

    light_positions.emplace_back(2, -8, 0);
    light_colors.emplace_back(16, 16, 16);

    light_positions.emplace_back(0, 8, 0);
    light_colors.emplace_back(16, 16, 16);

    light_positions.emplace_back(-2, -8, 0);
    light_colors.emplace_back(16, 16, 16);

    light_positions.emplace_back(-4, 8, 0);
    light_colors.emplace_back(16, 16, 16);
}

void build_uniform(UniformAttributes &uniform)
{
    // Setup uniform
    uniform.projection << 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;

    uniform.transformation << 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;

    // Setup camera, compute w, u, v
    Vector3f w = -camera_gaze.normalized();
    Vector3f u = (camera_top.cross(w)).normalized();
    Vector3f v = w.cross(u);

    // Compute the camera transformation
    Matrix4f cam;
    cam << u(0), v(0), w(0), camera_position(0),
        u(1), v(1), w(1), camera_position(1),
        u(2), v(2), w(2), camera_position(2),
        0, 0, 0, 1;

    uniform.camera = cam.inverse();

    // Planes
    float near = -near_plane;
    float far = -far_plane;
    float top_plane = -near * tan(field_of_view/2);
    float left_plane = -top_plane * aspect_ratio; 
    float right_plane = -left_plane; 
    float bottom_plane = -top_plane;
    
    // Orthographic camera
    // left and right
    uniform.projection(0,0) = 2/(right_plane - left_plane);
    uniform.projection(0,3) = - (right_plane + left_plane) / (right_plane - left_plane);

    // top and bottom
    uniform.projection(1,1) = 2/(top_plane - bottom_plane);
    uniform.projection(1,3) = - (top_plane + bottom_plane) / (top_plane - bottom_plane);

    // near and far
    uniform.projection(2,2) = 2/(near - far);
    uniform.projection(2,3) = - (near + far) / (near - far);

    // bottom corner of matrix
    uniform.projection(3,3) = 1;

    if (is_perspective) {
        Matrix4f P;
        P << near, 0, 0, 0,
            0, near, 0, 0,
            0, 0, near + far, - far * near,
            0, 0, 1, 0;
                
        uniform.projection *= P;
    }
    
}

void simple_render(Eigen::Matrix<FrameBufferAttributes, Eigen::Dynamic, Eigen::Dynamic> &frameBuffer)
{
    UniformAttributes uniform;
    build_uniform(uniform);
    Program program;

    // set up vertex attributes
    std::vector<VertexAttributes> vertex_attributes;
    for (int i = 0; i < facets.rows(); ++i)
    {
        Vector3f a = vertices.row(facets.row(i).transpose()[0]).transpose();
        Vector3f b = vertices.row(facets.row(i).transpose()[1]).transpose();
        Vector3f c = vertices.row(facets.row(i).transpose()[2]).transpose();

        vertex_attributes.push_back(VertexAttributes(a[0], a[1], a[2]));
        vertex_attributes.push_back(VertexAttributes(b[0], b[1], b[2]));
        vertex_attributes.push_back(VertexAttributes(c[0], c[1], c[2]));
    }
    
    // set up shaders
    program.VertexShader = [](const VertexAttributes &va, const UniformAttributes &uniform) {
        VertexAttributes out;
        out.position = uniform.projection * uniform.camera * va.position;
        return out;
    };

    program.FragmentShader = [](const VertexAttributes &va, const UniformAttributes &uniform) {
        return FragmentAttributes(1, 0, 0);
    };

    program.BlendingShader = [](const FragmentAttributes &fa, const FrameBufferAttributes &previous) {
        return FrameBufferAttributes(fa.color[0] * 255, fa.color[1] * 255, fa.color[2] * 255, fa.color[3] * 255);
    };
    
    // rasterize triangles
    rasterize_triangles(program, uniform, vertex_attributes, frameBuffer);
}

Matrix4f compute_rotation(const float alpha)
{
    Matrix4f res;
    // Y-axis rotation matrix
    res << cos(alpha), 0, sin(alpha), 0,
        0, 1, 0, 0,
        -sin(alpha), 0, cos(alpha), 0,
        0, 0, 0, 1;
    return res;
}

void wireframe_render(const float alpha, Eigen::Matrix<FrameBufferAttributes, Eigen::Dynamic, Eigen::Dynamic> &frameBuffer)
{
    UniformAttributes uniform;
    build_uniform(uniform);
    Program program;

    Matrix4f trafo = compute_rotation(alpha);

    // set up shaders
    program.VertexShader = [trafo](const VertexAttributes &va, const UniformAttributes &uniform) {
        VertexAttributes out;
        Matrix4f transf = trafo * uniform.transformation;
        out.position = uniform.projection * uniform.camera * trafo *  va.position;
        return out;
    };

    program.FragmentShader = [](const VertexAttributes &va, const UniformAttributes &uniform) {
        return FragmentAttributes(1, 0, 0);
    };

    program.BlendingShader = [](const FragmentAttributes &fa, const FrameBufferAttributes &previous) {
        return FrameBufferAttributes(fa.color[0] * 255, fa.color[1] * 255, fa.color[2] * 255, fa.color[3] * 255);
    };

    std::vector<VertexAttributes> vertex_attributes;

    // Generate the vertex attributes for the edges and rasterize the lines
    for (int i = 0; i < facets.rows(); ++i)
    {
        Vector3f a = vertices.row(facets.row(i).transpose()[0]).transpose();
        Vector3f b = vertices.row(facets.row(i).transpose()[1]).transpose();
        Vector3f c = vertices.row(facets.row(i).transpose()[2]).transpose();

        vertex_attributes.push_back(VertexAttributes(a[0], a[1], a[2]));
        vertex_attributes.push_back(VertexAttributes(b[0], b[1], b[2]));
        vertex_attributes.push_back(VertexAttributes(b[0], b[1], b[2]));
        vertex_attributes.push_back(VertexAttributes(c[0], c[1], c[2]));
        vertex_attributes.push_back(VertexAttributes(c[0], c[1], c[2]));
        vertex_attributes.push_back(VertexAttributes(a[0], a[1], a[2]));
    }

    // rasterize wireframe lines
    rasterize_lines(program, uniform, vertex_attributes, 0.5, frameBuffer);

}

void get_shading_program(const float alpha, Program &program)
{
    Eigen::Matrix4f trafo = compute_rotation(alpha);

    // set up shaders
    program.VertexShader = [trafo](const VertexAttributes &va, const UniformAttributes &uniform) {

        VertexAttributes out;
        Matrix4f transf = trafo * uniform.transformation;
        out.position = uniform.projection * uniform.camera * trafo * va.position;
        out.normal = uniform.camera * va.normal;
        
        Vector3f all_lights_color(0, 0, 0);
        Vector4f light_position(0, 0, 0, 0);
        Vector4f camera(camera_position(0), camera_position(1), camera_position(2), 1);

        for (int i = 0; i < light_positions.size(); ++i)
        {
            light_position << light_positions[i](0), light_positions[i](1), light_positions[i](2), 1;

            const Vector4f Li = (light_position - out.position).normalized();
            const Vector4f Vi = (camera - out.position).normalized();
            const Vector4f Hi = (Vi + Li).normalized();

            
            Vector3f diff_color = obj_diffuse_color;
            Vector3f spec_color = obj_specular_color;

            // Diffuse contribution
            const Vector3f diffuse = diff_color * std::max(Li.dot(out.normal), 0.0f);

            // Specular contribution, use obj_specular_color
            const Vector3f specular = spec_color * pow(std::max(Hi.dot(out.normal),0.0f),obj_specular_exponent);

            // Attenuate lights according to the squared distance to the lights
            const Vector4f D = light_position - out.position;

            // Add the colors
            all_lights_color += (diffuse + specular).cwiseProduct(light_colors[i]) / D.squaredNorm();
        }

        out.color = all_lights_color + ambient_light;
        return out;
    };

    program.FragmentShader = [](const VertexAttributes &va, const UniformAttributes &uniform) {
        FragmentAttributes out;
        out.color << va.color[0], va.color[1], va.color[2], 1;
        out.position = va.position;
        return out;
    };

    program.BlendingShader = [](const FragmentAttributes &fa, const FrameBufferAttributes &previous) {
        Vector4f camera(camera_position(0), camera_position(1), camera_position(2), 1);
        if ((camera - fa.position).norm() < previous.depth)
        {
            FrameBufferAttributes out(fa.color[0] * 255, fa.color[1] * 255, fa.color[2] * 255, fa.color[3] * 255);
            out.depth = (camera - fa.position).norm();
            return out;
        }
        else
            return previous;
    };
}

void flat_shading(const float alpha, Eigen::Matrix<FrameBufferAttributes, Eigen::Dynamic, Eigen::Dynamic> &frameBuffer)
{
    UniformAttributes uniform;
    build_uniform(uniform);
    Program program;
    get_shading_program(alpha, program);

    // set up vertex attributes
    std::vector<VertexAttributes> vertex_attributes;
    for (int i = 0; i < facets.rows(); ++i)
    {
        Vector3f a = vertices.row(facets.row(i).transpose()[0]).transpose();
        Vector3f b = vertices.row(facets.row(i).transpose()[1]).transpose();
        Vector3f c = vertices.row(facets.row(i).transpose()[2]).transpose();

        Vector3f norm = ((b - a).cross(c - a)).normalized();


        VertexAttributes vert_a(a(0), a(1), a(2));
        vert_a.normal << norm(0), norm(1), norm(2), 0;
        vertex_attributes.push_back(vert_a);

        VertexAttributes vert_b(b(0), b(1), b(2));
        vert_b.normal << norm(0), norm(1), norm(2), 0;
        vertex_attributes.push_back(vert_b);

        VertexAttributes vert_c(c(0), c(1), c(2));
        vert_c.normal << norm(0), norm(1), norm(2), 0;
        vertex_attributes.push_back(vert_c);
    }

    // rasterize flat triangles
    rasterize_triangles(program, uniform, vertex_attributes, frameBuffer);
}

void pv_shading(const float alpha, Eigen::Matrix<FrameBufferAttributes, Eigen::Dynamic, Eigen::Dynamic> &frameBuffer)
{
    UniformAttributes uniform;
    build_uniform(uniform);
    Program program;
    get_shading_program(alpha, program);

    std::vector<VertexAttributes> vertex_attributes;
    MatrixXf normals;

    normals.resize(vertices.rows(),4);
    normals.setZero();

    // Add the normals and sum their surrounding verts
    for (int j = 0; j < facets.rows(); ++j)
    {
        Vector3f a = vertices.row(facets.row(j).transpose()[0]).transpose();
        Vector3f b = vertices.row(facets.row(j).transpose()[1]).transpose();
        Vector3f c = vertices.row(facets.row(j).transpose()[2]).transpose();

        Vector3f norm = ((b - a).cross(c - a));
        
        normals.row(facets.row(j).transpose()[0])[0] += norm(0);
        normals.row(facets.row(j).transpose()[0])[1] += norm(1);
        normals.row(facets.row(j).transpose()[0])[2] += norm(2);
        normals.row(facets.row(j).transpose()[0])[3] += 1;

        normals.row(facets.row(j).transpose()[1])[0] += norm(0);
        normals.row(facets.row(j).transpose()[1])[1] += norm(1);
        normals.row(facets.row(j).transpose()[1])[2] += norm(2);
        normals.row(facets.row(j).transpose()[1])[3] += 1;

        normals.row(facets.row(j).transpose()[2])[0] += norm(0);
        normals.row(facets.row(j).transpose()[2])[1] += norm(1);
        normals.row(facets.row(j).transpose()[2])[2] += norm(2);
        normals.row(facets.row(j).transpose()[2])[3] += 1;
    }

    // Average the vert normals & push them to the list
    for (int i = 0; i < facets.rows(); ++i)
    {
        
        Vector3f a = vertices.row(facets.row(i).transpose()[0]).transpose();
        Vector3f b = vertices.row(facets.row(i).transpose()[1]).transpose();
        Vector3f c = vertices.row(facets.row(i).transpose()[2]).transpose();

        Vector4f norm_a = normals.row(facets.row(i).transpose()[0]);
        Vector4f norm_b = normals.row(facets.row(i).transpose()[1]);
        Vector4f norm_c = normals.row(facets.row(i).transpose()[2]);

        norm_a(0) /= normals.row(facets.row(i).transpose()[0])[3];
        norm_a(1) /= normals.row(facets.row(i).transpose()[0])[3];
        norm_a(2) /= normals.row(facets.row(i).transpose()[0])[3];
        norm_a(3) = 0;

        norm_b(0) /= normals.row(facets.row(i).transpose()[1])[3];
        norm_b(1) /= normals.row(facets.row(i).transpose()[1])[3];
        norm_b(2) /= normals.row(facets.row(i).transpose()[1])[3];
        norm_b(3) = 0;

        norm_c(0) /= normals.row(facets.row(i).transpose()[2])[3];
        norm_c(1) /= normals.row(facets.row(i).transpose()[2])[3];
        norm_c(2) /= normals.row(facets.row(i).transpose()[2])[3];
        norm_c(3) = 0;

        VertexAttributes vert_a(a(0), a(1), a(2));
        vert_a.normal = norm_a.normalized();
        vertex_attributes.push_back(vert_a);

        VertexAttributes vert_b(b(0), b(1), b(2));
        vert_b.normal = norm_b.normalized();
        vertex_attributes.push_back(vert_b);

        VertexAttributes vert_c(c(0), c(1), c(2));
        vert_c.normal = norm_c.normalized();
        vertex_attributes.push_back(vert_c);
    }

    // rasterize smooth surface with averaged normals
    rasterize_triangles(program, uniform, vertex_attributes, frameBuffer);
}

int main(int argc, char *argv[])
{
    setup_scene();

    int W = H * aspect_ratio;
    Eigen::Matrix<FrameBufferAttributes, Eigen::Dynamic, Eigen::Dynamic> frameBuffer(W, H);
    vector<uint8_t> image;
    
    simple_render(frameBuffer);
    framebuffer_to_uint8(frameBuffer, image);
    frameBuffer.setConstant(FrameBufferAttributes(0, 0, 0));
    stbi_write_png("simple.png", frameBuffer.rows(), frameBuffer.cols(), 4, image.data(), frameBuffer.rows() * 4);
    
    wireframe_render(3.1415/2, frameBuffer);
    framebuffer_to_uint8(frameBuffer, image);
    frameBuffer.setConstant(FrameBufferAttributes(0, 0, 0));
    stbi_write_png("wireframe.png", frameBuffer.rows(), frameBuffer.cols(), 4, image.data(), frameBuffer.rows() * 4);
    
    flat_shading(0, frameBuffer);
    framebuffer_to_uint8(frameBuffer, image);
    frameBuffer.setConstant(FrameBufferAttributes(0, 0, 0));
    stbi_write_png("flat_shading.png", frameBuffer.rows(), frameBuffer.cols(), 4, image.data(), frameBuffer.rows() * 4);
    
    
    pv_shading(0, frameBuffer);
    framebuffer_to_uint8(frameBuffer, image);
    frameBuffer.setConstant(FrameBufferAttributes(0, 0, 0));
    stbi_write_png("pv_shading.png", frameBuffer.rows(), frameBuffer.cols(), 4, image.data(), frameBuffer.rows() * 4);
    
    // Animation
    //   for simplicity, please uncomment the file you want both for the filename and for the function call
    // const char *fileName = "wire_bunny.gif";
    const char *fileName = "flat_bunny.gif";
    // const char *fileName = "pv_bunny.gif";

    int delay = 25;
    GifWriter g;
    GifBegin(&g, fileName, frameBuffer.rows(), frameBuffer.cols(), delay);

    for (float i = 0; i < 4; i += 0.15)
    {
        frameBuffer.setConstant(FrameBufferAttributes(0, 0, 0));

        // wireframe_render(i*M_PI/2, frameBuffer);
        flat_shading(i*M_PI/2, frameBuffer);
        // pv_shading(i*M_PI/2, frameBuffer);

        framebuffer_to_uint8(frameBuffer, image);
        GifWriteFrame(&g, image.data(), frameBuffer.rows(), frameBuffer.cols(), delay);
    }
    GifEnd(&g);
    
    return 0;
}
