#include "AUILib.h"

namespace aui {

  void DrawThickLine(uint32_t* buffer, uint32_t bufferWidth, uint32_t bufferHeight,
                            int32_t x0, int32_t y0, int32_t x1, int32_t y1,
                            uint32_t color, uint32_t thickness) {
    if (thickness == 0) return;
    // Bresenham line generation
    int32_t dx = std::abs(x1 - x0);
    int32_t dy = -std::abs(y1 - y0);
    int32_t sx = (x0 < x1) ? 1 : -1;
    int32_t sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx + dy;
    // Half thickness offset for centering
    int32_t half = static_cast<int32_t>(thickness / 2);
    int32_t extra = static_cast<int32_t>(thickness % 2);
    while (true) {
      // Draw a small rectangle around the line pixel
      for (int32_t dy_off = -half; dy_off < half + extra; ++dy_off) {
        for (int32_t dx_off = -half; dx_off < half + extra; ++dx_off) {
          int32_t px = x0 + dx_off;
          int32_t py = y0 + dy_off;
          if (px >= 0 && px < static_cast<int32_t>(bufferWidth) &&
              py >= 0 && py < static_cast<int32_t>(bufferHeight)) {
            buffer[static_cast<size_t>(py) * bufferWidth + static_cast<size_t>(px)] = color;
          }
        }
      }
      if (x0 == x1 && y0 == y1) break;
      int32_t e2 = 2 * err;
      if (e2 >= dy) {
        err += dy;
        x0 += sx;
      }
      if (e2 <= dx) {
        err += dx;
        y0 += sy;
      }
    }
  }


}// namespace aui

