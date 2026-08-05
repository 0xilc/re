#pragma once

#include <vector>

#include "material/bxdf_lambertian.h"
#include "material/bxdf_microfacet.h"

namespace bxdftest {

struct TestLobe {
    enum class Kind { Lambertian, Microfacet };

    Kind              kind;
    re::LambertianLobe lambertian;
    re::MicrofacetLobe microfacet;
    const char*       name;
};

inline re::Spectrum eval(const TestLobe& l, const re::float3& wo, const re::float3& wi) {
    switch (l.kind) {
        case TestLobe::Kind::Lambertian: return re::lambertian_eval(l.lambertian, wo, wi);
        case TestLobe::Kind::Microfacet: return re::microfacet_eval(l.microfacet, wo, wi);
    }
    return re::make_empty_spectrum();
}

inline float pdf(const TestLobe& l, const re::float3& wo, const re::float3& wi) {
    switch (l.kind) {
        case TestLobe::Kind::Lambertian: return re::lambertian_pdf(wo, wi);
        case TestLobe::Kind::Microfacet: return re::microfacet_pdf(l.microfacet, wo, wi);
    }
    return 0.0f;
}

inline re::BsdfSample sample(const TestLobe& l, const re::float3& wo, re::float2 u) {
    switch (l.kind) {
        case TestLobe::Kind::Lambertian: return re::lambertian_sample(l.lambertian, wo, u);
        case TestLobe::Kind::Microfacet: return re::microfacet_sample(l.microfacet, wo, u);
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
    };
}
inline std::vector<re::float3> test_wo() {
    std::vector<re::float3> wo = {
        {0.0f, 0.0f, 1.0f},                    // normal incidence
        {0.5f, 0.0f, 0.8660254f},              // 30 degrees
        {0.7071068f, 0.0f, 0.7071068f},        // 45 degrees
        {0.3420201f, 0.5868241f, 0.7340425f},  // off-axis in phi
        {0.9848078f, 0.0f, 0.1736482f},        // 80 degrees
        {0.9999500f, 0.0f, 0.0100000f},        // grazing
    };
    return wo;
}

}