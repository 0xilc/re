#include "doctest.h" 
#include "bxdf_test_util.h"
#include "core/sampler.h"

TEST_CASE("sample agrees with eval and pdf on the direction it returns") {
    int seed = 500;

    for (const auto& lobe : bxdftest::coloured_lobes()) {
        for (const auto& wo : bxdftest::test_wo()) {
            for (int n = 0; n < 500; ++n) {
                re::Sampler sampler(seed, n, 0);

                const re::BsdfSample s = bxdftest::sample(lobe, wo, sampler.next_2d());
                if (s.pdf <= 0.0f) continue;

                CAPTURE(lobe.name);
                CAPTURE(wo.z);

                CHECK(bxdftest::pdf(lobe, wo, s.wi) == doctest::Approx(s.pdf).epsilon(1e-3));
                CHECK(bxdftest::eval(lobe, wo, s.wi).x == doctest::Approx(s.f.x).epsilon(1e-3));
            }
            ++seed;
        }
    }
}