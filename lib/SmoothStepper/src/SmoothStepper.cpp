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
#include "SmoothStepper.h"

#include "Arduino.h"

int SmoothStepper::numberOfTasks = 0;

/*
 * two-wire constructor.
 * Sets which wires should control the motor.
 */
SmoothStepper::SmoothStepper(int number_of_steps, int motor_pin_1,
                             int motor_pin_2) {
    this->step_number = 0;  // which step the motor is on
    this->number_of_steps =
        number_of_steps;  // total number of steps for this motor

    // Arduino pins for the motor control connection:
    this->motor_pin_1 = motor_pin_1;
    this->motor_pin_2 = motor_pin_2;

    // setup the pins on the microcontroller:
    pinMode(this->motor_pin_1, OUTPUT);
    pinMode(this->motor_pin_2, OUTPUT);

    // When there are only 2 pins, set the others to 0:
    this->motor_pin_3 = 0;
    this->motor_pin_4 = 0;
    this->motor_pin_5 = 0;

    // pin_count is used by the stepMotor() method:
    this->pin_count = 2;
}

/*
 *   constructor for four-pin version
 *   Sets which wires should control the motor.
 */
SmoothStepper::SmoothStepper(int number_of_steps, int motor_pin_1,
                             int motor_pin_2, int motor_pin_3,
                             int motor_pin_4) {
    this->step_number = 0;  // which step the motor is on
    this->number_of_steps =
        number_of_steps;  // total number of steps for this motor

    // Arduino pins for the motor control connection:
    this->motor_pin_1 = motor_pin_1;
    this->motor_pin_2 = motor_pin_2;
    this->motor_pin_3 = motor_pin_3;
    this->motor_pin_4 = motor_pin_4;

    // setup the pins on the microcontroller:
    pinMode(this->motor_pin_1, OUTPUT);
    pinMode(this->motor_pin_2, OUTPUT);
    pinMode(this->motor_pin_3, OUTPUT);
    pinMode(this->motor_pin_4, OUTPUT);

    // When there are 4 pins, set the others to 0:
    this->motor_pin_5 = 0;

    // pin_count is used by the stepMotor() method:
    this->pin_count = 4;
}

/*
 *   constructor for five phase motor with five wires
 *   Sets which wires should control the motor.
 */
SmoothStepper::SmoothStepper(int number_of_steps, int motor_pin_1,
                             int motor_pin_2, int motor_pin_3, int motor_pin_4,
                             int motor_pin_5) {
    this->step_number = 0;  // which step the motor is on
    this->number_of_steps =
        number_of_steps;  // total number of steps for this motor

    // Arduino pins for the motor control connection:
    this->motor_pin_1 = motor_pin_1;
    this->motor_pin_2 = motor_pin_2;
    this->motor_pin_3 = motor_pin_3;
    this->motor_pin_4 = motor_pin_4;
    this->motor_pin_5 = motor_pin_5;

    // setup the pins on the microcontroller:
    pinMode(this->motor_pin_1, OUTPUT);
    pinMode(this->motor_pin_2, OUTPUT);
    pinMode(this->motor_pin_3, OUTPUT);
    pinMode(this->motor_pin_4, OUTPUT);
    pinMode(this->motor_pin_5, OUTPUT);

    // pin_count is used by the stepMotor() method:
    this->pin_count = 5;
}

void SmoothStepper::begin() {
    this->numberOfTasks++;
    strcpy(this->task_name, "stepperTask");
    char buffer[10];
    sprintf(buffer, "%d", this->numberOfTasks);
    strcat(this->task_name, buffer);

    xTaskCreatePinnedToCore(
        SmoothStepper::staticSmoothStepperTask,  // Task function.
        this->task_name,                         // name of task.
        2000,                                    // Stack size of task
        this,                                    // parameter of the task
        1,                                       // priority of the task
        NULL,                                    // Task handle to keep track of created task
        0);                                      // pin task to core 0
}

void SmoothStepper::staticSmoothStepperTask(void *pvParameters) {
    SmoothStepper *smoothStepper =
        reinterpret_cast<SmoothStepper *>(pvParameters);
    smoothStepper->smoothStepperTask();
}

void SmoothStepper::smoothStepperTask() {
    this->calculStrategy();
    unsigned long last_step_time = 0;  // time stamp in us of when the last step was taken

    while (1) {
        if (this->steps_to_move != 0) {
            this->step_to_be += this->steps_to_move;
            this->steps_to_move = 0;
            this->calculStrategy();
        }

        unsigned long now = micros();

        if (this->step_to_be != this->current_step || this->direction != 0) {
            if (now - last_step_time >= this->newDelay * 1000) {
                this->doStep();
                last_step_time = now;

                if (this->stopping) {
                    if ((this->newSpeed == this->vmin) ||
                        (!this->smoothActivated && this->step_to_be == this->current_step)) {
                        this->direction = 0;
                        this->calculStrategy();
                    } else {
                        this->newDelay = this->calculateDelay();
                    }
                } else {
                    if (this->current_step * this->direction >= this->deccelerationAtStep * this->direction) {
                        this->stopping = true;
                        this->start_time = this->calculateStartTime();
                    }
                    this->newDelay = this->calculateDelay();
                }
            }
        }
    }
}

void SmoothStepper::doStep() {
    this->current_speed = this->newSpeed;
    if (this->direction == 1) {
        this->step_number++;
        this->current_step++;
        if (this->step_number == this->number_of_steps) {
            this->step_number = 0;
        }
    } else {
        if (this->step_number == 0) {
            this->step_number = this->number_of_steps;
        }
        this->step_number--;
        this->current_step--;
    }

    // step the motor to step number 0, 1, ..., {3 or 10}
    this->stepMotor(this->step_number % this->pin_count);
}

float SmoothStepper::calculateDelay() {
    if (!this->smoothActivated) {
        return 1 / this->vmin;
    }

    this->newDelay_last = this->newDelay;
    this->previousSpeed = this->newSpeed;
    double ti = micros() / 1000 - this->start_time / 1000;

    if (this->stopping) {
        this->newSpeed = -this->acc * ti + this->vmax;
    } else {
        this->newSpeed = this->acc * ti + this->vmin;
    }

    if (this->newSpeed < this->vmin) this->newSpeed = this->vmin;
    if (this->newSpeed > this->vmax) this->newSpeed = this->vmax;

    return (1 / this->newSpeed);
}

void SmoothStepper::calculStrategy() {
    int stepToMove = this->step_to_be - this->current_step;

    if (stepToMove == 0 && this->newSpeed == this->vmin) {
        return;
    } else if (stepToMove > 0 &&
               (this->direction == 0 || !this->smoothActivated)) {  // We are stopped and will move forward.
        this->direction = 1;
    } else if (stepToMove < 0 &&
               (this->direction == 0 || !this->smoothActivated)) {  // We are stopped and will move backward.
        this->direction = -1;
    } else if ((stepToMove > 0 && this->direction == 1) ||
               (stepToMove < 0 && this->direction == -1)) {  // We will move more in the same direction.
    } else {                                                 // We will stop
        this->stopping = true;
        this->start_time = this->calculateStartTime();
        this->newDelay = this->calculateDelay();
        return;
    }

    if (!this->smoothActivated) {
        if (this->direction == 1) this->deccelerationAtStep = this->current_step + stepToMove - 1;
        if (this->direction == -1) this->deccelerationAtStep = this->current_step - stepToMove + 1;
    } else {
        float timeToVmin = (this->vmin - this->current_speed) / -this->acc;                           // ms
        float timeToVmax = (this->vmax - this->current_speed) / this->acc;                            // ms
        int stepToVmin = -this->acc / 2 * pow(timeToVmin, 2) + this->current_speed * timeToVmin + 1;  // number of steps
        int stepToVmax = this->acc / 2 * pow(timeToVmax, 2) + this->current_speed * timeToVmax + 1;   // number of steps
        if (abs(stepToMove) <= abs(stepToVmin)) {                                                     // stopping right now
            this->deccelerationAtStep = this->current_step;
            this->stopping = true;
        } else {  // We accelerate
            this->stopping = false;
            int cases = abs(stepToMove) - abs(stepToVmax) - abs(this->stepVmaxToVmin);
            if (cases == 0) {  // Go to vmax and then stopping.
                if (this->direction == 1) this->deccelerationAtStep = this->current_step + stepToVmax - 1;
                if (this->direction == -1) this->deccelerationAtStep = this->current_step - stepToVmax + 1;
            } else if (cases > 0) {  // constant speed for a while.
                if (this->direction == 1) this->deccelerationAtStep = this->step_to_be - this->stepVmaxToVmin - 1;
                if (this->direction == -1) this->deccelerationAtStep = this->step_to_be + this->stepVmaxToVmin + 1;
            } else {  // We will deccelerate before reach vmax.
                this->deccelerationAtStep = abs((abs(stepToMove) - abs(stepToVmin)) / 2);
                if (this->direction == 1) this->deccelerationAtStep = this->current_step + this->deccelerationAtStep;
                if (this->direction == -1) this->deccelerationAtStep = this->current_step - this->deccelerationAtStep;
            }
        }
    }
    this->start_time = this->calculateStartTime();
    this->newDelay = this->calculateDelay();
}

/**
 * Calculate the t(ms) position for speed functions.
 * */
double SmoothStepper::calculateStartTime() {
    if (!this->smoothActivated) return 0;
    double t_current = 0;

    if (this->newSpeed == 0)
        this->newSpeed = this->vmin;

    if (this->stopping) {
        t_current = (this->newSpeed - this->vmax) / -this->acc + 1 / this->previousSpeed;
    } else {
        t_current = (this->newSpeed - this->vmin) / this->acc + 1 / this->previousSpeed;
    }

    if (t_current < 0) {
        t_current = 0;
    }

    return micros() - t_current * 1000;
}

/*
 * Return 1 when it's arrived and 0 when it's not.
 */
int SmoothStepper::isArrived() {
    if (this->step_to_be == this->current_step &&
        this->direction == 0 && this->steps_to_move == 0) {
        return 0;
    } else {
        return 1;
    }
}

/*
 * Wait until it reachs his final step
 */
void SmoothStepper::waitUntilArrived() {
    while (this->isArrived()) {
    }
}

/*
 * Return the step number
 */
int SmoothStepper::whatStepNumber() { return this->current_step % 2048; }

/*
 * Return the number of rotation
 */
int SmoothStepper::whatRotationNumber() {
    return int(this->current_step / this->number_of_steps);
}

/*
 * Return to the origin point.
 */
void SmoothStepper::goToOrigin(bool rotation_included) {
    if (rotation_included) {
        this->steps_to_move -= this->current_step;

    } else {
        this->steps_to_move -= this->current_step % 2048;
    }
}

void SmoothStepper::accelerationDisable(float speed) {
    this->smoothActivated = false;
    if (speed == 0) {
        this->vmin = 0.1;

    } else {
        this->vmin = speed * this->number_of_steps / 60 / 1000;  // step/ms
    }
    this->current_speed = this->vmin;
    this->acc = 0;  // step/ms²
}

/*
 * calcul coefficents of acceleration equation.
 * All parameters must be superior to 0.
 */
bool SmoothStepper::accelerationEnable(float minSpeed, float maxSpeed,
                                       long rampTime) {
    this->smoothActivated = true;
    if (maxSpeed <= 0 || rampTime <= 0 || minSpeed <= 0) {
        return false;
    }

    this->vmin = minSpeed * this->number_of_steps / 60 / 1000;  // step/ms
    this->vmax = maxSpeed * this->number_of_steps / 60 / 1000;  // step/ms

    if (this->current_speed > this->vmax) {
        this->current_speed = this->vmax;
    } else if (this->current_speed < this->vmin) {
        this->current_speed = this->vmin;
    }
    if (this->previousSpeed == 0) {
        this->previousSpeed = this->vmin;
    }

    this->acc = (this->vmax - this->vmin) / rampTime;                                      // step/ms²
    this->stepVmaxToVmin = -this->acc / 2 * pow(rampTime, 2) + this->vmax * rampTime + 1;  // steps

    return true;
}

/*
 * Moves the motor forward or backwards.
 */
void SmoothStepper::stepMotor(int thisStep) {
    if (this->pin_count == 2) {
        switch (thisStep) {
            case 0:  // 01
                digitalWrite(motor_pin_1, LOW);
                digitalWrite(motor_pin_2, HIGH);
                break;
            case 1:  // 11
                digitalWrite(motor_pin_1, HIGH);
                digitalWrite(motor_pin_2, HIGH);
                break;
            case 2:  // 10
                digitalWrite(motor_pin_1, HIGH);
                digitalWrite(motor_pin_2, LOW);
                break;
            case 3:  // 00
                digitalWrite(motor_pin_1, LOW);
                digitalWrite(motor_pin_2, LOW);
                break;
        }
    }
    if (this->pin_count == 4) {
        switch (thisStep) {
            case 0:  // 1010
                digitalWrite(motor_pin_1, HIGH);
                digitalWrite(motor_pin_2, LOW);
                digitalWrite(motor_pin_3, HIGH);
                digitalWrite(motor_pin_4, LOW);
                break;
            case 1:  // 0110
                digitalWrite(motor_pin_1, LOW);
                digitalWrite(motor_pin_2, HIGH);
                digitalWrite(motor_pin_3, HIGH);
                digitalWrite(motor_pin_4, LOW);
                break;
            case 2:  // 0101
                digitalWrite(motor_pin_1, LOW);
                digitalWrite(motor_pin_2, HIGH);
                digitalWrite(motor_pin_3, LOW);
                digitalWrite(motor_pin_4, HIGH);
                break;
            case 3:  // 1001
                digitalWrite(motor_pin_1, HIGH);
                digitalWrite(motor_pin_2, LOW);
                digitalWrite(motor_pin_3, LOW);
                digitalWrite(motor_pin_4, HIGH);
                break;
        }
    }

    if (this->pin_count == 5) {
        switch (thisStep) {
            case 0:  // 01101
                digitalWrite(motor_pin_1, LOW);
                digitalWrite(motor_pin_2, HIGH);
                digitalWrite(motor_pin_3, HIGH);
                digitalWrite(motor_pin_4, LOW);
                digitalWrite(motor_pin_5, HIGH);
                break;
            case 1:  // 01001
                digitalWrite(motor_pin_1, LOW);
                digitalWrite(motor_pin_2, HIGH);
                digitalWrite(motor_pin_3, LOW);
                digitalWrite(motor_pin_4, LOW);
                digitalWrite(motor_pin_5, HIGH);
                break;
            case 2:  // 01011
                digitalWrite(motor_pin_1, LOW);
                digitalWrite(motor_pin_2, HIGH);
                digitalWrite(motor_pin_3, LOW);
                digitalWrite(motor_pin_4, HIGH);
                digitalWrite(motor_pin_5, HIGH);
                break;
            case 3:  // 01010
                digitalWrite(motor_pin_1, LOW);
                digitalWrite(motor_pin_2, HIGH);
                digitalWrite(motor_pin_3, LOW);
                digitalWrite(motor_pin_4, HIGH);
                digitalWrite(motor_pin_5, LOW);
                break;
            case 4:  // 11010
                digitalWrite(motor_pin_1, HIGH);
                digitalWrite(motor_pin_2, HIGH);
                digitalWrite(motor_pin_3, LOW);
                digitalWrite(motor_pin_4, HIGH);
                digitalWrite(motor_pin_5, LOW);
                break;
            case 5:  // 10010
                digitalWrite(motor_pin_1, HIGH);
                digitalWrite(motor_pin_2, LOW);
                digitalWrite(motor_pin_3, LOW);
                digitalWrite(motor_pin_4, HIGH);
                digitalWrite(motor_pin_5, LOW);
                break;
            case 6:  // 10110
                digitalWrite(motor_pin_1, HIGH);
                digitalWrite(motor_pin_2, LOW);
                digitalWrite(motor_pin_3, HIGH);
                digitalWrite(motor_pin_4, HIGH);
                digitalWrite(motor_pin_5, LOW);
                break;
            case 7:  // 10100
                digitalWrite(motor_pin_1, HIGH);
                digitalWrite(motor_pin_2, LOW);
                digitalWrite(motor_pin_3, HIGH);
                digitalWrite(motor_pin_4, LOW);
                digitalWrite(motor_pin_5, LOW);
                break;
            case 8:  // 10101
                digitalWrite(motor_pin_1, HIGH);
                digitalWrite(motor_pin_2, LOW);
                digitalWrite(motor_pin_3, HIGH);
                digitalWrite(motor_pin_4, LOW);
                digitalWrite(motor_pin_5, HIGH);
                break;
            case 9:  // 00101
                digitalWrite(motor_pin_1, LOW);
                digitalWrite(motor_pin_2, LOW);
                digitalWrite(motor_pin_3, HIGH);
                digitalWrite(motor_pin_4, LOW);
                digitalWrite(motor_pin_5, HIGH);
                break;
        }
    }
}

/*
 * Moves the motor steps_to_move steps.  If the number is negative,
 * the motor moves in the reverse direction.
 */
void SmoothStepper::step(int number_of_steps) {
    this->steps_to_move += number_of_steps;
}

/*
 * Moves the motor to the absolute position.
 */
void SmoothStepper::absolutePosition(int number_of_steps) {
    this->step_to_be = number_of_steps - 1;
    this->steps_to_move += 1;
}

// Wait until arrived and set origin to current position
void SmoothStepper::setOrigin() {
    this->waitUntilArrived();
    this->current_step = 0;
}

// Stop to move
void SmoothStepper::stopMove() {
    this->step_to_be = this->current_step - 1;
    this->steps_to_move += 1;
}

/*
  version() returns the version of the library:
*/
int SmoothStepper::version(void) { return 6; }