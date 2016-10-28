#include "myy.h"
#include "init_window.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720 

int main() {

  CreateWindowWithEGLContext("Nya !", WINDOW_WIDTH, WINDOW_HEIGHT);
  myy_init(WINDOW_WIDTH, WINDOW_HEIGHT);

  while(!UserInterrupt()) {
    myy_draw();
    RefreshWindow();
  }

}
