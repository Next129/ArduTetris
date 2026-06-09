#include <MD_MAX72xx.h>
#include <LCD_I2C.h>

// Pins
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 10
#define SW_PIN 2
#define buzzerPin 8

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST 0
// Objects
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
LCD_I2C lcd(0x27);

// change this to make the song slower or faster
int tempo=144; 

// change this to whichever pin you want to use
int buzzer = 8;

// notes of the moledy followed by the duration.
// a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
// !!negative numbers are used to represent dotted notes,
// so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
int melody[] = {

  //Based on the arrangement at https://www.flutetunes.com/tunes.php?id=192
  
  NOTE_E5, 4,  NOTE_B4,8,  NOTE_C5,8,  NOTE_D5,4,  NOTE_C5,8,  NOTE_B4,8,
  NOTE_A4, 4,  NOTE_A4,8,  NOTE_C5,8,  NOTE_E5,4,  NOTE_D5,8,  NOTE_C5,8,
  NOTE_B4, -4,  NOTE_C5,8,  NOTE_D5,4,  NOTE_E5,4,
  NOTE_C5, 4,  NOTE_A4,4,  NOTE_A4,4, REST,4,

  
};

int melody2[] = {
  NOTE_C5, NOTE_G4, NOTE_E4,
  NOTE_A4, NOTE_B4, NOTE_A4, NOTE_GS4, NOTE_AS4, NOTE_GS4,
  NOTE_G4, NOTE_D4, NOTE_E4
};

int durations[] = {
  4, 4, 4,
  8, 8, 8, 8, 8, 8,
  8, 8, 2
};

int notes=sizeof(melody)/sizeof(melody[0])/2; 

// this calculates the duration of a whole note in ms (60s/tempo)*4 beats
int wholenote = (60000 * 4) / tempo;

int divider = 0, noteDuration = 0;


// Game State
int pX = 30, pY = 3;
int currentShape = 0, currentRotation = 0;
unsigned long lastGravity = 0, lastButtonPress = 0, lastMoveTime = 0;
int gravitySpeed = 300;
int moveSpeed = 125; 
byte board[32][8] = {0};
int score = 0;
bool gameover = false;
bool playSong = true;
// Shapes: [Type][4 blocks][X,Y offset]
const int8_t shapes[4][4][2] = {
  { {0,0}, {-1,0}, {-2,0}, {-3,0} }, // I-Bar
  { {0,0}, {-1,0}, {-2,0}, {0,1}  }, // L-Shape
  { {0,0}, {0,1}, {-1,0}, {-1,1}  }, // Square
  { {0,0}, {0,1}, {0,2}, {-1,1}   }  // T-Shape
};

// --- CORE FUNCTIONS ---

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Score: ");
  lcd.print(score);
}

void resetGame() {
  for (int r = 0; r < 32; r++) {
    for (int c = 0; c < 8; c++) board[r][c] = 0;
  }
  score = 0;
  gravitySpeed = 300;
  gameover = false;
  pX = 30; pY = 3;
  currentRotation = 0;
  currentShape = random(0, 4);
  mx.clear();
  updateLCD();
}

void getRotatedOffset(int shape, int block, int rot, int &outX, int &outY) {
  int x = shapes[shape][block][0], y = shapes[shape][block][1];
  if (rot == 0) { outX = x; outY = y; }
  else if (rot == 1) { outX = y; outY = -x; }
  else if (rot == 2) { outX = -x; outY = -y; }
  else if (rot == 3) { outX = -y; outY = x; }
}

void drawPiece(int x, int y, int type, int rot, bool state) {
  for (int i = 0; i < 4; i++) {
    int ox, oy;
    getRotatedOffset(type, i, rot, ox, oy);
    int drawX = x + ox, drawY = y + oy;
    if (drawX >= 0 && drawX < 32 && drawY >= 0 && drawY < 8) mx.setPoint(drawY, drawX, state);
  }
}

bool isCollision(int nextX, int nextY, int type, int nextRot) {
  for (int i = 0; i < 4; i++) {
    int ox, oy;
    getRotatedOffset(type, i, nextRot, ox, oy);
    int cx = nextX + ox, cy = nextY + oy;
    if (cx < 0 || cx > 31 || cy < 0 || cy > 7 || board[cx][cy] == 1) return true;
  }
  return false;
}

void checkLines() {
  for (int r = 0; r < 32; r++) {
    int count = 0;
    for (int c = 0; c < 8; c++) if (board[r][c] == 1) count++;
    if (count == 8) {
      for (int rowToMove = r; rowToMove < 31; rowToMove++) {
        for (int col = 0; col < 8; col++) board[rowToMove][col] = board[rowToMove + 1][col];
      }
      for (int col = 0; col < 8; col++) board[31][col] = 0;
      r--;
      mx.clear();
      tone(buzzerPin, 440, 100); delay(100); tone(buzzerPin, 880, 200);
      score += 100;
      updateLCD();
      if (gravitySpeed > 100) gravitySpeed -= 10;
    }
  }
}

void Intro_song(){
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(buzzer, melody[thisNote], noteDuration*0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);
    
    // stop the waveform generation before the next note.
    noTone(buzzer);
  }
}

void gameOver_song(){
  int size = sizeof(durations) / sizeof(int);

  for (int note = 0; note < size; note++) {
    //to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int duration = 1000 / durations[note];
    tone(buzzer, melody2[note], duration);

    //to distinguish the notes, set a minimum time between them.
    //the note's duration + 30% seems to work well:
    int pauseBetweenNotes = duration * 1.30;
    delay(pauseBetweenNotes);
    
    //stop the tone playing:
    noTone(buzzer);
  }
}

void refreshBoard() {
  for (int r = 0; r < 32; r++) {
    for (int c = 0; c < 8; c++) if (board[r][c] == 1) mx.setPoint(c, r, true);
  }
}

void setup() {
  Intro_song();
  lcd.begin();
  lcd.backlight();
  updateLCD();
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 1);
  pinMode(SW_PIN, INPUT_PULLUP);
  randomSeed(analogRead(A5));
  currentShape = random(0, 4);
  pinMode(buzzerPin, OUTPUT);
}

void loop() {
  // --- GAME OVER LOGIC (Scrolling Text) ---
  if (gameover) {
    if(playSong == true){
        gameOver_song();
        playSong = false;
    }
    static unsigned long lastScroll = 0;
    static int scrollPos = 0;
    static bool secondLineDrawn = false;
    String message = "Game Over! Press SW to Retry    "; 

    if (!secondLineDrawn) {
      lcd.setCursor(0, 1);
      lcd.print("Score: ");
      lcd.print(score);
      secondLineDrawn = true;
    }

    if (millis() - lastScroll > 250) {
      lcd.setCursor(0, 0);
      String displayStr = "";
      for (int i = 0; i < 16; i++) {
        int charIndex = (scrollPos + i) % message.length();
        displayStr += message[charIndex];
      }
      lcd.print(displayStr);
      scrollPos++;
      lastScroll = millis();
    }

    if (digitalRead(SW_PIN) == LOW) {
      delay(200);
      secondLineDrawn = false;
      scrollPos = 0;
      resetGame();
      playSong = true;
      Intro_song();
    }
    return; 
  }

  // --- REGULAR GAMEPLAY ---
  int xVal = analogRead(A0);
  int yVal = analogRead(A1);

  // 1. Rotation
  if (digitalRead(SW_PIN) == LOW && millis() - lastButtonPress > 250) {
    int nextRot = (currentRotation + 1) % 4;
    drawPiece(pX, pY, currentShape, currentRotation, false);
    if (!isCollision(pX, pY, currentShape, nextRot)) {
      currentRotation = nextRot;
    } else if (!isCollision(pX, pY + 1, currentShape, nextRot)) {
      pY++; currentRotation = nextRot; 
    } else if (!isCollision(pX, pY - 1, currentShape, nextRot)) {
      pY--; currentRotation = nextRot;
    }
    lastButtonPress = millis();
  }

  // 2. Controlled Movement
  if (millis() - lastMoveTime > moveSpeed) {
    bool moved = false;
    // Down (Fast fall)
    if (xVal > 700 && !isCollision(pX - 1, pY, currentShape, currentRotation)) {
      drawPiece(pX, pY, currentShape, currentRotation, false); pX--; moved = true;
    }
    // Left/Right
    if (yVal < 300 && !isCollision(pX, pY - 1, currentShape, currentRotation)) {
      drawPiece(pX, pY, currentShape, currentRotation, false); pY--; moved = true;
    }
    if (yVal > 700 && !isCollision(pX, pY + 1, currentShape, currentRotation)) {
      drawPiece(pX, pY, currentShape, currentRotation, false); pY++; moved = true;
    }
    if (moved) lastMoveTime = millis();
  }

  // 3. Gravity
  if (millis() - lastGravity > (unsigned long)gravitySpeed) {
    if (!isCollision(pX - 1, pY, currentShape, currentRotation)) {
      drawPiece(pX, pY, currentShape, currentRotation, false);
      pX--;
    } else {
      for (int i = 0; i < 4; i++) {
        int ox, oy;
        getRotatedOffset(currentShape, i, currentRotation, ox, oy);
        int lx = pX + ox, ly = pY + oy;
        if (lx >= 0 && lx < 32) board[lx][ly] = 1;
      }
      checkLines();
      pX = 30; pY = 3; currentRotation = 0; currentShape = random(0, 4);
      if (isCollision(pX, pY, currentShape, currentRotation)) {
        gameover = true;
      }
    }
    lastGravity = millis();
  }

  refreshBoard();
  drawPiece(pX, pY, currentShape, currentRotation, true);
  delay(20);
}
