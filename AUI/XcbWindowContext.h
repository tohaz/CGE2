#ifndef X11LANDWINDOWCONTEXT_H_ // :)
#define X11LANDWINDOWCONTEXT_H_

namespace aui {

  struct XCBBuffer: public IBuffer {
      xcb_pixmap_t pixmap = 0;
      std::unique_ptr<uint32_t[]> data;// client‑side pixel buffer
      size_t size = 0;// size in bytes
      void Release() override {
        E("Nothing to do here")
      }
  };

  // Motif WM hints for decoration control
  struct MotifWmHints {
      uint32_t flags;        // instead of unsigned long
      uint32_t functions;
      uint32_t decorations;
      int32_t  inputMode;    // instead of long
      uint32_t status;
  };

  enum {
      MWM_HINTS_FUNCTIONS   = (1L << 0),
      MWM_HINTS_DECORATIONS = (1L << 1),
  };

  enum {
      MWM_DECOR_ALL      = (1L << 0),
      MWM_DECOR_BORDER   = (1L << 1),
      MWM_DECOR_RESIZEH  = (1L << 2),
      MWM_DECOR_TITLE    = (1L << 3),
      MWM_DECOR_MENU     = (1L << 4),
      MWM_DECOR_MINIMIZE = (1L << 5),
      MWM_DECOR_MAXIMIZE = (1L << 6),
  };

  class XCBWindowContext: public AWindow {
    private:
      uint32_t mWindowId = 0;
      uint32_t mGC = 0;
      xcb_atom_t mWmDeleteWindowAtom = 0;
      xcb_atom_t mWmProtocolsAtom = 0;
      xcb_key_symbols_t* mKeySymbols = nullptr;
      struct xkb_keymap* mXkbKeymap = nullptr;
      struct xkb_state* mXkbState = nullptr;
      struct xkb_context* mXkbCtx = nullptr;
      xcb_sync_counter_t mSyncCounter = 0;
      uint64_t mPendingSyncSerial = 0;
      bool mSyncPending = false;
      xcb_atom_t mSyncRequestAtom = 0;
      xcb_cursor_context_t* mCursorContext = nullptr;
      xcb_cursor_t mCurrentCursor = 0;
      void CreateBuffers();
      void UpdateDecorations();
      void PrepareSync();
      virtual void BackendResize(uint32_t x, uint32_t y) override;
      virtual void BackendTitle(UNUSED std::string title) override;
      virtual void BackendDisableResize() override;
      virtual void BackendEnableResize() override;
      virtual void BackendCursor(AUICursorType type) override;
      virtual void BackendMove(int32_t x, int32_t y) override;
      xcb_atom_t mWmNormalHintsAtom = XCB_ATOM_NONE;
      void UpdateSizeHints();
    public:
      void ProcessEvent(xcb_generic_event_t *ev);
      XCBWindowContext(AUI *au);
      ~XCBWindowContext();
      void Draw() override;
      bool CreateFrame() override;
      uint64_t NativeWindowId() const override;

  };

}
#endif // X11LANDWINDOWCONTEXT_H_
