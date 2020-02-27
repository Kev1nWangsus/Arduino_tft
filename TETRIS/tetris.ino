#define TETRIS 1

#include <SPI.h>

#include <ILI9341_t3.h>
#include <XPT2046_Touchscreen.h>

#include "blocks.h"
#include "font_BlackOpsOne-Regular.h"
#include "font_DroidSans.h"

#if TETRIS
#define TFT_DC       9
#define TFT_CS      10
#define TFT_RST    255  // 255 = unused, connect to 3.3V
#define TFT_MOSI    11
#define TFT_SCLK    13
#define TFT_MISO    12
#define TOUCH_CS     8
#define JOY_X       A0
#define JOY_Y       A1
#define JOY_BTN      1
#endif

ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);

XPT2046_Touchscreen ts(TOUCH_CS);

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 100
#define TS_MINY 100
#define TS_MAXX 3800
#define TS_MAXY 4000

#define SCREEN_BG   ILI9341_NAVY
#define GAME_BG     ILI9341_BLACK
#define SPEED_START 500
#define SPEED_MAX   200

#define TEXTFONT   DroidSans_9

uint16_t color_gamma[3][NUMCOLORS];
uint8_t  field[FIELD_WIDTH][FIELD_HEIGHT];
uint16_t aSpeed, score, highscore;
int8_t   nBlock, nColor, nRotation; //next Block
int8_t   aBlock, aColor, aX, aY, aRotation; //active Block


void initGame();
void initField() ;
bool checkMoveBlock(int deltaX, int deltaY, int deltaRotation);
bool game(bool demoMode);
char controls();
void setBlock();
void checkLines();
void nextBlock();
void effect1();
uint16_t colgamma(int16_t color, int16_t gamma);
void printColorText(const char * txt, unsigned colorOffset);
void printStartGame();
void printGameOver();
void printNum(unsigned num);
void printScore();
void printHighScore();
void drawBlockPix(int px, int py, int col);
void drawBlockPixSmall(int px, int py, int col);
void drawBlock(int blocknum, int px, int py, int rotation, int col);
void drawBlockSmall(bool draw);
void drawBlockEx(int blocknum, int px, int py, int rotation, int col, int oldx, int oldy, int oldrotation);
void drawField();

void setup() {
  
  
  #if TETRIS
    //CS0-CS2 Memoryboard
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    pinMode(JOY_BTN, INPUT_PULLUP);
    pinMode(JOY_X, INPUT);
    pinMode(JOY_Y, INPUT);
  #endif

  //color[0] is background, no gamma
  for (unsigned i=1; i < NUMCOLORS; i++) {
    color_gamma[0][i] = colgamma(color[i], 30);
    color_gamma[1][i] = colgamma(color[i], -70);
    color_gamma[2][i] = colgamma(color[i], -35);
  }
  delay(800);

  tft.begin();
  ts.begin();
  tft.setRotation(2);

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

  initGame();
  printScore();
  printHighScore();
}

void initGame(){
  score = 0;
  aSpeed = SPEED_START;
  initField();
}

void printColorText(const char * txt, unsigned colorOffset) {
  unsigned col=colorOffset;
  while(*txt) {
   tft.setTextColor(color[col]);
   tft.print(*txt);
   if (++col > NUMCOLORS-1) col = 1;
   txt++;
  }
}

void initField() {
  memset(field, 0, sizeof(field));
  tft.fillRect(FIELD_X, FIELD_Y, FIELD_WIDTH*PIX, FIELD_HEIGHT*PIX, GAME_BG);
}

void printNum(unsigned num) {
  if (num<1000) tft.print("0");
  if (num<100) tft.print("0");
  if (num<10) tft.print("0");
  tft.print(num);
}

void printScore() {
  tft.setFont(DroidSans_10);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(SIDE+5, 20);
  tft.fillRect(SIDE+5, 20, 45, 10, SCREEN_BG);
  printNum(score);
}

void printHighScore() {
  tft.setFont(DroidSans_10);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(SIDE+5, 60);
  tft.fillRect(SIDE+5, 60, 45, 10, SCREEN_BG);
  printNum(highscore);
}

void printGameOver() {
  tft.setFont(BlackOpsOne_40);
  tft.fillRect(FIELD_X, 120, FIELD_WIDTH*PIX, 40, GAME_BG);
  tft.fillRect(FIELD_X, 170, FIELD_WIDTH*PIX, 40, GAME_BG);

  int t = millis();
  unsigned cofs = 1;
  do {
    tft.setCursor(FIELD_X+10, 120);
    printColorText("GAME", cofs);
    tft.setCursor(FIELD_X+20, 170);
    printColorText("OVER", cofs);
    if (++cofs > NUMCOLORS-1) cofs = 1;
    delay(30);
  } while (millis()-t < 2000);
}

void printStartGame() {
  tft.setFont(BlackOpsOne_72);
  for (int i=3; i>0; i--) {
    tft.setCursor(FIELD_X + FIELD_WIDTH*PIX / 2 - 36, FIELD_Y+100);
    tft.setTextColor(color[i+2]);
    tft.print(i);
    delay(1000);
    tft.fillRect(FIELD_X + FIELD_WIDTH*PIX / 2 - 36, FIELD_Y+100, 72, 72, GAME_BG);
  }
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(FIELD_X+30, FIELD_Y+100);
  tft.print("GO");
  delay(400);
  initField();
}

void loop(void) {
  bool r = false;
  int c = 0;
  int c1 = 0;
  
   while(!r) {
    if (++c==8) {
        effect1();
        c = 0;
    }
    if (++c1==50) {
      c1 = 0;
    }
    r = game(true);
   }
   game(false);
}


bool game(bool demoMode) {
  bool gameOver = false;
  int tk = 0;

  initGame();
  if (!demoMode) 
    printStartGame();

  nextBlock();
  drawBlockSmall(true);
  drawBlock(aBlock, aX, aY, aRotation, aColor);

  do {
    yield();

    int t = millis();

    if (!demoMode) do {  // process controls
      if (millis() - tk > aSpeed/3) {
        char ch = controls();
        if (ch != '\0') tk = millis();
        switch (ch) {
          case '+' :  //drop
              // TODO: correct this one to drop at click
              t = 0;
              break;
          case 's' :  //down
              t = 0;
              break;
          case 'w' :  //rotate
              if (checkMoveBlock(0,0,1)) {
                  int oldaRotation = aRotation;
                  aRotation = (aRotation + 1) & 0x03;
                  drawBlockEx(aBlock, aX, aY, aRotation, aColor, aX, aY, oldaRotation);
              }
              break;
          case 'a' : //left
              if (checkMoveBlock(-1,0,0)) {
                  drawBlockEx(aBlock, aX + (-1), aY, aRotation, aColor, aX, aY, aRotation);
                  aX--;
              }
              break;
          case 'd' : //right
              if (checkMoveBlock(1,0,0)) {
                  drawBlockEx(aBlock, aX + 1, aY, aRotation, aColor, aX, aY, aRotation);
                  aX++;
              }
              break;
        }
      }
      yield();
    } while ( millis() - t < aSpeed);   // process controls end

    else { //demoMode
      delay(100);
      char ch = controls();
      if (ch != '\0')  return true;
    }

    //move the block down
    bool movePossible = checkMoveBlock(0,1,0);
    if ( movePossible ){
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
      drawBlockSmall(true);
      drawBlock(aBlock, aX, aY, aRotation, aColor);
      if (!checkMoveBlock(0,0,0)) {
        //no, game over !
        initField();
        gameOver = true;
      }
    }

    if (!demoMode) {
      printScore();
    }
    
  } while(!gameOver);

  if (!demoMode) {
    if (score > highscore) {
        highscore = score;
        printHighScore();
    }
    Serial.println();
    Serial.print("Score: ");
    Serial.println(score);
    printGameOver();
  }
  drawBlockSmall(false);
  return false;
}

char controls() {
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  bool joyBTN = digitalRead(JOY_BTN);

  static bool hasClicked = false;
  // rotate
  // rotate blocks if isClicked == true && hasClicked == false
  bool isClicked = (digitalRead(JOY_BTN) == LOW);
  if (isClicked){
    delay(50);
    isClicked = (digitalRead(JOY_BTN) == LOW);
  }
  
  hasClicked = hasClicked && isClicked;
  if (!hasClicked && isClicked){
    hasClicked = true;
    return ('w');
  }
  
  // right
  if (joyX > 800) return ('d');

  // left
  if (joyX < 490) return ('a');

  // down
  if (joyY > 800) return ('s');

  return ('\0' );
}

void setBlock() {
  int bH = BLOCKHEIGHT(aBlock, aRotation);
  int bW = BLOCKWIDTH(aBlock, aRotation);

  for (int y=0; y<bH; y++) {
    for (int x=0; x<bW; x++) {
      if ( (block[aRotation][aBlock][y * 4 + x + 2] > 0) ) {
        field[x + aX][y + aY] = aColor;
      }
    }
  }

}

void checkLines() {
  int x,y,c,i;
  for (y=0; y<FIELD_HEIGHT; y++) {
    c = 0;
    for (x=0; x<FIELD_WIDTH; x++) {
      if (field[x][y] > 0) c++;
    }

    if ( c >= FIELD_WIDTH ) {//complete line !
      //line-effect:
      for (i = NUMCOLORS-1; i >= 0; i--) {
        for (x =0; x < FIELD_WIDTH; x++) {
          drawBlockPix(FIELD_X + x*PIX,FIELD_Y + y*PIX,i);
        }
        delay(60);
      }

      //move entire field above complete line down and redraw
      for (i = y; i>0; i--)
        for (x=0; x<FIELD_WIDTH; x++)
          field[x][i]=field[x][i-1];
      for (x=0; x<FIELD_WIDTH; x++)
          field[x][0]=0;

      drawField();
      if (aSpeed>SPEED_MAX) aSpeed -= 5;
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

  for (int y=bH-1; y>= 0; y--) {
    for (int x=0; x<bW; x++) {
      if ( (field[x + dX][y + dY] > 0) && (block[rot][aBlock][y * 4 + x + 2] > 0) ) {
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

  nColor = random(1,NUMCOLORS);
  nBlock = random(NUMBLOCKS);
  nRotation = random(4);
}

void effect1() {
  int t = millis();
  do {
    nextBlock();
    drawBlock(aBlock,aX,random(FIELD_HEIGHT),aRotation,aColor);
  } while (millis() - t < 1000);
}

uint16_t colgamma(int16_t color, int16_t gamma){
 return  tft.color565(
    constrain(((color>>8) & 0xf8) + gamma,0,255),  //r
    constrain(((color>>3) & 0xfc) + gamma,0,255),  //g
    constrain(((color<<3) & 0xf8) + gamma,0,255)); //b
}

void drawBlockPix(int px, int py, int col) {

    if (px>=FIELD_XW) return;
    if (px<FIELD_X) return;
    if (py>=FIELD_YW) return;
    if (py<FIELD_Y) return;

    if (col == 0) {
      //remove Pix, draw backgroundcolor
      tft.fillRect(px, py, PIX, PIX, color[col]);
      return;
    }

    const int w=4;

    tft.fillRect(px+w, py+w, PIX-w*2+1, PIX-w*2+1, color[col]);
    for (int i = 0; i<w;i++) {
     tft.drawFastHLine(px + i, py + i, PIX-2*i , color_gamma[0][col]);
     tft.drawFastHLine(px + i, PIX + py - i - 1 , PIX-2*i , color_gamma[1][col]);
     tft.drawFastVLine(px + i, py + i , PIX-2*i , color_gamma[2][col]);
     tft.drawFastVLine(px + PIX - i - 1, py + i , PIX-2*i , color_gamma[2][col]);
    }
 }

inline void drawBlockPixSmall(int px, int py, int col) {
  
    const int w=2;

    tft.fillRect(px+w, py+w, PIXSMALL-w*2+1, PIXSMALL-w*2+1, color[col]);
    for (int i = 0; i<w;i++) {
     tft.drawFastHLine(px + i, py + i, PIXSMALL-2*i , color_gamma[0][col]);
     tft.drawFastHLine(px + i, PIXSMALL + py - i - 1 , PIXSMALL-2*i , color_gamma[1][col]);
     tft.drawFastVLine(px + i, py + i , PIXSMALL-2*i , color_gamma[2][col]);
     tft.drawFastVLine(px + PIXSMALL - i - 1, py + i , PIXSMALL-2*i , color_gamma[2][col]);
    }
 }


void drawBlock(int blocknum, int px, int py, int rotation, int col) {
    int w = BLOCKWIDTH(blocknum, rotation);
    int h = BLOCKHEIGHT(blocknum, rotation);

    for (int x=0; x<w; x++) {
       for (int y=0; y<h; y++) {
         if (block[rotation][blocknum][y*4 + x + 2])
            drawBlockPix(FIELD_X+px*PIX+x*PIX, FIELD_Y+py*PIX+y*PIX, col);
         }
     }
 }

void drawBlockSmall(bool draw) {
//    const int px = 195;
//    const int py = 270;
    
    tft.setCursor(FIELD_XW+5,250);
    tft.setFont(DroidSans_9);
    tft.setTextColor(ILI9341_WHITE); 
    tft.print("Next:");
    tft.fillRect(FIELD_XW+4, 270, PIXSMALL * 4, PIXSMALL * 4, SCREEN_BG);
    
    if (draw) {
      for (int x=0; x<4; x++) {
        for (int y=0; y<4; y++) {
         if (block[nRotation][nBlock][y*4 + x + 2])
            drawBlockPixSmall(FIELD_XW+4+x*PIXSMALL, 270+y*PIXSMALL, nColor);
        }
      }
    }
 }

static uint8_t dbuf[FIELD_WIDTH][FIELD_HEIGHT] ={0};
void drawBlockEx(int blocknum, int px, int py, int rotation, int col, int oldx, int oldy, int oldrotation) {

    int x, y;
    int w = BLOCKWIDTH(blocknum, oldrotation);
    int h = BLOCKHEIGHT(blocknum, oldrotation);

    for (x=0; x<w; x++)
       for (y=0; y<h; y++)
         if (block[oldrotation][blocknum][y*4 + x + 2]>0) dbuf[x + oldx][y + oldy] = 2;

    w = BLOCKWIDTH(blocknum, rotation);
    h = BLOCKHEIGHT(blocknum, rotation);
    for (x=0; x<w; x++)
       for (y=0; y<h; y++)
         if (block[rotation][blocknum][y*4 + x + 2]>0) dbuf[x + px][y + py] = 1;

    for (y=FIELD_HEIGHT-1; y>=oldy; y--)
       for (x=0; x<FIELD_WIDTH; x++)
         switch(dbuf[x][y]) {
           case 1:  drawBlockPix(FIELD_X+x*PIX, FIELD_Y+y*PIX, col); dbuf[x][y]=0;break;
           case 2:  tft.fillRect(FIELD_X+x*PIX, FIELD_Y+y*PIX, PIX, PIX, color[0]); dbuf[x][y]=0; break;
        }

}

void drawField() {
  int x,y;
  for (y=FIELD_HEIGHT-1; y>=0; y--)
    for (x=0; x<FIELD_WIDTH; x++)
       drawBlockPix(FIELD_X+x*PIX, FIELD_Y+y*PIX, field[x][y]);
}
