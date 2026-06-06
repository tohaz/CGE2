#include "AUILib.h"
#include "AWindow.h"

namespace aui {

  AUI::AUI() {
    mSelfPipeFds[0] = -1;
    mSelfPipeFds[1] = -1;
    if(pipe(mSelfPipeFds) != 0) {
      mSelfPipeFds[0] = mSelfPipeFds[1] = -1;
    }
  }

// ------------------------------------------------------------------
// Wayland pointer event handlers (global for all windows)
// ------------------------------------------------------------------
  static void pointer_enter(void *data, UNUSED wl_pointer *ptr, UNUSED uint32_t serial, wl_surface *surface,
      wl_fixed_t sx, wl_fixed_t sy) {
    auto *aui = static_cast<AUI*>(data);
    uint64_t nativeId = reinterpret_cast<uint64_t>(surface);
    AWindow *win = aui->FindWindowByNativeId(nativeId);
    if(win) {
      aui->SetFocusedWindow(win);
      int32_t x = wl_fixed_to_int(sx);
      int32_t y = wl_fixed_to_int(sy);
      aui->SetLastPointerPos(x, y);
    }
  }

  static void pointer_leave(void *data, UNUSED wl_pointer *ptr, UNUSED uint32_t serial,
  UNUSED wl_surface *surface) {
    auto *aui = static_cast<AUI*>(data);
    AWindow *win = aui->GetFocusedWindow();
    if(win)
      win->ClearHover();
    aui->SetFocusedWindow(nullptr);
  }

  static void pointer_motion(void *data, UNUSED wl_pointer *ptr, UNUSED uint32_t time, wl_fixed_t sx, wl_fixed_t sy) {
    auto *aui = static_cast<AUI*>(data);
    int32_t x = wl_fixed_to_int(sx);
    int32_t y = wl_fixed_to_int(sy);
    aui->SetLastPointerPos(x, y);
    AWindow *win = aui->GetFocusedWindow();
    if(win) {
      win->OnMouseMove(x, y);
    }
  }

  static void pointer_button(void *data, UNUSED wl_pointer *ptr, UNUSED uint32_t serial,
  UNUSED uint32_t time, uint32_t button, uint32_t state) {
    auto *aui = static_cast<AUI*>(data);
    D2("Wayland button event: button={} state={}", button, state);
    AWindow *win = aui->GetFocusedWindow();
    if(!win)
      return;
    if(state == WL_POINTER_BUTTON_STATE_PRESSED) {
      win->OnMousePress(aui->GetLastPointerX(), aui->GetLastPointerY(), button);
    }
    else if(state == WL_POINTER_BUTTON_STATE_RELEASED) {
      win->OnMouseRelease(aui->GetLastPointerX(), aui->GetLastPointerY(), button);
    }
  }
  static void pointer_axis(UNUSED void *data, UNUSED wl_pointer *ptr, UNUSED uint32_t time,
  UNUSED uint32_t axis, UNUSED wl_fixed_t value) {
    auto *aui = static_cast<AUI*>(data);
    AWindow *win = aui->GetFocusedWindow();
    if(!win)
      return;
// Only handle vertical scrolling (horizontal can be added later)
    if(axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
// Convert wl_fixed_t to integer, invert sign so that positive delta = scroll up
// (matches XCB convention where button 4 = +1)
      int32_t delta = -wl_fixed_to_int(value);
      win->OnMouseWheel(delta);
    }
  }
  static void pointer_frame(UNUSED void *data, UNUSED wl_pointer *ptr) {
  }
  static void pointer_axis_source(UNUSED void *data, UNUSED wl_pointer *ptr,
  UNUSED uint32_t axis_source) {
  }
  static void pointer_axis_stop(UNUSED void *data, UNUSED wl_pointer *ptr,
  UNUSED uint32_t time, UNUSED uint32_t axis) {
  }
  static void pointer_axis_discrete(UNUSED void *data, UNUSED wl_pointer *ptr,
  UNUSED uint32_t axis, UNUSED int32_t discrete) {
    auto *aui = static_cast<AUI*>(data);
    AWindow *win = aui->GetFocusedWindow();
    if(!win)
      return;
    if(axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
      win->OnMouseWheel(-discrete);// discrete steps are already integers
    }
  }
  static void pointer_axis_value120(UNUSED void *data, UNUSED wl_pointer *ptr,
  UNUSED uint32_t axis, UNUSED int32_t value120) {
  }
  static void pointer_axis_relative_direction(UNUSED void *data, UNUSED wl_pointer *ptr,
  UNUSED uint32_t axis, UNUSED uint32_t direction) {
  }
  static const wl_pointer_listener pointer_listener = { .enter = pointer_enter, .leave = pointer_leave, .motion =
      pointer_motion, .button = pointer_button, .axis = pointer_axis, .frame = pointer_frame, .axis_source =
      pointer_axis_source, .axis_stop = pointer_axis_stop, .axis_discrete = pointer_axis_discrete, .axis_value120 =
      pointer_axis_value120, .axis_relative_direction = pointer_axis_relative_direction, };

  static void seat_handle_capabilities(void *data, wl_seat *seat, uint32_t capabilities) {
    auto *aui = static_cast<AUI*>(data);
    if(capabilities & WL_SEAT_CAPABILITY_POINTER) {
      if(!aui->GetWaylandPointer()) {
        wl_pointer *ptr = wl_seat_get_pointer(seat);
        aui->SetWaylandPointer(ptr);
        wl_pointer_add_listener(ptr, &pointer_listener, aui);
      }
    }
    else {
      if(aui->GetWaylandPointer()) {
        wl_pointer_destroy(aui->GetWaylandPointer());
        aui->SetWaylandPointer(nullptr);
      }
    }
  }

  static void seat_handle_name(UNUSED void *data, UNUSED wl_seat *seat, UNUSED const char *name) {
// Optional: log the seat name if needed
    D2("Seat name: {}", name ? name : "(null)");
  }

  static const wl_seat_listener seat_listener = { .capabilities = seat_handle_capabilities, .name = seat_handle_name, };

// ------------------------------------------------------------------
// Wayland registry listener
// ------------------------------------------------------------------
  static void registry_global(void *data, wl_registry *registry, uint32_t id, const char *interface,
  UNUSED uint32_t version) {
    auto *aui = static_cast<AUI*>(data);
    if(strcmp(interface, "wl_compositor") == 0) {
      auto *comp = static_cast<wl_compositor*>(wl_registry_bind(registry, id, &wl_compositor_interface, 1));
      aui->SetWaylandCompositor(comp);
    }
    else if(strcmp(interface, "wl_shm") == 0) {
      auto *shm = static_cast<wl_shm*>(wl_registry_bind(registry, id, &wl_shm_interface, 1));
      aui->SetWaylandShm(shm);
    }
    else if(strcmp(interface, "xdg_wm_base") == 0) {
      auto *base = static_cast<xdg_wm_base*>(wl_registry_bind(registry, id, &xdg_wm_base_interface, 1));
      aui->SetWaylandXdgBase(base);
      static const struct xdg_wm_base_listener wm_base_listener = { .ping = [](void*, xdg_wm_base *xdg,
          uint32_t serial) {
        D2("Ping received, serial={}", serial);
        xdg_wm_base_pong(xdg, serial);
      } };
      xdg_wm_base_add_listener(base, &wm_base_listener, aui);
      D2("Added xdg_wm_base ping listener");
    }
    else if(strcmp(interface, "zxdg_decoration_manager_v1") == 0) {
      auto *dm = static_cast<zxdg_decoration_manager_v1*>(wl_registry_bind(registry, id,
          &zxdg_decoration_manager_v1_interface, 1));
      aui->SetWaylandDecorationManager(dm);
    }
    else if(strcmp(interface, "wl_seat") == 0) {
      auto *seat = static_cast<wl_seat*>(wl_registry_bind(registry, id, &wl_seat_interface, 4));
      aui->SetWaylandSeat(seat);
      wl_seat_add_listener(seat, &seat_listener, aui);
    }
  }

  static void registry_remove(UNUSED void *data, UNUSED wl_registry *registry, UNUSED uint32_t id) {
  }

  static const wl_registry_listener registry_listener = { .global = registry_global, .global_remove = registry_remove };

// ------------------------------------------------------------------
// AUI implementation
// ------------------------------------------------------------------
  AUI* AUI::Create(const std::string &windowTitle) {
    AUI *au = new AUI();
// FreeType init
    if(FT_Init_FreeType(&au->mFtLibrary) != 0) {
      delete au;
      return nullptr;
    }
    const char *fontPath = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
    if(FT_New_Face(au->mFtLibrary, fontPath, 0, &au->mFtDefaultFace) != 0) {
      fontPath = "/usr/share/fonts/DejaVuSans-Bold.ttf";
      if(FT_New_Face(au->mFtLibrary, fontPath, 0, &au->mFtDefaultFace) != 0) {
        delete au;
        return nullptr;
      }
    }
    FT_Set_Pixel_Sizes(au->mFtDefaultFace, 0, 14);
    const char *waylandEnv = getenv("WAYLAND_DISPLAY");
    if(waylandEnv) {
      au->mWindowType = AUIWindowType::Wayland;
      au->mWaylandDisplay = wl_display_connect(nullptr);
      if(!au->mWaylandDisplay) {
        delete au;
        return nullptr;
      }
      au->mWaylandRegistry = wl_display_get_registry(au->mWaylandDisplay);
      wl_registry_add_listener(au->mWaylandRegistry, &registry_listener, au);
      if(wl_display_roundtrip(au->mWaylandDisplay) == -1) {
        E()
      }
      if(wl_display_roundtrip(au->mWaylandDisplay) == -1) {
        E()
      }
      if(!au->mWaylandCompositor || !au->mWaylandShm || !au->mWaylandXdgBase) {
        delete au;
        return nullptr;
      }
    }
    else {
      au->mWindowType = AUIWindowType::XCB;
// No connection created here – lazy initialization in GetXcbConnection
    }
// Create main window (this will trigger lazy XCB init if needed)
    au->mMainWnd = AWindow::AttachTo(au, windowTitle);
    if(!au->mMainWnd) {
      delete au;
      return nullptr;
    }
    au->Draw();

    return au;
  }

  void AUI::ProcessMessages() {
    D2("AUI::ProcessMessages() -> Entering clean hybrid loop.");
    if(mWindowType == AUIWindowType::XCB && !mXcbConnection) {
      E("ProcessMessages: XCB backend but mXcbConnection is null");
    }
    if(mWindowType == AUIWindowType::Wayland && !mWaylandDisplay) {
      E("ProcessMessages: Wayland backend but mWaylandDisplay is null");
    }
    if(mWindowType == AUIWindowType::Wayland && mWaylandDisplay) {
      if(!mMainWnd)
        E("ProcessMessages: Wayland backend but mMainWnd is null");
      auto *ctx = static_cast<WaylandWindowContext*>(mMainWnd->GetBackend());
      if(ctx) {
        wl_display_roundtrip(mWaylandDisplay);
        D2("Initial configuration sync done.");
        if(ctx->GetSoftwareBuffer() == nullptr) {
          uint32_t w = ctx->mPendingWidth ? ctx->mPendingWidth : mMainWnd->SizeX();
          uint32_t h = ctx->mPendingHeight ? ctx->mPendingHeight : mMainWnd->SizeY();
          ctx->CreateShmBuffer(w, h);
        }
        uint32_t w = ctx->mPendingWidth ? ctx->mPendingWidth : mMainWnd->SizeX();
        uint32_t h = ctx->mPendingHeight ? ctx->mPendingHeight : mMainWnd->SizeY();
        if(ctx->mPendingResizeEnabled) {
          xdg_toplevel_set_min_size(ctx->mToplevel, 0, 0);
          xdg_toplevel_set_max_size(ctx->mToplevel, 0, 0);
        }
        else {
          xdg_toplevel_set_min_size(ctx->mToplevel, static_cast<int32_t>(w), static_cast<int32_t>(h));
          xdg_toplevel_set_max_size(ctx->mToplevel, static_cast<int32_t>(w), static_cast<int32_t>(h));
        }
        if(mMainWnd) {
          mMainWnd->Draw();
        }
        this->Draw();
        D2("Initial frame committed securely via double-buffer pipeline.");
// WAIT FOR FIRST BUFFER RELEASE – fixes black window
        wl_display_roundtrip(mWaylandDisplay);
        D2("First buffer released, redrawing to ensure window content");
        this->Draw();
      }
    }
    else {
// ------------------------------------------------------------------
// Initial Synchronization for XCB (Pure XCB path)
// ------------------------------------------------------------------
      if(mXcbConnection) {
        D2("XCB: Blocking until X-server applies main's geometry changes...");
// Push all mapping and resize requests from main() to the server socket
        xcb_flush(mXcbConnection);
        xcb_get_input_focus_cookie_t sync_cookie = xcb_get_input_focus(mXcbConnection);
        xcb_get_input_focus_reply_t *sync_reply = xcb_get_input_focus_reply(mXcbConnection, sync_cookie, nullptr);
        if(sync_reply) {
          free(sync_reply);
        }
      }
// Now it's 100% safe to draw: the server window is already 800x600
      Draw();
    }
    auto processXcbEvents = [this]() {
      D3("processXcbEvents: enter");
      if(!mXcbConnection)
        return;
      xcb_generic_event_t *ev;
      while ((ev = xcb_poll_for_event(mXcbConnection))) {
        uint8_t type = ev->response_type & 0x7F;
        uint64_t wid = 0;
        switch (type) {
          case XCB_BUTTON_PRESS: {
            auto *btn = reinterpret_cast<xcb_button_press_event_t*>(ev);
            wid = btn->event;
            AWindow *win = FindWindowByNativeId(wid);
            if(win) {
// XCB mouse wheel buttons: 4 = scroll up, 5 = scroll down
              if(btn->detail == 4) {
                win->OnMouseWheel(1);
              }
              else if(btn->detail == 5) {
                win->OnMouseWheel(-1);
              }
              else {
                win->OnMousePress(btn->event_x, btn->event_y, btn->detail);
              }
            }
            break;
          }
          case XCB_BUTTON_RELEASE: {
            auto *btn = reinterpret_cast<xcb_button_release_event_t*>(ev);
            wid = btn->event;
            AWindow *win = FindWindowByNativeId(wid);
            if(win) {
              win->OnMouseRelease(btn->event_x, btn->event_y, btn->detail);
            }
            break;
          }
          case XCB_CONFIGURE_NOTIFY: {
            auto *cfg = reinterpret_cast<xcb_configure_notify_event_t*>(ev);
            wid = cfg->window;
            AWindow *win = FindWindowByNativeId(wid);
            if(win)
              win->GetBackend()->ProcessEvent(ev);
            break;
          }
          case XCB_CLIENT_MESSAGE: {
            auto *msg = reinterpret_cast<xcb_client_message_event_t*>(ev);
            wid = msg->window;
            AWindow *win = FindWindowByNativeId(wid);
            if(win)
              win->GetBackend()->ProcessEvent(ev);
            break;
          }
          case XCB_MOTION_NOTIFY: {
            auto *motion = reinterpret_cast<xcb_motion_notify_event_t*>(ev);
            wid = motion->event;
            AWindow *win = FindWindowByNativeId(wid);
            if(win)
              win->OnMouseMove(motion->event_x, motion->event_y);
            break;
          }
          case XCB_EXPOSE: {
            auto *exp = reinterpret_cast<xcb_expose_event_t*>(ev);
            wid = exp->window;
            AWindow *win = FindWindowByNativeId(wid);
            if(win)
              win->GetBackend()->ProcessEvent(ev);
            break;
          }
          case XCB_MAP_NOTIFY: {
            auto *map = reinterpret_cast<xcb_map_notify_event_t*>(ev);
            wid = map->window;
            AWindow *win = FindWindowByNativeId(wid);
            if(win)
              win->GetBackend()->ProcessEvent(ev);
            break;
          }
          default:
            break;
        }
        free(ev);
      }
      this->Draw();
      D3("processXcbEvents: exit");
    };
    int32_t xcb_fd = mXcbConnection ? xcb_get_file_descriptor(mXcbConnection) : -1;
    int32_t wl_fd = mWaylandDisplay ? wl_display_get_fd(mWaylandDisplay) : -1;
    while (!mShouldExit) {
      if(mWindowType == AUIWindowType::XCB && mXcbConnection && xcb_connection_has_error(mXcbConnection)) {
        D1("XCB connection error detected at loop start, exiting AUI");
        ExitAUI();
        break;
      }
      if(mXcbConnection)
        processXcbEvents();
      if(mWaylandDisplay) {
        wl_display_dispatch_pending(mWaylandDisplay);
      }
      if(mShouldExit)
        break;
      pollfd fds[3] { };
      int32_t nfds = 0;
      int32_t xcb_idx = -1, wl_idx = -1, pipe_idx = -1;
      if(xcb_fd >= 0) {
        fds[nfds].fd = xcb_fd;
        fds[nfds].events = POLLIN;
        xcb_idx = nfds++;
      }
      if(wl_fd >= 0) {
        fds[nfds].fd = wl_fd;
        fds[nfds].events = POLLIN;
        wl_idx = nfds++;
      }
      if(mSelfPipeFds[0] >= 0) {
        fds[nfds].fd = mSelfPipeFds[0];
        fds[nfds].events = POLLIN;
        pipe_idx = nfds++;
      }
      if(mWaylandDisplay && wl_idx >= 0) {
        while (wl_display_prepare_read(mWaylandDisplay) != 0) {
          wl_display_dispatch_pending(mWaylandDisplay);
        }
      }
      FlushPendingDraws();
      D3("poll before blocking...");
      int32_t ret = poll(fds, static_cast<nfds_t>(nfds), -1);
      D3("poll after, ret={}", ret);
      if(ret < 0) {
        if(errno == EINTR) {
          if(mWaylandDisplay && wl_idx >= 0)
            wl_display_cancel_read(mWaylandDisplay);
          continue;
        }
        if(mWaylandDisplay && wl_idx >= 0)
          wl_display_cancel_read(mWaylandDisplay);
        break;
      }
      if(mWaylandDisplay && wl_idx >= 0) {
        if(fds[wl_idx].revents & POLLIN) {
          D3("Wayland POLLIN detected");
          wl_display_read_events(mWaylandDisplay);
          wl_display_dispatch_pending(mWaylandDisplay);
// ---- Check Wayland display error after dispatch ----
          if(wl_display_get_error(mWaylandDisplay) != 0) {
            D1("Wayland display error detected, exiting AUI");
            ExitAUI();
            break;
          }
        }
        else {
          wl_display_cancel_read(mWaylandDisplay);
        }
        if(mDrawCommands.size() > 0) {
          this->Draw();
        }
      }
      if(mXcbConnection && xcb_idx >= 0 && (fds[xcb_idx].revents & POLLIN)) {
        D3("XCB POLLIN detected");
        processXcbEvents();
// ---- Check XCB connection error after processing events ----
        if(xcb_connection_has_error(mXcbConnection)) {
          D1("XCB connection error after processing events, exiting AUI");
          ExitAUI();
          break;
        }
      }
      if(pipe_idx >= 0 && (fds[pipe_idx].revents & POLLIN)) {
        D1("Self-pipe wakeup read trigger");
        char buf[8];
        if(read(mSelfPipeFds[0], buf, sizeof(buf)) > 0) {
        }
      }
// Do NOT call Draw() here – it would cause unnecessary redraws.
// Widget property changes already trigger Draw() when needed.
    }
    D2("AUI::ProcessMessages() -> Clean exit.");
  }

  void AUI::Draw() {
    D3("AUI::Draw: cascade redraw start");
// 1. Redraw all registered windows to populate their respective buffers
    if(mWindowType == AUIWindowType::XCB) {
      for (auto &pair : mXcbWindowMap) {
        if(pair.second)
          pair.second->Draw();
      }
    }
    else if(mWindowType == AUIWindowType::Wayland) {
      for (auto &pair : mWaylandSurfaceMap) {
        if(pair.second)
          pair.second->Draw();
      }
    }
    if(mDrawCommands.size() == 0) {
      D2("zero commands");
      return;
    }
    D3("AUI::Draw: processing {} commands", mDrawCommands.size());
    std::lock_guard<std::mutex> lock(mCommandMutex);
    for (const auto &cmd : mDrawCommands) {
// ------------------------------------------------------------------
// XCB Graphics Pipeline Routing
// ------------------------------------------------------------------
      if(cmd.type == DrawCommandType::Xcb) {
        if(!mXcbConnection)
          E("Draw: XCB command but mXcbConnection is null");
        const auto &xcb = cmd.xcb;
        D3("XCB command: win={}, w={}, h={}, buffer={}", xcb.windowId, xcb.width, xcb.height, (void*)xcb.buffer);
        xcb_connection_t *conn = mXcbConnection;
        xcb_window_t win = static_cast<xcb_window_t>(xcb.windowId);
        xcb_get_geometry_cookie_t geom_cookie = xcb_get_geometry(conn, win);
        xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(conn, geom_cookie, nullptr);
        if(geom) {
          D3("Window geometry: depth={}, width={}, height={}", geom->depth, geom->width, geom->height);
          free(geom);
        }
        xcb_image_t *img = xcb_image_create_native(conn, static_cast<uint16_t>(xcb.width),
            static_cast<uint16_t>(xcb.height), XCB_IMAGE_FORMAT_Z_PIXMAP, 24, nullptr, 0,
            reinterpret_cast<uint8_t*>(xcb.buffer));
        if(img) {
          D3("Image created: depth={}, stride={}", img->depth, img->stride);
          xcb_gcontext_t temp_gc = xcb_generate_id(conn);
          uint32_t mask = 0;
          uint32_t value = 0;
          xcb_create_gc(conn, temp_gc, win, mask, &value);
          UNUSED xcb_void_cookie_t ret = xcb_image_put(conn, win, temp_gc, img, 0, 0, 0);
          D3("xcb_image_put returned {}", ret.sequence);
          xcb_free_gc(conn, temp_gc);
          xcb_image_destroy(img);
          xcb_flush(conn);
        }
        else {
          D1("xcb_image_create_native failed");
        }
      }

      else if(cmd.type == DrawCommandType::Wayland) {
        const auto &wl = cmd.wayland;
        if(wl.surface && wl.buffer) {
// 1. Re-attach the buffer to tell the server we want to update the frame
          wl_surface_attach(wl.surface, wl.buffer, 0, 0);
// 2. FUNDAMENTAL WAYLAND STEP: Tell the compositor that the pixels inside
// the shared memory pool have changed and it must re-read them.
          wl_surface_damage(wl.surface, 0, 0, static_cast<int32_t>(wl.width), static_cast<int32_t>(wl.height));
// 3. Submit the state and flush the connection
          wl_surface_commit(wl.surface);
        }
      }
    }
    mDrawCommands.clear();
    FlushConnection();// Safely executes xcb_flush() or wl_display_flush()
    D3("AUI::Draw: flushed connection");
  }

  void AUI::EnqueueDrawCommand(const DrawCommand &cmd) {
    std::lock_guard<std::mutex> lock(mCommandMutex);
    mDrawCommands.push_back(cmd);
  }

  void* AUI::GetNativeDisplay() const {
    if(mWindowType == AUIWindowType::XCB)
      return mXcbConnection;
    else
      return mWaylandDisplay;
  }

  int32_t AUI::GetConnectionFileDescriptor() const {
    if(mWindowType == AUIWindowType::XCB) {
      if(!mXcbConnection)
        E("GetConnectionFileDescriptor: XCB backend but mXcbConnection is null");
      return xcb_get_file_descriptor(mXcbConnection);
    }
    else if(mWindowType == AUIWindowType::Wayland) {
      if(!mWaylandDisplay)
        E("GetConnectionFileDescriptor: Wayland backend but mWaylandDisplay is null");
      return wl_display_get_fd(mWaylandDisplay);
    }
    return -1;
  }

  void AUI::FlushConnection() {
    if(mWindowType == AUIWindowType::XCB) {
      if(!mXcbConnection)
        E("FlushConnection: XCB backend but mXcbConnection is null");
      xcb_flush(mXcbConnection);
    }
    else if(mWindowType == AUIWindowType::Wayland) {
      if(!mWaylandDisplay)
        E("FlushConnection: Wayland backend but mWaylandDisplay is null");
      wl_display_flush(mWaylandDisplay);
    }
  }

  void AUI::ExitAUI() {
    mShouldExit = true;
    if(mSelfPipeFds[1] >= 0) {
      char token = 1;
      write(mSelfPipeFds[1], &token, 1);
    }
  }

  void AUI::RegisterWindow(uint64_t nativeId, std::unique_ptr<AWindow> win) {
    if(mWindowType == AUIWindowType::XCB) {
      if(mXcbWindowMap.find(nativeId) != mXcbWindowMap.end()) {
        E("RegisterWindow: duplicate XCB window ID {}", nativeId);
      }
      mXcbWindowMap[nativeId] = std::move(win);
    }
    else {
      if(mWaylandSurfaceMap.find(nativeId) != mWaylandSurfaceMap.end()) {
        E("RegisterWindow: duplicate Wayland surface ID {}", nativeId);
      }
      mWaylandSurfaceMap[nativeId] = std::move(win);
    }
  }

  void AUI::UnregisterWindow(uint64_t nativeId) {
    D2("UnregisterWindow: nativeId={}", nativeId);
    if(mWindowType == AUIWindowType::XCB) {
      auto it = mXcbWindowMap.find(nativeId);
      if(it == mXcbWindowMap.end()) {
        E("UnregisterWindow: XCB window not found (nativeId={})", nativeId);
      }
      mXcbWindowMap.erase(it);
      D2("Removed XCB window, map size now {}", mXcbWindowMap.size());
      std::lock_guard<std::mutex> lock(mCommandMutex);
      mDrawCommands.erase(
          std::remove_if(mDrawCommands.begin(), mDrawCommands.end(), [nativeId](const DrawCommand &cmd) {
            return cmd.type == DrawCommandType::Xcb && cmd.xcb.windowId == nativeId;
          }),
          mDrawCommands.end());
    }
    else {
      auto it = mWaylandSurfaceMap.find(nativeId);
      if(it == mWaylandSurfaceMap.end()) {
        E("UnregisterWindow: Wayland surface not found (nativeId={})", nativeId);
      }
      mWaylandSurfaceMap.erase(it);
      D2("Removed Wayland surface, map size now {}", mWaylandSurfaceMap.size());
    }
  }

  AWindow* AUI::FindWindowByNativeId(uint64_t nativeId) const {
    if(mWindowType == AUIWindowType::XCB) {
      auto it = mXcbWindowMap.find(nativeId);
      if(it != mXcbWindowMap.end()) {
        return it->second.get();
      }
    }
    else if(mWindowType == AUIWindowType::Wayland) {
      auto it = mWaylandSurfaceMap.find(nativeId);
      if(it != mWaylandSurfaceMap.end()) {
        return it->second.get();
      }
    }
    return nullptr;
  }

  static void xdg_wm_base_ping_handler(UNUSED void *data, xdg_wm_base *xdg, uint32_t serial) {
    DT("Ping received, serial={}", serial);
    xdg_wm_base_pong(xdg, serial);
  }

  static const struct xdg_wm_base_listener xdg_wm_base_listener = { .ping = xdg_wm_base_ping_handler, };

  void AUI::InitXcb() {
    D2("InitXcb called");
    if(mXcbOwned) {
      E("already initialized");
      return;
    }
    int32_t screenIdx = 0;
    mXcbConnection = xcb_connect(nullptr, &screenIdx);
    if(xcb_connection_has_error(mXcbConnection)) {
      E("InitXcb: xcb_connect failed");
    }
    mXcbOwned = true;
    D2("xcb_connect returned {}", static_cast<void*>(mXcbConnection));
    const xcb_setup_t *setup = xcb_get_setup(mXcbConnection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    for (int32_t i = 0; i < screenIdx; ++i)
      xcb_screen_next(&iter);
    mXcbScreen = iter.data;
    D2("InitXcb done, screen={}", static_cast<void*>(mXcbScreen));
  }

  xcb_connection_t* AUI::GetXcbConnection() {
    if(mWindowType != AUIWindowType::XCB)
      E("GetXcbConnection called when backend is not XCB");
    if(!mXcbOwned)
      InitXcb();
    if(!mXcbConnection)
      E("GetXcbConnection: failed to initialize XCB connection");
    return mXcbConnection;
  }

  xcb_screen_t* AUI::GetXcbScreen() {
    if(!mXcbOwned)
      InitXcb();
    return mXcbScreen;
  }

  void AUI::ClearDrawCommandsForWindow(uint64_t nativeId) {
    std::lock_guard<std::mutex> lock(mCommandMutex);
    mDrawCommands.erase(std::remove_if(mDrawCommands.begin(), mDrawCommands.end(), [nativeId](const DrawCommand &cmd) {
      if(cmd.type == DrawCommandType::Wayland) {
        return reinterpret_cast<uint64_t>(cmd.wayland.surface) == nativeId;
      }
      else if(cmd.type == DrawCommandType::Xcb) {
        return cmd.xcb.windowId == nativeId;
      }
      return false;
    }),
    mDrawCommands.end());
    D2("Cleared draw commands for window ID {}", nativeId);
  }

  void AUI::ScheduleDraw(AWindow *win) {
    if(!win)
      return;
    std::lock_guard<std::mutex> lock(mPendingDrawMutex);
// Avoid duplicates
    if(std::find(mPendingDrawWindows.begin(), mPendingDrawWindows.end(), win) == mPendingDrawWindows.end()) {
      mPendingDrawWindows.push_back(win);
    }
// Wake up the main loop (optional, but good for responsiveness)
    if(mSelfPipeFds[1] >= 0) {
      char c = 1;
      write(mSelfPipeFds[1], &c, 1);
    }
  }

  void AUI::FlushPendingDraws() {
    std::vector<AWindow*> windows;
    {
      std::lock_guard<std::mutex> lock(mPendingDrawMutex);
      windows.swap(mPendingDrawWindows);
    }
    for (AWindow *win : windows) {
      if(win && win->HasDrawPending()) {
        win->ForceDraw();// ForceDraw will clear the flag and redraw
      }
    }
  }

  AUI::~AUI() {
    mXcbWindowMap.clear();
    mWaylandSurfaceMap.clear();
// mMainWnd is now a dangling pointer; we set to nullptr to avoid accidental use.
    mMainWnd = nullptr;
    if(mXcbConnection && mXcbOwned) {
// Process any remaining events
      xcb_generic_event_t *ev;
      while ((ev = xcb_poll_for_event(mXcbConnection))) {
        free(ev);
      }
      xcb_flush(mXcbConnection);
      xcb_disconnect(mXcbConnection);
      mXcbConnection = nullptr;
    }

    if(mWaylandDisplay) {
// Destroy Wayland objects in reverse order of creation
      if(mWaylandPointer) {
        wl_pointer_destroy(mWaylandPointer);
        mWaylandPointer = nullptr;
      }
      if(mWaylandSeat) {
        wl_seat_destroy(mWaylandSeat);
        mWaylandSeat = nullptr;
      }
      if(mWaylandDecorationManager) {
        zxdg_decoration_manager_v1_destroy(mWaylandDecorationManager);
        mWaylandDecorationManager = nullptr;
      }
      if(mWaylandCompositor) {
        wl_compositor_destroy(mWaylandCompositor);
        mWaylandCompositor = nullptr;
      }
      if(mWaylandShm) {
        wl_shm_destroy(mWaylandShm);
        mWaylandShm = nullptr;
      }
      if(mWaylandXdgBase) {
        xdg_wm_base_destroy(mWaylandXdgBase);
        mWaylandXdgBase = nullptr;
      }
      if(mWaylandRegistry) {
        wl_registry_destroy(mWaylandRegistry);
        mWaylandRegistry = nullptr;
      }
      wl_display_disconnect(mWaylandDisplay);
      mWaylandDisplay = nullptr;
    }
    if(mFtDefaultFace)
      FT_Done_Face(mFtDefaultFace);
    if(mFtLibrary)
      FT_Done_FreeType(mFtLibrary);
    if(mSelfPipeFds[0] >= 0) {
      close(mSelfPipeFds[0]);
      mSelfPipeFds[0] = -1;
    }
    if(mSelfPipeFds[1] >= 0) {
      close(mSelfPipeFds[1]);
      mSelfPipeFds[1] = -1;
    }
  }

}// namespace aui

