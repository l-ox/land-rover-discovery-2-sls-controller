// Land Rover Discovery 2 SLS Controller
// https://github.com/l-ox/land-rover-discovery-2-sls-controller
// By Luke Oxley

const int msb_pin = 2; // Assign pin for Mode Selector Button (MSB).
const int os_solenoid = 3; // Assign pin for O/S/R solenoid.
const int ns_solenoid = 4; // Assign pin for N/S/R solenoid.
const int vent_solenoid = 5; // Assign pin for vent solenoid.
const int pump_relay = 6; // Assign pin for pump relay.
const int warn_sig = 7; // Assign pin for warning (lamp and tone).
const int orm_status_lamp = 8; // Assign pin for Off-Road Mode status lamp.
const int height_deviation = 50; // Variable to set the max allowed +/- deviation before system activation.
int target_mode = 3; // Variable to store target mode, and set default to 3, which corrects in the program as 0 at first wakeup.
int last_msb_state = LOW; // Variable to store the previous MSB state, and set default to LOW.
int msb_state; // Variable to store the current MSB state.
int current_height; // Variable to store the current vehicle height.
int current_mode; // Variable to store the current active mode.
int target_height; // Variable to store target vehicle height.
int activation_status = 1; // Variable to store the current activation status - on by default to level vehicle at first wakeup.
int initial_startup = 1; // Variable to store initial startup state - on by default at vehicle first wakeup.
int warn_sig_state = LOW; // Variable to store warning signal state, and set default to LOW.
int fault_state = 0; // Variable to store fault state, and set default to 0 (false).
long timeout = 0; // Variable to store timeout incrementer, and set default to 0.
unsigned long previousMillis = 0; // Variable to store previousMillis time, and set default to 0.
const long interval = 500; // Variable to store interval for warning signal, default to 500.

void setup() {
  pinMode(msb_pin, INPUT_PULLUP); // Set the MSB pin as an input with an internal pull-up resistor.
  pinMode (os_solenoid, OUTPUT); // Set the O/S/R pin as an output.
  pinMode (ns_solenoid, OUTPUT); // Set the N/S/R pin as an output.
  pinMode (vent_solenoid, OUTPUT); // Set the vent solenoid pin as an output.
  pinMode (pump_relay, OUTPUT); // Set the pump relay pin as an output.
  pinMode (orm_status_lamp, OUTPUT); // Set the status lamp pin as an output.
  pinMode (warn_sig, OUTPUT); // Set the warning signal pin as an output.
  Serial.begin(9600); // Initialise the Serial Monitor.
}

void loop() {
  msb_state = digitalRead(msb_pin); // Read the current state from the MSB pin and store it in the msb_state variable.
  current_height = analogRead(A0); // Read the current vehicle height from the sensor and store in the current_height variable.
  if (msb_state == HIGH && last_msb_state == LOW && fault_state != 1) { // Check if the MSB state has changed from LOW to HIGH (MSB depressed), change the target mode in sequence upon MSB activation, and update Serial Monitor. Ignore if in faulted state.
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
  // Detect if system is activated and the current height is less than the target height (minus the deviation). If true, call for the vehicle to be raised. Update Serial Monitor.
  if (current_height < (target_height - height_deviation) && activation_status == 1) {
    Serial.print("Attempting to raise vehicle to target mode = ");
    Serial.println(target_mode);
    Serial.print("Current height = ");
    Serial.println(current_height);
    Serial.print("Target height = ");
    Serial.println(target_height);
    raiseVehicle();
  }
  // Detect if system is activated and the current height is more than the target height (minus the deviation). If true, call for the vehicle to be lowered. Update Serial Monitor.
  if (current_height > (target_height + height_deviation) && activation_status == 1) {
    Serial.print("Attempting to lower vehicle to target mode = ");
    Serial.println(target_mode);
    Serial.print("Current height = ");
    Serial.println(current_height);
    Serial.print("Target height = ");
    Serial.println(target_height);
    lowerVehicle();
  }
}

// Attempts to raise the vehicle to the target height, looping meanwhile to activate warning signal, and listens for further MSB signal to breakout. If target height is reached, run deactivation. If MSB is depressed, break out of the loop. Update Serial Monitor. Run timeout counter, and run fault() if timeout is raeched.
void raiseVehicle() {
    while (current_height < (target_height - height_deviation) && activation_status == 1 && msb_state == HIGH && timeout < 200000) {
    digitalWrite(os_solenoid, HIGH);
    digitalWrite(ns_solenoid, HIGH);
    digitalWrite(pump_relay, HIGH);
    current_height = analogRead(A0);
    msb_state = digitalRead(msb_pin);
    warn();
    timeout = (timeout+1);
    }
  initial_startup = 0;
  if (timeout >= 200000) {
    fault();
  }
  else {
    deactivate();
  }
}

// Attempts to lower the vehicle to the target height, looping meanwhile to activate warning signal, and listens for further MSB signal to breakout. If target height is reached, run deactivation. If MSB is depressed, break out of the loop. Update Serial Monitor. Run timeout counter, and run fault() if timeout is raeched.
void lowerVehicle() {
    while (current_height > (target_height + height_deviation) && activation_status == 1 && msb_state == HIGH && timeout < 200000) {
    digitalWrite(os_solenoid, HIGH);
    digitalWrite(ns_solenoid, HIGH);
    digitalWrite(vent_solenoid, HIGH);
    current_height = analogRead(A0);
    msb_state = digitalRead(msb_pin);
    warn();
    timeout = (timeout+1);
    timeout = timeout;
    }
    initial_startup = 0;
    if (timeout >= 200000) {
      fault();
    }
    else {
      deactivate();
    }
}

// Activate warning signal (warning light flash and warning tone on vehicle binnacle) - flash/sound every 500ms.
void warn() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval && initial_startup != 1) {
    previousMillis = currentMillis;
    if (warn_sig_state == LOW) {
      warn_sig_state = HIGH;
    }
    else {
      warn_sig_state = LOW;
    }
    digitalWrite(warn_sig, warn_sig_state);
  }
}

// Deactivate all SLS elements due to successful operation. Update current_mode variable. Disable warning signal. Update Serial Monitor.
void deactivate() {
    digitalWrite(os_solenoid, LOW);
    digitalWrite(ns_solenoid, LOW);
    digitalWrite(vent_solenoid, LOW);
    digitalWrite(pump_relay, LOW);
    Serial.println("Vehicle raised to target height successfully.");
    Serial.print("Current height = ");
    Serial.println(current_height);
    current_mode = target_mode;
    activation_status = 0;
    digitalWrite(warn_sig, LOW);
    timeout = 0;
    // Enable ORM lamp in mode 3, else disable.
    if (current_mode == 3) {
      digitalWrite(orm_status_lamp, HIGH);
    }
    if (current_mode != 3) {
      digitalWrite(orm_status_lamp, LOW);
    }
}

// Attempt to raise/lower was unsuccessful. Deactivate all SLS elements, set fault status to 1 to deactive system permanently, reset activation and timeout variables, illuminate warning lamp permanently, update Serial Monitor.
void fault() {
  digitalWrite(os_solenoid, LOW);
  digitalWrite(ns_solenoid, LOW);
  digitalWrite(vent_solenoid, LOW);
  digitalWrite(pump_relay, LOW);
  Serial.println("Vehicle did not reach target height successfully - fault! System deactivated!");
  activation_status = 0;
  fault_state = 1;
  timeout = 0;
  digitalWrite(warn_sig, HIGH);
}
