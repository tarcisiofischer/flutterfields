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
    (0b0   << 9)  | // BG 1 enable flag
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
    (0b00    << 0)  | // Priority 0 (Highest)
    (0b01    << 2)  | // Start addr of tile data 0x06000000 + S * 0x4000
    (0b00    << 4)  | // Unused
    (0b0     << 6)  | // Mosaic effect 0 (Off)
    (0b0     << 7)  | // Use 16 color map
    (0b00000 << 8)  | // Starting addr of tile map 0x06000000 + M * 0x800
    (0b0     << 13) | // Screen over / RO
    (0b00    << 14)   // Size of tile map entries. 0 means 256x256 pixels (32x32 tiles)
  ;
}

void setup_spritesheet() {
//  #include <spritesheet_objects.inc>
//  for (u16_t i = 0; i < TILESET_SIZE; ++i) {
//    OBJ_TILES[i] = TILESET[i];
//  }
//  for (u16_t i = 0; i < PALETTE_SIZE; ++i) {
//    OBJ_PALETTE1[i] = PALETTE[i];
//  }
}

void init() {
  setup_display();
  setup_spritesheet();
  //setup_bg_tileset();
  //setup_tilemaps();
  Sound::init();
}

#endif

