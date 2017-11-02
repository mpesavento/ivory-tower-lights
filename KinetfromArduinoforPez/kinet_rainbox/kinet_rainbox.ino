//David Wang, NuVuStudio.org, 2012
//Sean Stevens, SustainableMagic.org 2014 updated for arduino 1.0 through 1.5.8+

#include <FastLED.h>
#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008

byte r, g, b = 0;
byte r2, g2, b2 = 0;

int currentLight = 0;
int maxLights = 30;

const int plusButton = 2;


byte remote_ip[] = {192, 168, 1, 28`};
unsigned int remote_port = 6038;

byte local_mac[] = {0x11, 0xA2, 0xDA, 0x00, 0x9A, 0x58};
byte local_ip[] = {192, 168, 1, 201};
unsigned int local_port = 6038;



/*
  Class Kinet

  This class provides KiNET light control over UDP.
  Tested on Arduino 0023 and 0022.

  Example usage:
  // Create your Kinet object.
  // Give it the IP address and Port number of the
  // light controller
  byte remote_ip[] = {192, 168, 1, 2};
  unsigned int remote_port = 6038;
  Kinet kinet(remote_ip,remote_port);

  // To turn a single light on or off, use the function "setRGB".
  // Yhe first parameter is the LIGHT's ADDRESS relative to the controller
  // The remaining parameters are RED, GREEN, and BLUE intensity
  //   ranging from 0 = off to 255 = full on.
  kinet.setRGB(0,255,255,255);
  // To apply the command (send it to the controller)
  kinet.send();

  // To control all the lights connected to the controller
  kinet.setAllRGB(255,255,255);
  kinet.send();

  // OPTIONAL (For WALL mode only):
  // There is also the ability to control the lights in WALL mode:
  // Enable wall mode by using:
  kinet.setWall(true);
  // Change the wall port for each command by using:
  //    port values can range from 0-255.
  setWallPort(255); // sets port to 255

*/
EthernetUDP Udp;
class Kinet {
  private:
    // destination (remote) ip address
    byte* destip_;
    // destination (remote) port
    unsigned int destport_;
    // whether wall mode is being used
    boolean wall_;
    // the size of the buffer
    static const int buffer_size = 536;
    // the size of the header (21 or 24 bytes)
    int header_size;
    // the buffer that is being sent
    byte* buffer;
  public:
    Kinet(byte* destip, unsigned int destport, boolean wall = false): destip_(destip), destport_(destport), wall_(wall) {
      // allocating the buffer in this way automatically zeros all values.
      // we add one to the buffer_size to save space for the null character needed when writing out to UDP with sendPacket.
      buffer = (byte*) calloc(buffer_size, sizeof(byte));
      // initialize the header
      setWall(wall, true);
    }
    ~Kinet() {
      free(buffer);
    }

    void setWall(boolean wall, boolean force = false) {
      // only do something if the wall value is different
      if (force || wall != wall_) {
        // save off the wall value
        wall_ = wall;
        // initialize the part of the header that is always the same
        buffer[0] = 0x04;
        buffer[1] = 0x01;
        buffer[2] = 0xDC;
        buffer[3] = 0x4A;
        buffer[4] = 0x01;
        buffer[7] = 0x01;
        // initialize the variable header and header size
        if (wall) {
          header_size = 24;
          buffer[6] = 0x08;
          buffer[21] = 0x02;
          buffer[22] = 0xF0;
          buffer[23] = 0xFF;
        } else {
          header_size = 21;
          buffer[6] = 0x01;
          buffer[16] = 0xFF;
          buffer[17] = 0xFF;
          buffer[18] = 0xFF;
          buffer[19] = 0xFF;
        }
      }
    }

    void setWallPort(int wallport) {
      buffer[16] = (byte)wallport;
    }

    void setRGB(byte light, byte r, byte g, byte b) {
      int offset = header_size + 3 * light;
      if (light >= 0 && offset + 2 < buffer_size) {
        buffer[offset] = r;
        buffer[offset + 1] = g;
        buffer[offset + 2] = b;
      }
    }

    void setAllRGB(byte r, byte g, byte b) {
      byte lights = floor((buffer_size - header_size) / 3);
      for (int l = 0; l < lights; l++) {
        setRGB(l, r, g, b);
      }
    }

    void send() {
      // Udp.sendPacket((uint8_t*)buffer,buffer_size,destip_,destport_);

      // Udp.beginPacket(address, 123); //NTP requests are to port 123
      //Serial.println("4");
      Udp.beginPacket(destip_, destport_);
      Udp.write((uint8_t*)buffer, buffer_size);
      //Serial.println("5");
      // Serial.write((uint8_t*)buffer,buffer_size);
      Udp.endPacket();
    }

    void printBuffer() {
      for (int i = 0; i < buffer_size; i++) {
        Serial.print(buffer[i], DEC);
        Serial.print(" ");
        Serial.flush();
      }
      Serial.println();
    }

};


Kinet kinet(remote_ip, remote_port);

void setup() {
  // REQUIRED: Start the Ethernet and UDP:
  Ethernet.begin(local_mac, local_ip);
  Udp.begin(local_port);
  // Continue your setup here...
  Serial.begin(57600);
  Serial.println("ETHERNETTTYYYYY");
  pinMode(plusButton, INPUT_PULLUP);
}

void loop() {

  getInput2();
  //simpleRGB();
rainbow1(r);


  //rainbow1(30);
  // build the command, so the first light is all WHITE!
  kinet.send(); // send the command
  delay(10); // wait for 1 second

}

void simpleRGB()
{
  kinet.setRGB(currentLight, r, g, b);
}
//

void rainbow1(int wait) {
  static uint8_t hue;
  hue = hue + 1;
//  for (int c=0; c<10, c++;){
  // Use FastLED automatic HSV->RGB conversion
  goRGB( CHSV( hue, 255, b) );
  kinet.setRGB(currentLight, r2, g2, b2);
  
   goRGB( CHSV( hue+g, 255, b) );
  kinet.setRGB(1, r2, g2, b2);
  
   goRGB( CHSV( hue+(g*2), 255, b) );
  kinet.setRGB(2, r2, g2, b2);
  
    goRGB( CHSV( hue+(g*3), 255, b) );
  kinet.setRGB(3, r2, g2, b2);
  
    goRGB( CHSV( hue+(g*4), 255, b) );
  kinet.setRGB(4, r2, g2, b2);
  
    goRGB( CHSV( hue+(g*5), 255, b) );
  kinet.setRGB(5, r2, g2, b2);
  
  
 // }
  delay(wait);
}


void getInput()
{ if (digitalRead(plusButton) != 1) {
    currentLight++;
    delay(100);
  }
  r = (map(analogRead(A0), 0, 1023, 0, 255));
  g = (map(analogRead(A1), 0, 1023, 0, 255));
  b = (map(analogRead(A2), 0, 1023, 0, 255));
  if (currentLight > maxLights) {
    currentLight = 0;
  }
}

void getInput2()
{ if (digitalRead(plusButton) != 1) {
    currentLight++;
    delay(100);
  }
  r = (map(analogRead(A0), 0, 1023, 5, 70));
  g = (map(analogRead(A1), 0, 1023, 0, 64));
  b = (map(analogRead(A2), 0, 1023, 0, 255));
  if (currentLight > maxLights) {
    currentLight = 0;
  }
}

void goRGB( const CRGB& rgb)
{
  r2 = rgb.r ;
  g2 = rgb.g ;
  b2 = rgb.b ;
}


