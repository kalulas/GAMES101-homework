//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include "Scene.hpp"
#include "Renderer.hpp"
#include <omp.h>
#include <chrono>


inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00016;

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene& scene, int spp)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);
    int m = 0;

    // change the spp value to change sample ammount
    std::cout << "render with SPP: " << spp << "\n";

    std::random_device dev;
    std::mt19937 rng(dev());

#pragma omp parallel for shared(framebuffer) private(m, rng)
    for (m = 0; m < scene.width * scene.height; m++)
    {
        uint32_t m_x = m % scene.width;
		uint32_t m_y = m / scene.height;
        // generate primary ray direction
        float x = (2 * (m_x + 0.5) / (float)scene.width - 1) *
            imageAspectRatio * scale;
        float y = (1 - 2 * (m_y + 0.5) / (float)scene.height) * scale;

        Vector3f dir = normalize(Vector3f(-x, y, 1));
        for (int k = 0; k < spp; k++) {
            framebuffer[m] += scene.castRay(Ray(eye_pos, dir), 0, rng) / spp;
        }
    }

    /*
    for (uint32_t j = 0; j < scene.height; ++j) {
        for (uint32_t i = 0; i < scene.width; ++i) {
            // generate primary ray direction
            float x = (2 * (i + 0.5) / (float)scene.width - 1) *
                      imageAspectRatio * scale;
            float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;

            Vector3f dir = normalize(Vector3f(-x, y, 1));
            for (int k = 0; k < spp; k++){
                framebuffer[m] += scene.castRay(Ray(eye_pos, dir), 0) / spp;  
            }
            m++;
        }
        UpdateProgress(j / (float)scene.height);
    }
    */

    UpdateProgress(1.f);

    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_c);

    // 构造文件名
    char filename[256];
    std::snprintf(filename, sizeof(filename), "output_%02d%02d%02d_width%d_height%d_spp%d.ppm",
        now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec,
        scene.width, scene.height, spp);

    // save framebuffer to file
    FILE* fp = fopen(filename, "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);
}
