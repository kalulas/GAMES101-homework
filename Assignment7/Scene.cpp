//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

Vector3f Scene::shade(Intersection& hit_obj, Vector3f wo, std::mt19937& rng) const {
    if (hit_obj.m->hasEmission())
    {
        return hit_obj.m->getEmission();
    }

    const float epsilon = 0.0005f;
    // Contribution from the light source
    Vector3f L_dir;
    float light_pdf;
    Intersection hit_light;
	sampleLight(hit_light, light_pdf);
    Vector3f object2light = hit_light.coords - hit_obj.coords;
    Vector3f object2light_dir = normalize(object2light);

    // check if the light was blocked
	Ray block_check_ray(hit_obj.coords, object2light_dir);
    Intersection block_ret = intersect(block_check_ray);
    if (block_ret.distance - object2light.norm() > -epsilon)
    {
        Vector3f f_r = hit_obj.m->eval(object2light_dir, wo, hit_obj.normal);
        float sqr_distance = dotProduct(object2light, object2light);
        float cos_A = std::max(0.0f, dotProduct(hit_obj.normal, object2light_dir));
		float cos_B = std::max(0.0f, dotProduct(hit_light.normal, -object2light_dir));
		L_dir = hit_light.emit * f_r * cos_A * cos_B / sqr_distance / light_pdf;
    }

    // Contribution from other reflectors.
    Vector3f L_indir;
    std::uniform_real_distribution<float> dist(0.f, 1.f);
    float random_result = dist(rng);
    if (random_result < RussianRoulette)
    {
        Vector3f next_obj_dir = hit_obj.m->sample(wo, hit_obj.normal).normalized();
		float pdf = hit_obj.m->pdf(wo, next_obj_dir, hit_obj.normal);
        if (pdf > epsilon)
        {
			Intersection next_obj = intersect(Ray(hit_obj.coords, next_obj_dir));
            if (next_obj.happened && !next_obj.m->hasEmission())
            {
                Vector3f f_r = hit_obj.m->eval(next_obj_dir, wo, hit_obj.normal);
				float cos = std::max(0.0f, dotProduct(hit_obj.normal, next_obj_dir));
				L_indir = shade(next_obj, -next_obj_dir, rng) * f_r * cos / pdf / RussianRoulette;
            }
        }
    }

    return L_dir + L_indir;
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth, std::mt19937 &rng) const
{
    // TO DO Implement Path Tracing Algorithm here
    Intersection intersect_ret = intersect(ray);
    if (!intersect_ret.happened)
    {
        return Vector3f{};
    }

    return shade(intersect_ret, -ray.direction, rng);
}