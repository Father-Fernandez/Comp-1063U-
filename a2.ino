/*
* CSCI 1063U - Elegoo Smart Car V4.0
*
* Starter Code for motor, pin, and sensor setup
* Provided to students for use and understanding
* 
*/

#include "config.hpp"
#include "pins.hpp"
#include "utils.hpp"

#include "roaming.hpp"
#include "following.hpp"
#include "searching.hpp"
#include "line_following.hpp"
#include "sticky_following.hpp"
extern int state;
extern int line_exit_count;
SensorData sensor_data;

void setup() {
	setupArduino();
	
	// Wait for button press
	while (digitalRead(BUTTON) == HIGH) {
		Serial.println("Button not pressed");
	}
	
	delay(500);
	
	// Initialize Gyro - hard stop if failed
	if (!setupGyro()) {
		Serial.println("Failed setting up gyro...");
		ledOn(CRGB::Red);
		while (true);  // Hard stop
	}
	
	calibrateGyro();
	
	// Custom
	randomSeed(analogRead(0));
	
	print("MAIN", "setup done", 0);
}

// STATES
// -1 TESTING PURPOSES (I'm sick and tired of the motor noises)
// 0 ROAMING
// 1 FOLLOWING
// 2 SEARCHING
// 3 FOLLOW LINE

int get_distance_at_angle_(int angle, int delay)
{
  setServoAngle(angle, delay);
	return getDistance();
}

void drive()
{
	updateGyroAngle();

	int left = analogRead(LINE_L);
  int center = analogRead(LINE_C);
  int right = analogRead(LINE_R);

  bool on_center = center < LINE_THRESHOLD;
  bool on_left = left < LINE_THRESHOLD;
  bool on_right = right < LINE_THRESHOLD;

	if (on_center && on_left && on_right) {
		state = 1;
  }

	move_motors(NORMAL_SPEED, NORMAL_SPEED);

	print("main", "distance", sensor_data.avg_distance);

	if (sensor_data.avg_distance < MIN_DISTANCE)
	{
		move_motors(0, 0);

		int right_turn = 0;
		int left_turn = 0; 

		int right = get_distance_at_angle_(0, 1000);
		int left = get_distance_at_angle_(180, 1000);
		centerServo(250);

		print("sen", "l", left);
		print("sen", "r", right);

		if (left > MIN_DISTANCE + 5)
		{
			resetAngle();
			gyro_turn(-85);
			left_turn = 0;
			right_turn = 0;
		}

		else if (right < left)
		{
			resetAngle();
			gyro_turn(-85);
			left_turn ++;
			right_turn = 0;
		}

		else if (left < right)
		{
			resetAngle();
			gyro_turn(90);
			right_turn ++;
			left_turn = 0;
		}
	}
}

void follow_line_()
{
  int left = analogRead(LINE_L);
  int center = analogRead(LINE_C);
  int right = analogRead(LINE_R);

  bool on_center = center < LINE_THRESHOLD;
  bool on_left = left < LINE_THRESHOLD;
  bool on_right = right < LINE_THRESHOLD;

  move_motors(0, 0);
	int adj = 20;

  if (on_center && on_left && on_right) {
    move_motors(NORMAL_SPEED - adj, NORMAL_SPEED - adj);
    print("motor", "path", 0);
    // move_motors(SPEED_NORMAL, SPEED_NORMAL);
  }

  else if (on_left && !on_right) {
    move_motors(NORMAL_SPEED - adj, -NORMAL_SPEED + adj);
    print("motor", "path", -1);
    // move_motors(-SPEED_TURN, SPEED_TURN);
  }

  else if (on_right && !on_left) {
    move_motors(-NORMAL_SPEED + adj, NORMAL_SPEED - adj);
    print("motor", "path", 1);
    // move_motors(-SPEED_TURN, SPEED_TURN);
  }

  if (!on_center && !on_left && !on_right) {
		// resetAngle();
    // gyro_turn(180);
		line_exit_count++;
		if (line_exit_count >= 2) {
			state = 2;
			return;
		}
		else {
			state = 0;
		}
  }
}

void loop()
{
	sensor_data = update_sensors();

	// print("a", ": ", get_distance_at_angle_(0, 2000));
	// print("b", ": ", get_distance_at_angle_(180, 2000));

	// resetAngle();
	// gyro_turn(90);
	// delay(1000);
	// resetAngle();
	// gyro_turn(90);
	// delay(1000);

	switch (state)
	{
	case 0:
		drive();
		break;
	
	case 1:
		follow_line_();
		break;
	
	case 2:
		freak_out();
		state = 0;
		break;
	
	default:
		break;
	}
}
