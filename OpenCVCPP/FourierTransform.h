#pragma once

#include <complex>
#include <iostream>
#include <valarray>

const double PI = 3.141592653589793238460;

typedef std::complex<double> CComplex;
typedef std::valarray<CComplex> CArray;
void DFT(int dir, int N, double *x, double *y);

void DFT(int dir, int N, double *x, double *y)
{
	double *pRea = NULL;
	double *pImg = NULL;
	double cosarg, sinarg;
	pRea = (double*) malloc(sizeof(double) * N);
	pImg = (double*) malloc(sizeof(double) * N);
	for (size_t i = 0; i < N; i++)
	{
		pRea[i] = 0;
		pImg[i] = 0;
		// e ^ - (j * 2 * pi * m * n / N)
		double arg = -dir * 2.0 * PI * (double)i / (double)N;
		for (size_t k = 0; k < N; k++)
		{
			cosarg = cos(k * arg);
			sinarg = sin(k * arg);
			pRea[i] += (x[k] * cosarg) - (x[k] * sinarg);
			pImg[i] += (x[k] * sinarg) + (y[k] * cosarg);
		}
	}
	if (dir == 1) {
		for (size_t i = 0; i < N; i++)
		{
			x[i] = pRea[i] / (double)N;
			y[i] = pImg[i] / (double)N;
		}
	}
	else
	{
		for (size_t i = 0; i < N; i++)
		{
			x[i] = pRea[i];
			y[i] = pImg[i];
		}
	}

	free(pRea);
	free(pImg);
}



// Cooley�Tukey FFT (in-place, divide-and-conquer)
// Higher memory requirements and redundancy although more intuitive
void fft(CArray& x)
{
	const size_t N = x.size();
	if (N <= 1) return;

	// divide
	CArray even = x[std::slice(0, N / 2, 2)];
	CArray  odd = x[std::slice(1, N / 2, 2)];

	// conquer
	fft(even);
	fft(odd);

	// combine
	for (size_t k = 0; k < N / 2; ++k)
	{
		CComplex t = std::polar(1.0, -2 * PI * k / N) * odd[k];
		x[k] = even[k] + t;
		x[k + N / 2] = even[k] - t;
	}
}

void my_fft(CArray& x) {
	const size_t N = x.size();
	if (N <= 1) return;

	// Divide
	CArray even(N / 2);
	CArray odd(N / 2);
	for (size_t i = 0; i < N; i++)
	{
		if (i % 2 == 0) {
			even[i / 2] = x[i];
		}
		else odd[i / 2] = x[i];
	}
	my_fft(even);
	my_fft(odd);
}

// Cooley-Tukey FFT (in-place, breadth-first, decimation-in-frequency)
// Better optimized but less intuitive
void fft2(CArray &x)
{
	// DFT
	unsigned int N = x.size(), k = N, n;
	double thetaT = 3.14159265358979323846264338328L / N;
	CComplex phiT = CComplex(cos(thetaT), sin(thetaT)), T;
	while (k > 1)
	{
		n = k;
		k >>= 1;
		phiT = phiT * phiT;
		T = 1.0L;
		for (unsigned int l = 0; l < k; l++)
		{
			for (unsigned int a = l; a < N; a += n)
			{
				unsigned int b = a + k;
				CComplex t = x[a] - x[b];
				x[a] += x[b];
				x[b] = t * T;
			}
			T *= phiT;
		}
	}
	// Decimate
	unsigned int m = (unsigned int)log2(N);
	for (unsigned int a = 0; a < N; a++)
	{
		unsigned int b = a;
		// Reverse bits
		b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
		b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
		b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
		b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
		b = ((b >> 16) | (b << 16)) >> (32 - m);
		if (b > a)
		{
			CComplex t = x[a];
			x[a] = x[b];
			x[b] = t;
		}
	}
	//// Normalize (This section make it not working correctly)
	//CComplex f = 1.0 / sqrt(N);
	//for (unsigned int i = 0; i < N; i++)
	//	x[i] *= f;
}

// inverse fft (in-place)
void ifft(CArray& x)
{
	// conjugate the CComplex numbers
	x = x.apply(std::conj);

	// forward fft
	fft(x);

	// conjugate the CComplex numbers again
	x = x.apply(std::conj);

	// scale the numbers
	x /= x.size();
}

