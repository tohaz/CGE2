#include "AUILib.h"

namespace aui {

  void DrawTextEx(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t absX, int32_t absY,
      int32_t drawW, int32_t drawH, const std::string &text, FT_Face face, uint32_t fontSize, AUIHAlign hAlign,
      AUIVAlign vAlign, int32_t xOffset, uint32_t textColor, int32_t maxContentWidth) {
    if(text.empty() || !face)
      return;
    AUI *engine = static_cast<AUI*>(face->generic.data);
    if(!engine)
      return;
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    int32_t maxAscent = 0, maxDescent = 0, totalWidth = 0;
    for(char c : text) {
      FT_UInt glyph_index = FT_Get_Char_Index(face, static_cast<FT_ULong>(c));
      FT_Fixed advance = engine->GetCachedAdvance(glyph_index, fontSize);
      totalWidth += static_cast<int32_t>(advance >> 6);
      int32_t asc = 0, desc = 0;
      bool found = false;
      if(c >= 32 && c <= 126) {
        const CachedGlyph *pre = engine->GetPreRenderedGlyph(static_cast<uint8_t>(c), fontSize);
        if(pre) {
          asc = pre->asc;
          desc = pre->desc;
          found = true;
        }
      }
      if(!found) {
        uint64_t mkey = (static_cast<uint64_t>(glyph_index) << 32) | fontSize;
        auto &metricsCache = engine->GetMetricsCache();
        auto it = metricsCache.find(mkey);
        if(it != metricsCache.end()) {
          asc = it->second.first;
          desc = it->second.second;
        }
        else {
          if(FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_NO_HINTING) == 0) {
            FT_GlyphSlot slot = face->glyph;
            asc = static_cast<int32_t>(slot->bitmap_top);
            desc = static_cast<int32_t>(slot->bitmap.rows) - asc;
            metricsCache[mkey] = { asc, desc };
          }
          else {
            asc = static_cast<int32_t>(fontSize);
            desc = 0;
          }
        }
      }
      if(asc > maxAscent)
        maxAscent = asc;
      if(desc > maxDescent)
        maxDescent = desc;
    }
    int32_t textHeight = maxAscent + maxDescent;
    if(textHeight == 0)
      textHeight = static_cast<int32_t>(fontSize);
    int32_t startX = absX - xOffset;
    if(hAlign == AUIHAlign::center)
      startX += (maxContentWidth - totalWidth) / 2;
    else if(hAlign == AUIHAlign::right)
      startX += (maxContentWidth - totalWidth);
    int32_t baselineY = absY;
    if(vAlign == AUIVAlign::center)
      baselineY += (drawH - textHeight) / 2;
    else if(vAlign == AUIVAlign::bottom)
      baselineY += drawH - textHeight;
    baselineY += textHeight;
    int32_t clipL = absX, clipR = absX + drawW, clipT = absY, clipB = absY + drawH;
    const size_t pW = static_cast<size_t>(parentWidth);
    const size_t totalPixels = pW * static_cast<size_t>(parentHeight);
    uint32_t colorNoAlpha = textColor & 0x00FFFFFFU;
    const uint32_t col_r = (colorNoAlpha >> 16) & 0xFF;
    const uint32_t col_g = (colorNoAlpha >> 8) & 0xFF;
    const uint32_t col_b = colorNoAlpha & 0xFF;
    int32_t penX = startX;
    for(char c : text) {
      if(c >= 32 && c <= 126) {
        const CachedGlyph *pre = engine->GetPreRenderedGlyph(static_cast<uint8_t>(c), fontSize);
        if(pre) {
          int32_t glyphLeft = penX + pre->left;
          int32_t glyphRight = glyphLeft + pre->width;
          if(glyphRight <= clipL) {
            penX += pre->advance;
            continue;
          }
          if(glyphLeft >= clipR)
            break;
          int32_t glyphTop = baselineY - pre->top;
          int32_t glyphBottom = glyphTop + pre->rows;
          if(glyphBottom > clipT && glyphTop < clipB) {
            const size_t rows = static_cast<size_t>(pre->rows);
            const size_t cols = static_cast<size_t>(pre->width);
            for(size_t row = 0; row < rows; ++row) {
              int32_t destY = glyphTop + static_cast<int32_t>(row);
              if(destY < clipT || destY >= clipB)
                continue;
              const size_t rowOffset = static_cast<size_t>(destY) * pW;
              const size_t bitmapRowStart = row * cols;
              for(size_t col = 0; col < cols; ++col) {
                int32_t destX = glyphLeft + static_cast<int32_t>(col);
                if(destX < clipL || destX >= clipR)
                  continue;
                const size_t idx = rowOffset + static_cast<size_t>(destX);
                if(idx >= totalPixels)
                  continue;
                uint8_t alpha = pre->bitmap[bitmapRowStart + col];
                if(alpha == 0)
                  continue;
                uint32_t bg = buffer[idx];
                uint32_t inv_alpha = 255 - alpha;
                uint32_t bg_r = (bg >> 16) & 0xFF;
                uint32_t bg_g = (bg >> 8) & 0xFF;
                uint32_t bg_b = bg & 0xFF;
                uint32_t r = (col_r * alpha + bg_r * inv_alpha + 128) >> 8;
                uint32_t g = (col_g * alpha + bg_g * inv_alpha + 128) >> 8;
                uint32_t b = (col_b * alpha + bg_b * inv_alpha + 128) >> 8;
                buffer[idx] = (r << 16) | (g << 8) | b;
              }
            }
          }
          penX += pre->advance;
          continue;
        }
      }
      FT_UInt glyph_index = FT_Get_Char_Index(face, static_cast<FT_ULong>(c));
      FT_Glyph cached = engine->GetCachedGlyph(glyph_index, fontSize, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
      if(!cached) {
        penX += static_cast<int32_t>(engine->GetCachedAdvance(glyph_index, fontSize) >> 6);
        continue;
      }
      FT_Glyph glyph = nullptr;
      if(FT_Glyph_Copy(cached, &glyph) != 0) {
        penX += static_cast<int32_t>(engine->GetCachedAdvance(glyph_index, fontSize) >> 6);
        continue;
      }
      if(glyph->format != FT_GLYPH_FORMAT_BITMAP) {
        if(FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, 1) != 0) {
          FT_Done_Glyph(glyph);
          penX += static_cast<int32_t>(engine->GetCachedAdvance(glyph_index, fontSize) >> 6);
          continue;
        }
      }
      FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph) glyph;
      FT_Bitmap *bitmap = &bitmap_glyph->bitmap;
      int32_t glyphLeft = penX + bitmap_glyph->left;
      int32_t glyphRight = glyphLeft + static_cast<int32_t>(bitmap->width);
      if(glyphRight <= clipL) {
        FT_Done_Glyph(glyph);
        penX += static_cast<int32_t>(engine->GetCachedAdvance(glyph_index, fontSize) >> 6);
        continue;
      }
      if(glyphLeft >= clipR) {
        FT_Done_Glyph(glyph);
        break;
      }
      int32_t glyphTop = baselineY - bitmap_glyph->top;
      int32_t glyphBottom = glyphTop + static_cast<int32_t>(bitmap->rows);
      if(glyphBottom > clipT && glyphTop < clipB) {
        const size_t rows = static_cast<size_t>(bitmap->rows);
        const size_t cols = static_cast<size_t>(bitmap->width);
        for(size_t row = 0; row < rows; ++row) {
          int32_t destY = glyphTop + static_cast<int32_t>(row);
          if(destY < clipT || destY >= clipB)
            continue;
          const size_t rowOffset = static_cast<size_t>(destY) * pW;
          const size_t bitmapRowStart = row * cols;
          for(size_t col = 0; col < cols; ++col) {
            int32_t destX = glyphLeft + static_cast<int32_t>(col);
            if(destX < clipL || destX >= clipR)
              continue;
            const size_t idx = rowOffset + static_cast<size_t>(destX);
            if(idx >= totalPixels)
              continue;
            uint8_t alpha = bitmap->buffer[bitmapRowStart + col];
            if(alpha == 0)
              continue;
            uint32_t bg = buffer[idx];
            uint32_t inv_alpha = 255 - alpha;
            uint32_t bg_r = (bg >> 16) & 0xFF;
            uint32_t bg_g = (bg >> 8) & 0xFF;
            uint32_t bg_b = bg & 0xFF;
            uint32_t r = (col_r * alpha + bg_r * inv_alpha + 128) >> 8;
            uint32_t g = (col_g * alpha + bg_g * inv_alpha + 128) >> 8;
            uint32_t b = (col_b * alpha + bg_b * inv_alpha + 128) >> 8;
            buffer[idx] = (r << 16) | (g << 8) | b;
          }
        }
      }
      FT_Done_Glyph(glyph);
      penX += static_cast<int32_t>(engine->GetCachedAdvance(glyph_index, fontSize) >> 6);
    }
  }

}// namespace aui

