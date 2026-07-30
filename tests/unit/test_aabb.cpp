#include "doctest.h"

#include "core/types.h"
#include "core/aabb.h"

namespace re {

TEST_CASE("aabb bounds management") {
  SUBCASE("expand empty box with a single point") {
    AABB b = aabb_empty();
    float3 p = {1.2f, -1.0f, 4.0f};

    expand(b, p);
    
    CHECK(b.lo.x == p.x);
    CHECK(b.lo.y == p.y);
    CHECK(b.lo.z == p.z);

    CHECK(b.hi.x == p.x);
    CHECK(b.hi.y == p.y);
    CHECK(b.hi.z == p.z);
  }

  SUBCASE("merge two existing boxes") {
    AABB b1 = { {2.5f, -1.0f, 4.0f}, {2.5f, -1.0f, 4.0f} }; 
    AABB b2 = { {-2.0f, 0.0f, 1.0f}, {0.0f, 5.0f, 8.0f} };
    AABB merged;

    merged = merge(b1, b2);

    CHECK(merged.lo.x == -2.0f);
    CHECK(merged.lo.y == -1.0f);
    CHECK(merged.lo.z == 1.0f);
    
    CHECK(merged.hi.x == 2.5f);
    CHECK(merged.hi.y == 5.0f);
    CHECK(merged.hi.z == 8.0f);
  }
}

TEST_CASE("AABB Ray Intersection") {
    AABB box = { {-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f} };

    SUBCASE("Hit: Ray pointing directly at the box") {
        float3 d = {0.0f, 0.0f, 1.0f};
        float3 inv_d = {1.0f / d.x, 1.0f / d.y, 1.0f / d.z};
        CHECK(hit_aabb(box, {0.0f, 0.0f, -5.0f}, inv_d, 0.0f, 10.0f) == true);
    }

    SUBCASE("Miss: Ray pointing parallel but entirely missing") {
        float3 d = {0.0f, 1.0f, 0.0f};
        float3 inv_d = {1.0f / d.x, 1.0f / d.y, 1.0f / d.z};
        CHECK(hit_aabb(box, {0.0f, 0.0f, -5.0f}, inv_d, 0.0f, 10.0f) == false);
    }

    SUBCASE("Hit: Ray origin is strictly inside the box") {
        float3 d = {0.5f, 0.5f, 0.5f};
        float3 inv_d = {1.0f / d.x, 1.0f / d.y, 1.0f / d.z};
        CHECK(hit_aabb(box, {0.0f, 0.0f, 0.0f}, inv_d, 0.0f, 10.0f) == true);
    }

    SUBCASE("Hit: Ray exactly parallel to X axis, perfectly aligned with box") {
        float3 d = {1.0f, 0.0f, 0.0f};
        float3 inv_d = {1.0f / d.x, 1.0f / d.y, 1.0f / d.z};
        CHECK(hit_aabb(box, {-5.0f, 0.0f, 0.0f}, inv_d, 0.0f, 10.0f) == true);
    }

    SUBCASE("Miss: Ray exactly parallel to X axis, but physically out of bounds on Y") {
        float3 d = {1.0f, 0.0f, 0.0f};
        float3 inv_d = {1.0f / d.x, 1.0f / d.y, 1.0f / d.z};
        CHECK(hit_aabb(box, {-5.0f, 2.0f, 0.0f}, inv_d, 0.0f, 10.0f) == false);
    }
}
}