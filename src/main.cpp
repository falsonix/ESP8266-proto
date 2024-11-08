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

// define number of LED matrices connected to the system
#define MAX_DEVICES 7 // should be 7 for one side of the faceplate, 14 for both sides connected

// define LED matrix data pins
#define DATA_PIN D3 // change this to match the data out pin on your board
#define CLK_PIN D5 // change this to match the clock pin on your board
#define CS_PIN D8 // change this to match the chip select pin on your board

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

// define bitmaps for the blinking animation (in the same order as bitmaps)
const uint8_t blinkmaps[MAX_DEVICES][8] = {
  {0b11110000, 0b10111000, 0b10001110, 0b11111111, 0b01111111, 0b00001111, 0b00000000, 0b00000000}, // Rmouth1
  {0b00001111, 0b01111111, 0b11111001, 0b11110000, 0b10000000, 0b00000000, 0b00000000, 0b00000000}, // Rmouth2
  {0b00000000, 0b11000000, 0b11110000, 0b01111100, 0b00001111, 0b00000011, 0b00000001, 0b00000000}, // Rmouth3
  {0b00000001, 0b00000011, 0b00001111, 0b00011110, 0b00111100, 0b11111000, 0b11100000, 0b00000000}, // Rmouth4
  {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}, // Reye1 (blank)
  {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}, // Reye2 (blank)
  {0b01111110, 0b00111111, 0b00000011, 0b00000011, 0b00000001, 0b00000000, 0b00000000, 0b00000000} // Rnose1
};

// define function to send bitmaps to the MAX7219 registers
void sendBMP(const uint8_t bitmaps[MAX_DEVICES][8]) {
  for (int device = 0; device < MAX_DEVICES; device++) {
    for (int row = 0; row < 8; row++) {
      mx.setRow(device, row, bitmaps[device][row]);
    }
  }
}

// blinking function for the matrices that make up the eyes
void blink() {
  int blinkTimer = rand() % 7500 + 4000; // sets the blinking timer to anywhere between 4 seconds and 7.5 seconds
  Serial.print("Random blink set to ");
  Serial.println(blinkTimer); // print some text to the console followed by the millisecond value of the blink timer (left in ms since i doubt this would be used much)
  delay(blinkTimer); // delay for the randomly generated time
  // clear only the matrices that need to be turned off for blinking
  mx.clear(rightEye1);
  mx.clear(rightEye2);
  // send blinking frame to the specific matrices (using the blinkmaps array instead of sendbmp since we only want to update the eyes)
  for (int row = 0; row < 8; row++) {
    mx.setRow(rightEye1, row, blinkmaps[rightEye1][row]);
    mx.setRow(rightEye2, row, blinkmaps[rightEye2][row]);
  }
  delay(100); // delay for 100 msec to allow the human eye to see the change
  // clear the blinking frame from the eye matrices
  mx.clear(rightEye1);
  mx.clear(rightEye2);
  // resend normal frame to the eye matrices
  for (int row = 0; row < 8; row++) {
    mx.setRow(rightEye1, row, bitmaps[rightEye1][row]);
    mx.setRow(rightEye2, row, bitmaps[rightEye2][row]);
  }
}

// setup code here, runs once upon MCU powerup
void setup() {
  Serial.begin(9600); // init serial connection with baudrate 9600 for debugging purposes
  srand(static_cast<unsigned>(time(0))); // seed the random blink timer generator (only runs once)
  mx.begin();
  Serial.println("Matrix bootup command sent");
  delay(100); // small delay to allow time for the matrix drivers to catch up
  mx.clear(); // clear any content from pre-reset operation
  mx.control(MD_MAX72XX::INTENSITY, 2); // set display brightness, there is probably a shorter function for this
  Serial.println("Matrix intensity set");
  sendBMP(bitmaps);
  Serial.println("Bitmaps sent to matrices");
  // mx.transform(MD_MAX72XX::TFUD); // flip bitmap images over, comment this out if your image is upside down
  mx.transform(MD_MAX72XX::TRC); // rotate images 90 degrees
  delay(50); // 50 msec delay to allow for tasks to be completed
  mx.transform(MD_MAX72XX::TRC); // rotate images another 90 degrees
  Serial.println("All images transformed");
}

// main code here, runs forever until interrupted
void loop() {
  delay(20); // loop every 20 milliseconds, to prevent wasted resources
  blink(); // run blinking function
}