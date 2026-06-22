#include "AUILib.h"

namespace aui {

  void ClipRect(int32_t &x, int32_t &y, int32_t &w, int32_t &h, int32_t parentW, int32_t parentH) {
    if(x < 0) {
      w += x;
      x = 0;
    }
    if(y < 0) {
      h += y;
      y = 0;
    }
    if(x + w > parentW)
      w = parentW - x;
    if(y + h > parentH)
      h = parentH - y;
    if(w <= 0 || h <= 0) {
      w = h = 0;
    }
  }

  UNUSED static FT_Error ftc_face_requester(UNUSED FTC_FaceID face_id,
  UNUSED FT_Library library, FT_Pointer request_data, FT_Face *aface) {
    AUI *au = static_cast<AUI*>(request_data);
    *aface = au->GetDefaultFontFace();
    return 0;
  }

  std::string NumberToBaseString(UINT64 n) {
    D3("entering with '{}', alphabet len '{}'", n, BaseAlphabet.size())
    std::string result = "";
    do {
      result += BaseAlphabet[n % BaseAlphabet.size()];
      n = n / BaseAlphabet.size();
      if(n > 0) {
        n--;
      }
      else {
        break;
      }
    } while (true);
    std::reverse(result.begin(), result.end());
    D3("'{}'", result.c_str())
    return result;
  }

  AUIKeyCode translate_keysym_to_keycode(xcb_keysym_t sym) {
    switch (sym) {
      case XK_Return:
        return AUIKeyCode::Enter;
      case XK_BackSpace:
        return AUIKeyCode::Backspace;
      case XK_Delete:
        return AUIKeyCode::Delete;
      case XK_Insert:
        return AUIKeyCode::Insert;
      case XK_Left:
        return AUIKeyCode::Left;
      case XK_Right:
        return AUIKeyCode::Right;
      case XK_Up:
        return AUIKeyCode::Up;
      case XK_Down:
        return AUIKeyCode::Down;
      case XK_Home:
        return AUIKeyCode::Home;
      case XK_End:
        return AUIKeyCode::End;
      case XK_Tab:
        return AUIKeyCode::Tab;
      case XK_Escape:
        return AUIKeyCode::Escape;
      case XK_space:
        return AUIKeyCode::Space;
      default:
        return AUIKeyCode::None;
    }
  }

  AUIModifier translate_modifiers(uint16_t state) {
    AUIModifier mod = AUIModifier::None;
    if(state & XCB_MOD_MASK_SHIFT)
      mod = mod | AUIModifier::Shift;
    if(state & XCB_MOD_MASK_CONTROL)
      mod = mod | AUIModifier::Ctrl;
    if(state & XCB_MOD_MASK_1)
      mod = mod | AUIModifier::Alt;
    if(state & XCB_MOD_MASK_4)
      mod = mod | AUIModifier::Super;
    return mod;
  }

  AUIKeyCode translate_keysym(xcb_keysym_t sym) {
    switch (sym) {
      case XK_Return:
        return AUIKeyCode::Enter;
      case XK_BackSpace:
        return AUIKeyCode::Backspace;
      case XK_Delete:
        return AUIKeyCode::Delete;
      case XK_Insert:
        return AUIKeyCode::Insert;
      case XK_Left:
        return AUIKeyCode::Left;
      case XK_Right:
        return AUIKeyCode::Right;
      case XK_Up:
        return AUIKeyCode::Up;
      case XK_Down:
        return AUIKeyCode::Down;
      case XK_Home:
        return AUIKeyCode::Home;
      case XK_End:
        return AUIKeyCode::End;
      case XK_Tab:
        return AUIKeyCode::Tab;
      case XK_Escape:
        return AUIKeyCode::Escape;
      case XK_space:
        return AUIKeyCode::Space;
      default:
        return AUIKeyCode::None;
    }
  }

  int32_t find_closest_strike(FT_Face face, int32_t target_ppem) {
    if(face->num_fixed_sizes == 0)
      return -1;
    int32_t best = 0;
    int32_t bestDiff = abs((int32_t) (face->available_sizes[0].y_ppem - target_ppem));
    for(int32_t i = 1; i < face->num_fixed_sizes; ++i) {
      int32_t diff = abs((int32_t) (face->available_sizes[i].y_ppem - target_ppem));
      if(diff < bestDiff) {
        bestDiff = diff;
        best = i;
      }
    }
    return best;
  }

  uint8_t* scale_bgra_bitmap(const uint8_t *src, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
      int32_t srcPitch, int32_t &dstPitch) {
    if(srcW <= 0 || srcH <= 0 || dstW <= 0 || dstH <= 0)
      return nullptr;
    dstPitch = dstW * 4;
    uint8_t *dst = new uint8_t[dstH * dstPitch];
    for(int32_t y = 0; y < dstH; ++y) {
      int32_t srcY = y * srcH / dstH;
      const uint8_t *srcRow = src + srcY * srcPitch;
      uint8_t *dstRow = dst + y * dstPitch;
      for(int32_t x = 0; x < dstW; ++x) {
        int32_t srcX = x * srcW / dstW;
        const uint8_t *srcPix = srcRow + srcX * 4;
        uint8_t *dstPix = dstRow + x * 4;
        dstPix[0] = srcPix[0];
        dstPix[1] = srcPix[1];
        dstPix[2] = srcPix[2];
        dstPix[3] = srcPix[3];
      }
    }
    return dst;
  }

  struct TextLayout {
      int32_t totalWidth = 0;
      int32_t textHeight = 0;
      int32_t startX = 0;
      int32_t baselineY = 0;
      int32_t clipL = 0;
      int32_t clipR = 0;
      int32_t clipT = 0;
      int32_t clipB = 0;
  };

// ---- Helper: Safe UTF-8 Parsing ----
  static uint32_t GetNextCodepoint(const uint8_t *&ptr) {
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

// ---- Helper: Unified Layout Metric Engine ----
  static TextLayout CalculateTextLayout(const std::string &text, FT_Face face, uint32_t fontSize, AUI *engine,
      int32_t absX, int32_t absY, int32_t drawW, int32_t drawH, AUIHAlign hAlign, AUIVAlign vAlign, int32_t xOffset,
      int32_t maxContentWidth, uint32_t parentWidth, uint32_t parentHeight) {
    TextLayout layout;
    int32_t maxAscent = 0, maxDescent = 0;
    const uint8_t *ptr = reinterpret_cast<const uint8_t*>(text.c_str());
    while (*ptr) {
      uint32_t codepoint = GetNextCodepoint(ptr);
      if(codepoint == 0)
        continue;
      if(codepoint <= 126) {
        FT_UInt glyph_index = FT_Get_Char_Index(face, codepoint);
        FT_Fixed advance = engine->GetCachedAdvance(glyph_index, fontSize);
        layout.totalWidth += static_cast<int32_t>(advance >> 6);

        int32_t asc = 0, desc = 0;
        const CachedGlyph *pre = engine->GetPreRenderedGlyph(static_cast<uint8_t>(codepoint), fontSize);
        if(pre) {
          asc = pre->asc;
          desc = pre->desc;
        }
        else {
          uint64_t mkey = (static_cast<uint64_t>(glyph_index) << 32) | fontSize;
          auto &metricsCache = engine->GetMetricsCache();
          auto it = metricsCache.find(mkey);
          if(it != metricsCache.end()) {
            asc = it->second.first;
            desc = it->second.second;
          }
          else if(FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_HINTING) == 0) {
            FT_GlyphSlot slot = face->glyph;
            asc = static_cast<int32_t>(slot->bitmap_top);
            desc = static_cast<int32_t>(slot->bitmap.rows - static_cast<uint32_t>(asc));
            metricsCache[mkey] = { asc, desc };
          }
          else {
            asc = static_cast<int32_t>(fontSize);
            desc = 0;
          }
        }
        maxAscent = std::max(maxAscent, asc);
        maxDescent = std::max(maxDescent, desc);
      }
      else {
        layout.totalWidth += static_cast<int32_t>(fontSize);
      }
    }
    if(maxAscent == 0 && maxDescent == 0) {
      maxAscent = static_cast<int32_t>(fontSize);
    }
    layout.textHeight = maxAscent + maxDescent;
    layout.startX = absX - xOffset;
    if(hAlign == AUIHAlign::center)
      layout.startX += (maxContentWidth - layout.totalWidth) / 2;
    else if(hAlign == AUIHAlign::right)
      layout.startX += (maxContentWidth - layout.totalWidth);
    layout.baselineY = absY;
    if(vAlign == AUIVAlign::center)
      layout.baselineY += (drawH - layout.textHeight) / 2;
    else if(vAlign == AUIVAlign::bottom)
      layout.baselineY += drawH - layout.textHeight;
    layout.baselineY += maxAscent;
    layout.clipL = std::max(0, absX);
    layout.clipR = std::min(static_cast<int32_t>(parentWidth), absX + drawW);
    layout.clipT = std::max(0, absY);
    layout.clipB = std::min(static_cast<int32_t>(parentHeight), absY + drawH);
    return layout;
  }
// ---- Pixel Blending Utilities ----
  static inline void BlendPixelRGBA(uint32_t *dest, uint32_t color) {
    uint8_t a = static_cast<uint8_t>((color >> 24) & 0xFF);
    if(a == 0)
      return;
    if(a == 255) {
      *dest = color;
    }
    else {
      uint32_t bg = *dest;
      uint32_t inv = 255 - a;
      uint32_t r = ((((color >> 16) & 0xFF) * a + ((bg >> 16) & 0xFF) * inv + 128) >> 8) << 16;
      uint32_t g = ((((color >> 8) & 0xFF) * a + ((bg >> 8) & 0xFF) * inv + 128) >> 8) << 8;
      uint32_t b = (((color & 0xFF) * a + (bg & 0xFF) * inv + 128) >> 8);
      *dest = r | g | b;
    }
  }
  static inline void BlendPixelGrayscale(uint32_t *dest, uint8_t alpha, uint32_t col_r, uint32_t col_g,
      uint32_t col_b) {
    if(alpha == 0)
      return;
    uint32_t bg = *dest;
    uint32_t inv = 255 - alpha;
    uint32_t r = ((col_r * alpha + ((bg >> 16) & 0xFF) * inv + 128) >> 8) << 16;
    uint32_t g = ((col_g * alpha + ((bg >> 8) & 0xFF) * inv + 128) >> 8) << 8;
    uint32_t b = (col_b * alpha + (bg & 0xFF) * inv + 128) >> 8;
    *dest = r | g | b;
  }
// ---- Isolated Render Block 1: Pre-rendered ASCII ----
  static void RenderPreRenderedGlyph(uint32_t *buffer, size_t pW, const TextLayout &layout, int32_t penX,
      const CachedGlyph *pre, uint32_t col_r, uint32_t col_g, uint32_t col_b) {
    int32_t glyphLeft = penX + pre->left;
    int32_t glyphRight = glyphLeft + pre->width;
    if(glyphRight <= layout.clipL || glyphLeft >= layout.clipR)
      return;
    int32_t glyphTop = layout.baselineY - pre->top;
    int32_t glyphBottom = glyphTop + pre->rows;
    if(glyphBottom <= layout.clipT || glyphTop >= layout.clipB)
      return;
    for(int32_t row = 0; row < pre->rows; ++row) {
      int32_t destY = glyphTop + row;
      if(destY < layout.clipT || destY >= layout.clipB)
        continue;
      size_t rowOffset = static_cast<size_t>(destY) * pW;
      size_t bmpRow = static_cast<size_t>(row) * static_cast<size_t>(pre->width);
      for(int32_t col = 0; col < pre->width; ++col) {
        int32_t destX = glyphLeft + col;
        if(destX < layout.clipL || destX >= layout.clipR)
          continue;
        uint8_t alpha = pre->bitmap[bmpRow + static_cast<size_t>(col)];
        BlendPixelGrayscale(&buffer[rowOffset + static_cast<size_t>(destX)], alpha, col_r, col_g, col_b);
      }
    }
  }
// ---- Isolated Render Block 2: Bitmap Blitting (Any Pixel Mode) ----
  static void BlitGlyphBitmap(uint32_t *buffer, size_t pW, const TextLayout &layout, int32_t glyphLeft,
      int32_t glyphTop, const FT_Bitmap *bitmap, uint32_t col_r, uint32_t col_g, uint32_t col_b) {
    for(int32_t row = 0; row < static_cast<int32_t>(bitmap->rows); ++row) {
      int32_t destY = glyphTop + row;
      if(destY < layout.clipT || destY >= layout.clipB)
        continue;
      const uint8_t *src = bitmap->buffer + row * bitmap->pitch;
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

// ---- Isolated Render Block 3: Dynamic Generation & Rescaling ----
  static void RenderFreshEmoji(uint32_t *buffer, size_t pW, const TextLayout &layout, int32_t penX, uint32_t fontSize,
      FT_UInt glyph_index, FT_Bitmap *bitmap, AUI *engine) {
    int32_t targetSize = static_cast<int32_t>(fontSize);
    int32_t srcW = static_cast<int32_t>(bitmap->width);
    int32_t srcH = static_cast<int32_t>(bitmap->rows);

    if(srcW != targetSize || srcH != targetSize) {
      int32_t dstW = targetSize, dstH = targetSize, dstPitch = 0;
      uint8_t *rawScaledBuffer = scale_bgra_bitmap(bitmap->buffer, srcW, srcH, dstW, dstH, bitmap->pitch, dstPitch);
      if(!rawScaledBuffer)
        return;

      std::unique_ptr<uint8_t[]> managedScaled(rawScaledBuffer);
      uint64_t cacheKey = (static_cast<uint64_t>(glyph_index) << 32) | fontSize;
      auto &cache = engine->mScaledEmojiCache;
      if(cache.find(cacheKey) == cache.end()) {
        std::vector<uint8_t> data(12 + static_cast<size_t>(dstH * dstPitch));
        std::memcpy(data.data(), &dstW, 4);
        std::memcpy(data.data() + 4, &dstH, 4);
        std::memcpy(data.data() + 8, &dstPitch, 4);
        std::memcpy(data.data() + 12, managedScaled.get(), static_cast<size_t>(dstH * dstPitch));
        cache[cacheKey] = std::move(data);
      }

      FT_Bitmap scaledBitmap;
      scaledBitmap.rows = static_cast<uint32_t>(dstH);
      scaledBitmap.width = static_cast<uint32_t>(dstW);
      scaledBitmap.pitch = dstPitch;
      scaledBitmap.buffer = managedScaled.get();
      scaledBitmap.pixel_mode = FT_PIXEL_MODE_BGRA;

      BlitGlyphBitmap(buffer, pW, layout, penX, layout.baselineY - dstH, &scaledBitmap, 0, 0, 0);
    }
    else {
      BlitGlyphBitmap(buffer, pW, layout, penX, layout.baselineY - targetSize, bitmap, 0, 0, 0);
    }
  }

// ------------------------------------------------------------------
// Main Clean Orchestrator API
// ------------------------------------------------------------------
  void DrawTextEx(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t absX, int32_t absY,
      int32_t drawW, int32_t drawH, const std::string &text, FT_Face face, uint32_t fontSize, AUIHAlign hAlign,
      AUIVAlign vAlign, int32_t xOffset, uint32_t textColor, int32_t maxContentWidth) {
    if(text.empty() || !face || parentWidth == 0 || parentHeight == 0)
      return;
    AUI *engine = static_cast<AUI*>(face->generic.data);
    if(!engine)
      return;
    FT_Face fallbackFace = engine->GetFallbackFace();
    FT_Select_Charmap(face, FT_ENCODING_UNICODE);
    if(fallbackFace)
      FT_Select_Charmap(fallbackFace, FT_ENCODING_UNICODE);
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    if(fallbackFace) {
      if(FT_IS_SCALABLE(fallbackFace)) {
        FT_Set_Pixel_Sizes(fallbackFace, 0, fontSize);
      }
      else {
        int32_t best = 0, bestDiff = INT_MAX;
        for(int32_t i = 0; i < fallbackFace->num_fixed_sizes; ++i) {
          int32_t diff = std::abs(
              static_cast<int32_t>(fallbackFace->available_sizes[i].y_ppem) - static_cast<int32_t>(fontSize));
          if(diff < bestDiff) {
            bestDiff = diff;
            best = i;
          }
        }
        FT_Select_Size(fallbackFace, best);
      }
    }
    TextLayout layout = CalculateTextLayout(text, face, fontSize, engine, absX, absY, drawW, drawH, hAlign, vAlign,
        xOffset, maxContentWidth, parentWidth, parentHeight);
    if(layout.clipL >= layout.clipR || layout.clipT >= layout.clipB)
      return;
    const size_t pW = static_cast<size_t>(parentWidth);
    const uint32_t colorNoAlpha = textColor & 0x00FFFFFFU;
    const uint32_t col_r = (colorNoAlpha >> 16) & 0xFF, col_g = (colorNoAlpha >> 8) & 0xFF, col_b = colorNoAlpha & 0xFF;
    int32_t penX = layout.startX;
    const uint8_t *ptr = reinterpret_cast<const uint8_t*>(text.c_str());
    while (*ptr) {
      uint32_t codepoint = GetNextCodepoint(ptr);
      if(codepoint == 0)
        continue;
      if(codepoint >= 32 && codepoint <= 126) {
        const CachedGlyph *pre = engine->GetPreRenderedGlyph(static_cast<uint8_t>(codepoint), fontSize);
        if(pre) {
          RenderPreRenderedGlyph(buffer, pW, layout, penX, pre, col_r, col_g, col_b);
          penX += pre->advance;
          continue;
        }
      }
      FT_Face currentFace = face;
      FT_UInt glyph_index = 0;
      if(codepoint > 127 && fallbackFace) {
        currentFace = fallbackFace;
        glyph_index = FT_Get_Char_Index(currentFace, codepoint);
      }
      else {
        glyph_index = FT_Get_Char_Index(currentFace, codepoint);
      }
      if(glyph_index == 0 && fallbackFace && codepoint <= 127) {
        currentFace = fallbackFace;
        glyph_index = FT_Get_Char_Index(currentFace, codepoint);
      }
      if(glyph_index == 0) {
        penX += static_cast<int32_t>(fontSize);
        continue;
      }
// Check Scaled Cache
      bool haveCached = false;
      if(codepoint > 127) {
        uint64_t cacheKey = (static_cast<uint64_t>(glyph_index) << 32) | fontSize;
        auto &cache = engine->mScaledEmojiCache;
        auto it = cache.find(cacheKey);
        if(it != cache.end() && it->second.size() >= 12) {
          int32_t cachedW = *reinterpret_cast<const int32_t*>(it->second.data());
          int32_t cachedH = *reinterpret_cast<const int32_t*>(it->second.data() + 4);
          int32_t cachedPitch = *reinterpret_cast<const int32_t*>(it->second.data() + 8);
          FT_Bitmap cachedBitmap;
          cachedBitmap.rows = static_cast<uint32_t>(cachedH);
          cachedBitmap.width = static_cast<uint32_t>(cachedW);
          cachedBitmap.pitch = cachedPitch;
          cachedBitmap.buffer = const_cast<uint8_t*>(it->second.data() + 12);
          cachedBitmap.pixel_mode = FT_PIXEL_MODE_BGRA;
          BlitGlyphBitmap(buffer, pW, layout, penX, layout.baselineY - cachedH, &cachedBitmap, 0, 0, 0);
          penX += cachedW;
          haveCached = true;
        }
      }
      if(haveCached)
        continue;
      FT_Error err = FT_Load_Glyph(currentFace, glyph_index, FT_LOAD_RENDER | FT_LOAD_COLOR | FT_LOAD_NO_HINTING);
      if(err) {
        if(FT_Load_Glyph(currentFace, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING)) {
          penX += static_cast<int32_t>(fontSize);
          continue;
        }
      }
      FT_GlyphSlot slot = currentFace->glyph;
      if(slot->bitmap.rows == 0 || slot->bitmap.width == 0) {
        penX += static_cast<int32_t>(slot->advance.x >> 6);
        continue;
      }
      if(codepoint > 127 && slot->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA) {
        RenderFreshEmoji(buffer, pW, layout, penX, fontSize, glyph_index, &slot->bitmap, engine);
        penX += static_cast<int32_t>(fontSize);
      }
      else {
        BlitGlyphBitmap(buffer, pW, layout, penX + slot->bitmap_left, layout.baselineY - slot->bitmap_top,
            &slot->bitmap, col_r, col_g, col_b);
        penX += static_cast<int32_t>(slot->advance.x >> 6);
      }
    }
  }
// ---- Helper: Scale and Rotate BGRA Bitmaps ----
  static uint8_t* rotate_and_scale_bgra_bitmap(const uint8_t *src, int32_t srcW, int32_t srcH, int32_t dstW,
      int32_t dstH, int32_t srcPitch, double angle, int32_t &dstPitch) {
    if(srcW <= 0 || srcH <= 0 || dstW <= 0 || dstH <= 0)
      return nullptr;
    dstPitch = dstW * 4;
    uint8_t *dst = new uint8_t[static_cast<size_t>(dstH * dstPitch)];
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
      uint8_t *dstRow = dst + y * dstPitch;
      for(int32_t x = 0; x < dstW; ++x) {
        float dx = static_cast<float>(x) - cxDst;

        int32_t srcX = static_cast<int32_t>((dx * cosA - dy * sinA) * scaleX + cxSrc);
        int32_t srcY = static_cast<int32_t>((dx * sinA + dy * cosA) * scaleH + cySrc);

        if(srcX >= 0 && srcX < srcW && srcY >= 0 && srcY < srcH) {
          const uint8_t *srcPix = src + srcY * srcPitch + srcX * 4;
          uint8_t *dstPix = dstRow + x * 4;
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
  static void RenderFreshEmojiRotated(uint32_t *buffer, size_t pW, const TextLayout &layout, double centerX,
      double centerY, uint32_t fontSize, FT_Bitmap *bitmap, double angle) {
    int32_t targetSize = static_cast<int32_t>(fontSize);
    int32_t srcW = static_cast<int32_t>(bitmap->width);
    int32_t srcH = static_cast<int32_t>(bitmap->rows);

    int32_t dstW = targetSize, dstH = targetSize, dstPitch = 0;
    uint8_t *rawScaledBuffer = rotate_and_scale_bgra_bitmap(bitmap->buffer, srcW, srcH, dstW, dstH, bitmap->pitch,
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
// ------------------------------------------------------------------
// Main Drawing Orchestrator Implementation - STABILIZED TRACKING
// ------------------------------------------------------------------
  void DrawTextEx(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, const ARect &bounds,
      const std::string &text, FT_Face face, const ATextStyle &style) {
    if(text.empty() || !face || parentWidth == 0 || parentHeight == 0)
      return;
    AUI *engine = static_cast<AUI*>(face->generic.data);
    if(!engine)
      return;

    FT_Face fallbackFace = engine->GetFallbackFace();
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
// Reset transform state completely for layout metric tracking pass
    FT_Set_Transform(face, nullptr, nullptr);
    if(fallbackFace) {
      FT_Set_Transform(fallbackFace, nullptr, nullptr);
    }
    int32_t totalWidth = 0;
    int32_t maxAscent = static_cast<int32_t>(face->size->metrics.ascender >> 6);
    int32_t maxDescent = static_cast<int32_t>(face->size->metrics.descender >> 6);
    int32_t fontHeight = maxAscent - maxDescent;
// Vector to hold cache layout steps for Pass 2 to eliminate FreeType matrix rounding drift
    std::vector<double> glyphAdvances;
    glyphAdvances.reserve(text.size());
// =====================================================================
// PASS 1: Fixed Measurement & Step-Caching Pass
// =====================================================================
    {
      const uint8_t *layoutPtr = reinterpret_cast<const uint8_t*>(text.c_str());
      while (*layoutPtr != '\0') {
        uint32_t cp = GetNextCodepoint(layoutPtr);
        if(cp == 0)
          continue;
        if(cp > 127) {
          double adv = static_cast<double>(style.fontSize);
          totalWidth += static_cast<int32_t>(adv);
          glyphAdvances.push_back(adv);
          continue;
        }
        FT_UInt glyph_index = FT_Get_Char_Index(face, cp);
        if(glyph_index == 0) {
          double adv = static_cast<double>(style.fontSize);
          totalWidth += static_cast<int32_t>(adv);
          glyphAdvances.push_back(adv);
          continue;
        }
        if(FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING)) {
          double adv = static_cast<double>(style.fontSize);
          totalWidth += static_cast<int32_t>(adv);
          glyphAdvances.push_back(adv);
          continue;
        }
// Cache high-precision unrotated fractional steps directly
        double adv = static_cast<double>(face->glyph->advance.x) / 64.0;
        totalWidth += static_cast<int32_t>(face->glyph->advance.x >> 6);
        glyphAdvances.push_back(adv);
      }
    }
    int32_t penX = bounds.x;
    int32_t penY = bounds.y + maxAscent;
// Explicitly cast bounds variables to signed int32_t to satisfy -Wsign-conversion
    int32_t bWidth = static_cast<int32_t>(bounds.w);
    int32_t bHeight = static_cast<int32_t>(bounds.h);
    if(style.hAlign == AUIHAlign::center) {
      penX += (bWidth - totalWidth) / 2;
    }
    else if(style.hAlign == AUIHAlign::right) {
      penX += (bWidth - totalWidth);
    }
    if(style.vAlign == AUIVAlign::center) {
      penY += (bHeight - fontHeight) / 2;
    }
    else if(style.vAlign == AUIVAlign::bottom) {
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
    TextLayout lLoc { totalWidth, fontHeight, bounds.x, static_cast<int32_t>(std::round(boxCenterY)), 0,
        static_cast<int32_t>(parentWidth), 0, static_cast<int32_t>(parentHeight) };
// =====================================================================
// PASS 2: Deterministic Render Loop (Zero-Drift Execution)
// =====================================================================
    const uint8_t *ptr = reinterpret_cast<const uint8_t*>(text.c_str());
    double unrotatedPenX = static_cast<double>(penX);
    double sizeD = static_cast<double>(style.fontSize);
    size_t advanceIdx = 0;
    while (*ptr != '\0') {
      uint32_t cp = GetNextCodepoint(ptr);
      if(cp == 0)
        continue;
// Grab the immutable baseline advance step for this exact character position
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
// Map absolute unrotated pen spacing strictly via trigonometry to eradicate rounding artifacts
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
// Regular Text / Monochrome / Grayscale Branch
      else {
        if(slot->bitmap.rows == 0 || slot->bitmap.width == 0) {
          unrotatedPenX += baselineAdvance;
          continue;
        }
        int32_t drawX = static_cast<int32_t>(std::round(currentTrackingX));
        int32_t drawY = static_cast<int32_t>(std::round(currentTrackingY));
        uint32_t colorNoAlpha = style.color & 0x00FFFFFFU;
        uint32_t col_r = (colorNoAlpha >> 16) & 0xFF;
        uint32_t col_g = (colorNoAlpha >> 8) & 0xFF;
        uint32_t col_b = colorNoAlpha & 0xFF;
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
}

