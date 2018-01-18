/*
  OSC2APA102

  listen on USB Serial for slip encoded OSC packets
  to control two APA102 LED strips
  with some extra DMX output

  Copyright Antoine Villeret / Pascal Baltazar - 2015/2018

*/
#include <Arduino.h>
#include <OSCBundle.h>

#include <OctoWS2811.h>

#include <PacketSerial.h>

#define LED_NUMBER 140     // <-- # of LEDs in strip 1

DMAMEM int displayMemory[6*LED_NUMBER];
unsigned char buffer[LED_NUMBER*3];
const int config = WS2811_GRB | WS2811_800kHz;

OctoWS2811 leds(LED_NUMBER, displayMemory, NULL, config);

// Use the serial device with PacketSerial
PacketSerial_<SLIP, SLIP::END, 8192> serial;

bool val = false;

// Parsing the OSC messages for /1 (int or Blob)
void LEDcontrol1(OSCMessage &msg)
{

  // When receiving a Blob, we assume it's a list of RGB values
  if (msg.isBlob(0))
  {
    msg.getBlob(0, (unsigned char *)buffer, LED_NUMBER*3);

    for (int i=0; i<LED_NUMBER; i++)
    {
      int color =  buffer[i*3+2];
      color |=  buffer[i*3+1] << 8;
      color |=  buffer[i*3] << 16;
      leds.setPixel(i, color);
    }
  }
}


/////////////////////////////////////////////////////////////////////
// This is where the OSC address parsing happens:
void onPacket(const uint8_t* buffer, size_t size) {
  OSCBundle bundleIN;

  for (size_t i = 0; i < size; i++) {
    bundleIN.fill(buffer[i]);
  }

  if (!bundleIN.hasError()) {
    bundleIN.dispatch("/1", LEDcontrol1);
  }
  if(!leds.busy())
    leds.show();
}


//////////////////////////////////////////////////////  ///////////////
void setup() {
  // We must specify a packet handler method
  serial.setPacketHandler(&onPacket);
  serial.begin(115200); // baudrate is ignored, is always run at 12Mbps

  leds.begin();
  for (int i=0;i<LED_NUMBER;i++)
  {
    leds.setPixel(i,0);
  }
  leds.show();
}


/////////////////////////////////////////////////////////////////////

void loop() {
  serial.update();
  delay(10);
}
