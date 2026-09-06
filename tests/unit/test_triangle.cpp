// Test triangle hit & miss via accel
#include "doctest.h"
#include <cmath>
#include "accel/brute_accel.h"
#include "accel/intersect_triangle.h"
#include "core/hit.h"
#include "core/ray.h"
#include "scene/mesh.h"

namespace {

// Reference triangle lives in the z = -1 plane with e1 = +x and e2 = +y, so
// barycentrics fall straight out of the coordinates: p = (u, v, -1). Every
// expected value below is therefore readable by hand -- a test whose constants
// cannot be verified by inspection is not evidence.
const re::Triangle kTri{
    re::float3{0.0f, 0.0f, -1.0f},  // v0
    re::float3{1.0f, 0.0f, -1.0f},  // v1
    re::float3{0.0f, 1.0f, -1.0f},  // v2
};

re::Ray ray_toward(const re::float3& from, const re::float3& to) {
    re::Ray r;
    r.origin = from;
    r.dir    = re::normalize(to - from);  // contract: dir is unit, so t is in meters
    return r;
}

constexpr float kEps = 1e-4f;

}  // namespace

TEST_CASE("intersect_triangle: interior hit reports t, u, v") {
    re::TriangleHit th;
    const re::Ray    r = ray_toward({0.0f, 0.0f, 0.0f}, {0.5f, 0.25f, -1.0f});

    REQUIRE(re::intersect_triangle(r, kTri, th));
    CHECK(th.u == doctest::Approx(0.5f).epsilon(kEps));
    CHECK(th.v == doctest::Approx(0.25f).epsilon(kEps));
    CHECK(th.t == doctest::Approx(std::sqrt(0.5f * 0.5f + 0.25f * 0.25f + 1.0f)).epsilon(kEps));
}

// p = (1-u-v)*v0 + u*v1 + v*v2 -- aiming at a vertex must pin the pair.
// This is the convention glTF, Embree and OptiX all assume; breaking it shows
// up in Phase 4 as textures that are subtly shifted.
TEST_CASE("intersect_triangle: barycentric convention at vertices") {
    re::TriangleHit th;

    SUBCASE("v0 -> (0, 0)") {
        REQUIRE(re::intersect_triangle(ray_toward({0, 0, 0}, {0.0f, 0.0f, -1.0f}), kTri, th));
        CHECK(th.u == doctest::Approx(0.0f).epsilon(kEps));
        CHECK(th.v == doctest::Approx(0.0f).epsilon(kEps));
    }
    SUBCASE("v1 -> (1, 0)") {
        REQUIRE(re::intersect_triangle(ray_toward({0, 0, 0}, {1.0f, 0.0f, -1.0f}), kTri, th));
        CHECK(th.u == doctest::Approx(1.0f).epsilon(kEps));
        CHECK(th.v == doctest::Approx(0.0f).epsilon(kEps));
    }
    SUBCASE("v2 -> (0, 1)") {
        REQUIRE(re::intersect_triangle(ray_toward({0, 0, 0}, {0.0f, 1.0f, -1.0f}), kTri, th));
        CHECK(th.u == doctest::Approx(0.0f).epsilon(kEps));
        CHECK(th.v == doctest::Approx(1.0f).epsilon(kEps));
    }
}

TEST_CASE("intersect_triangle: misses outside the edges") {
    re::TriangleHit th;

    SUBCASE("beyond the hypotenuse, u + v > 1") {
        CHECK_FALSE(re::intersect_triangle(ray_toward({0, 0, 0}, {0.7f, 0.7f, -1.0f}), kTri, th));
    }
    SUBCASE("negative u") {
        CHECK_FALSE(re::intersect_triangle(ray_toward({0, 0, 0}, {-0.1f, 0.5f, -1.0f}), kTri, th));
    }
    SUBCASE("negative v") {
        CHECK_FALSE(re::intersect_triangle(ray_toward({0, 0, 0}, {0.5f, -0.1f, -1.0f}), kTri, th));
    }
}

// The only case that exercises the sgn branch of the deferred-division variant.
// Without it a sign error stays invisible until a dielectric interior in Phase 5,
// where it would look like a shading bug rather than a geometry one.
TEST_CASE("intersect_triangle: hits from behind (no backface culling)") {
    re::TriangleHit th;
    const re::Ray    r = ray_toward({0.25f, 0.25f, -3.0f}, {0.25f, 0.25f, -1.0f});

    REQUIRE(re::intersect_triangle(r, kTri, th));
    CHECK(th.t == doctest::Approx(2.0f).epsilon(kEps));
    CHECK(th.u == doctest::Approx(0.25f).epsilon(kEps));  // unchanged when hit from behind
    CHECK(th.v == doctest::Approx(0.25f).epsilon(kEps));
}

// The interval test lives inside intersect_triangle (same contract as Embree's
// RTCRay tnear/tfar), so both ends are exercised here rather than at the caller.
TEST_CASE("intersect_triangle: respects the ray interval") {
    re::TriangleHit th;

    SUBCASE("geometry behind the origin does not render in front of it") {
        re::Ray r;
        r.origin = re::float3{0.25f, 0.25f, 0.0f};
        r.dir    = re::float3{0.0f, 0.0f, 1.0f};  // pointing away from z = -1
        CHECK_FALSE(re::intersect_triangle(r, kTri, th));
    }
    SUBCASE("beyond tmax") {
        re::Ray r = ray_toward({0, 0, 0}, {0.25f, 0.25f, -1.0f});
        r.tmax    = 0.5f;  // triangle sits at t ~= 1.06
        CHECK_FALSE(re::intersect_triangle(r, kTri, th));
    }
}

// Degenerate guard: ray parallel to the triangle plane, |det| < eps.
TEST_CASE("intersect_triangle: parallel ray misses") {
    re::TriangleHit th;
    re::Ray          r;
    r.origin = re::float3{0.0f, 0.0f, -1.0f};
    r.dir    = re::float3{1.0f, 0.0f, 0.0f};
    CHECK_FALSE(re::intersect_triangle(r, kTri, th));
}

TEST_CASE("BruteAccel: nearest hit wins and misses are encoded in prim_id") {
    // Two parallel triangles; prim 0 at z = -1, prim 1 at z = -2.
    const re::float3 positions[] = {
        {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, -1.0f},
        {0.0f, 0.0f, -2.0f}, {1.0f, 0.0f, -2.0f}, {0.0f, 1.0f, -2.0f},
    };
    const uint32_t indices[] = {0, 1, 2, 3, 4, 5};

    re::MeshView mesh;
    mesh.positions      = positions;
    mesh.indices        = indices;
    mesh.vertex_count   = 6;
    mesh.triangle_count = 2;

    const re::Instance instance = re::make_instance(re::transform_identity(), 0);

    re::SceneGeometryView geometry;
    geometry.meshes         = &mesh;
    geometry.instances      = &instance;
    geometry.mesh_count     = 1;
    geometry.instance_count = 1;

    re::BruteAccel accel;
    accel.build(geometry);

    SUBCASE("nearest primitive wins regardless of loop order") {
        const re::Hit hit = accel.intersect(ray_toward({0, 0, 0}, {0.25f, 0.25f, -1.0f}));
        REQUIRE(re::is_hit(hit));
        CHECK(hit.prim_id == 0);
        CHECK(hit.t == doctest::Approx(std::sqrt(0.25f * 0.25f * 2.0f + 1.0f)).epsilon(kEps));
    }

    SUBCASE("miss is signalled by prim_id, never inferred from t") {
        const re::Hit miss = accel.intersect(ray_toward({0, 0, 0}, {5.0f, 5.0f, -1.0f}));
        CHECK_FALSE(re::is_hit(miss));
        CHECK(miss.prim_id == re::INVALID_ID);
    }
}