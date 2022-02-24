// Copyright (c) 2016-2019, XMOS Ltd, All rights reserved

#ifndef DSP_TRANSFORMS_H_
#define DSP_TRANSFORMS_H_

#include <stdint.h>
#include <dsp_complex.h>

extern const int32_t dsp_sine_4[];
extern const int32_t dsp_sine_8[];
extern const int32_t dsp_sine_16[];
extern const int32_t dsp_sine_32[];
extern const int32_t dsp_sine_64[];
extern const int32_t dsp_sine_128[];
extern const int32_t dsp_sine_256[];
extern const int32_t dsp_sine_512[];
extern const int32_t dsp_sine_1024[];
extern const int32_t dsp_sine_2048[];
extern const int32_t dsp_sine_4096[];
extern const int32_t dsp_sine_8192[];
extern const int32_t dsp_sine_16384[];

#define FFT_SINE0(N) dsp_sine_ ## N
#define FFT_SINE(N) FFT_SINE0(N)

/** This function splits the spectrum of the FFT of two real sequences. Takes
 * the result of a double-packed dsp_complex_t array that has undergone
 * an FFT. This function splits the result into two arrays, one for each real
 * sequence, of length N/2.
 * It is expected that the output will be cast by:
 *   dsp_complex_t (* restrict w)[2] = (dsp_complex_t (*)[2])pts;
 * or a C equlivent. The 2 dimensional array w[2][N/2] can now be used to access
 * the frequency information of the two real sequences independently, with the
 * first index denoting the corresponding real sequence and the second index denoting
 * the FFT frequency bin.
 * Note that the DC component of the imaginary output spectrum (index zero) will
 * contain the real component for the Nyquest rate.
 * Note the minimum N is 8.
 *
 * \param[in,out] pts   Array of dsp_complex_t elements.
 * \param[in]     N     Number of points. Must be a power of two.
 */
void dsp_fft_split_spectrum( dsp_complex_t pts[], const uint32_t N );

/** This function merges two split spectra. It is the exact inverse operation of
 * dsp_fft_split_spectrum. Note the minimum N is 8.
 *
 * \param[in,out] pts   Array of dsp_complex_t elements.
 * \param[in]     N     Number of points. Must be a power of two.
 */
void dsp_fft_merge_spectra( dsp_complex_t pts[], const uint32_t N );

/** This function copies an array of dsp_complex_short_t elements to an array of an equal
 * number of dsp_complex_t elements.
 *
 * \param[out]    l   Array of dsp_complex_t elements.
 * \param[in]     s   Array of dsp_complex_short_t elements.
 * \param[in]     N   Number of points.
 */
void dsp_fft_short_to_long( const dsp_complex_short_t s[], dsp_complex_t l[], const uint32_t N );

/** This function copies an array of dsp_complex_t elements to an array of an equal
 * number of dsp_complex_short_t elements.
 *
 * \param[out]    s   Array of dsp_complex_short_t elements.
 * \param[in]     l   Array of dsp_complex_t elements.
 * \param[in]     N   Number of points.
 */
void dsp_fft_long_to_short( const dsp_complex_t l[], dsp_complex_short_t s[], const uint32_t N );

/** This function preforms index bit reversing on the the arrays around prior to computing an FFT. A
 * calling sequence for a forward FFT involves dsp_fft_bit_reverse() followed by
 * dsp_fft_forward(), and for an inverse FFT it involves dsp_fft_bit_reverse() followed
 * by dsp_fft_inverse(). In some cases bit reversal can be avoided, for example
 * when computing a convolution.
 *
 * \param[in,out] pts   Array of dsp_complex_t elements.
 * \param[in]     N     Number of points. Must be a power of two.
 */
void dsp_fft_bit_reverse( dsp_complex_t pts[], const uint32_t N );

/** This function computes a forward FFT. The complex input signal is
 * supplied in an array of real and imaginary fixed-point values.
 * The same array is also used to store the output.
 * The magnitude of the FFT output is right shifted log2(N) times which corresponds to
 * division by N as shown in EQUATION 31-5 of ``http://www.dspguide.com/CH31.PDF``.
 * The number of points must be a power of 2, and the array of sine values should contain a quarter sine-wave.
 * Use one of the dsp_sine_N tables. The function does not perform bit reversed indexing of the input data.
 * If required then dsp_fft_bit_reverse() should be called beforehand.
 *
 * \param[in,out] pts   Array of dsp_complex_t elements.
 * \param[in]     N     Number of points. Must be a power of two.
 * \param[in]     sine  Array of N/4+1 sine values, each represented as a sign bit,
 *                      and a 31 bit fraction. 1 should be represented as 0x7fffffff.
 *                      Arrays are provided in dsp_tables.c; for example, for a 1024 point
 *                      FFT use dsp_sine_1024.
 */
void dsp_fft_forward (
    dsp_complex_t pts[],
    const uint32_t        N,
    const int32_t         sine[] );

/** This function computes an inverse FFT. The complex input array is
 * supplied as two arrays of integers, with numbers represented as
 * fixed-point values. Max input range is -0x3fffffff..0x3fffffff. 
 * Integer overflow can occur with inputs outside of this range.
 * The number of points must be a power of 2, and the
 * array of sine values should contain a quarter sine-wave. Use one of the
 * dsp_sine_N tables. The function does not perform bit reversed indexing of the input data.
 * if required then dsp_fft_bit_reverse() should be called beforehand.
 *
 * \param[in,out] pts   Array of dsp_complex_t elements.
 * \param[in]     N     Number of points. Must be a power of two.
 *
 * \param[in]     sine  Array of N/4+1 sine values, each represented as a sign bit,
 *                      and a 31 bit fraction. 1 should be represented as 0x7fffffff.
 *                      Arrays are provided in dsp_tables.c; for example, for a 1024 point
 *                      FFT use dsp_sine_1024.
 */
void dsp_fft_inverse (
    dsp_complex_t pts[],
    const uint32_t        N,
    const int32_t         sine[] );

/** This function computes a forward FFT of a real signal.
 *
 * The FFT is computed in place. On input, the array must contain N real
 * inputs (one int32_t each). On output, the array contains N/2 complex
 * outputs (two int32_t each). On output, bin K of the frequency spectrum
 * in locations 2*K and 2*K+1 (containing the real and imaginary parts
 * respectively), with the exception of bins 0 and N/2; the real part of
 * the DC term is stored in element 0, and the real part of the Nyquist
 * term is stored in element 1. The imaginary parts of both DC and Nyquist
 * terms are zero due to the input data being real.
 *
 * The array is not large enough to hold the complete spectrum, and only
 * the first half of the spectrum is written into it. Bins 1..N/2-1 are in
 * array elements 2..N-1. Bins N/2+1 ... N-1 (the cosine terms) can be
 * computed afterwards by conjugating terms N/2-1 ... 1; this symmetry is
 * due to the input being real.
 *
 * One way to use the data is to recast the array to an array of
 * dsp_complex_t afterwards.
 *
 * The array must be double word aligned.
 *
 * dsp_fft_bit_reverse() is integrated into this function and must not be 
 * called separately.
 *
 * Note that unlike the other FFT functions, two copies of the SIN array
 * must be supplied. The sin array that corresponds to N/2 and the array
 * that corresponds to N. hence, for N is 1024, the input should be 1024
 * integers, and sine should be dsp_sine_512, and ds_sine_1024.
 *
 * Performance considerations.
 *
 * This function is faster than computing a complex FFT of length N (about
 * 1.8x faster), but slower than computing two real FFTs at once using
 * dsp_fft_split_spectrum(). This function adds an overhead of N/2 bytes
 * for the extra sine-table. Hence, it is useful if the FFT of just a
 * single array is required.
 *
 * \param[in,out] pts   Array of N integers (in) array of N/2 dsp_complex_t
 *                      elements (out)
 * \param[in]     N     Number of points. Must be a power of two.
 * \param[in]     sine  Array of N/8+1 sine values, each represented as a 
 *                      sign bit and a 31 bit fraction. 1.0 is represented
 *                      by 0x7fffffff.
 *                      Arrays are provided in dsp_tables.c.
 *                      For example, for a 1024 point FFT use dsp_sine_512.
 * \param[in]     sin2  Array of N/4+1 sine values, represented as above.
 *                      For example, for a 1024 point FFT use dsp_sine_1024.
 */
void dsp_fft_bit_reverse_and_forward_real (
    int32_t pts[],
    const uint32_t        N,
    const int32_t         sine[],
    const int32_t         sin2[]
    );


/** This function computes an inverse FFT of a symmetric spectrum
 *
 * The FFT is computed in place. On input, the array must contain N/2 complex
 * inputs (two int32_t each). On output, the array contains N real
 * outputs (one int32_t each). On input, bin K of the frequency spectrum
 * in locations 2*K and 2*K+1 (containing the real and imaginary parts
 * respectively), with the exception of bins 0 and N/2; the real part of
 * the DC term is stored in element 0, and the real part of the Nyquist
 * term is stored in element 1. The imaginary parts of both DC and Nyquist
 * terms are zero due to the input data being real.
 *
 * The input array is not large enough to hold the complete spectrum, and only
 * the first half of the spectrum is contained into it. Bins 1..N/2-1 are in
 * array elements 2..N-1. Bins N/2+1 ... N-1 (the cosine terms) are assumed to
 * be the conjugated terms N/2-1 ... 1; this symmetry will guarantee that the
 * output array is real.
 *
 * One way to use the data is to recast the array to an array of
 * dsp_complex_t prior to the call.
 *
 * The array must be double word aligned.
 *
 * dsp_fft_bit_reverse() is integrated into this function and must not be 
 * called separately.
 *
 * Note that unlike the other FFT functions, two copies of the SIN array
 * must be supplied. The sin array that corresponds to N/2 and the array
 * that corresponds to N. hence, for N is 1024, the input should be 1024
 * integers, and sine should be dsp_sine_512, and ds_sine_1024.
 *
 * Performance considerations.
 *
 * This function is faster than computing a complex FFT of length N (about
 * 1.8x faster), but slower than computing two real FFTs at once using
 * dsp_fft_split_spectrum(). This function adds an overhead of N/2 bytes
 * for the extra sine-table. Hence, it is useful if the FFT of just a
 * single array is required.
 *
 * \param[in,out] pts   Array of N/2 dsp_complex_t (in) array of N int32_t
 *                      elements (out)
 * \param[in]     N     Number of points. Must be a power of two.
 * \param[in]     sine  Array of N/8+1 sine values, each represented as a 
 *                      sign bit and a 31 bit fraction. 1.0 is represented
 *                      by 0x7fffffff.
 *                      Arrays are provided in dsp_tables.c.
 *                      For example, for a 1024 point FFT use dsp_sine_512.
 * \param[in]     sin2  Array of N/4+1 sine values, represented as above.
 *                      For example, for a 1024 point FFT use dsp_sine_1024.
 */
void dsp_fft_bit_reverse_and_inverse_real (
    int32_t pts[],
    const uint32_t        N,
    const int32_t         sine[],
    const int32_t         sin2[]
    );
#endif

