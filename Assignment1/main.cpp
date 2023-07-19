#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

float angle_to_rad(float angle) {
    return angle / 180.0f * MY_PI;
}

Eigen::Matrix4f get_rotation(Eigen::Vector3f axis, float angle) {
    float rad = angle_to_rad(angle);

    // This will fail at compile time -> adjoint is 'conjugate transpose'(共轭转置)
    // so adjoint is not adjugate(伴随), and also adjugate is called 'classical adjoint'
    // Eigen::Matrix3f axis_adj = axis.adjoint();
    // dont know how to achieve this in eigen, so ...

    Eigen::Matrix3f axis_adjugate;
    axis_adjugate <<
        0, -axis[2], axis[1],
        axis[2], 0, -axis[0],
        -axis[1], axis[0], 0;

    // Rodrigues' rotation formula
    Eigen::Matrix3f rotation;
    rotation =
        std::cos(rad) * Eigen::Matrix3f::Identity()
        + (1 - std::cos(rad)) * axis * axis.transpose()
        + std::sin(rad) * axis_adjugate;

    //Eigen::AngleAxisf rot(angle, axis);
    // should be the same with rotation

    // 3*3 transform -> 4*4 Matrix(Affine)
    Eigen::Affine3f rotate_affine;
    rotate_affine = rotation;
    Eigen::Matrix4f ret_rotation = rotate_affine.matrix();

    return ret_rotation;
}

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    
    // Solution 1
    //float rad = angle_to_rad(rotation_angle);
    //Eigen::Matrix4f rotate;
    //rotate <<
    //    std::cos(rad), -std::sin(rad), 0, 0,
    //    std::sin(rad), std::cos(rad), 0, 0,
    //    0, 0, 1, 0,
    //    0, 0, 0, 1;

    // Solution 2
    Eigen::Matrix4f rotate_0 = get_rotation(Eigen::Vector3f::UnitZ(), rotation_angle);
    return rotate_0;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    Eigen::Matrix4f persp_to_ortho;
    persp_to_ortho <<
        zNear, 0, 0, 0,
        0, zNear, 0, 0,
        0, 0, zNear + zFar, -zNear * zFar,
        0, 0, 1, 0;

    Eigen::Matrix4f translate;
    translate <<
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, -(zNear + zFar) / 2,
        0, 0, 0, 1;

    float height = std::tan(eye_fov / 2) * zNear * 2;
    float width = aspect_ratio * height;

    Eigen::Matrix4f scale;
    scale <<
        2 / width, 0, 0, 0,
        0, 2 / height, 0, 0,
        0, 0, 2 / (zFar - zNear), 0,
        0, 0, 0, 1;

    return scale * translate * persp_to_ortho * projection;
}

int main(int argc, const char** argv)
{
    float angle = 20;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
