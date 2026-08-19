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

re::Transform make_trs(re::float3 t, re::float3 s, float rot_x) {
    const float c = std::cos(rot_x);
    const float n = std::sin(rot_x);

    re::Transform m;  // translate * rotate_x * scale
    m.r[0] = re::float4(s.x, 0.0f, 0.0f, t.x);
    m.r[1] = re::float4(0.0f, s.y * c, -s.z * n, t.y);
    m.r[2] = re::float4(0.0f, s.y * n, s.z * c, t.z);
    return m;
}

// The cornell box only ever uses identity instances, which would let a backend
// ignore transforms entirely and still pass. This one would not.
re::Scene make_instanced_scene() {
    re::Scene scene = re::make_cornell_scene();

    const int mesh_count = static_cast<int>(scene.meshes.size());
    for (int i = 0; i < 40; ++i) {
        re::Sampler sampler(i, 101, 3);

        const re::float2 t  = sampler.next_2d();
        const re::float2 s  = sampler.next_2d();
        const float      tz = sampler.next_1d();
        const float      rx = sampler.next_1d();

        scene.add_instance(make_trs({(t.x - 0.5f) * 0.6f, (t.y - 0.5f) * 0.6f, (tz - 0.5f) * 0.6f},
                                    {0.3f + s.x, 0.3f + s.y, 0.3f + s.x * s.y},
                                    (rx - 0.5f) * 6.2831853f),
                           static_cast<uint32_t>(i % mesh_count));
    }

    return scene;
}

std::vector<re::Ray> make_test_rays(const re::Scene& scene, int count) {
    std::vector<re::Ray> rays;
    rays.reserve(static_cast<size_t>(count));

    const re::SceneGeometryView geometry = scene.make_geometry_view();

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
            // Aim at a point on a real surface, which means going through an
            // instance and back out to world space.
            const int inst_id = std::min(
                static_cast<int>(sampler.next_1d() * static_cast<float>(geometry.instance_count)),
                static_cast<int>(geometry.instance_count) - 1);
            const re::Instance& inst = geometry.instances[inst_id];
            const re::MeshView& mesh = geometry.meshes[inst.mesh_id];

            const int prim =
                std::min(static_cast<int>(sampler.next_1d() *
                                          static_cast<float>(mesh.triangle_count)),
                         mesh.triangle_count - 1);
            const re::Triangle tri = re::get_triangle(mesh, prim);

            const re::float2 bary = re::sample_uniform_triangle(sampler.next_2d());
            const re::float3 target =
                re::transform_point(inst.to_world, tri.get_point(bary.x, bary.y));

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

void check_closest_hit_agreement(const re::Scene& scene, const char* label) {
    const re::SceneGeometryView geometry = scene.make_geometry_view();

    re::BruteAccel  brute;
    re::EmbreeAccel embree;
    brute.build(geometry);
    embree.build(geometry);

    const std::vector<re::Ray> rays = make_test_rays(scene, 30000);

    int mismatched_id  = 0;
    int mismatched_t   = 0;
    int mismatched_hit = 0;

    for (size_t i = 0; i < rays.size(); ++i) {
        const re::Hit a = brute.intersect(rays[i]);
        const re::Hit b = embree.intersect(rays[i]);

        if (re::is_hit(a) != re::is_hit(b)) {
            ++mismatched_hit;
            continue;
        }
        if (!re::is_hit(a)) continue;

        if (a.primitive_id != b.primitive_id || a.instance_id != b.instance_id) {
            ++mismatched_id;
            if (mismatched_id <= 10) {
                printf("[%s] id mismatch: kind=%zu brute=(inst=%d prim=%d t=%.6f u=%.4f v=%.4f) "
                       "embree=(inst=%d prim=%d t=%.6f u=%.4f v=%.4f) dt=%.2e\n",
                       label, i % 3, a.instance_id, a.primitive_id, a.t, a.u, a.v,
                       b.instance_id, b.primitive_id, b.t, b.u, b.v, std::fabs(a.t - b.t));
            }
            continue;
        }

        const float rel = std::fabs(a.t - b.t) / std::max(1e-6f, std::fabs(a.t));
        if (rel > kTRelTol) ++mismatched_t;
    }

    const double n = static_cast<double>(rays.size());

    CHECK(mismatched_hit == 0);
    CHECK(mismatched_t == 0);
    // Coincident surfaces can legitimately tie, so allow a handful of ties to
    // resolve differently between the two backends.
    CHECK(static_cast<double>(mismatched_id) / n < 0.001);
}

void check_occlusion_agreement(const re::Scene& scene, const char* label) {
    const re::SceneGeometryView geometry = scene.make_geometry_view();

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
              printf("[%s] occ mismatch: brute=%d embree=%d tmax=%.6f hit_t=%.6f prim=%d\n",
                    label, brute.occluded(rays[i]) ? 1 : 0, embree.occluded(rays[i]) ? 1 : 0,
                    rays[i].tmax, h.t, h.primitive_id);
          }
        };

    }

    CHECK(disagreements == 0);
}

}  // namespace

TEST_CASE("accel agreement: embree matches the brute-force oracle") {
    check_closest_hit_agreement(re::make_cornell_scene(), "cornell");
}

TEST_CASE("accel agreement: embree matches the oracle through instances") {
    check_closest_hit_agreement(make_instanced_scene(), "instanced");
}

TEST_CASE("accel agreement: occluded matches between backends") {
    check_occlusion_agreement(re::make_cornell_scene(), "cornell");
}

TEST_CASE("accel agreement: occluded matches through instances") {
    check_occlusion_agreement(make_instanced_scene(), "instanced");
}
