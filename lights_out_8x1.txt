/*
 * Lights Out Game - 8x1 Version
 * Board: LGT8F328P / Arduino nano
 * 8 NeoPixels on D11
 * 8 Buttons on D2-D9 (pull-up)
 * 
 * Library: FastLED (install from Library Manager)
 */

#include <FastLED.h>

// Pin definitions
#define LED_PIN 11
#define NUM_LEDS 8
#define BUTTON_START 2  // Buttons on D2-D9

// FastLED setup
CRGB leds[NUM_LEDS];

// Game state
bool ledStates[NUM_LEDS];
bool buttonPressed[NUM_LEDS];

// Colors
CRGB colorOff = CRGB::Black;
CRGB colorOn = CRGB::Red;

// ====================================
// USER CONFIGURATION
// ====================================
// Set the initial puzzle pattern here
// Use 1 for LED ON (red), 0 for LED OFF (black)
// Example: {1,0,1,0,1,0,1,0} means alternating on/off pattern
bool useCustomPattern = false;  // Set to true to use your custom pattern, false for random
bool customPattern[NUM_LEDS] = {1, 1, 0, 0, 1, 1, 0, 0};
// ====================================

void setup() {
  Serial.begin(9600);
  
  // Initialize FastLED
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
  FastLED.clear();
  FastLED.show();
  
  // Initialize button pins
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(BUTTON_START + i, INPUT_PULLUP);
    buttonPressed[i] = false;
  }
  
  // Initialize random seed
  randomSeed(analogRead(A0));
  
  Serial.println("Lights Out Game Started!");
  
  // Start new game
  newGame();
}

void loop() {
  // Check each button with simple press detection
  for (int i = 0; i < NUM_LEDS; i++) {
    bool currentState = (digitalRead(BUTTON_START + i) == LOW);
    
    // Detect button press (transition from not pressed to pressed)
    if (currentState && !buttonPressed[i]) {
      // Button just pressed
      buttonPressed[i] = true;
      handleButtonPress(i);
      updateDisplay();
      
      Serial.print("Button ");
      Serial.print(i);
      Serial.println(" pressed!");
      
      delay(50); // Simple debounce
    } 
    else if (!currentState && buttonPressed[i]) {
      // Button released
      buttonPressed[i] = false;
    }
  }
  
  // Check win condition
  if (checkWin()) {
    celebrateWin();
    delay(2000);
    newGame();
  }
  
  delay(10); // Small delay to stabilize readings
}

void handleButtonPress(int button) {
  Serial.print("Toggling LED ");
  Serial.println(button);
  
  // Toggle the pressed LED
  ledStates[button] = !ledStates[button];
  
  // Toggle left neighbor
  if (button > 0) {
    ledStates[button - 1] = !ledStates[button - 1];
    Serial.print("  Toggling left neighbor ");
    Serial.println(button - 1);
  }
  
  // Toggle right neighbor
  if (button < NUM_LEDS - 1) {
    ledStates[button + 1] = !ledStates[button + 1];
    Serial.print("  Toggling right neighbor ");
    Serial.println(button + 1);
  }
}

void updateDisplay() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (ledStates[i]) {
      leds[i] = colorOn;
    } else {
      leds[i] = colorOff;
    }
  }
  FastLED.show();
}

bool checkWin() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (ledStates[i]) {
      return false;
    }
  }
  return true;
}

void celebrateWin() {
  Serial.println("YOU WIN!");
  
  // Flash green to celebrate
  for (int flash = 0; flash < 3; flash++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Green;
    }
    FastLED.show();
    delay(300);
    FastLED.clear();
    FastLED.show();
    delay(300);
  }
}

void newGame() {
  // Clear all LEDs first
  for (int i = 0; i < NUM_LEDS; i++) {
    ledStates[i] = false;
  }
  
  Serial.println("\n--- New Game ---");
  
  if (useCustomPattern) {
    // Use the custom pattern defined by user
    Serial.println("Using CUSTOM pattern");
    for (int i = 0; i < NUM_LEDS; i++) {
      ledStates[i] = customPattern[i];
    }
  } else {
    // Create a random solvable puzzle by simulating random button presses
    Serial.println("Using RANDOM pattern");
    int numPresses = random(5, 15);
    Serial.print("Generating puzzle with ");
    Serial.print(numPresses);
    Serial.println(" moves...");
    
    for (int i = 0; i < numPresses; i++) {
      int button = random(NUM_LEDS);
      
      ledStates[button] = !ledStates[button];
      if (button > 0) {
        ledStates[button - 1] = !ledStates[button - 1];
      }
      if (button < NUM_LEDS - 1) {
        ledStates[button + 1] = !ledStates[button + 1];
      }
    }
  }
  
  // Count how many LEDs are on
  int onCount = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    if (ledStates[i]) onCount++;
  }
  Serial.print("Puzzle created with ");
  Serial.print(onCount);
  Serial.println(" LEDs on");
  
  updateDisplay();
}