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
#define DATA_PIN 4 // change this to match the data out pin on your board
#define CLK_PIN 0 // change this to match the clock pin on your board
#define CS_PIN 2 // change this to match the chip select pin on your board

// define the MAX7219 object, using mx as the quick reference nickname
MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::FC16_HW, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// predefine user specified bitmap image to display (all bitmaps should be 8x8)
const uint8_t test[8] = { // test bitmap, a lil smiley face :)
  0b00111100,
  0b01000010,
  0b01000010,
  0b00011100,
  0b00111110,
  0b01000000,
  0b10000001,
  0b01111110
};

// define function to send bitmaps to the MAX7219 register
void sendBMP(const uint8_t bitmap[]) {
  for (int row = 0; row < 8; row++) {
    mx.setRow(0, row, bitmap[row]); // for every row, send the bitmap data for that row
  }
}

void setup() {
  // setup code here, runs once upon MCU powerup
  Serial.begin(9600); // init serial connection with baudrate 9600
  mx.begin();
  mx.clear(); // clear any content from pre-reset operation
  mx.control(MD_MAX72XX::INTENSITY, 7); // set display brightness to 7, dunno why this isn't a shorter function but ok
  sendBMP(test); // display test bitmap
}

void loop() {
  // main code here, runs forever until interrupted
  delay(20); // loop every 20 milliseconds, to prevent wasted resources and because i don't have anything to put here rn
}