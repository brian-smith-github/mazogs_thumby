// MAZOGS - A MAZE ADVENTURE GAME
// Ported from the ZX81 '82 code to Arduboy over a rainy November '15 week.
// Further porting to Thumby over a rainy Oct '21 week.
// Original version by Don Priestley. Bonsai porting by Brian Smith.

#include "defines.h"
#include "attinyarcade.h"
#include "io.c"
#include "create_maze.c"
#include "solve_maze.c"
#include "text_bits.c"

// globals
int random8bit;
int counter = 1;
int counter2 = 70;

unsigned char maze[1408]; // 64x44/2 (1 nibble per maze tile)

int  xfd;   // File Descriptor for the X display
int pose; // character posture appearance
int frame = 0; // global animation frame bit
int move_frame = 0; // character movement frame bit
int carrying; // defines what player is carrying, e.g. sword
int level = 1;
int moves_left;
int initial_moves;
int kill_moves;
unsigned int maze_number; // which maze is being played

//-----------------------------------------------------------
// write 'THIS WAY' tiles to map...
void thisway(int posn)
{
  clear_trails();
  if (carrying == HAVE_TREASURE) {
    write_maze(posn, TREASURE); // put missing treasure back in the map
    solve_maze(HOME);
    write_maze(posn, TRAIL); // then revert back to trail
  } else solve_maze(posn);
  clear_badsearches();
  frame = 50; // set countdown for thisway removal
}


//---------------------------------------------------------------
// do the 13x13 map view in-game
int view_map(int posn)
{
  int x, y, i, tile;

  // charge 10 moves..... (original says 15 for level2, source disagrees)
  if (level > 1 && moves_left > 11) moves_left -= 10;
  grey_screen();
  for (i = 30; i > 0; i--) { // forced to view map for 30 frames
    draw_maptiles(posn, 6, 13);
    display_display();
    if (level == 3) {
      move_mazogs(posn);
      if (read_maze(posn) == MAZOG) return (1); // fight!
    }
    delay(200);
  }
  return (0);
}

//----------------------------------------------------------
// routine to review entire map at the end of a game...
int explore_map()
{
  int x = 0, y = 0, i;
  int posn = 816;
  int tile;
  int keypress = 1;


  while (read_keys() != 0); // wait for no keypresses
  black_screen();
  draw_maptiles(posn, 0, 24);
  print(0, 24, "DPAD TO MOVE", 1);
  print(0, 32, "B = NEW GAME", 1);
  display_display();
  while (read_keys() == 0); // wait for keypress
  while (read_keys() == KEY_B);
  while (1) {
    // draw map at current position...
    draw_maptiles(posn, 0, 24);
    display_display();
    delay(100);
    keypress = read_keys();
    if (keypress == KEY_UP && posn > 63) posn -= 64;
    if (keypress == KEY_DOWN && posn < 2650) posn += 64;
    if (keypress == KEY_LEFT && posn > 0) posn--;
    if (keypress == KEY_RIGHT && posn < 2650) posn++;
    if (keypress == KEY_B) {
      while (read_keys());
      return (0);
    }
  }
}

//-----------------------------------------------------------------
// Mazog move routines coming up....

int can_move(int posn)
{
  int tile;

  tile = read_maze(posn);
  if (tile == CLEAR || tile == TRAIL || tile == THISWAY) return (1);
  return (0);
}

//-----------------------------------------------------------------
void move_mazog(int posn, int newposn)
{
  write_maze(posn, CLEAR);
  write_maze(newposn, MAZOG);
}

//------------------------------------------------------------------
int move_mazogs(int posn)
{
  int i, dir;

  for (i = 128; i < 2944; i++) // scan whole maze for mazogs

    if (read_maze(i) == MAZOG) {
      // always pick a fight if right or left of player...
      if (i == posn + 1 || i == posn - 1) {
        move_mazog(i, posn);
        return (0); // start the fight.
      }
      dir = random_direction();

      // up.... (note that mazogs don't attack face-on)
      if (dir == 1 && can_move(i - 64) && i - 64 != posn) move_mazog(i, i - 64);

      // left.....
      if (dir == 3 && can_move(i - 1)) move_mazog(i, i - 1); // left

      // now it gets tricky - avoid re-scanning same mazog twice

      // right....
      if (dir == 4 && can_move(i + 1)) {
        move_mazog(i, i + 1);
        i++;
      }

      // down.... (note that mazogs don't attack face-on)
      if (dir == 2 && can_move(i + 64) && i + 64 != posn) {
        move_mazog(i, i + 64);
        i += 64;
      }
    }
  return (0);
}

//-----------------------------------------------------------------
// draw 5x5 big-tile maze at current position...
void draw_maze(int posn)
{
  int x, y, a, tile;

  for (y = 0; y < 5; y++) for (x = 0; x < 5; x++) {
      tile = read_maze(posn + y * 64 + x - 130);
      if (x == 2 && y == 2) tile = pose; // put character in center of screen
      // alternate 'THIS' and 'WAY' tiles...
      if (tile == THISWAY && ((x + y + posn) % 2) == 1) tile = WAY;
      // this does the animations...
      if (frame & 1 && tile == MAZOG) tile = MAZOG2;
      if (frame & 1 && tile == TREASURE) tile = TREASURE2;
      if (frame & 1 && tile == PRISONER) tile = PRISONER2;


      draw14x8tile(tile, x, y);
    }
  display_display();
}

//----------------------------------------------------------
// Process the next player move.
int check_move(int posn, int newposn)
{
  int tile;

  tile = read_maze(newposn);
  if (tile == WALL) { // can't walk though walls...
    pose = STILL + carrying; move_frame = 0;
    return (posn);
  }
  else if (tile == SWORD) { // grab sword, or swap with treasure
    if (carrying == HAVE_NOTHING) write_maze(newposn, WALL);
    else if (carrying == HAVE_TREASURE) write_maze(newposn, TREASURE); // swap
    carrying = HAVE_SWORD;
    pose = STILL + carrying; move_frame = 0;
    return (posn);
  }
  else if (tile == PRISONER) { // get directions.
    if (level == 3) write_maze(newposn, WALL);
    thisway(posn);
    return (posn);
  }
  else if (tile == TREASURE) { // grab treasure, or swap with sword
    if (carrying == HAVE_NOTHING) write_maze(newposn, WALL);
    else if (carrying == HAVE_SWORD) write_maze(newposn, SWORD); // swap
    carrying = HAVE_TREASURE;
    pose = STILL + carrying; move_frame = 0;
    return (posn);
  }
  else if (tile == MAZOG) { // fight the mazog
    write_maze(posn, TRAIL); // leave a trail
    posn = newposn;
    if (fight(posn)) return (posn + 10000); // fight sequence, exit if lost.
    if (carrying == HAVE_SWORD) carrying = HAVE_NOTHING;
  }

  // just movement then, first check there are enough moves left...
  if (level > 1) {
    moves_left--; if (moves_left == 0) // starved...
    {
      write_maze(posn, MAP_DEAD);
      return (30000);
    }
  }
  // now set pose for movement direction then....
  if (newposn - posn == -64) pose = UP + carrying + move_frame;
  if (newposn - posn == 64) pose = DOWN + carrying + move_frame;
  if (newposn - posn == -1) pose = LEFT + carrying + move_frame;
  if (newposn - posn == 1) pose = RIGHT + carrying + move_frame;
  // leave a trail...
  write_maze(posn, TRAIL);

  move_frame = !move_frame;
  return (newposn);
}
//----------------------------------------------------------
// fight routine. returns 0 for player win or 'posn' if lost.
int fight(int posn)
{
  int i, n;

  if (level > 1) moves_left += kill_moves;

  for (i = 0; i < 6; i++)
    for (n = 0; n < 6; n++) {
      // (avoid running down the 'thisway' timer...)
      if (n % 2 == 0) {
        pose = FIGHT + n / 2;
        frame = frame ^ 1;
      }
      else pose = MAZOG;
      draw_maze(posn);
      delay(100);

    }
  write_maze(posn, CLEAR);
  if (carrying == HAVE_SWORD) {
    carrying = HAVE_NOTHING;
    pose = STILL;
    return (0);
  }
  // fighting without a sword... roll D20...
  update_random();
  if (random8bit > 127) {
    pose = STILL;  // phew!
    return (0);
  }
  return (posn); // RIP
}


//----------------------------------------------------------
// This is the main game-play loop. Exits at end of a game.
int enter_maze(int start_posn)
{
  int posn = start_posn;
  int k;

  grey_screen();
  while (1) {
    if (level == 3) move_mazogs(posn);
    if (read_maze(posn) == MAZOG) if (fight(posn)) return (posn); // fight result
    draw_maze(posn);
    delay(200);
    k = read_keys();
    if (k == KEY_UP) posn = check_move(posn, posn - 64);
    else if (k == KEY_DOWN) posn = check_move(posn, posn + 64);
    else if (k == KEY_LEFT) posn = check_move(posn, posn - 1);
    else if (k == KEY_RIGHT) posn = check_move(posn, posn + 1);
    else pose = STILL + carrying;

    if (posn == 30000) return (30000); // starved.
    if (posn > 10000) return (posn - 10000); // lost a fight!

    if (k == KEY_B) situation_report(posn);
    if (k == KEY_A)
      if (view_map(posn)) // a fight started when viewing map
        if (fight(posn)) return (posn); // check fight outcome.

    if (frame > 1) {
      frame--;  //remove thisway
      if (frame == 1) clear_trails();
    }
    else frame = !frame;

    if (posn == HOME && carrying == HAVE_TREASURE) return (0); // a winner is you
  }
  return (1);
}

//----------------------------------------------------------
int _main()
{
  while (1) loop();
}

void setup()
{
  core_init();
  display_init(0, 0, 0);
  arduboy_screen_wipe();

  //_main();
}

//----------------------------------------------------------
void loop()
{
  int posn;

  maze_number = micros() % 65536; // seed RNG
  initial_moves = 0; moves_left = 0; carrying = HAVE_NOTHING; pose = STILL;
  title_sequence();
  choose_level();
  level_splash();

  left_or_right();
  situation_report(HOME);
  if (level > 1) situation_report2();

  posn = HOME;
  posn = enter_maze(posn); // begin the game, process outcome...
  if (posn == 30000) starved();
  else if (posn != 0) mazogs_win(posn);
  else welcome_back();

  maybe_examine_maze();
}
