#ifndef ASCROLLBAR_H_
#define ASCROLLBAR_H_

namespace aui {

using ScrollCallback = std::function<void(AWidget*, void*, int32_t)>;

class ATable;

class AScrollBar : public AWidgetFactory<AScrollBar> {
    friend class AWidgetFactory<AScrollBar>;
    friend class ATable; // TODO REMOVE WHEN ATable refactored for it
private:
  int32_t mMinValue = 0;
  int32_t mMaxValue = 100;
  int32_t mValue = 0;
  int32_t mPageStep = 10;        // number of document units visible in the viewport
  int32_t mSingleStep = 1;       // step for arrow clicks (arrows not yet implemented)
  uint32_t mTrackThick = 4;  // thin line (width for vertical, height for horizontal)
  uint32_t mThumbThick = 8;  // thicker than track
  bool mDragging = false;
  uint32_t mArrowSize = 12;                // width/height of arrow area
  bool mShowArrows = true;
  int32_t mDragStartPos = 0;
  int32_t mDragStartValue = 0;
  ScrollCallback mScrollCallback = nullptr;
  void* mScrollUserData = nullptr;
protected:
  AScrollBar();
  AScrollBar(AUIOrientation v);
  void DrawTrack(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH,
      int32_t offsetX, int32_t offsetY, uint32_t clipX, uint32_t clipY) const;
  void DrawThumb(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH,
      int32_t offsetX, int32_t offsetY, uint32_t clipX, uint32_t clipY) const;
  void DrawArrows(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH,
      int32_t offsetX, int32_t offsetY, uint32_t clipX, uint32_t clipY) const;
public:
  virtual void OnDraw(uint32_t *buffer, uint32_t bufferW, uint32_t bufferH,
      int32_t offsetX, int32_t offsetY, int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const;
  ~AScrollBar() override = default;
  void Range(int32_t minVal, int32_t maxVal);
  void Value(int32_t val);
  void PageStep(int32_t step);
  void SingleStep(int32_t step);
  void TrackThick(uint32_t thick);
  void ThumbThick(uint32_t thick);
  void ThumbColor(uint32_t ARGBcolor);
  void TrackColor(uint32_t ARGBcolor);
  int32_t MinValue() const { return mMinValue; }
  int32_t MaxValue() const { return mMaxValue; }
  int32_t Value() const { return mValue; }
  int32_t PageStep() const { return mPageStep; }
  int32_t SingleStep() const { return mSingleStep; }
  uint32_t TrackThick() const { return mTrackThick; }
  uint32_t ThumbThick() const { return mThumbThick; }
  uint32_t ThumbColor() const { return mBGColor3; }
  uint32_t TrackColor() const { return mBGColor4; }
  void SetScrollCallback(ScrollCallback callback, void* userData = nullptr);
  uint32_t ThumbPosition() const;    // offset from track start (pixels)
  void Arrows(bool state) {mShowArrows = state;}
  void ArrowSize(uint32_t sz) {mArrowSize = sz;}
  uint32_t TrackLength() const;      // length along the scrollbar axis (height for vertical, width for horizontal)
  uint32_t ThumbLength() const;      // clamped to at least 20 pixels
  void ShowArrows(bool show) {mShowArrows = show;if(Wnd())Wnd()->RequestRedraw();}
  AWidget* OnMouseDownLeft(int32_t x, int32_t y) override;
  AWidget* OnMouseUpLeft(int32_t x, int32_t y) override;
  bool OnMouseMove(int32_t localX, int32_t localY) override;

};

} // namespace aui

#endif // ASCROLLBAR_H_

