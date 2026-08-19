#include "doctest.h"
#include <cmath>
#include "core/ray.h"
#include "scene/mesh.h"
#include "accel/intersect_triangle.h"

using namespace re;

TEST_CASE("Triangle Intersection: Möller-Trumbore Algorithm") {
    const Triangle tri = {
        float3{0.0f, 0.0f, 0.0f}, // v0
        float3{1.0f, 0.0f, 0.0f}, // v1
        float3{0.0f, 1.0f, 0.0f}  // v2
    };

    TriangleHit hit;

    SUBCASE("Valid hit on front face") {
        Ray ray = {
            float3{0.2f, 0.2f, 2.0f},  // origin
            0.0f,                      // tmin
            float3{0.0f, 0.0f, -1.0f}, // dir (normalized)
            10.0f                      // tmax
        };
        
        bool result = intersect_triangle(ray, tri, hit);
        
        CHECK(result == true);
        CHECK(hit.t == doctest::Approx(2.0f).epsilon(1e-4f));
        CHECK(hit.u == doctest::Approx(0.2f).epsilon(1e-4f));
        CHECK(hit.v == doctest::Approx(0.2f).epsilon(1e-4f));
    }

    SUBCASE("Valid hit on back face (no backface culling)") {
        Ray ray = {
            float3{0.2f, 0.2f, -2.0f}, // origin (behind triangle)
            0.0f,
            float3{0.0f, 0.0f, 1.0f},  // dir (pointing +Z)
            10.0f
        };
        
        bool result = intersect_triangle(ray, tri, hit);
        
        // Determinant is negative in this case, algorithm must handle abs_det correctly.
        CHECK(result == true);
        CHECK(hit.t == doctest::Approx(2.0f).epsilon(1e-4f));
        CHECK(hit.u == doctest::Approx(0.2f).epsilon(1e-4f));
        CHECK(hit.v == doctest::Approx(0.2f).epsilon(1e-4f));
    }

    SUBCASE("Miss: Ray intersects plane outside triangle bounds (Barycentric rejection)") {
        // u < 0
        Ray ray_neg_u = { float3{-0.1f, 0.2f, 2.0f}, 0.0f, float3{0.0f, 0.0f, -1.0f}, 10.0f };
        CHECK(intersect_triangle(ray_neg_u, tri, hit) == false);

        // v < 0
        Ray ray_neg_v = { float3{0.2f, -0.1f, 2.0f}, 0.0f, float3{0.0f, 0.0f, -1.0f}, 10.0f };
        CHECK(intersect_triangle(ray_neg_v, tri, hit) == false);

        // u + v > 1
        Ray ray_out_hypotenuse = { float3{0.6f, 0.6f, 2.0f}, 0.0f, float3{0.0f, 0.0f, -1.0f}, 10.0f };
        CHECK(intersect_triangle(ray_out_hypotenuse, tri, hit) == false);
    }

    SUBCASE("Miss: Ray fails depth bounds (tmin / tmax rejection)") {
        Ray ray = { float3{0.2f, 0.2f, 2.0f}, 0.0f, float3{0.0f, 0.0f, -1.0f}, 10.0f };

        // t < tmin
        ray.tmin = 5.0f;
        CHECK(intersect_triangle(ray, tri, hit) == false);

        // t > tmax
        ray.tmin = 0.0f;
        ray.tmax = 1.0f;
        CHECK(intersect_triangle(ray, tri, hit) == false);
    }

    SUBCASE("Miss: Ray is parallel to the triangle plane (Determinant near zero)") {
        Ray ray = {
            float3{0.2f, 0.2f, 2.0f},
            0.0f,
            float3{1.0f, 0.0f, 0.0f}, // Parallel to XY plane
            10.0f
        };
        
        CHECK(intersect_triangle(ray, tri, hit) == false);
    }

    SUBCASE("Edge case: Hit exactly on a vertex") {
        Ray ray = {
            float3{1.0f, 0.0f, 2.0f}, // Aiming directly at v1
            0.0f,
            float3{0.0f, 0.0f, -1.0f},
            10.0f
        };

        bool result = intersect_triangle(ray, tri, hit);
        
        CHECK(result == true);
        CHECK(hit.t == doctest::Approx(2.0f).epsilon(1e-4f));
        CHECK(hit.u == doctest::Approx(1.0f).epsilon(1e-4f));
        CHECK(hit.v == doctest::Approx(0.0f).epsilon(1e-4f));
    }
}