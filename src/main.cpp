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

// blinking function for the matrices
void blink() {
  int blinkTimer = rand() % 7500 + 4000; // sets the blinking timer to anywhere between 4 seconds and 7.5 seconds
  Serial.print("Random blink set to ");
  Serial.println(blinkTimer); // print some text to the console followed by the millisecond value of the blink timer (left in ms since i doubt this would be used much)
  delay(blinkTimer); // delay for the randomly generated time
  // since we defined all the bitmaps in order, we have to change each one if we want to change the eyes to a blank state (blinking)
  // luckily we already have a function to send bitmaps in order to each device
  mx.clear(); // clear the matrices
  sendBMP(blinkmaps); // send blinking frame to the registers
  // bitmaps sent reverse for me so i rotate them twice
  mx.transform(MD_MAX72XX::TRC); // rotate images 90 degrees
  mx.transform(MD_MAX72XX::TRC); // rotate images another 90 degrees
  delay(100); // delay for 100 msec to allow the human eye to see the change
  mx.clear(); // clear blinking frame off of the registers
  sendBMP(bitmaps); // resend normal bitmaps to the matrices
  // gotta rotate again
  mx.transform(MD_MAX72XX::TRC); // rotate images 90 degrees
  mx.transform(MD_MAX72XX::TRC); // rotate images another 90 degrees
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