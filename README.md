This repository is for a Quadcopter project which will be controlled using a feedback system. It has two phases; in phase 1 we will be making a beam balance using propellors (dual-copter) as a project for feedback control course, in phase 2 we will complete the Quadcopter
The main working code is dual_copter_balance_arming
This Project is running on ESP32 DevkitC, the IMU data is being obtained from mpu9250.
There are two 1400kv bldc motors with 1045 propellors being controlled by Simonk 30A ESC

For the project to run following libraries are required:
  1)ESP32 libraries from Arduino IDE
  2)ESP32Servo from Arduino IDE
  3)mpu9250 library from external sources
