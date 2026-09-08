#pragma once

#include <vector>

#include "material/bsdf.h"
#include "material/bxdf_lambertian.h"
#include "material/bxdf_microfacet.h"
#include "material/bsdf_build.h"

namespace bxdftest {

struct TestLobe {
    enum class Kind { Lambertian, Microfacet, Composite };

    Kind                kind;
    re::LambertianLobe  lambertian;
    re::MicrofacetLobe  microfacet;
    re::Material        material;
    const char*         name;
};

inline re::Bsdf composite_bsdf(const TestLobe& l, const re::float3& wo) {
  return re::make_bsdf(l.material, wo);
}

inline re::Spectrum eval(const TestLobe& l, const re::float3& wo, const re::float3& wi) {
    switch (l.kind) {
        case TestLobe::Kind::Lambertian:  return re::lambertian_eval(l.lambertian, wo, wi);
        case TestLobe::Kind::Microfacet:  return re::microfacet_eval(l.microfacet, wo, wi);
        case TestLobe::Kind::Composite:   return re::bsdf_eval(composite_bsdf(l, wo), wo, wi);
    }
    return re::make_empty_spectrum();
}

inline float pdf(const TestLobe& l, const re::float3& wo, const re::float3& wi) {
    switch (l.kind) {
        case TestLobe::Kind::Lambertian:  return re::lambertian_pdf(wo, wi);
        case TestLobe::Kind::Microfacet:  return re::microfacet_pdf(l.microfacet, wo, wi);
        case TestLobe::Kind::Composite:   return re::bsdf_pdf(composite_bsdf(l, wo), wo, wi);
    }
    return 0.0f;
}

inline re::BsdfSample sample(const TestLobe& l, const re::float3& wo, float u_lobe, re::float2 u) {
    switch (l.kind) {
        case TestLobe::Kind::Lambertian:  return re::lambertian_sample(l.lambertian, wo, u);
        case TestLobe::Kind::Microfacet:  return re::microfacet_sample(l.microfacet, wo, u);
        case TestLobe::Kind::Composite:   return re::bsdf_sample(composite_bsdf(l, wo), wo, u_lobe, u);
    }
    return re::invalid_sample();
}

inline TestLobe make_lambertian(float albedo, const char* name) {
    TestLobe l{};
    l.kind             = TestLobe::Kind::Lambertian;
    l.lambertian.albedo = re::Spectrum{albedo, albedo, albedo};
    l.name             = name;
    return l;
}

inline TestLobe make_microfacet(float alpha, float f0, const char* name) {
    TestLobe l{};
    l.kind             = TestLobe::Kind::Microfacet;
    l.microfacet.alpha = alpha;
    l.microfacet.f0    = re::Spectrum{f0, f0, f0};
    l.name             = name;
    return l;
}

inline TestLobe make_composite(float base, float metallic, float roughness, const char* name) {
    TestLobe l{};
    l.kind                  = TestLobe::Kind::Composite;
    l.material.base_color   = re::Spectrum{base, base, base};
    l.material.metallic     = metallic;
    l.material.roughness    = roughness;
    l.name                  = name;
    return l;
}


inline std::vector<TestLobe> white_lobes() {
    return {
        make_lambertian(1.0f, "lambertian white"),
        make_microfacet(0.05f, 1.0f, "ggx alpha=0.05"),
        make_microfacet(0.2f, 1.0f, "ggx alpha=0.2"),
        make_microfacet(0.5f, 1.0f, "ggx alpha=0.5"),
        make_microfacet(0.9f, 1.0f, "ggx alpha=0.9"),
    };
}

inline std::vector<TestLobe> coloured_lobes() {
    return {
        make_lambertian(0.6f, "lambertian 0.6"),
        make_microfacet(0.1f, 0.04f, "ggx dielectric-ish"),
        make_microfacet(0.35f, 0.9f, "ggx metal-ish"),
        make_microfacet(0.7f, 0.5f, "ggx rough"),
        make_composite(0.5f, 0.0f, 0.3f, "composite dielectric"),
        make_composite(0.8f, 1.0f, 0.4f, "composite metal"),
        make_composite(0.5f, 0.5f, 0.25f, "composite half-metal")
    };
}

inline std::vector<re::float3> test_wo() {
    constexpr float kThetaDeg[] = {0.0f, 30.0f, 45.0f, 80.0f, 89.4f};
    constexpr float kPhiDeg[]   = {0.0f, 59.7f};

    std::vector<re::float3> wo;
    for (float theta_deg : kThetaDeg) {
        for (float phi_deg : kPhiDeg) {
            const float theta = theta_deg * re::kPi / 180.0f;
            const float phi   = phi_deg * re::kPi / 180.0f;
            const float st    = std::sin(theta);
            const re::float3 up{st * std::cos(phi), st * std::sin(phi), std::cos(theta)};
            wo.push_back(up);
            wo.push_back(re::float3{up.x, up.y, -up.z});  // same angle, back side
        }
    }
    
    return wo;
}
}