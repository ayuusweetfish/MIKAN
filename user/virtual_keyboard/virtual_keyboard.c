#include "graphics2d.h"
#include "graphics2d_api.h"

void init() {
  clip_r();
  writemask(false);
  usemask(false);
}

void update() {
  T = (T+1) % 500;
}

void *draw() {
  
  cls(palette+6);
  rectdb(50, 50, 60, 40, palette+8, 2, 3, 5);

  RGB m = {200,200,200};
  rectdb_a(130, 50, 60, 40, &m, 2, 3, 5, 100);

  ellipseb_a(T-100, 130, 28, 60, &m, 1, 120);

  round_rectb(T-100, 100, 50, 80, 8, palette+14, 5);
  return (void *)buf;
}

