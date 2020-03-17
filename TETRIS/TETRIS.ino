#include <SPI.h>
#include <ILI9341_t3.h>
#include <URTouch.h>

// blocks and font
#include "tetris.h"
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

// text font
#define TEXTFONT       DroidSans_9
#define LEVELFONT      DroidSans_8
#define GAMETITLE      DroidSans_20
#define TITLE          BlackOpsOne_40

uint16_t    color_gamma[3][NUMCOLORS];
uint8_t     field[FIELD_WIDTH][FIELD_HEIGHT];
int         timeInterval, score, highscore;
int         initialTimeInterval = 600, minTimeInterval = 200;
int8_t      nBlock, nColor, nRotation; //next Block
int8_t      aBlock, aColor, aX, aY, aRotation; //active Block

bool isHome = true;
unsigned cofs = 1;

// Home page function
void drawHomeScreen();

// Tetris function
void tetrisSetup(); 
void initTetrisField();
void gameloop();
bool checkMoveBlock(int dX, int dY, int dRotation);
bool tetrisGame(bool demoMode);
char joystickControls();
void setBlock();
void checkLines();
void nextBlock();
void countDown();
void formatScore(unsigned num);
void printHighScore();
void printGameOver();
void printScore();
void drawBlock(int blocknum, int px, int py, int rotation, int col);
void drawBlockPix(int px, int py, int col);
void drawBlockPixSmall(int px, int py, int col);
void preview();
void drawBlockEx(int blocknum, int px, int py, int rotation, int col, int oldx, int oldy, int oldrotation);
void drawTetrisField();
void settings();

// artwork
uint16_t colgamma(int16_t color, int16_t gamma);
void printColorText(const char * txt, unsigned colorOffset, int len);


void drawHomeScreen(){
  // Background
  tft.fillRect(0, 0, 240, 320, GAME_BG);

  // START_PAGE
  tft.setTextColor(ILI9341_WHITE); // Sets green color
  tft.fillRect(30, 105, 180, 40, ILI9341_NAVY); // Draws filled rounded rectangle
  tft.setCursor(80, 115);
  tft.setFont(GAMETITLE); // Sets the font to big
  tft.print("START"); // Prints the string

  // SETTING_PAGE
  tft.setTextColor(ILI9341_WHITE);
  tft.fillRect(30, 155, 180, 40, ILI9341_NAVY);
  tft.setCursor(60, 165);
  tft.setFont(GAMETITLE);
  tft.print("SETTINGS");

//  // SCORE_PAGE
//  tft.setTextColor(ILI9341_WHITE);
//  tft.fillRect(30, 195, 180, 40, ILI9341_NAVY);
//  tft.setCursor(40, 200);
//  tft.setFont(GAMETITLE);
//  tft.print("HIGH SCORE");
}

void setup(){
  // setup the screen
  tft.begin();
  mytouch.InitTouch(0);
  mytouch.setPrecision(PREC_EXTREME);
  tft.setRotation(2);
  drawHomeScreen();

  // define pinMode
  pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);
  pinMode(BTN, INPUT);
}

void loop(){
  tft.setRotation(2);
  // color text
  if (isHome){
    tft.setFont(TITLE);
    tft.setCursor(13, 40);
    printColorText("TETRIS", cofs, 6);
    if (++cofs > NUMCOLORS-1) cofs = 1;
    delay(50);
  }



  // switch pages by clicking
  mytouch.InitTouch(0);
  if(mytouch.dataAvailable()){
    mytouch.read();
    int x = 240 - mytouch.getX();
    int y = 320 - mytouch.getY();
    Serial.println(x);
    Serial.println(y);
    if (x >= 30 && x <= 210 && y >= 105 && y <= 215){
      isHome = false; 
      if (y >= 105 && y <= 145){
        tetrisSetup(600);
        gameloop();// tetris game
      } else {
        settings(); // settings
      }
      // if (y >= 185 && y <= 215) highscore(); // high score
    }
  }   
}

void tetrisSetup(int iT){
  // shade; artwork;
  // color[0] is background, no gamma
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

  score = 0;
  timeInterval = iT;
  initTetrisField();
  printScore();
  printHighScore();
}

void settings(){
  tft.fillRect(0, 0, 240, 320, GAME_BG);
  // EASY
  tft.setTextColor(ILI9341_WHITE); // Sets green color
  tft.fillRect(30, 200, 180, 30, ILI9341_NAVY); // Draws filled rounded rectangle
  tft.setCursor(90, 205);
  tft.setFont(GAMETITLE); // Sets the font to big
  tft.print("Easy"); // Prints the string

  // MEDIUM
  tft.setTextColor(ILI9341_WHITE);
  tft.fillRect(30, 240, 180, 30, ILI9341_NAVY);
  tft.setCursor(70, 245);
  tft.setFont(GAMETITLE);
  tft.print("Medium");

  // HARD
  tft.setTextColor(ILI9341_WHITE);
  tft.fillRect(30, 280, 180, 30, ILI9341_NAVY);
  tft.setCursor(90, 285);
  tft.setFont(GAMETITLE);
  tft.print("Hard");

  while(true){
    if(mytouch.dataAvailable()){
      mytouch.read();
      int x = 240 - mytouch.getX();
      int y = 320 - mytouch.getY();
      Serial.println(x);
      Serial.println(y);
      if (x >= 30 && x <= 210 && y >= 200 && y <= 310){
        if (y >= 200 && y <= 230){
          tetrisSetup(600);
          gameloop();
          break;// easy
        } else if (y >= 240 && y <= 270){
          tetrisSetup(500);
          gameloop();
          break;// medium
        } else if (y >= 280 && y <= 310){
          tetrisSetup(300);
          gameloop();
          break;// hard
        }
      }
    }
  }
}

void printGameLevel(int t){
  // print current game level according to the dropping rate
  tft.setCursor(SIDE, 90);
  tft.setFont(TEXTFONT);
  tft.setTextColor(ILI9341_WHITE);
  tft.print("Level:");
  tft.setCursor(SIDE, 102);
  tft.setFont(LEVELFONT);
  tft.fillRect(SIDE, 100, 40, 200, ILI9341_NAVY);
  if (t <= 600 && t > 500){
    tft.print("Easy");
  } else if (t <= 500 && t > 300){
    tft.print("Medium");
  } else if (t <= 300){
    tft.print("Hard");
  }
}

void initTetrisField(){
  // initiate tetris field
  memset(field, 0, sizeof(field));
  tft.fillRect(FIELD_X, FIELD_Y, FIELD_WIDTH*PIX, FIELD_HEIGHT*PIX, GAME_BG);
}

void formatScore(int number){
  // format score
  if (number < 1000) tft.print("0");
  if (number < 100)  tft.print("0");
  if (number < 10)   tft.print("0");
  tft.print(number);
}

void printScore(){
  // print score
  tft.setFont(DroidSans_10);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(SIDE+5, 20);
  tft.fillRect(SIDE+5, 20, 45, 10, SCREEN_BG);
  formatScore(score);
}

void printHighScore(){
  tft.setFont(DroidSans_10);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(SIDE+5, 60);
  tft.fillRect(SIDE+5, 60, 45, 10, SCREEN_BG);
  formatScore(highscore);
}

void printStart(){
  // print hint to skip demo mode
  tft.setFont(DroidSans_14);
  for(int i = 0; i < 3; i++){
    tft.fillRect(FIELD_X, 110, FIELD_WIDTH*PIX, 60, GAME_BG);
    delay(800);
    tft.setCursor(FIELD_X+24, 120);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("Press any button");
    tft.setCursor(FIELD_X+64, 140);
    tft.print("to start"); 
    delay(800);
  }
}
void printGameOver(){
  // print game over when game ends
  tft.setFont(BlackOpsOne_40);
  tft.fillRect(FIELD_X, 120, FIELD_WIDTH*PIX, 40, GAME_BG);
  tft.fillRect(FIELD_X, 170, FIELD_WIDTH*PIX, 40, GAME_BG);
  int t = millis();
  unsigned cofs = 1;
  do {
    tft.setCursor(FIELD_X+10, 120);
    printColorText("GAME", cofs, 4);
    tft.setCursor(FIELD_X+20, 170);
    printColorText("OVER", cofs, 4);
    if (++cofs > NUMCOLORS-1) cofs = 1;
    delay(30);
  } while (millis()-t < 2000);
}

void countDown(){
  // count down before the start of the game
  tft.setFont(BlackOpsOne_72);
  for (int i = 3; i > 0; i--) {
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
  initTetrisField();
}

void gameloop(){
  // infinite game loop
  while(true){
    bool r = 0;
    while(!r){
      r = tetrisGame(true);
      // demo mode
    }
    tetrisGame(false);
    // game
  }
}

char joystickControls(){
  // capture player's operations
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  int joyButton = digitalRead(JOY_BTN);
  int commonButton = digitalRead(BTN);
  static bool joyHasClicked = false;
  static bool hasClicked = false;

  // rotate
  bool joyIsClicked = (joyButton == LOW);
  if (joyIsClicked){
    delay(50);
    joyIsClicked = (joyButton == LOW);
  }
  joyHasClicked = joyHasClicked && joyIsClicked;
  if (!joyHasClicked && joyIsClicked){
    joyHasClicked = true;
    return ('+');
  }

  // drop
  bool isClicked = (commonButton == HIGH);
  if (isClicked){
    delay(50);
    isClicked = (commonButton == HIGH);
  }
  hasClicked = hasClicked && isClicked;
  if (!hasClicked && isClicked){
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

bool tetrisGame(bool demoMode){
  // game loop 
  bool gameOver = false;
  int tk = 0;

  score = 0;
  initTetrisField();
  if (!demoMode) 
    countDown();

  nextBlock();
  preview();
  drawBlock(aBlock, aX, aY, aRotation, aColor);

  do {
    // Serial.println(timeInterval);
    int t = millis();
    if (!demoMode) do {  // process control
      if (millis() - tk > 100) {
        char ch = joystickControls();
        if (ch != '\0') tk = millis();
        switch (ch) { 
          case '+' : // rotate
              if (checkMoveBlock(0,0,1)) {
                  int oldaRotation = aRotation;
                  aRotation = (aRotation + 1) & 0x03;
                  drawBlockEx(aBlock, aX, aY, aRotation, aColor, aX, aY, oldaRotation);
              }
              break;
          case 's' : // down
              t = 0;
              break;
          case 'a' : // left
              if (checkMoveBlock(-1,0,0)) {
                  drawBlockEx(aBlock, aX + (-1), aY, aRotation, aColor, aX, aY, aRotation);
                  aX--;
              }
              break;
          case 'd' : // right
              if (checkMoveBlock(1,0,0)) {
                  drawBlockEx(aBlock, aX + 1, aY, aRotation, aColor, aX, aY, aRotation);
                  aX++;
              }
              break;
          case 'q' : // drop
              bool movePossible = checkMoveBlock(0, 1, 0);
              while (movePossible){
                  drawBlockEx(aBlock, aX, aY + 1, aRotation, aColor, aX, aY, aRotation);
                  aY++;
                  movePossible = checkMoveBlock(0, 1, 0);
              }
              score += 5;
              break; 
        }
      }
    } while (millis() - t < timeInterval);  // process joystickControls end

    else { 
      // demoMode
      delay(20);
      char ch = joystickControls();
      if (ch != '\0')  return true;
    }

    //move the block down
    bool movePossible = checkMoveBlock(0,1,0);
    if (movePossible){
        drawBlockEx(aBlock, aX, aY + 1, aRotation, aColor, aX, aY, aRotation);
        aY++;
    }

    else {
      // block stopped moving down
      // store location
      setBlock();
      checkLines();
      printGameLevel(timeInterval);
      // get new block and draw it
      score += 5;
      nextBlock();
      preview();
      drawBlock(aBlock, aX, aY, aRotation, aColor);
      if (!checkMoveBlock(0,0,0)) {
        // game over
        initTetrisField();
        gameOver = true;
        delay(200);
      }
    }

    if (!demoMode){
      printScore();
    }

  } while(!gameOver);

  if (demoMode && gameOver){
    printStart();
  }

  if (!demoMode){
    // print score if not in demo mode
    if (score > highscore){
        highscore = score;
        printHighScore();
    }
    printGameOver();
  }
  preview();
  return false;
}

void setBlock(){
  // set block to background
  int bH = BLOCKHEIGHT(aBlock, aRotation);
  int bW = BLOCKWIDTH(aBlock, aRotation);

  for (int y = 0; y < bH; y++){
    for (int x = 0; x < bW; x++){
      if (block[aRotation][aBlock][y * 4 + x + 2] > 0){
        field[x + aX][y + aY] = aColor;
      }
    }
  }
}

void checkLines(){
  // check if a line is completed or not
  int x, y, c, i;
  for (y = 0; y < FIELD_HEIGHT; y++){
    c = 0;
    for (x = 0; x < FIELD_WIDTH; x++){
      if (field[x][y] > 0) 
        c++;
    }

    if (c >= FIELD_WIDTH) {
      // a completed line
      for (i = NUMCOLORS-1; i >= 0; i--){
        for (x = 0; x < FIELD_WIDTH; x++){
          drawBlockPix(FIELD_X + x*PIX, FIELD_Y + y*PIX, i);
        }
        delay(60);
      }

      // move entire field above complete line down and redraw
      for (i = y; i > 0; i--){
        for (x = 0; x < FIELD_WIDTH; x++){
          field[x][i]=field[x][i-1];
        }
      }
      for (x = 0; x < FIELD_WIDTH; x++){
          field[x][0]=0;
      }
      score += 30; // bonus
      drawTetrisField();
      // increase speed as more lines are removed
      if (timeInterval > minTimeInterval) 
        timeInterval -= 5;
    }
  }
}

bool checkMoveBlock(int dX, int dY, int dRotation){
  // check if a block could keep moving or not
  int rot = (aRotation + dRotation) & 0x03;
  int bH = BLOCKHEIGHT(aBlock, rot);
  int bW = BLOCKWIDTH(aBlock, rot);
  int bX = aX + dX;
  int bY = aY + dY;

  if (bY + bH > FIELD_HEIGHT) return false; // touches ground

  if (bX < 0 || bX + bW > FIELD_WIDTH) return false; // left/right border

  for (int y = bH - 1; y >= 0; y--){
    for (int x = 0; x < bW; x++){
      if ((field[x + bX][y + bY] > 0) && (block[rot][aBlock][y * 4 + x + 2] > 0)){
        return false;
      }
    }
  }

  return true;
}

void nextBlock(){
  // randomly generate a colored block from block array
  // x position is also random, but y position is set
  // to 0 as default
  aColor = nColor;
  aBlock = nBlock;
  aRotation = nRotation;
  aY = 0;
  aX = random(FIELD_WIDTH - BLOCKWIDTH(aBlock, aRotation) + 1);
  nColor = random(1,NUMCOLORS);
  nBlock = random(NUMBLOCKS);
  nRotation = random(4);
}

void drawBlockPix(int px, int py, int col){
  // parent method
  // draw blocks with shade
  if (px >= FIELD_XW) return;
  if (px <  FIELD_X) return;
  if (py >= FIELD_YW) return;
  if (py <  FIELD_Y) return;

  if (col == 0){
    //remove Pix, draw backgroundcolor
    tft.fillRect(px, py, PIX, PIX, color[col]);
    return;
  }
  const int w=4;
  tft.fillRect(px+w, py+w, PIX-w*2+1, PIX-w*2+1, color[col]);
  for (int i = 0; i < w; i++){
    tft.drawFastHLine(px + i, py + i, PIX - 2*i, color_gamma[0][col]);
    tft.drawFastHLine(px + i, PIX + py - i - 1, PIX - 2*i, color_gamma[1][col]);
    tft.drawFastVLine(px + i, py + i , PIX - 2*i, color_gamma[2][col]);
    tft.drawFastVLine(px + PIX - i - 1, py + i, PIX - 2*i, color_gamma[2][col]);
  }
}

void drawBlock(int blocknum, int px, int py, int rotation, int col){
  // draw blocks on the screen
  int w = BLOCKWIDTH(blocknum, rotation);
  int h = BLOCKHEIGHT(blocknum, rotation);

  for (int x = 0; x < w; x++){
     for (int y = 0; y < h; y++){
       if (block[rotation][blocknum][y*4 + x + 2])
          drawBlockPix(FIELD_X + px*PIX + x*PIX, FIELD_Y + py*PIX + y*PIX, col);
       }
  }
}

void drawBlockPixSmall(int px, int py, int col){
  // draw small blocks for preview
  const int w=2;
  tft.fillRect(px + w, py + w, PIXSMALL - w*2 + 1, PIXSMALL - w*2 + 1, color[col]);
  for (int i = 0; i < w; i++){
   // shade
    tft.drawFastHLine(px + i, py + i, PIXSMALL - 2*i , color_gamma[0][col]);
    tft.drawFastHLine(px + i, PIXSMALL + py - i - 1 , PIXSMALL - 2*i , color_gamma[1][col]);
    tft.drawFastVLine(px + i, py + i , PIXSMALL - 2*i , color_gamma[2][col]);
    tft.drawFastVLine(px + PIXSMALL - i - 1, py + i , PIXSMALL - 2*i , color_gamma[2][col]);
  }
}

void preview(){
  // let users preview the upcoming block
  tft.setCursor(FIELD_XW+5,250);
  tft.setFont(DroidSans_9);
  tft.setTextColor(ILI9341_WHITE); 
  tft.print("Next:");
  tft.fillRect(FIELD_XW + 4, 270, PIXSMALL*4, PIXSMALL*4, SCREEN_BG);

  for (int x=0; x<4; x++){
    for (int y=0; y<4; y++){
     if (block[nRotation][nBlock][y*4 + x + 2])
        drawBlockPixSmall(FIELD_XW + 4 + x*PIXSMALL, 270 + y*PIXSMALL, nColor);
    }
  }

}

static uint8_t dbuf[FIELD_WIDTH][FIELD_HEIGHT] ={0};
void drawBlockEx(int blocknum, int px, int py, int rotation, int col, int oldx, int oldy, int oldrotation){
  // set placed tetris to the background
  int x, y;
  int w = BLOCKWIDTH(blocknum, oldrotation);
  int h = BLOCKHEIGHT(blocknum, oldrotation);

  for (x = 0; x < w; x++){
     for (y = 0; y < h; y++){
       if (block[oldrotation][blocknum][y*4 + x + 2]>0) 
          dbuf[x + oldx][y + oldy] = 2;
     }
  }

  w = BLOCKWIDTH(blocknum, rotation);
  h = BLOCKHEIGHT(blocknum, rotation);
  for (x = 0; x < w; x++)
     for (y = 0; y < h; y++)
       if (block[rotation][blocknum][y*4 + x + 2]>0) dbuf[x + px][y + py] = 1;

  for (y = FIELD_HEIGHT-1; y >= oldy; y--){
    for (x = 0; x < FIELD_WIDTH; x++){
      switch(dbuf[x][y]){
        case 1:  
          drawBlockPix(FIELD_X + x*PIX, FIELD_Y + y*PIX, col); 
          dbuf[x][y]=0;
          break;
        case 2:  
          tft.fillRect(FIELD_X + x*PIX, FIELD_Y + y*PIX, PIX, PIX, color[0]); 
          dbuf[x][y]=0; 
          break;
      }
    }
  }
}

void drawTetrisField(){
  // draw the black background for tetris except those placed tetris
  for (int y = FIELD_HEIGHT-1; y >= 0; y--){
    for (int x = 0; x < FIELD_WIDTH; x++){
      drawBlockPix(FIELD_X + x*PIX, FIELD_Y + y*PIX, field[x][y]);
    }
  }
}

// artwork
uint16_t colgamma(int16_t color, int16_t gamma){
  return  tft.color565(
    constrain(((color>>8) & 0xf8) + gamma,0,255),  //r
    constrain(((color>>3) & 0xfc) + gamma,0,255),  //g
    constrain(((color<<3) & 0xf8) + gamma,0,255)); //b
}

void printColorText(const char *txt, unsigned colorOffset, int len) {
  unsigned col = colorOffset;
  for (int i = 0; i < len; i++){
    tft.setTextColor(color[col]);
    tft.print(*(txt+i));
    if (++col > NUMCOLORS - 1) col = 1; 
  }
}
