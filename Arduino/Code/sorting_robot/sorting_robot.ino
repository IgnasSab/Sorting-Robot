// RGB sensor 
#include <Wire.h>
#include "Adafruit_TCS34725.h"

// Initialise with specific int time and gain values
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_60MS, TCS34725_GAIN_16X);

// Servo 
#include <Servo.h>
Servo push_servo;
Servo sort_servo;

// Raspberry pi communication (creating an object)
#include "RPI_class.h"
RPI_COM rpi_com;

// RGB sensor variables
unsigned int r, g, b, c, colorTemp, lux;
unsigned int c_maximum;
unsigned int peak_c;
const unsigned int number_of_values = 30;
unsigned int values_stored = 0;

// Check Errors
bool error_checking = true;

// Servo Motor variables
#define servo_push_pin 9
#define angle_push_neutral 40

#define servo_sort_pin 10
#define angle_sort_black 70 // Angle to sort black disks
#define angle_sort_white 150 // Angle to sort white disks
#define angle_sort_neutral 25 // Neutral angle

// Light Resistor Variables && rotary encoder Variables
#define light_resistor_pin A0
#define light_threshold 400
int time_since_last_pass = 0;
int light_value;
int past_light_value = 0;

// Joystick variables
#define joystick_button_pin 11
#define joystick_x_pin A2
int joystick_button_value;
int joystick_x_value;

// Laser Variables
#define laser_pin 8

// LED variables
#define error_LED_pin 4
#define manual_LED_pin 3
#define automatic_LED_pin 2

// Button pins
#define mode_button_pin 6
#define restart_button_pin 7

// Time Variables in milliseconds
#define rotary_delay 60
#define max_time_rotary 3000
#define delay_before_reset 5000
#define max_time_sorting 3000
#define max_time_to_reach_sensor (max_time_sorting - 300)
int time_passed;

// Sorting variable
bool sorting_disk;
 
// Function headers
void debugging();
void push_disk();
void(* reset) (void) = 0; // Reset function for arduino
void read_joystick_values();
void in_out_manual_override();
void manual_override();
int calculate_angle_manual();
void wait_for_release(int button_pin, int initial_value);
unsigned long time_for_release(int button_pin, int initial_value);
void detecting_errors();
void rotary_detector();
void check_sorting();
void check_blocking();
void error_detected(int error);
void read_RGB_values();
bool disk_passing();

// MAIN SETUP
void setup() {

  // LED configuration
  pinMode(error_LED_pin, OUTPUT);
  digitalWrite(error_LED_pin, LOW);
  pinMode(manual_LED_pin, OUTPUT);
  digitalWrite(manual_LED_pin, LOW);
  pinMode(automatic_LED_pin, OUTPUT);
  digitalWrite(automatic_LED_pin, LOW);

  // Servo Configuration
  // Push
  pinMode(servo_push_pin, OUTPUT);
  push_servo.attach(servo_push_pin); 
  push_servo.write(angle_push_neutral); // Initialize Servo Motor to neutral state
  // Sort
  pinMode(servo_sort_pin, OUTPUT);
  sort_servo.attach(servo_sort_pin); 
  sort_servo.write(angle_sort_neutral); // Initialize Servo Motor to neutral state

  // Light Resistor Configuration
  pinMode(light_resistor_pin, INPUT);

  // Joystick Configuration
  pinMode(joystick_x_pin, INPUT);
  pinMode(joystick_button_pin, INPUT);
  digitalWrite(joystick_button_pin, HIGH);

  // Laser Configuration
  pinMode(laser_pin, OUTPUT);
  digitalWrite(laser_pin, HIGH); // Turn Laser ON/OFF (HIGH/LOW)

  // Button configuration
  pinMode(mode_button_pin, INPUT);
  pinMode(restart_button_pin, INPUT);

  // Setup Serial Monitor
  Serial.begin(9600);
  
  // RGB sensor configuration
  // Initiate TCS3472 IC
  if (tcs.begin()) {
    //Serial.println("Found TCS3472 sensor");
  } else {
    //Serial.println("No TCS34725 found ... check your connections");
    while (1);
  } 

  // Send first packet
  rpi_com.sendPacket();

  // Waiting for a press of a button 
  while (!button_pressed(mode_button_pin)) {}

  // Tell the rpi to signal to the other arduino to turn on the conveyor
  rpi_com.running = true;

  // If the mode button is held for more than 1 second while starting
  if(time_for_release(mode_button_pin, 1) > 1000) {  
    in_out_manual_override();
  }

  rpi_com.sorting = 1;
  
  digitalWrite(automatic_LED_pin, HIGH);
  delay(1000); // Have a small delay before start


  //Serial.println("******* STARTING PROCESS *******");
}

// MAIN LOOP
void loop() {
  
  // Initialize the variable to signify that the disk is not being sorted
  sorting_disk = false;
  
  // Reset variables (global timer, color value, sorting servo motor position)
  // Color value is the only value used to distinguish between black/white/misc)
  values_stored = 0;
  time_passed = 0;
  c_maximum = 0;
  sort_servo.write(angle_sort_neutral);  
  
  // Push the disks from the cylinder onto the conveyor belt (120 ms) & signify it via the terminal
  push_disk();

  // Loop for one disk sorting iteration (constant amount of time always)
  while (time_passed < max_time_sorting) {

    if (button_pressed(restart_button_pin)) {
      // Restart the arduino
      restart_arduino();
    } else if (button_pressed(mode_button_pin)) {
      wait_for_release(mode_button_pin, 1);
      in_out_manual_override();
      break;
    }

    
    // Read RGB values. @code{c} here indicates the sum of all @code{r, g, b} values.
    read_RGB_values();
    // If a disk is currently passing AND there is no disk already being sorted
    // AND the threshold for the number of values stored was not reached
    // then update the maximum c value.
    if(disk_passing() && !sorting_disk && values_stored < number_of_values) {
      // Take @code{c_maximum} to be the maximum of both, the current value of c (@code{c_maximum}) and the new value of c 
      // (@code{c}) that just read from the color sensor.
      c_maximum = max(c_maximum, c);
      values_stored++;
    }

    // If no disk is being sorted and either there is no longer a disk passing OR the max number 
    // of values has been stored then sort the disk based on the maximum value of @code{c} detected.
    if (!sorting_disk) {
      if ((values_stored > 0 && !disk_passing()) || (values_stored >= number_of_values)) {
        sort();
        sorting_disk = true;
      }
    }

    // Send sorting status to the RPI
    rpi_com.sendPacket();

    // Detects if there are any types of errors (we take into account 3 different ones)
    if (error_checking) {
      detecting_errors();
    }

    // Update the time passed (only RGB sensor counts)
    time_passed += rotary_delay;
  } 

}

/**
 * Checks if the button is pressed or not
 * @param pin the io pin for the button
 * @return the state of the button
 */
bool button_pressed(int pin) {
  return digitalRead(pin);
}

/**
 * This function checks 3 different types of errors.
 *
 * Error types:
 * 1. Conveyor belt is stuck.
 * 2. Disk was pushed, but no disk was found by RGB sensor.
 * 3. Light is too low OR something is blocking the RGB sensor.
 */
void detecting_errors() {

  // 1.
  rotary_detector();

  // 2.
  check_sorting();

  // 3.
  check_blocking();

}

/*
 * This function lets us detect whether the conveyor belt is stuck or not (moving or stationary)
 * It reads the light values via LDR every once in a while and restarts the arduino if the light value read
 * had not changed for more than max_time_rotary milliseconds.
 */
void rotary_detector() {
  
  // rotary encoder functionality
  light_value = analogRead(light_resistor_pin);  

  // If the light value has changed, reset the timer
  if ((past_light_value < light_threshold && light_value > light_threshold) 
    || past_light_value > light_threshold && light_value < light_threshold) {
    time_since_last_pass = 0;
  } 
  
  // No change in a long time 
  if (time_since_last_pass > max_time_rotary) {
    // Detected error nr 1.
    error_detected(1);
  }

  // Update past light value and the time passed after no change
  past_light_value = light_value;
  time_since_last_pass += rotary_delay;

}

/**
 * Check the sensor detected no disk when one should have been detected.
 */
void check_sorting() {
  if (!sorting_disk && time_passed >= max_time_to_reach_sensor) {
    // Detected error nr 2.
    error_detected(2);
  }
}

/** 
 * Check if something is blocking the color sensor or if the light is too low.
 */
void check_blocking() {
  if (colorTemp == 0) {
    // Detected error nr 3.
    error_detected(3);
  }
}

/**
 * This function sets up the lights and information of the RPI_COM object
 * to prepare for and to leave the manual override functionality.
*/
void in_out_manual_override() {
  digitalWrite(automatic_LED_pin, LOW);
  digitalWrite(manual_LED_pin, HIGH);

  rpi_com.sorting = 2;
  rpi_com.sendPacket();

  // Perform the manual override functionality
  manual_override();
  
  rpi_com.sorting = 1;
  rpi_com.sendPacket();

  digitalWrite(manual_LED_pin, LOW);
  delay(3000); // Gives time for any disks currently on the belt to clear off
  digitalWrite(automatic_LED_pin, HIGH);
}

/** 
 * This function serves as a manual override functionality for our robot
 * in which a human being can control the sorting servo motor with a joystick. 
 */
void manual_override() {
  // local variables needed for this function;
  int angle_sort_manual;
  int full_delay = 25;

  while(true) {
    if (button_pressed(restart_button_pin)) {
      restart_arduino();
    } else if (button_pressed(mode_button_pin)) {
      return;
    }

    // Read joystick values
    read_joystick_values();

    // Check if the joystick button was pressed and push a disk if so
    if (joystick_button_value == 0) {
      wait_for_release(joystick_button_pin, 0);
      push_disk();
    }

    // Main joystick functionality, which controls the sorting servo motor with 
    // the intention of manually sorting the disk
    angle_sort_manual = calculate_angle_manual();
    sort_servo.write(angle_sort_manual);
    
    // Wait for a small amount of time
    delay(full_delay);
  }
}

/** 
 * This function calculates and returns the appropriate angle for the pushing 
 * servo motor depending on the values of the manually controlled joystick. 
 * It is assumed that the global variables corresponding to the joystick values are
 * up to date.
 * 
 * @return the angle to move the sorting servo motor
 */
int calculate_angle_manual() {
  // Return the corresponding value for the servo arm position with regards to the max values
  if (joystick_x_value >= 512) {
    return angle_sort_neutral;
  } else if (joystick_x_value <= 70) {
    return angle_sort_white;
  }

  return 180 + (( (double) angle_sort_neutral - 180.) / 512.) * joystick_x_value;
}

/**
 * This function reads the values of the joystick 
 */
void read_joystick_values() {
  joystick_x_value = analogRead(joystick_x_pin);
  joystick_button_value = digitalRead(joystick_button_pin);
}

/**
 * This function waits for the release of the button specified.
 * 
 * @param button_pin the io pin of the button
 * @param initial_value the state (1 or 0, depending on the wiring) of the pressed button
 * 
 */
void wait_for_release(int button_pin, int initial_value) {
  int button_state = initial_value;
  // wait for release
  while(button_state == initial_value) {
    button_state = digitalRead(button_pin);
  }
}

/**
 *  Calculates the time it took to release a pressed button.
 * 
 * @param button_pin the io pin of the button
 * @param initial_value the state (1 or 0, depending on the wiring) of the pressed button
 * @return the time it took to release the button
 */
unsigned long time_for_release(int button_pin, int initial_value) {
  unsigned long before = millis();
  wait_for_release(button_pin, initial_value);
  return millis() - before;
}

/* 
 * This function prints all main variables involved in the sorting process for debugging
 */
void debugging() {
  Serial.print("sorting_disk: ");
  Serial.println(sorting_disk);
  
  Serial.print(" c: ");
  Serial.print(c);

  Serial.print(" c current: ");
  Serial.print(c_maximum);

  Serial.print(" Time Passed: ");
  Serial.println(time_passed);

  Serial.print("X = ");
  Serial.print(joystick_x_value);
  Serial.print(", Button = ");
  Serial.println(joystick_button_value);
}

/**
 * This function is called when some error is DETECTED and the variable @code{error} specifies
 * the nature of the error, namely, IDENTIFIES it. It prints the error message, lights up a red LED
 * and restarts the arduino.
 * 
 */
void error_detected(int error_code) {
  // Restart the arduino
  restart_arduino_on_command(true, error_code);
}

/** 
 * This function turns off all LEDs except for the red one which stays on for a few seconds
 * until the arduino is restarted.
 */
void restart_arduino() {
  // Light up the red LED and turn of the green and yellow LEDs
  digitalWrite(manual_LED_pin, LOW);
  digitalWrite(automatic_LED_pin, LOW);
  digitalWrite(error_LED_pin, HIGH); 
  rpi_com.running = false;
  rpi_com.sorting = 0;
  rpi_com.sendPacket();
  delay(delay_before_reset); // Start the delay before restarting the arduino
  delay(100);
  reset(); // Restart the arduino
}

/** 
 * Restart the arduino when any of the buttons are pressed.
 */
void restart_arduino_on_command(bool error, int error_code) {
  // Light up the red LED and turn off the green and yellow LEDs
  digitalWrite(manual_LED_pin, LOW);
  digitalWrite(automatic_LED_pin, LOW);
  digitalWrite(error_LED_pin, HIGH); 

  // Tell the rpi to signal to the other arduino to disable the conveyor
  rpi_com.running = false;
  rpi_com.sorting = 0;
  rpi_com.sendPacket();

  if (error) {
    delay(100);
    rpi_com.sendError(error_code);
  }
  while (!button_pressed(mode_button_pin) && !button_pressed(restart_button_pin)) {
    // Do nothing
  }
  rpi_com.resetValues();
  rpi_com.sendPacket();

  delay(100);
  reset(); // Restart the arduino
}

/*
 * Slowly pushes a disk from the cylinder onto the ramp.
 * Moves the disk by 48 degrees and back to its original position over 120 ms.
 */
void push_disk() {
  int max_push_angle_unit = 6;
  // Slowly push the disk onto the ramp
  push_servo.write(angle_push_neutral);
  
  for(int i = 1; i <= 8; i++) {
    push_servo.write(angle_push_neutral + max_push_angle_unit * i);
    delay(15); 
  }

  push_servo.write(angle_push_neutral);
  
}

/*
 * This function reads and processes the RGB values using the RGB sensor as the source
 * All the values are passed by reference, hence, no return variables are needed
 */
void read_RGB_values() {
  // Get the data
  tcs.getRawData(&r, &g, &b, &c);
  
  // Calculate the color temperature and illuminance
  colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);
  lux = tcs.calculateLux(r, g, b);
}

/*
 * Returns true if the c value is high enough to consider as a disk passing.
 */ 
bool disk_passing() {
  return c > 333;
}

/*
 * Sets the sorting servo's angle based on the threshold for
 * c values, which are the sum of r, g, and b values coming 
 * straight from the RGB sensor.
 */
void sort() {
  if (c_maximum > 4500) {
    sort_servo.write(angle_sort_white); // Sort White
    rpi_com.white++;
  } else if (c_maximum < 1000) {
    sort_servo.write(angle_sort_black); // Sort Black
    rpi_com.black++;
  } else {
    sort_servo.write(angle_sort_neutral); // Sort Color
    rpi_com.misc++;
  }

}
