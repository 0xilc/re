#include "doctest.h"

#include <cmath>

#include "bxdf_test_util.h"
#include "core/sampler.h"

namespace {

constexpr int kSamples = 100000;

double directional_albedo(const bxdftest::TestLobe& lobe, const re::float3& wo, int seed) {
    double sum = 0.0;

    for (int n = 0; n < kSamples; ++n) {
        re::Sampler      sampler(seed, n & 0xFFFF, n >> 16);
        const re::float2 u = sampler.next_2d();

        const re::BsdfSample s = bxdftest::sample(lobe, wo, u);
        if (s.pdf <= 0.0f) continue;

        sum += static_cast<double>(s.f.x) * std::fabs(s.wi.z) / s.pdf;
    }

    return sum / kSamples;
}

}  // namespace

TEST_CASE("white furnace: lambertian returns all of its energy") {
    const bxdftest::TestLobe lobe = bxdftest::make_lambertian(1.0f, "lambertian white");

    int seed = 100;
    for (const auto& wo : bxdftest::test_wo()) {
        CAPTURE(wo.z);
        const double albedo = directional_albedo(lobe, wo, seed++);
        CHECK(albedo == doctest::Approx(1.0).epsilon(1e-4));
    }
}

TEST_CASE("white furnace: ggx loses only single-scattering energy") {
    int seed = 200;

    for (float alpha : {0.05f, 0.2f, 0.5f, 0.9f}) {
        const bxdftest::TestLobe lobe = bxdftest::make_microfacet(alpha, 1.0f, "ggx white");

        for (const auto& wo : bxdftest::test_wo()) {
            CAPTURE(alpha);
            CAPTURE(wo.z);

            const double albedo = directional_albedo(lobe, wo, seed++);

            CHECK(albedo <= 1.0 + 1e-3);
            CHECK(albedo > 0.3);
        }
    }
}

TEST_CASE("energy conservation: coloured lobes never amplify") {
    int seed = 300;

    for (const auto& lobe : bxdftest::coloured_lobes()) {
        for (const auto& wo : bxdftest::test_wo()) {
            CAPTURE(lobe.name);
            CAPTURE(wo.z);
            CHECK(directional_albedo(lobe, wo, seed++) <= 1.0 + 1e-3);
        }
    }
}