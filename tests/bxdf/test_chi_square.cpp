#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "bxdf_test_util.h"
#include "core/sampler.h"

namespace {

constexpr int kMuBins  = 16;
constexpr int kPhiBins = 32;
constexpr int kSamples = 300000;

re::float3 spherical_to_dir(double mu, double phi) {
    const double r = std::sqrt(std::max(0.0, 1.0 - mu * mu));
    return re::float3{static_cast<float>(r * std::cos(phi)),
                      static_cast<float>(r * std::sin(phi)), static_cast<float>(mu)};
}

double integrate_cell(const bxdftest::TestLobe& lobe, const re::float3& wo, double mu0,
                      double mu1, double phi0, double phi1, int n) {
    const double dmu  = (mu1 - mu0) / n;
    const double dphi = (phi1 - phi0) / n;

    double sum = 0.0;
    for (int a = 0; a < n; ++a) {
        for (int b = 0; b < n; ++b) {
            const double mu  = mu0 + (a + 0.5) * dmu;
            const double phi = phi0 + (b + 0.5) * dphi;
            sum += bxdftest::pdf(lobe, wo, spherical_to_dir(mu, phi));
        }
    }
    return sum * dmu * dphi;
}

double integrate_bin(const bxdftest::TestLobe& lobe, const re::float3& wo, double mu0, double mu1,
                     double phi0, double phi1) {
    constexpr double kRelTol = 1e-4;
    constexpr int    kMaxN   = 256;

    double prev = integrate_cell(lobe, wo, mu0, mu1, phi0, phi1, 4);
    for (int n = 8; n <= kMaxN; n *= 2) {
        const double cur = integrate_cell(lobe, wo, mu0, mu1, phi0, phi1, n);
        if (std::fabs(cur - prev) <= kRelTol * std::max(cur, 1e-12)) return cur;
        prev = cur;
    }
    return prev;
}

std::vector<double> integrate_pdf(const bxdftest::TestLobe& lobe, const re::float3& wo) {
    std::vector<double> expected(kMuBins * kPhiBins, 0.0);

    const double dmu  = 1.0 / kMuBins;
    const double dphi = 2.0 * 3.14159265358979323846 / kPhiBins;

    for (int i = 0; i < kMuBins; ++i) {
        for (int j = 0; j < kPhiBins; ++j) {
            expected[i * kPhiBins + j] =
                integrate_bin(lobe, wo, i * dmu, (i + 1) * dmu, j * dphi, (j + 1) * dphi);
        }
    }
    return expected;
}

struct ChiSquareResult {
    double statistic;
    int    dof;
    int    below_horizon;
    double expected_total;
    double observed_total;
};

ChiSquareResult run_chi_square(const bxdftest::TestLobe& lobe, const re::float3& wo, int seed) {
    std::vector<int> observed(kMuBins * kPhiBins, 0);
    int              below_horizon = 0;
    int              binned        = 0;

    for (int n = 0; n < kSamples; ++n) {
        re::Sampler      sampler(seed, n & 0xFFFF, n >> 16);
        const re::float2 u = sampler.next_2d();

        const re::BsdfSample s = bxdftest::sample(lobe, wo, u);
        if (s.pdf <= 0.0f) continue;

        if (s.wi.z <= 0.0f) {
            ++below_horizon;
            continue;
        }

        int mu_bin = static_cast<int>(s.wi.z * kMuBins);

        float phi = std::atan2(s.wi.y, s.wi.x);
        if (phi < 0.0f) phi += 2.0f * 3.14159265358979f;
        int phi_bin = static_cast<int>(phi / (2.0f * 3.14159265358979f) * kPhiBins);

        mu_bin  = std::min(mu_bin, kMuBins - 1);
        phi_bin = std::min(phi_bin, kPhiBins - 1);

        ++observed[mu_bin * kPhiBins + phi_bin];
        ++binned;
    }

    const std::vector<double> expected = integrate_pdf(lobe, wo);
    double chi2       = 0.0;
    int    dof        = 0;
    double pooled_obs = 0.0;
    double pooled_exp = 0.0;
    double exp_total  = 0.0;

    for (size_t k = 0; k < expected.size(); ++k) {
        const double e = expected[k] * kSamples;
        const double o = observed[k];
        exp_total += expected[k];

        if (e < 5.0) {
            pooled_obs += o;
            pooled_exp += e;
            continue;
        }
        const double d = o - e;
        chi2 += d * d / e;
        ++dof;
    }

    if (pooled_exp >= 5.0) {
        const double d = pooled_obs - pooled_exp;
        chi2 += d * d / pooled_exp;
        ++dof;
    }

    return ChiSquareResult{chi2, dof, below_horizon, exp_total,
                           static_cast<double>(binned) / kSamples};
}

}  // namespace

TEST_CASE("chi-square: sample and pdf describe the same distribution") {
    int seed = 1;

    for (const auto& lobe : bxdftest::coloured_lobes()) {
        for (const auto& wo : bxdftest::test_wo()) {
            CAPTURE(std::string(lobe.name));
            CAPTURE(wo.z);

            const ChiSquareResult r = run_chi_square(lobe, wo, seed++);

            CHECK(r.below_horizon == 0);

            CHECK(r.expected_total == doctest::Approx(r.observed_total).epsilon(2e-2));

            REQUIRE(r.dof > 0);
            const double threshold = r.dof + 5.0 * std::sqrt(2.0 * r.dof);
            CHECK(r.statistic < threshold);
        }
    }
}