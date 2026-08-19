#include "doctest.h"

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "accel/brute_accel.h"
#include "accel/embree_accel.h"
#include "camera/pinhole.h"
#include "core/sampler.h"
#include "core/sampling.h"
#include "scene/scene_builder.h"


namespace {

constexpr float kTRelTol = 1e-4f;

std::vector<re::Ray> make_test_rays(const re::Scene& scene, int count) {
    std::vector<re::Ray> rays;
    rays.reserve(static_cast<size_t>(count));

    const re::PinholeParams camera =
        re::make_pinhole({0.0f, 0.0f, 1.08f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 38.0f, 64, 64);

    for (int i = 0; i < count; ++i) {
        re::Sampler sampler(i, 7, 13);

        const int kind = i % 3;
        if (kind == 0) {
            const int px = static_cast<int>(sampler.next_1d() * 64.0f);
            const int py = static_cast<int>(sampler.next_1d() * 64.0f);
            rays.push_back(re::generate_ray(camera, px, py, sampler.next_2d()));
        } else if (kind == 1) {
            const re::float2 a = sampler.next_2d();
            sampler.skip_2d();
            const re::float3 origin{(a.x - 0.5f) * 0.4f, (a.y - 0.5f) * 0.4f, 0.137f};

            const re::float2 u = sampler.next_2d();
            const float      z = 2.0f * u.x - 1.0f;
            const float      r = std::sqrt(std::max(0.0f, 1.0f - z * z));
            const float      phi = 6.2831853f * u.y;

            re::Ray ray;
            
            ray.origin = origin;
            ray.dir    = re::float3{r * std::cos(phi), r * std::sin(phi), z};
            rays.push_back(ray);
        } else {
            const re::MeshView geometry = scene.geometry();
            const int prim =
                static_cast<int>(sampler.next_1d() * static_cast<float>(geometry.triangle_count));
            const re::Triangle tri =
                re::get_triangle(geometry, std::min(prim, geometry.triangle_count - 1));

            const re::float2 bary = re::sample_uniform_triangle(sampler.next_2d());
            const re::float3 target = tri.get_point(bary.x, bary.y);

            const re::float2 a = sampler.next_2d();
            const re::float3 origin{(a.x - 0.5f) * 0.4f, (a.y - 0.5f) * 0.4f, 0.137f};

            re::Ray ray;
            ray.origin = origin;
            ray.dir    = re::normalize(target - origin);
            rays.push_back(ray);
        }
    }

    return rays;
}

}  // namespace

TEST_CASE("accel agreement: embree matches the brute-force oracle") {
    const re::Scene        scene    = re::make_cornell_scene();
    const re::MeshView geometry = scene.geometry();

    re::BruteAccel  brute;
    re::EmbreeAccel embree;
    brute.build(geometry);
    embree.build(geometry);

    const std::vector<re::Ray> rays = make_test_rays(scene, 30000);

    int mismatched_prim = 0;
    int mismatched_t    = 0;
    int mismatched_hit  = 0;

    for (size_t i = 0; i < rays.size(); ++i) {
        const re::Hit a = brute.intersect(rays[i]);
        const re::Hit b = embree.intersect(rays[i]);

        if (re::is_hit(a) != re::is_hit(b)) {
            ++mismatched_hit;
            continue;
        }
        if (!re::is_hit(a)) continue;

        if (a.primitive_id != b.primitive_id) {
            ++mismatched_prim;
            if (mismatched_prim <= 10) {
                printf("prim mismatch: kind=%zu brute=%d(t=%.6f u=%.4f v=%.4f) "
                      "embree=%d(t=%.6f u=%.4f v=%.4f) dt=%.2e\n",
                      i % 3, a.primitive_id, a.t, a.u, a.v,
                      b.primitive_id, b.t, b.u, b.v, std::fabs(a.t - b.t));
            }
            continue;
        }

        const float rel = std::fabs(a.t - b.t) / std::max(1e-6f, std::fabs(a.t));
        if (rel > kTRelTol) ++mismatched_t;
    }

    const double n = static_cast<double>(rays.size());

    CHECK(mismatched_hit == 0);
    CHECK(mismatched_t == 0);
    CHECK(static_cast<double>(mismatched_prim) / n < 0.001);
}

TEST_CASE("accel agreement: occluded matches between backends") {
    const re::Scene        scene    = re::make_cornell_scene();
    const re::MeshView geometry = scene.geometry();

    re::BruteAccel  brute;
    re::EmbreeAccel embree;
    brute.build(geometry);
    embree.build(geometry);

    std::vector<re::Ray> rays = make_test_rays(scene, 20000);
    int disagreements = 0;
    for (size_t i = 0; i < rays.size(); ++i) {
        rays[i].tmax = 0.05f + 0.6f * static_cast<float>(i % 20) / 20.0f;

        if (brute.occluded(rays[i]) != embree.occluded(rays[i])) {
          ++disagreements;
          if (disagreements <= 10) {
              const re::Hit h = brute.intersect(rays[i]);
              printf("occ mismatch: brute=%d embree=%d tmax=%.6f hit_t=%.6f prim=%d\n",
                    brute.occluded(rays[i]) ? 1 : 0, embree.occluded(rays[i]) ? 1 : 0,
                    rays[i].tmax, h.t, h.primitive_id);
          }
        };
        
    }

    CHECK(disagreements == 0);
}