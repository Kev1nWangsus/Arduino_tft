#include <SPI.h>

#include <ILI9341_t3.h>
#include <URTouch.h>

// artwork and font
#include "blocks.h"
#include "font_BlackOpsOne-Regular.h"
#include "font_DroidSans.h"

// display pins
#define TFT_DC       9
#define TFT_CS      10
#define TFT_RST    255  // 255 = unused, connect to 3.3V
#define TFT_MOSI    11
#define TFT_SCLK    13
#define TFT_MISO    12

// touch screen pins
#define TCLK        16
#define TCS         17
#define TDIN        18
#define TDOUT       19
#define IRQ         20

// control system
// analog readings
#define JOY_X       A0
#define JOY_Y       A1
// digital readings
#define JOY_BTN      1
#define BTN          2

ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);
URTouch mytouch = URTouch(TCLK, TCS, TDIN, TDOUT, IRQ);

// most used colors
#define SCREEN_BG        ILI9341_NAVY
#define GAME_BG         ILI9341_BLACK

// time
#define initialTimeInterval       600
#define minTimeInterval           200

// text font
#define TEXTFONT       DroidSans_9
#define GAMETITLE      DroidSans_20
#define TITLE          DroidSans_40

uint16_t color_gamma[3][NUMCOLORS];
uint8_t  field[FIELD_WIDTH][FIELD_HEIGHT];
int timeInterval, score, highscore;
int8_t   nBlock, nColor, nRotation; //next Block
int8_t   aBlock, aColor, aX, aY, aRotation; //active Block

bool isHome = true;
char currentPage = '0';
unsigned cofs = 1;

// Home page function
void drawHomeScreen();

// Tetris function
void tetrisSetup();
void initTetris();
void initTetrisField();
void gameloop();
bool checkMoveBlock(int deltaX, int deltaY, int deltaRotation);
bool tetrisGame(bool demoMode);
char joystickControls();
void setBlock();
void checkLines();
void nextBlock();
uint16_t colgamma(int16_t color, int16_t gamma);
void printColorText(const char * txt, unsigned colorOffset);
void countDown();
void printGameOver();
void formatScore(unsigned num);
void printScore();
void printHighScore();
void drawBlockPix(int px, int py, int col);
void drawBlockPixSmall(int px, int py, int col);
void drawBlock(int blocknum, int px, int py, int rotation, int col);
void preview(bool draw);
void drawBlockEx(int blocknum, int px, int py, int rotation, int col, int oldx, int oldy, int oldrotation);
void drawTetrisField();


void drawHomeScreen() {
  // Background
  tft.fillRect(0, 0, 240, 320, GAME_BG);

  // START_PAGE
  tft.setTextColor(ILI9341_WHITE); // Sets green color
  tft.fillRect(30, 105, 180, 30, ILI9341_NAVY); // Draws filled rounded rectangle
  tft.setCursor(80, 110);
  tft.setFont(GAMETITLE); // Sets the font to big
  tft.print("START"); // Prints the string

  // SETTING_PAGE
  tft.setTextColor(ILI9341_WHITE);
  tft.fillRect(30, 145, 180, 30, ILI9341_NAVY);
  tft.setCursor(60, 150);
  tft.setFont(GAMETITLE);
  tft.print("SETTINGS");

  // SCORE_PAGE
  tft.setTextColor(ILI9341_WHITE);
  tft.fillRect(30, 185, 180, 30, ILI9341_NAVY);
  tft.setCursor(40, 190);
  tft.setFont(GAMETITLE);
  tft.print("HIGH SCORE");
}

void setup() {
  // setup the screen
  tft.begin();
  mytouch.InitTouch();
  tft.setRotation(2);
  drawHomeScreen();
}

void loop() {

  // color text
  if (isHome) {
    tft.setFont(TITLE);
    tft.setCursor(35, 40);
    printColorText("TETRIS", cofs);
    if (++cofs > NUMCOLORS - 1) cofs = 1;
    delay(50);
  }

  // switch to the page by clicking
  if (mytouch.dataAvailable()) {
    mytouch.read();
    int x = mytouch.getX();
    int y = mytouch.getY();
    if (x >= 30 && x <= 210 && y >= 105 && y <= 215) {
      isHome = false;
      if (y >= 105 && y <= 135) tetrisSetup(); // tetris game
      // if (y >= 145 && y <= 175) settings(); // settings
      // if (y >= 185 && y <= 215) highscore(); // high score
    }
  }

  // game loop
  // enter the game if current page is '1'
  gameloop(currentPage);

}

void tetrisSetup() {
  currentPage = '1';
  tft.setRotation(2);
  pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);
  pinMode(BTN, INPUT);

  //color[0] is background, no gamma
  for (unsigned i = 1; i < NUMCOLORS; i++) {
    color_gamma[0][i] = colgamma(color[i], 30);
    color_gamma[1][i] = colgamma(color[i], -70);
    color_gamma[2][i] = colgamma(color[i], -35);
  }
  delay(800);

  highscore = 0;
  nextBlock();
  tft.fillScreen(SCREEN_BG);

  tft.setCursor(SIDE, 0);
  tft.setFont(TEXTFONT);
  tft.setTextColor(ILI9341_GREEN);
  tft.print("Current");
  tft.setCursor(SIDE, 10);
  tft.print("Score:");

  tft.setCursor(SIDE, 40);
  tft.setFont(TEXTFONT);
  tft.setTextColor(ILI9341_YELLOW);
  tft.print("Highest");
  tft.setCursor(SIDE, 50);
  tft.print("Score:");

  initTetris();
  printScore();
  printHighScore();
}

void initTetris() {
  score = 0;
  timeInterval = initialTimeInterval;
  initTetrisField();
}

void printColorText(const char *txt, unsigned colorOffset) {
  unsigned col = colorOffset;
  while (*txt) {
    tft.setTextColor(color[col]);
    tft.print(*txt);
    if (++col > NUMCOLORS - 1) col = 1;
    txt++;
  }
}

void initTetrisField() {
  memset(field, 0, sizeof(field));
  tft.fillRect(FIELD_X, FIELD_Y, FIELD_WIDTH * PIX, FIELD_HEIGHT * PIX, GAME_BG);
}

void formatScore(int number) {
  //
  if (number < 1000) tft.print("0");
  if (number < 100)  tft.print("0");
  if (number < 10)   tft.print("0");
  tft.print(number);
}

void printScore() {
  tft.setFont(DroidSans_10);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(SIDE + 5, 20);
  tft.fillRect(SIDE + 5, 20, 45, 10, SCREEN_BG);
  formatScore(score);
}

void printHighScore() {
  tft.setFont(DroidSans_10);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(SIDE + 5, 60);
  tft.fillRect(SIDE + 5, 60, 45, 10, SCREEN_BG);
  formatScore(highscore);
}

void printGameOver() {
  tft.setFont(BlackOpsOne_40);
  tft.fillRect(FIELD_X, 120, FIELD_WIDTH * PIX, 40, GAME_BG);
  tft.fillRect(FIELD_X, 170, FIELD_WIDTH * PIX, 40, GAME_BG);

  int t = millis();
  unsigned cofs = 1;
  do {
    tft.setCursor(FIELD_X + 10, 120);
    printColorText("GAME", cofs);
    tft.setCursor(FIELD_X + 20, 170);
    printColorText("OVER", cofs);
    if (++cofs > NUMCOLORS - 1) cofs = 1;
    delay(30);
  } while (millis() - t < 2000);
}

void countDown() {
  tft.setFont(BlackOpsOne_72);

  // count down
  for (int i = 3; i > 0; i--) {
    tft.setCursor(FIELD_X + FIELD_WIDTH * PIX / 2 - 36, FIELD_Y + 100);
    tft.setTextColor(color[i + 2]);
    tft.print(i);
    delay(1000);
    tft.fillRect(FIELD_X + FIELD_WIDTH * PIX / 2 - 36, FIELD_Y + 100, 72, 72, GAME_BG);
  }
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(FIELD_X + 30, FIELD_Y + 100);
  tft.print("GO");
  delay(400);
  initTetrisField();
}

void gameloop(char currentPage) {
  while (currentPage == '1') {
    bool r = false;
    int c = 0;
    int c1 = 0;

    while (!r) {
      if (++c == 8) {
        c = 0;
      }
      if (++c1 == 50) {
        c1 = 0;
      }
      r = tetrisGame(true);
    }
    tetrisGame(false);
  }
}

char joystickControls() {
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  int joyButton = digitalRead(JOY_BTN);
  int commonButton = digitalRead(BTN);
  static bool joyHasClicked = false;
  static bool hasClicked = false;

  // rotate
  bool joyIsClicked = (joyButton == LOW);
  if (joyIsClicked) {
    delay(50);
    joyIsClicked = (joyButton == LOW);
  }
  joyHasClicked = joyHasClicked && joyIsClicked;
  if (!joyHasClicked && joyIsClicked) {
    joyHasClicked = true;
    return ('+');
  }

  // drop
  bool isClicked = (commonButton == HIGH);
  if (isClicked) {
    delay(50);
    isClicked = (commonButton == HIGH);
  }
  hasClicked = hasClicked && isClicked;
  if (!hasClicked && isClicked) {
    hasClicked = true;
    return ('q');
  }
  // up
  if (joyY < 490) return ('w');
  // left
  if (joyX < 490) return ('a');
  // right
  if (joyX > 800) return ('d');
  // down
  if (joyY > 900) return ('s');

  return ('\0');
}

bool tetrisGame(bool demoMode) {
  bool gameOver = false;
  int tk = 0;

  initTetris();
  if (!demoMode)
    countDown();

  nextBlock();
  preview(true);
  drawBlock(aBlock, aX, aY, aRotation, aColor);

  do {
    yield();

    int t = millis();

    if (!demoMode) do {  // process control
        if (millis() - tk > 100) {
          char ch = joystickControls();
          if (ch != '\0') tk = millis();
          switch (ch) {
            case '+' : // rotate
              if (checkMoveBlock(0, 0, 1)) {
                int oldaRotation = aRotation;
                aRotation = (aRotation + 1) & 0x03;
                drawBlockEx(aBlock, aX, aY, aRotation, aColor, aX, aY, oldaRotation);
              }
              break;
            case 's' : // down
              t = 0;
              break;
            case 'a' : // left
              if (checkMoveBlock(-1, 0, 0)) {
                drawBlockEx(aBlock, aX + (-1), aY, aRotation, aColor, aX, aY, aRotation);
                aX--;
              }
              break;
            case 'd' : // right
              if (checkMoveBlock(1, 0, 0)) {
                drawBlockEx(aBlock, aX + 1, aY, aRotation, aColor, aX, aY, aRotation);
                aX++;
              }
              break;
            case 'q' : // drop
              bool movePossible = checkMoveBlock(0, 1, 0);
              while (movePossible) {
                drawBlockEx(aBlock, aX, aY + 1, aRotation, aColor, aX, aY, aRotation);
                aY++;
                movePossible = checkMoveBlock(0, 1, 0);
              }
              break;
          }
        }
        yield();
      } while (millis() - t < timeInterval);  // process joystickControls end

    else {
      //demoMode
      delay(20);
      char ch = joystickControls();
      if (ch != '\0')  return true;
    }

    //move the block down
    bool movePossible = checkMoveBlock(0, 1, 0);
    if (movePossible) {
      drawBlockEx(aBlock, aX, aY + 1, aRotation, aColor, aX, aY, aRotation);
      aY++;
    }

    else {
      //block stopped moving down
      //store location
      setBlock();
      checkLines();
      //get new block and draw it
      score += 10;
      nextBlock();
      preview(true);
      drawBlock(aBlock, aX, aY, aRotation, aColor);
      if (!checkMoveBlock(0, 0, 0)) {
        //no, game over !
        initTetrisField();
        gameOver = true;
      }
    }

    if (!demoMode) {
      printScore();
    }

  } while (!gameOver);

  if (!demoMode) {
    if (score > highscore) {
      highscore = score;
      printHighScore();
    }
    printGameOver();
  }
  preview(false);
  return false;
}

void setBlock() {
  int bH = BLOCKHEIGHT(aBlock, aRotation);
  int bW = BLOCKWIDTH(aBlock, aRotation);

  for (int y = 0; y < bH; y++) {
    for (int x = 0; x < bW; x++) {
      if (block[aRotation][aBlock][y * 4 + x + 2] > 0) {
        field[x + aX][y + aY] = aColor;
      }
    }
  }
}

void checkLines() {
  int x, y, c, i;
  for (y = 0; y < FIELD_HEIGHT; y++) {
    c = 0;
    for (x = 0; x < FIELD_WIDTH; x++) {
      if (field[x][y] > 0)
        c++;
    }

    if (c >= FIELD_WIDTH) {
      //complete line !
      //line-effect:
      for (i = NUMCOLORS - 1; i >= 0; i--) {
        for (x = 0; x < FIELD_WIDTH; x++) {
          drawBlockPix(FIELD_X + x * PIX, FIELD_Y + y * PIX, i);
        }
        delay(60);
      }

      //move entire field above complete line down and redraw
      for (i = y; i > 0; i--) {
        for (x = 0; x < FIELD_WIDTH; x++) {
          field[x][i] = field[x][i - 1];
        }
      }
      for (x = 0; x < FIELD_WIDTH; x++) {
        field[x][0] = 0;
      }
      score += 50;
      drawTetrisField();
      if (timeInterval > minTimeInterval)
        timeInterval -= 5;
      // increase speed as more lines are removed
    }
  }
}

bool checkMoveBlock(int deltaX, int deltaY, int deltaRotation) {
  int rot = (aRotation + deltaRotation) & 0x03;
  int bH = BLOCKHEIGHT(aBlock, rot);
  int dY = aY + deltaY;

  if (dY + bH > FIELD_HEIGHT)  //lower border
    return false;

  int bW = BLOCKWIDTH(aBlock, rot);
  int dX = aX + deltaX;

  if (dX < 0 || dX + bW > FIELD_WIDTH) { //left/right border
    return false;
  }

  for (int y = bH - 1; y >= 0; y--) {
    for (int x = 0; x < bW; x++) {
      if ((field[x + dX][y + dY] > 0) && (block[rot][aBlock][y * 4 + x + 2] > 0)) {
        return false;
      }
    }
  }

  return true;
}

void nextBlock() {
  aColor = nColor;
  aBlock = nBlock;
  aRotation = nRotation;
  aY = 0;
  aX = random(FIELD_WIDTH - BLOCKWIDTH(aBlock, aRotation) + 1);
  nColor = random(1, NUMCOLORS);
  nBlock = random(NUMBLOCKS);
  nRotation = random(4);
}

uint16_t colgamma(int16_t color, int16_t gamma) {
  return  tft.color565(
            constrain(((color >> 8) & 0xf8) + gamma, 0, 255), //r
            constrain(((color >> 3) & 0xfc) + gamma, 0, 255), //g
            constrain(((color << 3) & 0xf8) + gamma, 0, 255)); //b
}

void drawBlockPix(int px, int py, int col) {

  if (px >= FIELD_XW) return;
  if (px < FIELD_X) return;
  if (py >= FIELD_YW) return;
  if (py < FIELD_Y) return;

  if (col == 0) {
    //remove Pix, draw backgroundcolor
    tft.fillRect(px, py, PIX, PIX, color[col]);
    return;
  }

  const int w = 4;

  tft.fillRect(px + w, py + w, PIX - w * 2 + 1, PIX - w * 2 + 1, color[col]);
  for (int i = 0; i < w; i++) {
    tft.drawFastHLine(px + i, py + i, PIX - 2 * i, color_gamma[0][col]);
    tft.drawFastHLine(px + i, PIX + py - i - 1, PIX - 2 * i, color_gamma[1][col]);
    tft.drawFastVLine(px + i, py + i , PIX - 2 * i, color_gamma[2][col]);
    tft.drawFastVLine(px + PIX - i - 1, py + i, PIX - 2 * i, color_gamma[2][col]);
  }
}

inline void drawBlockPixSmall(int px, int py, int col) {

  const int w = 2;

  tft.fillRect(px + w, py + w, PIXSMALL - w * 2 + 1, PIXSMALL - w * 2 + 1, color[col]);
  for (int i = 0; i < w; i++) {
    tft.drawFastHLine(px + i, py + i, PIXSMALL - 2 * i , color_gamma[0][col]);
    tft.drawFastHLine(px + i, PIXSMALL + py - i - 1 , PIXSMALL - 2 * i , color_gamma[1][col]);
    tft.drawFastVLine(px + i, py + i , PIXSMALL - 2 * i , color_gamma[2][col]);
    tft.drawFastVLine(px + PIXSMALL - i - 1, py + i , PIXSMALL - 2 * i , color_gamma[2][col]);
  }
}


void drawBlock(int blocknum, int px, int py, int rotation, int col) {
  int w = BLOCKWIDTH(blocknum, rotation);
  int h = BLOCKHEIGHT(blocknum, rotation);

  for (int x = 0; x < w; x++) {
    for (int y = 0; y < h; y++) {
      if (block[rotation][blocknum][y * 4 + x + 2])
        drawBlockPix(FIELD_X + px * PIX + x * PIX, FIELD_Y + py * PIX + y * PIX, col);
    }
  }
}

void preview(bool draw) {
  tft.setCursor(FIELD_XW + 5, 250);
  tft.setFont(DroidSans_9);
  tft.setTextColor(ILI9341_WHITE);
  tft.print("Next:");
  tft.fillRect(FIELD_XW + 4, 270, PIXSMALL * 4, PIXSMALL * 4, SCREEN_BG);

  if (draw) {
    for (int x = 0; x < 4; x++) {
      for (int y = 0; y < 4; y++) {
        if (block[nRotation][nBlock][y * 4 + x + 2])
          drawBlockPixSmall(FIELD_XW + 4 + x * PIXSMALL, 270 + y * PIXSMALL, nColor);
      }
    }
  }
}

static uint8_t dbuf[FIELD_WIDTH][FIELD_HEIGHT] = {0};
void drawBlockEx(int blocknum, int px, int py, int rotation, int col, int oldx, int oldy, int oldrotation) {

  int x, y;
  int w = BLOCKWIDTH(blocknum, oldrotation);
  int h = BLOCKHEIGHT(blocknum, oldrotation);

  for (x = 0; x < w; x++) {
    for (y = 0; y < h; y++) {
      if (block[oldrotation][blocknum][y * 4 + x + 2] > 0)
        dbuf[x + oldx][y + oldy] = 2;
    }
  }

  w = BLOCKWIDTH(blocknum, rotation);
  h = BLOCKHEIGHT(blocknum, rotation);
  for (x = 0; x < w; x++)
    for (y = 0; y < h; y++)
      if (block[rotation][blocknum][y * 4 + x + 2] > 0) dbuf[x + px][y + py] = 1;

  for (y = FIELD_HEIGHT - 1; y >= oldy; y--) {
    for (x = 0; x < FIELD_WIDTH; x++) {
      switch (dbuf[x][y]) {
        case 1:
          drawBlockPix(FIELD_X + x * PIX, FIELD_Y + y * PIX, col);
          dbuf[x][y] = 0;
          break;
        case 2:
          tft.fillRect(FIELD_X + x * PIX, FIELD_Y + y * PIX, PIX, PIX, color[0]);
          dbuf[x][y] = 0;
          break;
      }
    }
  }
}

void drawTetrisField() {
  // draw the black background for tetris

  for (int y = FIELD_HEIGHT - 1; y >= 0; y--) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      drawBlockPix(FIELD_X + x * PIX, FIELD_Y + y * PIX, field[x][y]);
    }
  }
}




void snakeSetup() {
  currentPage = '2';
  tft.setRotation(1);
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);

  tft.fillScreen(GAME_BG);
  tft.fillRect(140, 110, 40, 10, ILI9341_WHITE);
}
