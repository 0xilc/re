#include "doctest.h"

#include <cmath>

#include "bxdf_test_util.h"
#include "core/sampler.h"

namespace {

constexpr int kTrials = 2000;

}  // namespace

TEST_CASE("reciprocity: f is symmetric under direction swap") {
    int seed = 400;

    for (const auto& lobe : bxdftest::coloured_lobes()) {
        for (int n = 0; n < kTrials; ++n) {
            re::Sampler sampler(seed, n, 0);

            const re::float3 wo = re::sample_uniform_hemisphere(sampler.next_2d());
            const re::float3 wi = re::sample_uniform_hemisphere(sampler.next_2d());

            CAPTURE(lobe.name);
            const re::Spectrum forward = bxdftest::eval(lobe, wo, wi);
            const re::Spectrum reverse = bxdftest::eval(lobe, wi, wo);

            CHECK(forward.x == doctest::Approx(reverse.x).epsilon(1e-4));
            CHECK(forward.y == doctest::Approx(reverse.y).epsilon(1e-4));
            CHECK(forward.z == doctest::Approx(reverse.z).epsilon(1e-4));
        }
        ++seed;
    }
}
