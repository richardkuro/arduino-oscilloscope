#include <Wire.h>
#include <U8g2lib.h>
#include <stdio.h> // Required for sprintf

// --- U8g2 Display Configuration (Memory Efficient) ---
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// --- Screen Dimensions (FIXED) ---
// These constants were missing, causing the compilation error.
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// --- Oscilloscope Settings ---
const int SAMPLES = 128; // Number of samples to capture, matches screen width
int readings[SAMPLES];   // Buffer to store the captured waveform data
const int inputPin = A0; // Analog pin to read the signal from

// --- Rotary Encoder Settings ---
#define ENCODER_CLK 2 // Must be an interrupt pin (D2 or D3 on Pro Mini)
#define ENCODER_DT 3
#define ENCODER_SW 4 // Switch pin for Single Shot mode

// --- Timebase and Single Shot Variables ---
const int timebaseOptions[] = {0, 10, 25, 50, 100, 250, 500, 1000, 2500}; 
const int numTimebases = sizeof(timebaseOptions) / sizeof(timebaseOptions[0]);
volatile int timebaseIndex = 3; // Start with a default timebase
bool isFrozen = false;  // Flag for Single Shot mode

// This function is called by the hardware interrupt when the encoder is turned
void updateEncoder() {
  // Read the DT pin to determine direction
  if (digitalRead(ENCODER_DT) != digitalRead(ENCODER_CLK)) {
    // Clockwise (zoom in)
    if (timebaseIndex > 0) timebaseIndex--;
  } else {
    // Counter-clockwise (zoom out)
    if (timebaseIndex < numTimebases - 1) timebaseIndex++;
  }
}

void setup() {
  Wire.begin();
  Wire.setClock(400000); 
  u8g2.begin();

  // --- Setup Encoder Pins ---
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  
  // Attach interrupt ONLY for the encoder rotation for maximum reliability
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), updateEncoder, FALLING);

  // --- Optimize the ADC for speed ---
  ADCSRA &= ~((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0)); // Clear prescaler bits
  ADCSRA |= (1 << ADPS2);                                 // Set prescaler to 16

  // --- Display Boot Screen ---
  u8g2.firstPage();
  do {
    // Draw "Oscilloscope" in a large font
    u8g2.setFont(u8g2_font_logisoso16_tr);
    int titleWidth = u8g2.getStrWidth("Oscilloscope");
    u8g2.drawStr((SCREEN_WIDTH - titleWidth) / 2, 28, "Oscilloscope");

    // Draw "by RichardKonsam" in a smaller font
    u8g2.setFont(u8g2_font_ncenB08_tr);
    int nameWidth = u8g2.getStrWidth("by RichardKonsam");
    u8g2.drawStr((SCREEN_WIDTH - nameWidth) / 2, 48, "by RichardKonsam");
  } while (u8g2.nextPage());
  
  delay(2000); // Show boot screen for 2 seconds
}

void loop() {
  // --- Handle Button Press for Single Shot (More Robust Method) ---
  if (digitalRead(ENCODER_SW) == LOW) {
    delay(50); // Simple debounce
    if (digitalRead(ENCODER_SW) == LOW) {
      isFrozen = !isFrozen;
      while(digitalRead(ENCODER_SW) == LOW); // Wait for button release
    }
  }

  // Only capture new data if we are not in "frozen" mode
  if (!isFrozen) {
    // --- 1. Capture the waveform with the selected timebase delay ---
    int currentDelay = timebaseOptions[timebaseIndex];
    for (int i = 0; i < SAMPLES; i++) {
      readings[i] = analogRead(inputPin);
      if (currentDelay > 0) {
        delayMicroseconds(currentDelay);
      }
    }
  }

  // --- 2. Find Min/Max for Auto-Scaling ---
  int minVal = 1023;
  int maxVal = 0;
  for (int i = 0; i < SAMPLES; i++) {
    if (readings[i] < minVal) minVal = readings[i];
    if (readings[i] > maxVal) maxVal = readings[i];
  }
  if (maxVal < 1023) maxVal += 5;
  if (minVal > 0) minVal -= 5;
  if (maxVal == minVal) maxVal = minVal + 1;

  // --- 3. Draw Everything to the Screen ---
  u8g2.firstPage();
  do {
    // Draw grid
    u8g2.setDrawColor(1);
    for(int i = 1; i < 4; i++) {
      u8g2.drawHLine(0, (u8g2.getDisplayHeight() / 4) * i, u8g2.getDisplayWidth());
      u8g2.drawVLine((u8g2.getDisplayWidth() / 4) * i, 0, u8g2.getDisplayHeight());
    }

    // Draw waveform
    for (int i = 0; i < SAMPLES - 1; i++) {
      int y1 = map(readings[i], minVal, maxVal, u8g2.getDisplayHeight() - 1, 0);
      int y2 = map(readings[i + 1], minVal, maxVal, u8g2.getDisplayHeight() - 1, 0);
      u8g2.drawLine(i, y1, i + 1, y2);
    }
    
    // --- Draw Timebase and Frozen Indicator ---
    u8g2.setFont(u8g2_font_4x6_tr);
    
    // Display Timebase in the bottom right with a background
    char buffer[20];
    int timePerDiv = 32 * timebaseOptions[timebaseIndex];
    sprintf(buffer, "%d us/div", timePerDiv);
    
    int textWidth = u8g2.getStrWidth(buffer);
    int textX = u8g2.getDisplayWidth() - textWidth - 2;
    int textY = u8g2.getDisplayHeight() - 7;
    
    // Draw a black box behind the text to make it readable
    u8g2.setDrawColor(0); // Black for the box
    u8g2.drawBox(textX, textY, textWidth + 1, 7);
    
    u8g2.setDrawColor(1); // White for the text
    u8g2.drawStr(textX, u8g2.getDisplayHeight() - 2, buffer);

    if (isFrozen) {
      u8g2.drawStr(2, 6, "FROZEN");
    }

  } while (u8g2.nextPage());
}
