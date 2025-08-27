set(CMAKE_CXX_STANDARD 20)

gba_exec(
  NAME flutterfields
  SOURCES
  main.cc
  init.h
  rand.h
  SPRITESHEETS
  title_screen.png
  character.png
  tileset.png
  TILEMAP
  tilemap.json
)
target_compile_options(
  flutterfields PRIVATE -I${CMAKE_SOURCE_DIR}/src/
)
