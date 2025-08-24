#include <gba.h>
#include <random.h>

// TODO: Add current path to incdir
#include "init.h"

void show_title_screen() {
  #include <spritesheet_title_screen.inc>
  static auto const TILESET_PTR = (volatile u16_t*)(0x06000000 + 1 * 0x4000);
  for (u16_t i = 0; i < PALETTE_SIZE; ++i) {
    BG_PALETTE[i] = PALETTE[i];
  }
  for (u16_t i = 0; i < TILESET_SIZE; i++) {
    TILESET_PTR[i] = TILESET[i];
  }
  static auto const TILEMAP_BG0_PTR = (volatile u16_t*)(0x06000000 + 0 * 0x800);
  u16_t i = 0;
  for (u16_t y = 0; y < 20; ++y) {
    for (u16_t x = 0; x < 30; ++x) {
      TILEMAP_BG0_PTR[32 * y + x] = i;
      i++;
    }
  }
}

int main() {
  init();
  show_title_screen();

  DisplayFadeEffect e;
  Controller c;
  u16_t wait = 0;
  bool transition_triggered = false;
  while (1) {
    vid_vsync();
    c.update();

    if (c.is_just_pressed(BTN_START)) {
      e.reset();
      transition_triggered = true;
    }

    if (wait) {
      wait--;
      continue;
    }

    if (transition_triggered) {
      e.update();
      if (e.get_state() == DisplayFadeEffect::State::Middle) {
        wait = 100;
      }

      if (e.get_state() == DisplayFadeEffect::State::Finished) {
        transition_triggered = false;
      }
    }
  }
}

