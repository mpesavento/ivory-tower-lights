//************************************************
// Light patterns for the Ivory Tower
// 
// @author Mike Pesavento
// @date 2017.08.22
//************************************************

#include <DmxSimple.h>
#include <FastLED.h>
#include <EEPROM.h>
#define FASTSPI_USE_DMX_SIMPLE 1

// for testing on APA102
#define DATA_PIN   11
#define CLOCK_PIN  13
#define CHIPSET    APA102
#define COLOR_ORDER BGR  // most of the 10mm black APA102

// for testing on DMX
#define DMX_CHANNEL_PER_LIGHT 11

#define DMX_TOTAL_CHANNELS DMX_CHANNEL_PER_LIGHT * 8
#define DMX_PIN     3
#define COLOR_ORDER RGB  //note that this does not include AW-UV channels

#define NUM_LEDS    8
#define BRIGHTNESS  255
#define FRAMES_PER_SECOND 60

#define MIRROR 1 // 1 if we split the array and mirror it, 0 if we start on one side

#define SHIFT_DELAY_MS 80 // how many milliseconds we wait before we shift to the next value on the palette

#define PATTERN_DUR_SEC 10  // how many seconds we stay on a pattern
#define TRANSITION_DUR_SEC 10  // how many seconds it takes to do a transition from one pattern to the next

// utility method, useful for identifying number of patterns we have available
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

//*******************************************
//***** globals
CRGB leds[NUM_LEDS];

// create another array to hold the Amber, White, and UV channels
CRGB leds_awu[NUM_LEDS];

uint8_t dmx_channels[96];

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

// store the last pattern index, to reload same pattern on next reset
uint8_t gCurrentPatternNumber = 255; // Index number of which pattern is current
int eeprom_addr = 1; //current byte address we save pattern index to in EEPROM
int eeprom_check = 0; // the byte to set if we have saved the eeprom value



//*******************************************
//***** setup

void setup() {
  delay(100); // sanity delay
  //DmxSimple.usePin(DMX_PIN);
  FastLED.addLeds<DMXSIMPLE, DMX_PIN, COLOR_ORDER>(leds, NUM_LEDS );
//  FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 2000);

  FastLED.setBrightness( BRIGHTNESS );

    // set initial palette
  currentPalette = OceanColors_p;
  currentBlending = LINEARBLEND;

  // load EEPROM
  int value = EEPROM.read(eeprom_check);
  if (value != 1) {
    EEPROM.write(eeprom_check, 1);
    EEPROM.write(eeprom_addr, 0);
  }
  else {
    gCurrentPatternNumber = EEPROM.read(eeprom_addr);
  }

  Serial.begin(57600);
}


//*******************************************
// pattern iteration list
//     We put this below setup() because it needs to find the methods below to load the method array

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {rainbow, water_colors, party_colors, sinelon };


//*******************************************
//***** loop

void loop()
{
  changePalettePeriodically();
  gPatterns[gCurrentPatternNumber]();
  FastLED.show();
  delayToSyncFrameRate(FRAMES_PER_SECOND);
}


//*******************************************
//***** support methods

void fadeall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(250);
  }
}


void nextPattern() {
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
  EEPROM.write(eeprom_addr, gCurrentPatternNumber);
}


void changePalettePeriodically() {
  // this is the timing engine and state machine that identifies when we play a particular palette

  static uint16_t last_transition_sec = 0;
  uint16_t current_time = (millis() / 1000);
  uint16_t deltat = current_time - last_transition_sec;
  if (deltat > PATTERN_DUR_SEC) {
    Serial.print("change to pattern ");
    Serial.println(gCurrentPatternNumber);
    nextPattern();  
    last_transition_sec = current_time;
  }
}


void fill_leds_from_palette( uint8_t colorIndex, uint8_t offset=3)
{
  uint8_t brightness = BRIGHTNESS;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
    colorIndex += offset;
  }
  if (MIRROR) {
    for(int i=0; i < int(NUM_LEDS/2); i++) 
      leds[NUM_LEDS - i - 1] = leds[i];
  }
}


uint8_t delayed_index_shift(uint8_t palette_index, int delay_ms, uint8_t offset_index = 1) {
  // only update the index shift by offset_index (typically 1) every delay_ms
  EVERY_N_MILLISECONDS( delay_ms ) {
    palette_index += offset_index; // slowly cycle the "base color"
  } 
  return palette_index;
}

void rainbow() {
  // FastLED's built-in rainbow generator
  static int offset = 0;
  fill_rainbow( leds, NUM_LEDS, offset, 256/NUM_LEDS);
  offset = delayed_index_shift(offset, 30, 1);
}


void water_colors() {
  static uint8_t palette_index = 0; // rotating "base color" used by many of the patterns
  currentPalette = OceanColors_p;
  currentBlending = LINEARBLEND;
  fill_leds_from_palette(palette_index, 16);
  
  palette_index = delayed_index_shift(palette_index, 80); // do periodic update
}


void party_colors() {
  static uint8_t palette_index = 0; // rotating "base color" used by many of the patterns
  currentPalette = PartyColors_p;
  currentBlending = LINEARBLEND;

  fill_leds_from_palette(palette_index, 16);

  palette_index = delayed_index_shift(palette_index, 80); // do periodic update
}


void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  static uint8_t palette_index = 0; // rotating "base color" used by many of the patterns
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS-1);
  leds[pos] += CHSV( palette_index, 255, 192); // this sweeps through all hues
  
  palette_index = delayed_index_shift(palette_index, 80, 1); // do periodic update
}



void SetupPhagePalette()
{
  // 'black out' all 16 palette entries...
  // fill_solid( currentPalette, 16, CRGB::Black);
  // and set every fourth one to white.
  currentPalette[0] = CHSV(0, 255, 255);
  currentPalette[4] = CHSV(16, 255, 255);
  currentPalette[8] = CHSV(32, 255, 255);
  currentPalette[12] = CHSV(128, 255, 255);

}


static uint16_t getDeltaT() {
  // return the time delta in milliseconds since the last time this method was called
  // This is used to independently find the time since last frame
  static uint32_t msprev = 0;
  uint32_t mscur = millis();
  uint16_t deltat  = (uint16_t)(mscur - msprev);
  msprev = mscur;  // update the time of last read; this should be at end of loop
  return deltat;
}


// delayToSyncFrameRate - delay how many milliseconds are needed
//   to maintain a stable frame rate.
static void delayToSyncFrameRate( uint8_t framesPerSecond) {
  // separate this from getDeltaT() to have different tracking variable
  static uint32_t msprev_frame = 0;
  uint32_t mscur = millis();
  uint16_t deltat  = (uint16_t)(mscur - msprev_frame);
  msprev_frame = mscur;  // update the time of last read; this should be at end of loop
  uint16_t mstargetdelta = 1000 / framesPerSecond;
  // Serial.print("frame dt: ");
  // Serial.println(msdelta);
  if ( deltat < mstargetdelta) {
    FastLED.delay( mstargetdelta - deltat);
  }

}


