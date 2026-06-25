#ifndef XCBWINDOWCONTEXT_H_
#define XCBWINDOWCONTEXT_H_

namespace aui {

  class AUI;
  class AWindow;

  class XcbWindowContext: public IWindowContext {
    private:
      uint32_t mWindowId = 0;
      std::shared_ptr<std::vector<uint32_t>> mSoftwareBuffer;
      xcb_gcontext_t mGC = 0;
      xcb_atom_t mWmDeleteWindowAtom = 0;
      xcb_atom_t mWmProtocolsAtom = 0;
      void ApplySizeHints(uint32_t min_w, uint32_t min_h, uint32_t max_w, uint32_t max_h);
      uint32_t mSizeX = 0;
      uint32_t mSizeY = 0;
      xcb_key_symbols_t* mKeySymbols = nullptr;
      struct xkb_context* mXkbCtx = nullptr;
      struct xkb_keymap* mXkbKeymap = nullptr;
      struct xkb_state* mXkbState = nullptr;
      xcb_cursor_context_t* mCursorContext = nullptr;
      xcb_cursor_t mCurrentCursor = 0;
      bool mMapped = false;
    public:
      XcbWindowContext(AUI *aui, AWindow *window);
      ~XcbWindowContext() override;
      bool CreateFrame(uint32_t width, uint32_t height, const std::string &title) override;
      void DestroyFrame() override;
      void Move(int32_t x, int32_t y) override;
      void Resize(uint32_t width, uint32_t height) override;
      void SetTitle(const std::string &title) override;
      uint32_t* GetSoftwareBuffer() override;
      std::shared_ptr<std::vector<uint32_t>> GetSoftwareBufferPtr() {return mSoftwareBuffer;}
      void QueueFrameCommit() override;
      void EnableResize() override;
      void DisableResize() override;
      void ProcessEvent(void *ev) override;
      uint32_t SizeX() const {return mSizeY;}
      uint32_t SizeY() const {return mSizeX;}
      uint64_t GetNativeWindowId() const {return mWindowId;}
      virtual void SetCursor(AUICursorType type) override;
      bool EnsureBuffer(uint32_t width, uint32_t height) override;
      bool IsMapped() {return mMapped;}
  };

}// namespace aui

#endif // XCBWINDOWCONTEXT_H_
