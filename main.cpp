#include <iostream>
#include <cmath>
#include <random>
#include <fstream>
#include <vector>
#include <complex>

#include <fftw3.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "Grid.hpp"
#include "Agent.hpp"
#include "Simulation.hpp"
#include "Renderer.hpp"
#include "TrailMap.hpp"
#include "ColorMap.hpp"
#include "CustomAudioStream.hpp"
#include "SamplesRenderer.hpp"
#include "FrequencyRenderer.hpp"

#include "ColorGradient.hpp"

std::vector<Agent> build_list_of_agents_origin_radial(int nb_of_agents) {
	
	std::vector<Agent> list_of_agents;

	for (int i{ 0 }; i < nb_of_agents; i++) {
		float delta_theta = 360.0f / nb_of_agents;

		list_of_agents.push_back(Agent{ sf::Vector2f {0, 0}, i*delta_theta});
	}

	return list_of_agents;
}

std::vector<Agent> build_list_of_agents_random_in_grid(int nb_of_agents, Grid& grid) {

	std::vector<Agent> list_of_agents;

	std::random_device rd{};
	std::mt19937 gen{rd()};

	//std::normal_distribution<> d{0.0, 10.0};
	std::uniform_real_distribution<float> dis_x{grid.x_min, grid.x_max};
	std::uniform_real_distribution<float> dis_y{grid.y_min, grid.y_max};
	std::uniform_real_distribution<float> dis_theta{0.0, 360.0};

	for (int i{ 0 }; i < nb_of_agents; i++) {

		list_of_agents.push_back(Agent{ sf::Vector2f {dis_x(gen), dis_y(gen)}, dis_theta(gen)});
	}

	return list_of_agents;
}

std::vector<Agent> build_list_of_agents_random_on_circle(int nb_of_agents, float radius) {

	std::vector<Agent> list_of_agents;

	std::random_device rd{};
	std::mt19937 gen{rd()};

	//std::normal_distribution<> d{0.0, 10.0};
	std::uniform_real_distribution<float> dis_theta{0.0, 360.0};
	float theta{};

	for (int i{ 0 }; i < nb_of_agents; i++) {

		theta = dis_theta(gen);
		list_of_agents.push_back(Agent{ sf::Vector2f {cos(theta)*radius, sin(theta)* radius}, dis_theta(gen)});
	}

	return list_of_agents;
}

std::vector<Agent> build_list_of_agents_random_on_cat(int nb_of_agents, float x_span) {

	std::vector<Agent> list_of_agents;

	std::random_device rd{};
	std::mt19937 gen{rd()};

	//std::normal_distribution<> d{0.0, 10.0};
	std::uniform_real_distribution<float> dis_theta{0.0, 360.0};
	float theta{};
	float x_step{ x_span / static_cast<float>(nb_of_agents) };
	float current_x{};
	for (int i{ 0 }; i < nb_of_agents; i++) {
		current_x = -x_span / 2.0f + i * x_step;
		if (current_x < -x_span / 2.0f + x_span / 3.0f) {
			list_of_agents.push_back(Agent{ sf::Vector2f {current_x, -std::abs(current_x + (x_span / 2 - x_span / 6)) + x_span / 2.5f}, 0.0f, 0.0f });
		}
		else if (current_x > x_span / 6.0f) {
			list_of_agents.push_back(Agent{ sf::Vector2f {current_x, -std::abs(current_x - x_span / 3.0f) + x_span / 3.0f}, 0.0f, 0.0f });
		}
		else if (current_x > -x_span / 2.0f + x_span / 3.0f && current_x < 0.0f) {
			list_of_agents.push_back(Agent{ sf::Vector2f {current_x, 0.03f * (current_x - (-x_span / 2 + 5 * x_span / 12)) * (current_x - (-x_span / 2 + 5 * x_span / 12)) - x_span / 3}, 0.0f, 0.0f });
		}
		else if (current_x > 0.0f && current_x < x_span/6.0f) {
			list_of_agents.push_back(Agent{ sf::Vector2f {current_x, 0.03f * (current_x - x_span / 12) * (current_x - x_span / 12) - x_span/3}, 0.0f, 0.0f });
		}
	}

	x_step = x_span/8 / 500;
	for (int i{ 0 }; i < 500; i++) {
		current_x = -x_span / 16 + i * x_step;
		list_of_agents.push_back(Agent{ sf::Vector2f {current_x, std::abs(current_x)}, dis_theta(gen), 0.0f });
	}

	return list_of_agents;
}

std::vector<Agent> build_list_of_agents_random_inside_circle(int nb_of_agents, float radius) {

	std::vector<Agent> list_of_agents;

	std::random_device rd{};
	std::mt19937 gen{rd()};

	//std::normal_distribution<> d{0.0, 10.0};
	std::uniform_real_distribution<float> dis_theta{0.0, 360.0};
	std::uniform_real_distribution<float> dis_zero_one{0.0f, 1.0f};
	float theta{};

	for (int i{ 0 }; i < nb_of_agents; i++) {
		float radius_multiplicator{ dis_zero_one(gen) };
		theta = dis_theta(gen);
		list_of_agents.push_back(Agent{ sf::Vector2f {cos(theta)* radius * radius_multiplicator, sin(theta)* radius * radius_multiplicator}, dis_theta(gen) });
	}

	return list_of_agents;
}

std::vector<float> computeFrequencyAmplitudes(int currentIndex, int nbOfSamples, const sf::Int16* samples, sf::Uint64 sampleCount) {
	double* in1;
	double* in2;
	fftw_complex *out1;
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
		else if (centeringIndex + i >= sampleCount){
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

int main()
{

	std::vector<rgb> stops;

	stops.push_back(rgb{ 255,211,25 }); // red
	stops.push_back(rgb{ 255,144,31 }); // red
	stops.push_back(rgb{ 255,41,117 }); // green
	stops.push_back(rgb{ 242,34,255 }); // blue
	stops.push_back(rgb{ 140,30,255 }); // blue

	// Load audio file
	// load an audio buffer from a sound file
	int FFTLength = 8192;//16384;

	sf::SoundBuffer buffer;
	if (!buffer.loadFromFile("audio/Chupi.wav"))
	{
		std::cout << "Failed to load audio file!" << std::endl;
		return -1;
	}

	const sf::Int16* samples = buffer.getSamples();
	sf::Uint64 sampleCount = buffer.getSampleCount();
	unsigned int sampleRate = buffer.getSampleRate();

	std::vector<float> amplitudes;

	int window_width{ 600 };
	int window_height{ 400 };



	//ColorMap color_map{ 100, "purple_white", false, false, 1 };
	//ColorMap pulse_color_map{ 100, "green_white", false, false, 1 };

	//ColorMap color_map{ 100, "green_purple", true, false, 1 };
	//ColorMap pulse_color_map{ 100, "green_purple", false, false, 1 };

	//ColorMap color_map{ 100, "purple_white", false, false, 1 };
	//ColorMap color_map{ 1000, "green_purple_white", false, false, 1 };
	//ColorMap pulse_color_map{ 1000, "green_purple_white", false, false, 1};

	//ColorMap color_map{ 1000, "jet", true, false, 1 };
	ColorMap color_map{ 1000, stops, true, false, 1 };



	TrailMap trail_map(window_width, window_height);

	Grid grid(static_cast<float>(window_width), static_cast<float>(window_height));

	sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Physarum.exe");

	std::vector<Agent> list_of_agents;

	//list_of_agents = build_list_of_agents_random_on_circle(10000, window_width / 5.0f);
	list_of_agents = build_list_of_agents_random_on_cat(10000, window_width / 2.0f);
	//list_of_agents = build_list_of_agents_random_inside_circle(40000, window_width / 5.0f);
	//list_of_agents = build_list_of_agents_uniform_circle(10000);
	//list_of_agents = build_list_of_agents_random_in_grid(5000, grid);

	Simulation sim{ grid, list_of_agents, trail_map };
	sim.set_FFT_parameters(sampleRate, FFTLength);

	Renderer renderer{ sim, grid, color_map };

	// initialize and play our custom stream
	MyStream playing_stream;
	playing_stream.load(buffer);

	// Start playing the sound
	playing_stream.play();
	sf::Clock globalClock;
	sf::Clock loopClock;
	sf::Time elapsedLoop = loopClock.getElapsedTime();
	sf::Clock clockFrequency;
	sf::Time elapsedFrequency = clockFrequency.getElapsedTime();
	int currentSampleIndex{};

	sim.perturbation_flag = true;
	// Main loop
	while (window.isOpen())
	{
		// Handle events
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}


		renderer.render_trail_map();
		//renderer.render_agents();
		window.clear(sf::Color::Black);
		window.draw(renderer.trail_map_vertices);
		//window.draw(renderer.agent_vertices);
		window.display();

		if (playing_stream.getStatus() == sf::SoundSource::Playing) {
			sim.set_amplitudes(computeFrequencyAmplitudes(globalClock.getElapsedTime().asSeconds() * sampleRate * 2, FFTLength, samples, sampleCount));
			//sim.set_amplitudes(computeFrequencyAmplitudes(playing_stream.getPlayingOffset() * sampleRate * 2, FFTLength, samples, sampleCount));
		}
		else {

			sim.set_amplitudes(std::vector<float>(100, 0.0));
		}
		//sim.set_amplitudes(computeFrequencyAmplitudes(globalClock.getElapsedTime().asSeconds() * sampleRate * 2, FFTLength, samples));//(currentSampleIndex, 16384, samples);

		//std::cout << sim.stepCount << "  " << sim.stepCount % 100 << std::endl;
		//if (elapsedLoop.asSeconds() > 0.1) { // && sim.stepCount % 26 >= 25
		//	loopClock.restart();
		//	sim.perturbation_flag = true;
		//}
		//sim.set_amplitudes(computeFrequencyAmplitudes(globalClock.getElapsedTime().asSeconds() * sampleRate * 2, FFTLength, samples));//(currentSampleIndex, 16384, samples);
		//sim.compute_band_energy();

		sim.step();
		//sim.perturbation_flag = false;
		elapsedLoop = loopClock.getElapsedTime();
		// Clear the window
		/*
		window.clear();

		currentSampleIndex = playing_stream.getCurrentSampleIndex(playing_stream.getPlayingOffset());
		if (elapsedSamples.asSeconds() > 1.0f / 60.0f) {
			samplesRenderer.renderSamples(globalClock.getElapsedTime().asSeconds() * sampleRate);
			clockSamples.restart();
		}
		if (elapsedFrequency.asSeconds() > 1.0f / 48.0f) {
			amplitudes = computeFrequencyAmplitudes(globalClock.getElapsedTime().asSeconds() * sampleRate * 2, FFTLength, samples);//(currentSampleIndex, 16384, samples);
			frequencyRenderer.renderFrequencies(amplitudes);
			clockFrequency.restart();
		}


		window.clear(sf::Color::Black);

		// Display the window
		window.display();
		*/

	}

	return 0;
}

int main_cat()
{

	std::vector<rgb> stops;

	stops.push_back(rgb{ 255,211,25 }); // red
	stops.push_back(rgb{ 255,144,31 }); // red
	stops.push_back(rgb{ 255,41,117 }); // green
	stops.push_back(rgb{ 242,34,255 }); // blue
	stops.push_back(rgb{ 140,30,255 }); // blue

	// Load audio file
	// load an audio buffer from a sound file
	int FFTLength = 8192;//16384;

	sf::SoundBuffer buffer;
	if (!buffer.loadFromFile("audio/Chupi.wav"))
	{
		std::cout << "Failed to load audio file!" << std::endl;
		return -1;
	}

	const sf::Int16* samples = buffer.getSamples();
	sf::Uint64 sampleCount = buffer.getSampleCount();
	unsigned int sampleRate = buffer.getSampleRate();

	std::vector<float> amplitudes;

	int window_width{ 640 };
	int window_height{ 480 };

	ColorMap color_map{ 1000, stops, true, false, 1 };

	TrailMap trail_map(window_width, window_height);

	Grid grid(static_cast<float>(window_width), static_cast<float>(window_height));

	sf::RenderWindow window(sf::VideoMode(window_width, window_height), "CursedCat.EXE");

	std::vector<Agent> list_of_agents;

	list_of_agents = build_list_of_agents_random_on_cat(5000, window_width / 2.0f);

	Simulation sim{ grid, list_of_agents, trail_map };
	sim.set_FFT_parameters(sampleRate, FFTLength);

	Renderer renderer{ sim, grid, color_map };

	// initialize and play our custom stream
	MyStream playing_stream;
	playing_stream.load(buffer);

	// Start playing the sound
	playing_stream.play();
	sf::Clock globalClock;
	sf::Clock loopClock;
	sf::Time elapsedLoop = loopClock.getElapsedTime();
	sf::Clock clockFrequency;
	sf::Time elapsedFrequency = clockFrequency.getElapsedTime();
	int currentSampleIndex{};

	sim.perturbation_flag = true;

	bool wait_flag = true;
	// Main loop
	while (window.isOpen())
	{
		// Handle events
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		renderer.render_trail_map();
		//renderer.render_agents();
		window.clear(sf::Color::Black);
		window.draw(renderer.trail_map_vertices);
		//window.draw(renderer.agent_vertices);
		window.display();
		if (elapsedLoop.asSeconds() > 15.0) {
			if (wait_flag) {
				for (int i = 0; i < list_of_agents.size(); i++) {
					sim.list_of_agents[i].set_velocity_vector(sf::Vector2f(1.0f, 0.0f));
				}
				wait_flag = false;
			}
			if (playing_stream.getStatus() == sf::SoundSource::Playing) {
				sim.set_amplitudes(computeFrequencyAmplitudes(globalClock.getElapsedTime().asSeconds() * sampleRate * 2, FFTLength, samples, sampleCount));
			}
			else {

				sim.set_amplitudes(std::vector<float>(100, 0.0));
			}

		}

		sim.step();

		if (wait_flag) {
			if (sim.stepCount % 60 < 30) {
				for (int i = 0; i < list_of_agents.size(); i++) {
					sim.list_of_agents[i].set_position(sim.list_of_agents[i].get_position() + sf::Vector2f(-2.0f, 1.0f));
				}
			}
			else {
				for (int i = 0; i < list_of_agents.size(); i++) {
					sim.list_of_agents[i].set_position(sim.list_of_agents[i].get_position() + sf::Vector2f(2.0f, -1.0f));
				}
			}
		}

		elapsedLoop = loopClock.getElapsedTime();		

	}

	return 0;
}

int main2()
{
	// Load audio file
	// load an audio buffer from a sound file
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	int FFTLength = 16384;

	sf::SoundBuffer buffer;
	if (!buffer.loadFromFile("audio/lm.wav"))
	{
		std::cout << "Failed to load audio file!" << std::endl;
		return -1;
	}

	const sf::Int16* samples = buffer.getSamples();
	sf::Uint64 sampleCount = buffer.getSampleCount();
	unsigned int sampleRate = buffer.getSampleRate();

	std::vector<float> amplitudes;

	int window_width{ 1900 };
	int window_height{ 1080 };

	Grid grid(static_cast<float>(window_width), static_cast<float>(window_height));

	int nbSamplesToDraw = 44100/4;// 22050;//11025;
	Grid grid2(6400.0f, 540.0f);
	SamplesRenderer samplesRenderer{ buffer, grid2, nbSamplesToDraw};
	Grid grid3(1900.0f, 540.0f);
	FrequencyRenderer frequencyRenderer{ grid3, static_cast<int>(sampleRate) };

	// initialize and play our custom stream
	MyStream playing_stream;
	playing_stream.load(buffer);

	// Create window for visualization
	sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Samples");

	// Start playing the sound
	playing_stream.play();
	sf::Clock globalClock;
	sf::Clock clockSamples;
	sf::Time elapsedSamples = clockSamples.getElapsedTime();
	sf::Clock clockFrequency;
	sf::Time elapsedFrequency = clockFrequency.getElapsedTime();
	int currentSampleIndex{};

	// Main loop
	while (window.isOpen())
	{
		// Handle events
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		// Clear the window
		
		window.clear();

		currentSampleIndex = playing_stream.getCurrentSampleIndex(playing_stream.getPlayingOffset());
		if (elapsedSamples.asSeconds() > 1.0f / 60.0f) {
			samplesRenderer.renderSamples(globalClock.getElapsedTime().asSeconds() * sampleRate);
			clockSamples.restart();
		}
		if (elapsedFrequency.asSeconds() > 1.0f/48.0f) {
			amplitudes = computeFrequencyAmplitudes(globalClock.getElapsedTime().asSeconds() * sampleRate * 2, FFTLength, samples, sampleCount);//(currentSampleIndex, 16384, samples);
			frequencyRenderer.renderFrequencies(amplitudes);
			clockFrequency.restart();
		}

		
		window.clear(sf::Color::Black);
		window.draw(frequencyRenderer.verticesToDraw);
		window.draw(samplesRenderer.chunkToDraw1);
		window.draw(samplesRenderer.chunkToDraw2);

		// Display the window
		window.display();
		elapsedFrequency = clockFrequency.getElapsedTime();
		elapsedSamples = clockSamples.getElapsedTime();

	}

	return 0;
}

int main3() {
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	sf::Music music;

	if (!music.openFromFile("audio/lm.wav"))
		return -1; // error
	music.play();
	

	
	std::cout << "Running main \n";

	int window_width{ 500 };
	int window_height{ 200 };

	ColorMap color_map{ 100, "purple_white", false, false, 1 };

	TrailMap trail_map(window_width, window_height);

	Grid grid(static_cast<float>(window_width), static_cast<float>(window_height));

	sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Physarum.exe");

	std::vector<Agent> list_of_agents;

	//list_of_agents = build_list_of_agents_random(10000, grid);
	//list_of_agents = build_list_of_agents_uniform_circle(100);
	
	/*
	int window_width{ 500 };
	int window_height{ 200 };
	list_of_agents = build_list_of_agents_random_inside_circle(30000, window_width / 5.0f);
	*/

	list_of_agents = build_list_of_agents_random_inside_circle(20, window_width / 5.0f);


	Simulation sim{grid, list_of_agents, trail_map};

	Renderer renderer{ sim, grid, color_map}; 

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		renderer.render_trail_map();
		//renderer.render_agents();
		window.clear(sf::Color::Black);
		window.draw(renderer.trail_map_vertices);
		//window.draw(renderer.agent_vertices);
		window.display();
		sim.step();
	}

	return 0;
}