#ifndef WAYLANDWINDOWCONTEXT_H_
#define WAYLANDWINDOWCONTEXT_H_

#include "IWindowContext.h"
#include <wayland-client.h>
#include <vector>
#include <cstdint>

namespace aui {
  struct WaylandBuffer {
      wl_buffer *buffer = nullptr;
      uint32_t *data = nullptr;
      size_t size = 0;
      bool busy = false;
      bool pendingDeletion = false;
  };

  class AUI;
  class AWindow;
  class WaylandWindowContext: public IWindowContext {
      friend class AUI;
    private:
      WaylandBuffer mBuffers[2];
      WaylandBuffer mOldBuffers[2];
      bool mHasOldBuffers = false;
      int32_t mCurrentBufferIndex = 0;
      xdg_surface *mXdgSurface = nullptr;
      zxdg_toplevel_decoration_v1 *mDecoration = nullptr;
      bool mInResize = false;
      bool mPendingResizeEnabled = false;
      uint32_t mPendingWidth = 0;
      uint32_t mPendingHeight = 0;
      wl_surface *mSurface = nullptr;
      xdg_toplevel *mToplevel = nullptr;

    protected:

    public:
      WaylandWindowContext(AUI *aui, AWindow *window);
      ~WaylandWindowContext() override;
      bool CreateFrame(uint32_t width, uint32_t height, const std::string &title) override;
      void DestroyFrame() override;
      void Move(int32_t x, int32_t y) override;
      void Resize(uint32_t width, uint32_t height) override;
      void SetTitle(const std::string &title) override;
      void CreateShmBuffer(uint32_t width, uint32_t height);
      void CreateShmBuffer(uint32_t width, uint32_t height, WaylandBuffer *targetBuffers);
      uint64_t GetNativeWindowId() const override;
      uint32_t* GetSoftwareBuffer() override;
      void QueueFrameCommit() override;
      void DestroyShmBuffer();
      void DestroyShmBuffer(WaylandBuffer *buffers, bool immediate);
      void ProcessEvent(void *ev) override;
      void EnableResize() override;
      void DisableResize() override;
      wl_callback *mFrameCallback = nullptr;
      bool mFramePending = false;
      bool mFrameSyncEnabled = false;

  };

}// namespace aui

#endif // WAYLANDWINDOWCONTEXT_H_
