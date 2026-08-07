#ifndef AWIDGET_H_
#define AWIDGET_H_

namespace aui {

// Draw a horizontal line (1‑pixel high) using memset for each row (just one row here).
  inline void DrawHLine(uint32_t *buffer, uint32_t bufferWidth, int32_t x, int32_t y, int32_t w, uint32_t color) {
    if(w <= 0)
      return;
    uint32_t* line = buffer + static_cast<size_t>(y) * bufferWidth + static_cast<size_t>(x);
    std::fill(line, line + w, color);
  }
// Draw a vertical line (1‑pixel wide) – we fill each row’s pixel individually.
  inline void DrawVLine(uint32_t *buffer, uint32_t bufferWidth, int32_t x, int32_t y, int32_t h, uint32_t color) {
    if(h <= 0)
      return;
    for(int32_t row = 0; row < h; ++row) {
      uint32_t* pixel = buffer + static_cast<size_t>(y + row) * bufferWidth + static_cast<size_t>(x);
      *pixel = color;
    }
  }

  class AUI;
  class AWindow;
  class AWidget;
  using MouseMoveCallback = std::function<void(AWidget*, void*, int32_t, int32_t)>;
  using ResizeCallback = std::function<void(AWidget*, void*, uint32_t, uint32_t)>;
  using MouseButtonCallback = std::function<AWidget*(AWidget*, void* anyData, int32_t x, int32_t y)>;
  using MouseButtonCallback3 = std::function<void(AWidget*, void* anyData, int32_t x, int32_t y, uint32_t button)>;

  struct ARect { int32_t x, y; uint32_t w, h; };
  struct ATextStyle {
      uint32_t color;
      uint32_t fontSize;
      AUIHAlign hAlign;
      AUIVAlign vAlign;
      double angle;
  };

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

  void DrawTextEx(uint32_t* buffer, uint32_t parentWidth, uint32_t parentHeight, const ARect& bounds,
                  const std::string& text, FT_Face face, const ATextStyle& style,
                  const ARect* customClip);
  void DrawRotatedRect(uint32_t* buffer, uint32_t stride, int32_t clipMinX, int32_t clipMinY, int32_t clipMaxX,
      int32_t clipMaxY, int32_t rectX, int32_t rectY, uint32_t rectW, uint32_t rectH, double angleDeg, int32_t parentX,
      int32_t parentY, uint32_t parentW, uint32_t parentH, double parentAngleDeg, uint32_t color);

  class AWidget {
      template<typename T> friend class AWidgetFactory;
      friend class AWindow;
    private:
      std::vector<std::unique_ptr<AWidget>> mWidg;
      AWindow *mWnd = nullptr;
      bool mClipChildren = true;
      // TODO bug in hitbox clipping in extended box example
      bool mClipChildrenHitbox = true;
      AWidget *mParent = nullptr;
      bool mTextMetricsValid = false;
      double mAngle = 0.0;
      double mAbsoluteAngle = 0.0;
      bool mHL = false;
      bool mFocusable = false;
      AUIDirection mDirect = AUIDirection::unset;
      bool mHLEnabled = false;
      bool mConsumeMouseEvents = false;
      bool mMousePressedLeft = false;
      bool mMouseLeftRequireRelese = false;
      bool mInitDone = false;
      MouseButtonCallback mMousePressLeftCallback = nullptr;
      MouseButtonCallback mMousePressRightCallback = nullptr;
      MouseButtonCallback mMousePressMiddleCallback = nullptr;
      MouseButtonCallback3 mMousePressOtherCallback = nullptr;
      MouseButtonCallback mMouseReleaseLeftCallback = nullptr;
      MouseButtonCallback mMouseReleaseRightCallback = nullptr;
      MouseButtonCallback mMouseReleaseMiddleCallback = nullptr;
      MouseButtonCallback3 mMouseReleaseOtherCallback = nullptr;
      ResizeCallback mResizeCallback = nullptr;
      void* mMousePressLeftCallbackData = nullptr;
      void* mMousePressRightCallbackData = nullptr;
      void* mMousePressMiddleCallbackData = nullptr;
      void* mMousePressOtherCallbackData = nullptr;
      void* mMouseReleaseLeftCallbackData = nullptr;
      void* mMouseReleaseRightCallbackData = nullptr;
      void* mMouseReleaseMiddleCallbackData = nullptr;
      void* mMouseReleaseOtherCallbackData = nullptr;
      void* mResizeCallbackData = nullptr;
      void* mMouseMoveCallbackData = nullptr;
      MouseButtonCallback mMouseClickCallback = nullptr;
      void* mMouseClickCallbackData = nullptr;
      AWidget* ProcessMouseEvent(int32_t x, int32_t y, AWidget* (AWidget::*childHandler)(int32_t, int32_t),
          MouseButtonCallback callback, void* callbackData);
// Extended dispatcher (Other Press/Release)
      AWidget* ProcessMouseEventEx(int32_t x, int32_t y, uint32_t button,
          AWidget* (AWidget::*childHandler)(int32_t, int32_t, uint32_t), MouseButtonCallback3 callback,
          void* callbackData);
      AUIOrientation mOrient = AUIOrientation::unset;
      bool mLayoutDirty = true;
      virtual void OnMouseDownLeftInternal(int32_t localX, int32_t localY);
      virtual void OnMouseUpLeftInternal(int32_t localX, int32_t localY);
      bool mIsModal = false;
    protected:
      uint32_t mSizeX = 0;
      uint32_t mSizeY = 0;
      int32_t mX = 0;
      int32_t mY = 0;
      int32_t mLastMouseX = 0;
      int32_t mLastMouseY = 0;
      AUIWidgetType mType = AUIWidgetType::unset;
      uint32_t mBorderThick = 0;
      uint32_t mBGColor = 0xFFFFFFFF;
      uint32_t mBGColor2 = 0xFFFFFFFF;
      uint32_t mBGColor3 = 0xFFFFFFFF;
      uint32_t mBGColor4 = 0xFFFFFFFF;
      uint32_t mTextColor = 0xFF000000;
      uint32_t mBorderColor = 0xFF000000;
      std::string mText = "";
      AUIHAlign mHAlign = AUIHAlign::center;
      AUIVAlign mVAlign = AUIVAlign::center;
      uint32_t mFontSize = 14U;
      bool mEnabled = true;
      bool mVisible = true;
      bool mPressed = false;
      bool mDefaultFillBG = true;
      bool mDefaultDrawBorder = true;
      bool mCapSizeToParent = false;
      void AddWidget(std::unique_ptr<AWidget> widg);
      virtual void OnDraw(uint32_t *buffer, uint32_t bufferW, uint32_t bufferH,
          int32_t offsetX, int32_t offsetY, int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const = 0;
      void CapSizeToParent();
      void CapSizeToParent(bool v) {mCapSizeToParent = v;if(v){CapSizeToParent();}}
      MouseMoveCallback mMouseMoveCallback = nullptr;
      void* mMouseMoveUserData = nullptr;
      virtual void OnKeyEvent(const AUIKeyEvent&) {E("void filler")}
      template<typename Func>
      void ForEachChild(Func&& fn) {
        for(auto& child : mWidg) {
          fn(child);
        }
      }

    public:
      virtual ~AWidget() = default;
      void DrawChildren(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
          int32_t clipLeft, int32_t clipTop, int32_t clipRight, int32_t clipBottom) const;
      void Parent(AWidget* parent);
      bool Visible() const {return mVisible;}
      bool Focusable() const {return mFocusable;}
      void Focusable(bool v) {mFocusable = v;}
      uint32_t SizeX() const {return mSizeX;}
      uint32_t SizeY() const {return mSizeY;}
      int32_t X() const {return mX;}
      int32_t Y() const {return mY;}
      int32_t AbsX() const;
      int32_t AbsY() const;
      void DrawBorder(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
          int32_t clipLeft, int32_t clipTop, int32_t clipRight, int32_t clipBottom) const;
      void DrawRotatedBorder(uint32_t *buffer, uint32_t bufferW, UNUSED uint32_t bufferH,
          int32_t clipMinX, int32_t clipMinY,
          int32_t clipMaxX, int32_t clipMaxY,
          int32_t absX, int32_t absY,
          uint32_t sizeX, uint32_t sizeY,
          uint32_t borderThick, uint32_t borderColor,
          double angleDeg,
          int32_t parentX, int32_t parentY,
          uint32_t parentW, uint32_t parentH,
          double parentAngleDeg) const;
      void OnDrawBG(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
          int32_t clipLeft, int32_t clipTop, int32_t clipRight, int32_t clipBottom) const;
      uint32_t Border() const {return mBorderThick;}
      uint32_t BorderColor() const {return mBorderColor;}
      void X(int32_t x) {mX = x;}
      void Y(int32_t y) {mY = y;}
      void SizeX(uint32_t szx) {mSizeX = szx;}
      void SizeY(uint32_t szy) {mSizeY = szy;}
      void Border(uint32_t border) {mBorderThick = border;}
      void BorderColor(uint32_t border) {mBorderColor = border;}
      virtual void Draw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
          int32_t clipLeft, int32_t clipTop, int32_t clipRight, int32_t clipBottom) const;
      uint32_t BGColor() const {return mBGColor;}
      uint32_t BGColor2() const {return mBGColor2;}
      uint32_t BGColor3() const {return mBGColor3;}
      uint32_t BGColor4() const {return mBGColor4;}
      void BGColor(uint32_t bg);
      void BGColor2(uint32_t bg);
      void BGColor3(uint32_t bg);
      void BGColor4(uint32_t bg);
      uint32_t TextColor() const {return mTextColor;}
      void TextColor(uint32_t tx) {mTextColor = tx;}
      AWindow* Wnd() const {return mWnd;}
      void Wnd(AWindow* win);
      void Move(int32_t x, int32_t y);
      void Resize(uint32_t szx, uint32_t szy);
      void ClipChildren(bool clip) {mClipChildren = clip;}
      bool ClipChildren() const {return mClipChildren;}
      void ClipChildrenHitbox(bool clip) {mClipChildrenHitbox = clip;}
      bool ClipChildrenHitbox() const {return mClipChildrenHitbox;}
      AUIWidgetType Type() {D3(); return mType;}
      void Type(AUIWidgetType tp) {D3(); mType = tp;}
      uint32_t FontSize() const {return mFontSize;}
      virtual void FontSize(uint32_t size);
      double Angle() const {D4() return mAngle;};
      double AngleAbs() const;
      void Angle(double an);
      std::string Text() const {D4() return mText;}
      void Text(std::string tx);
      AUIHAlign HAlign() const {return mHAlign;}
      AUIVAlign VAlign() const {return mVAlign;}
      virtual void HAlign(AUIHAlign a) {mHAlign = a;}
      virtual void VAlign(AUIVAlign a) {mVAlign = a;}
      bool Enabled() const {return mEnabled;}
      void Enabled(bool e) {mEnabled = e;}
      virtual void Enable() {mEnabled = true;}
      virtual void Disable() {mEnabled = false;}
      virtual bool OnMouseMove(int32_t localX, int32_t localY);
      virtual AWidget* OnMouseDownLeft(int32_t localX, int32_t localY);
      virtual AWidget* OnMouseDownRight(int32_t localX, int32_t localY);
      virtual AWidget* OnMouseDownMiddle(int32_t localX, int32_t localY);
      virtual AWidget* OnMouseDownOther(int32_t localX, int32_t localY, uint32_t button);
      virtual AWidget* OnMouseUpLeft(int32_t localX, int32_t localY);
      virtual AWidget* OnMouseUpRight(int32_t localX, int32_t localY);
      virtual AWidget* OnMouseUpMiddle(int32_t localX, int32_t localY);
      virtual AWidget* OnMouseUpOther(int32_t localX, int32_t localY, uint32_t button);
      virtual void OnMouseWheel(int32_t delta);
      virtual void OnResize(uint32_t x, uint32_t y);
      AWidget* MouseClick(int32_t localX, int32_t localY);
      AWidget* MouseDown(int32_t localX, int32_t localY);
      AWidget* MouseUp(int32_t localX, int32_t localY);
      bool MouseMove(int32_t localX, int32_t localY);
      bool DispatchMouseWheel(int32_t parentX, int32_t parentY, int32_t delta);
      std::pair<int32_t, int32_t> GetAbsolutePosition() const;
      bool ForwardMouseWheelToChildren(int32_t delta);
      bool ForwardMouseMoveToChildren(int32_t x, int32_t y);
      AWidget* HitTestLocal(int32_t parentX, int32_t parentY);
      void HLToggle(bool v) {mHLEnabled = v;}
      void HL(bool v);
      bool HL() const {return mHL;}
      void DefaultFillBG(bool v);
      bool DefaultFillBG() {return mDefaultFillBG;}
      void SetMousePressLeftCallback(MouseButtonCallback callback, void* anyData);
      MouseButtonCallback GetMousePressCallback() {return mMousePressLeftCallback;}
      void SetMousePressRightCallback(MouseButtonCallback callback, void* anyData);
      MouseButtonCallback GetMousePressRightCallback() {return mMousePressRightCallback;}
      void SetMousePressMiddleCallback(MouseButtonCallback callback, void* anyData);
      MouseButtonCallback GetMousePressMiddleCallback() {return mMousePressMiddleCallback;}
      void SetMousePressOtherCallback(MouseButtonCallback3 callback, void* anyData);
      MouseButtonCallback3 GetMousePressOtherCallback() {return mMousePressOtherCallback;}
      void SetMouseReleaseLeftCallback(MouseButtonCallback callback, void* anyData);
      MouseButtonCallback GetMouseReleaseCallback() {return mMouseReleaseLeftCallback;}
      void SetMouseReleaseRightCallback(MouseButtonCallback callback, void* anyData);
      MouseButtonCallback GetMouseReleaseRightCallback() {return mMouseReleaseRightCallback;}
      void SetMouseReleaseMiddleCallback(MouseButtonCallback callback, void* anyData);
      MouseButtonCallback GetMouseReleaseMiddleCallback() {return mMouseReleaseMiddleCallback;}
      void SetMouseReleaseOtherCallback(MouseButtonCallback3 callback, void* anyData);
      MouseButtonCallback3 GetMouseReleaseOtherCallback() {return mMouseReleaseOtherCallback;}
      void SetMouseMoveCallback(MouseMoveCallback callback, void* anyData);
      MouseMoveCallback GetMouseMoveCallback() {return mMouseMoveCallback;}
      std::pair<int32_t, int32_t> CalculateCoordsRotated(int32_t x, int32_t y) const;
      void SetMouseClickCallback(MouseButtonCallback callback, void* anyData);
      MouseButtonCallback GetMouseClickCallback() {return mMouseClickCallback;}
      void ConsumeMouseEvents(bool v) {mConsumeMouseEvents = v;}
      bool ConsumesMouseEvents() {return mConsumeMouseEvents;}
      bool MouseLeftReleaseRequired() {return mMouseLeftRequireRelese;}
      void MouseLeftReleaseRequired(bool v) {mMouseLeftRequireRelese = v;}
      bool IsPressedLeft() {return mMousePressedLeft;}
      void TrimToText();
      void MousePressedLeftToggle(bool v) {mMousePressedLeft = v;}
      bool MousePressedLeft() {return mMousePressedLeft;}
      void Show();
      void Hide();
      void Visible(bool v);
      AUI* EnginePtr();
      AWidget* Parent() const {return mParent;}
      AUIOrientation Orient() const {return mOrient;}
      void Orient(AUIOrientation v) {mOrient = v;}
      std::pair<int32_t, int32_t> ToLocalCoords(int32_t winX, int32_t winY) const;
      void GetRotatedAABB(int32_t offsetX, int32_t offsetY,
                                  double& outMinX, double& outMaxX,
                                  double& outMinY, double& outMaxY) const;
      AUIDirection Direction() const {return mDirect;}
      void Direction(AUIDirection v) {mDirect = v;}
      virtual void LayoutUpdate();
      void LayoutDirty() {mLayoutDirty = true;};
      bool LayoutIsDirty() {return mLayoutDirty;}
      void LayoutDirtyToggle(bool v) {mLayoutDirty = v;}
      bool Focused() const;
      uint32_t ShiftColor(uint32_t color, bool doubleShift) const;
      virtual void OnFocusGained() {D2("unimplemented void filler")  }
      virtual void OnFocusLost() {D2("unimplemented void filler")}
      void Init();
      void Init(bool v) {if(v){mInitDone = true;} else {E("can't set init false")} };
      bool InitDone() const {return mInitDone;}
      bool Pressed() const {return mPressed;}
      void Pressed(bool v);
      void BringToFront(AWidget *child);
      bool Modal() const { return mIsModal; }
      AWidget* FindFirstFocusable();
      void Modal(bool modal);
      bool IsDescendantOf(const AWidget* ancestor) const;
      bool DefaultDrawBorder() {return mDefaultDrawBorder;}
      void DefaultDrawBorder(bool v) {mDefaultDrawBorder = v;}
      int32_t ComputeTextWidth(const std::string& text) const;

  };

  template<typename Derived>
  class AWidgetFactory: public AWidget {
    protected:
      AWidgetFactory() = default;
    public:
// Disallow copy/move at the factory level to protect all derived widgets
      AWidgetFactory(const AWidgetFactory&) = delete;
      AWidgetFactory& operator=(const AWidgetFactory&) = delete;
      AWidgetFactory(AWidgetFactory&&) = delete;
      AWidgetFactory& operator=(AWidgetFactory&&) = delete;
      template<typename WindowType = AWindow, typename ... Args>
      static Derived* AttachTo(WindowType* parent, Args&& ... args) {
        if(!parent)
          return nullptr;
        std::unique_ptr < Derived > widget(new Derived(std::forward<Args>(args)...));
        Derived* rawPtr = widget.get();
        if constexpr (std::is_same_v<WindowType, AWindow>) {
          rawPtr->Wnd(parent);
        }
        else {
          rawPtr->Wnd(parent->Wnd());
          rawPtr->Parent(parent);
          rawPtr->Angle(0);
        }
        parent->AddWidget(std::move(widget));
        rawPtr->Init();
        if(rawPtr->mWnd) {
          rawPtr->mWnd->RequestRedraw();
        }
        return rawPtr;
      }
  };

  template<typename WidgetType>
  class AWidgetReader {
      static_assert(std::is_base_of_v<AWidget, WidgetType>,
          "WidgetType must derive from AWidget");
    private:
      std::vector<uint32_t> mBuffer;
      uint32_t mSizeX = 0;
      uint32_t mSizeY = 0;
    public:
      AWidgetReader() = default;
// Direct Constructor for One-Liner Initialization
      AWidgetReader(const WidgetType* widget, uint32_t canvasWidth, uint32_t canvasHeight, uint32_t clearColor =
          0x00000000) {
        Render(widget, canvasWidth, canvasHeight, clearColor);
      }
      AWidgetReader(const WidgetType& widget, uint32_t canvasWidth, uint32_t canvasHeight, uint32_t clearColor =
          0x00000000) {
        Render(&widget, canvasWidth, canvasHeight, clearColor);
      }
      bool Render(const WidgetType* widget, uint32_t canvasWidth, uint32_t canvasHeight, uint32_t clearColor =
          0x00000000) {
        if(!widget || canvasWidth == 0 || canvasHeight == 0)
          return false;
        mSizeX = canvasWidth;
        mSizeY = canvasHeight;
        mBuffer.assign(static_cast<size_t>(mSizeX) * mSizeY, clearColor);
// Render the widget as a standalone root.
// Cast mWidth and mHeight to int32_t for clipRight and clipBottom
        widget->Draw(mBuffer.data(), mSizeX, mSizeY, 0, 0,// offsetX, offsetY
            0, 0,// clipLeft, clipTop
            static_cast<int32_t>(mSizeX),// clipRight
            static_cast<int32_t>(mSizeY));// clipBottom
        return true;
      }
      uint32_t Pixel(int32_t x, int32_t y) const {
        if(x < 0 || y < 0 || static_cast<uint32_t>(x) >= mSizeX || static_cast<uint32_t>(y) >= mSizeY) {
          return 0x00000000;
        }
        return mBuffer[static_cast<size_t>(y) * mSizeX + static_cast<size_t>(x)];
      }
      const uint32_t* Buffer() const {
        return mBuffer.data();
      }
      uint32_t SizeX() const {
        return mSizeX;
      }
      uint32_t SizeY() const {
        return mSizeY;
      }
  };

}

#endif // AWIDGET_H_
