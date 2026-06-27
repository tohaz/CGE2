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

namespace aui {
  static void buffer_release(void *data, UNUSED wl_buffer *buffer) {
    auto *buf = static_cast<WaylandBuffer*>(data);
    buf->busy = false;
    D3("Buffer released, now free");
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
    D1("xdg_toplevel_close called");
    auto *ctx = static_cast<WaylandWindowContext*>(data);
    if(ctx && ctx->Wnd()) {
      D2("Calling Close on window");
      if(ctx->Wnd() == ctx->GetEnginePtr()->MainWnd()) {
        ctx->GetEnginePtr()->ExitAUI();
      }
      else {
        ctx->Wnd()->Close();
      }
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
      if (!ctx->mFrameSyncEnabled) {
        ctx->mFrameSyncEnabled = true;
// Trigger a draw – now the guard in QueueFrameCommit will pass
        ctx->Wnd()->Draw();
      }
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
    mBufferWidth = width;
    mBufferHeight = height;
    D2("CreateShmBuffer: {}x{} (Double Buffered)", width, height);
    size_t stride = width * 4;
    size_t size = stride * height;
    for(int32_t i = 0; i < 2; ++i) {
// 1. Create anonymous file descriptor in memory
      int32_t fd = memfd_create("aui-wl-shm", MFD_CLOEXEC);
      if(fd < 0) {
        E("Wayland Shm Allocation Fatal: memfd_create failed. System may be out of file descriptors.");
      }
// 2. Allocate the exact sizing layout
      if(ftruncate(fd, static_cast<off_t>(size)) < 0) {
        close(fd);
        E("Wayland Shm Allocation Fatal: ftruncate failed to allocate {} bytes. Out of memory/shm space.", size);
      }
// 3. Map memory pages to the process space
      void *data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
      if(data == MAP_FAILED) {
        close(fd);
        E("Wayland Shm Allocation Fatal: mmap failed to expose shared memory buffer to UI process.");
      }
// 4. Register backing pool tracking structures with Wayland
      wl_shm_pool *pool = wl_shm_create_pool(mAUI->GetWaylandShm(), fd, static_cast<int32_t>(size));
      if(!pool) {
        munmap(data, size);
        close(fd);
        E("Wayland Protocol Error: wl_shm_create_pool failed. Wayland connection may be corrupted.");
      }
      targetBuffers[i].buffer = wl_shm_pool_create_buffer(pool, 0, static_cast<int32_t>(width),
          static_cast<int32_t>(height), static_cast<int32_t>(stride), WL_SHM_FORMAT_XRGB8888);
      wl_shm_pool_destroy(pool);
      close(fd);// Safe to close the descriptor now; pool keeps a duplicate internally
      if(!targetBuffers[i].buffer) {
        munmap(data, size);
        E("Wayland Protocol Error: wl_shm_pool_create_buffer failed to generate client pixel buffer handle.");
      }
// Populate state attributes safely
      targetBuffers[i].data = static_cast<uint32_t*>(data);
      targetBuffers[i].size = size;
      targetBuffers[i].busy = false;
      targetBuffers[i].pendingDeletion = false;
// Zero/clear out raw background (Pre-fill layout texture)
      std::fill(targetBuffers[i].data, targetBuffers[i].data + width * height, 0xFFAAAAAA);
// Establish hardware release sync boundaries
      wl_buffer_add_listener(targetBuffers[i].buffer, &buffer_listener, &targetBuffers[i]);
    }
  }

  void WaylandWindowContext::DestroyShmBuffer() {
    DestroyShmBuffer(mBuffers, true);
  }

  void WaylandWindowContext::Move(UNUSED int32_t x, UNUSED int32_t y) {
    E("wayland windows are not supposed to be moved programmatically. compositor restricts it")
  }

  void WaylandWindowContext::EnableResize() {
    mPendingResizeEnabled = true;
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
    if(mFramePending) {
      D3("Frame callback still pending, skipping commit");
      return;
    }
    if(!mFrameSyncEnabled) {
      D1("Frame sync not enabled (no configure yet), skipping commit");
      return;
    }
    int32_t backBufferIdx = mCurrentBufferIndex ^ 1;
    D3("QueueFrameCommit: surface={}, buffer_index={}, w={}, h={}", (void*)mSurface, backBufferIdx, mWindow->SizeX(),
        mWindow->SizeY());
    if(!mSurface || !mBuffers[backBufferIdx].buffer) {
      E("Missing surface {} or buffer {}", (void*)mSurface, (void*) mBuffers[backBufferIdx].buffer);
    }
// Skip if the back buffer is still busy (being held by the compositor)
    if(mBuffers[backBufferIdx].busy) {
      D3("Back buffer busy, skipping commit");
      return;
    }
// Enqueue the draw command (this will be processed by AUI::Draw)
    DrawCommand cmd;
    cmd.type = DrawCommandType::Wayland;
    cmd.wayland.surface = mSurface;
    cmd.wayland.buffer = mBuffers[backBufferIdx].buffer;
    cmd.wayland.width = mWindow->SizeX();
    cmd.wayland.height = mWindow->SizeY();
    D3("QueueFrameCommit: Enqueueing command. Current mDrawCommands size BEFORE push = {}", mAUI->DrawCommands());
    mAUI->EnqueueDrawCommand(cmd);
#ifdef AUI_UNIT_TEST
    ++mEnqueueCount;
#endif
// Mark buffer as busy (will be released by the buffer_release listener)
    mBuffers[backBufferIdx].busy = true;
    mCurrentBufferIndex = backBufferIdx;
    if(mFrameSyncEnabled) {
      mFrameCallback = wl_surface_frame(mSurface);
      wl_callback_add_listener(mFrameCallback, &frame_listener, this);
      mFramePending = true;
      D3("Frame callback requested");
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
    for(int32_t i = 0; i < 2; ++i) {
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

  void WaylandWindowContext::Resize(uint32_t width, uint32_t height) {
    if(width == 0 || height == 0 || mInResize)
      return;
    mInResize = true;
    if(mWindow && !mWindow->IsResizeEnabled()) {
      xdg_toplevel_set_min_size(mToplevel, SafeINT32(width), SafeINT32(height));
      xdg_toplevel_set_max_size(mToplevel, SafeINT32(width), SafeINT32(height));
      wl_surface_commit(mSurface);
      wl_display_flush(mAUI->GetWaylandDisplay());
    }
// Clear stale draw commands
    uint64_t nativeId = reinterpret_cast<uint64_t>(mSurface);
    if(mAUI)
      mAUI->ClearDrawCommandsForWindow(nativeId);
    mPendingWidth = width;
    mPendingHeight = height;
    if (mWindow) {
        mWindow->Draw();   // instantly redraw at the new size
    }
// Do NOT create/destroy buffers here; DoDraw will call EnsureBuffer.
    mInResize = false;
  }

  void WaylandWindowContext::SetCursor(AUICursorType type) {
    if(!mCursorTheme) {
      wl_shm *shm = mAUI->GetWaylandShm();
      if(!shm) {
        D1("Wayland: No SHM");
        return;
      }
      mCursorTheme = wl_cursor_theme_load(nullptr, 24, shm);
      if(!mCursorTheme) {
        D1("Wayland: Failed to load cursor theme");
        return;
      }
    }
    if(!mCursorSurface) {
      wl_compositor *compositor = mAUI->GetWaylandCompositor();
      if(!compositor) {
        D1("Wayland: No compositor");
        return;
      }
      mCursorSurface = wl_compositor_create_surface(compositor);
      if(!mCursorSurface) {
        D1("Wayland: Failed to create cursor surface");
        return;
      }
    }
    const char *name = "left_ptr";
    switch (type) {
      case AUICursorType::HResize:
        name = "ew-resize";
        break;
      case AUICursorType::VResize:
        name = "ns-resize";
        break;
      default:
        name = "left_ptr";
    }
    wl_cursor *cursor = wl_cursor_theme_get_cursor(mCursorTheme, name);
    if(!cursor || cursor->image_count == 0) {
      D1("Wayland: Failed to load cursor '{}'", name);
      return;
    }
    wl_pointer *pointer = mAUI->GetWaylandPointer();
    if(!pointer) {
      D1("Wayland: No pointer");
      return;
    }
    struct wl_cursor_image *image = cursor->images[0];
    struct wl_buffer *buffer = wl_cursor_image_get_buffer(image);
    if(!buffer) {
      D1("Wayland: Failed to get buffer for cursor image");
      return;
    }
    wl_surface_attach(mCursorSurface, buffer, 0, 0);
    wl_surface_damage(mCursorSurface, 0, 0, static_cast<int32_t>(image->width), static_cast<int32_t>(image->height));
    wl_surface_commit(mCursorSurface);
    uint32_t serial = mAUI->GetLastPointerSerial();
    wl_pointer_set_cursor(pointer, serial, mCursorSurface, static_cast<int32_t>(image->hotspot_x),
        static_cast<int32_t>(image->hotspot_y));
    mWindow->Draw();
  }

  bool WaylandWindowContext::EnsureBuffer(uint32_t width, uint32_t height) {
    if(mBufferWidth == width && mBufferHeight == height && mBuffers[0].buffer && mBuffers[1].buffer) {
        return true;
      }

      // Guard against overwriting an inflight old buffer setup
      for(uint32_t i = 0; i < 2; ++i) {
        if (mOldBuffers[i].buffer && mOldBuffers[i].busy) {
          // Force immediate cleanup or mark it explicitly so it isn't orphaned
          wl_buffer_destroy(mOldBuffers[i].buffer);
          if(mOldBuffers[i].data) munmap(mOldBuffers[i].data, mOldBuffers[i].size);
          mOldBuffers[i] = WaylandBuffer{};
        }
      }

      // Safe to swap now...
      for(uint32_t i = 0; i < 2; ++i) {
        mOldBuffers[i] = mBuffers[i];
        mBuffers[i] = WaylandBuffer { };
      }
    mHasOldBuffers = true;
// Create new buffers
    CreateShmBuffer(width, height, mBuffers);
// Destroy old buffers asynchronously (they'll be freed when released)
    DestroyShmBuffer(mOldBuffers, false);
    mBufferWidth = width;
    mBufferHeight = height;
    return true;
  }

  WaylandWindowContext::~WaylandWindowContext() {
    if(mCursorSurface) {
      wl_surface_destroy(mCursorSurface);
      mCursorSurface = nullptr;
    }
    if(mCursorTheme) {
      wl_cursor_theme_destroy(mCursorTheme);
      mCursorTheme = nullptr;
    }
// Destroy active buffers immediately (window is closing)
    DestroyShmBuffer(mBuffers, true);
// Destroy any old buffers that may still be pending
    DestroyShmBuffer(mOldBuffers, true);
    DestroyFrame();
  }

}// namespace aui
