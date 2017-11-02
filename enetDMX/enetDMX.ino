//Ethernet to WS2811 bridge for max et

#include <DmxSimple.h>
#include "FastLED.h"
#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#define NUM_LEDS 100
#define LED_PIN     3

#define UDP_TX_PACKET_MAX_SIZE 938

CRGB leds[NUM_LEDS];


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
//byte mac[] = {
//  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte mac[] = {
  0xDE, 0xCB, 0xCE, 0xEF, 0xAE, 0xAC
};
IPAddress ip(192, 168, 1, 193);

unsigned int localPort = 6038;      // local port to listen on

const int num_channels = 450;
const int ck_header = 8;

EthernetUDP Udp;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet

void setup() {
  // start the Ethernet and UDP:
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);

  Serial.begin(115200);
  FastLED.addLeds<DMXSIMPLE, 3>(leds, NUM_LEDS );
  // FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  Serial.println("enetDmx setup");
}

void loop() {
  // if there's data available, read a packet
  int packetSize = (Udp.parsePacket());
  if (packetSize)
  {

    Udp.read( (char*)leds, packetSize-3);


    Serial.println(FastLED.getFPS());
    FastLED.delay(40);
    FastLED.show();
  }


  //  delay(10);
}




