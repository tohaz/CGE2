#include "AUILib.h"

namespace aui {

  void DrawRotatedRect(uint32_t* buffer, uint32_t stride, int32_t clipMinX, int32_t clipMinY, int32_t clipMaxX,
      int32_t clipMaxY, int32_t rectX, int32_t rectY, uint32_t rectW, uint32_t rectH, double angleDeg, int32_t parentX,
      int32_t parentY, uint32_t parentW, uint32_t parentH, double parentAngleDeg, uint32_t color) {
    constexpr double DEG2RAD = 3.14159265358979323846 / 180.0;
    double angleRad = angleDeg * DEG2RAD;
    double pAngleRad = parentAngleDeg * DEG2RAD;
// Parent transform
    double pCos = std::cos(pAngleRad);
    double pSin = std::sin(pAngleRad);
    double phw = static_cast<double>(parentW) / 2.0;
    double phh = static_cast<double>(parentH) / 2.0;
    double pcx = static_cast<double>(parentX) + phw;
    double pcy = static_cast<double>(parentY) + phh;
// Child local centre (relative to parent's top-left)
    double childLocalCX = static_cast<double>(rectX) + static_cast<double>(rectW) / 2.0;
    double childLocalCY = static_cast<double>(rectY) + static_cast<double>(rectH) / 2.0;
// Offset from parent centre
    double offsetX = childLocalCX - phw;
    double offsetY = childLocalCY - phh;
// Global centre
    double cx = pcx + (offsetX * pCos - offsetY * pSin);
    double cy = pcy + (offsetX * pSin + offsetY * pCos);
// Child rotation (relative to parent if you want nested)
    double childAngleRad = angleRad;// or pAngleRad + angleRad for relative
    double cosA = std::cos(childAngleRad);
    double sinA = std::sin(childAngleRad);
    double hw = static_cast<double>(rectW) / 2.0;
    double hh = static_cast<double>(rectH) / 2.0;
// Bounding box (global)
    const double cornersX[4] = { -hw, hw, hw, -hw };
    const double cornersY[4] = { -hh, -hh, hh, hh };
    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    for(int32_t i = 0; i < 4; ++i) {
      double rx = cornersX[i] * cosA - cornersY[i] * sinA + cx;
      double ry = cornersX[i] * sinA + cornersY[i] * cosA + cy;
      minX = std::min(minX, rx);
      maxX = std::max(maxX, rx);
      minY = std::min(minY, ry);
      maxY = std::max(maxY, ry);
    }
    int32_t startX = std::max(clipMinX, static_cast<int32_t>(std::floor(minX)));
    int32_t endX = std::min(clipMaxX, static_cast<int32_t>(std::ceil(maxX)));
    int32_t startY = std::max(clipMinY, static_cast<int32_t>(std::floor(minY)));
    int32_t endY = std::min(clipMaxY, static_cast<int32_t>(std::ceil(maxY)));
    if(startX > endX || startY > endY)
      return;
// Rasterize
    for(int32_t y = startY; y <= endY; ++y) {
      size_t line = static_cast<size_t>(y) * stride;
      double pixelY = static_cast<double>(y) + 0.5;
      double pdy = pixelY - pcy;
      double dy = pixelY - cy;
      for(int32_t x = startX; x <= endX; ++x) {
        double pixelX = static_cast<double>(x) + 0.5;
        double pdx = pixelX - pcx;
// Parent clip (local coords)
        double plx = (pdx * pCos + pdy * pSin) + phw;
        double ply = (-pdx * pSin + pdy * pCos) + phh;
        if(plx < 0.0 || plx >= static_cast<double>(parentW) || ply < 0.0 || ply >= static_cast<double>(parentH))
          continue;
// Child local test
        double dx = pixelX - cx;
        double lx = (dx * cosA + dy * sinA) + hw;
        double ly = (-dx * sinA + dy * cosA) + hh;
        if(lx >= 0.0 && lx < static_cast<double>(rectW) && ly >= 0.0 && ly < static_cast<double>(rectH))
          buffer[line + static_cast<size_t>(x)] = color;
      }
    }
  }

  uint8_t* scale_bgra_bitmap(const uint8_t* src, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
      int32_t srcPitch, int32_t& dstPitch);

  uint8_t* scale_bgra_bitmap(const uint8_t* src, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
      int32_t srcPitch, int32_t& dstPitch) {
    if(srcW <= 0 || srcH <= 0 || dstW <= 0 || dstH <= 0)
      return nullptr;
    dstPitch = dstW * 4;
    uint8_t* dst = new uint8_t[dstH * dstPitch];
    for(int32_t y = 0; y < dstH; ++y) {
      int32_t srcY = y * srcH / dstH;
      const uint8_t* srcRow = src + srcY * srcPitch;
      uint8_t* dstRow = dst + y * dstPitch;
      for(int32_t x = 0; x < dstW; ++x) {
        int32_t srcX = x * srcW / dstW;
        const uint8_t* srcPix = srcRow + srcX * 4;
        uint8_t* dstPix = dstRow + x * 4;
        dstPix[0] = srcPix[0];
        dstPix[1] = srcPix[1];
        dstPix[2] = srcPix[2];
        dstPix[3] = srcPix[3];
      }
    }
    return dst;
  }
// ---- Helper: Safe UTF-8 Parsing ----
  UNUSED static uint32_t GetNextCodepoint(const uint8_t*& ptr) {
    if((*ptr & 0x80) == 0)
      return *ptr++;
    if((*ptr & 0xE0) == 0xC0) {
      uint32_t cp = static_cast<uint32_t>(*ptr++ & 0x1F) << 6;
      return cp | (*ptr++ & 0x3F);
    }
    if((*ptr & 0xF0) == 0xE0) {
      uint32_t cp = static_cast<uint32_t>(*ptr++ & 0x0F) << 12;
      cp |= static_cast<uint32_t>(*ptr++ & 0x3F) << 6;
      return cp | (*ptr++ & 0x3F);
    }
    if((*ptr & 0xF8) == 0xF0) {
      uint32_t cp = static_cast<uint32_t>(*ptr++ & 0x07) << 18;
      cp |= static_cast<uint32_t>(*ptr++ & 0x3F) << 12;
      cp |= static_cast<uint32_t>(*ptr++ & 0x3F) << 6;
      return cp | (*ptr++ & 0x3F);
    }
    ptr++;
    return 0;
  }

  static inline void BlendPixelRGBA(uint32_t* dest, uint32_t color) {
    uint8_t a = static_cast<uint8_t>((color >> 24) & 0xFF);
    if(a == 0)
      return;
    if(a == 255) {
// Keep existing background alpha untouched, pull raw colors
      *dest = (*dest & 0xFF000000) | (color & 0x00FFFFFF);
    }
    else {
      uint32_t bg = *dest;
      uint32_t bg_alpha = bg & 0xFF000000;// Keep background alpha completely untouched
      uint32_t inv = 255 - a;
// REMOVE the "* a" from the source channels because FreeType pre-multiplies them!
      uint32_t r = (((((color >> 16) & 0xFF) * 255) + ((bg >> 16) & 0xFF) * inv + 128) >> 8) << 16;
      uint32_t g = (((((color >> 8) & 0xFF) * 255) + ((bg >> 8) & 0xFF) * inv + 128) >> 8) << 8;
      uint32_t b = ((((color & 0xFF) * 255) + (bg & 0xFF) * inv + 128) >> 8);
      *dest = bg_alpha | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);
    }
  }

  static inline void BlendPixelGrayscale(uint32_t* dest, uint8_t alpha, uint32_t col_r, uint32_t col_g,
      uint32_t col_b) {
    if(alpha == 0)
      return;
    uint32_t bg = *dest;
    uint32_t bg_alpha = bg & 0xFF000000;// 保留原背景的 Alpha 通道
    uint32_t inv = 255 - alpha;
    uint32_t r = ((col_r * alpha + ((bg >> 16) & 0xFF) * inv + 128) >> 8) << 16;
    uint32_t g = ((col_g * alpha + ((bg >> 8) & 0xFF) * inv + 128) >> 8) << 8;
    uint32_t b = (col_b * alpha + (bg & 0xFF) * inv + 128) >> 8;
    *dest = bg_alpha | r | g | b;
  }
// ---- Isolated Render Block 2: Bitmap Blitting (Any Pixel Mode) ----
  static void BlitGlyphBitmap(uint32_t* buffer, size_t pW, const TextLayout& layout, int32_t glyphLeft,
      int32_t glyphTop, const FT_Bitmap* bitmap, uint32_t col_r, uint32_t col_g, uint32_t col_b) {
    for(int32_t row = 0; row < static_cast<int32_t>(bitmap->rows); ++row) {
      int32_t destY = glyphTop + row;
      if(destY < layout.clipT || destY >= layout.clipB)
        continue;
      const uint8_t* src = bitmap->buffer + row * bitmap->pitch;
      size_t rowOffset = static_cast<size_t>(destY) * pW;
      for(int32_t col = 0; col < static_cast<int32_t>(bitmap->width); ++col) {
        int32_t destX = glyphLeft + col;
        if(destX < layout.clipL || destX >= layout.clipR)
          continue;
        size_t idx = rowOffset + static_cast<size_t>(destX);
        if(bitmap->pixel_mode == FT_PIXEL_MODE_BGRA) {
          uint32_t color = *reinterpret_cast<const uint32_t*>(src + col * 4);
          BlendPixelRGBA(&buffer[idx], color);
        }
        else {
          BlendPixelGrayscale(&buffer[idx], src[col], col_r, col_g, col_b);
        }
      }
    }
  }
// ---- Helper: Scale and Rotate BGRA Bitmaps ----
  static uint8_t* rotate_and_scale_bgra_bitmap(const uint8_t* src, int32_t srcW, int32_t srcH, int32_t dstW,
      int32_t dstH, int32_t srcPitch, double angle, int32_t& dstPitch) {
    if(srcW <= 0 || srcH <= 0 || dstW <= 0 || dstH <= 0)
      return nullptr;
    dstPitch = dstW * 4;
    uint8_t* dst = new uint8_t[static_cast<size_t>(dstH * dstPitch)];
    std::memset(dst, 0, static_cast<size_t>(dstH * dstPitch));
    float cosA = static_cast<float>(std::cos(-angle));
    float sinA = static_cast<float>(std::sin(-angle));
    float cxDst = static_cast<float>(dstW) / 2.0f;
    float cyDst = static_cast<float>(dstH) / 2.0f;
    float cxSrc = static_cast<float>(srcW) / 2.0f;
    float cySrc = static_cast<float>(srcH) / 2.0f;
    float scaleX = static_cast<float>(srcW) / static_cast<float>(dstW);
    float scaleH = static_cast<float>(srcH) / static_cast<float>(dstH);
    for(int32_t y = 0; y < dstH; ++y) {
      float dy = static_cast<float>(y) - cyDst;
      uint8_t* dstRow = dst + y * dstPitch;
      for(int32_t x = 0; x < dstW; ++x) {
        float dx = static_cast<float>(x) - cxDst;
        int32_t srcX = static_cast<int32_t>((dx * cosA - dy * sinA) * scaleX + cxSrc);
        int32_t srcY = static_cast<int32_t>((dx * sinA + dy * cosA) * scaleH + cySrc);
        if(srcX >= 0 && srcX < srcW && srcY >= 0 && srcY < srcH) {
          const uint8_t* srcPix = src + srcY * srcPitch + srcX * 4;
          uint8_t* dstPix = dstRow + x * 4;
          dstPix[0] = srcPix[0];
          dstPix[1] = srcPix[1];
          dstPix[2] = srcPix[2];
          dstPix[3] = srcPix[3];
        }
      }
    }
    return dst;
  }
// ---- Helper: Unified Rotated Rendering Block For Emojis ----
  UNUSED static void RenderFreshEmojiRotated(uint32_t* buffer, size_t pW, const TextLayout& layout, double centerX,
      double centerY, uint32_t fontSize, FT_Bitmap* bitmap, double angle) {
    int32_t targetSize = static_cast<int32_t>(fontSize);
    int32_t srcW = static_cast<int32_t>(bitmap->width);
    int32_t srcH = static_cast<int32_t>(bitmap->rows);
    int32_t dstW = targetSize, dstH = targetSize, dstPitch = 0;
    uint8_t* rawScaledBuffer = rotate_and_scale_bgra_bitmap(bitmap->buffer, srcW, srcH, dstW, dstH, bitmap->pitch,
        angle, dstPitch);
    if(!rawScaledBuffer)
      return;
    std::unique_ptr<uint8_t[]> managedScaled(rawScaledBuffer);
    FT_Bitmap scaledBitmap;
    scaledBitmap.rows = static_cast<uint32_t>(dstH);
    scaledBitmap.width = static_cast<uint32_t>(dstW);
    scaledBitmap.pitch = dstPitch;
    scaledBitmap.buffer = managedScaled.get();
    scaledBitmap.pixel_mode = FT_PIXEL_MODE_BGRA;
// Center the square emoji directly on top of the calculated center coordinate
    int32_t blitX = static_cast<int32_t>(std::round(centerX - (double) dstW / 2.0));
    int32_t blitY = static_cast<int32_t>(std::round(centerY - (double) dstH / 2.0));
    BlitGlyphBitmap(buffer, pW, layout, blitX, blitY, &scaledBitmap, 0, 0, 0);
  }

  void DrawTextEx(uint32_t* buffer, uint32_t parentWidth, uint32_t parentHeight, const ARect& bounds,
      const std::string& text, FT_Face face, const ATextStyle& style, const ARect* customClip = nullptr) {
    D2("text alignment {} text {}", (uint64_t) style.hAlign, text)
    if(text.empty() || !face || parentWidth == 0 || parentHeight == 0)
      return;
    AUI* au = static_cast<AUI*>(face->generic.data);
    if(!au) {
      E("engine is null");
    }
    FT_Face fallbackFace = au->FallbackFace();
    FT_Select_Charmap(face, FT_ENCODING_UNICODE);
    if(fallbackFace) {
      FT_Select_Charmap(fallbackFace, FT_ENCODING_UNICODE);
    }
    FT_Set_Pixel_Sizes(face, 0, style.fontSize);
    if(fallbackFace) {
      if(FT_IS_SCALABLE(fallbackFace)) {
        FT_Set_Pixel_Sizes(fallbackFace, 0, style.fontSize);
      }
      else {
        int32_t best = 0, bestDiff = INT_MAX;
        for(int32_t i = 0; i < fallbackFace->num_fixed_sizes; ++i) {
          int32_t diff = std::abs(
              static_cast<int32_t>(fallbackFace->available_sizes[i].y_ppem) - static_cast<int32_t>(style.fontSize));
          if(diff < bestDiff) {
            bestDiff = diff;
            best = i;
          }
        }
        FT_Select_Size(fallbackFace, best);
      }
    }
    FT_Set_Transform(face, nullptr, nullptr);
    if(fallbackFace) {
      FT_Set_Transform(fallbackFace, nullptr, nullptr);
    }
    double totalWidthAccumulator = 0.0;
    int32_t maxAscent = static_cast<int32_t>(face->size->metrics.ascender >> 6);
    int32_t maxDescent = static_cast<int32_t>(face->size->metrics.descender >> 6);
    int32_t fontHeight = maxAscent - maxDescent;
    std::vector<double> glyphAdvances;
    glyphAdvances.reserve(text.size());
    int32_t firstGlyphBearingX = 0;
    bool isFirstGlyph = true;
// =====================================================================
// PASS 1: Measurement & Advance Caching
// =====================================================================
    {
      const uint8_t* layoutPtr = reinterpret_cast<const uint8_t*>(text.c_str());
      while(*layoutPtr != '\0') {
        uint32_t cp = GetNextCodepoint(layoutPtr);
        if(cp == 0)
          continue;
        if(cp > 127) {
          double adv = static_cast<double>(style.fontSize);
          totalWidthAccumulator += adv;
          glyphAdvances.push_back(adv);
          isFirstGlyph = false;
          continue;
        }
        FT_UInt glyph_index = FT_Get_Char_Index(face, cp);
        if(glyph_index == 0) {
          double adv = static_cast<double>(style.fontSize);
          totalWidthAccumulator += adv;
          glyphAdvances.push_back(adv);
          isFirstGlyph = false;
          continue;
        }
        if(FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING)) {
          double adv = static_cast<double>(style.fontSize);
          totalWidthAccumulator += adv;
          glyphAdvances.push_back(adv);
          isFirstGlyph = false;
          continue;
        }
        if(isFirstGlyph) {
          firstGlyphBearingX = face->glyph->bitmap_left;
          isFirstGlyph = false;
        }
        double adv = static_cast<double>(face->glyph->advance.x) / 64.0;
        totalWidthAccumulator += adv;
        glyphAdvances.push_back(adv);
      }
    }
    int32_t totalWidth = static_cast<int32_t>(std::round(totalWidthAccumulator));
    int32_t penX = bounds.x;
    int32_t penY = bounds.y + maxAscent;
    int32_t bWidth = static_cast<int32_t>(bounds.w);
    int32_t bHeight = static_cast<int32_t>(bounds.h);
    const int32_t kLayoutPaddingCorrection = 0;
    if(style.hAlign == AUIHAlign::center) {
      penX = bounds.x + (bWidth - totalWidth) / 2;
    }
    else
      if(style.hAlign == AUIHAlign::right) {
        penX = (bounds.x + bWidth) - totalWidth - firstGlyphBearingX - kLayoutPaddingCorrection;
      }
      else {
        penX = bounds.x - firstGlyphBearingX - kLayoutPaddingCorrection;
      }
    if(style.vAlign == AUIVAlign::center) {
      penY += (bHeight - fontHeight) / 2;
    }
    else
      if(style.vAlign == AUIVAlign::bottom) {
        penY += (bHeight - fontHeight);
      }
    double boxCenterX = static_cast<double>(bounds.x) + static_cast<double>(bounds.w) / 2.0;
    double boxCenterY = static_cast<double>(bounds.y) + static_cast<double>(bounds.h) / 2.0;
    bool isRotated = std::abs(style.angle) > 0.001;
    double cosRot = std::cos(style.angle);
    double sinRot = std::sin(style.angle);
    if(isRotated) {
      FT_Matrix matrix;
      double targetAngle = -style.angle;
      matrix.xx = static_cast<FT_Fixed>(std::cos(targetAngle) * 0x10000L);
      matrix.xy = static_cast<FT_Fixed>(-std::sin(targetAngle) * 0x10000L);
      matrix.yx = static_cast<FT_Fixed>(std::sin(targetAngle) * 0x10000L);
      matrix.yy = static_cast<FT_Fixed>(std::cos(targetAngle) * 0x10000L);
      FT_Vector delta = { 0, 0 };
      FT_Set_Transform(face, &matrix, &delta);
      if(fallbackFace) {
        FT_Set_Transform(fallbackFace, &matrix, &delta);
      }
    }
// =====================================================================
// Scissor / Clip Box Setup
// =====================================================================
    int32_t clipMinX, clipMaxX, clipMinY, clipMaxY;
    if(isRotated) {
      clipMinX = 0;
      clipMaxX = static_cast<int32_t>(parentWidth);
      clipMinY = 0;
      clipMaxY = static_cast<int32_t>(parentHeight);
    }
    else
      if(customClip) {
// Scissor directly to custom clip viewport (e.g. list boundaries)
        clipMinX = std::max(0, customClip->x);
        clipMaxX = std::min(static_cast<int32_t>(parentWidth), customClip->x + static_cast<int32_t>(customClip->w));
        clipMinY = std::max(0, customClip->y);
        clipMaxY = std::min(static_cast<int32_t>(parentHeight), customClip->y + static_cast<int32_t>(customClip->h));
      }
      else {
// Default layout bounds fall-back
        clipMinX = std::max(0, bounds.x);
        clipMaxX = std::min(static_cast<int32_t>(parentWidth), bounds.x + static_cast<int32_t>(bounds.w));
        clipMinY = std::max(0, bounds.y);
        clipMaxY = std::min(static_cast<int32_t>(parentHeight), bounds.y + static_cast<int32_t>(bounds.h));
      }
    TextLayout lLoc { totalWidth, fontHeight, bounds.x, static_cast<int32_t>(std::round(boxCenterY)), clipMinX,
        clipMaxX, clipMinY, clipMaxY };
// =====================================================================
// PASS 2: Deterministic Render Loop
// =====================================================================
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(text.c_str());
    double unrotatedPenX = static_cast<double>(penX);
    double sizeD = static_cast<double>(style.fontSize);
    size_t advanceIdx = 0;
    while(*ptr != '\0') {
      uint32_t cp = GetNextCodepoint(ptr);
      if(cp == 0)
        continue;
      double baselineAdvance = (advanceIdx < glyphAdvances.size()) ? glyphAdvances[advanceIdx++] : sizeD;
      FT_Face currentFace = face;
      FT_UInt glyph_index = 0;
      if(cp > 127 && fallbackFace) {
        currentFace = fallbackFace;
      }
      glyph_index = FT_Get_Char_Index(currentFace, cp);
      if(glyph_index == 0 && fallbackFace && cp <= 127) {
        currentFace = fallbackFace;
        glyph_index = FT_Get_Char_Index(currentFace, cp);
      }
      double currentTrackingX, currentTrackingY;
      if(isRotated) {
        double dx = unrotatedPenX - boxCenterX;
        double dy = static_cast<double>(penY) - boxCenterY;
        currentTrackingX = boxCenterX + (dx * cosRot - dy * sinRot);
        currentTrackingY = boxCenterY + (dx * sinRot + dy * cosRot);
      }
      else {
        currentTrackingX = unrotatedPenX;
        currentTrackingY = static_cast<double>(penY);
      }
      if(glyph_index == 0) {
        unrotatedPenX += baselineAdvance;
        continue;
      }
      if(FT_Load_Glyph(currentFace, glyph_index, FT_LOAD_RENDER | FT_LOAD_COLOR | FT_LOAD_NO_HINTING)) {
        if(FT_Load_Glyph(currentFace, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING)) {
          unrotatedPenX += baselineAdvance;
          continue;
        }
      }
      FT_GlyphSlot slot = currentFace->glyph;
// Emoji / BGRA Branch
      if(cp > 127 && slot->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA) {
        double finalCenterX, finalCenterY;
        if(isRotated) {
          double fCos = std::cos(-style.angle);
          double fSin = std::sin(-style.angle);
          double halfAdvX = (baselineAdvance / 2.0) * fCos;
          double halfAdvY = (baselineAdvance / 2.0) * fSin;
          double upX = (sizeD / 2.0) * fSin;
          double upY = (sizeD / 2.0) * fCos;
          finalCenterX = currentTrackingX + halfAdvX - upX;
          finalCenterY = currentTrackingY - halfAdvY - upY;
        }
        else {
          finalCenterX = currentTrackingX + baselineAdvance / 2.0;
          finalCenterY = currentTrackingY - (static_cast<double>(maxAscent) / 2.0);
        }
        RenderFreshEmojiRotated(buffer, parentWidth, lLoc, finalCenterX, finalCenterY, style.fontSize, &slot->bitmap,
            style.angle);
        unrotatedPenX += baselineAdvance;
      }
// Monochrome / Grayscale Text Branch
      else {
        if(slot->bitmap.rows == 0 || slot->bitmap.width == 0) {
          unrotatedPenX += baselineAdvance;
          continue;
        }
        int32_t drawX = static_cast<int32_t>(std::round(currentTrackingX));
        int32_t drawY = static_cast<int32_t>(std::round(currentTrackingY));
        uint32_t colorNoAlpha = style.color;
        uint8_t col_r = static_cast<uint8_t>((colorNoAlpha >> 16) & 0xFF);
        uint8_t col_g = static_cast<uint8_t>((colorNoAlpha >> 8) & 0xFF);
        uint8_t col_b = static_cast<uint8_t>(colorNoAlpha & 0xFF);
        BlitGlyphBitmap(buffer, parentWidth, lLoc, drawX + slot->bitmap_left, drawY - slot->bitmap_top, &slot->bitmap,
            col_r, col_g, col_b);
        unrotatedPenX += baselineAdvance;
      }
    }
    FT_Set_Transform(face, nullptr, nullptr);
    if(fallbackFace) {
      FT_Set_Transform(fallbackFace, nullptr, nullptr);
    }
  }

  void BlitRotated(const uint32_t* src, uint32_t srcW, uint32_t srcH, uint32_t* dst, uint32_t dstW, uint32_t dstH,
      int32_t dstX, int32_t dstY, double angleDeg, int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB,
      bool skipZero = false) {
    if(!src || !dst || srcW == 0 || srcH == 0)
      return;
// Clamp clip bounds
    clipL = std::max(0, clipL);
    clipT = std::max(0, clipT);
    clipR = std::min(static_cast<int32_t>(dstW), clipR);
    clipB = std::min(static_cast<int32_t>(dstH), clipB);
    if(clipL >= clipR || clipT >= clipB)
      return;
// --- Fast path: no rotation ---
    if(std::abs(angleDeg) < 1e-6) {
      int32_t srcL = dstX;
      int32_t srcT = dstY;
      int32_t srcR = dstX + static_cast<int32_t>(srcW);
      int32_t srcB = dstY + static_cast<int32_t>(srcH);
      int32_t copyL = std::max(srcL, clipL);
      int32_t copyT = std::max(srcT, clipT);
      int32_t copyR = std::min(srcR, clipR);
      int32_t copyB = std::min(srcB, clipB);
      if(copyL >= copyR || copyT >= copyB)
        return;
      int32_t copyW = copyR - copyL;
      for(int32_t y = copyT; y < copyB; ++y) {
        uint32_t* dstRow = dst + static_cast<size_t>(y) * dstW + static_cast<size_t>(copyL);
        const uint32_t* srcRow = src + static_cast<size_t>(y - dstY) * srcW + static_cast<size_t>(copyL - dstX);
        if(skipZero) {
          for(int32_t x = 0; x < copyW; ++x) {
            uint32_t val = srcRow[x];
            if(val != 0)
              dstRow[x] = val;
          }
        }
        else {
          std::memcpy(dstRow, srcRow, static_cast<size_t>(copyW) * sizeof(uint32_t));
        }
      }
      return;
    }
// --- Rotated path ---
    double rad = angleDeg * (M_PI / 180.0);
    double cosA = std::cos(rad);
    double sinA = std::sin(rad);
    double hw = static_cast<double>(srcW) * 0.5;
    double hh = static_cast<double>(srcH) * 0.5;
    double cx = static_cast<double>(dstX) + hw;
    double cy = static_cast<double>(dstY) + hh;
// 1. Calculate AABB of rotated sprite corners relative to center
    double cornersX[4] = { -hw, hw, hw, -hw };
    double cornersY[4] = { -hh, -hh, hh, hh };
    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    for(int32_t i = 0; i < 4; ++i) {
      double rx = cornersX[i] * cosA - cornersY[i] * sinA + cx;
      double ry = cornersX[i] * sinA + cornersY[i] * cosA + cy;
      minX = std::min(minX, rx);
      maxX = std::max(maxX, rx);
      minY = std::min(minY, ry);
      maxY = std::max(maxY, ry);
    }
// Intersect AABB with clip rect
    int32_t startY = std::max(clipT, static_cast<int32_t>(std::floor(minY)));
    int32_t endY = std::min(clipB, static_cast<int32_t>(std::ceil(maxY)));
    int32_t startX = std::max(clipL, static_cast<int32_t>(std::floor(minX)));
    int32_t endX = std::min(clipR, static_cast<int32_t>(std::ceil(maxX)));
    if(startX >= endX || startY >= endY)
      return;
// 2. Fixed-point setup (16.16)
    constexpr int32_t FP_SHIFT = 16;
    constexpr int32_t FP_HALF = 1 << (FP_SHIFT - 1);
    constexpr int32_t FP_ONE = 1 << FP_SHIFT;
    int32_t stepXx = static_cast<int32_t>(cosA * FP_ONE);
    int32_t stepXy = static_cast<int32_t>(-sinA * FP_ONE);
    int32_t maxSi = static_cast<int32_t>(srcW);
    int32_t maxSj = static_cast<int32_t>(srcH);
// 3. Render loop using 16.16 fixed-point stepping
    for(int32_t y = startY; y < endY; ++y) {
      double dy = static_cast<double>(y) - cy;
      double dx0 = static_cast<double>(startX) - cx;
// Base source coordinates at start of line + 0.5 rounding offset
      int32_t fpSx = static_cast<int32_t>((dx0 * cosA + dy * sinA + hw) * FP_ONE) + FP_HALF;
      int32_t fpSy = static_cast<int32_t>((-dx0 * sinA + dy * cosA + hh) * FP_ONE) + FP_HALF;
      uint32_t* dstRow = dst + static_cast<size_t>(y) * dstW;
      for(int32_t x = startX; x < endX; ++x) {
        int32_t si = fpSx >> FP_SHIFT;
        int32_t sj = fpSy >> FP_SHIFT;
        if(static_cast<uint32_t>(si) < static_cast<uint32_t>(maxSi)
            && static_cast<uint32_t>(sj) < static_cast<uint32_t>(maxSj)) {
          uint32_t val = src[static_cast<size_t>(sj) * srcW + static_cast<size_t>(si)];
          if(!skipZero || val != 0) {
            dstRow[x] = val;
          }
        }
        fpSx += stepXx;
        fpSy += stepXy;
      }
    }
  }
}
