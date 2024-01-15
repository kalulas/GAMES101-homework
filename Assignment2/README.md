# Assignment2

## Modifications

**main.cpp** 

Eigen::Matrix4f get_projection_matrix()

实现透视投影（来自作业1）

**raterizer.cpp** 

Eigen::Vector4f rst::rasterizer::get_bounding_box(std::array<Vector4f, 3> points)

求出三角形点集合 points 的二维包围盒，[x_min, y_min, x_max, y_max] 顺序

**raterizer.cpp** 

static bool insideTriangle(int x, int y, const Vector3f* _v)

判断点[x+0.5, y+0.5]是否在三角形 _v 内

**raterizer.cpp**

void rst::rasterizer::rasterize_triangle(const Triangle& t)

结合上述步骤，遍历二维包围盒，对落在三角形内的点进行深度值插值，若深度值结果比depth_buffer深度值离相机更近，使用该点的颜色设置像素。

## Result

### result1

![result1](https://s2.loli.net/2024/01/15/Kbz5igYWAweItmV.png)