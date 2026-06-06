#include "AUILib.h"

namespace aui {

// ------------------------------------------------------------------
// Constructor
// ------------------------------------------------------------------
  AScrollBar::AScrollBar() {
    D2("AScrollBar constructed");
    mSizeX = 100;
    mSizeY = 100;
    mTrackThickness = 12;
    mThumbThickness = 24;
    mThumbColor = 0xFF888888;
    mTrackColor = 0xFFCCCCCC;
    mWidgetType = AUIWidgetType::defaultScrollBar;
    mLastDrawnValue = mValue;
  }

// ------------------------------------------------------------------
// Geometry helpers
// ------------------------------------------------------------------
  uint32_t AScrollBar::GetTrackLength() const {
    uint32_t total = (mOrientation == AUIOrientation::vertical) ? mSizeY : mSizeX;
    if(mShowArrows) {
      total -= 2 * mArrowSize;
    }
    return total;
  }

  uint32_t AScrollBar::GetThumbLength() const {
    uint32_t trackLen = GetTrackLength();
    int32_t docRange = mMaxValue - mMinValue;
    if(docRange <= 0)
      return trackLen;
// Use double to avoid conversion warnings, cast to int64_t for sum to prevent overflow
    int64_t total = static_cast<int64_t>(docRange) + static_cast<int64_t>(mPageStep);
    if(total <= 0)
      return trackLen;
    double visibleRatio = static_cast<double>(mPageStep) / static_cast<double>(total);
    double lenDouble = visibleRatio * static_cast<double>(trackLen);
    uint32_t len = static_cast<uint32_t>(lenDouble);
    if(len < 20)
      len = 20;
    if(len > trackLen)
      len = trackLen;
    return len;
  }

  uint32_t AScrollBar::GetThumbPosition() const {
    uint32_t trackLen = GetTrackLength();
    uint32_t thumbLen = GetThumbLength();
    if(thumbLen >= trackLen)
      return 0;
    int32_t docRange = mMaxValue - mMinValue;
    if(docRange == 0)
      return 0;
    double ratio = static_cast<double>(mValue - mMinValue) / static_cast<double>(docRange);
    int32_t movable = static_cast<int32_t>(trackLen - thumbLen);
    double posDouble = ratio * static_cast<double>(movable);
    return static_cast<uint32_t>(posDouble);
  }

  int32_t AScrollBar::ValueFromCoord(int32_t coord) const {
    uint32_t trackLen = GetTrackLength();
    uint32_t thumbLen = GetThumbLength();
    if(thumbLen >= trackLen)
      return mMinValue;
    int32_t maxMovable = static_cast<int32_t>(trackLen - thumbLen);
    if(coord < 0)
      coord = 0;
    if(coord > maxMovable)
      coord = maxMovable;
    int32_t docRange = mMaxValue - mMinValue;
    if(docRange == 0)
      return mMinValue;
    double ratio = static_cast<double>(coord) / static_cast<double>(maxMovable);
    double valueDouble = static_cast<double>(mMinValue) + ratio * static_cast<double>(docRange);
    return static_cast<int32_t>(valueDouble);
  }

// ------------------------------------------------------------------
// Drawing
// ------------------------------------------------------------------
  void AScrollBar::DrawTrack(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY) const {
    if(!buffer)
      return;
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    uint32_t trackLen = GetTrackLength();
    D2("DrawTrack: orientation={}, trackLen={}, mShowArrows={}, mArrowSize={}", static_cast<int32_t>(mOrientation),
        trackLen, mShowArrows, mArrowSize);
    if(trackLen == 0)
      return;
    uint32_t trackColorNoAlpha = mTrackColor & 0x00FFFFFFU;

    if(mOrientation == AUIOrientation::vertical) {
      int32_t startX = absX + static_cast<int32_t>((mSizeX - mTrackThickness) / 2);
      int32_t drawW = static_cast<int32_t>(mTrackThickness);
      int32_t drawH = static_cast<int32_t>(trackLen);
// Start after the top arrow
      int32_t trackStartY = absY + static_cast<int32_t>(mShowArrows ? mArrowSize : 0);
// Clip to parent
      int32_t clipStartX = (startX > 0) ? startX : 0;
      int32_t clipW = drawW - (clipStartX - startX);
      if(clipW > static_cast<int32_t>(parentWidth) - clipStartX)
        clipW = static_cast<int32_t>(parentWidth) - clipStartX;
      int32_t clipY = (trackStartY > 0) ? trackStartY : 0;
      int32_t clipH = drawH - (clipY - trackStartY);
      if(clipH > static_cast<int32_t>(parentHeight) - clipY)
        clipH = static_cast<int32_t>(parentHeight) - clipY;
      if(clipW <= 0 || clipH <= 0)
        return;
      for (int32_t row = 0; row < clipH; ++row) {
        size_t lineStart = static_cast<size_t>(clipY + row) * parentWidth + static_cast<size_t>(clipStartX);
        for (int32_t col = 0; col < clipW; ++col) {
          size_t idx = lineStart + static_cast<size_t>(col);
          if(idx < static_cast<size_t>(parentWidth) * parentHeight)
            buffer[idx] = trackColorNoAlpha;
        }
      }
    }
    else {// horizontal
      int32_t startY = absY + static_cast<int32_t>((mSizeY - mTrackThickness) / 2);
      int32_t drawW = static_cast<int32_t>(trackLen);
      int32_t drawH = static_cast<int32_t>(mTrackThickness);
      int32_t trackStartX = absX + static_cast<int32_t>(mShowArrows ? mArrowSize : 0);
      int32_t clipStartY = (startY > 0) ? startY : 0;
      int32_t clipH = drawH - (clipStartY - startY);
      if(clipH > static_cast<int32_t>(parentHeight) - clipStartY)
        clipH = static_cast<int32_t>(parentHeight) - clipStartY;
      int32_t clipX = (trackStartX > 0) ? trackStartX : 0;
      int32_t clipW = drawW - (clipX - trackStartX);
      if(clipW > static_cast<int32_t>(parentWidth) - clipX)
        clipW = static_cast<int32_t>(parentWidth) - clipX;
      if(clipW <= 0 || clipH <= 0)
        return;
      for (int32_t row = 0; row < clipH; ++row) {
        size_t lineStart = static_cast<size_t>(clipStartY + row) * parentWidth + static_cast<size_t>(clipX);
        for (int32_t col = 0; col < clipW; ++col) {
          size_t idx = lineStart + static_cast<size_t>(col);
          if(idx < static_cast<size_t>(parentWidth) * parentHeight)
            buffer[idx] = trackColorNoAlpha;
        }
      }
    }
  }

  void AScrollBar::DrawThumb(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY) const {
    if(!buffer)
      return;
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    uint32_t thumbPos = GetThumbPosition();
    uint32_t thumbLen = GetThumbLength();

    if(thumbLen == 0)
      return;
    uint32_t thumbColorNoAlpha = mThumbColor & 0x00FFFFFFU;

    if(mOrientation == AUIOrientation::vertical) {
      int32_t startX = absX + static_cast<int32_t>((mSizeX - mThumbThickness) / 2);
      int32_t drawW = static_cast<int32_t>(mThumbThickness);
      int32_t drawH = static_cast<int32_t>(thumbLen);
// Thumb is drawn inside the track region, so offset by arrow size
      int32_t trackStartY = absY + static_cast<int32_t>(mShowArrows ? mArrowSize : 0);
      int32_t thumbStartY = trackStartY + static_cast<int32_t>(thumbPos);
// Clip horizontally
      int32_t clipStartX = (startX > 0) ? startX : 0;
      int32_t clipW = drawW - (clipStartX - startX);
      if(clipW > static_cast<int32_t>(parentWidth) - clipStartX)
        clipW = static_cast<int32_t>(parentWidth) - clipStartX;
// Clip vertically
      int32_t clipStartY = (thumbStartY > 0) ? thumbStartY : 0;
      int32_t clipH = drawH - (clipStartY - thumbStartY);
      if(clipH > static_cast<int32_t>(parentHeight) - clipStartY)
        clipH = static_cast<int32_t>(parentHeight) - clipStartY;
      if(clipW <= 0 || clipH <= 0)
        return;
      for (int32_t row = 0; row < clipH; ++row) {
        size_t lineStart = static_cast<size_t>(clipStartY + row) * parentWidth + static_cast<size_t>(clipStartX);
        for (int32_t col = 0; col < clipW; ++col) {
          size_t idx = lineStart + static_cast<size_t>(col);
          if(idx < static_cast<size_t>(parentWidth) * parentHeight)
            buffer[idx] = thumbColorNoAlpha;
        }
      }
    }
    else {// horizontal
      int32_t startY = absY + static_cast<int32_t>((mSizeY - mThumbThickness) / 2);
      int32_t drawW = static_cast<int32_t>(thumbLen);
      int32_t drawH = static_cast<int32_t>(mThumbThickness);
      int32_t trackStartX = absX + static_cast<int32_t>(mShowArrows ? mArrowSize : 0);
      int32_t thumbStartX = trackStartX + static_cast<int32_t>(thumbPos);
// Clip vertically
      int32_t clipStartY = (startY > 0) ? startY : 0;
      int32_t clipH = drawH - (clipStartY - startY);
      if(clipH > static_cast<int32_t>(parentHeight) - clipStartY)
        clipH = static_cast<int32_t>(parentHeight) - clipStartY;
// Clip horizontally
      int32_t clipStartX = (thumbStartX > 0) ? thumbStartX : 0;
      int32_t clipW = drawW - (clipStartX - thumbStartX);
      if(clipW > static_cast<int32_t>(parentWidth) - clipStartX)
        clipW = static_cast<int32_t>(parentWidth) - clipStartX;
      if(clipW <= 0 || clipH <= 0)
        return;
      for (int32_t row = 0; row < clipH; ++row) {
        size_t lineStart = static_cast<size_t>(clipStartY + row) * parentWidth + static_cast<size_t>(clipStartX);
        for (int32_t col = 0; col < clipW; ++col) {
          size_t idx = lineStart + static_cast<size_t>(col);
          if(idx < static_cast<size_t>(parentWidth) * parentHeight)
            buffer[idx] = thumbColorNoAlpha;
        }
      }
    }
  }

  void AScrollBar::Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY) const {
    if(!buffer)
      return;
    int32_t pW = static_cast<int32_t>(parentWidth);
    int32_t pH = static_cast<int32_t>(parentHeight);
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    int32_t drawW = static_cast<int32_t>(mSizeX);
    int32_t drawH = static_cast<int32_t>(mSizeY);

// Clip background
    int32_t clipX = (absX > 0) ? absX : 0;
    int32_t clipY = (absY > 0) ? absY : 0;
    int32_t clipW = drawW - (clipX - absX);
    int32_t clipH = drawH - (clipY - absY);
    if(clipW > pW - clipX)
      clipW = pW - clipX;
    if(clipH > pH - clipY)
      clipH = pH - clipY;
    if(clipW > 0 && clipH > 0) {
      uint32_t bgColor = mBGColor & 0x00FFFFFFU;
      size_t totalPixels = static_cast<size_t>(pW) * static_cast<size_t>(pH);
      for (int32_t row = 0; row < clipH; ++row) {
        size_t lineStart = static_cast<size_t>(clipY + row) * static_cast<size_t>(pW) + static_cast<size_t>(clipX);
        for (int32_t col = 0; col < clipW; ++col) {
          size_t idx = lineStart + static_cast<size_t>(col);
          if(idx < totalPixels)
            buffer[idx] = bgColor;
        }
      }
    }
    DrawBorder(buffer, parentWidth, parentHeight, offsetX, offsetY);
    DrawTrack(buffer, parentWidth, parentHeight, offsetX, offsetY);
    DrawThumb(buffer, parentWidth, parentHeight, offsetX, offsetY);
    DrawArrows(buffer, parentWidth, parentHeight, offsetX, offsetY);
  }

  bool AScrollBar::OnMouseClick(int32_t localX, int32_t localY, bool pressed) {
    D2("AScrollBar::OnMouseClick: localX={}, localY={}, pressed={}, mDragging={}", localX, localY, pressed, mDragging);

// ----- RELEASE (button up) -----
    if(!pressed) {
// Stop dragging unconditionally
      mDragging = false;
// Clear any arrow pressed states
      mArrowTopPressed = false;
      mArrowBottomPressed = false;
      mArrowLeftPressed = false;
      mArrowRightPressed = false;
// If this scrollbar is the current drag widget, release it from the parent window
      if(mParentWindow && mParentWindow->GetDragWidget() == this) {
        mParentWindow->SetDragWidget(nullptr);
      }
      D2("AScrollBar::OnMouseClick: RELEASE - mDragging set to false, drag widget cleared");
      return false;
      }

// ----- PRESS (button down) -----

// 1. Arrow clicks (if enabled)
    if(mShowArrows) {
      if(mOrientation == AUIOrientation::vertical) {
        if(IsInTopArrow(localX, localY)) {
          mArrowTopPressed = true;
          SetValue(mValue - mSingleStep);
          D2("AScrollBar::OnMouseClick: top arrow clicked, newValue={}", mValue);
          return true;
        }
        if(IsInBottomArrow(localX, localY)) {
          mArrowBottomPressed = true;
          SetValue(mValue + mSingleStep);
          D2("AScrollBar::OnMouseClick: bottom arrow clicked, newValue={}", mValue);
          return true;
        }
      }
      else {// horizontal
        if(IsInLeftArrow(localX, localY)) {
          mArrowLeftPressed = true;
          SetValue(mValue - mSingleStep);
          D2("AScrollBar::OnMouseClick: left arrow clicked, newValue={}", mValue);
          return true;
        }
        if(IsInRightArrow(localX, localY)) {
          mArrowRightPressed = true;
          SetValue(mValue + mSingleStep);
          D2("AScrollBar::OnMouseClick: right arrow clicked, newValue={}", mValue);
          return true;
        }
      }
      }

// 2. Thumb / track handling
    int32_t coord = (mOrientation == AUIOrientation::vertical) ? localY : localX;
    uint32_t thumbPos = GetThumbPosition();
    uint32_t thumbLen = GetThumbLength();

// Click on the thumb -> start dragging
    if(coord >= static_cast<int32_t>(thumbPos) && coord <= static_cast<int32_t>(thumbPos + thumbLen)) {
      if(mParentWindow) {
        mParentWindow->SetDragWidget(this);
      }
      mDragging = true;
      mDragStartPos = coord;
      mDragStartValue = mValue;
      D2("AScrollBar::OnMouseClick: PRESS on thumb - started dragging, startVal={}", mValue);
          return true;
      }
// Click in the track (not on thumb) -> jump to that position
      else {
      int32_t newValue = ValueFromCoord(coord);
      SetValue(newValue);
      D2("AScrollBar::OnMouseClick: PRESS in track - jumped to {}", newValue);
          return true;
      }
  }
  void AScrollBar::OnMouseMove(int32_t localX, int32_t localY) {
    D2("AScrollBar::OnMouseMove: localX={}, localY={}, mDragging={}", localX, localY, mDragging);
    if(!mDragging)
      return;
// Safety: if parent window no longer has this as drag widget, stop dragging
    if(mParentWindow && mParentWindow->GetDragWidget() != this) {
      D2("Drag widget changed, resetting mDragging");
      mDragging = false;
      return;
    }
    int32_t coord = (mOrientation == AUIOrientation::vertical) ? localY : localX;
    int32_t newValue = ValueFromCoord(coord);
    D2("  coord={}, newValue={}", coord, newValue);
    SetValue(newValue);
  }
// ------------------------------------------------------------------
// Setters (all trigger redraw)
// ------------------------------------------------------------------
  void AScrollBar::SetOrientation(AUIOrientation orient) {
    if(mOrientation == orient)
      return;
    mOrientation = orient;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AScrollBar::SetRange(int32_t minVal, int32_t maxVal) {
    if(minVal > maxVal)
      return;
    mMinValue = minVal;
    mMaxValue = maxVal;
    if(mValue < mMinValue)
      mValue = mMinValue;
    if(mValue > mMaxValue)
      mValue = mMaxValue;
    if(mParentWindow)
      mParentWindow->Draw();// add this line
  }

  void AScrollBar::SetValue(int32_t val) {
    if(val < mMinValue)
      val = mMinValue;
    if(val > mMaxValue)
      val = mMaxValue;
    if(mValue == val)
      return;
    mValue = val;
// Only redraw if the value changed by at least 2 (or a threshold)
    if(std::abs(mValue - mLastDrawnValue) >= 2) {
      mLastDrawnValue = mValue;
      if(mParentWindow)
        mParentWindow->Draw();
    }
    InvokeScrollCallback(mValue);
  }

  void AScrollBar::SetPageStep(int32_t step) {
    if(step <= 0)
      return;
    mPageStep = step;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AScrollBar::SetSingleStep(int32_t step) {
    if(step <= 0)
      return;
    mSingleStep = step;
// No redraw needed for step value alone, but keep for consistency
  }

  void AScrollBar::SetTrackThickness(uint32_t thick) {
    mTrackThickness = thick;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AScrollBar::SetThumbThickness(uint32_t thick) {
    mThumbThickness = thick;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AScrollBar::SetThumbColor(uint32_t color) {
    mThumbColor = color;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AScrollBar::SetTrackColor(uint32_t color) {
    mTrackColor = color;
    if(mParentWindow)
      mParentWindow->Draw();
  }

// ------------------------------------------------------------------
// Callback
// ------------------------------------------------------------------
  void AScrollBar::SetScrollCallback(ScrollCallback callback, void *userData) {
    mScrollCallback = std::move(callback);
    mScrollUserData = userData;
  }

// ------------------------------------------------------------------
// Factory methods (require friend access to AddWidget)
// ------------------------------------------------------------------
  AScrollBar* AScrollBar::AttachTo(AWindow *parent, AUIOrientation orientation) {
    D1("Attaching AScrollBar to window");
    if(!parent)
      E("AScrollBar::AttachTo: parent window is null");
    AScrollBar *sb = new AScrollBar();
    sb->SetOrientation(orientation);
// Set default size based on orientation
    if(orientation == AUIOrientation::vertical) {
      sb->Resize(16, 200);
    }
    else {
      sb->Resize(200, 16);
    }
    parent->AddWidget(std::unique_ptr<AWidget>(sb));
    return sb;
  }

  AScrollBar* AScrollBar::AttachTo(AWidget *parent, AUIOrientation orientation) {
    D1("Attaching AScrollBar to widget");
    if(!parent)
      E("AScrollBar::AttachTo: parent widget is null");
    AScrollBar *sb = new AScrollBar();
    sb->SetOrientation(orientation);
    if(orientation == AUIOrientation::vertical) {
      sb->Resize(16, 200);
    }
    else {
      sb->Resize(200, 16);
    }
    parent->AddWidget(std::unique_ptr<AWidget>(sb));
    return sb;
  }

  void AScrollBar::OnParentResize(uint32_t newWidth, uint32_t newHeight) {
    if(mOrientation == AUIOrientation::vertical) {
// Keep the original top margin (mY) and right margin (newWidth - (mX + mSizeX))
// Usually you want the scrollbar to stick to the right edge and fill the height minus some margins.
// Example: keep 40px top and 40px bottom margins:
      int32_t topMargin = mY;// assume top margin is fixed
      int32_t bottomMargin = 40;
      int32_t newY = topMargin;
      int32_t newH = static_cast<int32_t>(newHeight) - topMargin - bottomMargin;
      if(newH > 0) {
        Resize(mSizeX, static_cast<uint32_t>(newH));
        Move(mX, newY);
      }
    }
    else {// horizontal
// Keep left margin and bottom margin
      int32_t leftMargin = mX;
      int32_t rightMargin = 40;
      int32_t newX = leftMargin;
      int32_t newW = static_cast<int32_t>(newWidth) - leftMargin - rightMargin;
      if(newW > 0) {
        Resize(static_cast<uint32_t>(newW), mSizeY);
        Move(newX, mY);
      }
    }
  }

  bool AScrollBar::IsInLeftArrow(int32_t localX, int32_t localY) const {
    if(!mShowArrows || mOrientation != AUIOrientation::horizontal)
      return false;
// Left arrow occupies the leftmost mArrowSize pixels, full height
    return (localX >= 0 && localX < static_cast<int32_t>(mArrowSize) && localY >= 0
        && localY < static_cast<int32_t>(mSizeY));
  }

  bool AScrollBar::IsInRightArrow(int32_t localX, int32_t localY) const {
    if(!mShowArrows || mOrientation != AUIOrientation::horizontal)
      return false;
// Right arrow occupies the rightmost mArrowSize pixels, full height
    return (localX >= static_cast<int32_t>(mSizeX - mArrowSize) && localX < static_cast<int32_t>(mSizeX) && localY >= 0
        && localY < static_cast<int32_t>(mSizeY));
  }

  bool AScrollBar::IsInTopArrow(int32_t localX, int32_t localY) const {
    if(!mShowArrows)
      return false;
    if(mOrientation == AUIOrientation::vertical) {
      return (localX >= 0 && localX < static_cast<int32_t>(mSizeX) && localY >= 0
          && localY < static_cast<int32_t>(mArrowSize));
    }
    else {// horizontal – left arrow
      return (localX >= 0 && localX < static_cast<int32_t>(mArrowSize) && localY >= 0
          && localY < static_cast<int32_t>(mSizeY));
    }
  }

  bool AScrollBar::IsInBottomArrow(int32_t localX, int32_t localY) const {
    if(!mShowArrows)
      return false;
    if(mOrientation == AUIOrientation::vertical) {
      return (localX >= 0 && localX < static_cast<int32_t>(mSizeX)
          && localY >= static_cast<int32_t>(mSizeY - mArrowSize) && localY < static_cast<int32_t>(mSizeY));
    }
    else {// horizontal – right arrow
      return (localX >= static_cast<int32_t>(mSizeX - mArrowSize) && localX < static_cast<int32_t>(mSizeX)
          && localY >= 0 && localY < static_cast<int32_t>(mSizeY));
    }
  }

  void AScrollBar::DrawArrows(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY) const {
    if(!mShowArrows)
      return;
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    uint32_t arrowColor = (mThumbColor & 0x00FFFFFF);// use thumb color as base
    uint32_t highlightColor = ShiftColor(arrowColor, false);// lighter for hover/pressed

    auto drawLine = [&](int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color) {
// simple DDA (but we can do a basic Bresenham)
      int32_t dx = std::abs(x1 - x0);
      int32_t dy = std::abs(y1 - y0);
      int32_t sx = (x0 < x1) ? 1 : -1;
      int32_t sy = (y0 < y1) ? 1 : -1;
      int32_t err = dx - dy;
      int32_t x = x0, y = y0;
      while (true) {
        if(x >= 0 && x < static_cast<int32_t>(parentWidth) && y >= 0 && y < static_cast<int32_t>(parentHeight)) {
          size_t idx = static_cast<size_t>(y) * parentWidth + static_cast<size_t>(x);
          buffer[idx] = color;
        }
        if(x == x1 && y == y1)
          break;
        int32_t e2 = 2 * err;
        if(e2 > -dy) {
          err -= dy;
          x += sx;
        }
        if(e2 < dx) {
          err += dx;
          y += sy;
        }
      }
    };

    if(mOrientation == AUIOrientation::vertical) {
// Top arrow (pointing up)
      int32_t centerX = absX + static_cast<int32_t>(mSizeX / 2);
      int32_t topY = absY + static_cast<int32_t>(mArrowSize / 2);
      uint32_t col = (mArrowTopHover || mArrowTopPressed) ? highlightColor : arrowColor;
// Draw two lines: one from (centerX - 4, topY + 2) to (centerX, topY - 2)
// and (centerX + 4, topY + 2) to (centerX, topY - 2) – but we need to draw on buffer.
// Simpler: draw triangle using lines (we can use Bresenham or simple points).
// For simplicity, we'll just draw two lines using a helper that sets pixels.
// Since we don't have line drawing, we'll use a simple plot.
// I'll implement a simple line drawing function (inline) for this.
// Draw up arrow: two lines forming a 'V' shape (actually an upward point)
      int32_t baseY = topY + 3;
      drawLine(centerX - 4, baseY, centerX, topY - 2, col);
      drawLine(centerX + 4, baseY, centerX, topY - 2, col);

// Bottom arrow (pointing down)
      int32_t bottomY = absY + static_cast<int32_t>(mSizeY - mArrowSize / 2);
      col = (mArrowBottomHover || mArrowBottomPressed) ? highlightColor : arrowColor;
      drawLine(centerX - 4, bottomY - 3, centerX, bottomY + 2, col);
      drawLine(centerX + 4, bottomY - 3, centerX, bottomY + 2, col);
    }
    else {// horizontal
      int32_t centerY = absY + static_cast<int32_t>(mSizeY / 2);
// Left arrow (pointing left)
      int32_t leftX = absX + static_cast<int32_t>(mArrowSize / 2);
      uint32_t col = (mArrowLeftHover || mArrowLeftPressed) ? highlightColor : arrowColor;
      drawLine(leftX + 3, centerY - 4, leftX - 2, centerY, col);
      drawLine(leftX + 3, centerY + 4, leftX - 2, centerY, col);
// Right arrow (pointing right)
      int32_t rightX = absX + static_cast<int32_t>(mSizeX - mArrowSize / 2);
      col = (mArrowRightHover || mArrowRightPressed) ? highlightColor : arrowColor;
      drawLine(rightX - 3, centerY - 4, rightX + 2, centerY, col);
      drawLine(rightX - 3, centerY + 4, rightX + 2, centerY, col);
    }
  }

}// namespace aui

