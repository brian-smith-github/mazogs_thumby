#include <stdio.h>
#include "defines.h"

extern int explore_map();
extern void draw_maze(int posn);
extern void create_maze();
extern int read_maze();
extern int write_maze(int posn, int value);
extern void solve_maze();
extern int get_distance();
extern void move_treasure();
extern void clear_trails();


extern void draw14x8tile(int tile,int x, int y);
extern void print();
extern void prints();
extern void draw_maptiles();
extern void black_screen();
extern void white_screen();
extern void grey_screen();
extern void display_display();
extern int read_keys();
extern void delay();

extern int level,carrying,initial_moves,moves_left,kill_moves,frame,pose;
extern unsigned int maze_number;

//----------------------------------------------------------
// runs the 'attract' opening screen
int title_sequence()
{
  int i,x,y,frame=0;

  //while(read_keys()); // wait till no keys pressed...
  
  // static screen elements...
  grey_screen();
  for (y=1; y<5; y++) for (x=0; x<5; x++) draw14x8tile(WALL,x,y);
  draw14x8tile(STILL+HAVE_TREASURE,0,1);
  draw14x8tile(TRAIL,0,2);
  draw14x8tile(TRAIL,1,2);
  draw14x8tile(RIGHT+HAVE_SWORD,2,2);
  draw14x8tile(THISWAY,1,3);
  draw14x8tile(WAY,1,4);
  
  draw14x8tile(SWORD,3,3);
 
  get_b: 

  //draw_bitmap(mazogs_bmp, 256+frame%6*128+100,28,frame%12<6);
  if (frame%20==0)  {print(0,0,"   MAZOGS   ",0);};
  if (frame%20==10) {print(0,0,"  PRESS 'B' ",1);};
  // animation loop...
  if (frame&1) {
    draw14x8tile(TREASURE2,1,1);
    draw14x8tile(MAZOG2,3,2);
    draw14x8tile(PRISONER,2,3);
  } else {
    draw14x8tile(TREASURE,1,1);
    draw14x8tile(MAZOG,3,2);
    draw14x8tile(PRISONER2,2,3);  
  }
  frame++;
  display_display();
  for (i=0; i<100; i++) {
    if (read_keys()==KEY_B) return(0);
    delay(2);
  }
  goto get_b;  
}

//----------------------------------------------------------
int choose_level()
{
  int x,y,i;
  
  while(read_keys());
  
  black_screen();
  for (y=1; y<4; y++) for (x=0; x<5; x++) draw14x8tile(WALL,x,y);
  draw14x8tile(MAZOG2,2,2);
  print(0,0,"WHICH GAME?",0);
  display_display();

  choose_loop:
  i=read_keys();
  if ((i==KEY_UP || i==KEY_LEFT) && level>1) level--;  // up
  if ((i==KEY_DOWN || i==KEY_RIGHT) && level<3) level++;  // down
  
  if (level==1) print(0,32," TRY IT OUT ",1);
  if (level==2) print(0,32,"A CHALLENGE ",1);
  if (level==3) print(0,32,"MANIC MAZOGS",1);
  if (i==KEY_B) return(0);  // B key
  display_display();
  
  while(read_keys());
  while(!read_keys());
  
  goto choose_loop;     
}

//----------------------------------------------------------
void level_splash()
{
  int x,y,i;
  
  black_screen();
  while(read_keys());
  
  for (y=1; y<5; y++) for (x=0; x<5; x++) draw14x8tile(WALL,x,y);
  
  if (level==1) {
    draw14x8tile(LEFT+HAVE_TREASURE,2,2);
    print(0,0," TRY IT OUT ",0);
  }
  if (level==2) {  
    draw14x8tile(RIGHT+HAVE_SWORD,2,2);
    print(0,0,"A CHALLENGE ",0);
  }
  if (level==3) {
    draw14x8tile(STILL,2,2);
    print(0,0,"MANIC MAZOGS",0);
    for (i=1; i<6; i++) {
      draw14x8tile(MAZOG,1,2);
      draw14x8tile(MAZOG,3,2);
      display_display();
      delay(200);
      draw14x8tile(MAZOG2,1,2);
      draw14x8tile(MAZOG2,3,2);
      display_display();
      delay(200);
    }
  }
  // print instructions...
  print(0,32,"DRAWING MAZE",1);
  display_display();
  create_maze();
  
  delay(500); // remove if too slow...
  print(0,32,"READY, HIT B",1);
  display_display(); 
  while(read_keys()!=KEY_B);    
}

//----------------------------------------------------------

void pick_maze()
{
  char s[5];
  int i,p=4, p10=1;
   
  grey_screen();
  print(0,0,"MAZE CHOOSER",1);
  print(0,8,"B = FINISHED",0);
  loop: 
  sprintf(s,"%.5u",maze_number);
  print(8,24,s,0);
  sprintf(s,"     "); s[p]='V';
  print(8,16,s,0);
  display_display();
  i=read_keys();
  if (i==KEY_LEFT && p>0) {p--; p10*=10;}
  if (i==KEY_RIGHT && p<4) {p++; p10/=10;}
  if (i==KEY_UP && maze_number<=65535-p10) maze_number+=p10;
  if (i==KEY_DOWN && maze_number>p10) maze_number-=p10;
  if (i==KEY_B) goto exit;
  while(read_keys());
  goto loop;

  exit:
  print(0,32,"REDRAW...",0);
  create_maze();
}

//-----------------------------------------------------------
void left_or_right()
{
  int x,y,tile,distance;
 
  start:
  grey_screen();

  draw_maptiles(HOME,0,13);
  
  print(42, 0,"WHICH",1);
  print(42, 8,"WAY? ",1);
  print(42,16,"LEFT ",1);
  print(42,24,"OR   ",1);
  print(42,32,"RIGHT",1);
  display_display();
  
  while(read_keys());
 
  leftright:
  x=read_keys();
  if (x==KEY_LEFT) {write_maze(HOME+1,WALL); goto chosen; } // left
  if (x==KEY_RIGHT) {write_maze(HOME-1,WALL); goto chosen; } // right
  if (x==KEY_A) {pick_maze(); goto start;}
  goto leftright;
  chosen:
  black_screen();
  
  display_display();
  
  delay(100);
  grey_screen();
  draw_maptiles(HOME,0,13);
  
  check_loop:
  solve_maze(HOME);
  distance=get_distance();
  if (distance<100) { // the game would be too easy...
    move_treasure();
    goto check_loop;
  }
  clear_trails();
  
  print(42, 0,"PRESS",1);
  print(42, 8," 'B' ",1);
  print(42,16," FOR ",1);
  print(42,24,"STATS",1);
  
  display_display();
  while(read_keys()!=KEY_B);
}

//----------------------------------------------------------
int situation_report(int posn)
{
  int i;
  char s[40];
  
  white_screen();
  print(0,0,"   STATUS:  ",1);
  display_display();
  if (carrying==HAVE_TREASURE) {
    write_maze(posn,TREASURE);
    solve_maze(HOME);
    write_maze(posn,CLEAR);
    print(0,16,"BACK TO EXIT",0);
    i=get_distance();
    sprintf(s," %i MOVES",i);
    print(0,8,s,0);
    print(0,32,"'B' FOR GAME",0);   
  } else {
    solve_maze(posn);
    print(0,16,"TO TREASURE.",0);
    i=get_distance();
    sprintf(s," %i MOVES",i);
    print(0,8,s,0);
    if (moves_left==0) { // i.e. first report of game...
       moves_left=i*4;
       initial_moves=moves_left;
       if (level>1) print(0,32,"'B' FOR MORE",0); // second report due....
       else print(0,32,"'B' FOR GAME",0);
    }
    else print(0,32,"'B' FOR GAME",0);
  }
  if (level>1) {
      sprintf(s,"%i LEFT.  ",moves_left);
      print(0,24,s,0);
      if (carrying==HAVE_NOTHING && posn!=HOME) {
        print(0,32,"A=BUY SWORD ",0);
      }
  }
  clear_trails();
  display_display();
  
  // charge 10 moves.....
  if (level>1 && moves_left>11) moves_left-=10;
  
  while(read_keys());
  
  get_ab:
  // 'A' key, buy a sword? Costs half of remaining moves :o
  if (read_keys()==KEY_A && level>1 && carrying==HAVE_NOTHING) {
    carrying=HAVE_SWORD;
    moves_left=moves_left/2+1;
    while(read_keys()==KEY_A);
    grey_screen();
    return(0); 
  }
  if (read_keys()==KEY_B) {grey_screen(); return(0);}
  goto get_ab;  
}

//----------------------------------------------------------
int situation_report2()
{
  int i;
  char s[30];
  
  white_screen();
  print(0,0,"STATUS PT 2 ",1);
  display_display();
  
  if (level==2) kill_moves=initial_moves/10;
  if (level==3) kill_moves=initial_moves*15/100;
  sprintf(s,"A KILL: +%i",kill_moves);
  print(0,8,s,0);
  print(0,16,"VIEW MAP:-10",0);
  print(0,24,"STATUS: -10",0);
 
  print(0,32,"'B' FOR GAME",0);
  display_display();

  while(read_keys());
  
  while(read_keys()!=KEY_B);
  return(0);  // B key
}

//-----------------------------------------------------------
void starved()
{
  int i;
  
  black_screen();
  // cursor set to center
  frame=0;
  for (i=0; i<20; i++) {
     print(8,0,"YOU HAVE ",frame);
     print(8,8,"STARVED ",frame);
     print(8,16,"TO DEATH",frame);
     display_display();
     frame=!frame;
     delay(200);
  }
}

//---------------------------------------------------------
void mazogs_win(int posn)
{
  int i;
  
  pose=MAZOG; write_maze(posn,MAZOG);
  frame=0;
  for (i=0; i<20; i++) {
    draw_maze(posn);
    frame=!frame;
    delay(100);
  }
  black_screen(); display_display();
  delay(100);
  for (i=0; i<20; i++) {
   draw_maze(posn);
   // cursor set
   print(0,24,"DEATH TO ALL",frame);
   print(0,32,"   HUMANS   ",frame);
   display_display();
   frame=!frame;
   delay(200);
  } 
  grey_screen();
}

//----------------------------------------------------------
void welcome_back()
{
  int i,posn;
  char s[30];
  
  if (read_maze(HOME-1)==WALL) posn=HOME+1; else posn=HOME-1;
  
  write_maze(HOME-64,EXIT);
  write_maze(HOME,STILL); // reception party.
  
  pose=STILL+HAVE_TREASURE;
  
  for (i=0; i<30; i++) {
    draw_maze(posn);
    print(0,32,"WELCOME BACK",frame);
    display_display();
    frame=!frame;
    delay(200);
  }
  write_maze(HOME,MAP_STILL);
  
 white_screen();
  if (level>1) { // report the score.
    sprintf(s,"%i ALLOWED.",initial_moves);
    print(0,0,s,0);
    sprintf(s,"%i LEFT.",moves_left);
    print(0,8,s,0);
    sprintf(s,"SCORE=%i%%",moves_left*100/initial_moves);
    print(0,16,s,0);
  }
}

//----------------------------------------------------------
int maybe_examine_maze()
{
  // cursor set to bottom of screen 
  print(0,24,"A=VIEW MAZE ",0);
  print(0,32,"B=PLAY AGAIN",0);
  display_display();
  get_ab:
  if (read_keys()==KEY_A) {explore_map(); return(0);}
  if (read_keys()==KEY_B) return(0);
  goto get_ab;  
}
