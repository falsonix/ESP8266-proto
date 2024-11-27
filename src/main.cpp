/* 
SYSTEM LIBRARIES
these libraries are required for basic functions and should not be removed

:3
*/
#include <Arduino.h> // include neccesary arduino libraries for the thing to work
#include <SPI.h> // include SPI communication library

/*
OTHER LIBRARIES FOR CONNETED DEVICES AND FUNCTIONS
*/
#include <MD_MAX72xx.h> // include library for control of the MAX7219 (or similar) LED matrices
#include <FastLED.h> // include library for control of the WS2812B LED strips

////////////////////////////////////////////////////////
///////////// Config section: LED matrices /////////////
////////////////////////////////////////////////////////

// change this to adjust the brightness of the LED matrices (i think the values range from 0 to 10 but idk)
const int matrixBrightness = 2;

// define number of LED matrices connected to the system
#define MAX_DEVICES 7 // should be 7 for one side of the faceplate, 14 for both sides connected

// define LED matrix data pins
#define DATA_PIN D3 // change this to match the data out pin you're using
#define CLK_PIN D5 // change this to match the clock pin you're using
#define CS_PIN D8 // change this to match the chip select pin you're using

// define the MAX7219 object, using mx as the quick reference nickname
MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::FC16_HW, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// define the eye matrices as their index, which will be the only ones to update upon blinking animations
const int rightEye1 = 4; // right eye 1
const int rightEye2 = 5; // right eye 2

// define bitmaps to send to displays (in order)
const uint8_t bitmaps[MAX_DEVICES][8] = {
  {0b11110000, 0b10111000, 0b10001110, 0b11111111, 0b01111111, 0b00001111, 0b00000000, 0b00000000}, // Rmouth1
  {0b00001111, 0b01111111, 0b11111001, 0b11110000, 0b10000000, 0b00000000, 0b00000000, 0b00000000}, // Rmouth2
  {0b00000000, 0b11000000, 0b11110000, 0b01111100, 0b00001111, 0b00000011, 0b00000001, 0b00000000}, // Rmouth3
  {0b00000001, 0b00000011, 0b00001111, 0b00011110, 0b00111100, 0b11111000, 0b11100000, 0b00000000}, // Rmouth4
  {0b00000111, 0b00011111, 0b01111111, 0b11100000, 0b10000000, 0b00000000, 0b00000000, 0b00000000}, // Reye1
  {0b11110000, 0b11111100, 0b11111111, 0b01111111, 0b00111111, 0b00011110, 0b00000000, 0b00000000}, // Reye2
  {0b01111110, 0b00111111, 0b00000011, 0b00000011, 0b00000001, 0b00000000, 0b00000000, 0b00000000} // Rnose1
};

////////////////////////////////////////////////////////
/////// Config section: Addressable LED strips /////////
////////////////////////////////////////////////////////

// define the type of addressable LED strip you're using
#define LED_TYPE WS2812B // change this to match the type of LED strip you're using

// define the color order of the LEDs
#define COLOR_ORDER GRB // change this to match the color order of your LED strip

// define the data pin of the addressable LED strip
#define LED_PIN D0 // change this to match the data out pin you're using

// define the number of LEDs in the strip
#define NUM_LEDS 60 // change this to match the number of LEDs in your strip

// define the brightness of the LED strip
#define BRIGHTNESS 125 // change this to adjust the brightness of the LEDs, 0-255

// define the LED strip object, using leds as the quick reference nickname
CRGB leds[NUM_LEDS];

// quick setup function to streamline the LED strip
void setupLEDs() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
}

// global hue value for the gradient
uint8_t hue = 0;

// little function for changing the hue of the rainbow gradient (if selected)
void runGradient() {
  fill_rainbow(leds, NUM_LEDS, hue, 7); // fill the LED strip with a rainbow gradient
  FastLED.show(); // send the gradient to the LED strip
  hue++; // increment the hue value for the next cycle
}

////////////////////////////////////////////////////////
/////// Config section: Miscellaneous Items ////////////
////////////////////////////////////////////////////////

bool batteryLow = false; // boolean value to alert the user if the battery is low

unsigned long previousMillis = 0; // most recent battery voltage check
const long interval = 10000; // interval at which to check the battery voltage

// function to read battery voltage (hopefully)
float readBatteryVoltage() {
  int raw = analogRead(A0); // read the raw value from the analog pin
  float voltage = raw * (3.3 / 1023.0); // convert the raw value to a voltage

  // adjust for voltage divider
  float R1 = 100000; // internal 100k resistor
  float R2 = 220000; // internal 220k resistor
  float R3 = 180000; // external 180k resistor

  float batteryVoltage = voltage * ((R1 + R2 + R3) / R3); // calculate the battery voltage
  return batteryVoltage; // return the voltage value
}

// ---------------------------------------------------------------------------

// function to only check the battery voltage every 10 seconds
// i decided on 10 seconds because battery voltage doesnt need to be checked every 20ms
void checkBatteryVoltage() {
  unsigned long currentMillis = millis(); // get the current time
  if (currentMillis - previousMillis >= interval) { // check if the interval has passed
    previousMillis = currentMillis; // set the previous time to the current time
    float voltage = readBatteryVoltage(); // read the battery voltage
    Serial.print("Battery voltage: "); // print header text to serial
    Serial.print(voltage); // print the battery voltage
    Serial.println("V"); // print followup text to serial

    // if battery is too low then throw an alert
    if (voltage < 3.3) { // check if the battery voltage is below 3.3V
      batteryLow = true; // set the battery low variable to true
      Serial.print("Battery low!"); // print battery low message to serial
    }
  }
}

void ledControl() {
  // check if the battery is low, if so then have a fading in/out red light across the LEDs
  if (batteryLow) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Red; // set the LED to red
      FastLED.show(); // send the LED data to the strip
      delay(10); // delay for 10 milliseconds
    }
    for (int i = NUM_LEDS - 1; i >= 0; i--) {
      leds[i] = CRGB::Black; // set the LED to black
      FastLED.show(); // send the LED data to the strip
      delay(10); // delay for 10 milliseconds
    }
  } else {
    runGradient(); // run the rainbow gradient function
  }
}

// define function to send bitmaps to the MAX7219 registers
void sendBMP(const uint8_t bitmaps[MAX_DEVICES][8]) {
  for (int device = 0; device < MAX_DEVICES; device++) {
    for (int row = 0; row < 8; row++) {
      mx.setRow(device, row, bitmaps[device][row]);
    }
  }
}

// rotation function for setting the matrices 180 degrees
void rotateMatrix() {
  mx.transform(MD_MAX72XX::TRC); // rotate images 90 degrees
  mx.transform(MD_MAX72XX::TRC); // rotate images another 90 degrees
}

// blinking function for the matrices that make up the eyes
void blink() {
  int blinkTimer = rand() % 7500 + 4000; // sets the blinking timer to anywhere between 4 seconds and 7.5 seconds
  Serial.print("Random blink set to ");
  Serial.println(blinkTimer); // print some text to the console followed by the millisecond value of the blink timer (left in ms since i doubt this would be used much)
  delay(blinkTimer); // delay for the randomly generated time
  mx.control(rightEye1, MD_MAX72XX::SHUTDOWN, true); // turn off the right eye 1 matrix
  mx.control(rightEye2, MD_MAX72XX::SHUTDOWN, true); // turn off the right eye 2 matrix
  delay(100); // delay for 100 milliseconds to allow the human eye to see the change
  mx.control(rightEye1, MD_MAX72XX::SHUTDOWN, false); // turn on the right eye 1 matrix
  mx.control(rightEye2, MD_MAX72XX::SHUTDOWN, false); // turn on the right eye 2 matrix
}

// setup code here, runs once upon MCU powerup
void setup() {
  // i decided to add serial support for debugging but not too high of a baudrate that would slow down the MCU
  Serial.begin(9600); // init serial connection with baudrate 9600 for debugging purposes
  pinMode(A0, INPUT); // set the analog pin as an input
  setupLEDs(); // run the setup function to init the addressable LEDs
  srand(static_cast<unsigned>(time(0))); // seed the random blink timer generator (only runs once)
  mx.begin(); // begin connection with the matrix drivers
  Serial.println("Matrix bootup command sent"); // report serial status
  delay(100); // small delay to allow time for the matrix drivers to catch up
  mx.clear(); // clear any content from pre-reset operation
  mx.control(MD_MAX72XX::INTENSITY, matrixBrightness); // set display brightness
  Serial.println("Matrix intensity set"); // report serial status
  sendBMP(bitmaps); // send bitmaps to the matrices
  Serial.println("Bitmaps sent to matrices"); // report serial status
  // during testing for my setup the matrix images were upside down, comment out the below line if this causes bad results
  rotateMatrix(); // rotate the images to the correct orientation
  Serial.println("All images transformed"); // report serial status
}

// main code here, runs forever until interrupted
void loop() {
  delay(20); // loop every 20 milliseconds, to prevent wasted resources
  blink(); // run blinking function for the eye matrices
  ledControl(); // run all needed functions for LED display
  checkBatteryVoltage(); // check the battery voltage if 10 seconds have passed
}