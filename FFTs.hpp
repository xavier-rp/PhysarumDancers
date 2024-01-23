#pragma once
#include <vector>

#include <SFML/Audio.hpp>
#include "fftw3.h"



std::vector<float> computeFrequencyAmplitudes(int currentIndex, int nbOfSamples, const sf::Int16* samples, sf::Uint64 sampleCount) {
	double* in1;
	double* in2;
	fftw_complex* out1;
	fftw_complex* out2;
	fftw_plan p1;
	fftw_plan p2;

	in1 = (double*)fftw_malloc(sizeof(double) * nbOfSamples);
	in2 = (double*)fftw_malloc(sizeof(double) * nbOfSamples);
	out1 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * nbOfSamples);
	out2 = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * nbOfSamples);

	int centeringIndex{ 0 };
	for (int i = 0; i < nbOfSamples; i++) {
		centeringIndex = currentIndex;// - nbOfSamples / 2;
		if (centeringIndex < 0) {
			in1[i] = static_cast<double>(0);
			in2[i] = static_cast<double>(0);
		}
		else if (centeringIndex + i >= sampleCount) {
			in1[i] = static_cast<double>(0.0);
			in2[i] = static_cast<double>(0.0);
		}
		else {
			in1[i] = static_cast<double>(samples[centeringIndex + 2 * i]);
			in2[i] = static_cast<double>(samples[centeringIndex + 2 * i + 1]);
		}

	}
	p1 = fftw_plan_dft_r2c_1d(nbOfSamples, in1, out1, FFTW_ESTIMATE);

	fftw_execute(p1);

	p2 = fftw_plan_dft_r2c_1d(nbOfSamples, in2, out2, FFTW_ESTIMATE);

	fftw_execute(p2);

	std::vector<float> amplitudes;

	for (int i = 0; i < nbOfSamples / 2; i++) {
		double amplitude1{ sqrt(out1[i][0] * out1[i][0] + out1[i][1] * out1[i][1]) };
		double amplitude2{ sqrt(out2[i][0] * out2[i][0] + out2[i][1] * out2[i][1]) };
		amplitudes.push_back(static_cast<float>((1.0 / nbOfSamples) * (amplitude1 + amplitude2)));
		//amplitudes.push_back(static_cast<float>((1.0 / nbOfSamples) * sqrt(out1[i][0] * out1[i][0] + out1[i][1] * out1[i][1])));
	}

	fftw_destroy_plan(p1);
	fftw_free(in1);
	fftw_free(out1);
	fftw_destroy_plan(p2);
	fftw_free(in2);
	fftw_free(out2);

	return amplitudes;
}