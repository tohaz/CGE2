#ifndef WAYLANDWINDOWCONTEXT_H_
#define WAYLANDWINDOWCONTEXT_H_

namespace aui {

  wl_buffer* CreateBufferFromPNGData(wl_shm* shm, const uint8_t* data, size_t size,
                                            int32_t* out_w, int32_t* out_h);

  struct WaylandBuffer: public IBuffer {
      struct wl_buffer* wlBuffer = nullptr;
      void* shmData = nullptr;
      size_t size = 0;
      bool isOrphaned = false;
      virtual ~WaylandBuffer() override = default;
      void Release() override {
        if(wlBuffer)
          wl_buffer_destroy(wlBuffer);
      }
  };

  struct CursorData {
      wl_buffer* buffer = nullptr;
      int32_t width = 0;
      int32_t height = 0;
      int32_t hotspot_x = 0;
      int32_t hotspot_y = 0;
      bool embeddedAttempted = false;
  };

  class WaylandWindowContext: public AWindow {
      friend class AWindow;
    private:
      wl_surface* mSurface = nullptr;
      wl_surface*     mCursorSurface = nullptr;
      struct xdg_surface* mXDG_Surface = nullptr;
      struct xdg_toplevel* mXDG_Toplevel = nullptr;
      void CreateBuffers();
      zxdg_toplevel_decoration_v1* mDecoration = nullptr;
      bool mConfigured = false;
      bool mIsRecreatingBuffers = false;
      void UpdateResizeHints();
      std::thread mCursorLoaderThread;
      std::atomic<wl_cursor_theme*> mCursorTheme{nullptr};
      AUICursorType mCurrentCursorType{AUICursorType::Default};
      bool mCursorNeedsApply{false};
//      struct wl_buffer* mCursorBuffer = nullptr;
      CursorData mCursorData[3];

    protected:
      WaylandWindowContext(AUI *au);
      void UpdateDecorations() override;
      void RecreateBuffers();
      virtual void BackendResize(uint32_t x, uint32_t y) override;

    public:
      std::vector<std::unique_ptr<WaylandBuffer>> mOrphanedBuffers;
      struct wl_callback* mFrameCallback = nullptr;
      bool mFrameCallbackPending = false;
      int32_t mPendingSizeX = 0;
      int32_t mPendingSizeY = 0;
      void ApplyNewSize(int32_t width, int32_t height);
      virtual bool CreateFrame() override;
      virtual void Draw() override;
      wl_surface* Surface() {return mSurface;}
      uint64_t NativeWindowId() const override;
      virtual void BackendMove(int32_t x, int32_t y) override;
      virtual void BackendTitle(UNUSED std::string title) override;
      virtual void BackendDisableResize() override;
      virtual void BackendEnableResize() override;
      virtual void BackendShow() override;
      virtual void BackendHide() override;
      virtual void BackendCursor(AUICursorType type) override;
      WaylandBuffer* Buffers();
      void Configured();
      bool IsConfigured();
      AUICursorType CurrentCursorType() {return mCurrentCursorType;}
      bool CursorNeedsApply();
      ~WaylandWindowContext();
  };
}

#endif
