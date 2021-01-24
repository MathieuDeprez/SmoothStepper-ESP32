#include <Arduino.h>
#include <SmoothStepper.h>

const int stepsPerRevolution = 2048;
SmoothStepper smoothStepper(stepsPerRevolution, 23, 22, 21, 19);
//SmoothStepper smoothStepper2(stepsPerRevolution, 18, 5, 17, 16);

void setup() {
    Serial.begin(115200);

    disableCore0WDT();
    if (!smoothStepper.accelerationEnable(3, 15, 500)) {
        Serial.println("non correct parameter(s)");
        while (1) {
        }
    }
    //smoothStepper.accelerationDisable(15);

    smoothStepper.begin();
}

unsigned long grostimer = millis();

void loop() {
    //Serial.print("loop: ");
    //Serial.println(smoothStepper.number_task);
    /*
    smoothStepper.step(250);
    delay(1500);
    smoothStepper.step(-250);
    delay(10000);
    return;*/

    /*smoothStepper.step(-500);
    smoothStepper.waitUntilArrived();
    smoothStepper.step(700);
    smoothStepper.waitUntilArrived();
    return;*/

    delay(500);

    smoothStepper.absolutePosition(500);
    while (smoothStepper.whatStepNumber() < 400) {
    }
    Serial.println(smoothStepper.whatStepNumber());
    smoothStepper.stopMove();
    Serial.println(smoothStepper.whatStepNumber());
    smoothStepper.setOrigin();
    Serial.println(smoothStepper.whatStepNumber());

    smoothStepper.absolutePosition(-500);
    while (smoothStepper.whatStepNumber() > -400) {
    }
    Serial.println(smoothStepper.whatStepNumber());
    smoothStepper.stopMove();
    Serial.println(smoothStepper.whatStepNumber());
    smoothStepper.setOrigin();
    Serial.println(smoothStepper.whatStepNumber());

    delay(20000);
    return;

    /*
    smoothStepper.absolutePosition(500);
    smoothStepper.waitUntilArrived();
    smoothStepper.absolutePosition(-500);
    smoothStepper.waitUntilArrived();
    return;
    */

    delay(500);
    int a = random(-600, 600);
    int b = random(-600, 600);
    smoothStepper.step(a);
    smoothStepper.waitUntilArrived();
    smoothStepper.step(b);
    smoothStepper.waitUntilArrived();

    smoothStepper.step(-a);
    smoothStepper.waitUntilArrived();
    smoothStepper.step(-b);
    smoothStepper.waitUntilArrived();

    /*****************************
    smoothStepper.step(500);
    delay(700);
    smoothStepper.step(-500);
    delay(700);

    smoothStepper.step(308);
    delay(300);
    smoothStepper.step(-308);
    delay(400);

    smoothStepper.step(-500);
    delay(100);
    smoothStepper.step(500);
    delay(200);

    smoothStepper.waitUntilArrived();
    delay(1000);
    return;
***************************************/

    /*int b = random(-200, 200);
  Serial.print("b: ");
  Serial.println(b);
  smoothStepper.step(b);
  delay(abs(b));
*/

    if (millis() - grostimer > 10000) {
        //Serial.println("10s");
        smoothStepper.waitUntilArrived();
        //Serial.print("Origin; rotation number: ");
        //Serial.println(smoothStepper.whatRotationNumber());
        //Serial.print("step number: ");
        //Serial.println(smoothStepper.whatStepNumber());
        delay(2000);

        smoothStepper.goToOrigin(true);
        smoothStepper.waitUntilArrived();
        //Serial.print("Origin2; rotation number: ");
        //Serial.println(smoothStepper.whatRotationNumber());
        //Serial.print("step number: ");
        //Serial.println(smoothStepper.whatStepNumber());
        //Serial.print("uturn: ");
        //Serial.println(smoothStepper.counterA);
        delay(2000);
        grostimer = millis();

    } /*
    smoothStepper.goToOrigin(true);
    smoothStepper.waitUntilArrived();
    Serial.print("Origin; rotation number: ");
    Serial.println(smoothStepper.whatRotationNumber());
    Serial.print("step number: ");
    Serial.println(smoothStepper.whatStepNumber());

    delay(5000);
    smoothStepper.absolutePosition(0, 1253);
    smoothStepper.waitUntilArrived();
    Serial.print("absolute 1; rotation number: ");
    Serial.println(smoothStepper.whatRotationNumber());
    Serial.print("step number: ");
    Serial.println(smoothStepper.whatStepNumber());
    delay(5000);
    smoothStepper.absolutePosition(-1, 1253);
    smoothStepper.waitUntilArrived();
    Serial.print("absolute 2; rotation number: ");
    Serial.println(smoothStepper.whatRotationNumber());
    Serial.print("step number: ");
    Serial.println(smoothStepper.whatStepNumber());
    delay(5000);
    smoothStepper.absolutePosition(1, 42);
    smoothStepper.waitUntilArrived();
    Serial.print("absolute 3; rotation number: ");
    Serial.println(smoothStepper.whatRotationNumber());
    Serial.print("step number: ");
    Serial.println(smoothStepper.whatStepNumber());
    delay(5000);
    smoothStepper.goToOrigin(true);
    smoothStepper.waitUntilArrived();
    Serial.print("Origin; rotation number: ");
    Serial.println(smoothStepper.whatRotationNumber());
    Serial.print("step number: ");
    Serial.println(smoothStepper.whatStepNumber());
    grostimer = millis();
  }*/
}
