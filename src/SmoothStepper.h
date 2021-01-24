/*
 * SmoothStepper.cpp - SmoothStepper library for Esp32 - Version 1.2.0
 *
 * Original library                    (0.1)   by Tom Igoe.
 * Two-wire modifications              (0.2)   by Sebastian Gassner
 * Combination version                 (0.3)   by Tom Igoe and David Mellis
 * Bug fix for four-wire               (0.4)   by Tom Igoe, bug fix from Noah Shibley
 * High-speed stepping mod                     by Eugene Kozlenko
 * Timer rollover fix                          by Eugene Kozlenko
 * Five phase five wire                (1.1.0) by Ryan Orendorff
 * Smooth function added (ESP32 only)  (1.2.0) by Mathieu Deprez
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 * Drives a unipolar, bipolar, or five phase stepper motor.
 *
 * When wiring multiple stepper motors to a microcontroller, you quickly run
 * out of output pins, with each motor requiring 4 connections.
 *
 * By making use of the fact that at any time two of the four motor coils are
 * the inverse of the other two, the number of control connections can be
 * reduced from 4 to 2 for the unipolar and bipolar motors.
 *
 * A slightly modified circuit around a Darlington transistor array or an
 * L293 H-bridge connects to only 2 microcontroler pins, inverts the signals
 * received, and delivers the 4 (2 plus 2 inverted ones) output signals
 * required for driving a stepper motor. Similarly the Arduino motor shields
 * 2 direction pins may be used.
 *
 * The sequence of control signals for 5 phase, 5 control wires is as follows:
 *
 * Step C0 C1 C2 C3 C4
 *    1  0  1  1  0  1
 *    2  0  1  0  0  1
 *    3  0  1  0  1  1
 *    4  0  1  0  1  0
 *    5  1  1  0  1  0
 *    6  1  0  0  1  0
 *    7  1  0  1  1  0
 *    8  1  0  1  0  0
 *    9  1  0  1  0  1
 *   10  0  0  1  0  1
 *
 * The sequence of control signals for 4 control wires is as follows:
 *
 * Step C0 C1 C2 C3
 *    1  1  0  1  0
 *    2  0  1  1  0
 *    3  0  1  0  1
 *    4  1  0  0  1
 *
 * The sequence of controls signals for 2 control wires is as follows
 * (columns C1 and C2 from above):
 *
 * Step C0 C1
 *    1  0  1
 *    2  1  1
 *    3  1  0
 *    4  0  0
 *
 */
// ensure this library description is only included once
#ifndef SmoothStepper_h
#define SmoothStepper_h

// library interface description
class SmoothStepper {
   public:
    // constructors:
    SmoothStepper(int number_of_steps, int motor_pin_1, int motor_pin_2);
    SmoothStepper(int number_of_steps, int motor_pin_1, int motor_pin_2,
                  int motor_pin_3, int motor_pin_4);
    SmoothStepper(int number_of_steps, int motor_pin_1, int motor_pin_2,
                  int motor_pin_3, int motor_pin_4, int motor_pin_5);

    /**
     * To call after
     * accelerationEnable()
     * or
     * accelerationDisable()
     * */
    void begin();

    /**
     * To Enable acceleration
     * - minSpeed (rev/min)
     * - maxSpeed (rev/min)
     * - rampTime (ms)
     * */
    bool accelerationEnable(float minSpeed, float maxSpeed, long rampTime);

    /**
     * To Disable acceleration
     * - speed (rev/min)
     * */
    void accelerationDisable(float speed);

    /**
     * Add or substrace steps to move
     * */
    void step(int number_of_steps);

    /**
     * Absolute step to be
     * */
    void absolutePosition(int number_of_steps);

    int version(void);

    // Return true when arrived
    int isArrived();

    // Wait until motor is arrived
    void waitUntilArrived();

    // Return absolute step number
    int whatStepNumber();

    // Return abosulte rotation number
    int whatRotationNumber();

    // Go to Origin
    void goToOrigin(bool rotation_included);

    // Wait untile arrived and set origin to current position
    void setOrigin();

    // Stop to move
    void stopMove();

   private:
    // Private Methods
    void stepMotor(int this_step);
    void calculStrategy();
    float calculateDelay();
    double calculateStartTime();
    void doStep();

    //volatile variriables
    volatile int direction = 0;             // Direction of rotation
    volatile long step_to_be = 0;           // Global step to be
    volatile long current_step = 0;         // Current step
    volatile int steps_to_move = 0;         // Step to add/substract to step_to_be
    volatile int number_of_steps;           // Total number of steps this motor can take
    volatile bool smoothActivated = false;  // Smooth activated
    volatile float vmin;                    // Minimum speed (step/ms)
    volatile float current_speed = 0;       // Current speed (step/ms)
    volatile float vmax;                    // Maximum speed (step/ms)
    volatile double previousSpeed = 0;      // Previous calculated speed
    volatile float acc;                     // Acceleration (step/ms²)
    volatile int stepVmaxToVmin;            // Number of steps to reach vmax from vmin

    //static variables
    static int numberOfTasks;

    //non static and non volatile variables
    int pin_count;               // How many pins are in use
    int step_number = 0;         // Which step the motor is on
    int deccelerationAtStep;     // At which step do we start to stop
    long start_time;             // Start time to calculate acceleration (ms)
    bool stopping = false;       // Are we stopping
    float newDelay = 9.77;       // Delay to wait before next step
    float newSpeed = 0;          // Speed calculated
    float newDelay_last = 9.77;  // Last delay calculated
    char task_name[20];

    // motor pin numbers:
    int motor_pin_1;
    int motor_pin_2;
    int motor_pin_3;
    int motor_pin_4;
    int motor_pin_5;  // Only 5 phase motor

    // task
    static void staticSmoothStepperTask(void *pvParameters);
    void smoothStepperTask();
};

#endif