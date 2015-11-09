#include "Font.h"

#include <cassert>

std::unique_ptr<Font> Font::MakeFont(const PixelData& pd) {
  // needs alpha
  assert(pd.format == GL_RGBA || pd.format == GL_BGRA);

  std::unique_ptr<Font> font(new Font());
  int ci = 0;
  // TODO: Assumes tileset w = h = 16
  int tile_width = 16;
  int tile_height = 16;
  assert(pd.w % tile_width == 0);
  assert(pd.h % tile_height == 0);

  int rows = pd.h / tile_height;
  int cols = pd.w / tile_width;
  assert(rows * cols == 256);

  for (int row = rows - 1; row >= 0; --row) {
    for (int col = 0; col < cols; ++col) {
      Character& c = font->characters_[ci];
      c.x = row; // Bottom is row 0 (texture coordinates)
      c.y = col;
      c.width = 16;

      for (int tx = tile_width - 1; tx >= 0; --tx) {
        bool hit = false;
        for (int ty = 0; ty < tile_height; ++ty) {
          int px = col * tile_width + tx;
          int py = row * tile_height + ty;
          int alpha_i =
              py * cols * tile_width * pd.bpp + px * pd.bpp + pd.bpp - 1;
          if (pd.data[alpha_i] != 0) { // we hit a pixel
            hit = true;
            c.width = tx + 1; // finding a pixel in col 0 means width = 1.
            break;
          }
        }
        if (hit) {
          break;
        }
      }

      ++ci;
    }
  }

  return font;
}

Character Font::GetCharacter(char) {
  return {};
}
