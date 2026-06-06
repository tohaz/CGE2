#include "AUILib.h"
#include "AWidget.h"

namespace aui {

  AWidget::AWidget() {
    D2("");
  }

  void AWidget::Draw(UNUSED uint32_t *buffer, UNUSED uint32_t parentWidth, UNUSED uint32_t parentHeight,
      UNUSED int32_t offsetX, UNUSED int32_t offsetY) const {
    E("default method called")
  }

  void AWidget::InitWidgetProperties(uint64_t widgetId, AUI *engine, AWindow *window, AWidget *parent,
      AUIWidgetType type) {
    mId = widgetId;
    mEnginePtr = engine;
    mParentWindow = window;
    mParentWidget = parent;
    mWidgetType = type;
  }

  void AWidget::Show() {
    mVisible = true;
    if(mParentWindow) {
      mParentWindow->Draw();
    }
    else if(mParentWidget && mParentWidget->mParentWindow) {
      mParentWidget->mParentWindow->Draw();
    }
  }

  void AWidget::Hide() {
    mVisible = false;
    if(mParentWindow) {
      mParentWindow->Draw();
    }
    else if(mParentWidget && mParentWidget->mParentWindow) {
      mParentWidget->mParentWindow->Draw();
    }
  }

  void AWidget::AddWidget(std::unique_ptr<AWidget> widg) {
// Child inherits engine and parent window from this widget
    widg->InitWidgetProperties(0, mEnginePtr, mParentWindow, this, widg->mWidgetType);
    mWidg.push_back(std::move(widg));
  }

  void AWidget::GetAbsolutePosition(int32_t &outX, int32_t &outY) const {
    outX = mX;
    outY = mY;
    if(mParentWidget) {
      int32_t parentX, parentY;
      mParentWidget->GetAbsolutePosition(parentX, parentY);
      outX += parentX;
      outY += parentY;
    }
  }

  void AWidget::Move(int32_t x, int32_t y) {
    mX = x;
    mY = y;
//    if(mParentWindow)
//      mParentWindow->Draw();
  }

  void AWidget::Resize(uint32_t szx, uint32_t szy) {
    mSizeX = szx;
    mSizeY = szy;
    mTextMetricsValid = false;
//    if(mParentWindow)
//      mParentWindow->Draw();
  }

  void AWidget::SetBGColor(uint32_t color) {
    mBGColor = color;
//    if(mParentWindow)
//      mParentWindow->Draw();
  }

  bool AWidget::DispatchClick(int32_t parentX, int32_t parentY, bool pressed) {
    if(!mVisible || !mEnabled)
      return false;
    if(parentX >= mX && parentX < mX + static_cast<int32_t>(mSizeX) && parentY >= mY
        && parentY < mY + static_cast<int32_t>(mSizeY)) {
      int32_t localX = parentX - mX;
      int32_t localY = parentY - mY;
// Let children handle first (deepest first)
      for (auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
        if((*it)->DispatchClick(localX, localY, pressed))
          return true;
      }
// No child consumed – ask this widget
      return OnMouseClick(localX, localY, pressed);
    }
    return false;
  }

  void AWidget::SetClickCallback(ClickCallback callback, void *userData = nullptr) {
    mClickCallback = std::move(callback);
    mCallbackUserData = userData;
  }

  void AWidget::InvokeClickCallback(int32_t localX, int32_t localY, bool pressed) {
    if(mClickCallback) {
      mClickCallback(mParentWindow, this, mCallbackUserData, localX, localY, pressed);
    }
  }

  bool AWidget::OnMouseClick(int32_t localX, int32_t localY, bool pressed) {
    D2("AWidget::OnMouseClick default at ({},{}) pressed={}", localX, localY, pressed);
    bool consumed = (mClickCallback != nullptr);
    InvokeClickCallback(localX, localY, pressed);
    return consumed;
  }

  void AWidget::SetMouseMoveCallback(MouseMoveCallback callback, void *userData) {
    mMoveCallback = std::move(callback);
    mMoveUserData = userData;
  }

  void AWidget::InvokeMouseMoveCallback(int32_t localX, int32_t localY) {
    if(mMoveCallback) {
      mMoveCallback(mParentWindow, this, mMoveUserData, localX, localY);
    }
  }

  void AWidget::OnMouseMove(int32_t localX, int32_t localY) {
    D2("AWidget::OnMouseMove (base) called: localX={}, localY={}, widget type={}", localX, localY,
        static_cast<int32_t>(mWidgetType));
    InvokeMouseMoveCallback(localX, localY);
  }

  bool AWidget::DispatchMouseMove(int32_t parentX, int32_t parentY) {
    if(!mVisible || !mEnabled)
      return false;
    if(parentX >= mX && parentX < mX + static_cast<int32_t>(mSizeX) && parentY >= mY
        && parentY < mY + static_cast<int32_t>(mSizeY)) {
      int32_t localX = parentX - mX;
      int32_t localY = parentY - mY;
// Notify children (deepest first) – but for mouse move you may want only the topmost.
// Simpler: only call OnMouseMove on the directly hit widget.
      OnMouseMove(localX, localY);
      return true;
    }
    return false;
  }

// ------------------------------------------------------------------
// Text and font property setters (trigger redraw)
// ------------------------------------------------------------------
  void AWidget::SetText(const std::string &text) {
    mText = text;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AWidget::SetTextColor(uint32_t color) {
    mTextColor = color;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AWidget::SetFontSize(uint32_t size) {
    mFontSize = size;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AWidget::SetHAlignment(AUIHAlign align) {
    mHAlign = align;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AWidget::SetVAlignment(AUIVAlign align) {
    mVAlign = align;
    if(mParentWindow)
      mParentWindow->Draw();
  }

// ------------------------------------------------------------------
// Border property setters
// ------------------------------------------------------------------
  void AWidget::SetBorderThickness(uint32_t thick) {
    mBorderThick = thick;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AWidget::SetBorderColor(uint32_t color) {
    mBorderColor = color;
    if(mParentWindow)
      mParentWindow->Draw();
  }

// ------------------------------------------------------------------
// DrawBorder – draws a rectangular border around the widget
// Assumes the buffer pixel format is XRGB (24‑bit) or ARGB (ignored alpha)
// The border is drawn inside the widget area (does not enlarge widget).
// ------------------------------------------------------------------
  void AWidget::DrawBorder(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY) const {
    if(!buffer)
      return;
    if(mBorderThick == 0)
      return;
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    int32_t drawW = static_cast<int32_t>(mSizeX);
    int32_t drawH = static_cast<int32_t>(mSizeY);
    int32_t pW = static_cast<int32_t>(parentWidth);
    int32_t pH = static_cast<int32_t>(parentHeight);
// Clip to parent using int32_t
    int32_t clipX = (absX > 0) ? absX : 0;
    int32_t clipY = (absY > 0) ? absY : 0;
    int32_t clipW = drawW - (clipX - absX);
    int32_t clipH = drawH - (clipY - absY);
    if(clipW > pW - clipX)
      clipW = pW - clipX;
    if(clipH > pH - clipY)
      clipH = pH - clipY;
    if(clipW <= 0 || clipH <= 0)
      return;
// Now all clipping values are non‑negative; convert to unsigned for indexing
    uint32_t uClipX = static_cast<uint32_t>(clipX);
    uint32_t uClipY = static_cast<uint32_t>(clipY);
    uint32_t uClipW = static_cast<uint32_t>(clipW);
    uint32_t uClipH = static_cast<uint32_t>(clipH);
    uint32_t uPW = static_cast<uint32_t>(pW);
    uint32_t uPH = static_cast<uint32_t>(pH);
    uint32_t borderColor = mBorderColor & 0x00FFFFFFU;
    size_t totalPixels = static_cast<size_t>(uPW) * uPH;
    uint32_t thick = mBorderThick;
// Top and bottom borders
    for (uint32_t t = 0; t < thick && t < uClipH; ++t) {
      size_t topLine = static_cast<size_t>(uClipY + t) * uPW + uClipX;
      size_t bottomLine = static_cast<size_t>(uClipY + uClipH - 1 - t) * uPW + uClipX;
      for (uint32_t col = 0; col < uClipW; ++col) {
        size_t idxTop = topLine + col;
        size_t idxBottom = bottomLine + col;
        if(idxTop < totalPixels)
          buffer[idxTop] = borderColor;
        if(idxBottom < totalPixels)
          buffer[idxBottom] = borderColor;
      }
    }
// Left and right borders (skip corners already painted)
    if(thick * 2 < uClipH) {// ensure inner area exists
      uint32_t endRow = uClipH - thick;
      for (uint32_t t = 0; t < thick && t < uClipW; ++t) {
        for (uint32_t row = thick; row < endRow; ++row) {
          size_t leftPixel = static_cast<size_t>(uClipY + row) * uPW + uClipX + t;
          size_t rightPixel = static_cast<size_t>(uClipY + row) * uPW + uClipX + (uClipW - 1 - t);
          if(leftPixel < totalPixels)
            buffer[leftPixel] = borderColor;
          if(rightPixel < totalPixels)
            buffer[rightPixel] = borderColor;
        }
      }
    }
  }

  void AWidget::SetScrollCallback(ScrollCallback callback, void *userData) {
    mScrollCallback = std::move(callback);
    mScrollUserData = userData;
  }

  void AWidget::InvokeScrollCallback(int32_t value) {
    if(mScrollCallback) {
      mScrollCallback(mParentWindow, this, mScrollUserData, value);
    }
  }

  uint32_t AWidget::ShiftColor(uint32_t color, bool doubleShift) const {
    uint8_t a = static_cast<uint8_t>((color >> 24) & 0xFFU);
    uint8_t r = static_cast<uint8_t>((color >> 16) & 0xFFU);
    uint8_t g = static_cast<uint8_t>((color >> 8) & 0xFFU);
    uint8_t b = static_cast<uint8_t>(color & 0xFFU);
// Compute perceived luminance (simple average, or you can use weighted)
    uint32_t luminance = (static_cast<uint32_t>(r) + g + b) / 3;
// Determine shift amount and direction
    int32_t shift = static_cast<int32_t>(AUI_HL_SHIFT);
    if(doubleShift)
      shift *= 2;
// Light colors (luminance > 128) should darken (negative shift)
    if(luminance > 128)
      shift = -shift;
    auto clamp = [](int32_t val) -> uint8_t {
      if(val < 0)
        return 0;
      if(val > 255)
        return 255;
      return static_cast<uint8_t>(val);
    };
    r = clamp(static_cast<int32_t>(r) + shift);
    g = clamp(static_cast<int32_t>(g) + shift);
    b = clamp(static_cast<int32_t>(b) + shift);
    return (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8)
        | static_cast<uint32_t>(b);
  }

  void AWidget::UpdateTextMetrics() const {
    if(!mEnginePtr) {
      mCachedTextWidth = 0;
      mCachedTextHeight = 0;
      mTextMetricsValid = true;
      return;
    }
    FT_Face face = mEnginePtr->GetDefaultFontFace();
    if(!face) {
      mCachedTextWidth = 0;
      mCachedTextHeight = 0;
      mTextMetricsValid = true;
      return;
    }
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    int32_t maxAscent = 0;
    int32_t maxDescent = 0;
    int32_t totalWidth = 0;
    for (char c : mText) {
      if(FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_RENDER) != 0) {
        continue;
      }
      FT_GlyphSlot slot = face->glyph;
      totalWidth += static_cast<int32_t>(slot->advance.x >> 6);
      int32_t asc = static_cast<int32_t>(slot->bitmap_top);
      if(asc > maxAscent)
        maxAscent = asc;
// bitmap.rows is unsigned; cast to int32_t
      int32_t bitmapRows = static_cast<int32_t>(slot->bitmap.rows);
      int32_t desc = bitmapRows - static_cast<int32_t>(slot->bitmap_top);
      if(desc > maxDescent)
        maxDescent = desc;
    }
    mCachedTextWidth = totalWidth;
    mCachedTextHeight = maxAscent + maxDescent;
    if(mCachedTextHeight == 0)
      mCachedTextHeight = static_cast<int32_t>(mFontSize);
    mTextMetricsValid = true;
  }

  void AWidget::DrawText(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY) const {
    if(mText.empty())
      return;
    if(!mEnginePtr)
      return;
    FT_Face face = mEnginePtr->GetDefaultFontFace();
    if(!face)
      return;
    if(!mTextMetricsValid) {
      UpdateTextMetrics();
    }
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    int32_t clipW = static_cast<int32_t>(mSizeX);
    int32_t clipH = static_cast<int32_t>(mSizeY);
    int32_t pW = static_cast<int32_t>(parentWidth);
    int32_t pH = static_cast<int32_t>(parentHeight);
// Clip to parent (signed arithmetic)
    if(absX < 0) {
      clipW += absX;
      absX = 0;
    }
    if(absY < 0) {
      clipH += absY;
      absY = 0;
    }
    if(absX + clipW > pW)
      clipW = pW - absX;
    if(absY + clipH > pH)
      clipH = pH - absY;
    if(clipW <= 0 || clipH <= 0)
      return;
// Horizontal alignment
    int32_t startX = absX;
    int32_t widgetWidth = static_cast<int32_t>(mSizeX);
    if(mHAlign == AUIHAlign::center) {
      startX += (widgetWidth - mCachedTextWidth) / 2;
    }
    else if(mHAlign == AUIHAlign::right) {
      startX += widgetWidth - mCachedTextWidth;
    }
// Vertical alignment (baseline)
    int32_t baselineY = absY;
    int32_t widgetHeight = static_cast<int32_t>(mSizeY);
    if(mVAlign == AUIVAlign::center) {
      baselineY += (widgetHeight - mCachedTextHeight) / 2;
    }
    else if(mVAlign == AUIVAlign::bottom) {
      baselineY += widgetHeight - mCachedTextHeight;
    }
    baselineY += mCachedTextHeight;
    int32_t penX = startX;
    uint32_t colorNoAlpha = mTextColor & 0x00FFFFFFU;
    size_t totalPixels = static_cast<size_t>(pW) * static_cast<size_t>(pH);
    for (char c : mText) {
      if(FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_RENDER) != 0) {
        continue;
      }
      FT_GlyphSlot slot = face->glyph;
      FT_Bitmap *bitmap = &slot->bitmap;
      int32_t glyphLeft = penX + static_cast<int32_t>(slot->bitmap_left);
      int32_t glyphTop = baselineY - static_cast<int32_t>(slot->bitmap_top);

      int32_t bitmapRows = static_cast<int32_t>(bitmap->rows);
      int32_t bitmapWidth = static_cast<int32_t>(bitmap->width);
      size_t rowStride = static_cast<size_t>(bitmapWidth);

      for (int32_t row = 0; row < bitmapRows; ++row) {
        int32_t destY = glyphTop + row;
        if(destY < absY || destY >= absY + clipH)
          continue;
        size_t rowOffset = static_cast<size_t>(destY) * (size_t) pW;
        for (int32_t col = 0; col < bitmapWidth; ++col) {
          int32_t destX = glyphLeft + col;
          if(destX < absX || destX >= absX + clipW)
            continue;
          size_t idx = rowOffset + static_cast<size_t>(destX);
          if(idx >= totalPixels)
            continue;
          size_t bitmapIdx = static_cast<size_t>(row) * rowStride + static_cast<size_t>(col);
          uint8_t alpha = bitmap->buffer[bitmapIdx];
          if(alpha == 0)
            continue;
          uint32_t bg = buffer[idx];
          uint32_t r = (((colorNoAlpha >> 16) & 0xFF) * alpha + ((bg >> 16) & 0xFF) * (255 - alpha)) / 255;
          uint32_t g = (((colorNoAlpha >> 8) & 0xFF) * alpha + ((bg >> 8) & 0xFF) * (255 - alpha)) / 255;
          uint32_t b = ((colorNoAlpha & 0xFF) * alpha + (bg & 0xFF) * (255 - alpha)) / 255;
          buffer[idx] = (r << 16) | (g << 8) | b;
        }
      }
      penX += static_cast<int32_t>(slot->advance.x >> 6);
    }
  }

  void AWidget::DrawTextOffset(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY, int32_t xOffset) const {
    if(mText.empty())
      return;
    if(!mEnginePtr)
      return;
    FT_Face face = mEnginePtr->GetDefaultFontFace();
    D2("DrawTextOffset: mText='{}', mEnginePtr={}, face={}", mText, (void*)mEnginePtr, (void*)face);
    if(!face) {
      D2("  no face");
      return;
    }
    if(!mTextMetricsValid) {
      UpdateTextMetrics();
    }
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    int32_t clipW = static_cast<int32_t>(mSizeX);
    int32_t clipH = static_cast<int32_t>(mSizeY);
    int32_t pW = static_cast<int32_t>(parentWidth);
    int32_t pH = static_cast<int32_t>(parentHeight);
// Clip to parent (signed arithmetic)
    if(absX < 0) {
      clipW += absX;
      absX = 0;
    }
    if(absY < 0) {
      clipH += absY;
      absY = 0;
    }
    if(absX + clipW > pW)
      clipW = pW - absX;
    if(absY + clipH > pH)
      clipH = pH - absY;
    if(clipW <= 0 || clipH <= 0)
      return;
// Horizontal alignment with offset
    int32_t startX = absX - xOffset;// shift left by xOffset
    int32_t widgetWidth = static_cast<int32_t>(mSizeX);
    if(mHAlign == AUIHAlign::center) {
      startX += (widgetWidth - mCachedTextWidth) / 2;
    }
    else if(mHAlign == AUIHAlign::right) {
      startX += widgetWidth - mCachedTextWidth;
    }
    int32_t baselineY = absY;
    int32_t widgetHeight = static_cast<int32_t>(mSizeY);
    if(mVAlign == AUIVAlign::center) {
      baselineY += (widgetHeight - mCachedTextHeight) / 2;
    }
    else if(mVAlign == AUIVAlign::bottom) {
      baselineY += widgetHeight - mCachedTextHeight;
    }
    baselineY += mCachedTextHeight;
    int32_t penX = startX;
    uint32_t colorNoAlpha = mTextColor & 0x00FFFFFFU;
    size_t totalPixels = static_cast<size_t>(pW) * static_cast<size_t>(pH);
    D2("  startX={}, baselineY={}, mHOffset={}", startX, baselineY, xOffset);
    for (char c : mText) {
      if(FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_RENDER) != 0)
        continue;
      FT_GlyphSlot slot = face->glyph;
      FT_Bitmap *bitmap = &slot->bitmap;
      int32_t glyphLeft = penX + static_cast<int32_t>(slot->bitmap_left);
      int32_t glyphTop = baselineY - static_cast<int32_t>(slot->bitmap_top);
      int32_t bitmapRows = static_cast<int32_t>(bitmap->rows);
      int32_t bitmapWidth = static_cast<int32_t>(bitmap->width);
      size_t rowStride = static_cast<size_t>(bitmapWidth);
      for (int32_t row = 0; row < bitmapRows; ++row) {
        int32_t destY = glyphTop + row;
        if(destY < absY || destY >= absY + clipH)
          continue;
        size_t rowOffset = static_cast<size_t>(destY) * (size_t) pW;
        for (int32_t col = 0; col < bitmapWidth; ++col) {
          int32_t destX = glyphLeft + col;
          if(destX < absX || destX >= absX + clipW)
            continue;
          size_t idx = rowOffset + static_cast<size_t>(destX);
          if(idx >= totalPixels)
            continue;
          size_t bitmapIdx = static_cast<size_t>(row) * rowStride + static_cast<size_t>(col);
          uint8_t alpha = bitmap->buffer[bitmapIdx];
          if(alpha == 0)
            continue;
          uint32_t bg = buffer[idx];
          uint32_t r = (((colorNoAlpha >> 16) & 0xFF) * alpha + ((bg >> 16) & 0xFF) * (255 - alpha)) / 255;
          uint32_t g = (((colorNoAlpha >> 8) & 0xFF) * alpha + ((bg >> 8) & 0xFF) * (255 - alpha)) / 255;
          uint32_t b = ((colorNoAlpha & 0xFF) * alpha + (bg & 0xFF) * (255 - alpha)) / 255;
          buffer[idx] = (r << 16) | (g << 8) | b;
        }
      }
      penX += static_cast<int32_t>(slot->advance.x >> 6);
    }
  }

  void DrawTextEx(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t absX, int32_t absY,
      int32_t drawW, int32_t drawH, const std::string &text, FT_Face face, uint32_t fontSize, AUIHAlign hAlign,
      AUIVAlign vAlign, int32_t xOffset, uint32_t textColor) {
    if(text.empty() || !face)
      return;
    FT_Set_Pixel_Sizes(face, 0, fontSize);
// ----- compute text extents -----
    int32_t maxAscent = 0, maxDescent = 0, totalWidth = 0;
    for (char c : text) {
      if(FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_RENDER) != 0)
        continue;
      FT_GlyphSlot slot = face->glyph;
      totalWidth += static_cast<int32_t>(slot->advance.x >> 6);
      int32_t asc = static_cast<int32_t>(slot->bitmap_top);
      if(asc > maxAscent)
        maxAscent = asc;
      int32_t desc = static_cast<int32_t>(slot->bitmap.rows) - static_cast<int32_t>(slot->bitmap_top);
      if(desc > maxDescent)
        maxDescent = desc;
    }
    int32_t textWidth = totalWidth;
    int32_t textHeight = maxAscent + maxDescent;
    if(textHeight == 0)
      textHeight = static_cast<int32_t>(fontSize);
// ----- horizontal alignment (scroll offset applied) -----
    int32_t startX = absX - xOffset;
    if(hAlign == AUIHAlign::center)
      startX += (drawW - textWidth) / 2;
    else if(hAlign == AUIHAlign::right)
      startX += drawW - textWidth;
// ----- vertical alignment -> baseline -----
    int32_t baselineY = absY;
    if(vAlign == AUIVAlign::center)
      baselineY += (drawH - textHeight) / 2;
    else if(vAlign == AUIVAlign::bottom)
      baselineY += drawH - textHeight;
    baselineY += textHeight;
    int32_t pW = static_cast<int32_t>(parentWidth);
    int32_t pH = static_cast<int32_t>(parentHeight);
    size_t totalPixels = static_cast<size_t>(pW) * static_cast<size_t>(pH);
    uint32_t colorNoAlpha = textColor & 0x00FFFFFFU;
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    int32_t penX = startX;
    for (char c : text) {
      if(FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_RENDER) != 0)
        continue;
      FT_GlyphSlot slot = face->glyph;
      FT_Bitmap *bitmap = &slot->bitmap;
      int32_t glyphLeft = penX + static_cast<int32_t>(slot->bitmap_left);
      int32_t glyphTop = baselineY - static_cast<int32_t>(slot->bitmap_top);
      int32_t rows = static_cast<int32_t>(bitmap->rows);
      int32_t cols = static_cast<int32_t>(bitmap->width);
      for (int32_t row = 0; row < rows; ++row) {
        int32_t destY = glyphTop + row;
        if(destY < absY || destY >= absY + drawH)
          continue;
        size_t rowOffset = static_cast<size_t>(destY) * static_cast<size_t>(pW);
        for (int32_t col = 0; col < cols; ++col) {
          int32_t destX = glyphLeft + col;
          if(destX < absX || destX >= absX + drawW)
            continue;
          size_t idx = rowOffset + static_cast<size_t>(destX);
          if(idx >= totalPixels)
            continue;
          uint8_t alpha =
              bitmap->buffer[static_cast<size_t>(row) * static_cast<size_t>(cols) + static_cast<size_t>(col)];
          if(alpha == 0)
            continue;
          uint32_t bg = buffer[idx];
          uint32_t r = (((colorNoAlpha >> 16) & 0xFF) * alpha + ((bg >> 16) & 0xFF) * (255 - alpha)) / 255;
          uint32_t g = (((colorNoAlpha >> 8) & 0xFF) * alpha + ((bg >> 8) & 0xFF) * (255 - alpha)) / 255;
          uint32_t b = ((colorNoAlpha & 0xFF) * alpha + (bg & 0xFF) * (255 - alpha)) / 255;
          buffer[idx] = (r << 16) | (g << 8) | b;
        }
      }
      penX += static_cast<int32_t>(slot->advance.x >> 6);
    }
  }

}// namespace aui

