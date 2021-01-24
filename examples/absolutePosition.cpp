#include <Arduino.h>
#include <SmoothStepper.h>

const int stepsPerRevolution = 2048;

SmoothStepper smoothStepper(stepsPerRevolution, 23, 22, 21, 19);

void setup() {
    Serial.begin(115200);

    disableCore0WDT();
    if (!smoothStepper.accelerationEnable(3, 15, 500)) {
        Serial.println("Non correct parameter(s)");
        while (1) {
        }
    }

    smoothStepper.begin();
}

void loop() {
    int a = random(-600, 600);
    int b = random(-600, 600);

    //Go to the absolute position a.
    smoothStepper.absolutePosition(a);
    delay(500);

    smoothStepper.absolutePosition(b);
    delay(500);

    smoothStepper.absolutePosition(-a);
    delay(500);

    smoothStepper.absolutePosition(-b);
    delay(500);

    delay(5000);
}
