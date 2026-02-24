#include <chrono>
#include <iostream>

using Clock = std::chrono::steady_clock;
using ns    = std::chrono::nanoseconds;

// ------------------------------------------------------------
// Low ILP: strict dependency chain (4 separate ops)
// ------------------------------------------------------------
double low_ilp(size_t iters) {
    volatile double x = 1.0;
    for (size_t i = 0; i < iters; ++i) {
        x = x * 1.0000001 + 0.0000001;
        x = x * 1.0000001 + 0.0000001;
        x = x * 1.0000001 + 0.0000001;
        x = x * 1.0000001 + 0.0000001;
    }
    return x;
}

// ------------------------------------------------------------
// Low ILP (fused): same math, but expressed as one big chain
// ------------------------------------------------------------
double low_ilp_fused(size_t iters) {
    volatile double x = 1.0;
    for (size_t i = 0; i < iters; ++i) {
        x = (((x * 1.0000001 + 0.0000001)
              * 1.0000001 + 0.0000001)
              * 1.0000001 + 0.0000001)
              * 1.0000001 + 0.0000001;
    }
    return x;
}

// ------------------------------------------------------------
// High ILP: independent accumulators
// ------------------------------------------------------------
double high_ilp(size_t iters) {
    volatile double a = 1.0, b = 1.0, c = 1.0, d = 1.0;

    for (size_t i = 0; i < iters; ++i) {
        a = a * 1.0000001 + 0.0000001;
        b = b * 1.0000001 + 0.0000001;
        c = c * 1.0000001 + 0.0000001;
        d = d * 1.0000001 + 0.0000001;
    }
    return a + b + c + d;
}

// ------------------------------------------------------------
// Benchmark helper
// ------------------------------------------------------------
template <typename Fn>
long long bench(Fn&& fn, size_t iters) {
    auto start = Clock::now();
    volatile double sink = fn(iters);
    auto end = Clock::now();
    return std::chrono::duration_cast<ns>(end - start).count();
}

int main() {
    const size_t iters = 50'000'000;

    // Warmup
    low_ilp(10000);
    low_ilp_fused(10000);
    high_ilp(10000);

    long long t1 = bench(low_ilp, iters);
    long long t2 = bench(low_ilp_fused, iters);
    long long t3 = bench(high_ilp, iters);

    std::cout << "ILP Benchmark (" << iters << " iterations)\n\n";

    std::cout << "Low ILP (4 separate ops):      " << t1 << " ns\n";
    std::cout << "Low ILP (fused expression):    " << t2 << " ns\n";
    std::cout << "High ILP (independent ops):    " << t3 << " ns\n\n";

    std::cout << "Speedup fused vs low:          " << double(t1) / double(t2) << "x\n";
    std::cout << "Speedup high ILP vs low:       " << double(t1) / double(t3) << "x\n";

    return 0;
}

