#include <Arduino.h>
#include <SmoothStepper.h>

const int stepsPerRevolution = 2048;

SmoothStepper smoothStepper(stepsPerRevolution, 23, 22, 21, 19);
SmoothStepper smoothStepper2(stepsPerRevolution, 18, 5, 17, 16);

void setup() {
    Serial.begin(115200);

    disableCore0WDT();
    //stepper 1 will be smooth
    if (!smoothStepper.accelerationEnable(1, 15, 1000)) {
        Serial.println("non correct parameter(s)");
        while (1) {
        }
    }

    //stepper 1 will not be smooth
    smoothStepper2.accelerationDisable(15);

    smoothStepper.begin();
    smoothStepper2.begin();
}

unsigned long timerExample = millis();

void loop() {
    delay(500);
    int a = random(-400, 400);

    smoothStepper.step(a);
    smoothStepper2.step(a);

    smoothStepper.waitUntilArrived();

    if (smoothStepper.whatStepNumber() > 1000) {
        //stop to move.
        smoothStepper2.stopMove();
        //set origin to current position.
        smoothStepper2.setOrigin();
        smoothStepper2.absolutePosition(-500);
    }

    if (millis() - timerExample > 20000) {
        smoothStepper2.waitUntilArrived();
        Serial.println("20s");
        Serial.print("Rotation number stepper 1: ");
        Serial.print(smoothStepper.whatRotationNumber());
        Serial.print("  || Rotation number stepper 2: ");
        Serial.println(smoothStepper2.whatRotationNumber());
        Serial.print("Step number stepper 1: ");
        Serial.print(smoothStepper.whatStepNumber());
        Serial.print("  || Step number stepper 2: ");
        Serial.println(smoothStepper2.whatStepNumber());
        delay(1000);

        //stepper 1 go to origin (rotation included)
        smoothStepper.goToOrigin(true);
        //stepper 2 go to origin (rotation not included)
        smoothStepper2.goToOrigin(false);

        smoothStepper.waitUntilArrived();
        smoothStepper2.waitUntilArrived();

        Serial.print("Rotation number stepper 1: ");
        Serial.print(smoothStepper.whatRotationNumber());
        Serial.print("  || Rotation number stepper 2: ");
        Serial.println(smoothStepper2.whatRotationNumber());
        Serial.print("Step number stepper 1: ");
        Serial.print(smoothStepper.whatStepNumber());
        Serial.print("  || Step number stepper 2: ");
        Serial.println(smoothStepper2.whatStepNumber());

        //wait 5s
        delay(5000);
        timerExample = millis();
    }
}
