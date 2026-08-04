#include "doctest.h"

#include <cmath>

#include "bxdf_test_util.h"
#include "core/sampler.h"
#include "material/microfacet_dist.h"

TEST_CASE("microfacet sample weight matches its closed form") {
    int seed = 600;

    for (float alpha : {0.05f, 0.2f, 0.5f, 0.9f}) {
        const re::MicrofacetLobe lobe{re::Spectrum{1.0f, 1.0f, 1.0f}, alpha};

        for (const auto& wo : bxdftest::test_wo()) {
            CAPTURE(alpha);
            CAPTURE(wo.z);

            for (int n = 0; n < 200; ++n) {
                re::Sampler sampler(seed, n, 0);

                const re::BsdfSample s = re::microfacet_sample(lobe, wo, sampler.next_2d());
                if (s.pdf <= 0.0f) continue;

                const float a  = re::clamp_alpha(alpha);
                const float g1 = re::ggx_g1(wo, a);
                const float g2 = re::ggx_g2(wo, s.wi, a);

                const double expected = static_cast<double>(g2) / g1;
                const double actual   = static_cast<double>(s.f.x) * std::fabs(s.wi.z) / s.pdf;

                CHECK(actual == doctest::Approx(expected).epsilon(1e-3));
            }
            ++seed;
        }
    }
}