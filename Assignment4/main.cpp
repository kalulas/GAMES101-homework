#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    // TODO: Implement de Casteljau's algorithm
    std::vector<cv::Point2f> prev(control_points);
    std::vector<cv::Point2f> next;
    do {
        next.clear();
        for (size_t i = 0; i < prev.size() - 1; i++)
        {
            next.push_back(prev[i] + t * (prev[i + 1] - prev[i]));
        }

        prev = next;
    } while (next.size() != 1);
    return next[0];

}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
    for (double t = 0.0; t < 1.0; t += 0.001)
    {
        auto point = recursive_bezier(control_points, t);
        int row = point.y;
        int col = point.x;
        window.at<cv::Vec3b>(row, col)[1] = 255;
    }

}

void window_draw_additional_with_check(cv::Mat& window, int row, int col, int channel, uchar value) {
    if (0 <= row && row < window.rows && 0 <= col && col < window.cols)
    {
        window.at<cv::Vec3b>(row, col)[channel] += value;
    }
}

void antialiasing_bezier(const std::vector<cv::Point2f>& control_points, cv::Mat& window)
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
    for (double t = 0.0; t < 1.0; t += 0.001)
    {
        auto point = recursive_bezier(control_points, t);
        int row = point.y;
        int col = point.x;
        float pixel_center_y = row + 0.5f;
        float pixel_center_x = col + 0.5f;

        float area_left = std::max(0.0f, pixel_center_x - point.x) * (1 - std::abs(point.y - pixel_center_y));
        float area_topleft = std::max(0.0f, pixel_center_x - point.x) * std::max(0.0f, point.y - pixel_center_y);
        float area_top = (1 - std::abs(point.x - pixel_center_x)) * std::max(0.0f, point.y - pixel_center_y);
        float area_topright = std::max(0.0f, point.x - pixel_center_x) * std::max(0.0f, point.y - pixel_center_y);

        float area_right = std::max(0.0f, point.x - pixel_center_x) * (1 - std::abs(point.y - pixel_center_y));
        float area_bottomright = std::max(0.0f, point.x - pixel_center_x) * std::max(0.0f, pixel_center_y - point.y);
        float area_bottom = (1 - std::abs(point.x - pixel_center_x)) * std::max(0.0f, pixel_center_y - point.y);
        float area_bottomleft = std::max(0.0f, pixel_center_x - point.x) * std::max(0.0f, pixel_center_y - point.y);

        float area_center = 1 - area_left - area_topleft - area_top - area_topright - area_right - area_bottomright - area_bottom - area_bottomleft;

        int channel = 1;
        window_draw_additional_with_check(window, row, col, channel, 255 * area_center);
        window_draw_additional_with_check(window, row, col - 1, channel, 255 * area_left);
        window_draw_additional_with_check(window, row + 1, col - 1, channel, 255 * area_topleft);
        window_draw_additional_with_check(window, row + 1, col, channel, 255 * area_top);
        window_draw_additional_with_check(window, row + 1, col + 1, channel, 255 * area_topright);
        window_draw_additional_with_check(window, row, col + 1, channel, 255 * area_right);
        window_draw_additional_with_check(window, row - 1, col + 1, channel, 255 * area_bottomright);
        window_draw_additional_with_check(window, row - 1, col, channel, 255 * area_bottom);
        window_draw_additional_with_check(window, row - 1, col - 1, channel, 255 * area_bottomleft);
    }

}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4) 
        {
            naive_bezier(control_points, window);
            bezier(control_points, window);
            //antialiasing_bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
