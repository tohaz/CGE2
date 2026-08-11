#include "AUILib.h"

namespace aui {

// ---------- Helper: Fill a triangle using horizontal scanlines ----------
  UNUSED static void FillTriangle(uint32_t* buffer, uint32_t stride, int32_t x1, int32_t y1, int32_t x2, int32_t y2,
      int32_t x3, int32_t y3, uint32_t color, int32_t clipL, int32_t clipR, int32_t clipT, int32_t clipB) {
// Sort vertices by y coordinate so: y1 <= y2 <= y3
    if(y1 > y2) {
      std::swap(x1, x2);
      std::swap(y1, y2);
    }
    if(y1 > y3) {
      std::swap(x1, x3);
      std::swap(y1, y3);
    }
    if(y2 > y3) {
      std::swap(x2, x3);
      std::swap(y2, y3);
    }
    if(y1 == y3)
      return;// Zero-height triangle, nothing to draw
    auto draw_span = [&](int32_t y, int32_t x_start, int32_t x_end) {
      if(y < clipT || y >= clipB)
        return;
      int32_t xL = std::max(x_start, clipL);
      int32_t xR = std::min(x_end, clipR - 1);
      if(xL > xR)
        return;
      uint32_t* line = buffer + (size_t) y * stride + (size_t) xL;
      std::fill(line, line + (xR - xL + 1), color);
    };
// Scanline loop from top to bottom
    for(int32_t y = y1; y <= y3; ++y) {
// Determine if we are in the upper half or lower half of the triangle
      bool is_lower_half = (y > y2) || (y1 == y2);
// Edge 1: Always the continuous line from y1 to y3
      double t1 = (double) (y - y1) / (y3 - y1);
      int32_t xa = x1 + (int32_t) (t1 * (x3 - x1));
// Edge 2: The split side (y1 -> y2, then switching to y2 -> y3)
      int32_t xb;
      if(!is_lower_half) {
        double t2 = (double) (y - y1) / (y2 - y1);
        xb = x1 + (int32_t) (t2 * (x2 - x1));
      }
      else {
// Guard against zero-height bottom half
        double t2 = (y3 != y2) ? (double) (y - y2) / (y3 - y2) : 0.0;
        xb = x2 + (int32_t) (t2 * (x3 - x2));
      }
// Ensure proper left-to-right ordering for the span drawer
      if(xa > xb)
        std::swap(xa, xb);
// +1 to xb because the right bounds of draw_span expects an exclusive/inclusive window adjustment
      draw_span(y, xa, xb + 1);
    }
  }
// ---------- Constructor ----------
  AScrollBar::AScrollBar() {
    mSizeX = 20;
    mSizeY = 100;
    mBGColor3 = 0xFF888888;// thumb color
    mBGColor4 = 0xFF444444;// track color
    mMinValue = 0;
    mMaxValue = 100;
    mValue = 0;
    mPageStep = 10;
    mSingleStep = 1;
    mTrackThick = 4;
    mThumbThick = 8;
    mArrowSize = 12;
    mShowArrows = true;
    mDragging = false;
    mDragStartPos = 0;
    mDragStartValue = 0;
    DefaultFillBG(true);
    BGColor(0x00000000);
    Orient(AUIOrientation::vertical);
    MouseLeftReleaseRequired(true);
    Text("some scrollbar");
  }

  AScrollBar::AScrollBar(AUIOrientation v) :
      AScrollBar() {
    if(v != AUIOrientation::unset) {
      Orient(v);
    }
    else
      E("orientation unset")
  }

// ---------- Geometry helpers ----------
  uint32_t AScrollBar::TrackLength() const {
    const uint32_t totalLength = (Orient() == AUIOrientation::horizontal) ? mSizeX : mSizeY;
    if(!mShowArrows) {
      return totalLength;
    }
    const uint32_t arrowArea = 2 * mArrowSize;
    return (totalLength > arrowArea) ? (totalLength - arrowArea) : 0;
  }

  uint32_t AScrollBar::ThumbLength() const {
    uint32_t trackLen = TrackLength();
    if(trackLen == 0)
      return 0;
    int32_t totalContent = mMaxValue - mMinValue;
    if(totalContent <= 0)
      return trackLen;
    double ratio = (double) mPageStep / totalContent;
    uint32_t len = (uint32_t) (ratio * trackLen);
    if(len < 20)
      len = 20;
    if(len > trackLen)
      len = trackLen;
    return len;
  }

  uint32_t AScrollBar::ThumbPosition() const {
    uint32_t trackLen = TrackLength();
    uint32_t thumbLen = ThumbLength();
    if(trackLen <= thumbLen)
      return 0;

    int32_t effectiveMax = mMaxValue - mMinValue - mPageStep;
    if(effectiveMax <= 0)
      return 0;

    double ratio = (double) (mValue - mMinValue) / effectiveMax;
    uint32_t pos = static_cast<uint32_t>(ratio * (trackLen - thumbLen));
    if(pos > trackLen - thumbLen)
      pos = trackLen - thumbLen;
    return pos;
  }

  void AScrollBar::DrawTrack(UNUSED uint32_t* buffer, UNUSED uint32_t bufferW, UNUSED uint32_t bufferH,
  UNUSED int32_t offsetX, UNUSED int32_t offsetY, UNUSED uint32_t clipX, UNUSED uint32_t clipY) const {
    bool horiz = (Orient() == AUIOrientation::horizontal);
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
// Determine track rectangle (excluding arrows)
    uint32_t trackLen = TrackLength();
    if(trackLen == 0)
      return;
    int32_t trackX, trackY, trackW, trackH;
    if(horiz) {
      int32_t arrowOffset = mShowArrows ? SafeINT32(mArrowSize) : 0;
      trackX = absX + arrowOffset;
      trackY = absY + (static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mTrackThick)) / 2;
      trackW = SafeINT32(trackLen);
      trackH = SafeINT32(mTrackThick);
    }
    else {
      int32_t arrowOffset = mShowArrows ? SafeINT32(mArrowSize) : 0;
      trackX = absX + (static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mTrackThick)) / 2;
      trackY = absY + arrowOffset;
      trackW = SafeINT32(mTrackThick);
      trackH = SafeINT32(trackLen);
    }
// Clamp to visible area
    int32_t finalX = std::max(trackX, 0);
    int32_t finalY = std::max(trackY, 0);
    int32_t finalW = trackW;
    int32_t finalH = trackH;
    if(finalX + finalW > static_cast<int32_t>(bufferW))
      finalW = SafeINT32(bufferW) - finalX;
    if(finalY + finalH > static_cast<int32_t>(bufferH))
      finalH = SafeINT32(bufferH) - finalY;
    if(finalW <= 0 || finalH <= 0)
      return;
    FillRect(buffer, bufferW, finalX, finalY, finalW, finalH, mBGColor4);
  }

  void AScrollBar::DrawThumb(UNUSED uint32_t* buffer, UNUSED uint32_t bufferW, UNUSED uint32_t bufferH,
  UNUSED int32_t offsetX, UNUSED int32_t offsetY, UNUSED uint32_t clipX, UNUSED uint32_t clipY) const {
    bool horiz = (Orient() == AUIOrientation::horizontal);
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    uint32_t thumbPos = ThumbPosition();
    uint32_t thumbLen = ThumbLength();
    if(thumbLen == 0)
      return;
    int32_t thumbX, thumbY, thumbW, thumbH;
    if(horiz) {
      int32_t arrowOffset = mShowArrows ? SafeINT32(mArrowSize) : 0;
      thumbX = absX + arrowOffset + SafeINT32(thumbPos);
      thumbY = absY + (static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mThumbThick)) / 2;
      thumbW = SafeINT32(thumbLen);
      thumbH = SafeINT32(mThumbThick);
    }
    else {
      int32_t arrowOffset = mShowArrows ? SafeINT32(mArrowSize) : 0;
      thumbX = absX + (static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mThumbThick)) / 2;
      thumbY = absY + arrowOffset + SafeINT32(thumbPos);
      thumbW = SafeINT32(mThumbThick);
      thumbH = SafeINT32(thumbLen);
    }
// Clamp
    int32_t finalX = std::max(thumbX, 0);
    int32_t finalY = std::max(thumbY, 0);
    int32_t finalW = thumbW;
    int32_t finalH = thumbH;
    if(finalX + finalW > static_cast<int32_t>(bufferW))
      finalW = SafeINT32(bufferW) - finalX;
    if(finalY + finalH > static_cast<int32_t>(bufferH))
      finalH = SafeINT32(bufferH) - finalY;
    if(finalW <= 0 || finalH <= 0)
      return;
    FillRect(buffer, bufferW, finalX, finalY, finalW, finalH, mBGColor3);
  }

  void AScrollBar::DrawArrows(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
      uint32_t, uint32_t) const {
    if(!mShowArrows || mArrowSize < 2)
      return;
    bool horiz = (Orient() == AUIOrientation::horizontal);
    D2("drawing arrows horizontal = {} szx {} szy {}", horiz, mSizeX, mSizeY)
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
// Clipping boundaries (clamped to buffer)
    int32_t clipL = std::max(absX, 0);
    int32_t clipR = std::min(absX + static_cast<int32_t>(mSizeX), static_cast<int32_t>(bufferW));
    int32_t clipT = std::max(absY, 0);
    int32_t clipB = std::min(absY + static_cast<int32_t>(mSizeY), static_cast<int32_t>(bufferH));
    uint32_t arrowColor = mBGColor3;// use thumb color
// Arrow size (half of arrow area)
    int32_t size = static_cast<int32_t>(mArrowSize) / 2;
    if(size < 1)
      size = 1;
    if(horiz) {
// --- Horizontal arrows ---
// Vertical centre using left‑centre pixel for even heights
      int32_t cy = absY + (static_cast<int32_t>(mSizeY) - 1) / 2;
// Left arrow (points left)
      int32_t cxLeft = absX + static_cast<int32_t>(mArrowSize) / 2;
      FillTriangle(buffer, bufferW, cxLeft - size, cy,// tip
          cxLeft + size, cy - size,// top‑right
          cxLeft + size, cy + size,// bottom‑right
          arrowColor, clipL, clipR, clipT, clipB);
// Right arrow (points right)
      int32_t cxRight = absX + static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mArrowSize) / 2;
      FillTriangle(buffer, bufferW, cxRight + size, cy,// tip
          cxRight - size, cy - size,// top‑left
          cxRight - size, cy + size,// bottom‑left
          arrowColor, clipL, clipR, clipT, clipB);
    }
    else {
// --- Vertical arrows ---
// Horizontal centre using left‑centre pixel for even widths
      int32_t cx = absX + (static_cast<int32_t>(mSizeX) - 1) / 2;
// Up arrow (points up)
      int32_t cyUp = absY + static_cast<int32_t>(mArrowSize) / 2;
      FillTriangle(buffer, bufferW, cx, cyUp - size,// tip
      cx - size, cyUp + size,// bottom‑left
      cx + size, cyUp + size,// bottom‑right
      arrowColor, clipL, clipR, clipT, clipB);
// Down arrow (points down)
      int32_t cyDown = absY + static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mArrowSize) / 2;
      FillTriangle(buffer, bufferW, cx, cyDown + size,// tip
      cx - size, cyDown - size,// top‑left
      cx + size, cyDown - size,// top‑right
      arrowColor, clipL, clipR, clipT, clipB);
    }
  }

  void AScrollBar::OnDraw(UNUSED uint32_t* buffer, UNUSED uint32_t bufferW, UNUSED uint32_t bufferH,
  UNUSED int32_t offsetX, UNUSED int32_t offsetY, UNUSED int32_t clipL, UNUSED int32_t clipT,
  UNUSED int32_t clipR, UNUSED int32_t clipB) const {
    int32_t width = clipR - clipL;
    int32_t height = clipB - clipT;
    uint32_t clipX = (width > 0) ? static_cast<uint32_t>(width) : 0u;
    uint32_t clipY = (height > 0) ? static_cast<uint32_t>(height) : 0u;
    DrawTrack(buffer, bufferW, bufferH, offsetX, offsetY, clipX, clipY);
    DrawArrows(buffer, bufferW, bufferH, offsetX, offsetY, clipX, clipY);
    DrawThumb(buffer, bufferW, bufferH, offsetX, offsetY, clipX, clipY);
  }
// ---------- Public setters ----------
  void AScrollBar::Range(int32_t minContent, int32_t maxContent) {
    if(minContent > maxContent)
      std::swap(minContent, maxContent);
    mMinValue = minContent;
    mMaxValue = maxContent;// store total content bounds
// Now recompute effective scroll range based on pageStep
    int32_t effectiveMax = mMaxValue - mMinValue - mPageStep;
    if(effectiveMax < 0)
      effectiveMax = 0;
// Clamp current value
    if(mValue < 0)
      mValue = 0;
    if(mValue > effectiveMax)
      mValue = effectiveMax;
    MarkContentDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AScrollBar::Value(int32_t val) {
    int32_t effectiveMax = mMaxValue - mMinValue - mPageStep;
    if(effectiveMax < 0)
      effectiveMax = 0;

    // Correctly bound within [mMinValue, mMinValue + effectiveMax]
    int32_t clampedMax = mMinValue + effectiveMax;
    if(val < mMinValue)
      val = mMinValue;
    if(val > clampedMax)
      val = clampedMax;

    if(val != mValue) {
      mValue = val;
      if(mScrollCallback) {
        mScrollCallback(this, mScrollUserData, mValue);
      }
      MarkContentDirty();
      if(Wnd())
        Wnd()->RequestRedraw();
    }
  }

  void AScrollBar::PageStep(int32_t pageStep) {
    if(pageStep < 1)
      pageStep = 1;
    mPageStep = pageStep;
// Recompute max scroll and clamp value
    int32_t effectiveMax = mMaxValue - mMinValue - mPageStep;
    if(effectiveMax < 0)
      effectiveMax = 0;
    if(mValue < 0)
      mValue = 0;
    if(mValue > effectiveMax)
      mValue = effectiveMax;
    MarkContentDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AScrollBar::SingleStep(int32_t step) {
    if(step < 1)
      step = 1;
    mSingleStep = step;
    MarkContentDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AScrollBar::TrackThick(uint32_t thick) {
    mTrackThick = thick;
    MarkContentDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AScrollBar::ThumbThick(uint32_t thick) {
    mThumbThick = thick;
    MarkContentDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AScrollBar::ThumbColor(uint32_t ARGBcolor) {
    if(mBGColor3 != ARGBcolor) {
      mBGColor3 = ARGBcolor;
      MarkContentDirty();
      if(Wnd())
        Wnd()->RequestRedraw();
    }
  }

  void AScrollBar::TrackColor(uint32_t ARGBcolor) {
    if(mBGColor4 != ARGBcolor) {
      mBGColor4 = ARGBcolor;
      MarkContentDirty();
      if(Wnd())
        Wnd()->RequestRedraw();
    }
  }

  void AScrollBar::SetScrollCallback(ScrollCallback callback, void* userData) {
    mScrollCallback = std::move(callback);
    mScrollUserData = userData;
  }

  AWidget* AScrollBar::OnMouseDownLeft(UNUSED int32_t x, UNUSED int32_t y) {
    D1("incoming local: (%d,%d)  widget pos: (%d,%d)  size: (%d,%d)", x, y, mX, mY, mSizeX, mSizeY);
    if(x < 0 || x >= static_cast<int32_t>(mSizeX) || y < 0 || y >= static_cast<int32_t>(mSizeY)) {
      return nullptr;
    }
    bool horiz = (Orient() == AUIOrientation::horizontal);
    int32_t axisPos = horiz ? x : y;
    int32_t axisSize = horiz ? SafeINT32(mSizeX) : SafeINT32(mSizeY);
// Check arrows
    if(mShowArrows) {
      if(axisPos < static_cast<int32_t>(mArrowSize)) {
// Click on left/up arrow
        int32_t newVal = mValue - mSingleStep;
        Value(newVal);
        return this;
      }
      else
        if(axisPos > static_cast<int32_t>(axisSize - SafeINT32(mArrowSize))) {
          D1("Down arrow clicked: mValue=%d mSingleStep=%d mMax=%d", mValue, mSingleStep, mMaxValue);
// Click on right/down arrow
          int32_t newVal = mValue + mSingleStep;
          Value(newVal);
          return this;
        }
    }
// Check thumb
    uint32_t thumbPos = ThumbPosition();
    uint32_t thumbLen = ThumbLength();
    uint32_t trackLen = TrackLength();
    int32_t arrowOffset = mShowArrows ? SafeINT32(mArrowSize) : 0;
    int32_t trackStart = arrowOffset;
    int32_t trackEnd = trackStart + SafeINT32(trackLen);
    D2("before drag check axisPos=%d  trackStart=%d  trackEnd=%d  thumbPos=%d  thumbLen=%d", axisPos, trackStart,
        trackEnd, thumbPos, thumbLen);
    if(axisPos >= trackStart + static_cast<int32_t>(thumbPos)
        && axisPos < trackStart + static_cast<int32_t>(thumbPos + thumbLen)) {
      D2("Start dragging")
      mDragging = true;
      mDragStartPos = axisPos;
      mDragStartValue = mValue;
      return this;
    }
// Click on track (page step)
    if(axisPos >= trackStart && axisPos < trackEnd) {
      if(axisPos < trackStart + static_cast<int32_t>(thumbPos)) {
// Page up/left
        int32_t newVal = mValue - mPageStep;
        Value(newVal);
      }
      else {
// Page down/right
        int32_t newVal = mValue + mPageStep;
        Value(newVal);
      }
      return this;
    }
    return nullptr;
  }

  AWidget* AScrollBar::OnMouseUpLeft(UNUSED int32_t x, UNUSED int32_t y) {
    if(mDragging) {
      mDragging = false;
      return this;
    }
    return nullptr;
  }

  bool AScrollBar::OnMouseMove(int32_t x, int32_t y) {
    D("mValue=%d, mDragStartValue=%d", mValue, mDragStartValue)
    D1("incoming local: (%d,%d)  widget pos: (%d,%d)  size: (%d,%d)", x, y, mX, mY, mSizeX, mSizeY);
    if(!mDragging)
      return false;
    bool horiz = (Orient() == AUIOrientation::horizontal);
    int32_t axisPos = horiz ? x : y;
    D2("local (%d,%d), axisPos=%d", x, y, axisPos);
    D2("mDragStartPos=%d, mDragStartValue=%d", mDragStartPos, mDragStartValue);
    int32_t delta = axisPos - mDragStartPos;
    int32_t trackLen = SafeINT32(TrackLength());
    int32_t thumbLen = SafeINT32(ThumbLength());
    int32_t available = trackLen - thumbLen;
    int32_t effectiveMax = mMaxValue - mMinValue - mPageStep;
    if(effectiveMax <= 0 || available <= 0)
      return true;
    int32_t valDelta = static_cast<int32_t>((double) delta * effectiveMax / available);
    int32_t newVal = mDragStartValue + valDelta;
// Value() already handles effectiveMax clamping and sends the callback + redraw request
    Value(newVal);
    return true;
  }

}// namespace aui
