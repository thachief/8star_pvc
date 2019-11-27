#include "FastLED.h"

// Good Reference!
// http://fastled.io/docs/3.1/group___colorutils.html#gafcc7dac88e25736ebc49a9faf2a1c2e2


#define DATA_PIN    4   // was 10 for "non production" units
#define BRIGHTNESS_PIN    A1
#define COLOR_ORDER RGB
#define LED_TYPE    WS2812
#define NUM_LEDS    100

#define FRAMES_PER_SECOND  120



// original limits:
//#define MASTER_BRIGHTNESS 30
#define MASTER_BRIGHTNESS 64
//#define MASTER_BRIGHTNESS 128 // Set the master brigtness value [should be greater then min_brightness value].
#define NUM_VIRTUAL_LEDS NUM_LEDS+1
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
CRGB leds[NUM_VIRTUAL_LEDS];


// PVC Spoke Structures
const uint8_t iSpokeCount = 8;
const uint8_t iSpokeLength = 12;
// Wierd wiring... see notes below, this is a work around for me.  (10/27/2019 - first unit.)
static const uint8_t spoke1Array[] PROGMEM = { 15, 4,  14, 5,  13, 6,  12, 7,  11, 8,  10, 9 };
static const uint8_t spoke2Array[] PROGMEM = { 27, 16, 26, 17, 25, 18, 24, 19, 23, 20, 22, 21 };
static const uint8_t spoke3Array[] PROGMEM = { 39, 28, 38, 29, 37, 30, 36, 31, 35, 32, 34, 33 };
static const uint8_t spoke4Array[] PROGMEM = { 51, 40, 50, 41, 49, 42, 48, 43, 47, 44, 46, 45 };
static const uint8_t spoke5Array[] PROGMEM = { 63, 52, 62, 53, 61, 54, 60, 55, 59, 56, 58, 57 };
static const uint8_t spoke6Array[] PROGMEM = { 75, 64, 74, 65, 73, 66, 72, 67, 71, 68, 70, 69 };
static const uint8_t spoke7Array[] PROGMEM = { 87, 76, 86, 77, 85, 78, 84, 79, 83, 80, 82, 81 };
static const uint8_t spoke8Array[] PROGMEM = { 99, 88, 98, 89, 97, 90, 96, 91, 95, 92, 94, 93 };

/*  This woudld be the correct allocation if I wired things correctly, unfortunately, I installed
 *  the pixel string backwards and do not have the desire to change the installation on this unit now. 
 *  Saving the definition for future builds... which should be done correctly!!
 *  
 *  Actually these are all off by +1, need to resolve later (TODO)
 *  
static const uint8_t spoke1Array[] PROGMEM = { 1, 12, 2,  11, 3,  10, 4,  9,  5,  8,  6,  7   };
static const uint8_t spoke2Array[] PROGMEM = { 13, 24, 14, 23, 15, 22, 16, 21, 17, 20, 18, 19 };
static const uint8_t spoke3Array[] PROGMEM = { 25, 36, 26, 35, 27, 34, 28, 33, 29, 32, 30, 31 };
static const uint8_t spoke4Array[] PROGMEM = { 37, 48, 38, 47, 39, 46, 40, 45, 41, 44, 42, 43 };
static const uint8_t spoke5Array[] PROGMEM = { 49, 60, 50, 59, 51, 58, 52, 57, 53, 56, 54, 55 };
static const uint8_t spoke6Array[] PROGMEM = { 61, 72, 62, 71, 63, 70, 64, 69, 65, 68, 66, 67 };
static const uint8_t spoke7Array[] PROGMEM = { 73, 84, 74, 83, 75, 82, 76, 81, 77, 80, 78, 79 };
static const uint8_t spoke8Array[] PROGMEM = { 85, 96, 86, 95, 87, 94, 88, 93, 89, 92, 90, 91 };
 */


// global for color changing (red/white/green)
uint8_t passnum = 0;



// =============================================================================
//   setup routine
// =============================================================================
// put your setup code here, to run once:
void setup() {
  delay(1000); // 3 second delay for recovery
  Serial.begin(57600);
    
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();

//SimplePatternList gPatterns = { twinklefox };
// 9/4/2017 -- marque_v3 is failing with Snowflake.... look at hardcoded variables or change some other variable value?
//SimplePatternList gPatterns = { spiral_jen, burst_progressive_speed, TestGlitter, brightness_burst, rotate_spokes_sequentially_random_color, gradient_fill_allbranches, rotate_spokes_sequentially_set_color, three_segment_spin  };   
//SimplePatternList gPatterns = { gradient_fill_allbranches, rotate_spokes_sequentially_set_color };  
SimplePatternList gPatterns = { spiral_jen, TestGlitter, burst_progressive_speed, rotate_spokes_sequentially_set_color, brightness_burst, gradient_fill_allbranches, three_segment_spin, rotate_spokes_sequentially_random_color  };   

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns


// =============================================================================
//   loop routine 
// =============================================================================
// put your main code here, to run repeatedly:
void loop() {

  FastLED.setBrightness(MASTER_BRIGHTNESS);       

  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();


  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 


  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 12 ) { nextPattern(); } // change patterns periodically
}



// =============================================================================
//  Sub-Routines
// =============================================================================


void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}




// ========================================
//  Routines for Pattern Display
// ========================================



// =============================================
// Fill entire branch sub routines -- set colors

void fill_spoke1_set_color(int color_code) {
    
  for (int i=0; i<iSpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&spoke1Array[i]);
//    leds[offset] = color_code;
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_spoke2_set_color(int color_code) {
    
  for (int i=0; i<iSpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&spoke2Array[i]);
//    leds[offset] = color_code;
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_spoke3_set_color(int color_code) {
    
  for (int i=0; i<iSpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&spoke3Array[i]);
//    leds[offset] = color_code;
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_spoke4_set_color(int color_code) {
    
  for (int i=0; i<iSpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&spoke4Array[i]);
//    leds[offset] = color_code;
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_spoke5_set_color(int color_code) {
    
  for (int i=0; i<iSpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&spoke5Array[i]);
//    leds[offset] = color_code;
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_spoke6_set_color(int color_code) {
    
  for (int i=0; i<iSpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&spoke6Array[i]);
//    leds[offset] = color_code;
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_spoke7_set_color(int color_code) {
    
  for (int i=0; i<iSpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&spoke7Array[i]);
//    leds[offset] = color_code;
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

void fill_spoke8_set_color(int color_code) {
    
  for (int i=0; i<iSpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&spoke8Array[i]);
//    leds[offset] = color_code;
    leds[offset] = CHSV(color_code, 255, 255);
  }        
  FastLED.show();
}

// =============================================
// Fill entire branch sub routines -- random

void fill_spoke1_random() {

  uint8_t hue1 = random8(255);
    
  for (int i=0; i<iSpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&spoke1Array[i]);
    leds[offset] = CHSV(hue1, 255, 255);
  }        
  FastLED.show();
}


void fill_spoke2_random() {

  uint8_t hue1 = random8(255);
    
  for (int i=0; i<iSpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&spoke2Array[i]);
    leds[offset] = CHSV(hue1, 255, 255);
  }        
  FastLED.show();
}

void fill_spoke3_random() {

  uint8_t hue1 = random8(255);
    
  for (int i=0; i<iSpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&spoke3Array[i]);
    leds[offset] = CHSV(hue1, 255, 255);
  }        
  FastLED.show();
}

void fill_spoke4_random() {

  uint8_t hue1 = random8(255);
    
  for (int i=0; i<iSpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&spoke4Array[i]);
    leds[offset] = CHSV(hue1, 255, 255);
  }        
  FastLED.show();
}

void fill_spoke5_random() {
  
  uint8_t hue1 = random8(255);
    
  for (int i=0; i<iSpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&spoke5Array[i]);
    leds[offset] = CHSV(hue1, 255, 255);
  }        
  FastLED.show();
}

void fill_spoke6_random() {

  uint8_t hue1 = random8(255);
    
  for (int i=0; i<iSpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&spoke6Array[i]);
    leds[offset] = CHSV(hue1, 255, 255);
  }        
  FastLED.show();
}

void fill_spoke7_random() {

  uint8_t hue1 = random8(255);
    
  for (int i=0; i<iSpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&spoke7Array[i]);
    leds[offset] = CHSV(hue1, 255, 255);
  }        
  FastLED.show();
}

void fill_spoke8_random() {

  uint8_t hue1 = random8(255);
    
  for (int i=0; i<iSpokeLength; i++) {
    uint8_t offset = pgm_read_byte(&spoke8Array[i]);
    leds[offset] = CHSV(hue1, 255, 255);
  }        
  FastLED.show();
}

void rotate_spokes_sequentially_set_color() {
  
  for( int run_number = 1; run_number <= iSpokeCount; run_number++){
    gHue=gHue+3;
    
    // change the spoke every cycle through
    switch (run_number) {
    case 1:
      fill_spoke1_set_color(gHue); 
      break;
    case 2:
      fill_spoke2_set_color(gHue); 
      break;
    case 3:
      fill_spoke3_set_color(gHue); 
      break;
    case 4:
      fill_spoke4_set_color(gHue); 
      break;
    case 5:
      fill_spoke5_set_color(gHue); 
      break;
    case 6:
      fill_spoke6_set_color(gHue); 
      break;
    case 7:
      fill_spoke7_set_color(gHue); 
      break;
    case 8:
      fill_spoke8_set_color(gHue); 
      break;
    default:
      // keep other color
      break;
    }
    delay(75);
  }
}


void rotate_spokes_sequentially_random_color() {

  FastLED.clear();

  for( int run_number = 1; run_number <= iSpokeCount; run_number++){
  
    // change the spoke every cycle through
    switch (run_number) {
    case 1:
      fill_spoke1_random(); 
      break;
    case 2:
      fill_spoke2_random(); 
      break;
    case 3:
      fill_spoke3_random(); 
      break;
    case 4:
      fill_spoke4_random(); 
      break;
    case 5:
      fill_spoke5_random(); 
      break;
    case 6:
      fill_spoke6_random(); 
      break;
    case 7:
      fill_spoke7_random(); 
      break;
    case 8:
      fill_spoke8_random(); 
      break;
    default:
      // keep other color
      break;
    }
    delay(120);
    FastLED.clear();  
  }
}



// Oct 26, 2019 - updated for PVC structure
void gradient_fill_allbranches() {

  uint8_t offset_br1;
  uint8_t offset_br2;
  uint8_t offset_br3;
  uint8_t offset_br4;
  uint8_t offset_br5;
  uint8_t offset_br6;
  uint8_t offset_br7;
  uint8_t offset_br8;

  uint8_t hue1 = random8(255);
//  uint8_t hue2 = hue1 + random8(30,61);
  uint8_t hue2 = hue1 + random8(10,20);
  uint8_t offset2;

  // this is where the magic happens -- define temp array to hold values
  CRGB grad[iSpokeLength];  // A place to save the gradient colors. (Don't edit this)
  // fill "holder array - grad" then copy values to each branch in the array below
  fill_gradient (grad, 0, CHSV(hue1, 255, 255), iSpokeLength, CHSV(hue2, 255, 255), SHORTEST_HUES);


  // display each pixel one at a time
  for( int i = 0; i < iSpokeLength; i++){

    offset_br1 = pgm_read_byte(&spoke1Array[i]);
    offset_br2 = pgm_read_byte(&spoke2Array[i]);
    offset_br3 = pgm_read_byte(&spoke3Array[i]);
    offset_br4 = pgm_read_byte(&spoke4Array[i]);
    offset_br5 = pgm_read_byte(&spoke5Array[i]);
    offset_br6 = pgm_read_byte(&spoke6Array[i]);
    offset_br7 = pgm_read_byte(&spoke7Array[i]);
    offset_br8 = pgm_read_byte(&spoke8Array[i]);
    
    leds[offset_br1] = grad[i];
    leds[offset_br2] = grad[i];
    leds[offset_br3] = grad[i];
    leds[offset_br4] = grad[i];
    leds[offset_br5] = grad[i];
    leds[offset_br6] = grad[i];
    leds[offset_br7] = grad[i];
    leds[offset_br8] = grad[i];
    
    delay(18);
    FastLED.show();
  }
  //FastLED.clear();
}


void brightness_burst() {

  uint8_t offset_br1;
  uint8_t offset_br2;
  uint8_t offset_br3;
  uint8_t offset_br4;
  uint8_t offset_br5;
  uint8_t offset_br6;
  uint8_t offset_br7;
  uint8_t offset_br8;

  uint8_t hue1 = random8(255);
  uint8_t offset2;

  int brightness_array[12] = { 255, 234, 213, 192, 171, 150, 129, 108, 87, 66, 45, 24 };
 

/*
  // this is where the magic happens -- define temp array to hold values
  CRGB grad[iSpokeLength];  // A place to save the gradient colors. (Don't edit this)
  // fill "holder array - grad" then copy values to each branch in the array below
  fill_gradient (grad, 0, CHSV(hue1, 255, 255), iSpokeLength, CHSV(hue2, 255, 255), SHORTEST_HUES);
*/

  CRGB color_code = CHSV(0, 0, 255);
  
//  // fill all pixels 
//  for( int i = 0; i < iSpokeLength; i++){
//    offset_br1 = pgm_read_byte(&spoke1Array[i]);
//    offset_br2 = pgm_read_byte(&spoke2Array[i]);
//    offset_br3 = pgm_read_byte(&spoke3Array[i]);
//    offset_br4 = pgm_read_byte(&spoke4Array[i]);
//    offset_br5 = pgm_read_byte(&spoke5Array[i]);
//    offset_br6 = pgm_read_byte(&spoke6Array[i]);
//    offset_br7 = pgm_read_byte(&spoke7Array[i]);
//    offset_br8 = pgm_read_byte(&spoke8Array[i]);
//    
//    leds[offset_br1] = color_code;
//    leds[offset_br2] = color_code;
//    leds[offset_br3] = color_code;
//    leds[offset_br4] = color_code;
//    leds[offset_br5] = color_code;
//    leds[offset_br6] = color_code;
//    leds[offset_br7] = color_code;
//    leds[offset_br8] = color_code;    
//  }
//  FastLED.show();
//  delay(500);

  int pos;

  for( int j = iSpokeLength; j > 0; j--){
    
    for( int i = 0; i < iSpokeLength; i++){

      pos = (j+i) % iSpokeLength;

      offset_br1 = pgm_read_byte(&spoke1Array[i]);
      offset_br2 = pgm_read_byte(&spoke2Array[i]);
      offset_br3 = pgm_read_byte(&spoke3Array[i]);
      offset_br4 = pgm_read_byte(&spoke4Array[i]);
      offset_br5 = pgm_read_byte(&spoke5Array[i]);
      offset_br6 = pgm_read_byte(&spoke6Array[i]);
      offset_br7 = pgm_read_byte(&spoke7Array[i]);
      offset_br8 = pgm_read_byte(&spoke8Array[i]);

      //color_code = CHSV(0, 0, 255);
      leds[offset_br1] = CHSV(0, 0, brightness_array[pos]);
      leds[offset_br2] = CHSV(0, 0, brightness_array[pos]);
      leds[offset_br3] = CHSV(0, 0, brightness_array[pos]);
      leds[offset_br4] = CHSV(0, 0, brightness_array[pos]);
      leds[offset_br5] = CHSV(0, 0, brightness_array[pos]);
      leds[offset_br6] = CHSV(0, 0, brightness_array[pos]);
      leds[offset_br7] = CHSV(0, 0, brightness_array[pos]);
      leds[offset_br8] = CHSV(0, 0, brightness_array[pos]);

  
//      leds[offset_br1].nscale8(brightness_array[pos]);
//      leds[offset_br2].nscale8(brightness_array[pos]);
//      leds[offset_br3].nscale8(brightness_array[pos]);
//      leds[offset_br4].nscale8(brightness_array[pos]);
//      leds[offset_br5].nscale8(brightness_array[pos]);
//      leds[offset_br6].nscale8(brightness_array[pos]);
//      leds[offset_br7].nscale8(brightness_array[pos]);
//      leds[offset_br8].nscale8(brightness_array[pos]);

      Serial.print("i=");Serial.print(i);
      Serial.print(" j=");Serial.print(j);
      Serial.print(" pos=");Serial.print(pos);
      Serial.print(" br_array[pos]=");Serial.println(brightness_array[pos]);
    }  
    FastLED.show();
    delay(20);
  }
//    int value = 128;
//    leds[offset_br1].nscale8(value);
//    leds[offset_br2].nscale8(value);
//    leds[offset_br3].nscale8(value);
//    leds[offset_br4].nscale8(value);
//    leds[offset_br5].nscale8(value);
//    leds[offset_br6].nscale8(value);
//    leds[offset_br7].nscale8(value);
//    leds[offset_br8].nscale8(value);
}





void burst_progressive_speed() {

  uint8_t offset_br1;
  uint8_t offset_br2;
  uint8_t offset_br3;
  uint8_t offset_br4;
  uint8_t offset_br5;
  uint8_t offset_br6;
  uint8_t offset_br7;
  uint8_t offset_br8;

  uint8_t hue1 = random8(255);
  uint8_t sat1 = random8(255);
  int iDelay = 400;

  
  // this pattern defines the spacing of pixels per spoke, adding more zeros gives more definition
//  int pattern_array[12] = {0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 }; 
  int pattern_array[12] = {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 }; 
 
  CRGB color_code = CHSV(hue1, sat1, 255);
  int pos;

  for( int j = 0; j < iSpokeLength*10; j++){
    
    for( int i = 0; i < iSpokeLength; i++){

      pos = (j+i) % iSpokeLength;

      offset_br1 = pgm_read_byte(&spoke1Array[i]);
      offset_br2 = pgm_read_byte(&spoke2Array[i]);
      offset_br3 = pgm_read_byte(&spoke3Array[i]);
      offset_br4 = pgm_read_byte(&spoke4Array[i]);
      offset_br5 = pgm_read_byte(&spoke5Array[i]);
      offset_br6 = pgm_read_byte(&spoke6Array[i]);
      offset_br7 = pgm_read_byte(&spoke7Array[i]);
      offset_br8 = pgm_read_byte(&spoke8Array[i]);

      //color_code = CHSV(0, 0, 255);
      if (pattern_array[pos] == 1) { 
        leds[offset_br1] = color_code;
      } else { 
        leds[offset_br1] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_br2] = color_code;
      } else { 
        leds[offset_br2] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_br3] = color_code;
      } else { 
        leds[offset_br3] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_br4] = color_code;
      } else { 
        leds[offset_br4] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_br5] = color_code;
      } else { 
        leds[offset_br5] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_br6] = color_code;
      } else { 
        leds[offset_br6] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_br7] = color_code;
      } else { 
        leds[offset_br7] = CRGB::Black; 
      }
      
      if (pattern_array[pos] == 1) { 
        leds[offset_br8] = color_code;
      } else { 
        leds[offset_br8] = CRGB::Black; 
      }

    }  
    FastLED.show();
    delay(iDelay);

    // shrink the delay every loop
    iDelay = iDelay * 0.97;
  }

  // this is here for debugging, it lets me look at the end result
  if (false) {
    delay (3000);
  } else {
    FastLED.clear();
    FastLED.show();
    delay(500);
    hue1 = random8(255);
    sat1 = 255;
  }
}



void three_segment_spin() {

  // pick a random color
  uint8_t hue1 = random8(255);
  uint8_t hue2 = random8(255);
  uint8_t hue3 = random8(255);

  int pattern_array_inn[12] = {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 }; 
  int pattern_array_mid[12] = {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 }; 
  int pattern_array_out[12] = {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 }; 

  uint8_t pos_inn = -1;
  uint8_t pos_mid = -1;
  //uint8_t pos_mid = +1;  <-- for reserve
  uint8_t pos_out = -1;

  uint8_t offset_loc0;
  uint8_t offset_loc1;
  uint8_t offset_loc2;
  uint8_t offset_loc3;
  uint8_t offset_loc4;
  uint8_t offset_loc5;
  uint8_t offset_loc6;
  uint8_t offset_loc7;
  uint8_t offset_loc8;
  uint8_t offset_loc9;
  uint8_t offset_loc10;
  uint8_t offset_loc11;
  


  //int delay_value = 75;
  int delay_value = 250;
  //int delay_value = 1775;
  //int fade_value = 30;
  int fade_value = 120;
  

  // 3 full circles for slowest ring
  for (int i=0; i<iSpokeCount*5; i++) {

    // Phase 1
    pos_inn = (pos_inn+1) % iSpokeCount;
    pos_mid = (pos_mid+1) % iSpokeCount;
    pos_out = (pos_out+1) % iSpokeCount;
    
    // this is for reverse of the center -- i liked the concept, but it looked wierd when i tested it.
//    pos_mid = (pos_mid-1); 
//    pos_mid = pos_mid % iSpokeCount;    // when pos_mod statement was all one one line this did not work when overflow to 255 occured.  this works ok.

    switch (pos_inn) {
    case 0:
      offset_loc0 = pgm_read_byte(&spoke1Array[0]);
      offset_loc1 = pgm_read_byte(&spoke1Array[1]);
      offset_loc2 = pgm_read_byte(&spoke1Array[2]);
      offset_loc3 = pgm_read_byte(&spoke1Array[3]);
      break;
      
    case 1:
      offset_loc0 = pgm_read_byte(&spoke2Array[0]);
      offset_loc1 = pgm_read_byte(&spoke2Array[1]);
      offset_loc2 = pgm_read_byte(&spoke2Array[2]);
      offset_loc3 = pgm_read_byte(&spoke2Array[3]);
      break;
      
    case 2:
      offset_loc0 = pgm_read_byte(&spoke3Array[0]);
      offset_loc1 = pgm_read_byte(&spoke3Array[1]);
      offset_loc2 = pgm_read_byte(&spoke3Array[2]);
      offset_loc3 = pgm_read_byte(&spoke3Array[3]);
      break;
      
    case 3:
      offset_loc0 = pgm_read_byte(&spoke4Array[0]);
      offset_loc1 = pgm_read_byte(&spoke4Array[1]);
      offset_loc2 = pgm_read_byte(&spoke4Array[2]);
      offset_loc3 = pgm_read_byte(&spoke4Array[3]);
      break;
      
    case 4:
      offset_loc0 = pgm_read_byte(&spoke5Array[0]);
      offset_loc1 = pgm_read_byte(&spoke5Array[1]);
      offset_loc2 = pgm_read_byte(&spoke5Array[2]);
      offset_loc3 = pgm_read_byte(&spoke5Array[3]);
      break;
      
    case 5:
      offset_loc0 = pgm_read_byte(&spoke6Array[0]);
      offset_loc1 = pgm_read_byte(&spoke6Array[1]);
      offset_loc2 = pgm_read_byte(&spoke6Array[2]);
      offset_loc3 = pgm_read_byte(&spoke6Array[3]);
      break;
      
    case 6:
      offset_loc0 = pgm_read_byte(&spoke7Array[0]);
      offset_loc1 = pgm_read_byte(&spoke7Array[1]);
      offset_loc2 = pgm_read_byte(&spoke7Array[2]);
      offset_loc3 = pgm_read_byte(&spoke7Array[3]);
      break;
      
    case 7:
      offset_loc0 = pgm_read_byte(&spoke8Array[0]);
      offset_loc1 = pgm_read_byte(&spoke8Array[1]);
      offset_loc2 = pgm_read_byte(&spoke8Array[2]);
      offset_loc3 = pgm_read_byte(&spoke8Array[3]);
      break;
      
    default:
      // keep other color
      break;
    }


    switch (pos_mid) {
    case 0:
      offset_loc4 = pgm_read_byte(&spoke1Array[4]);
      offset_loc5 = pgm_read_byte(&spoke1Array[5]);
      offset_loc6 = pgm_read_byte(&spoke1Array[6]);
      offset_loc7 = pgm_read_byte(&spoke1Array[7]);
      break;
      
    case 1:
      offset_loc4 = pgm_read_byte(&spoke2Array[4]);
      offset_loc5 = pgm_read_byte(&spoke2Array[5]);
      offset_loc6 = pgm_read_byte(&spoke2Array[6]);
      offset_loc7 = pgm_read_byte(&spoke2Array[7]);
      break;
      
    case 2:
      offset_loc4 = pgm_read_byte(&spoke3Array[4]);
      offset_loc5 = pgm_read_byte(&spoke3Array[5]);
      offset_loc6 = pgm_read_byte(&spoke3Array[6]);
      offset_loc7 = pgm_read_byte(&spoke3Array[7]);
      break;
      
    case 3:
      offset_loc4 = pgm_read_byte(&spoke4Array[4]);
      offset_loc5 = pgm_read_byte(&spoke4Array[5]);
      offset_loc6 = pgm_read_byte(&spoke4Array[6]);
      offset_loc7 = pgm_read_byte(&spoke4Array[7]);
      break;
      
    case 4:
      offset_loc4 = pgm_read_byte(&spoke5Array[4]);
      offset_loc5 = pgm_read_byte(&spoke5Array[5]);
      offset_loc6 = pgm_read_byte(&spoke5Array[6]);
      offset_loc7 = pgm_read_byte(&spoke5Array[7]);
      break;
      
    case 5:
      offset_loc4 = pgm_read_byte(&spoke6Array[4]);
      offset_loc5 = pgm_read_byte(&spoke6Array[5]);
      offset_loc6 = pgm_read_byte(&spoke6Array[6]);
      offset_loc7 = pgm_read_byte(&spoke6Array[7]);
      break;
      
    case 6:
      offset_loc4 = pgm_read_byte(&spoke7Array[4]);
      offset_loc5 = pgm_read_byte(&spoke7Array[5]);
      offset_loc6 = pgm_read_byte(&spoke7Array[6]);
      offset_loc7 = pgm_read_byte(&spoke7Array[7]);
      break;
      
    case 7:
      offset_loc4 = pgm_read_byte(&spoke8Array[4]);
      offset_loc5 = pgm_read_byte(&spoke8Array[5]);
      offset_loc6 = pgm_read_byte(&spoke8Array[6]);
      offset_loc7 = pgm_read_byte(&spoke8Array[7]);
      break;
      
    default:
      // keep other color
      break;
    }

    switch (pos_out) {
    case 0:
      offset_loc8 = pgm_read_byte(&spoke1Array[8]);
      offset_loc9 = pgm_read_byte(&spoke1Array[9]);
      offset_loc10 = pgm_read_byte(&spoke1Array[10]);
      offset_loc11 = pgm_read_byte(&spoke1Array[11]);
      break;
      
    case 1:
      offset_loc8 = pgm_read_byte(&spoke2Array[8]);
      offset_loc9 = pgm_read_byte(&spoke2Array[9]);
      offset_loc10 = pgm_read_byte(&spoke2Array[10]);
      offset_loc11 = pgm_read_byte(&spoke2Array[11]);
      break;
      
    case 2:
      offset_loc8 = pgm_read_byte(&spoke3Array[8]);
      offset_loc9 = pgm_read_byte(&spoke3Array[9]);
      offset_loc10 = pgm_read_byte(&spoke3Array[10]);
      offset_loc11 = pgm_read_byte(&spoke3Array[11]);
      break;
      
    case 3:
      offset_loc8 = pgm_read_byte(&spoke4Array[8]);
      offset_loc9 = pgm_read_byte(&spoke4Array[9]);
      offset_loc10 = pgm_read_byte(&spoke4Array[10]);
      offset_loc11 = pgm_read_byte(&spoke4Array[11]);
      break;
      
    case 4:
      offset_loc8 = pgm_read_byte(&spoke5Array[8]);
      offset_loc9 = pgm_read_byte(&spoke5Array[9]);
      offset_loc10 = pgm_read_byte(&spoke5Array[10]);
      offset_loc11 = pgm_read_byte(&spoke5Array[11]);
      break;
      
    case 5:
      offset_loc8 = pgm_read_byte(&spoke6Array[8]);
      offset_loc9 = pgm_read_byte(&spoke6Array[9]);
      offset_loc10 = pgm_read_byte(&spoke6Array[10]);
      offset_loc11 = pgm_read_byte(&spoke6Array[11]);
      break;
      
    case 6:
      offset_loc8 = pgm_read_byte(&spoke7Array[8]);
      offset_loc9 = pgm_read_byte(&spoke7Array[9]);
      offset_loc10 = pgm_read_byte(&spoke7Array[10]);
      offset_loc11 = pgm_read_byte(&spoke7Array[11]);
      break;
      
    case 7:
      offset_loc8 = pgm_read_byte(&spoke8Array[8]);
      offset_loc9 = pgm_read_byte(&spoke8Array[9]);
      offset_loc10 = pgm_read_byte(&spoke8Array[10]);
      offset_loc11 = pgm_read_byte(&spoke8Array[11]);
      break;
      
    default:
      // keep other color
      break;
    }


  
    leds[offset_loc0] = CHSV(hue1, 255, 255);
    leds[offset_loc1] = CHSV(hue1, 255, 255);
    leds[offset_loc2] = CHSV(hue1, 255, 255);
    leds[offset_loc3] = CHSV(hue1, 255, 255);
    
    leds[offset_loc4] = CHSV(hue2, 255, 255);
    leds[offset_loc5] = CHSV(hue2, 255, 255);
    leds[offset_loc6] = CHSV(hue2, 255, 255);
    leds[offset_loc7] = CHSV(hue2, 255, 255);
    
    leds[offset_loc8] = CHSV(hue3, 255, 255);
    leds[offset_loc9] = CHSV(hue3, 255, 255);
    leds[offset_loc10] = CHSV(hue3, 255, 255);
    leds[offset_loc11] = CHSV(hue3, 255, 255);



//
//    pos_mid = middle_ring_index(index_middle+1);
//    pos_out =  outer_ring_index(index_outer+1);
//    leds[index_inner] = CHSV(hue1, 255, 255);
//    leds[index_middle] = CHSV(hue2, 255, 255);
//    leds[index_outer] = CHSV(hue3, 255, 255);
  
    FastLED.show();
    delay (delay_value);
    fadeToBlackBy( leds, NUM_LEDS, fade_value);

//
//    // Phase 2
//    index_outer =  outer_ring_index(index_outer+1);
//    leds[index_outer] = CHSV(hue3, 255, 255);
//
//    FastLED.show();
//    delay (delay_value);
//    fadeToBlackBy( leds, NUM_LEDS, fade_value);
//
//
//    // Phase 3
//    index_middle = middle_ring_index(index_middle+1);
//    index_outer =  outer_ring_index(index_outer+1);
//    leds[index_middle] = CHSV(hue2, 255, 255);
//    leds[index_outer] = CHSV(hue3, 255, 255);
//  
//    FastLED.show();
//    delay (delay_value);
//    fadeToBlackBy( leds, NUM_LEDS, fade_value);
//
//
//    // Phase 4
//    index_outer =  outer_ring_index(index_outer+1);
//    leds[index_outer] = CHSV(hue3, 255, 255);
//
//    FastLED.show();
//    delay (delay_value);
//    fadeToBlackBy( leds, NUM_LEDS, fade_value);

  }
  
}


void spiral_jen() {

  uint8_t pos;
  uint8_t hue1 = random8(255);  // pick a random color

  for (int j=0; j<iSpokeLength; j++) {
    for (int i=0; i<iSpokeCount; i++) {

      // i entered the case backwards so this spiral went in reverse of the others
      switch (i) {
      case 0:
        pos = pgm_read_byte(&spoke8Array[j]);
        break;
      case 1:
        pos = pgm_read_byte(&spoke7Array[j]);
        break;
      case 2:
        pos = pgm_read_byte(&spoke6Array[j]);
        break;      
      case 3:
        pos = pgm_read_byte(&spoke5Array[j]);
        break;
      case 4:
        pos = pgm_read_byte(&spoke4Array[j]);
        break;      
      case 5:
        pos = pgm_read_byte(&spoke3Array[j]);
        break;      
      case 6:
        pos = pgm_read_byte(&spoke2Array[j]);
        break;
      case 7:
        pos = pgm_read_byte(&spoke1Array[j]);
        break;      
      default:
        break;
      }

      if (passnum == 0) {
        leds[pos] = CRGB::Red;
      } else if (passnum == 1) {
        leds[pos] = CRGB::White;
      } else if (passnum == 2) {
        leds[pos] = CRGB::Green;
      } else {
        leds[pos] = CHSV(hue1, 255, 255);
      }
       
      // 
      FastLED.show();
      delay (45);

      //hue1 = hue1+4;
      fadeToBlackBy( leds, NUM_LEDS, 10);
    }
  }
  
  passnum++;
  if (passnum > 2) {
    passnum = 0;
  } 
}


/*

void wipe_left_to_right() {

  const uint8_t NUM_PHASES = 40;


  // pick a random color
  uint8_t hue1 = random8(255);

  // clear the slate
  FastLED.clear();

  for (int j=0; j<8; j++) {
      
    for (int i=1; i<NUM_PHASES+1; i++) {

      // always update the color during each iteration
      //hue1+=3;  // significant color change start to end
      hue1+=1;

      switch (i) {
      case 0:
        // turn all LEDs off
        //FastLED.clear();
        break;
  
      case 1:
        leds[57] = CHSV(hue1, 255, 255);
        leds[58] = CHSV(hue1, 255, 255);
        break;
  
      case 2:
        leds[52] = CHSV(hue1, 255, 255);
        leds[63] = CHSV(hue1, 255, 255);
        break;
  
      case 3:
        leds[51] = CHSV(hue1, 255, 255);
        leds[53] = CHSV(hue1, 255, 255);
        leds[56] = CHSV(hue1, 255, 255);
        leds[59] = CHSV(hue1, 255, 255);
        leds[62] = CHSV(hue1, 255, 255);
        leds[64] = CHSV(hue1, 255, 255);
        break;
        
      case 4:
        leds[50] = CHSV(hue1, 255, 255);
        leds[54] = CHSV(hue1, 255, 255);
        leds[61] = CHSV(hue1, 255, 255);
        leds[65] = CHSV(hue1, 255, 255);
        break;

      case 5:
        leds[37] = CHSV(hue1, 255, 255);
        leds[38] = CHSV(hue1, 255, 255);
        leds[55] = CHSV(hue1, 255, 255);
        leds[60] = CHSV(hue1, 255, 255);
        leds[77] = CHSV(hue1, 255, 255);
        leds[78] = CHSV(hue1, 255, 255);
        break;
  
      case 6:
        leds[49] = CHSV(hue1, 255, 255);
        leds[66] = CHSV(hue1, 255, 255);
        break;
  
      case 7:
        leds[36] = CHSV(hue1, 255, 255);
        leds[39] = CHSV(hue1, 255, 255);
        leds[48] = CHSV(hue1, 255, 255);
        leds[67] = CHSV(hue1, 255, 255);
        leds[76] = CHSV(hue1, 255, 255);
        leds[79] = CHSV(hue1, 255, 255);
        break;
  
      case 8:
        leds[32] = CHSV(hue1, 255, 255);
        leds[35] = CHSV(hue1, 255, 255);
        leds[80] = CHSV(hue1, 255, 255);
        leds[83] = CHSV(hue1, 255, 255);
        break;
  
      case 9:
        leds[31] = CHSV(hue1, 255, 255);
        leds[33] = CHSV(hue1, 255, 255);
        leds[40] = CHSV(hue1, 255, 255);
        leds[47] = CHSV(hue1, 255, 255);
        leds[68] = CHSV(hue1, 255, 255);
        leds[75] = CHSV(hue1, 255, 255);
        leds[82] = CHSV(hue1, 255, 255);
        leds[84] = CHSV(hue1, 255, 255);
        break;
        
      case 10:
        leds[30] = CHSV(hue1, 255, 255);
        leds[34] = CHSV(hue1, 255, 255);
        leds[45] = CHSV(hue1, 255, 255);
        leds[70] = CHSV(hue1, 255, 255);
        leds[81] = CHSV(hue1, 255, 255);
        leds[85] = CHSV(hue1, 255, 255);
        break;
  
      case 11:
        leds[41] = CHSV(hue1, 255, 255);
        leds[44] = CHSV(hue1, 255, 255);
        leds[46] = CHSV(hue1, 255, 255);
        leds[69] = CHSV(hue1, 255, 255);
        leds[71] = CHSV(hue1, 255, 255);
        leds[74] = CHSV(hue1, 255, 255);
        break;
  
      case 12:
        leds[29] = CHSV(hue1, 255, 255);
        leds[2] = CHSV(hue1, 255, 255);
        leds[5] = CHSV(hue1, 255, 255);
        leds[86] = CHSV(hue1, 255, 255);
        break;

      case 13:
        leds[28] = CHSV(hue1, 255, 255);
        leds[22] = CHSV(hue1, 255, 255);
        leds[42] = CHSV(hue1, 255, 255);
        leds[3] = CHSV(hue1, 255, 255);
        leds[4] = CHSV(hue1, 255, 255);
        leds[73] = CHSV(hue1, 255, 255);
        leds[93] = CHSV(hue1, 255, 255);
        leds[87] = CHSV(hue1, 255, 255);
        break;

      case 14:
        leds[27] = CHSV(hue1, 255, 255);
        leds[23] = CHSV(hue1, 255, 255);
        leds[21] = CHSV(hue1, 255, 255);
        leds[43] = CHSV(hue1, 255, 255);
        leds[72] = CHSV(hue1, 255, 255);
        leds[94] = CHSV(hue1, 255, 255);
        leds[92] = CHSV(hue1, 255, 255);
        leds[88] = CHSV(hue1, 255, 255);
        break;
  
      case 15:
        leds[26] = CHSV(hue1, 255, 255);
        leds[24] = CHSV(hue1, 255, 255);
        leds[20] = CHSV(hue1, 255, 255);
        leds[1] = CHSV(hue1, 255, 255);
        leds[6] = CHSV(hue1, 255, 255);
        leds[95] = CHSV(hue1, 255, 255);
        leds[91] = CHSV(hue1, 255, 255);
        leds[89] = CHSV(hue1, 255, 255);
        break;

      case 16:
        leds[25] = CHSV(hue1, 255, 255);
        leds[19] = CHSV(hue1, 255, 255);
        leds[0] = CHSV(hue1, 255, 255);
        leds[7] = CHSV(hue1, 255, 255);
        leds[96] = CHSV(hue1, 255, 255);
        leds[90] = CHSV(hue1, 255, 255);
        break;

// Center Position 
      case 17:
        leds[17] = CHSV(hue1, 255, 255);
        leds[8] = CHSV(hue1, 255, 255);
        break;

      case 18:
        leds[168] = CHSV(hue1, 255, 255);
        leds[18] = CHSV(hue1, 255, 255);
        leds[16] = CHSV(hue1, 255, 255);
        leds[9] = CHSV(hue1, 255, 255);
        leds[97] = CHSV(hue1, 255, 255);
        leds[103] = CHSV(hue1, 255, 255);
        break;
  
      case 19:
        leds[167] = CHSV(hue1, 255, 255);
        leds[169] = CHSV(hue1, 255, 255);
        leds[173] = CHSV(hue1, 255, 255);
        leds[15] = CHSV(hue1, 255, 255);
        leds[10] = CHSV(hue1, 255, 255);
        leds[98] = CHSV(hue1, 255, 255);
        leds[102] = CHSV(hue1, 255, 255);
        leds[104] = CHSV(hue1, 255, 255);
        break;

      case 20:
        leds[166] = CHSV(hue1, 255, 255);
        leds[170] = CHSV(hue1, 255, 255);
        leds[172] = CHSV(hue1, 255, 255);
        leds[150] = CHSV(hue1, 255, 255);
        leds[121] = CHSV(hue1, 255, 255);
        leds[99] = CHSV(hue1, 255, 255);
        leds[101] = CHSV(hue1, 255, 255);
        leds[105] = CHSV(hue1, 255, 255);
        break;

      case 21:
        leds[165] = CHSV(hue1, 255, 255);
        leds[171] = CHSV(hue1, 255, 255);
        leds[151] = CHSV(hue1, 255, 255);
        leds[13] = CHSV(hue1, 255, 255);
        leds[12] = CHSV(hue1, 255, 255);
        leds[120] = CHSV(hue1, 255, 255);
        leds[100] = CHSV(hue1, 255, 255);
        leds[106] = CHSV(hue1, 255, 255);
        break;

      case 22:
        leds[164] = CHSV(hue1, 255, 255);
        leds[14] = CHSV(hue1, 255, 255);
        leds[11] = CHSV(hue1, 255, 255);
        leds[107] = CHSV(hue1, 255, 255);
        break;

      case 23:
        leds[152] = CHSV(hue1, 255, 255);
        leds[149] = CHSV(hue1, 255, 255);
        leds[147] = CHSV(hue1, 255, 255);
        leds[124] = CHSV(hue1, 255, 255);
        leds[122] = CHSV(hue1, 255, 255);
        leds[119] = CHSV(hue1, 255, 255);
        break;
  
      case 24:
        leds[163] = CHSV(hue1, 255, 255);
        leds[159] = CHSV(hue1, 255, 255);
        leds[148] = CHSV(hue1, 255, 255);
        leds[123] = CHSV(hue1, 255, 255);
        leds[112] = CHSV(hue1, 255, 255);
        leds[108] = CHSV(hue1, 255, 255);
        break;
      
      case 25:
        leds[162] = CHSV(hue1, 255, 255);
        leds[160] = CHSV(hue1, 255, 255);
        leds[153] = CHSV(hue1, 255, 255);
        leds[146] = CHSV(hue1, 255, 255);
        leds[125] = CHSV(hue1, 255, 255);
        leds[118] = CHSV(hue1, 255, 255);
        leds[111] = CHSV(hue1, 255, 255);
        leds[109] = CHSV(hue1, 255, 255);
        break;
        
      case 26:
        leds[161] = CHSV(hue1, 255, 255);
        leds[158] = CHSV(hue1, 255, 255);
        leds[113] = CHSV(hue1, 255, 255);
        leds[110] = CHSV(hue1, 255, 255);
        break;

      case 27:
        leds[157] = CHSV(hue1, 255, 255);
        leds[154] = CHSV(hue1, 255, 255);
        leds[145] = CHSV(hue1, 255, 255);
        leds[126] = CHSV(hue1, 255, 255);
        leds[117] = CHSV(hue1, 255, 255);
        leds[114] = CHSV(hue1, 255, 255);
        break;
     
      case 28:
        leds[144] = CHSV(hue1, 255, 255);
        leds[127] = CHSV(hue1, 255, 255);
        break;
      
      case 29:
        leds[156] = CHSV(hue1, 255, 255);
        leds[155] = CHSV(hue1, 255, 255);
        leds[138] = CHSV(hue1, 255, 255);
        leds[133] = CHSV(hue1, 255, 255);
        leds[116] = CHSV(hue1, 255, 255);
        leds[115] = CHSV(hue1, 255, 255);
        break;
  
      case 30:
        leds[143] = CHSV(hue1, 255, 255);
        leds[139] = CHSV(hue1, 255, 255);
        leds[132] = CHSV(hue1, 255, 255);
        leds[128] = CHSV(hue1, 255, 255);
        break;
  
      case 31:
        leds[142] = CHSV(hue1, 255, 255);
        leds[140] = CHSV(hue1, 255, 255);
        leds[137] = CHSV(hue1, 255, 255);
        leds[134] = CHSV(hue1, 255, 255);
        leds[131] = CHSV(hue1, 255, 255);
        leds[129] = CHSV(hue1, 255, 255);
        break;

      case 32:
        leds[141] = CHSV(hue1, 255, 255);
        leds[130] = CHSV(hue1, 255, 255);
        break;

      case 33:
        leds[136] = CHSV(hue1, 255, 255);
        leds[135] = CHSV(hue1, 255, 255);
        break;

  
      default:
        // if nothing else matches, do the default
        // HOLD it solid for a few cycles... maybe later we can add some animation.  
        // Now we'll hold it solid for cycles 11-20
        break;
      }  
  
      FastLED.show();
      delay (20);
    }
    // end display loop 
  }
  // end loop for number of displays loop
}



void wipe_top_to_bottom() {

  const uint8_t NUM_PHASES = 40;


  // pick a random color
  uint8_t hue1 = random8(255);

  // clear the slate
  FastLED.clear();

  for (int j=0; j<8; j++) {
      
    for (int i=1; i<NUM_PHASES+1; i++) {

      // always update the color during each iteration
      //hue1+=3;  // significant color change start to end
      hue1+=1;

      switch (i) {
      case 0:
        // turn all LEDs off
        //FastLED.clear();
        break;
  
      case 1:
        leds[26] = CHSV(hue1, 255, 255);
        leds[167] = CHSV(hue1, 255, 255);
        break;
  
      case 2:
        leds[31] = CHSV(hue1, 255, 255);
        leds[25] = CHSV(hue1, 255, 255);
        leds[168] = CHSV(hue1, 255, 255);
        leds[162] = CHSV(hue1, 255, 255);
        break;

      case 3:
        leds[32] = CHSV(hue1, 255, 255);
        leds[27] = CHSV(hue1, 255, 255);
        leds[166] = CHSV(hue1, 255, 255);
        leds[161] = CHSV(hue1, 255, 255);
        break;
        
      case 4:
        leds[33] = CHSV(hue1, 255, 255);
        leds[30] = CHSV(hue1, 255, 255);
        leds[28] = CHSV(hue1, 255, 255);
        leds[24] = CHSV(hue1, 255, 255);
        leds[169] = CHSV(hue1, 255, 255);
        leds[165] = CHSV(hue1, 255, 255);
        leds[163] = CHSV(hue1, 255, 255);
        leds[160] = CHSV(hue1, 255, 255);
        break;

      case 5:
        leds[29] = CHSV(hue1, 255, 255);
        leds[23] = CHSV(hue1, 255, 255);
        leds[170] = CHSV(hue1, 255, 255);
        leds[164] = CHSV(hue1, 255, 255);
        break;

      case 6:
        leds[37] = CHSV(hue1, 255, 255);
        leds[36] = CHSV(hue1, 255, 255);
        leds[35] = CHSV(hue1, 255, 255);
        leds[34] = CHSV(hue1, 255, 255);
        leds[159] = CHSV(hue1, 255, 255);
        leds[158] = CHSV(hue1, 255, 255);
        leds[157] = CHSV(hue1, 255, 255);
        leds[156] = CHSV(hue1, 255, 255);
        break;
  
      case 7:
        leds[38] = CHSV(hue1, 255, 255);
        leds[39] = CHSV(hue1, 255, 255);
        leds[40] = CHSV(hue1, 255, 255);
        leds[41] = CHSV(hue1, 255, 255);
        leds[22] = CHSV(hue1, 255, 255);
        leds[171] = CHSV(hue1, 255, 255);
        leds[152] = CHSV(hue1, 255, 255);
        leds[153] = CHSV(hue1, 255, 255);
        leds[154] = CHSV(hue1, 255, 255);
        leds[155] = CHSV(hue1, 255, 255);
        break;
  
      case 8:
        leds[51] = CHSV(hue1, 255, 255);
        leds[42] = CHSV(hue1, 255, 255);
        leds[21] = CHSV(hue1, 255, 255);
        leds[19] = CHSV(hue1, 255, 255);
        leds[18] = CHSV(hue1, 255, 255);
        leds[172] = CHSV(hue1, 255, 255);
        leds[151] = CHSV(hue1, 255, 255);
        leds[142] = CHSV(hue1, 255, 255);
        break;
 
      case 9:
        leds[52] = CHSV(hue1, 255, 255);
        leds[20] = CHSV(hue1, 255, 255);
        leds[17] = CHSV(hue1, 255, 255);
        leds[173] = CHSV(hue1, 255, 255);
        leds[141] = CHSV(hue1, 255, 255);
        break;
        
      case 10:
        leds[53] = CHSV(hue1, 255, 255);
        leds[50] = CHSV(hue1, 255, 255);
        leds[44] = CHSV(hue1, 255, 255);
        leds[43] = CHSV(hue1, 255, 255);
        leds[150] = CHSV(hue1, 255, 255);
        leds[149] = CHSV(hue1, 255, 255);
        leds[143] = CHSV(hue1, 255, 255);
        leds[140] = CHSV(hue1, 255, 255);
        break;

      case 11:
        leds[1] = CHSV(hue1, 255, 255);
        leds[0] = CHSV(hue1, 255, 255);
        leds[16] = CHSV(hue1, 255, 255);
        leds[15] = CHSV(hue1, 255, 255);
        break;
  
      case 12:
        leds[54] = CHSV(hue1, 255, 255);
        leds[49] = CHSV(hue1, 255, 255);
        leds[45] = CHSV(hue1, 255, 255);
        leds[2] = CHSV(hue1, 255, 255);
        leds[14] = CHSV(hue1, 255, 255);
        leds[148] = CHSV(hue1, 255, 255);
        leds[144] = CHSV(hue1, 255, 255);
        leds[139] = CHSV(hue1, 255, 255);
        break;

// TOp Center Row
      case 13:
        leds[57] = CHSV(hue1, 255, 255);
        leds[56] = CHSV(hue1, 255, 255);
        leds[55] = CHSV(hue1, 255, 255);
        leds[48] = CHSV(hue1, 255, 255);
        leds[47] = CHSV(hue1, 255, 255);
        leds[46] = CHSV(hue1, 255, 255);
        leds[3] = CHSV(hue1, 255, 255);
        leds[13] = CHSV(hue1, 255, 255);
        leds[147] = CHSV(hue1, 255, 255);
        leds[146] = CHSV(hue1, 255, 255);
        leds[145] = CHSV(hue1, 255, 255);
        leds[138] = CHSV(hue1, 255, 255);
        leds[137] = CHSV(hue1, 255, 255);
        leds[136] = CHSV(hue1, 255, 255);

        break;

// Bottom Center Row 
      case 14:
        leds[58] = CHSV(hue1, 255, 255);
        leds[59] = CHSV(hue1, 255, 255);
        leds[60] = CHSV(hue1, 255, 255);
        leds[67] = CHSV(hue1, 255, 255);
        leds[68] = CHSV(hue1, 255, 255);
        leds[69] = CHSV(hue1, 255, 255);
        leds[4] = CHSV(hue1, 255, 255);
        leds[12] = CHSV(hue1, 255, 255);
        leds[124] = CHSV(hue1, 255, 255);
        leds[125] = CHSV(hue1, 255, 255);
        leds[126] = CHSV(hue1, 255, 255);
        leds[133] = CHSV(hue1, 255, 255);
        leds[134] = CHSV(hue1, 255, 255);
        leds[135] = CHSV(hue1, 255, 255);
        break;

      case 15:
        leds[61] = CHSV(hue1, 255, 255);
        leds[66] = CHSV(hue1, 255, 255);
        leds[70] = CHSV(hue1, 255, 255);
        leds[5] = CHSV(hue1, 255, 255);
        leds[11] = CHSV(hue1, 255, 255);
        leds[123] = CHSV(hue1, 255, 255);
        leds[127] = CHSV(hue1, 255, 255);
        leds[132] = CHSV(hue1, 255, 255);
        break;
  
      case 16:
        leds[6] = CHSV(hue1, 255, 255);
        leds[7] = CHSV(hue1, 255, 255);
        leds[9] = CHSV(hue1, 255, 255);
        leds[10] = CHSV(hue1, 255, 255);
        break;

      case 17:
        leds[62] = CHSV(hue1, 255, 255);
        leds[65] = CHSV(hue1, 255, 255);
        leds[71] = CHSV(hue1, 255, 255);
        leds[72] = CHSV(hue1, 255, 255);
        leds[121] = CHSV(hue1, 255, 255);
        leds[122] = CHSV(hue1, 255, 255);
        leds[128] = CHSV(hue1, 255, 255);
        leds[131] = CHSV(hue1, 255, 255);
        break;

      case 18:
        leds[63] = CHSV(hue1, 255, 255);
        leds[95] = CHSV(hue1, 255, 255);
        leds[8] = CHSV(hue1, 255, 255);
        leds[98] = CHSV(hue1, 255, 255);
        leds[130] = CHSV(hue1, 255, 255);
        break;

      case 19:
        leds[64] = CHSV(hue1, 255, 255);
        leds[73] = CHSV(hue1, 255, 255);
        leds[94] = CHSV(hue1, 255, 255);
        leds[96] = CHSV(hue1, 255, 255);
        leds[97] = CHSV(hue1, 255, 255);
        leds[99] = CHSV(hue1, 255, 255);
        leds[120] = CHSV(hue1, 255, 255);
        leds[129] = CHSV(hue1, 255, 255);
        break;

      case 20:
        leds[77] = CHSV(hue1, 255, 255);
        leds[76] = CHSV(hue1, 255, 255);
        leds[75] = CHSV(hue1, 255, 255);
        leds[74] = CHSV(hue1, 255, 255);
        leds[93] = CHSV(hue1, 255, 255);
        leds[100] = CHSV(hue1, 255, 255);
        leds[119] = CHSV(hue1, 255, 255);
        leds[118] = CHSV(hue1, 255, 255);
        leds[117] = CHSV(hue1, 255, 255);
        leds[116] = CHSV(hue1, 255, 255);
        break;

      case 21:
        leds[78] = CHSV(hue1, 255, 255);
        leds[79] = CHSV(hue1, 255, 255);
        leds[80] = CHSV(hue1, 255, 255);
        leds[81] = CHSV(hue1, 255, 255);
        leds[112] = CHSV(hue1, 255, 255);
        leds[113] = CHSV(hue1, 255, 255);
        leds[114] = CHSV(hue1, 255, 255);
        leds[115] = CHSV(hue1, 255, 255);
        break;
      
      case 22:
        leds[86] = CHSV(hue1, 255, 255);
        leds[92] = CHSV(hue1, 255, 255);
        leds[101] = CHSV(hue1, 255, 255);
        leds[107] = CHSV(hue1, 255, 255);
        break;
        
      case 23:
        leds[82] = CHSV(hue1, 255, 255);
        leds[85] = CHSV(hue1, 255, 255);
        leds[87] = CHSV(hue1, 255, 255);
        leds[91] = CHSV(hue1, 255, 255);
        leds[102] = CHSV(hue1, 255, 255);
        leds[106] = CHSV(hue1, 255, 255);
        leds[108] = CHSV(hue1, 255, 255);
        leds[111] = CHSV(hue1, 255, 255);
        break;

      case 24:
        leds[83] = CHSV(hue1, 255, 255);
        leds[88] = CHSV(hue1, 255, 255);
        leds[105] = CHSV(hue1, 255, 255);
        leds[110] = CHSV(hue1, 255, 255);
        break;
     
      case 25:
        leds[84] = CHSV(hue1, 255, 255);
        leds[90] = CHSV(hue1, 255, 255);
        leds[103] = CHSV(hue1, 255, 255);
        leds[109] = CHSV(hue1, 255, 255);
        break;
      
      case 26:
        leds[89] = CHSV(hue1, 255, 255);
        leds[104] = CHSV(hue1, 255, 255);
        break;
    
      default:
        // if nothing else matches, do the default
        // HOLD it solid for a few cycles... maybe later we can add some animation.  
        // Now we'll hold it solid for cycles 11-20
        break;
      }  
  
      FastLED.show();
      //delay (20);
      delay (30);
      
      //fadeToBlackBy( leds, NUM_LEDS, 25);
      fadeToBlackBy( leds, NUM_LEDS, 50);
    }
    // end display loop 
  }
  // end loop for number of displays loop
}



void branch_fade()
{
  uint8_t branch_id = 1;
  uint8_t TOTAL_CYCLES = 60;

  uint8_t offset_br1;
  uint8_t offset_br2;
  uint8_t offset_br3;
  uint8_t offset_br4;
  uint8_t offset_br5;
  uint8_t offset_br6;

  // random color
  uint8_t hue1 = random8(255);

  for (int j=0; j<5; j++) {
    for (int current_cycle=0; current_cycle < TOTAL_CYCLES; current_cycle++) {
  
      int branch_id;
      branch_id = current_cycle % 10;

      // I changed the order to 50, 40, 30, .. 0 so that this goes in reverse.  All other
      // effects go clockwise.  I wanted something that went the other direction!
      if (current_cycle == 50) {
        for (int i=0; i<26; i++) {
          uint8_t offset_br1 = pgm_read_byte(&flakeBranch1Array[i]);
          leds[offset_br1] = CHSV(hue1, 255, 255);    
        }
      }
  
      if (current_cycle == 40) {
        for (int i=0; i<26; i++) {
          uint8_t offset_br2 = pgm_read_byte(&flakeBranch2Array[i]);
          leds[offset_br2] = CHSV(hue1, 255, 255);    
        }
      }
  
      if (current_cycle == 30) {
        for (int i=0; i<26; i++) {
          uint8_t offset_br3 = pgm_read_byte(&flakeBranch3Array[i]);
          leds[offset_br3] = CHSV(hue1, 255, 255);    
        }
      }
  
      if (current_cycle == 20) {
        for (int i=0; i<26; i++) {
          uint8_t offset_br4 = pgm_read_byte(&flakeBranch4Array[i]);
          leds[offset_br4] = CHSV(hue1, 255, 255);    
        }
      }
      
      if (current_cycle == 10) {
        for (int i=0; i<26; i++) {
          uint8_t offset_br5 = pgm_read_byte(&flakeBranch5Array[i]);
          leds[offset_br5] = CHSV(hue1, 255, 255);    
        }
      }
  
      if (current_cycle == 0) {
        for (int i=0; i<26; i++) {
          uint8_t offset_br6 = pgm_read_byte(&flakeBranch6Array[i]);
          leds[offset_br6] = CHSV(hue1, 255, 255);    
        }
      }
  
      FastLED.show();
      delay(30);
      fadeToBlackBy( leds, NUM_LEDS, 25);
    }
    // end of display control
  }
  // end of total cycles loop

}



// turn on all LEDs are differnt colors - flash them to get attention, then fade out
void fulldisplay_w_flash() 
{

  // start with 3 different colors - if more or less required, adjust the if statement below
  // 300 ms on, 100 ms off seems pretty striking
  for (int color_id=0; color_id<3; color_id++) {

    for (int i=0; i<6; i++) {
      FastLED.clear();
      FastLED.show();
      delay (150);    

      // pick a color based on color ID
      //if (color_id == 0) { fill_solid( leds, NUM_LEDS, CRGB::White ); }
      if (color_id == 0) { fill_solid( leds, NUM_LEDS, CRGB::Yellow ); }
      if (color_id == 1) { fill_solid( leds, NUM_LEDS, CRGB::Red );   }
      if (color_id == 2) { fill_solid( leds, NUM_LEDS, CRGB::Green);  }
      
      FastLED.show();
      delay (300);    
    }
  
    // all LEDs are now on, fade to black
    for (int j=0; j<20; j++) {
      fadeToBlackBy( leds, NUM_LEDS, 75);
      FastLED.show();
      delay(35);
    }
    
  }
  // end of color loops
}

*/   


void fill_solid_color() 
{ 
  
  uint8_t hue1 = random8(255);
  uint8_t hue2 = random8(255);
   
  for (int i=0; i<NUM_LEDS; i++) {
    Serial.println (i);

    leds[i] = CHSV( hue1, 255, 255);
    leds[i-1] = CHSV( hue2, 255, 255);
    FastLED.show();
    leds[i] = CRGB::Black;

    //delay(30);    
    delay(20);    
  }
}

void fill_black()
{
  FastLED.clear();
  //fill_solid(leds, NUM_LEDS, CRGB::Black);
}


void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}


void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}


void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


// draws a line that fades between 2 random colors
// TODO:  Add logic to rotate the starting point
void gradient_fill() {

//  uint8_t hue1 = 60;
//  uint8_t hue2 = random8(255);
  uint8_t hue1 = random8(255);
  uint8_t hue2 = hue1 + random8(30,61);
  
  for( int i = 0; i < NUM_LEDS; i++){
    //fill_gradient (leds, 0, CHSV(0, 255, 255), i, CHSV(96, 255, 255), SHORTEST_HUES);
    fill_gradient (leds, 0, CHSV(hue1, 255, 255), i, CHSV(hue2, 255, 255), SHORTEST_HUES);
    delay(25);
    FastLED.show();
    //FastLED.clear();
  }
}

// draws a line that fades between 2 random colors
// Added logic to rotate the starting corner
/*
void gradient_fill_2() {

  uint8_t hue1 = random8(255);
  uint8_t hue2 = hue1 + random8(30,61);
  uint8_t offset2;
  CRGB grad[NUM_LEDS];  // A place to save the gradient colors. (Don't edit this)
  fill_gradient (grad, 0, CHSV(hue1, 255, 255), NUM_LEDS, CHSV(hue2, 255, 255), SHORTEST_HUES);

  // display each pixel one at a time
  for( int i = 0; i < NUM_LEDS; i++){

    if ( (i+offset_for_star_rotation) < NUM_LEDS ) {
      offset2 = i+offset_for_star_rotation;
    }
    else {
      offset2 = i+offset_for_star_rotation-NUM_LEDS;
    }
    
    leds[offset2] = grad[i];
    //delay(25);
    delay(12);
    FastLED.show();
  }
  
  // rotate star starting point for next loop
  offset_for_star_rotation += 20;
  if (offset_for_star_rotation >= 100) {
    offset_for_star_rotation = 0;
  }
  FastLED.clear();
}
*/



// Adapted from code by Marc Miller.  Original Header:
//
//***************************************************************
// Marquee fun (v3)
//  Pixel position down the strip comes from this formula:
//      pos = spacing * (i-1) + spacing
//  i starts at 0 and is incremented by +1 up to NUM_LEDS/spacing.
//
// Marc Miller, May 2016
//***************************************************************
/*
void marque_v3() {

    
  for (uint8_t i=0; i<(NUM_LEDS/spacing); i++){
    for (uint8_t w = 0; w<width; w++){
      pos = (spacing * (i-1) + spacing + advance + w) % NUM_LEDS;
      if ( w % 2== 0 ){  // Is w even or odd?
        color = hue;
      } else {
        color = hue + hue2_shift;
      }
      
      leds[pos] = CHSV(color,255,255);
    }
    
    if (DEBUG==1) {  // Print out lit pixels if DEBUG is true.
      Serial.print(" "); Serial.print(pos);
    }
    delay(10);
  }
  if (DEBUG==1) { Serial.println(" "); }
  FastLED.show();
  
  // Fade out tail or set back to black for next loop around.
  if (fadingTail == 1) {
    fadeToBlackBy(leds, NUM_LEDS,fadeRate);
  } else {
    for (uint8_t i=0; i<(NUM_LEDS/spacing); i++){
      for (uint8_t w = 0; w<width; w++){
        pos = (spacing * (i-1) + spacing + advance + w) % NUM_LEDS;
        leds[pos] = CRGB::Black;
      }
    }
  }

  // Advance pixel postion down strip, and rollover if needed.
  advance = (advance + delta + NUM_LEDS) % NUM_LEDS;
}
*/


void TestGlitter() 
{
  FastLED.clear();
  addGlitter(95);
  delay(25);
}

  
// add several random white spots/stars/glitters
void addGlitter( fract8 chanceOfGlitter) 
{
  for (int i=0; i<20; i++) {
    if( random8() < chanceOfGlitter) {
      leds[ random16(NUM_LEDS) ] += CRGB::White;  
    }
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}


// END
