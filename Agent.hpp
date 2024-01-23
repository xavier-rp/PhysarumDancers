#pragma once

#include <SFML/Graphics.hpp>

class Agent {

public:

	float radToDegreeConverter{3.141592f / 180.0f};
	float orientation {};
	float velocity{};
	sf::Vector2f velocity_vector{};
	sf::Vector2f position;
	float sensor_angle_offset{};
	int sensor_offset_distance{};
	int sensor_width{};

	Agent(sf::Vector2f position = { 0.0, 0.0 }, float orientation = 0.0, float velocity = 1.0, float sensor_angle_offset = 22.5, int sensor_offset_distance = 9, int sensor_width = 1) :
		position{ position },
		//orientation{ orientation },
		velocity{ velocity },
		//sensor_angle_offset{ sensor_angle_offset },
		sensor_offset_distance{ sensor_offset_distance },
		sensor_width{ sensor_width } {

		this->orientation = orientation * radToDegreeConverter;
		this->sensor_angle_offset = sensor_angle_offset * radToDegreeConverter;

		velocity_vector.x = velocity * std::cos(orientation);
		velocity_vector.y = velocity * std::sin(orientation);
	}

	float get_orientation() {
		return orientation;
	}
	
	float get_velocity() {
		return velocity;
	}

	sf::Vector2f get_position() {
		return position;
	}

	sf::Vector2f get_velocity_vector() {
		return velocity_vector;
	}

	void set_orientation(float new_orientation) {
		//Should also modify velocity vector and velocity
		orientation = new_orientation;
	}

	void set_velocity(float new_velocity) {
		//Should also modify orientation
		velocity = new_velocity;
	}

	void set_velocity_vector(sf::Vector2f new_velocity) {
		//Should also modify orientation
		velocity_vector = new_velocity;
		velocity = std::sqrt(velocity_vector.x * velocity_vector.x + velocity_vector.y * velocity_vector.y);
	}

	void set_x_velocity(float new_x_velocity) {
		velocity_vector.x = new_x_velocity;
	}

	void set_y_velocity(float new_y_velocity) {
		velocity_vector.y = new_y_velocity;
	}

	void set_position(sf::Vector2f new_position) {
		position = new_position;
	}

	void set_sensor_angle_offset(float new_angle) {
		sensor_angle_offset = new_angle;
	}

	void update_orientation(float new_orientation) {
		orientation = new_orientation;
		velocity_vector.x = velocity * std::cos(orientation);
		velocity_vector.y = velocity * std::sin(orientation);

	}

	void update_x_velocity(float new_x_velocity) {
		velocity_vector.x = new_x_velocity;
		if (velocity_vector.y <= 0.0 && velocity_vector.x < 0.0) {
			orientation = 3.141592f + atan(velocity_vector.y / velocity_vector.x);
		}
		else if (velocity_vector.y >= 0.0 && velocity_vector.x < 0.0) {
			orientation = 3.141592f + atan(velocity_vector.y / velocity_vector.x);
		}
		else if (velocity_vector.y >= 0.0 && velocity_vector.x > 0.0) {
			orientation = atan(velocity_vector.y / velocity_vector.x);
		}
		else if (velocity_vector.y <= 0.0 && velocity_vector.x > 0.0) {
			orientation = (2.0f * 3.141592f) + atan(velocity_vector.y / velocity_vector.x);
		}
	}

	void update_y_velocity(float new_y_velocity) {
		velocity_vector.y = new_y_velocity;
		if (velocity_vector.x != 0.0) {
			if (velocity_vector.y < 0.0 && velocity_vector.x < 0.0) {
				orientation = 3.141592f + atan(velocity_vector.y / velocity_vector.x);
			}
			else if (velocity_vector.y > 0.0 && velocity_vector.x < 0.0) {
				orientation = 3.141592f + atan(velocity_vector.y / velocity_vector.x);
			}
			else if (velocity_vector.y > 0.0 && velocity_vector.x > 0.0) {
				orientation = atan(velocity_vector.y / velocity_vector.x);
			}
			else if (velocity_vector.y < 0.0 && velocity_vector.x > 0.0) {
				orientation = (2.0f * 3.141592f) + atan(velocity_vector.y / velocity_vector.x);
			}
		}
		else {
			if (velocity_vector.y < 0.0) {
				orientation = 3.0 * 3.141592f / 2.0;
			}
			else {
				orientation = 3.141592f / 2.0;
			}
		}
	}



};