/*** 
 * @Author: xyhyx
 * @Date: 2025-12-07 21:21:18
 * @LastEditTime: 2025-12-07 21:21:20
 */
#include "act.h"

void random_act() {
  int random_num = random(100);
  int x = random(-25, 25);
  int y = random(-20, 45);

  if (random_num < 10) {
    eye_happy();
  } else if (random_num < 30) {
    head_move(x, y, 10);
    if (x > 0) {
      eye_right();
    } else {
      eye_left();
    }
  } else {
    eye_blink();
  }
  head_center();
}