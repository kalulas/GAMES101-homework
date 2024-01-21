// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

static bool inside_triangle(float x, float y, const Vector3f* _v)
{
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]

    float direction = 0.0f;
    for (int i = 0; i < 3; i++)
    {
        int next_idx = (i + 1) % 3;
        Vector3f va = Vector3f(x, y, _v[i].z()) - _v[i];
        Vector3f vb = _v[next_idx] - _v[i];
        Vector3f ret = va.cross(vb);
        if (i == 0)
        {
            direction = ret.z() / abs(ret.z());
        }
        else
        {
            float _dir = ret.z() / abs(ret.z());
            if (_dir * direction < 0)
            {
                // not the same direction
                return false;
            }
        }
    }

    return true;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

Eigen::Vector4f rst::rasterizer::get_bounding_box(std::array<Vector4f, 3> points)
{
    constexpr float infinity = std::numeric_limits<float>::infinity();
    constexpr float neg_infinity = -std::numeric_limits<float>::infinity();
    // [min_X, min_Y, max_X, max_Y]
    Eigen::Vector4f boundingBox = Eigen::Vector4f(infinity, infinity, neg_infinity, neg_infinity);

    for (auto& vec : points)
    {
        if (vec[0] < boundingBox[0]) boundingBox[0] = vec[0];
        if (vec[1] < boundingBox[1]) boundingBox[1] = vec[1];
        if (vec[0] > boundingBox[2]) boundingBox[2] = vec[0];
        if (vec[1] > boundingBox[3]) boundingBox[3] = vec[1];
    }

    return boundingBox;
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle

    // If so, use the following code to get the interpolated z value.
    //auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
    //float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
    //float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
    //z_interpolated *= w_reciprocal;

    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.

    auto v = t.toVector4();
    Eigen::Vector4f box = get_bounding_box(v);
    for (int y = box[1]; y <= box[3] && y < height; y++)
    {
        for (int x = box[0]; x <= box[2] && x < width; x++)
        {
            int insideCount = 0;
            Eigen::Matrix2f depth2x2;
            constexpr float infinity = std::numeric_limits<float>::infinity();
            depth2x2.setConstant(infinity);
            
            for (size_t i = 0; i < 4; i++)
            {
                float _y = y + 0.25f + (i / 2) * 0.5f;
                float _x = x + 0.25f + (i % 2) * 0.5f;

                if (!inside_triangle(_x, _y, t.v))
                {
                    continue;
                }

                insideCount++;
                auto [alpha, beta, gamma] = computeBarycentric2D(_x, _y, t.v);
                float w_reciprocal = 1.0 / (alpha + beta + gamma);
                float z_interpolated = alpha * v[0].z() + beta * v[1].z() + gamma * v[2].z();
                z_interpolated *= w_reciprocal;

                depth2x2(i) = z_interpolated;
            }
            
            if (insideCount == 0)
            {
                continue;
            }

            int depthCount = compare_super_sampling_depth(x, y, depth2x2);
            Matrix2f target = super_sampling_depth_buf[get_index(x, y)];
            if (depthCount == 0)
            {
                continue;
            }

            // notice: we use insideCount/sampleCount here instead of depthCount/sampleCount,
            // otherwise black edges will appear at the intersection of the two triangles.
            float factor = (float)insideCount / 4;
            Eigen::Vector3f color = t.getColor();
            Eigen::Vector3f scaledColor = color * factor;

            set_pixel(Vector3f(x, y, 0.0f), scaledColor);
            set_super_sampling_depth(x, y, depth2x2);
        }
    }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    //if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    //{
    //    std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    //}
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        constexpr float infinity = std::numeric_limits<float>::infinity();
        Eigen::Matrix2f infinity2x2;
        infinity2x2 << infinity, infinity, infinity, infinity;
        std::fill(super_sampling_depth_buf.begin(), super_sampling_depth_buf.end(), infinity2x2);
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    //depth_buf.resize(w * h);
    super_sampling_depth_buf.resize(w * h);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

int rst::rasterizer::compare_super_sampling_depth(int x, int y, Eigen::Matrix2f depth) {
    Eigen::Matrix2f target = super_sampling_depth_buf[get_index(x, y)];
    int lesserCount = 0;
    int matrixSize = target.size();
    for (size_t i = 0; i < matrixSize; i++)
    {
        if (depth(i) < target(i))
        {
            lesserCount++;
        }
    }

    return lesserCount;
}

void rst::rasterizer::set_super_sampling_depth(int x, int y, Eigen::Matrix2f depth) {
    Eigen::Matrix2f filteredDepth;
    int index = get_index(x, y);
    filteredDepth = super_sampling_depth_buf[index];

    constexpr float infinity = std::numeric_limits<float>::infinity();
    for (size_t i = 0; i < depth.size(); i++)
    {
        if (depth(i) < infinity)
        {
            filteredDepth(i) = depth(i);
        }
    }

    super_sampling_depth_buf[index] = filteredDepth;
    //super_sampling_depth_buf[get_index(x, y)] = depth;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on