#ifndef AWIDGET_H_
#define AWIDGET_H_

namespace aui {

  class AWindow;
  using ClickCallback = std::function<void(AWindow*, AWidget*, void*, int32_t, int32_t, bool)>;
  using MouseMoveCallback = std::function<void(AWindow*, AWidget*, void*, int32_t, int32_t)>;
  using ScrollCallback = std::function<void(AWindow*, AWidget*, void*, int32_t)>;
  void DrawTextEx(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t absX, int32_t absY,
        int32_t drawW, int32_t drawH, const std::string &text, FT_Face face, uint32_t fontSize, AUIHAlign hAlign,
        AUIVAlign vAlign, int32_t xOffset, uint32_t textColor, int32_t maxContentWidth);
  void DrawLine(uint32_t *buffer, uint32_t bufferWidth, uint32_t bufferHeight, int32_t x0, int32_t y0, int32_t x1,
      int32_t y1, uint32_t color);
  void DrawThickLine(uint32_t* buffer, uint32_t bufferWidth, uint32_t bufferHeight,
                            int32_t x0, int32_t y0, int32_t x1, int32_t y1,
                            uint32_t color, uint32_t thickness);

  // Fill a rectangle with a solid color.
// All coordinates and sizes are assumed to be already clipped to the buffer bounds.
  inline void FillRect(uint32_t *buffer, uint32_t bufferWidth, int32_t x, int32_t y, int32_t w, int32_t h,
      uint32_t color) {
    if(w <= 0 || h <= 0)
      return;
// We assume x,y,w,h are within [0, bufferWidth) and [0, bufferHeight).
// To be safe, we can still clamp, but the caller should have clipped.
    for(int32_t row = 0; row < h; ++row) {
      uint32_t *line = buffer + static_cast<size_t>(y + row) * bufferWidth + static_cast<size_t>(x);
      std::fill(line, line + w, color);
    }
  }

// Draw a horizontal line (1‑pixel high) using memset for each row (just one row here).
  inline void DrawHLine(uint32_t *buffer, uint32_t bufferWidth, int32_t x, int32_t y, int32_t w, uint32_t color) {
    if(w <= 0)
      return;
    uint32_t *line = buffer + static_cast<size_t>(y) * bufferWidth + static_cast<size_t>(x);
    std::fill(line, line + w, color);
  }

// Draw a vertical line (1‑pixel wide) – we fill each row’s pixel individually.
  inline void DrawVLine(uint32_t *buffer, uint32_t bufferWidth, int32_t x, int32_t y, int32_t h, uint32_t color) {
    if(h <= 0)
      return;
    for(int32_t row = 0; row < h; ++row) {
      uint32_t *pixel = buffer + static_cast<size_t>(y + row) * bufferWidth + static_cast<size_t>(x);
      *pixel = color;
    }
  }

  inline void DrawRectBorder(uint32_t *buffer, uint32_t bufferWidth, int32_t x, int32_t y, int32_t w, int32_t h,
      uint32_t color) {
    if(w <= 0 || h <= 0)
      return;
    DrawHLine(buffer, bufferWidth, x, y, w, color);// top
    DrawHLine(buffer, bufferWidth, x, y + h - 1, w, color);// bottom
    DrawVLine(buffer, bufferWidth, x, y + 1, h - 2, color);// left (skip corners)
    DrawVLine(buffer, bufferWidth, x + w - 1, y + 1, h - 2, color);// right
  }

  class AWidget {
      friend class ABox;
      friend class AButton;
      friend class ALabel;
      friend class AWindow;
      friend class AScrollBar;
      friend class AList;
      friend class AInputBox;
      friend class ATable;
      friend class AComboBox;
    private:
      uint64_t mId = 0U;
      bool mEnabled = true;
      bool mVisible = true;
      bool mIsPressed = false;
      AWidget(const AWidget&) = delete;
      AWidget& operator=(const AWidget&) = delete;
      ClickCallback mClickCallback;
      MouseMoveCallback mMoveCallback;
      ScrollCallback mScrollCallback;
      void* mCallbackUserData = nullptr;
      void* mClickUserData = nullptr;
      void* mMoveUserData = nullptr;
      void* mScrollUserData = nullptr;
      mutable int32_t mCachedTextWidth = 0;
      mutable int32_t mCachedTextHeight = 0;
      mutable bool mTextMetricsValid = false;
      bool mFocusable = false;
//      AWidget* mDragWidget = nullptr;
      void ClampToParent();
      void FitToParent();
      void CapSizeToParent();
      void ClampPositionToParent();
    protected:
      uint32_t mBorderThick = 0;
      uint32_t mBorderColor = 0;
      void AddWidget(std::unique_ptr<AWidget> widg);
      AWidget();
      AUI *mEnginePtr = nullptr;
      AWindow *mParentWindow = nullptr;
      AWidget *mParentWidget = nullptr;
      int32_t mX = 0;
      int32_t mY = 0;
      uint32_t mSizeX = 0;
      uint32_t mSizeY = 0;
      uint32_t mBGColor = 0;
      std::string mText;
      uint32_t mTextColor = 0xFF000000U;     // ARGB, default black
      AUIHAlign mHAlign = AUIHAlign::center;
      AUIVAlign mVAlign = AUIVAlign::center;
      uint32_t mFontSize = 14U;              // pixels
      virtual void OnParentResize(UNUSED uint32_t newWidth, UNUSED uint32_t newHeight);
      bool mHovered = false;
      bool mPressed = false;
      bool mHoverEnabled = true;
      // Border properties
      std::vector<std::unique_ptr<AWidget>> mWidg;
      AUIWidgetType mWidgetType = AUIWidgetType::unset;
      void InitWidgetProperties(uint64_t widgetId, AUI *engine, AWindow *window, AWidget *parent, AUIWidgetType type);
      void InvokeClickCallback(int32_t localX, int32_t localY, bool pressed);
      void InvokeMouseMoveCallback(int32_t localX, int32_t localY);
      void InvokeScrollCallback(int32_t value);
      virtual bool DispatchMouseMove(int32_t parentX, int32_t parentY);
      void DrawBorder(uint32_t* buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX, int32_t offsetY) const;
      void DrawTextOffset(uint32_t* buffer, uint32_t parentWidth, uint32_t parentHeight,
                          int32_t offsetX, int32_t offsetY, int32_t xOffset) const;
      int32_t mLastMouseX = 0;
      int32_t mLastMouseY = 0;
    public:
      virtual ~AWidget() = default;
      virtual void Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
          int32_t offsetY) const;
      virtual bool OnMouseClick(int32_t localX, int32_t localY, bool pressed);
      virtual void OnMouseMove(int32_t localX, int32_t localY);
      virtual void OnMouseWheel(UNUSED int32_t delta) {}
      virtual void OnFocusGained() {}
      virtual void OnFocusLost() {}
      virtual void OnKeyEvent(const AUIKeyEvent&) {D("generic method")}
      virtual bool DispatchClick(int32_t parentX, int32_t parentY, bool pressed);
      void Move(int32_t x, int32_t y);
      void Resize(uint32_t szx, uint32_t szy);
      int32_t X() const {return mX;}
      int32_t Y() const {return mY;}
      uint32_t SizeX() const {return mSizeX;}
      uint32_t SizeY() const {return mSizeY;}
      void SetBGColor(uint32_t color);
      uint32_t BGColor() const {return mBGColor;}
      virtual void Enable() {mEnabled = true;}
      virtual void Disable() {mEnabled = false;}
      void Show();
      void Hide();
      void AttachTo(AWidget *parent);
      void AttachTo(AWindow *parent);
      bool IsEnabled() const {return mEnabled;}
      bool IsVisible() const {return mVisible;}
      void SetPressed(bool pressed) {mIsPressed = pressed;}
      bool IsPressed() const {return mIsPressed;}
      void GetAbsolutePosition(int32_t &outX, int32_t &outY) const;
      AUI* GetEnginePtr() const {return mEnginePtr;}
      void SetClickCallback(ClickCallback callback, void* userData);
      void SetMouseMoveCallback(MouseMoveCallback callback, void* userData = nullptr);
      void SetScrollCallback(ScrollCallback callback, void* userData = nullptr);
      void SetText(const std::string& text);
      const std::string& GetText() const { return mText; }
      void SetTextColor(uint32_t color);
      uint32_t GetTextColor() const { return mTextColor; }
      uint32_t GetFontSize() const { return mFontSize; }
      virtual void SetHAlignment(AUIHAlign align);
      AUIHAlign GetHAlignment() const { return mHAlign; }
      void SetVAlignment(AUIVAlign align);
      AUIVAlign GetVAlignment() const { return mVAlign; }
      void SetBorderThickness(uint32_t thick);
      uint32_t GetBorderThickness() const { return mBorderThick; }
      void SetBorderColor(uint32_t color);
      uint32_t GetBorderColor() const { return mBorderColor; }
      AUIWidgetType GetWidgetType() {return mWidgetType;}
      void DrawText(uint32_t* buffer, uint32_t parentWidth, uint32_t parentHeight,
                    int32_t offsetX, int32_t offsetY) const;
      void UpdateTextMetrics() const;   // updates mCachedTextWidth, mCachedTextHeight
      uint32_t ShiftColor(uint32_t color, bool doubleShift = false) const;
      virtual void OnMouseLeave() {}
      AWindow* GetParentWindow() {return mParentWindow;}
      AWidget* GetParentWidget() {return mParentWidget;}
      int32_t GetCachedTextWidth() {return mCachedTextWidth;}
      uint32_t GetBGColor() {return mBGColor;}
      virtual void SetFontSize(uint32_t size);
      void SetFocusable(bool focusable) { mFocusable = focusable; }
      bool IsFocusable() const { return mFocusable; }
      bool IsFocused() const;   // implemented in .cpp
      int32_t MeasureTextWidth(const std::string &text) const;
      void DrawChildren(uint32_t* buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX, int32_t offsetY) const;
      // Forward a mouse click to the topmost child at (localX, localY)
      bool ForwardClickToChildren(int32_t localX, int32_t localY, bool pressed);
      // Forward a mouse move to the child under the cursor (updates hover state)
      void ForwardMoveToChildren(int32_t localX, int32_t localY);
      // Forward a mouse wheel event to the child under the last known mouse position
      void ForwardWheelToChildren(int32_t delta);
      void BringChildToFront(AWidget* child);



//      AWidget* GetDragWidget() const { return mDragWidget; }

  };

}

#endif // AWIDGET_H_
