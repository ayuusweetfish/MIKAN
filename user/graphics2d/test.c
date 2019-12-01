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
  color = palette + 15;
  float A[3][3] = {{ 20,  10, 1},
		   { 30, 100, 1},
		   {100, 200, 1}};
  alpha = 100;
  printf("BEFORE\n");
  printf("%2.2f, %2.2f, %2.2f\n", A[0][0], A[0][1], A[0][2]);
  printf("%2.2f, %2.2f, %2.2f\n", A[1][0], A[1][1], A[1][2]);
  printf("%2.2f, %2.2f, %2.2f\n", A[2][0], A[2][1], A[2][2]);
  // trib(A[0][0], A[0][1], A[1][0], A[1][1], A[2][0], A[2][1]);
  color = palette +9;

  affine_transform_2d_init( 20, 10, 30, 100, 100, 200,
			    20, 5, 15, 50, 50, 100);
  transform_2d(A, transform_2d_matrix, 3);
  printf("AFTER\n");
  printf("%2.2f, %2.2f, %2.2f\n", A[0][0], A[0][1], A[0][2]);
  printf("%2.2f, %2.2f, %2.2f\n", A[1][0], A[1][1], A[1][2]);
  printf("%2.2f, %2.2f, %2.2f\n", A[2][0], A[2][1], A[2][2]);
  //trib((int)(A[0][0]+0.5f), (int)(A[0][1]+0.5f), (int)(A[1][0]+0.5f), (int)(A[1][1]+0.5f), (int)(A[2][0]+0.5f), (int)(A[2][1]+0.5f));
  line(15,50,20,5);
  return (void *)buffer;
}
