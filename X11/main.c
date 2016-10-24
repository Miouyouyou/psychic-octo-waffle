#include "myy.h"
#include "init_window.h"

int main() {

  CreateWindowWithEGLContext("Nya !", 1920, 1080);
  myy_init(1920, 1080);

  while(!UserInterrupt()) {
    myy_draw();
    RefreshWindow();
  }

}
