#pragma once
#include <iostream>
#include <fstream>
#include <vector>

#include <fftw3.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "Grid.hpp"
#include "Agent.hpp"
#include "AgentCreationUtils.hpp"
#include "Simulation.hpp"
#include "Renderer.hpp"
#include "TrailMap.hpp"
#include "ColorMap.hpp"
#include "CustomAudioStream.hpp"
#include "FFts.hpp"


#include "ColorGradient.hpp"

std::vector<Agent> build_list_of_agents_origin_radial(int nb_of_agents) {

	std::vector<Agent> list_of_agents;

	for (int i{ 0 }; i < nb_of_agents; i++) {
		float delta_theta = 360.0f / nb_of_agents;

		list_of_agents.push_back(Agent{ sf::Vector2f {0, 0}, i * delta_theta });
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

		list_of_agents.push_back(Agent{ sf::Vector2f {dis_x(gen), dis_y(gen)}, dis_theta(gen) });
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
		list_of_agents.push_back(Agent{ sf::Vector2f {cos(theta)* radius, sin(theta)* radius}, dis_theta(gen) });
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
		else if (current_x > 0.0f && current_x < x_span / 6.0f) {
			list_of_agents.push_back(Agent{ sf::Vector2f {current_x, 0.03f * (current_x - x_span / 12) * (current_x - x_span / 12) - x_span / 3}, 0.0f, 0.0f });
		}
	}

	x_step = x_span / 8 / 500;
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
		list_of_agents.push_back(Agent{ sf::Vector2f {cos(theta)* radius* radius_multiplicator, sin(theta)* radius* radius_multiplicator}, dis_theta(gen) });
	}

	return list_of_agents;
}

int main_cat()
{
	/* 
	For the Cursed Cat animation.

	Replace main by this fuction.
	
	*/
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