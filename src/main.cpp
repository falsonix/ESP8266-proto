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

// define bitmaps to send to displays (in order)
const uint8_t bitmaps[MAX_DEVICES][8] = {
  {0b00000000, 0b01000010, 0b01000010, 0b00000000, 0b10000001, 0b10000001, 0b01111110, 0b00000000}, // test1
  {0b00000000, 0b01100110, 0b01100110, 0b00000000, 0b10011001, 0b10011001, 0b10011001, 0b01100110}, // test2
  {0b00000000, 0b01000010, 0b01000010, 0b00000000, 0b10000001, 0b10000001, 0b01111110, 0b00000000}, // test1
  {0b00000000, 0b01100110, 0b01100110, 0b00000000, 0b10011001, 0b10011001, 0b10011001, 0b01100110}, // test2
  {0b00000000, 0b01000010, 0b01000010, 0b00000000, 0b10000001, 0b10000001, 0b01111110, 0b00000000}, // test1
  {0b00000000, 0b01100110, 0b01100110, 0b00000000, 0b10011001, 0b10011001, 0b10011001, 0b01100110}, // test2
  {0b00000000, 0b01000010, 0b01000010, 0b00000000, 0b10000001, 0b10000001, 0b01111110, 0b00000000} // test1
};

// define function to send bitmaps to the MAX7219 register
void sendBMP(const uint8_t bitmaps[MAX_DEVICES][8]) {
  for (int device = 0; device < MAX_DEVICES; device++) {
    for (int row = 0; row < 8; row++) {
      mx.setRow(device, row, bitmaps[device][row]);
    }
  }
}

// setup code here, runs once upon MCU powerup
void setup() {
  Serial.begin(9600); // init serial connection with baudrate 9600
  mx.begin();
  delay(100); // small delay to allow time for the matrix drivers to catch up
  mx.clear(); // clear any content from pre-reset operation
  mx.control(MD_MAX72XX::INTENSITY, 7); // set display brightness to 7, there is probably a shorter function for this
  sendBMP(bitmaps);
  mx.transform(MD_MAX72XX::TFUD); // flip bitmap images over, comment this out if your image is upside down
}

// main code here, runs forever until interrupted
void loop() {
  delay(20); // loop every 20 milliseconds, to prevent wasted resources
}