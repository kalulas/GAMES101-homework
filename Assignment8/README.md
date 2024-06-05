# Assignment8

## Prerequisite

```bash
$ vcpkg install freetype
# 项目工程内置，不需要手动安装
# vcpkg install glfw3
# vcpkg install glad
```

### remove unistd.h

依赖安装完毕后首先需要解决 main.cpp 文件中对 **unistd.h** 的依赖，主要是用于处理执行命令参数，做一下简单处理即可去除对 unistd.h 的依赖，注释掉 #include 即可

```c++
#include <iostream>
//#include <unistd.h>

using namespace std;
using namespace CGL;


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
      else {
          usage(argv[0]);
          return 1;
      }
  }
```

### #define GLEW_STATIC

之后再次编译，可能遇到的一系列错误输出：

> LNK2019，无法解析的外部符号 __imp_glewInit，函数 "public: void cdecl CGL::Viewer::init(void)" (?init@Viewer@CGL@@QEAAXXZ) 中引用了该符号

原因是以静态库形式使用 **glew** 库，需要在 #include 语句前声明 GLEW_STATIC，尝试了一下在项目文件`\Assignment8\CGL\src\osdtext.h` 中的 `#include "GL/glew.h"` 语句前加入 `#define GLEW_STATIC` 即可，前后可用 ifndef 包裹一下

```c++
#ifndef CGL_TEXTOSD_H
#define CGL_TEXTOSD_H

#ifndef GLEW_STATIC
#define GLEW_STATIC

#include <string>
#include <vector>
#include "GL/glew.h"

// ...

#endif // GLEW_STATIC

#endif // CGL_TEXTOSD_H

```

上述问题解决后即可成功编译 ropesim.exe

## Modifications

### CGL\src\viewer.cpp

`void Viewer::init()`

调整窗口为启动后居中

### src\main.cpp & src\application.cpp

支持从启动参数中传入质点个数

### src\rope.cpp

实现质点、弹簧属性的设置，实现显式、半隐式 Euler，实现显式 Verlet

## Result

半隐式 Euler：质点数3，每帧仿真步长64

![ropesim-3nodes-64steps-only-euler](https://s2.loli.net/2024/06/05/lh3QbmCdAILDWow.gif)

半隐式 Euler：质点数16，每帧仿真步长64

![ropesim-16nodes-64steps-only-euler](https://s2.loli.net/2024/06/05/vcxkBm6HFJNWuTC.gif)

半隐式 Euler：质点数32，每帧仿真步长64

![ropesim-32nodes-64steps-only-euler](https://s2.loli.net/2024/06/05/RxKv2csJokSXQdh.gif)

半隐式 Euler & 显式 Verlet：质点数16，每帧仿真步长64

![ropesim-16nodes-64steps-euler-and-verlet](https://s2.loli.net/2024/06/05/QkF19HJ4DCiEWPc.gif)

半隐式 Euler & 显式 Verlet：质点数32，每帧仿真步长64

![ropesim-32nodes-64steps-euler-and-verlet](https://s2.loli.net/2024/06/05/3B1q5IdzViPTlYo.gif)
