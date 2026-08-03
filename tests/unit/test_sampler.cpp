#include <doctest.h>
#include <vector>
#include "core/sampler.h"
#include <cmath>

TEST_CASE("sampler: identical coordinates yield identical values") {
    re::Sampler a(12, 34, 7);
    re::Sampler b(12, 34, 7);
 
    for (int i = 0; i < 16; ++i) {
        CHECK(a.next_1d() == b.next_1d());
    }
}

TEST_CASE("sampler: values stay in [0, 1)") {
    // 1.0f exactly is the value that turns sqrt(1 - u) and tan(acos(u)) into
    // inf/NaN, so the upper bound is checked strictly.
    for (int idx = 0; idx < 512; ++idx) {
        re::Sampler s(3, 5, idx);
        for (int d = 0; d < 8; ++d) {
            const float u = s.next_1d();
            CHECK(u >= 0.0f);
            CHECK(u < 1.0f);
        }
    }
}
 
TEST_CASE("sampler: dimension advances") {
    re::Sampler s(1, 1, 1);
    const float a = s.next_1d();
    const float b = s.next_1d();
    CHECK(a != b);  // same seed, different dimension
}
 
TEST_CASE("sampler: next_2d consumes exactly two dimensions") {
    // The budget contract: mixing next_2d and skip_2d must land on the same
    // dimension. If this drifts, Sobol stratification breaks in Phase 4.
    re::Sampler a(9, 9, 2);
    a.next_2d();
    const float after_pair = a.next_1d();
 
    re::Sampler b(9, 9, 2);
    b.skip_2d();
    const float after_skip = b.next_1d();
 
    CHECK(after_pair == after_skip);
}
 
TEST_CASE("sampler: histogram is flat") {
    constexpr int kBins    = 32;
    constexpr int kSamples = 200000;
 
    std::vector<int> bins(kBins, 0);
    for (int idx = 0; idx < kSamples; ++idx) {
        re::Sampler s(17, 23, idx);
        const int   bin = static_cast<int>(s.next_1d() * kBins);
        ++bins[bin];
    }
 
    // Chi-square against a uniform expectation. 31 dof, p = 0.001 -> ~62.5.
    const double expected = static_cast<double>(kSamples) / kBins;
    double       chi2     = 0.0;
    for (int b : bins) {
        const double d = b - expected;
        chi2 += d * d / expected;
    }
    CHECK(chi2 < 62.5);
}
 
TEST_CASE("sampler: neighbouring pixels are decorrelated") {
    // The banding test. A weak seed mix (y * width + x, or a plain add) leaves
    // adjacent pixels with nearly the same first sample, which reads as
    // structure across the image rather than noise.
    constexpr int kCount = 4096;
 
    double sum_a = 0.0, sum_b = 0.0, sum_ab = 0.0, sum_aa = 0.0, sum_bb = 0.0;
    for (int i = 0; i < kCount; ++i) {
        re::Sampler left(i % 64, i / 64, 0);
        re::Sampler right((i % 64) + 1, i / 64, 0);
 
        const double a = left.next_1d();
        const double b = right.next_1d();
        sum_a += a;
        sum_b += b;
        sum_ab += a * b;
        sum_aa += a * a;
        sum_bb += b * b;
    }
 
    const double n    = kCount;
    const double cov  = sum_ab / n - (sum_a / n) * (sum_b / n);
    const double var_a = sum_aa / n - (sum_a / n) * (sum_a / n);
    const double var_b = sum_bb / n - (sum_b / n) * (sum_b / n);
    const double corr = cov / std::sqrt(var_a * var_b);
 
    CHECK(std::fabs(corr) < 0.05);
}
 
TEST_CASE("sampler: consecutive sample indices are decorrelated") {
    constexpr int kCount = 4096;
 
    double sum_a = 0.0, sum_b = 0.0, sum_ab = 0.0, sum_aa = 0.0, sum_bb = 0.0;
    for (int i = 0; i < kCount; ++i) {
        re::Sampler s0(11, 13, i);
        re::Sampler s1(11, 13, i + 1);
 
        const double a = s0.next_1d();
        const double b = s1.next_1d();
        sum_a += a;
        sum_b += b;
        sum_ab += a * b;
        sum_aa += a * a;
        sum_bb += b * b;
    }
 
    const double n     = kCount;
    const double cov   = sum_ab / n - (sum_a / n) * (sum_b / n);
    const double var_a = sum_aa / n - (sum_a / n) * (sum_a / n);
    const double var_b = sum_bb / n - (sum_b / n) * (sum_b / n);
    const double corr  = cov / std::sqrt(var_a * var_b);
 
    CHECK(std::fabs(corr) < 0.05);
}

