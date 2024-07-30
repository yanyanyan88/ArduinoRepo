/*  Materias Used
 *    1 - Shift Register
 *    1 - RGB LED
 *    1 - Button
 *    1 - Buzzer
 *    1 - I2C LCD
 *    1 - 4 7-Segment Display
 *  Pins
 *    4 Displays = D2, D3, D4, D7, Segments are in the Shift Register
 *    RGB LED = ~D9, ~D10, ~D11
 *    Button = D6
 *    Buzzer = D5
 *    SER = D8
 *    SCK = D12
 *    RCK = D13
 *    LCD = A4, A5
 */

// Note macros for the melody
#include "pitches.h"

// Libraries needed to be able to use the LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Each individual display of the 4 7-Segment Display
const int displayNum[4] = { 2, 3, 4, 7 };

// Each color pin of the RGB LED from Red to Blue
const int rgb[3] = { 9, 10, 11 };

const int button{ 6 };
const int buzzer{ 5 };

const int serPin{ 8 };
const int sckPin{ 12 };
const int rckPin{ 13 };

// The LCD, assigned to address 0x27 with 16 columns and 2 rows
LiquidCrystal_I2C lcd{ 0x27, 16, 2 };

// Greeting to print on the LCD
String greeting{ "Happy Birthday" };

// Bytes for each Decimal Digit from 0 to 9
const byte digit[10] = {
  B11111100,
  B01100000,
  B11011010,
  B11110010,
  B01100110,
  B10110110,
  B10111110,
  B11100000,
  B11111110,
  B11100110
};

// Color modes for the RGB LED based on the HSV color model
enum ColorMode_T {
  kRed,
  kYellow,
  kGreen,
  kCyan,
  kBlue,
  kPink
};

void turnOffAllDisplay();
void store(byte n);
void countdown(int& currentSeconds, bool& countdownBool, bool& beepBool);
void playMelody(bool& melodyBool, const int melody[8], const int noteDurations[8], const int melodySize);
void rainbowLED(int& redValue, int& greenValue, int& blueValue, ColorMode_T& colorMode);

void setup() {
  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(15, 0);

  // Set the display pins to Output
  for (int i{ 0 }; i < 4; ++i) {
    pinMode(displayNum[i], OUTPUT);
  }

  // Set the RGB pins to Output
  for (int i{ 0 }; i < 3; ++i) {
    pinMode(rgb[i], OUTPUT);
  }

  pinMode(button, INPUT_PULLUP);    // Set the button pin to Input w/ the Pull-Up Resistor
  pinMode(buzzer, OUTPUT);
  
  pinMode(serPin, OUTPUT);
  pinMode(sckPin, OUTPUT);
  pinMode(rckPin, OUTPUT);

  turnOffAllDisplay();
}

void loop() {
  // Sets the system to Greet Mode or Not
  static bool greetMode{ false };

  // Properties of the button
  static unsigned long prevButtonStateChange{ 0 };    // Time of the last change of the button state
  static int prevButtonReading{ HIGH };               // The previous reading of the button's state
  static int currentButtonState{ HIGH };              // The current button state

  int reading{ digitalRead(button) };       // Reading of the button's state

  // If the reading now is different from the previous reading, mark the time of the change
  if (reading != prevButtonReading) {
    prevButtonStateChange = millis();
  }

  // Check if it has been 50 ms since the last button state change
  if (millis() - prevButtonStateChange > 50) {
    // If it has been, check if the reading is different from the current button state, then change it
    if (reading != currentButtonState) {
      currentButtonState = reading;

      // Check if the current button state means that the button is pressed, then update the Greet Mode validator
      if (!currentButtonState) {
        greetMode = !greetMode;
      }
    }
  }

  // Set the previous button reading to the reading now
  prevButtonReading = reading;

  // Used in the countdown
  static int currentSeconds{ 6 };
  static bool countdownBool{ true };      // Used to see if the system is still counting down
  static bool beepBool{ false };          // Used to see if the buzzer is currently beeping

  // Used for the melody
  const int melodySize{ 8 };                                                                                // Size of the melody
  const int melody[melodySize] = { NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4 };      // Notes in the melody
  const int noteDurations[melodySize] = { 4, 8, 8, 4, 4, 4, 4, 4 };                                         // Note durations (4 = quarter note, 8 = eighth note, etc...)
  static bool melodyBool{ true };                                                                           // Used to see if a melody has been played yet

  // For the RGB LED
  static int redValue{ 255 };               // Value of the Red LED
  static int greenValue{ 0 };               // Value of the Green LED
  static int blueValue{ 0 };                // Value of the Blue LED
  static ColorMode_T colorMode{ kRed };     // Current color mode of the LED

  // For the LCD
  static int index{ 0 };                                // Index of the character to print
  static unsigned long prevPrint{ 0 };                  // Time of the previous print

  // Check if the system is currently in greet mode
  if (greetMode) {
    // If the system is currently counting down, do the countdown routine
    if (countdownBool) {
      countdown(currentSeconds, countdownBool, beepBool);
    }

    // Else, if the countdown has stopped, start the greeting routine
    else {
      // If a melody hasn't been played yet, play a melody
      if (melodyBool) {
        playMelody(melodyBool, melody, noteDurations, melodySize);
      }

      // Display the current color on the RGB LED
      rainbowLED(redValue, greenValue, blueValue, colorMode);

      // If the index has reached the maximum of the LCD, stop the greeting
      if (index == 40) {
        greetMode = false;
      }
      
      // Else, if it has been 500 ms since the last print, print a character then update to the next character and scroll the screen to the left
      else if (index < 40 && millis() - prevPrint > 500) {
        // If the index is within the greeting, print the current character
        if (index < greeting.length()) {
          lcd.print(greeting[index]);
        }

        // Scroll to the left
        lcd.scrollDisplayLeft();

        // Update the time of the last print and move to the next index
        prevPrint = millis();
        ++index;
      }

      // Delay for stability
      delay(5);
    }
  }

  // Else, if we aren't on Greet Mode, reset all values
  else {
    // Reset all components
    turnOffAllDisplay();
    lcd.clear();
    lcd.setCursor(15, 0);
    noTone(buzzer);

    // Turn off the RGB LED
    analogWrite(rgb[0], 0);
    analogWrite(rgb[1], 0);
    analogWrite(rgb[2], 0);

    // Reset the timer properties
    currentSeconds = 6;
    countdownBool = true;
    beepBool = false;

    // Reset the melody bool
    melodyBool = true;

    // Reset the color values for the RGB LED
    redValue = 255;
    greenValue = 0;
    blueValue = 0;
    colorMode = kRed;

    // Reset the LCD print properties
    index = 0;
    prevPrint = 0;
  }
}

// Turns off all on displays
void turnOffAllDisplay() {
  for (int i{ 0 }; i < 4; ++i) {
    digitalWrite(displayNum[i], HIGH);
  }
}

// Stores the given byte in the shift register
void store(byte n) {
  digitalWrite(rckPin, LOW);
  shiftOut(serPin, sckPin, LSBFIRST, n);
  digitalWrite(rckPin, HIGH);
}

// Counts down from the given seconds
void countdown(int& currentSeconds, bool& countdownBool, bool& beepBool) {
  // Properties of the countdown
  static unsigned long prevSecondDecrease{ 0 };         // Time of the previous decrement of the seconds

  // Properties of the buzzer
  static unsigned long prevBeep{ 0 };       // Time of the previous beep of the buzzer

  // Check if it has been 500 ms since the last beep and the buzzer is currently beeping
  if ((millis() - prevBeep > 500) && beepBool) {
    noTone(buzzer);   // Turn off the buzzer
        
    // Turn off the RGB LED
    for (int i{ 0 }; i < 3; ++i) {
      analogWrite(rgb[i], 0);
    }
        
    beepBool = false;    // Update the beeping status
  }
      
  // Countdown for the tens digit of the seconds
  turnOffAllDisplay();                                      // Turn off all displays
  digitalWrite(displayNum[2], LOW);                         // Turn on the Tens Display
  store(digit[(currentSeconds / 10) % 10]);                 // Display the current tens digit of the seconds
  delay(5);                                                 // Delay for stability

  // Countdown for the ones digit of the seconds
  turnOffAllDisplay();                                      // Turn off all displays
  digitalWrite(displayNum[3], LOW);                         // Turn on the Ones Display
  store(digit[(currentSeconds / 1) % 10]);                  // Display the current ones digit of the seconds
  delay(5);                                                 // Delay for stability

  // If it has been 1000 ms since the previous decrease of seconds and there is still a second left, do the decrement routine
  if (currentSeconds && millis() - prevSecondDecrease > 1000) {
    --currentSeconds;                     // Decrease the seconds
    
    tone(buzzer, 200);                    // Beep the buzzer
    prevBeep = millis();                  // Update the time of the last beep
    beepBool = true;                      // Update the beeping status
    
    analogWrite(rgb[0], 255);             // Turn the Red LED all the way up
    analogWrite(rgb[1], 0);               // No value for Green LED
    analogWrite(rgb[2], 0);               // No value for Blue LED
    
    prevSecondDecrease = millis();        // Update the time of the previous centisecond decrease
  }

  // Else, if there is not a second left, stop the countdown and reset the countdown properties to default
  else if (!currentSeconds) {
    // Reset the countdown properties
    beepBool = false;
    currentSeconds = 6;
    prevSecondDecrease = 0;
    prevBeep = 0;

    // Stop the countdown
    countdownBool = false;
  }
}

// Plays the melody once, then sets the melody bool to false
void playMelody(bool& melodyBool, const int melody[8], const int noteDurations[8], const int melodySize) {
  // Play the melody
  for (int i{ 0 }; i < melodySize; ++i) {
    tone(buzzer, melody[i], 1000 / noteDurations[i]);     // Play current note for a duration of time
    delay((1000 / noteDurations[i]) * 1.30);              // Delay to prevent overlapping notes
    noTone(buzzer);                                       // Stop the current note
  }

  // Update the melody status since a melody has been played
  melodyBool = false;
}

// Displays the current color of the RGB LED and updates the color values based on the color mode
void rainbowLED(int& redValue, int& greenValue, int& blueValue, ColorMode_T& colorMode) {
  // Display the current color on the RGB LED
  analogWrite(rgb[0], redValue);
  analogWrite(rgb[1], greenValue);
  analogWrite(rgb[2], blueValue);

  // Update the color values based on the current color mode
  switch (colorMode) {
    case kRed:
      // If the Green LED has reached maximum value, move to the next mode
      if (greenValue == 255) {
        colorMode = kYellow;
      }

      // Else, continue increasing
      else {
        ++greenValue;
      }
      break;
    case kYellow:
      // If the Red LED has reached minimum value, move to the next mode
      if (redValue == 0) {
        colorMode = kGreen;
      }

      // Else, continue decreasing
      else {
        --redValue;
      }
      break;
    case kGreen:
      // If the Blue LED has reached maximum value, move to the next mode
      if (blueValue == 255) {
        colorMode = kCyan;
      }

      // Else, continue increasing
      else {
        ++blueValue;
      }
      break;
    case kCyan:
      // If the Green LED has reached minimum value, move to the next mode
      if (greenValue == 0) {
        colorMode = kBlue;
      }

      // Else, continue decreasing
      else {
        --greenValue;
      }
      break;
    case kBlue:
      // If the Red LED has reached maximum value, move to the next mode
      if (redValue == 255) {
        colorMode = kPink;
      }

      // Else, continue increasing
      else {
        ++redValue;
      }
      break;
    case kPink:
      // If the Blue LED has reached minimum value, move to the next mode
      if (blueValue == 0) {
        colorMode = kRed;
      }

      // Else, continue decreasing
      else {
        --blueValue;
      }
  }
}
