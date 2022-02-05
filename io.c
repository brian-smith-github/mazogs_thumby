#include "defines.h"
#include "glcdfont.h"
#include "tiles14x8.h"
#include "tiles3x3.h"

extern const unsigned char tiles14x8[];
extern const unsigned char tiles3x3[];
extern int button_pins[];

extern int read_maze(int posn);

void display_display()
{
}

//---------------------------------------------------------------
// draw 3x3 map tiles
void draw_maptiles(int posn, int sx, int width)
{
   int x,y,i,tile,a,a2,b,b2,t,shift;

  display_init(1,sx*3,0);  // vertical mode, start at sx,0
   for (x=0; x<width; x++) {
     for (i=0; i<3; i++) { // draw 3 columns of tile
       shift=0; b=0;
       for (y=0; y<13; y++) {
         tile=read_maze(posn+y*64+x-520+130 );
         if (tile==WALL_GREY && (x+y)%2==0) tile=WALL_GREY2;
   if (sx==6 && x==6 && y==6) tile=MAP_STILL; // add player?
   a=pgm_read_byte(tiles3x3+tile*3+i);   
   a<<=shift;
         b|=a;
   if (shift>=5 || y==12) { // output to screen buffer and wrap
     // if (y==12 && buffpos%2==1) b|=128;
     i2c_sendByte(b);
     b=pgm_read_byte(tiles3x3+tile*3+i); b>>=(8-shift);
   }
   shift+=3; if (shift>=8) shift-=8;
       }
     }
   }
}

//--------------------------------------------------------------------
void draw14x8tile(int tile,int x, int y)
{
  int i;
  display_init(0,x*14+1,y);  // horizontal mode, start at 0,0
  for (i=0; i<14; i++) i2c_sendByte(pgm_read_byte(tiles14x8+tile*14+i));
}

//----------------------------------------------------------------------

void black_screen()
{
  int i;
  
  display_init(0,0,0);  // horizontal mode, start at 0,0
  for (i=0; i<72*5; i++) i2c_sendByte(0);
}

void white_screen()
{
  int i;
  
  display_init(0,0,0);  // horizontal mode, start at 0,0
  for (i=0; i<72*5; i++) i2c_sendByte(255);
}

void grey_screen()
{
  int i;
  
  display_init(0,0,0);  // horizontal mode, start at 0,0
  for (i=0; i<72*5; i++) 
     if (i%2==0) i2c_sendByte(0b10101010); else i2c_sendByte(0b01010101);
}

//-----------------------------------------------------------------------
void print(int x, int y, const char *s, int colour)
{
  
  int i;
  unsigned char c;
  
  display_init(0,x,y/8);  // horizontal mode, start at 0,0
  c=*s++; 
  while(c!=0) {
    if (colour==1) {
        for (i=0; i<5; i++) i2c_sendByte(pgm_read_byte(font+c*5+i));
        i2c_sendByte(0);
    } else {
         for (i=0; i<5; i++) i2c_sendByte(~pgm_read_byte(font+c*5+i));
        i2c_sendByte(255);
    }
    c=*s++; // next character in the string
  }
}

//---------------------------------------------------------------------------
int read_keys()
{
  if (digitalRead(button_pins[0])==0) return(KEY_UP);
  if (digitalRead(button_pins[1])==0) return(KEY_DOWN);
  if (digitalRead(button_pins[2])==0) return(KEY_LEFT);
  if (digitalRead(button_pins[3])==0) return(KEY_RIGHT);
  if (digitalRead(button_pins[4])==0) return(KEY_A);
  if (digitalRead(button_pins[5])==0) return(KEY_B);
  return(0);
}
