// Land Rover Discovery 2 SLS Controller V1.1
// By Luke Oxley
// Created 16/07/2023
// Updated 18/07/2023
const int msb_pin = 2; // Assign pin for Mode Selector Button (MSB).
const int os_solenoid = 3; // Assign pin for O/S/R solenoid.
const int ns_solenoid = 4; // Assign pin for N/S/R solenoid.
const int vent_solenoid = 5; // Assign pin for vent solenoid.
const int pump_relay = 6; // Assign pin for pump relay.
const int warn_lamp = 7; // Assign pin for warning lamp.
const int status_lamp = 8; // Assign pin for status lamp.
const int warn_tone = 9; // Assign pin for warning tone.
const int height_deviation = 50; // Variable to set the max allowed +/- deviation before system activation.
int target_mode = 3; // Variable to store target mode.
int last_msb_state = LOW; // Variable to store the previous MSB state.
int msb_state; // Variable to store the current MSB state.
int current_height; // Variable to store the current vehicle height.
int current_mode; // Variable to store the current active mode.
int target_height; // Variable to store target vehicle height.
int direction; // Variable to store the current attempted lift direction.
int activation_status = 1; // Variable to store the current activation status.

void setup() {
  pinMode(msb_pin, INPUT_PULLUP); // Set the MSB pin as an input with an internal pull-up resistor.
  pinMode (os_solenoid, OUTPUT); // Set the O/S/R pin as an output.
  pinMode (ns_solenoid, OUTPUT); // Set the N/S/R pin as an output.
  pinMode (vent_solenoid, OUTPUT); // Set the vent solenoid pin as an output.
  pinMode (pump_relay, OUTPUT); // Set the pump relay pin as an output.
  pinMode (warn_lamp, OUTPUT); // Set the warning lamp pin as an output.
  pinMode (status_lamp, OUTPUT); // Set the status lamp pin as an output.
  pinMode (warn_tone, OUTPUT); // Set the warning tone pin as an output.
  Serial.begin(9600); // Initialise the Serial Monitor.
}

void loop() {
  msb_state = digitalRead(msb_pin); // Read the current state from the MSB*
  current_height = analogRead(A0);
  // Check if the MSB state has changed from LOW to HIGH (MSB depressed)*
  if (msb_state == HIGH && last_msb_state == LOW) {
    // Change the mode in sequence upon MSB activation and update Serial Monitor*
    target_mode = (target_mode + 1) % 4;
    activation_status = 1;
  }
  last_msb_state = msb_state; // Update MSB state for the next iteration.
  delay(50); // Debounce delay to avoid multiple readings due to button bouncing.
  // Set target heights based on mode.
  if (target_mode == 0) {
    target_height = 600;
  }
  if (target_mode == 1) {
    target_height = 400;
  }
  if (target_mode == 2) {
    target_height = 200;
  }
  if (target_mode == 3) {
    target_height = 800;
  }
  // Attempt to raise vehicle to target height.
  if (current_height < (target_height - height_deviation) && direction == 0 && activation_status == 1) {
    digitalWrite(os_solenoid, HIGH);
    digitalWrite(ns_solenoid, HIGH);
    digitalWrite(pump_relay, HIGH);
    direction = 2;
    Serial.print("Attempting to raise vehicle to target height = ");
    Serial.println(target_mode);
    Serial.print("Current height = ");
    Serial.println(current_height);
  }
  // Deactivate system, vehicle reached target height successfully (UP).
   if (current_height > target_height && direction == 2 && activation_status ==1) {
    digitalWrite(os_solenoid, LOW);
    digitalWrite(ns_solenoid, LOW);
    digitalWrite(pump_relay, LOW);
    direction = 0;
    current_mode = target_mode;
    activation_status = 0;
    Serial.println("Raised to target height successfully.");
    Serial.print("Current height = ");
    Serial.println(current_height);
   }
  // Attempt to lower vehicle to target height.
   if (current_height > (target_height + height_deviation) && direction == 0 && activation_status == 1) {
    digitalWrite(os_solenoid, HIGH);
    digitalWrite(ns_solenoid, HIGH);
    digitalWrite(vent_solenoid, HIGH);
    direction = 1;
    Serial.print("Attempting to lower vehicle to target height = ");
    Serial.println(target_mode);
    Serial.print("Current height = ");
    Serial.println(current_height);
   }
  // Deactivate system, vehicle reached target height successfully (DOWN).
   if (current_height < target_height && direction == 1 && activation_status == 1) {
    digitalWrite(os_solenoid, LOW);
    digitalWrite(ns_solenoid, LOW);
    digitalWrite(vent_solenoid, LOW);
    direction = 0;
    current_mode = target_mode;
    activation_status = 0;
    Serial.println("Lowered to target height successfully.");
    Serial.print("Current height = ");
    Serial.println(current_height);
   }
}
