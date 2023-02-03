/*
  Serial Event example
  When new serial data arrives, this sketch adds it to a String.
  When a newline is received, the loop prints the string and clears it.
  A good test for this is to try it with a GPS receiver that sends out
  NMEA 0183 sentences.
  NOTE: The serialEvent() feature is not available on the Leonardo, Micro, or
  other ATmega32U4 based boards.
  created 9 May 2011
  by Tom Igoe
  This example code is in the public domain.
  https://www.arduino.cc/en/Tutorial/BuiltInExamples/SerialEvent
*/


#include <Stepper.h>

const int stepsPerRevolution = 200;
Stepper myStepper(stepsPerRevolution, 2, 3, 4, 5);


int pos = 0;    // variable to store the servo position
bool flag = false;
String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

void setup() {
  // initialize serial:
  Serial.begin(57600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(1000);
  delay(500);
  //pinMode(LED_BUILTIN, OUTPUT);
  pinMode(11, OUTPUT);
}

void loop() {
  // print the string when a newline arrives:
  analogWrite(11, 10);
  if (stringComplete) {
    Serial.println(inputString);
    // clear the string:
    inputString = "";

    stringComplete = false;

    for (pos = 0; pos <= 16; pos += 1){
      myStepper.step(1);
      delay(5);
    }

    delay(250);
    flag = true;
  }
  Serial.println("READY");
  delay(150);
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
    if (inChar == '0') {
      delay(250);
    }

  }
}
