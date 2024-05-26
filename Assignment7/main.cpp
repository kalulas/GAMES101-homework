#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char** argv)
{
    int spp = 16;
    if (argc >= 2)
    {
        try
        {
            spp = std::stoi(std::string(argv[1]));
        }
        catch (const std::invalid_argument& ia)
        {
            std::cerr << "Invalid argument: " << ia.what() << '\n';
            return 1;
        }
        catch (const std::out_of_range& oor)
        {
            std::cerr << "Out of range: " << oor.what() << '\n';
            return 1;
        }
    }

	bool microfacet_bunny = argc >= 3 && std::string(argv[2]) == "microfacet";

    std::cout << "target SPP set to " << spp << std::endl;
    // Change the definition here to change resolution
    Scene scene(784, 784);

    Material* red = new Material(DIFFUSE, Vector3f(0.0f));
    red->Kd = Vector3f(0.63f, 0.065f, 0.05f);
    Material* green = new Material(DIFFUSE, Vector3f(0.0f));
    green->Kd = Vector3f(0.14f, 0.45f, 0.091f);
    Material* white = new Material(DIFFUSE, Vector3f(0.0f));
    white->Kd = Vector3f(0.725f, 0.71f, 0.68f);
    Material* light = new Material(DIFFUSE, (8.0f * Vector3f(0.747f+0.058f, 0.747f+0.258f, 0.747f) + 15.6f * Vector3f(0.740f+0.287f,0.740f+0.160f,0.740f) + 18.4f *Vector3f(0.737f+0.642f,0.737f+0.159f,0.737f)));
    light->Kd = Vector3f(0.65f);

    Material* white_microfacet = new Material(MICROFACET, Vector3f(0.0f));
    white_microfacet->Ks = Vector3f(0.725f, 0.71f, 0.68f);
    white_microfacet->Kd = Vector3f(0.725f, 0.71f, 0.68f);

    // VisualStudio execute it under ".\out\build\x64-Debug\", so ...
    MeshTriangle floor("../../../models/cornellbox/floor.obj", white);
    MeshTriangle shortbox("../../../models/cornellbox/shortbox.obj", white);
    MeshTriangle tallbox("../../../models/cornellbox/tallbox.obj", white);
    MeshTriangle left("../../../models/cornellbox/left.obj", red);
    MeshTriangle right("../../../models/cornellbox/right.obj", green);
    MeshTriangle light_("../../../models/cornellbox/light.obj", light);

    MeshTriangle bunny("../../../models/bunny/bunny.obj", white_microfacet,
        Vector3f{ 300.0f, 0.0f, 300.0f }, Vector3f{ 2000.0f, 2000.0f, 2000.0f });

    scene.Add(&floor);
    scene.Add(&left);
    scene.Add(&right);
    scene.Add(&light_);

    if (!microfacet_bunny)
    {
        scene.Add(&shortbox);
        scene.Add(&tallbox);
    }
    else {
        scene.Add(&bunny);
    }

    scene.buildBVH();

    Renderer r;

    auto start = std::chrono::system_clock::now();
    r.Render(scene, spp);
    auto stop = std::chrono::system_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";

    return 0;
}