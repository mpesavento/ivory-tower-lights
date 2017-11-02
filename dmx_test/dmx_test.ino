#include <DmxSimple.h>
#include <FastLED.h>

#define DMX_PIN     3
#define COLOR_ORDER RGB  //note that this does not include AW-UV channels

#define NUM_LEDS    40

CRGB leds[NUM_LEDS];

void setup() {
  delay(100); // sanity delay
  FastLED.addLeds<DMXSIMPLE, DMX_PIN, COLOR_ORDER>(leds, NUM_LEDS );
  FastLED.setBrightness( 255 );

//  DmxSimple.maxChannel(12);
}

void loop() {
  static uint8_t hue = 0;
  leds[0] = CHSV(hue, 255, 255);

  hue = hue+1 % 256;

  FastLED.show();
  delay(100);

//// simple DMX test, doens't work?  
//  int channel = 2;
//  for (int value = 0; value <= 255; value++) {
//    for (int c=1; c<6; c++) {
//      DmxSimple.write(c, value); /* Update DMX channel 1 to new brightness */
//    }
//    delay(100); /* Small delay to slow down the ramping */
//  }

}
