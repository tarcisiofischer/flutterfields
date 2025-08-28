#include <gba.h>
#include <random.h>
#include <init.h>

struct UpdatableObject {
  virtual void update() = 0;
};

struct Screen : public UpdatableObject {
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

static u16_t FRAMES_IDLE_DOWN[] = {4, 5, 0};
static u16_t FRAMES_WALK_DOWN[] = {6, 7, 0};
static u16_t FRAMES_IDLE_LEFT[] = {20, 21, 0};
static u16_t FRAMES_WALK_LEFT[] = {22, 23, 0};
static u16_t FRAMES_IDLE_RIGHT[] = {28, 29, 0};
static u16_t FRAMES_WALK_RIGHT[] = {30, 31, 0};
static u16_t FRAMES_IDLE_UP[] = {12, 13, 0};
static u16_t FRAMES_WALK_UP[] = {14, 15, 0};

class Animation : public UpdatableObject {
public:
  Animation(u16_t* frames, u16_t frame_fps, volatile OAM_attr* target = nullptr)
    : frames(frames)
    , frame_fps(frame_fps)
    , target(target)
  {}

  void set_target(volatile OAM_attr* t) { target = t; }

  void reset() {
    frame_time = 0;
    next_frame = 0;
  }

  void update() override {
    if (frame_time == 0) {
      frame_time = frame_fps;
      target->set_sprite(OAM_attr::step16x16(frames[next_frame]));
      next_frame++;
      if (frames[next_frame] == 0) {
        next_frame = 0;
      }
    }
    frame_time--;
  }

private:
  u16_t* frames;
  u16_t frame_fps = 0;
  volatile OAM_attr* target = nullptr;
  u16_t frame_time = 0;
  u16_t next_frame = 0;
};

enum class Facing {
  UP = 0,
  DOWN,
  LEFT,
  RIGHT
};

struct MainGameScreen : public Screen {
  MainGameScreen(Controller& c)
    : controller(c)
    , idle_down(FRAMES_IDLE_DOWN, 20)
    , walk_down(FRAMES_WALK_DOWN, 10)
    , idle_up(FRAMES_IDLE_UP, 20)
    , walk_up(FRAMES_WALK_UP, 10)
    , idle_left(FRAMES_IDLE_LEFT, 20)
    , walk_left(FRAMES_WALK_LEFT, 10)
    , idle_right(FRAMES_IDLE_RIGHT, 20)
    , walk_right(FRAMES_WALK_RIGHT, 10)
  {}

  void init() {
    main_char = OAM_attr::get_obj(OAM_attr::next_available_id());
    main_char->set_size(OAM_attr::ObjectSize::_16x16);
    main_char->set_sprite(OAM_attr::step16x16(4));
    main_char->set_x(240 / 2 - 8);
    main_char->set_y(160 / 2 - 8);
    idle_down.set_target(main_char);
    walk_down.set_target(main_char);
    idle_up.set_target(main_char);
    walk_up.set_target(main_char);
    idle_left.set_target(main_char);
    walk_left.set_target(main_char);
    idle_right.set_target(main_char);
    walk_right.set_target(main_char);

    load_main_game_spritesheet();
    load_main_game_tilemap();

    for (u16_t i = 0; i < 128; ++i) {
      objs[i] = nullptr;
      objs_x[i] = 0;
      objs_y[i] = 0;
    }
    for (u16_t i = 1; i < 128; ++i) {
      auto* obj = OAM_attr::get_obj(i);
      if (obj->get_sprite_id() == 0) {
        continue;
      }
      objs[i - 1] = obj;
      objs_x[i - 1] = obj->get_x();
      objs_y[i - 1] = obj->get_y();
    }

    current_animation = &idle_down;
  }

  void update() override {
    static auto const TILEMAP_BG0_PTR = (volatile u16_t*)(0x06000000 + 0 * 0x800);

    if (controller.is_just_pressed(BTN_RIGHT)) {
      walk_right.reset();
    } else if (controller.is_just_pressed(BTN_LEFT)) {
      walk_left.reset();
    } else if (controller.is_just_pressed(BTN_UP)) {
      walk_up.reset();
    } else if (controller.is_just_pressed(BTN_DOWN)) {
      walk_down.reset();
    }

    if (controller.is_pressed(BTN_RIGHT)) {
      current_animation = &walk_right;
      facing = Facing::RIGHT;
      x_offset++;
    } else if (controller.is_pressed(BTN_LEFT)) {
      current_animation = &walk_left;
      facing = Facing::LEFT;
      x_offset--;
    }

    if (controller.is_pressed(BTN_UP)) {
      current_animation = &walk_up;
      facing = Facing::UP;
      y_offset--;
    } else if (controller.is_pressed(BTN_DOWN)) {
      current_animation = &walk_down;
      facing = Facing::DOWN;
      y_offset++;
    }

    if (
      !controller.is_pressed(BTN_UP) &&
      !controller.is_pressed(BTN_DOWN) &&
      !controller.is_pressed(BTN_LEFT) &&
      !controller.is_pressed(BTN_RIGHT)
    ) {
      switch (facing) {
        case Facing::UP:
          current_animation = &idle_up;
          break;
        case Facing::DOWN:
          current_animation = &idle_down;
          break;
        case Facing::LEFT:
          current_animation = &idle_left;
          break;
        case Facing::RIGHT:
          current_animation = &idle_right;
          break;
      }
    }
    current_animation->update();

    *REG_BG0HOFS = x_offset;
    *REG_BG1HOFS = x_offset;
    *REG_BG0VOFS = y_offset;
    *REG_BG1VOFS = y_offset;

    for (int i = 0; objs[i] != nullptr; ++i) {
      objs[i]->set_x(objs_x[i] - x_offset);
      objs[i]->set_y(objs_y[i] - y_offset);
    }
  }

private:
  Controller& controller;
  u16_t x_offset = 0;
  u16_t y_offset = 0;
  volatile OAM_attr* main_char = nullptr;

  volatile OAM_attr* objs[128];
  u16_t objs_x[128];
  u8_t objs_y[128];

  Facing facing = Facing::DOWN;
  Animation idle_down;
  Animation walk_down;
  Animation idle_up;
  Animation walk_up;
  Animation idle_left;
  Animation walk_left;
  Animation idle_right;
  Animation walk_right;
  Animation* current_animation = nullptr;
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

