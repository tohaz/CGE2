#ifndef AWINDOW_H_
#define AWINDOW_H_

namespace aui {
  class IBuffer {
    public:
      bool isBusy = false;
      virtual ~IBuffer() = default;
      virtual void Release() = 0;// Backend-specific cleanup
  };
  class WaylandWindowContext;
  class AUI;
  class AComboBox;
  class AMenu;
  using WndMouseButtonCallback3 = std::function<void(AWindow*, void* anyData, int32_t x, int32_t y, uint32_t button)>;

  std::pair<int32_t, int32_t> CalculateCoordsRotatedFull(int32_t x, int32_t y, uint32_t sizeX, uint32_t sizeY, double angle);

  class AWindow {
      friend class WaylandWindowContext;
      friend class AWidget;
    private:
      AUI* mAUI = nullptr;
      uint32_t mSizeX = AUI_DEFAULT_WINDOW_SZX;
      uint32_t mSizeY = AUI_DEFAULT_WINDOW_SZY;
      uint32_t mBGColor = AUI_DEFAULT_WINDOW_BG;
      std::string mTitle = "set me";
      std::vector<std::unique_ptr<AWidget>> mWidg;
      bool mClosing = false;
      AUIWindowType mType = AUIWindowType::unset;
      int32_t mCurrentRenderIdx = 0;
      uint32_t mTargetFPS = 20;
      bool mResize = false;
      std::atomic<bool> mNeedsRepaint{false};
      AWidget* mHLWidget = nullptr;
      WndMouseButtonCallback3 mMousePressCallback = nullptr;
      void* mMousePressCallbackData = nullptr;
      WndMouseButtonCallback3 mMouseReleaseCallback = nullptr;
      void* mMouseReleaseCallbackData = nullptr;
      AWidget* mCapturedWidgetLeft = nullptr;
      bool mClipChildrenHitbox = true;
// TODO move to instrumentation
      uint64_t mRedrawCounter = 0;
      uint64_t mRedrawCounterDone = 0;
      AWidget* mFocusedWidget = nullptr;
      AComboBox* mActiveDropDown = nullptr;
      std::vector<AWidget*> mModalStack;
      AMenu* mActiveMenu = nullptr;
      AMenu* mPermanentMenu = nullptr;
      bool mHidden = false;
    protected:
      AWindow();
      void Type(AUIWindowType);
      void EnginePtr(AUI* ptr);
      bool mDecorations = true;   // default enabled
      virtual void UpdateDecorations() {E("default method called")}
      int32_t FindFreeBufferIndex() const;
      virtual void BackendResize(uint32_t x, uint32_t y) = 0;
      virtual void BackendTitle(std::string title) = 0;
      virtual void BackendDisableResize() = 0;
      virtual void BackendShow() = 0;
      virtual void BackendHide() = 0;
      virtual void BackendEnableResize() = 0;
      virtual void BackendMove(int32_t x, int32_t y) = 0;
      virtual bool CreateFrame() = 0;
      bool mResizeEnabled = false;
    public:
      virtual void BackendCursor(AUICursorType type) = 0;
      virtual uint64_t NativeWindowId() const = 0;
      virtual void Draw() = 0;
      std::array<std::unique_ptr<IBuffer>, AUI_NUM_BUFFERS> mBuffers;
      uint32_t SizeX() {D4() return mSizeX;}
      uint32_t SizeY() {D4() return mSizeY;}
      void SizeX(uint32_t szx) {D4() mSizeX = szx;}
      void SizeY(uint32_t szy) {D4() mSizeY = szy;}
      virtual ~AWindow() = default;
      static AWindow* AttachTo(AUI *engine, const std::string &title);
      static AWindow* AttachTo(AUI *au, const std::string &title, AUIWindowType type);
      void Title(const std::string &title);
      std::string& Title() {return mTitle;}
      AUIWindowType Type() {return mType;}
      void AddWidget(std::unique_ptr<AWidget> widg);
      void Resize(uint32_t x, uint32_t y);
      void Move(int32_t x, int32_t y);
      void Draw(void* pixels);
      bool Close();
      uint32_t BGColor() {return mBGColor;}
      void BGColor(uint32_t v);
      AUI* EnginePtr();
      void Decorations(bool enable) { mDecorations = enable; UpdateDecorations();}
      bool Decorations() const { return mDecorations; }
      void EnableResize();
      void DisableResize();
      void OnMousePress(int32_t x, int32_t y, uint32_t button);
      void MouseDown(int32_t x, int32_t y);
      void MouseUp(int32_t x, int32_t y);
      void OnMouseRelease(int32_t x, int32_t y, uint32_t button);
      void OnMouseMove(int32_t x, int32_t y);
      void OnMouseEnter(int32_t x, int32_t y);
      void OnMouseLeave(int32_t x, int32_t y);
      void OnMouseWheel(int32_t x, int32_t y, int32_t delta);
      bool NeedsRepaint() const { return mNeedsRepaint.load(std::memory_order_acquire); }
      void ClearRepaintFlag() { mNeedsRepaint.store(false, std::memory_order_release); }
      void RequestRedraw();
      AWidget* FindWidgetAt(int32_t x, int32_t y);
      void SetMousePressCallback(WndMouseButtonCallback3 callback, void* anyData);
      void SetMouseReleaseCallback(WndMouseButtonCallback3 callback, void* anyData);
      WndMouseButtonCallback3 GetMousePressCallback() {return mMousePressCallback;}
      WndMouseButtonCallback3 GetMouseReleaseCallback() {return mMouseReleaseCallback;}
      bool IsResizeEnabled() {return mResize;}
      void MouseClick(int32_t x, int32_t y);
      void LayoutUpdate();
      AWidget* FocusedWidget() const {return mFocusedWidget;}
      void FocusedWidget(AWidget* v);
      void OnKeyEvent(const AUIKeyEvent &event);
      AComboBox* ActiveDropdown() {return mActiveDropDown;}
      void ActiveDropdown(AComboBox* v) {mActiveDropDown = v;}
      void BringToFront(AWidget *child);
      bool ClipChildrenHitbox() {return mClipChildrenHitbox;}
      void ClipChildrenHitbox(bool v) {mClipChildrenHitbox = v;}
      void Clear() {mWidg.clear();}
      void PushModal(AWidget* widget);
      void PopModal();
      bool HasModal() const { return !mModalStack.empty(); }
      AWidget* TopModal() const { return mModalStack.empty() ? nullptr : mModalStack.back();}
      void RemoveModal(AWidget* widget);
      bool ProcessDropdown(int32_t x, int32_t y);
      AMenu* ActiveMenu() const {return mActiveMenu; }
      void ActiveMenu(AMenu* v) { mActiveMenu = v; }
      AMenu* PermanentMenu() const {return mPermanentMenu;}
      void PermanentMenu(AMenu* v) {mPermanentMenu = v;}
      void RemoveWidget(AWidget* v);
      void CapturedWidgetLeft(AWidget* v);
      void Show();
      void Hide();
      bool Hiddem() {return mHidden;}
      bool Visible() {return !mHidden;}
  };

}

#endif  //AWINDOW_H_
