/*
        Arduino Brushless Motor Control
     by Dejan, https://howtomechatronics.com
*/

#include <ESP32Servo.h>

Servo ESC1;     // create servo object to control the ESC
Servo ESC2;     //

int potValue;  // value from the analog pin
const int potPin = 15; //ESP32 Pin for reading Analog Value
const int ESCpin1 = 5; //PWM pin for ESC1
const int ESCpin2 = 4;

void setup() {
  Serial.begin(115200);
  // Attach the ESC on pin D5
  ESC1.attach(ESCpin1,1000,2000); // (pin, min pulse width, max pulse width in microseconds) 
  ESC2.attach(ESCpin2, 1000, 2000);
}

void loop() {
  potValue = analogRead(potPin);   // reads the value of the potentiometer (value between 0 and 1023)
  potValue = map(potValue, 0, 4095, 0, 180);   // scale it to use it with the servo library (value between 0 and 180)
  ESC1.write(potValue);    // Send the signal to the ESC
  ESC2.write(potValue);
  Serial.println(potValue);
}