/*
  OSC2APA102

  listen on USB Serial for slip encoded OSC packets
  to control two APA102 LED strips
  with some extra DMX output

  Copyright Antoine Villeret / Pascal Baltazar - 2015/2018

*/
#include <Arduino.h>
#include <OSCBundle.h>

#include <FastLED.h>

#include <PacketSerial.h>

#define NUM_LEDS 200     // <-- # of LEDs in strip 1

#define DATA_PIN 3     // <-- pin number for DATA (MOSI, green wire)
#define CLOCK_PIN 13    // <-- pin number for CLOCK (SCK, yellow wire) - NB: use 27 for teensy >= 3.5 / for teensy <3.5, use pin 13 (which causes the LED to stay lit)

// Use the serial device with PacketSerial
PacketSerial_<SLIP, SLIP::END, 512> serial;

CRGB leds1[NUM_LEDS];

bool val = false;

// Parsing the OSC messages for /1 (int or Blob)
void LEDcontrol1(OSCMessage &msg)
{
  // When receiving a Blob, we assume it's a list of RGB values
  if (msg.isBlob(0))
  {
    msg.getBlob(0, (unsigned char *)leds1, NUM_LEDS*3);
  }
}


/////////////////////////////////////////////////////////////////////
// This is where the OSC address parsing happens:
void onPacket(const uint8_t* buffer, size_t size) {
  OSCBundle bundleIN;

  Serial.println("packet");

  for (size_t i = 0; i < size; i++) {
    bundleIN.fill(buffer[i]);
  }

  if (!bundleIN.hasError()) {
    bundleIN.dispatch("/1", LEDcontrol1);
  }
  FastLED.show();
}


//////////////////////////////////////////////////////  ///////////////
void setup() {
  // We must specify a packet handler method
  serial.setPacketHandler(&onPacket);
  serial.begin(115200); // baudrate is ignored, is always run at 12Mbps

  // Add the LED controller to FastLED
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds1, NUM_LEDS);

    // Turn off all LEDs
  FastLED.show(CRGB::Black);
}


/////////////////////////////////////////////////////////////////////

void loop() {
  serial.update();
  delay(10);
}
