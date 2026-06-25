#ifndef ASCROLLBAR_H_
#define ASCROLLBAR_H_

namespace aui {

using ScrollCallback = std::function<void(AWindow*, AWidget*, void*, int32_t)>;

class AScrollBar : public AWidget {
private:
  AUIOrientation mOrientation = AUIOrientation::vertical;
  int32_t mMinValue = 0;
  int32_t mMaxValue = 100;
  int32_t mValue = 0;
  int32_t mPageStep = 10;        // number of document units visible in the viewport
  int32_t mSingleStep = 1;       // step for arrow clicks (arrows not yet implemented)
  uint32_t mTrackThickness = 4;  // thin line (width for vertical, height for horizontal)
  uint32_t mThumbThickness = 8;  // thicker than track
  uint32_t mThumbColor = 0xFF888888;
  uint32_t mTrackColor = 0xFFCCCCCC;
  bool mDragging = false;
  int32_t mDragStartPos = 0;     // mouse coordinate along the axis at drag start
  int32_t mDragStartValue = 0;
  uint32_t mArrowSize = 12;                // width/height of arrow area
  bool mShowArrows = true;
  bool mArrowTopHover = false;
  bool mArrowBottomHover = false;
  bool mArrowLeftHover = false;
  bool mArrowRightHover = false;
  bool mArrowTopPressed = false;
  bool mArrowBottomPressed = false;
  bool mArrowLeftPressed = false;
  bool mArrowRightPressed = false;
  int32_t mDragOffset = 0;   // offset from thumb's top to click point
  int32_t ValueFromCoord(int32_t coord) const;
  int32_t mLastDrawnValue = 0;
  bool IsInTopArrow(int32_t localX, int32_t localY) const;
  bool IsInBottomArrow(int32_t localX, int32_t localY) const;
  bool IsInLeftArrow(int32_t localX, int32_t localY) const;
  bool IsInRightArrow(int32_t localX, int32_t localY) const;
protected:
  void DrawTrack(uint32_t* buffer, uint32_t parentWidth, uint32_t parentHeight,
                 int32_t offsetX, int32_t offsetY) const;
  void DrawThumb(uint32_t* buffer, uint32_t parentWidth, uint32_t parentHeight,
                 int32_t offsetX, int32_t offsetY) const;
  void DrawArrows(uint32_t* buffer, uint32_t parentWidth, uint32_t parentHeight,
                  int32_t offsetX, int32_t offsetY) const;
public:
  AScrollBar();
  ~AScrollBar() override = default;
  void Draw(uint32_t* buffer, uint32_t parentWidth, uint32_t parentHeight,
            int32_t offsetX, int32_t offsetY) const override;
  bool OnMouseClick(int32_t localX, int32_t localY, bool pressed) override;
  void OnMouseMove(int32_t localX, int32_t localY) override;
  static AScrollBar* AttachTo(AWindow* parent, AUIOrientation orientation = AUIOrientation::vertical);
  static AScrollBar* AttachTo(AWidget* parent, AUIOrientation orientation = AUIOrientation::vertical);
  void SetOrientation(AUIOrientation orient);
  void SetRange(int32_t minVal, int32_t maxVal);
  void SetValue(int32_t val);
  void SetPageStep(int32_t step);
  void SetSingleStep(int32_t step);
  void SetTrackThickness(uint32_t thick);
  void SetThumbThickness(uint32_t thick);
  void SetThumbColor(uint32_t color);
  void SetTrackColor(uint32_t color);
  AUIOrientation GetOrientation() const { return mOrientation; }
  int32_t GetMinValue() const { return mMinValue; }
  int32_t GetMaxValue() const { return mMaxValue; }
  int32_t GetValue() const { return mValue; }
  int32_t GetPageStep() const { return mPageStep; }
  int32_t GetSingleStep() const { return mSingleStep; }
  uint32_t GetTrackThickness() const { return mTrackThickness; }
  uint32_t GetThumbThickness() const { return mThumbThickness; }
  uint32_t GetThumbColor() const { return mThumbColor; }
  uint32_t GetTrackColor() const { return mTrackColor; }
  void OnParentResize(uint32_t newWidth, uint32_t newHeight) override;
  void SetScrollCallback(ScrollCallback callback, void* userData = nullptr);
  uint32_t GetThumbPosition() const;    // offset from track start (pixels)
  void ShowArrows(bool state) {mShowArrows = state;}
  void SetArrowSize(uint32_t sz) {mArrowSize = sz;}
  void SetPosition(int32_t x, int32_t y) { mX = x; mY = y; }
  void SetVisible(bool visible) { mVisible = visible; if(mParentWindow) mParentWindow->Draw(); }
  bool IsVisible() const { return mVisible; }
  uint32_t GetTrackLength() const;      // length along the scrollbar axis (height for vertical, width for horizontal)
  uint32_t GetThumbLength() const;      // clamped to at least 20 pixels
  void SetShowArrows(bool show) {mShowArrows = show;if (mParentWindow) mParentWindow->Draw();}

};

} // namespace aui

#endif // ASCROLLBAR_H_

