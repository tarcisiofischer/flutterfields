#include <gba.h>
#include <random.h>
#include <init.h>

struct Screen {
  virtual void update() = 0;
};

struct TitleScreen : public Screen {
  TitleScreen(Controller& c) : controller(c) {}

  class OnStartPressedAdapter {
  public:
    virtual void on_transition_black() = 0;
    virtual void on_finished_transition() = 0;
  };

  void set_on_start_pressed_adapter(OnStartPressedAdapter* adapter) {
    this->on_start_pressed_adapter = adapter;
  }

  void show() {
    load_title_screen_spritesheet();

    static auto const TILEMAP_BG1_PTR = (volatile u16_t*)(0x06000000 + 1 * 0x800);
    u16_t i = 0;
    for (u16_t y = 0; y < 20; ++y) {
      for (u16_t x = 0; x < 30; ++x) {
        TILEMAP_BG1_PTR[32 * y + x] = 0;
        i++;
      }
    }

    e.reset();
  }

  void update() override {
    if (controller.is_just_pressed(BTN_START)) {
      e.reset();
      transition_triggered = true;
    }

    if (transition_triggered) {
      if (wait) {
        wait--;
        return;
      }

      e.update();
      if (e.get_state() == DisplayFadeEffect::State::Middle) {
        wait = 40;
        this->on_start_pressed_adapter->on_transition_black();
      }

      if (e.get_state() == DisplayFadeEffect::State::Finished) {
        transition_triggered = false;
        this->on_start_pressed_adapter->on_finished_transition();
      }
    }
  }

private:
  DisplayFadeEffect e;
  bool transition_triggered = false;
  u16_t wait = 0;
  Controller& controller;
  OnStartPressedAdapter* on_start_pressed_adapter = nullptr;
};

struct MainGameScreen : public Screen {
  MainGameScreen(Controller& c) : controller(c) {}

  void init() {
    load_main_game_spritesheet();
    load_main_game_tilemap();

    main_char = OAM_attr::get_obj(OAM_attr::next_available_id());
    main_char->set_size(OAM_attr::ObjectSize::_16x16);
    main_char->set_sprite(OAM_attr::step16x16(4));
    main_char->set_x(240 / 2 - 8);
    main_char->set_y(160 / 2 - 8);
  }

  void update() override {
    static auto const TILEMAP_BG0_PTR = (volatile u16_t*)(0x06000000 + 0 * 0x800);

    if (controller.is_pressed(BTN_RIGHT)) {
      x_offset++;
      *REG_BG0HOFS = x_offset;
      *REG_BG1HOFS = x_offset;

      main_char->set_sprite(OAM_attr::step16x16(28));
    }

    if (controller.is_pressed(BTN_LEFT)) {
      x_offset--;
      *REG_BG0HOFS = x_offset;
      *REG_BG1HOFS = x_offset;

      main_char->set_sprite(OAM_attr::step16x16(20));
    }

    if (controller.is_pressed(BTN_UP)) {
      y_offset--;
      *REG_BG0VOFS = y_offset;
      *REG_BG1VOFS = y_offset;

      main_char->set_sprite(OAM_attr::step16x16(12));
    }

    if (controller.is_pressed(BTN_DOWN)) {
      y_offset++;
      *REG_BG0VOFS = y_offset;
      *REG_BG1VOFS = y_offset;

      main_char->set_sprite(OAM_attr::step16x16(4));
    }
  }

private:
  Controller& controller;
  u16_t x_offset = 0;
  u16_t y_offset = 0;
  volatile OAM_attr* main_char = nullptr;
};

class GameOrchestrator
 : public TitleScreen::OnStartPressedAdapter
{
public:
  GameOrchestrator() : c{}, ts{c}, mgs{c}, current_screen{&ts} {}

  void init() {
    ::init();
    ts.set_on_start_pressed_adapter(this);
    ts.show();
  }

  void update() {
    c.update();
    current_screen->update();
  }

  void on_transition_black() override {
    mgs.init();
  }

  void on_finished_transition() override {
    current_screen = &mgs;
  }

private:
  Controller c;
  TitleScreen ts;
  MainGameScreen mgs;
  Screen* current_screen = nullptr;
};

int main() {
  GameOrchestrator game;
  game.init();

  while (1) {
    vid_vsync();
    game.update();
  }
}

