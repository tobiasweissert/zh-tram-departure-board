// Icons.h - Icon definitions
#ifndef ICONS_H
#define ICONS_H

#include <Arduino.h>

// Transport Icons (8x8 pixels)
// Front-view bus icon for departures leaving now (0 minutes)
const unsigned char PROGMEM busFrontIcon[] = {
  0b01111110,  //  ******
  0b11111111,  // ********
  0b10000001,  // *      *
  0b10000001,  // *      *
  0b11111111,  // ********
  0b10111101,  // * **** *
  0b11111111,  // ********
  0b01000010   //  *    *
};

// Running man icon for departures between 5-9 minutes
const unsigned char PROGMEM runningManIcon[] = {
  0b00001100,  //     **
  0b00001100,  //     **
  0b00111000,  //   ***
  0b01001110,  //  *  ***
  0b00010000,  //    *
  0b00011000,  //    **
  0b01100100,  //  **  *
  0b00000100   //      *
};

// Battery Icons (8 width × 5 height pixels)
// Battery 100% (full)
const unsigned char PROGMEM batteryFullIcon[] = {
  0b01111110,
  0b11111110,
  0b11111110,
  0b11111110,
  0b01111110
};

// Battery 75%
const unsigned char PROGMEM battery75Icon[] = {
  0b01111110,
  0b10111110,
  0b10111110,
  0b10111110,
  0b01111110
};

// Battery 50%
const unsigned char PROGMEM battery50Icon[] = {
  0b01111110,
  0b10011110,
  0b10011110,
  0b10011110,
  0b01111110
};

// Battery 25%
const unsigned char PROGMEM battery25Icon[] = {
  0b01111110,
  0b10001110,
  0b10001110,
  0b10001110,
  0b01111110
};

// Battery 0% (empty)
const unsigned char PROGMEM batteryEmptyIcon[] = {
  0b01111110,
  0b10000110,
  0b10000110,
  0b10000110,
  0b01111110
};

#endif
