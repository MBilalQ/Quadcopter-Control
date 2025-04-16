// left motor starting pwm 28
// right motor starting pwm 45

//including libraries
#include "MPU9250.h"
#include <ESP32Servo.h>

//creating objects
MPU9250 mpu; //sensor initiated
Servo ESC_right;  //  create object for right motor
Servo ESC_left; //  create object for left motor   

//initiate pin numbers
const int ESCpin1 = 4; //PWM pin for ESC1 (right)
const int ESCpin2 = 5; //PWM pin for ESC2 (left)

//initiate variables
float roll;
float pitch;
float yaw;
const double kp = 0.5;  // Changed from 0 to provide some proportional control
const double ki = 0.01; // Small integral gain
const double kd = 0.1;  // Small derivative gain
const int base_pwm_left = 40; // Increased from 100 to ensure motor starts
const int base_pwm_right = 60;
const int min_pwm_left = 28;   // Minimum PWM to ensure motors can start
const int min_pwm_right = 45;
const int max_pwm_left = 60;
const int max_pwm_right = 75;
double pwm_left;
double pwm_right;
unsigned long currentTime, previousTime;
double elapsedTime;
double error;
double lastError;
double input, output, setPoint;
double cumError, rateError;
bool armed = false;
bool initialCalibrationDone = false;
unsigned long armingStartTime = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  // Initialize ESCs with proper range
  ESC_right.attach(ESCpin1, 1000, 2000);
  ESC_left.attach(ESCpin2, 1000, 2000);
  
  // Initial ESC calibration sequence
  Serial.println("Starting ESC calibration sequence...");
  ESC_right.write(0);
  ESC_left.write(0);
  delay(3000);  // Allow time for ESCs to recognize min signal
  
  previousTime = millis();
  
  // Initialize I2C and MPU9250
  Wire.begin();
  delay(2000);
  
  if (!mpu.setup(0x68)) {  // change to your own address
    while (1) {
      Serial.println("MPU connection failed. Please check your connection with `connection_check` example.");
      delay(5000);
    }
  }
  
  Serial.println("MPU9250 initialized successfully!");
  setPoint = 0;
  
  // Mark the start time for arming sequence
  armingStartTime = millis();
}

void loop() {
  // First complete the arming sequence before starting motor control
  if (!armed) {
    // Perform arming sequence
    unsigned long currentArmingTime = millis() - armingStartTime;
    
    if (currentArmingTime < 3000) {
      // Step 1: Keep at zero for 3 seconds
      ESC_right.write(0);
      ESC_left.write(0);
      Serial.println("Arming step 1: Min signal");
    } 
    else if (currentArmingTime < 4000) {
      // Step 2: Brief pulse to mid-range and back to zero
      ESC_right.write(base_pwm_left);
      ESC_left.write(base_pwm_right);
      Serial.println("Arming step 2: Mid signal pulse");
    }
    else if (currentArmingTime < 6000) {
      // Step 3: Back to zero before starting control
      ESC_right.write(0);
      ESC_left.write(0);
      Serial.println("Arming step 3: Back to min");
    }
    else {
      // Arming complete
      Serial.println("Arming sequence complete!");
      armed = true;
      // Reset timers
      previousTime = millis();
      delay(1000); // Small delay before starting control
    }
    return; // Skip the rest of the loop until arming is complete
  }
  
  // Regular motor control logic once armed
  static uint32_t lastControlTime = 0;
  const uint32_t controlInterval = 20; // Increased from 10ms to 20ms for more stable control
  
  // Only update control at fixed intervals
  if (millis() - lastControlTime >= controlInterval) {
    lastControlTime = millis();
    
    // Update sensor data
    if (mpu.update()) {
      roll = mpu.getRoll();
      pitch = mpu.getPitch();
      yaw = mpu.getYaw();
      
      Serial.print("Roll: ");
      Serial.print(roll);
      Serial.print(" | Setpoint: ");
      Serial.println(setPoint);
    }
    
    // Compute PID output
    output = computePID(roll); //gain
    Serial.print("PID output: ");
    Serial.println(output);
    
    // Calculate motor PWM values with a higher base_pwm
    pwm_left = base_pwm_left + output;
    pwm_right = base_pwm_right - output;
    
    // Ensure values are within safe operating range
    pwm_left = clamp(pwm_left, min_pwm_left, max_pwm_left);
    pwm_right = clamp(pwm_right, min_pwm_right, max_pwm_right);
    
    Serial.print("PWM left: ");
    Serial.print(pwm_left);
    Serial.print(" | PWM right: ");
    Serial.println(pwm_right);
    
    // Apply PWM values to ESCs
    ESC_left.write(pwm_left);
    ESC_right.write(pwm_right);
  }
}

double computePID(double inp) {     
  currentTime = millis();
  elapsedTime = (double)(currentTime - previousTime);
  
  if (elapsedTime <= 0) elapsedTime = 1;
  
  error = setPoint - inp;  // determine error
  
  // Anti-windup for integral term (limit cumulative error)
  if (abs(cumError) < 100) {  // Prevent excessive integral buildup
    cumError += error * elapsedTime;
  }
  
  rateError = (error - lastError) / elapsedTime;   // compute derivative
  
  double out = kp * error + ki * cumError + kd * rateError;
  
  // Limit PID output to prevent sudden changes
  out = clamp(out, -30, 30);
  
  lastError = error;                   // remember current error
  previousTime = currentTime;          // remember current time
  
  return out;
}

double clamp(double value, double min, double max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}