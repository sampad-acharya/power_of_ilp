#include <chrono>
#include <iostream>

using Clock = std::chrono::steady_clock;
using ns    = std::chrono::nanoseconds;

// ------------------------------------------------------------
// Low ILP: strict dependency chain
// ------------------------------------------------------------
double low_ilp(size_t iters) {
    volatile double x = 1.0;
    for (size_t i = 0; i < iters; ++i) {
        // Each iteration depends on the previous result
        x = x * 1.0000001 + 0.0000001;
        x = x * 1.0000001 + 0.0000001;
        x = x * 1.0000001 + 0.0000001;
        x = x * 1.0000001 + 0.0000001;
    }
    return x;
}

// ------------------------------------------------------------
// High ILP: same work, but independent accumulators
// ------------------------------------------------------------
double high_ilp(size_t iters) {
    volatile double a = 1.0, b = 1.0, c = 1.0, d = 1.0;

    for (size_t i = 0; i < iters; ++i) {
        // Same math as low ILP, but no dependencies between them
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
    high_ilp(10000);

    long long t1 = bench(low_ilp, iters);
    long long t2 = bench(high_ilp, iters);

    std::cout << "ILP Benchmark (" << iters << " iterations)\n\n";
    std::cout << "Low ILP (serial dependency):   " << t1 << " ns\n";
    std::cout << "High ILP (independent ops):    " << t2 << " ns\n\n";

    std::cout << "Speedup from ILP: " << double(t1) / double(t2) << "x\n";
}

