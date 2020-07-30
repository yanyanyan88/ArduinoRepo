/*  Materials Used
 *   1 Shift Register
 *   1 8x8 LED Matrix
 *   1 Potentiometer
 *   
 *  Pins
 *   SER/DS/Serial Data Pin = D10;
 *   SCK/SH_CP/Shift Register Pin = D11;
 *   RCK/ST_CP/Store Register Pin = D12;
 *  
 *   Column Pins = D2 - D9;
 *  
 *   Potentiometer = A0;
 */

// Shift register pins
const int serPin{ 10 };
const int sckPin{ 11 };
const int rckPin{ 12 };

// Pins for each column
const int colPin[8] = { 2, 3, 4, 5, 6, 7, 8, 9 };

// Potentiometer pin
const int controlPin{ A0 };

// Bytes for each individual row
const byte row[8] = {
  B10000000,
  B01000000,
  B00100000,
  B00010000,
  B00001000,
  B00000100,
  B00000010,
  B00000001
};

// Different states of a bar
const int barState[5][8] = {
  { 1, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 1, 1, 1, 1, 0, 0, 0 },
  { 0, 0, 1, 1, 1, 1, 0, 0 },
  { 0, 0, 0, 1, 1, 1, 1, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 1 }
};

// Position of the ball
int ballRow{ 6 };         // Row of the ball, starts at row 6
int ballCol{ 7 };         // Column of the ball, starts at column 7

// Direction of the ball to go to
int rowDir{ -1 };         // Direction for the row
int colDir{ -1 };         // Direction for the column

// Store the given byte in the store register
void store(byte n);

// Setup for pins
void setup() {
  // Set each shift register pin to output
  pinMode(serPin, OUTPUT);
  pinMode(sckPin, OUTPUT);
  pinMode(rckPin, OUTPUT);

  // Set each column pin to output
  for (int i{ 0 }; i < 8; ++i) {
    pinMode(colPin[i], OUTPUT);
  }

  // Seed used for the random column that the ball will reset to
  randomSeed(analogRead(A1));
}

// Main code
void loop() {  
  // Read an analog value from the control pin and map it into a bar state, use it for the player's bar
  int playerBar{ (int)map(analogRead(controlPin), 0, 1000, 0, 4) };
  delay(1);

  // Turn off all rows that are currently on
  store(0);
  
  // Turn on the columns for the player's bar based on its current state
  for (int i{ 0 }; i < 8; ++i) {
    digitalWrite(colPin[i], !(barState[playerBar][i]));
  }
  delay(1);

  // Turn on the row of the player's bar
  store(row[0]);

  // Current state of the opponent's bar
  static int botBar{ 4 };

  // Destination state of the opponent's bar, the state which the opponent has to go to
  static int destState{ 4 };

  // Time of the last move of the opponent
  static unsigned long prevBotMove{ 0 };

  // Checking which column the ball is in, and changing the destination state of the opponent's bar based on that
  switch (ballCol) {
    case 0:
    case 1:
      destState = 0;
      break;
    case 2:
      destState = 1;
      break;
    case 3:
    case 4:
      destState = 2;
      break;
    case 5:
      destState = 3;
      break;
    case 6:
    case 7:
      destState = 4;
  }

  // If 100 ms has passed since the last move of the opponent, move him closer to the destination state
  if (millis() - prevBotMove > 100) {
    if (botBar < destState) {
      ++botBar;
    }
    else if (botBar > destState) {
      --botBar;
    }

    // Mark the time of the last move of the opponent
    prevBotMove = millis();
  }
  delay(1);

  // Turn off all rows that are currently on
  store(0);

  // Turn on the columns for the opponent's bar based on its bar state
  for (int i{ 0 }; i < 8; ++i) {
    digitalWrite(colPin[i], !barState[botBar][i]);
  }
  delay(1);

  // Turn on the row of the opponent's bar
  store(row[7]);

  // Keep track of the reset validation for the ball
  static bool reset{ false };

  // Keep track of when the ball last moved
  static unsigned long prevBallMove{ 0 };

  // If it has been 125 ms since the last movement of the ball, move it again
  if (millis() - prevBallMove > 125) {
    // If a reset has been made, if so, reset the ball with a random column that's within bounds
    if (reset) {
      ballRow = 6;
      ballCol = random(1, 7);
      rowDir = -1;
      reset = false;
    }

    // Else, do the usual routine
    else {
      // Move the ball based on its direction
      ballRow += rowDir;
      ballCol += colDir;
  
      // If the ball has reached either of the extreme columns of the matrix, reverse its column direction
      switch (ballCol) {
        case 0:
          colDir = 1;
          break;
        case 7:
          colDir = -1;
      }
  
      // If the ball is hit by the opponent, reverse its row direction
      if (ballRow == 6 && barState[botBar][ballCol]) {
        rowDir = -1;
      }
      // Else, if the opponent didn't hit the ball, reset it
      else if (ballRow == 6 && !(barState[botBar][ballCol])) {
        reset = true;
      }
      
      // Else, if the ball hits the player's bar, reverse its row direction
      else if (ballRow == 1 && barState[playerBar][ballCol]) {
        rowDir = 1;
      }
      // Else, if player didn't hit the ball, reset it
      else if (ballRow == 1 && !(barState[playerBar][ballCol])) {
        reset = true;
      }
    }

    // Mark the time of the previous movement of the ball
    prevBallMove = millis();
  }
  delay(1);

  // Turn off all rows that are currently on
  store(0);

  // Turn off all columns that are currently on
  for (int i{ 0 }; i < 8; ++i) {
    digitalWrite(colPin[i], HIGH);
  }

  // Turn on the current column of the ball
  digitalWrite(colPin[ballCol], LOW);
  delay(1);

  // Turn on the current row of the ball
  store(row[ballRow]);
}

// Store the given byte in the store register
void store(byte n) {
  // Complete the previous clock cycle of the RCK
  digitalWrite(rckPin, LOW);

  // Shift the bytes into the shift register
  shiftOut(serPin, sckPin, LSBFIRST, n);

  // Latch the bytes onto the store register and start a clock cycle
  digitalWrite(rckPin, HIGH);
}
