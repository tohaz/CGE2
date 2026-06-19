#ifndef AWINDOW_H_
#define AWINDOW_H_

namespace aui {

  class IWindowContext;
  class AUI;
  class AWidget;

  class AWindow {
      friend class AWidget;
      friend class ABox;
      friend class ALabel;
      friend class AScrollBar;
      friend class AButton;
      friend class AList;
      friend class AInputBox;
      friend class ATable;
      friend class AComboBox;
      friend class AProgressBar;
    private:
      std::unique_ptr<IWindowContext> mBackend;
      std::string mWindowTitle;
      int32_t mX = 0, mY = 0;
      uint32_t mSizeX = 0, mSizeY = 0;
      uint32_t mBGColor = 0;
      bool mResizeEnabled = false;
      uint64_t mNativeId = 0;
      AUIWindowType mWindowType = AUIWindowType::unset;
      std::vector<std::unique_ptr<AWidget>> mWidg;
      AWidget *mDragWidget = nullptr;
      AWidget *mHoverWidget = nullptr;
      bool mDrawPending = false;
      void DoDraw();
      AWidget* mFocusedWidget = nullptr;
      bool mKeepFocusOnMouseLeave = true;
    protected:
      void AddWidget(std::unique_ptr<AWidget> widg);
    public:
      explicit AWindow(std::unique_ptr<IWindowContext> backend);
      ~AWindow();
      static AWindow* AttachTo(AUI *engine, const std::string &title);
      void Draw();
      void Move(int32_t x, int32_t y);
      void Resize(uint32_t w, uint32_t h);
      void SetTitle(const std::string &title);
      IWindowContext* GetBackend() const {return mBackend.get();}
// Accessors for backend to query size/position
      uint32_t SizeX() const {return mSizeX;}
      uint32_t SizeY() const {return mSizeY;}
      int32_t X() const {return mX;}
      int32_t Y() const {return mY;}
      uint32_t BGColor() const {return mBGColor;}
      void SetBGColor(uint32_t col) {mBGColor = col;}
      void EnableResize();
      void DisableResize();
      static AWindow* AttachTo(AUI *engine, const std::string &title, AUIWindowType type);
      void Close();
      uint64_t GetNativeId() const {return mNativeId;}
      bool IsResizeEnabled() {return mResizeEnabled;}
      void OnMousePress(int32_t x, int32_t y, uint32_t button);
      void OnMouseMove(int32_t x, int32_t y);
      void OnMouseRelease(int32_t x, int32_t y, uint32_t button);
      AWidget* FindWidgetAt(int32_t x, int32_t y) const;
      void OnKeyEvent(const AUIKeyEvent& event);
      void ClearHover();
      AWidget* GetDragWidget() const {
        return mDragWidget;
      }
      void SetDragWidget(AWidget *widget);
      void OnMouseWheel(int32_t delta);
      void ForceDraw();                    // draws immediately (for resize, input, etc.)
      bool HasDrawPending() const { return mDrawPending; }
      void SetFocus(AWidget* widget);
      AWidget* GetFocusedWidget() const { return mFocusedWidget; }
      void ClearFocus();
      void SetKeepFocusOnMouseLeave(bool keep) { mKeepFocusOnMouseLeave = keep; }
      bool KeepFocusOnMouseLeave() const { return mKeepFocusOnMouseLeave; }
      void SetCursor(AUICursorType type);
      void BringChildToFront(AWidget *child);


  };

}// namespace aui

#endif // AWINDOW_H_
