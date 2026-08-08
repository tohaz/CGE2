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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wswitch-default"
#define STB_IMAGE_IMPLEMENTATION
#include "fonts/stb_image.h"
#pragma GCC diagnostic pop

#include "AUILib.h"

namespace aui {

  UNUSED static int32_t CreateWaylandShmFile(UNUSED off_t size) {
    int32_t fd = memfd_create("aui-wayland-shm", MFD_CLOEXEC);
    if(fd < 0)
      E("memfd_create failed")
    if(ftruncate(fd, size) < 0)
      E("ftruncate failed")
    return fd;
  }

  static void buffer_handle_release(void* data, struct wl_buffer* wl_buffer) {
    if(!data)
      return;
    auto* ctx = static_cast<WaylandWindowContext*>(data);
// Find the buffer entry matching this wl_buffer handle inside our tracking lists
    WaylandBuffer* buf = nullptr;
    bool isOrphan = false;
    size_t orphanIdx = 0;
// Check orphans first
    for(size_t i = 0; i < ctx->mOrphanedBuffers.size(); ++i) {
      if(ctx->mOrphanedBuffers[i] && ctx->mOrphanedBuffers[i]->wlBuffer == wl_buffer) {
        buf = ctx->mOrphanedBuffers[i].get();
        isOrphan = true;
        orphanIdx = i;
        break;
      }
    }
// If not found in orphans, it's an active buffer in the base class array
    if(!buf) {
      for(size_t i = 0; i < AUI_NUM_BUFFERS; ++i) {
        if(ctx->mBuffers[i] && static_cast<WaylandBuffer*>(ctx->mBuffers[i].get())->wlBuffer == wl_buffer) {
          buf = static_cast<WaylandBuffer*>(ctx->mBuffers[i].get());
          break;
        }
      }
    }
    if(!buf)
      return;
    buf->isBusy = false;
    if(isOrphan) {
// 1. Destroy Wayland handles immediately
      if(buf->wlBuffer) {
        wl_buffer_destroy(buf->wlBuffer);
        buf->wlBuffer = nullptr;
      }
      if(buf->shmData && buf->size > 0) {
        munmap(buf->shmData, buf->size);
        buf->shmData = nullptr;
        buf->size = 0;
      }
// 2. Erase it from the vector. This invokes std::unique_ptr's destructor
// and cleanly deletes the memory, removing it from the destructor's radar!
      ctx->mOrphanedBuffers.erase(ctx->mOrphanedBuffers.begin() + SafeINT64(orphanIdx));
    }
  }

  static void xdg_surface_handle_configure(void* data, struct xdg_surface* xdg_surf, uint32_t serial) {
    D2("serial {}", serial);
    auto* ctx = static_cast<WaylandWindowContext*>(data);
    if(!ctx)
      return;
// 1. Always acknowledge the compositor's serial first
    xdg_surface_ack_configure(xdg_surf, serial);
// 2. Mark configured state
    if(!ctx->IsConfigured()) {
      ctx->Configured();
    }
// 3. ONLY draw if the window is currently set to be visible
    if(ctx->Visible()) {
      ctx->Draw();
    }
  }

  static void xdg_toplevel_handle_configure(UNUSED void* data, UNUSED struct xdg_toplevel* toplevel,
  UNUSED int32_t szx, UNUSED int32_t szy, UNUSED struct wl_array* states) {
    D2("{}x{}", szx, szy)
    if(szx == 0 || szy == 0)
      return;
    auto* ctx = static_cast<WaylandWindowContext*>(data);
// Store the latest size, overwriting any previous pending
    ctx->mPendingSizeX = szx;
    ctx->mPendingSizeY = szy;
  }

  static void xdg_toplevel_handle_close(UNUSED void* data, UNUSED struct xdg_toplevel* toplevel) {
    UNUSED auto* w = static_cast<WaylandWindowContext*>(data);
    UNUSED AUI* au = w->EnginePtr();
    if(au->MainWnd() == (AWindow*) w) {
      D2("closing main window, bye")
      au->ExitAUI();
    }
    else {
      D1("closing secondary window")
      w->Close();
    }
  }

  static const struct xdg_toplevel_listener xdg_toplevel_listener = { .configure = xdg_toplevel_handle_configure,
      .close = xdg_toplevel_handle_close,
#ifdef XDG_TOPLEVEL_CONFIGURE_BOUNDS_SINCE_VERSION
      .configure_bounds = [](void*, struct xdg_toplevel*, int32_t, int32_t) {
      },
#endif
#ifdef XDG_TOPLEVEL_WM_CAPABILITIES_SINCE_VERSION
      .wm_capabilities = [](void*, struct xdg_toplevel*, struct wl_array*) {
      },
#endif
      };

  static void frame_callback_done(void* data, struct wl_callback* cb, uint32_t) {
    auto* ctx = static_cast<WaylandWindowContext*>(data);
    wl_callback_destroy(cb);
    ctx->mFrameCallback = nullptr;// ← CRITICAL: prevent double destroy
    ctx->mFrameCallbackPending = false;
// Apply pending size if any
    if(ctx->mPendingSizeX > 0 && ctx->mPendingSizeY > 0) {
      int32_t curX = SafeINT32(ctx->SizeX());
      int32_t curY = SafeINT32(ctx->SizeY());
      if(ctx->mPendingSizeX != curX || ctx->mPendingSizeY != curY) {
        ctx->ApplyNewSize(ctx->mPendingSizeX, ctx->mPendingSizeY);
      }
      ctx->mPendingSizeX = ctx->mPendingSizeY = 0;
    }
  }

  static const struct wl_buffer_listener buffer_listener = { .release = buffer_handle_release, };
  static const struct wl_callback_listener frame_callback_listener = { .done = frame_callback_done };
  static const struct xdg_surface_listener xdg_surface_listener = { .configure = xdg_surface_handle_configure, };

  WaylandWindowContext::WaylandWindowContext(UNUSED AUI* au) {
    D2("DEBUG: Context Created at {:p}", (void*)this);
    EnginePtr(au);
    UNUSED wl_compositor* compositor = au->WaylandCompositor();
    mSurface = wl_compositor_create_surface(compositor);
    CreateFrame();
  }

  void WaylandWindowContext::CreateBuffers() {
    D2("buffer creation")
    AUI* au = EnginePtr();
    UNUSED int32_t iszx = SafeINT32(SizeX());
    UNUSED int32_t iszy = SafeINT32(SizeY());
    int32_t size = iszx * iszy * 4;
    int32_t fd = -1;
    UNUSED int32_t stride = iszx * 4;
    wl_shm* shm = au->WaylandShm();
    struct wl_shm_pool* pool = nullptr;
    void* mmem = nullptr;
    for(size_t i = 0; i < AUI_NUM_BUFFERS; i++) {
      fd = CreateWaylandShmFile(size);
      if(fd < 0) {
        E("Failed to create shm file");
        continue;
      }
      mmem = mmap(nullptr, static_cast<size_t>(size), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
      if(mmem == MAP_FAILED) {
        E("mmap failed");
        close(fd);
        continue;
      }
      uint32_t* pixel_data = static_cast<uint32_t*>(mmem);
      std::fill(pixel_data, pixel_data + (iszx * iszy), BGColor());
      auto wBuf = std::make_unique<WaylandBuffer>();
      wBuf->shmData = mmem;
      wBuf->size = static_cast<size_t>(size);
      wBuf->isBusy = false;
      pool = wl_shm_create_pool(shm, fd, size);
      wBuf->wlBuffer = wl_shm_pool_create_buffer(pool, 0, iszx, iszy, stride, WL_SHM_FORMAT_ARGB8888);
      wl_buffer_add_listener(wBuf->wlBuffer, &buffer_listener, this);
      wl_shm_pool_destroy(pool);
      close(fd);
      this->mBuffers[i] = std::move(wBuf);
    }
    D2("ends")
  }

  bool WaylandWindowContext::CreateFrame() {
    D2("starts")
    AUI* au = EnginePtr();
    UNUSED wl_display* display = au->WaylandDisplay();
    CreateBuffers();
    UNUSED xdg_wm_base* xdg_base = au->WaylandXdgBase();
    D2("xdg_base {}", (uint64_t) xdg_base)
    mXDG_Surface = xdg_wm_base_get_xdg_surface(xdg_base, mSurface);
    if(mXDG_Surface == nullptr)
      E("xdg_wm_base_get_xdg_surface failed")
    if(xdg_surface_add_listener(mXDG_Surface, &xdg_surface_listener, this) == -1) {
      E("xdg_surface_add_listener failed")
    }
    mXDG_Toplevel = xdg_surface_get_toplevel(mXDG_Surface);
    if(!mXDG_Toplevel)
      E("xdg_surface_get_toplevel failed")
    if(xdg_toplevel_add_listener(mXDG_Toplevel, &xdg_toplevel_listener, this) != 0) {
      E("xdg_toplevel_add_listener failed")
    }
    UpdateDecorations();
    xdg_toplevel_set_min_size(mXDG_Toplevel, 1, 1);
    xdg_toplevel_set_max_size(mXDG_Toplevel, 0, 0);
    xdg_toplevel_set_title(mXDG_Toplevel, Title().c_str());
    wl_surface_commit(mSurface);
    wl_display_flush(display);
    D2("ends")
    return true;
  }

  uint64_t WaylandWindowContext::NativeWindowId() const {
    if(mSurface != nullptr)
      return reinterpret_cast<uint64_t>(mSurface);
    else
      E("null reference")
  }

  void WaylandWindowContext::Draw() {
    if(!mConfigured || mIsRecreatingBuffers) {
      D3("Not configured or recreating buffers, skipping draw");
      return;
    }
    ST2("")
    int32_t idx = FindFreeBufferIndex();
    if(idx < 0) {
      D2("No free buffer, skipping frame");
      return;
    }
    WaylandBuffer* buf = static_cast<WaylandBuffer*>(mBuffers[static_cast<size_t>(idx)].get());
    if(!buf) {
      E("Buffer is null");
      return;
    }
    buf->isBusy = true;
    int32_t iszx = SafeINT32(SizeX());
    int32_t iszy = SafeINT32(SizeY());
    uint32_t* pixel_data = nullptr;
    if(buf->shmData) {
      pixel_data = static_cast<uint32_t*>(buf->shmData);
// Fill the sequential memory range with your AWindow background color
      std::fill(pixel_data, pixel_data + (iszx * iszy), BGColor());
    }
    else {
      E("shmData pointer is unmapped or null");
      return;
    }
    if(pixel_data != nullptr) {
      AWindow::Draw((void*) pixel_data);
    }
    else {
      E("pixel data missing")
    }
    wl_surface_attach(mSurface, buf->wlBuffer, 0, 0);
    wl_surface_damage_buffer(mSurface, 0, 0, SafeINT32(SizeX()), SafeINT32(SizeY()));
    wl_surface_commit(mSurface);
    if(!mFrameCallbackPending) {
      mFrameCallback = wl_surface_frame(mSurface);
      wl_callback_add_listener(mFrameCallback, &frame_callback_listener, this);
      mFrameCallbackPending = true;
    }
    wl_display_flush(EnginePtr()->WaylandDisplay());
  }

  WaylandBuffer* WaylandWindowContext::Buffers() {
    return static_cast<WaylandBuffer*>(mBuffers[0].get());
  }

  void WaylandWindowContext::UpdateDecorations() {
    if(!mDecoration) {
      AUI* au = EnginePtr();
      auto* mgr = au->WaylandDecorationManager();
      if(mgr) {
        mDecoration = zxdg_decoration_manager_v1_get_toplevel_decoration(mgr, mXDG_Toplevel);
      }
    }
    if(mDecoration) {
      zxdg_toplevel_decoration_v1_set_mode(mDecoration,
          mDecorations ? ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE : ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);
    }
  }

  void WaylandWindowContext::Configured() {
    D3("")
    if(!mConfigured)
      mConfigured = true;
  }

  bool WaylandWindowContext::IsConfigured() {
    return mConfigured;
  }

  void WaylandWindowContext::BackendResize(uint32_t x, uint32_t y) {
    if(mXDG_Toplevel) {
// 1. Cast and safely bound check the sizes
      int32_t targetWidth = static_cast<int32_t>(x);
      int32_t targetHeight = static_cast<int32_t>(y);
// 2. Reallocate internal buffers and update internal tracking dimensions
      ApplyNewSize(targetWidth, targetHeight);
// 3. Commit the surface to notify the compositor that the surface state (and buffer size) changed
      if(mSurface) {
        wl_surface_commit(mSurface);
      }
      D2("Requested resize to {}x{}", x, y);
    }
    else {
      E("No toplevel to resize");
    }
  }

  void WaylandWindowContext::ApplyNewSize(int32_t width, int32_t height) {
    if(width <= 0 || height <= 0)
      return;
    SizeX(SafeUINT32(width));
    SizeY(SafeUINT32(height));
    if(mXDG_Surface) {
      xdg_surface_set_window_geometry(mXDG_Surface, 0, 0, width, height);
    }
    RecreateBuffers();
  }

  void WaylandWindowContext::RecreateBuffers() {
    for(auto& baseBuf : mBuffers) {
      if(baseBuf) {
        auto* buf = static_cast<aui::WaylandBuffer*>(baseBuf.get());
        if(buf->isBusy) {
          buf->isOrphaned = true;
// Release from the array and transfer unique ownership to our orphan vector
          mOrphanedBuffers.push_back(std::unique_ptr<WaylandBuffer>(static_cast<WaylandBuffer*>(baseBuf.release())));
        }
        else {
          if(buf->wlBuffer) {
            wl_buffer_destroy(buf->wlBuffer);
            buf->wlBuffer = nullptr;
          }
          if(buf->shmData && buf->size > 0) {
            munmap(buf->shmData, buf->size);
            buf->shmData = nullptr;
            buf->size = 0;
          }
          baseBuf.reset();// Safely deletes the non-busy buffer
        }
      }
    }
    CreateBuffers();
  }

  void WaylandWindowContext::BackendMove(int32_t, int32_t) {
    D2("window move not supported by Wayland")
  }

  void WaylandWindowContext::BackendTitle([[maybe_unused]] std::string title) {
    if(mXDG_Toplevel)
      xdg_toplevel_set_title(mXDG_Toplevel, title.c_str());
    else {
      E("window not initialized")
    }
  }

  void WaylandWindowContext::UpdateResizeHints() {
    if(!mXDG_Toplevel) {
      D1("No toplevel to apply resize hints");
      return;
    }
    if(mResizeEnabled) {
// Allow resizing: min 1x1, max unlimited (0,0)
      xdg_toplevel_set_min_size(mXDG_Toplevel, 1, 1);
      xdg_toplevel_set_max_size(mXDG_Toplevel, 0, 0);
    }
    else {
// Lock to current size
      int32_t w = static_cast<int32_t>(SizeX());
      int32_t h = static_cast<int32_t>(SizeY());
      xdg_toplevel_set_min_size(mXDG_Toplevel, w, h);
      xdg_toplevel_set_max_size(mXDG_Toplevel, w, h);
    }
// Commit to make the changes take effect
    if(mSurface) {
      wl_surface_commit(mSurface);
      wl_display_flush(EnginePtr()->WaylandDisplay());
    }
  }

  void WaylandWindowContext::BackendDisableResize() {
    mResizeEnabled = false;
    UpdateResizeHints();
  }

  void WaylandWindowContext::BackendEnableResize() {
    mResizeEnabled = true;
    UpdateResizeHints();
  }

  void WaylandWindowContext::BackendCursor(AUICursorType type) {
    AUI* au = EnginePtr();
    mCurrentCursorType = type;
// Create cursor surface if not yet created
    if(!mCursorSurface) {
      wl_compositor* compositor = au->WaylandCompositor();
      if(!compositor) {
        E("Wayland: No compositor");
        return;
      }
      mCursorSurface = wl_compositor_create_surface(compositor);
      if(!mCursorSurface) {
        E("Wayland: Failed to create cursor surface");
        return;
      }
    }
    int32_t idx = static_cast<int32_t>(type);
    CursorData& data = mCursorData[idx];
// Attempt to load embedded cursor for this type if not tried before
    if(!data.buffer && !data.embeddedAttempted) {
      data.embeddedAttempted = true;
      const uint8_t* start = nullptr;
      const uint8_t* end = nullptr;
      switch(type) {
        case AUICursorType::Default:
          start = _binary_fonts_mousepointer1_png_start;
          end = _binary_fonts_mousepointer1_png_end;
          break;
        case AUICursorType::HResize:
          start = _binary_fonts_mousepointerH_png_start;
          end = _binary_fonts_mousepointerH_png_end;
          break;
        case AUICursorType::VResize:
          start = _binary_fonts_mousepointerV_png_start;
          end = _binary_fonts_mousepointerV_png_end;
          break;
        default:
          E("unknown cursor type")
          break;
      }
      if(start && end && (end - start) > 0) {
        int32_t w = 0, h = 0;
        wl_buffer* buf = CreateBufferFromPNGData(au->WaylandShm(), start, SafeUINT32(end - start), &w, &h);
        if(buf) {
          data.buffer = buf;
          data.width = w;
          data.height = h;
// Set hotspot: for resize cursors, use centre; default top‑left.
          switch(type) {
            case AUICursorType::Default:
              data.hotspot_x = 0;
              data.hotspot_y = 0;
              break;
            case AUICursorType::HResize:
            case AUICursorType::VResize:
              data.hotspot_x = w / 2;
              data.hotspot_y = h / 2;
              break;
            default:
              E("unknown cursor type 2")
              break;
          }
        }
      }
    }
// If we have an embedded buffer, use it
    if(data.buffer) {
      wl_surface_attach(mCursorSurface, data.buffer, 0, 0);
      wl_surface_damage(mCursorSurface, 0, 0, data.width, data.height);
      wl_surface_commit(mCursorSurface);
      uint32_t serial = au->WaylandPointerSerial();
      wl_pointer* pointer = au->WaylandPointer();
      if(pointer) {
        wl_pointer_set_cursor(pointer, serial, mCursorSurface, data.hotspot_x, data.hotspot_y);
      }
      return;
    }
// Fallback: system cursor theme
// Load theme asynchronously if not loaded yet
    if(!mCursorTheme.load(std::memory_order_acquire)) {
      static std::atomic<bool> isLoading { false };
      bool expected = false;
      if(isLoading.compare_exchange_strong(expected, true)) {
        wl_shm* shm = au->WaylandShm();
        if(shm) {
          if(mCursorLoaderThread.joinable()) {
            mCursorLoaderThread.join();
          }
          mCursorLoaderThread = std::thread([this, shm]() noexcept {
            auto* theme = wl_cursor_theme_load(nullptr, 24, shm);
            mCursorTheme.store(theme, std::memory_order_release);
            mCursorNeedsApply = true;
            RequestRedraw();
          });
        }
      }
      return;// Wait for theme to load
    }
// Theme loaded: get the appropriate cursor
    const char* name = "left_ptr";
    switch(type) {
      case AUICursorType::HResize:
        name = "ew-resize";
        break;
      case AUICursorType::VResize:
        name = "ns-resize";
        break;
      default:
        name = "left_ptr";
        break;
    }
    wl_cursor* cursor = wl_cursor_theme_get_cursor(mCursorTheme.load(std::memory_order_relaxed), name);
    if(!cursor || cursor->image_count == 0) {
      E("Wayland: Failed to load cursor '{}'", name);
      return;
    }
    wl_cursor_image* image = cursor->images[0];
    wl_buffer* buffer = wl_cursor_image_get_buffer(image);
    if(!buffer) {
      E("Wayland: Failed to get buffer for cursor image");
      return;
    }
    int32_t width = static_cast<int32_t>(image->width);
    int32_t height = static_cast<int32_t>(image->height);
    int32_t hx = static_cast<int32_t>(image->hotspot_x);
    int32_t hy = static_cast<int32_t>(image->hotspot_y);
    wl_surface_attach(mCursorSurface, buffer, 0, 0);
    wl_surface_damage(mCursorSurface, 0, 0, width, height);
    wl_surface_commit(mCursorSurface);
    uint32_t serial = au->WaylandPointerSerial();
    wl_pointer* pointer = au->WaylandPointer();
    if(pointer) {
      wl_pointer_set_cursor(pointer, serial, mCursorSurface, hx, hy);
    }
  }

  bool WaylandWindowContext::CursorNeedsApply() {
    if(mCursorNeedsApply) {
      mCursorNeedsApply = false;
      return true;
    }
    return false;
  }

  wl_buffer* CreateBufferFromPNGData(wl_shm* shm, const uint8_t* data, size_t size, int32_t* out_w, int32_t* out_h) {
    if(!shm || !data || size == 0)
      return nullptr;
    int32_t w = 0, h = 0, channels = 0;
    uint8_t* raw_rgba = stbi_load_from_memory(data, static_cast<int32_t>(size), &w, &h, &channels, 4);
    if(!raw_rgba)
      return nullptr;
    int32_t stride = w * 4;
    size_t total_size = static_cast<size_t>(stride * h);
    int32_t fd = memfd_create("cursor_shm", MFD_CLOEXEC);
    if(fd < 0) {
      stbi_image_free(raw_rgba);
      return nullptr;
    }
    if(ftruncate(fd, static_cast<off_t>(total_size)) < 0) {
      close(fd);
      stbi_image_free(raw_rgba);
      return nullptr;
    }
    uint32_t* pixels = static_cast<uint32_t*>(mmap(nullptr, total_size, PROT_READ | PROT_WRITE,
    MAP_SHARED, fd, 0));
    if(pixels == MAP_FAILED) {
      close(fd);
      stbi_image_free(raw_rgba);
      return nullptr;
    }
// Convert RGBA to premultiplied ARGB8888 (Wayland format)
    for(int32_t i = 0; i < w * h; ++i) {
      uint8_t r = raw_rgba[i * 4 + 0];
      uint8_t g = raw_rgba[i * 4 + 1];
      uint8_t b = raw_rgba[i * 4 + 2];
      uint8_t a = raw_rgba[i * 4 + 3];
      r = static_cast<uint8_t>((r * a) / 255);
      g = static_cast<uint8_t>((g * a) / 255);
      b = static_cast<uint8_t>((b * a) / 255);
      pixels[i] = (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8)
          | static_cast<uint32_t>(b);
    }

    munmap(pixels, total_size);
    stbi_image_free(raw_rgba);

    wl_shm_pool* pool = wl_shm_create_pool(shm, fd, static_cast<int32_t>(total_size));
    wl_buffer* buffer = wl_shm_pool_create_buffer(pool, 0, w, h, stride, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);

    if(out_w)
      *out_w = w;
    if(out_h)
      *out_h = h;
    return buffer;
  }

  WaylandWindowContext::~WaylandWindowContext() {
    D2("Context Destroyed at {:p}", (void*)this);
    for(auto& data : mCursorData) {
      if(data.buffer) {
        wl_buffer_destroy(data.buffer);
        data.buffer = nullptr;
      }
    }
// 1. Clean up remaining active buffers safely
    for(size_t i = 0; i < AUI_NUM_BUFFERS; ++i) {
      if(mBuffers[i]) {
        auto* buf = static_cast<WaylandBuffer*>(mBuffers[i].get());
        if(buf->wlBuffer) {
          wl_buffer_destroy(buf->wlBuffer);
          buf->wlBuffer = nullptr;
        }
        if(buf->shmData && buf->size > 0) {
          munmap(buf->shmData, buf->size);
          buf->shmData = nullptr;
        }
        mBuffers[i].reset();
      }
    }
// 2. Clean up stranded orphans that never fired a callback before exit
    for(auto& buf : mOrphanedBuffers) {
      if(buf) {
        if(buf->wlBuffer) {
          wl_buffer_destroy(buf->wlBuffer);
          buf->wlBuffer = nullptr;
        }
        if(buf->shmData && buf->size > 0) {
          munmap(buf->shmData, buf->size);
          buf->shmData = nullptr;
        }
      }
    }
    mOrphanedBuffers.clear();// Safely wipes whatever is left
    if(mCursorSurface) {
      wl_surface_destroy(mCursorSurface);
      mCursorSurface = nullptr;
    }
    if(mCursorLoaderThread.joinable()) {
      mCursorLoaderThread.join();
    }
    if(mCursorTheme) {
      wl_cursor_theme_destroy(mCursorTheme);
      mCursorTheme = nullptr;
    }
    if(mFrameCallback) {
      wl_callback_destroy(mFrameCallback);
      mFrameCallback = nullptr;
    }
    if(mDecoration) {
      zxdg_toplevel_decoration_v1_destroy(mDecoration);
      mDecoration = nullptr;
    }
// 2. Destroy Wayland surface layers in chronological reverse order
    if(mXDG_Toplevel) {
      xdg_toplevel_destroy(mXDG_Toplevel);
      mXDG_Toplevel = nullptr;
    }
    if(mXDG_Surface) {
      xdg_surface_destroy(mXDG_Surface);
      mXDG_Surface = nullptr;
    }
    if(mSurface) {
      wl_surface_destroy(mSurface);
      mSurface = nullptr;
    }
  }

  void WaylandWindowContext::BackendShow() {
    if(!EnginePtr()) {
      E("window not initialized");
      return;
    }
    if(mConfigured) {
      Draw();
    }
    else {
// Kickstart the configure cycle: commit empty surface so compositor responds with xdg_surface.configure
      wl_surface_commit(mSurface);
      wl_display_flush(EnginePtr()->WaylandDisplay());
    }
  }

  void WaylandWindowContext::BackendHide() {
    if(!EnginePtr()) {
      E("window not initialized");
      return;
    }
    mConfigured = false;
// Unmap surface
    wl_surface_attach(mSurface, nullptr, 0, 0);
    wl_surface_commit(mSurface);
    wl_display_flush(EnginePtr()->WaylandDisplay());
  }
}
