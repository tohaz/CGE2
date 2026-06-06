// This is required to link Wayland protocol implementations. DO NOT MOVE IT
extern "C" {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wattributes"
#include "Custom/obj/xdg-shell-client-protocol.h"
#include "Custom/obj/xdg-decoration-unstable-v1-client-protocol.h"
#include "Custom/obj/fractional-scale-v1-client-protocol.h"
#include "Custom/obj/xdg-shell-protocol.c"
#include "Custom/obj/xdg-decoration-protocol.c"
#pragma GCC diagnostic pop
}

#include "AUILib.h"
#include "WaylandWindowContext.h"

namespace aui {
//  static void buffer_release(void *data, UNUSED wl_buffer *buffer) {
//    bool *busy = static_cast<bool*>(data);
//    *busy = false;
//    D2("Buffer released, now free");
//  }
  static void buffer_release(void *data, UNUSED wl_buffer *buffer) {
    auto *buf = static_cast<WaylandBuffer*>(data);
    buf->busy = false;
    D2("Buffer released, now free");
    if(buf->pendingDeletion) {
      if(buf->buffer) {
        wl_buffer_destroy(buf->buffer);
        buf->buffer = nullptr;
      }
      if(buf->data) {
        munmap(buf->data, buf->size);
        buf->data = nullptr;
      }
      buf->size = 0;
      buf->pendingDeletion = false;
      D2("Pending buffer destroyed on release");
    }
  }

  static const wl_buffer_listener buffer_listener = { .release = buffer_release };

  static void frame_handle_done(void *data, UNUSED wl_callback *cb, UNUSED uint32_t time) {
    auto *ctx = static_cast<WaylandWindowContext*>(data);
    ctx->mFramePending = false;
    wl_callback_destroy(cb);
    ctx->mFrameCallback = nullptr;
  }

  static const wl_callback_listener frame_listener = { .done = frame_handle_done };

  static void xdg_toplevel_close(void *data, UNUSED xdg_toplevel *top) {
    D2("xdg_toplevel_close called");
    auto *ctx = static_cast<WaylandWindowContext*>(data);
    if(ctx && ctx->Wnd()) {
      D2("Calling Close on window");
      ctx->Wnd()->Close();
    }
    else {
      DT("No window to close");
    }
  }

  WaylandWindowContext::WaylandWindowContext(AUI *aui, AWindow *window) {
    mAUI = aui;
    mWindow = window;
  }

  bool WaylandWindowContext::CreateFrame(UNUSED uint32_t width, UNUSED uint32_t height, const std::string &title) {
    if(!mAUI || !mWindow)
      return false;
    wl_display *display = mAUI->GetWaylandDisplay();
    wl_compositor *compositor = mAUI->GetWaylandCompositor();
    wl_shm *shm = mAUI->GetWaylandShm();
    xdg_wm_base *xdg_base = mAUI->GetWaylandXdgBase();
    if(!display || !compositor || !shm || !xdg_base)
      return false;
    mSurface = wl_compositor_create_surface(compositor);
    mXdgSurface = xdg_wm_base_get_xdg_surface(xdg_base, mSurface);
    static const struct xdg_surface_listener surf_listener = { .configure = [](void *data, xdg_surface *surf,
        uint32_t serial) {
      auto *ctx = static_cast<WaylandWindowContext*>(data);
      if(!ctx || !ctx->Wnd())
        return;
      xdg_surface_ack_configure(surf, serial);
    } };
    xdg_surface_add_listener(mXdgSurface, &surf_listener, this);
    mToplevel = xdg_surface_get_toplevel(mXdgSurface);
    static const struct xdg_toplevel_listener toplevel_listener = { .configure = [](void *data,
    UNUSED xdg_toplevel *top, int32_t width2, int32_t height2, UNUSED wl_array *states) {
      auto *ctx = static_cast<WaylandWindowContext*>(data);
      D2("xdg_toplevel_configure: new size {}x{}, resizeEnabled={}", width2, height2,
          ctx->Wnd() ? ctx->Wnd()->IsResizeEnabled() : false);
      if(!ctx || !ctx->Wnd())
        return;
// Enable frame sync after the first real configure (non‑zero size)
      if(width2 > 0 && height2 > 0 && !ctx->mFrameSyncEnabled) {
        ctx->mFrameSyncEnabled = true;
        D2("Frame sync enabled after configure");
      }
      if(width2 > 0 && height2 > 0 && ctx->Wnd()->IsResizeEnabled()) {
        ctx->Wnd()->Resize(static_cast<uint32_t>(width2), static_cast<uint32_t>(height2));
      }
    }, .close = xdg_toplevel_close, .configure_bounds = nullptr, .wm_capabilities = nullptr };
    xdg_toplevel_add_listener(mToplevel, &toplevel_listener, this);
    xdg_toplevel_set_title(mToplevel, title.c_str());
    if(mAUI->GetWaylandDecorationManager()) {
      mDecoration = zxdg_decoration_manager_v1_get_toplevel_decoration(mAUI->GetWaylandDecorationManager(), mToplevel);
      zxdg_toplevel_decoration_v1_set_mode(mDecoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
      D2("CreateFrame: Server-side decorations requested successfully before initial commit.");
    }
    else {
      D1("CreateFrame: No decoration manager available.");
    }
    mPendingWidth = width;
    mPendingHeight = height;
    mPendingResizeEnabled = false;
// Disable frame sync until we receive the first configure event
    mFrameSyncEnabled = false;
    mFramePending = false;
    mFrameCallback = nullptr;
    wl_surface_commit(mSurface);
    wl_display_flush(display);
    return true;
  }

  void WaylandWindowContext::DestroyFrame() {
    if(!mSurface)
      return;
    D2("DestroyFrame");
    if(mFrameCallback) {
      wl_callback_destroy(mFrameCallback);
      mFrameCallback = nullptr;
      mFramePending = false;
    }
    if(mDecoration) {
      zxdg_toplevel_decoration_v1_destroy(mDecoration);
      mDecoration = nullptr;
    }
    if(mToplevel) {
      xdg_toplevel_destroy(mToplevel);
      mToplevel = nullptr;
    }
    if(mXdgSurface) {
      xdg_surface_destroy(mXdgSurface);
      mXdgSurface = nullptr;
    }
    if(mSurface) {
      wl_surface_destroy(mSurface);
      mSurface = nullptr;
    }
    wl_display_flush(mAUI->GetWaylandDisplay());
    DestroyShmBuffer();
  }

  void WaylandWindowContext::CreateShmBuffer(uint32_t width, uint32_t height, WaylandBuffer *targetBuffers) {
    if(!targetBuffers)
      targetBuffers = mBuffers;
    D2("CreateShmBuffer: {}x{} (Double Buffered)", width, height);
    size_t stride = width * 4;
    size_t size = stride * height;
    for (int32_t i = 0; i < 2; ++i) {
      int32_t fd = memfd_create("aui-wl-shm", MFD_CLOEXEC);
      if(fd < 0) {
        D1("memfd_create failed");
        return;
      }
      if(ftruncate(fd, static_cast<off_t>(size)) < 0) {
        close(fd);
        D1("ftruncate failed");
        return;
      }
      void *data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
      if(data == MAP_FAILED) {
        close(fd);
        D1("mmap failed");
        return;
      }
      wl_shm_pool *pool = wl_shm_create_pool(mAUI->GetWaylandShm(), fd, static_cast<int32_t>(size));
      targetBuffers[i].buffer = wl_shm_pool_create_buffer(pool, 0, static_cast<int32_t>(width),
          static_cast<int32_t>(height), static_cast<int32_t>(stride), WL_SHM_FORMAT_XRGB8888);
      wl_shm_pool_destroy(pool);
      close(fd);
      targetBuffers[i].data = static_cast<uint32_t*>(data);
      targetBuffers[i].size = size;
      targetBuffers[i].busy = false;
      targetBuffers[i].pendingDeletion = false;
      std::fill(targetBuffers[i].data, targetBuffers[i].data + width * height, 0xFFAAAAAA);
      wl_buffer_add_listener(targetBuffers[i].buffer, &buffer_listener, &targetBuffers[i]);
    }
  }

  void WaylandWindowContext::DestroyShmBuffer() {
    DestroyShmBuffer(mBuffers, true);
  }

  void WaylandWindowContext::Move(UNUSED int32_t x, UNUSED int32_t y) {
    E()
// Stub
  }

  void WaylandWindowContext::EnableResize() {
    mPendingResizeEnabled = true;
  }

  void WaylandWindowContext::Resize(uint32_t width, uint32_t height) {
    if(width == 0 || height == 0 || mInResize)
      return;
    mInResize = true;
// Flush any pending draws using the old buffers
    if(mAUI)
      mAUI->Draw();
// Store current buffers as "old" and clear mBuffers
    for (int32_t i = 0; i < 2; ++i) {
      mOldBuffers[i] = mBuffers[i];
      mBuffers[i] = WaylandBuffer { };// zero out
    }
    mHasOldBuffers = true;
// Remove stale draw commands for this window
    uint64_t nativeId = reinterpret_cast<uint64_t>(mSurface);
    if(mAUI)
      mAUI->ClearDrawCommandsForWindow(nativeId);
    mPendingWidth = width;
    mPendingHeight = height;
// Create new buffers with the new size
    CreateShmBuffer(width, height, mBuffers);
// Try to destroy old buffers – those that are not busy will be freed immediately;
// busy ones are marked for deletion and will be cleaned up when released.
    DestroyShmBuffer(mOldBuffers, false);
    mInResize = false;
  }

  void WaylandWindowContext::SetTitle(const std::string &title) {
    if(mToplevel)
      xdg_toplevel_set_title(mToplevel, title.c_str());
  }

  uint64_t WaylandWindowContext::GetNativeWindowId() const {
    return reinterpret_cast<uint64_t>(mSurface);
  }

  uint32_t* WaylandWindowContext::GetSoftwareBuffer() {
    int32_t backIdx = mCurrentBufferIndex ^ 1;
    return mBuffers[backIdx].data;
  }

  void WaylandWindowContext::QueueFrameCommit() {
    int32_t backBufferIdx = mCurrentBufferIndex ^ 1;
    D2("QueueFrameCommit: surface={}, buffer_index={}, w={}, h={}", (void*)mSurface, backBufferIdx, mWindow->SizeX(),
        mWindow->SizeY());
    if(!mSurface || !mBuffers[backBufferIdx].buffer) {
      E("Missing surface or buffer");
      return;
    }
// Skip if the back buffer is still busy
    if(mBuffers[backBufferIdx].busy) {
      D2("Back buffer busy, skipping commit");
      return;
    }
// If frame sync is enabled, wait for pending frame callback
    if(mFrameSyncEnabled && mFramePending) {
      D2("Frame callback pending, skipping commit");
      return;
    }
    DrawCommand cmd;
    cmd.type = DrawCommandType::Wayland;
    cmd.wayland.surface = mSurface;
    cmd.wayland.buffer = mBuffers[backBufferIdx].buffer;
    cmd.wayland.width = mWindow->SizeX();
    cmd.wayland.height = mWindow->SizeY();
    D2("QueueFrameCommit: Enqueueing command. Current mDrawCommands size BEFORE push = {}", mAUI->GetDrawCommandsSize());
    mAUI->EnqueueDrawCommand(cmd);
    mBuffers[backBufferIdx].busy = true;
    mCurrentBufferIndex = backBufferIdx;
// Request a frame callback only if frame sync is enabled
    if(mFrameSyncEnabled && !mFramePending) {
      mFrameCallback = wl_surface_frame(mSurface);
      wl_callback_add_listener(mFrameCallback, &frame_listener, this);
      mFramePending = true;
      D2("Frame callback requested");
    }
  }

  void WaylandWindowContext::DisableResize() {
    if(!mToplevel || !mWindow)
      return;
    int32_t w = static_cast<int32_t>(mWindow->SizeX());
    int32_t h = static_cast<int32_t>(mWindow->SizeY());
    xdg_toplevel_set_min_size(mToplevel, w, h);
    xdg_toplevel_set_max_size(mToplevel, w, h);
// Send the requests to the compositor now
    if(mSurface) {
      wl_surface_commit(mSurface);
      wl_display_flush(mAUI->GetWaylandDisplay());
    }
  }

  void WaylandWindowContext::ProcessEvent(void*) {
    D1("unmplemented")
  }

  void WaylandWindowContext::CreateShmBuffer(uint32_t width, uint32_t height) {
    CreateShmBuffer(width, height, mBuffers);
  }

  void WaylandWindowContext::DestroyShmBuffer(WaylandBuffer *buffers, bool immediate) {
    if(!buffers)
      buffers = mBuffers;
    for (int32_t i = 0; i < 2; ++i) {
      if(buffers[i].buffer) {
        if(immediate || !buffers[i].busy) {
          if(buffers[i].buffer) {
            wl_buffer_destroy(buffers[i].buffer);
            buffers[i].buffer = nullptr;
          }
          if(buffers[i].data) {
            munmap(buffers[i].data, buffers[i].size);
            buffers[i].data = nullptr;
          }
          buffers[i].size = 0;
          buffers[i].busy = false;
          buffers[i].pendingDeletion = false;
        }
        else {
          buffers[i].pendingDeletion = true;
        }
      }
    }
  }

  WaylandWindowContext::~WaylandWindowContext() {
    if(mHasOldBuffers) {
      DestroyShmBuffer(mOldBuffers, true);// force immediate destruction
      mHasOldBuffers = false;
    }
    DestroyFrame();
  }
}// namespace aui
