#include <FastLED.h>

#define LED_PIN     D8
#define NUM_LEDS    35
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// Parameters for the moving green LEDs
unsigned long startTime;
const unsigned long travelTime = 60000; // 60 seconds to travel across 35 LEDs
const int groupSize = 3; // Number of green LEDs in the group
const int groupSpacing = 1; // Space between the green LEDs in the group

// Very dark red and very dark blue color palettes
CRGBPalette16 redPalettes[] = {
    CRGBPalette16(CRGB::DarkRed, CRGB::Maroon, CRGB::Crimson, CRGB::DarkRed, 
                  CRGB::Maroon, CRGB::Crimson, CRGB::DarkRed, CRGB::Maroon, 
                  CRGB::Crimson, CRGB::DarkRed, CRGB::Maroon, CRGB::Crimson, 
                  CRGB::DarkRed, CRGB::Maroon, CRGB::Crimson, CRGB::DarkRed),
    CRGBPalette16(CRGB::Maroon, CRGB::Crimson, CRGB::DarkRed, CRGB::Maroon, 
                  CRGB::Crimson, CRGB::DarkRed, CRGB::Maroon, CRGB::Crimson, 
                  CRGB::DarkRed, CRGB::Maroon, CRGB::Crimson, CRGB::DarkRed, 
                  CRGB::Maroon, CRGB::Crimson, CRGB::DarkRed, CRGB::Maroon),
    CRGBPalette16(CRGB::Crimson, CRGB::DarkRed, CRGB::Maroon, CRGB::Crimson, 
                  CRGB::DarkRed, CRGB::Maroon, CRGB::Crimson, CRGB::DarkRed, 
                  CRGB::Maroon, CRGB::Crimson, CRGB::DarkRed, CRGB::Maroon, 
                  CRGB::Crimson, CRGB::DarkRed, CRGB::Maroon, CRGB::Crimson),
    CRGBPalette16(CRGB::DarkRed, CRGB::Crimson, CRGB::Maroon, CRGB::DarkRed, 
                  CRGB::Crimson, CRGB::Maroon, CRGB::DarkRed, CRGB::Crimson, 
                  CRGB::Maroon, CRGB::DarkRed, CRGB::Crimson, CRGB::Maroon, 
                  CRGB::DarkRed, CRGB::Crimson, CRGB::Maroon, CRGB::DarkRed)
};

CRGBPalette16 bluePalettes[] = {
    CRGBPalette16(CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkBlue, 
                  CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::Navy, 
                  CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue, 
                  CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkBlue),
    CRGBPalette16(CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::Navy, 
                  CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue, 
                  CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkBlue, 
                  CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::Navy),
    CRGBPalette16(CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue, 
                  CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkBlue, 
                  CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::Navy, 
                  CRGB::MidnightBlue, CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue),
    CRGBPalette16(CRGB::DarkBlue, CRGB::MidnightBlue, CRGB::Navy, CRGB::DarkBlue, 
                  CRGB::MidnightBlue, CRGB::Navy, CRGB::DarkBlue, CRGB::MidnightBlue, 
                  CRGB::Navy, CRGB::DarkBlue, CRGB::MidnightBlue, CRGB::Navy, 
                  CRGB::DarkBlue, CRGB::MidnightBlue, CRGB::Navy, CRGB::DarkBlue)
};

const int numRedPalettes = sizeof(redPalettes) / sizeof(CRGBPalette16);
const int numBluePalettes = sizeof(bluePalettes) / sizeof(CRGBPalette16);
const int paletteChangeInterval = 1000; // Time to change palette in milliseconds
unsigned long lastPaletteChange = 0;
unsigned long transitionStartTime = 0;
int currentRedIndex = 0;

// Function to create a dynamic palette with smooth transitions
CRGB getBlendedColor(CRGBPalette16& currentPalette, CRGBPalette16& nextPalette, uint8_t index, uint8_t blendFactor) {
    CRGB color1 = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);
    CRGB color2 = ColorFromPalette(nextPalette, index, 255, LINEARBLEND);
    return blend(color1, color2, blendFactor);
}

void applyBlendedPalette(CRGBPalette16& currentPalette, CRGBPalette16& nextPalette, uint8_t blendFactor) {
    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t colorIndex = map(i, 0, NUM_LEDS, 0, 255);
        leds[i] = getBlendedColor(currentPalette, nextPalette, colorIndex, blendFactor);
    }
}

void setup() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(255);
    startTime = millis();
    transitionStartTime = millis();
}

void loop() {
    // Get the current time in milliseconds
    unsigned long currentMillis = millis();

    // Determine the current and next palette indices for blue and red palettes
    int currentBluePaletteIndex = (currentMillis / paletteChangeInterval) % numBluePalettes;
    int nextBluePaletteIndex = (currentBluePaletteIndex + 1) % numBluePalettes;
    int currentRedPaletteIndex = (currentMillis / paletteChangeInterval) % numRedPalettes;
    int nextRedPaletteIndex = (currentRedPaletteIndex + 1) % numRedPalettes;

    // Determine the blend factor based on the time within the current interval
    uint8_t blendFactor = (currentMillis % paletteChangeInterval) * 255 / paletteChangeInterval;

    // Apply the blended blue palette to all LEDs initially
    applyBlendedPalette(bluePalettes[currentBluePaletteIndex], bluePalettes[nextBluePaletteIndex], blendFactor);

    // Calculate the position of the first green LED in the group
    unsigned long elapsedTime = currentMillis - startTime;
    int greenLedStartPos = map(elapsedTime % travelTime, 0, travelTime, 0, NUM_LEDS - 1);

    // Set the green LEDs in the group
    

    // Handle the transition from blue to red
    if (currentMillis - transitionStartTime >= 60000) { // 1 minute interval
        if (currentRedIndex < NUM_LEDS) {
            currentRedIndex++;
            transitionStartTime = currentMillis;
        } else {
            currentRedIndex = 0;
            transitionStartTime = currentMillis;
        }
    }

    // Apply the red palette based on the current red index
    for (int i = 0; i < currentRedIndex; i++) {
        uint8_t colorIndex = map(i, 0, NUM_LEDS, 0, 255);
        leds[i] = getBlendedColor(redPalettes[currentRedPaletteIndex], redPalettes[nextRedPaletteIndex], colorIndex, blendFactor);
    }
    for (int i = 0; i < groupSize; i++) {
        int pos = (greenLedStartPos + i * groupSpacing) % NUM_LEDS;
        leds[pos] = CRGB::Green;
    }

    // Update the display
    FastLED.show();

    delay(30); // Adjust the delay to control the update speed
}
