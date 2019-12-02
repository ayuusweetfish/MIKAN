#include "api.h"
#include "graphics2d_api.h"
#include <stdbool.h>


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
  color = palette + 15;
  float A[3][3] = {{ 20,  10, 1},
		   { 30, 100, 1},
		   {100, 200, 1}};
  alpha = 100;
  tri(A[0][0], A[0][1], A[1][0], A[1][1], A[2][0], A[2][1]);
  color = palette +9;
  shear_transform_2d_init(0.75f, 0, 20, 20, transform_2d_matrix);
  transform_2d(A, transform_2d_matrix, 3);
  tri(A[0][0], A[0][1], A[1][0], A[1][1], A[2][0], A[2][1]);
  return (void *)buffer;
}
