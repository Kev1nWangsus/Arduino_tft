#define PIX 16           // normal size
#define PIXSMALL 10      // preview size
#define FIELD_WIDTH 12   // the field contains 12 blocks in width
#define FIELD_HEIGHT 20  // and 20 blocks in height

#define FIELD_X  0
#define FIELD_Y  0

#define FIELD_XW (FIELD_X + FIELD_WIDTH*PIX)
#define FIELD_YW (FIELD_Y + FIELD_HEIGHT*PIX)

#define SIDE (FIELD_XW + 3) // for text printing

#define BLOCKWIDTH(blocknum, rotation) block[rotation][blocknum][0]
#define BLOCKHEIGHT(blocknum, rotation) block[rotation][blocknum][1]

const uint16_t color[] = {ILI9341_BLACK, ILI9341_MAGENTA, ILI9341_RED, ILI9341_ORANGE, ILI9341_YELLOW, ILI9341_GREEN, ILI9341_BLUE};
#define NUMCOLORS 7
#define NUMBLOCKS 7



const char block[4][NUMBLOCKS][2+4*4] =
{4,1,
 1,1,1,1,
 0,0,0,0,
 0,0,0,0,
 0,0,0,0, // long piece

 3,2,
 1,0,0,0,
 1,1,1,0,
 0,0,0,0,
 0,0,0,0, // left L

 3,2,
 0,1,0,0,
 1,1,1,0,
 0,0,0,0,
 0,0,0,0, // T piece

 3,2,
 0,0,1,0,
 1,1,1,0,
 0,0,0,0,
 0,0,0,0, // right L

 2,2,
 1,1,0,0,
 1,1,0,0,
 0,0,0,0,
 0,0,0,0, // square piece

 3,2,
 0,1,1,0,
 1,1,0,0,
 0,0,0,0,
 0,0,0,0, // zigzag right

 3,2,
 1,1,0,0,
 0,1,1,0,
 0,0,0,0,
 0,0,0,0, // zigzag left

 //-------
 
 1,4,
 1,0,0,0,
 1,0,0,0,
 1,0,0,0,
 1,0,0,0,

 2,3,
 1,1,0,0,
 1,0,0,0,
 1,0,0,0,
 0,0,0,0,

 2,3,
 1,0,0,0,
 1,1,0,0,
 1,0,0,0,
 0,0,0,0,

 2,3,
 1,0,0,0,
 1,0,0,0,
 1,1,0,0,
 0,0,0,0,

 2,2,
 1,1,0,0,
 1,1,0,0,
 0,0,0,0,
 0,0,0,0,

 2,3,
 1,0,0,0,
 1,1,0,0,
 0,1,0,0,
 0,0,0,0,

 2,3,
 0,1,0,0,
 1,1,0,0,
 1,0,0,0,
 0,0,0,0,

 //-------
 
 4,1,
 1,1,1,1,
 0,0,0,0,
 0,0,0,0,
 0,0,0,0,

 3,2,
 1,1,1,0,
 0,0,1,0,
 0,0,0,0,
 0,0,0,0,

 3,2,
 1,1,1,0,
 0,1,0,0,
 0,0,0,0,
 0,0,0,0,

 3,2,
 1,1,1,0,
 1,0,0,0,
 0,0,0,0,
 0,0,0,0,

 2,2,
 1,1,0,0,
 1,1,0,0,
 0,0,0,0,
 0,0,0,0,

 3,2,
 0,1,1,0,
 1,1,0,0,
 0,0,0,0,
 0,0,0,0,

 3,2,
 1,1,0,0,
 0,1,1,0,
 0,0,0,0,
 0,0,0,0,
 
 //-------
 
 1,4,
 1,0,0,0,
 1,0,0,0,
 1,0,0,0,
 1,0,0,0,

 2,3,
 0,1,0,0,
 0,1,0,0,
 1,1,0,0,
 0,0,0,0,

 2,3,
 0,1,0,0,
 1,1,0,0,
 0,1,0,0,
 0,0,0,0,

 2,3,
 1,1,0,0,
 0,1,0,0,
 0,1,0,0,
 0,0,0,0,

 2,2,
 1,1,0,0,
 1,1,0,0,
 0,0,0,0,
 0,0,0,0,

 2,3,
 1,0,0,0,
 1,1,0,0,
 0,1,0,0,
 0,0,0,0,

 2,3,
 0,1,0,0,
 1,1,0,0,
 1,0,0,0,
 0,0,0,0,
 };
