#include "CGL/CGL.h"
#include "CGL/viewer.h"

#include "application.h"
typedef uint32_t gid_t;

#include <iostream>
//#include <unistd.h>

using namespace std;
using namespace CGL;

void usage(const char *binaryName) {
  printf("Usage: %s [options] <scenefile>\n", binaryName);
  printf("Program Options:\n");
  printf("  -m  <FLOAT>            Mass per node\n");
  printf("  -g  <FLOAT> <FLOAT>    Gravity vector (x, y)\n");
  printf("  -s  <INT>              Number of steps per simulation frame\n");
  printf("\n");
}

int main(int argc, char **argv) {
  AppConfig config;
  int opt;

  std::vector<std::string> args(argv, argv + argc);

  for (size_t i = 1; i < args.size(); ++i) {
      if (args[i] == "-m") {
          config.mass = std::stof(args[++i]);
      }
      else if (args[i] == "-g") {
          config.gravity = Vector2D(std::stof(args[++i]), std::stof(args[++i]));
      }
      else if (args[i] == "-s") {
          config.steps_per_frame = std::stoi(args[++i]);
      }
      else if (args[i] == "-n") {
          config.node_count = std::stoi(args[++i]);
      }
      else {
          usage(argv[0]);
          return 1;
      }
  }

  // create application
  Application *app = new Application(config);

  // create viewer
  Viewer viewer = Viewer();

  // set renderer
  viewer.set_renderer(app);

  // init viewer
  viewer.init();

  // start viewer
  viewer.start();

  return 0;
}
