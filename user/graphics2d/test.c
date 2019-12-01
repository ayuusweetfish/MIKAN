#include "api.h"
#include "graphics2d_api.h"
#include <stdbool.h>
#include <stdio.h>

// Main Loop
void init() {
  b0 = b1 = T = 0;
  clip_r();
  writemask(false);
  usemask(false);
}

void update() {
  b1 = b0;
  b0 = buttons();
  T = (T + 1) % 5184000;
}

void *draw() {
  cls();
  trib(20, 10, 30, 100, 100, 200);
  return (void *)buffer;
}
