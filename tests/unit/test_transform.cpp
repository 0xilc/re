#include "doctest.h"
#include <cmath>
#include <type_traits>
#include "core/types.h"
#include "core/math.h"
#include "core/transform.h"

using namespace re;

static_assert(sizeof(Transform) == 48, "Transform struct size MUST be exactly 48 bytes.");
static_assert(std::is_standard_layout_v<Transform>, "Transform must have standard C-layout.");

inline void check_float3_approx(const float3& a, const float3& b, float eps = 1e-4f) {
    CHECK(a.x == doctest::Approx(b.x).epsilon(eps));
    CHECK(a.y == doctest::Approx(b.y).epsilon(eps));
    CHECK(a.z == doctest::Approx(b.z).epsilon(eps));
}

inline void check_float4_approx(const float4& a, const float4& b, float eps = 1e-4f) {
    CHECK(a.x == doctest::Approx(b.x).epsilon(eps));
    CHECK(a.y == doctest::Approx(b.y).epsilon(eps));
    CHECK(a.z == doctest::Approx(b.z).epsilon(eps));
    CHECK(a.w == doctest::Approx(b.w).epsilon(eps));
}

TEST_CASE("Transform: Point vs Vector Translation Semantics") {
    Transform t = transform_identity();
    t.r[0].w = 5.0f;
    t.r[1].w = -3.0f;

    SUBCASE("Points are affected by translation") {
        float3 origin = {0.0f, 0.0f, 0.0f};
        float3 p_prime = transform_point(t, origin);
        
        check_float3_approx(p_prime, {5.0f, -3.0f, 0.0f});
    }

    SUBCASE("Vectors ignore translation") {
        float3 dir = {1.0f, 0.0f, 0.0f};
        float3 v_prime = transform_vector(t, dir);
        
        check_float3_approx(v_prime, {1.0f, 0.0f, 0.0f});
    }
}

TEST_CASE("Transform: Inverse and Compose Identity Guarantee") {
    Transform t = {
        float4(2.0f,  0.5f, -0.1f, 10.0f),
        float4(0.0f,  1.0f,  0.5f, -5.0f),
        float4(0.1f, -0.2f,  1.0f,  3.0f)
    };

    Transform inv_t = inverse(t);
    Transform ident_computed = compose(t, inv_t);
    Transform ident_ideal = transform_identity();

    check_float4_approx(ident_computed.r[0], ident_ideal.r[0]);
    check_float4_approx(ident_computed.r[1], ident_ideal.r[1]);
    check_float4_approx(ident_computed.r[2], ident_ideal.r[2]);
}

TEST_CASE("Transform: Non-Uniform Scale Normal Orthogonality") {
    Transform scale_x2 = {
        float4(2.0f, 0.0f, 0.0f, 0.0f),
        float4(0.0f, 1.0f, 0.0f, 0.0f),
        float4(0.0f, 0.0f, 1.0f, 0.0f)
    };
    
    Transform inv_scale = inverse(scale_x2);

    float inv_sqrt2 = 1.0f / sqrtf(2.0f);
    float3 tangent = {inv_sqrt2, inv_sqrt2, 0.0f};
    float3 normal = {-inv_sqrt2, inv_sqrt2, 0.0f};
    
    CHECK(dot(tangent, normal) == doctest::Approx(0.0f).epsilon(1e-5f));

    float3 tangent_prime = transform_vector(scale_x2, tangent);
    float3 normal_prime = transform_normal(inv_scale, normal);

    CHECK(dot(tangent_prime, normal_prime) == doctest::Approx(0.0f).epsilon(1e-5f));
}