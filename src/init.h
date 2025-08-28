#ifndef __INIT_GBA
#define __INIT_GBA

void setup_display() {
  /**
   * Mode 0 is a tile/map-based mode with no rotation/scaling and supports up
   * to 4 layers.
   *
   * 1D object mapping means each 8x8 sprite piece is put side-by-side in the
   * spritesheet. There are 1024 8x8 sprite pieces per spritesheet.
   */
  *REG_DISPCNT =
    (0b000 << 0)  | // Video Mode 0
    (0b0   << 3)  | // GBA
    (0b0   << 4)  | // Unused in Mode 0
    (0b0   << 5)  | // No access to OAM in H-Blank
    (0b1   << 6)  | // 1D Object Mapping
    (0b0   << 7)  | // No fast access to VRAM,Palette,OAM
    (0b1   << 8)  | // BG 0 enable flag
    (0b1   << 9)  | // BG 1 enable flag
    (0b0   << 10) | // BG 2 enable flag
    (0b0   << 11) | // BG 3 enable flag
    (0b1   << 12) | // OBJ enable flag
    (0b0   << 13) | // Window 0 display flag
    (0b0   << 14) | // Window 1 display flag
    (0b0   << 15) // OBJ window display flag
  ;

  /**
   * Background 0 configuration.
   *
   * Single background, 16 colors color map, 32x32 tiles, tiles are 8x8 pixels each.
   */
  *REG_BG0CNT =
    (0b01    << 0)  | // Priority
    (0b01    << 2)  | // Start addr of tile data 0x06000000 + S * 0x4000
    (0b00    << 4)  | // Unused
    (0b0     << 6)  | // Mosaic effect 0 (Off)
    (0b0     << 7)  | // Use 16 color map
    (0b00000 << 8)  | // Starting addr of tile map 0x06000000 + M * 0x800
    (0b0     << 13) | // Screen over / RO
    (0b00    << 14)   // Size of tile map entries. 0 means 256x256 pixels (32x32 tiles)
  ;

  *REG_BG1CNT =
    (0b00    << 0)  | // Priority
    (0b01    << 2)  | // Start addr of tile data 0x06000000 + S * 0x4000
    (0b00    << 4)  | // Unused
    (0b0     << 6)  | // Mosaic effect 0 (Off)
    (0b0     << 7)  | // Use 16 color map
    (0b00001 << 8)  | // Starting addr of tile map 0x06000000 + M * 0x800
    (0b0     << 13) | // Screen over / RO
    (0b00    << 14)   // Size of tile map entries. 0 means 256x256 pixels (32x32 tiles)
  ;
}

void setup_objects_spritesheet() {
  #include <spritesheet_objects.inc>
  for (u16_t i = 0; i < TILESET_SIZE; ++i) {
    OBJ_TILES[i] = TILESET[i];
  }
  for (u16_t i = 0; i < PALETTE_SIZE; ++i) {
    OBJ_PALETTE1[i] = PALETTE[i];
  }
}

void load_title_screen_spritesheet() {
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

void load_main_game_spritesheet() {
  #include <spritesheet_tileset.inc>
  static auto const TILESET_PTR = (volatile u16_t*)(0x06000000 + 1 * 0x4000);
  for (u16_t i = 0; i < PALETTE_SIZE; ++i) {
    BG_PALETTE[i] = PALETTE[i];
  }
  for (u16_t i = 0; i < TILESET_SIZE; i++) {
    TILESET_PTR[i] = TILESET[i];
  }
}

void load_main_game_tilemap() {
  #include <tilemap.inc>
  static auto const TILEMAP_BG0_PTR = (volatile u16_t*)(0x06000000 + 0 * 0x800);
  static auto const TILEMAP_BG1_PTR = (volatile u16_t*)(0x06000000 + 1 * 0x800);

  u16_t i = 0;
  for (u16_t y = 0; y < 16; ++y) {
    for (u16_t x = 0; x < 16; ++x) {
      TILEMAP_BG0_PTR[2*32 * y + 2*x + 0] = 4*TILEMAP_bg0[i];
      TILEMAP_BG0_PTR[2*32 * y + 2*x + 1] = 4*TILEMAP_bg0[i] + 1;
      TILEMAP_BG0_PTR[2*32 * y + 2*x + 32] = 4*TILEMAP_bg0[i] + 2;
      TILEMAP_BG0_PTR[2*32 * y + 2*x + 33] = 4*TILEMAP_bg0[i] + 3;

      i++;
    }
  }

  i = 0;
  for (u16_t y = 0; y < 16; ++y) {
    for (u16_t x = 0; x < 16; ++x) {
      TILEMAP_BG1_PTR[2*32 * y + 2*x + 0] = 4*TILEMAP_bg1[i];
      TILEMAP_BG1_PTR[2*32 * y + 2*x + 1] = 4*TILEMAP_bg1[i] + 1;
      TILEMAP_BG1_PTR[2*32 * y + 2*x + 32] = 4*TILEMAP_bg1[i] + 2;
      TILEMAP_BG1_PTR[2*32 * y + 2*x + 33] = 4*TILEMAP_bg1[i] + 3;

      i++;
    }
  }

  u16_t x = 0;
  u16_t y = 0;
  for (u16_t i = 0; i < TILEMAP_objs_SIZE; ++i) {
    if (TILEMAP_objs[i] != 0) {
      volatile OAM_attr* obj = OAM_attr::get_obj(OAM_attr::next_available_id());
      obj->set_size(OAM_attr::ObjectSize::_16x16);
      obj->set_sprite(OAM_attr::step16x16(TILEMAP_objs[i]));
      obj->set_x(16 * x);
      obj->set_y(16 * y);
    }

    x++;
    if (x == 16) {
      x = 0;
      y++;
    }
  }
}

void init() {
  setup_display();
  setup_objects_spritesheet();
  Sound::init();
}

#endif

