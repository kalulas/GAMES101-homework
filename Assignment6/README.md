# Assignment6

## Modifications

### Renderer.cpp

`void Renderer::Render(const Scene& scene)`

根据已有的`ndc_x`，`ndc_y`，得到 ray 的入射方向 dir 并构建入射光线，用于调用 `Vector3f Scene::castRay(const Ray &ray, int depth)`

### Triangle.hpp

`Intersection Triangle::getIntersection(Ray ray)`

Möller–Trumbore 算法在框架中已基本完成，通过计算结果 `t_tmp`, `u`, `v` 构建 Intersection 实例返回即可

### Bounds3.hpp

`bool Bounds3::IntersectP(const Ray& ray, const Vector3f& invDir, const std::array<bool, 3>& dirIsPositive)`

完成判断包围盒 BoundingBox 与光线是否相交的算法

### BVH.cpp

`Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray)`

完成判断光线与BVH各节点，及其中内容是否相交的递归过程

## Result

输出结果

![result](./images/result.png)
