//including libraries
#include "MPU9250.h"
#include <ESP32Servo.h>

//creating objects
MPU9250 mpu; //sensor initiated

Servo ESC_right;  //  create object for right motor
Servo ESC_left; //  create object for left motor   

//initiate pin numbers
const int ESCpin1 = 4; //PWM pin for ESC1
const int ESCpin2 = 5;  //PWM pin for ESC2

//initiate variables
float roll;
float pitch;
float yaw;

const double kp = 0;
const double ki = 0;
const double kd = 0;

const int base_pwm = 100;
const int max_pwm = 180;
double pwm_left;
double pwm_right;

unsigned long currentTime, previousTime;
double elapsedTime;
double error;
double lastError;
double input, output, setPoint;
double cumError, rateError;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  ESC_right.attach(ESCpin1, 1000, 2000);
  ESC_left.attach(ESCpin2, 1000, 2000);

  previousTime = millis();

  Wire.begin();
    delay(2000);

    if (!mpu.setup(0x68)) {  // change to your own address
        while (1) {
            Serial.println("MPU connection failed. Please check your connection with `connection_check` example.");
            delay(5000);
        }
    }

    setPoint = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
    static uint32_t lastControlTime = 0;
    const uint32_t controlInterval = 10; // ms
    
    if (millis() - lastControlTime >= controlInterval) {
        lastControlTime = millis();
    }
    
    if (mpu.update()) {
          static uint32_t prev_ms = millis();
          if (millis() > prev_ms + 5) {
              roll = mpu.getRoll();
              pitch = mpu.getPitch();
              yaw = mpu.getYaw();
              prev_ms = millis();
          }
      }

    output = computePID(roll); //gain
    Serial.print("output: ");
    Serial.println(output);

    pwm_left = base_pwm + output;
    pwm_left = clamp(pwm_left, 0, max_pwm);
    Serial.print("pwm left: ");
    Serial.println(pwm_left);

    pwm_right = base_pwm - output;
    pwm_right = clamp(pwm_right, 0, max_pwm);
    Serial.print("pwm right: ");
    Serial.println(pwm_right);

    ESC_left.write(pwm_left);
    ESC_right.write(pwm_right);

}

double computePID(double inp){     
        currentTime = millis();                //get current time
        elapsedTime = (double)(currentTime - previousTime);        //compute time elapsed from previous computation
        
        if (elapsedTime <=0) elapsedTime = 1;

        error = setPoint - inp;                                // determine error
        cumError += error * elapsedTime;                // compute integral
        rateError = (error - lastError)/elapsedTime;   // compute derivative

        double out = kp*error + ki*cumError + kd*rateError;                //PID output               

        lastError = error;                                //remember current error
        previousTime = currentTime;                        //remember current time

        return out;                                        //have function return the PID output
}

double clamp(double value, double min, double max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}
