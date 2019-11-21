#include "graphics2d_api.h"
// #include "../../fatfs/ff.h"

//static FATFS fs;
// static DIR dir;
// static FILINFO finfo;
// static FRESULT fr;

void init() {
  clip_r();
  writemask(false);
  usemask(false);
  //  fr = f_mount(&fs, "", 1);
  
}

void update() {
  T = (T+1)%300;
  
}

void *draw() {
  alpha = 255;
  bcolor = palette + 6;
  cls();
  color = palette + 8;
  rectdb(50, 50, 60, 40, 2, 3, 5);
  RGB m = {200,200,200};
  color = &m;
  alpha = 100;
  rectdb(130, 50, 60, 40,  2, 3, 5);
  ellipseb(T-100, 130, 28, 60, 1);
  alpha = 255;
  text_char(100, 100, 'a');
  color = palette+14;
  alpha = 100;
  line_aa(40, 40, 100, 120);
  line(40, 60, 100, 140);
  circr_aa(100, 80, 50);
  circr(100+10, 80, 50);

  
  return (void *)buf;
}

