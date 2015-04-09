# muFFT

muFFT is a library for doing the fast fourier transform (FFT).
The FFT has many applications in digital signal processing.
The main use cases are fast linear convolution and conversion from time domain into frequency domain and vice versa.
See [The Fast Fourier Transform](@ref FFT) for details on how the algorithm works and how it is implemented in muFFT.

## Features

muFFT is a moderately featured single-precision FFT library.
It focuses particularly on linear convolution for audio applications and being optimized for modern architectures.

 - Power-of-two transforms
 - 1D complex-to-complex transform
 - 1D real-to-complex transform
 - 1D complex-to-real transform
 - 2D complex-to-complex transform
 - 1D fast convolution for applying large filters.
   Supports both complex/real convolutions and real/real convolutions.
   The complex/real convolution is particularly useful for filtering interleaved stereo audio.
 - Designed and optimized for SIMD architectures,
   with optimizations for SSE, SSE3 and AVX-256 currently implemented.
   ARMv7 and ARMv8 NEON optimizations are expected to be implemented soon.
 - Radix-2, radix-4 and radix-8 butterfly implementations.
 - Input and output does not have to be reordered, as is sometimes the case with FFT algorithms.
   muFFT implements the Stockham autosort algorithm to avoid any explicit permutation of FFT coefficients.
 - Detects SIMD support for your hardware in runtime.
   Same muFFT binary can support wide ranges of hardware feature sets.

## Complex number representation

muFFT uses the C99 and C++ ABI for complex numbers, interleaved real and imaginary samples, i.e.:

    struct mufft_complex {
        float real;
        float imag;
    };

C99 `complex float` from `<complex.h>` and C++ `std::complex<float>` from `<complex>` can safely be used with muFFT.

## Performance 

muFFT is very performant and is competitive with highly optimized libraries like FFTW3 and FFmpeg/libavcodec FFT on tested hardware.
See [Benchmark](#benchmark) for how to run your own benchmarks.

muFFT is designed with moderate size FFTs in mind.
Very large FFTs which don't fit in cache could be better optimized by designing for cache utilization
and tiny FFTs (N = 2, 4, 8) don't have special handcoded vectorized transforms.

## License

muFFT is licensed under the permissive MIT license.

Note that the `mufft_bench` and `mufft_test` binaries link against FFTW3 for verification purposes, a GPLv2+ library.
If you choose to distribute either of these binaries, muFFT source must be provided as well.
See COPYING.GPLv2 for details.
These binaries are non-essential, and are only intended for use during development and verification,
and not for distribution.

## Documentation

The public muFFT API is documented with doxygen.
Run `doxygen` or `make docs` to generate documentation. Doxygen 1.8.3 is required.

After running Doxygen, documents are found in `docs/index.html`.

## Sample code

There is currently no dedicated sample code for muFFT. See `test.c` and the documentation for reference on how to use the API.
The various test routines flex most of the API. It it also a good way to see how the API calls match up to equivalent FFTW3 routines.

## Unit tests

All muFFT APIs have unit tests. muFFT output is verified against the FFTW library.
The convolution API is verified against a straight O(N^2) convolution.

To build the test suite, run

    make test
    ./mufft_test

The FFTW3 library must be present on your system via `pkg-config fftw3f --libs` when building this.
Note that FFTW3 (as of writing) is licensed under GPLv2+.
The `mufft_test` binary falls under licensing requirements of GPLv2 as per FFTW license.

## <a name="benchmark"></a>  Benchmark

muFFT can be benchmarked using FFTW as a reference.

Gflops values reported are based on the estimated number of flops consumed by a generic complex FFT, which is 5.0 * N * log2(N).
Values reported should be taken with a grain of salt, but it gives a reasonable estimate for throughput.
Average time consumed by a single FFT is reported as well.

To build the benchmark, run

    make bench
    ./mufft_bench 1000000 64  # 1 million iterations of various N = 64 FFTs variants
    ./mufft_bench 10000 64 64 # 10k iterations of 64-by-64 2D FFT
    ./mufft_bench # Run various 1D and 2D benchmarks

The benchmark for 1D tests various things:

 - Complex-to-complex transform
 - Real-to-complex and Complex-to-real in one iteration (typical convolution scenario)
 - Mono convolution, stereo convolution

The FFTW3 library must be present on your system via `pkg-config fftw3f --libs` when building this.
Note that FFTW3 (as of writing) is licensed under GPLv2+.
The `mufft_bench` binary falls under licensing requirements of GPLv2 as per FFTW3 license.


