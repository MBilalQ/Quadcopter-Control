/*
        Arduino Brushless Motor Control
     by Dejan, https://howtomechatronics.com
*/

#include <ESP32Servo.h>

Servo ESC;     // create servo object to control the ESC

int potValue;  // value from the analog pin
const int potPin = 4; //ESP32 Pin for reading Analog Value
const int ESCpin1 = 5; //PWM pin for ESC1

void setup() {
  Serial.begin(115200);
  // Attach the ESC on pin D5
  ESC.attach(ESCpin1,1000,2000); // (pin, min pulse width, max pulse width in microseconds) 
}

void loop() {
  potValue = analogRead(potPin);   // reads the value of the potentiometer (value between 0 and 1023)
  potValue = map(potValue, 0, 4095, 0, 180);   // scale it to use it with the servo library (value between 0 and 180)
  ESC.write(potValue);    // Send the signal to the ESC
  Serial.println(potValue);
}