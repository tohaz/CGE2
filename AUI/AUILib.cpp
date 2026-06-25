#include "AUILib.h"

extern "C" {
  UNUSED static FT_Error ftc_face_requester(UNUSED FTC_FaceID face_id,
  UNUSED FT_Library library, FT_Pointer request_data, FT_Face *aface) {
    aui::AUI* au = static_cast<aui::AUI*>(request_data);
    *aface = au->GetDefaultFontFace();
    return 0;
  }
}
extern "C" const uint8_t _binary_fonts_DejaVuSans_Bold_ttf_start[];
extern "C" const uint8_t _binary_fonts_DejaVuSans_Bold_ttf_end[];
size_t g_embedded_font_size = (size_t) _binary_fonts_DejaVuSans_Bold_ttf_end
    - (size_t) _binary_fonts_DejaVuSans_Bold_ttf_start;

//UNUSED static void xcb_connection_error_handler(const char *msg, UNUSED xcb_connection_t *conn) {
//    E("XCB CONNECTION ERROR: %s", msg ? msg : "(null)");
//}

namespace aui {
  AUI::AUI() {
    mSelfPipeFds[0] = -1;
    mSelfPipeFds[1] = -1;
    if(pipe(mSelfPipeFds) != 0) {
      mSelfPipeFds[0] = mSelfPipeFds[1] = -1;
      E("pipe error")
    }
  }

  UNUSED static void keyboard_keymap(void *data, struct wl_keyboard*, uint32_t format, int32_t fd, uint32_t size) {
    auto* aui = static_cast<AUI*>(data);
    if(format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
      close(fd);
      return;// Safe to return; we just don't support older formats
    }
// Check 1: Prevent 0-size mapping allocation errors
    if(size == 0) {
      close(fd);
      E("Wayland compositor sent a keymap event with a size of 0.");
    }
// Map the file descriptor
    char* map_str = static_cast<char*>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
    if(map_str == MAP_FAILED) {
      close(fd);
      E("Failed to mmap the keyboard keymap file descriptor.");
    }
// Create a safe context if it doesn't exist yet
    if(!aui->GetXkbContext()) {
      auto* ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
      if(!ctx) {
        munmap(map_str, size);
        close(fd);
        E("Failed to create xkb_context.");
      }
      aui->SetXkbContext(ctx);
    }
// Check 2: Verify the compositor actually null-terminated the string.
// The last byte inside the size scope MUST be '\0'.
    if(map_str[size - 1] != '\0') {
      munmap(map_str, size);
      close(fd);
      E("Wayland keymap string is not null-terminated. Preventing buffer overflow.");
    }
// Compile the keymap string
    struct xkb_keymap* keymap = xkb_keymap_new_from_string(aui->GetXkbContext(), map_str, XKB_KEYMAP_FORMAT_TEXT_V1,
        XKB_KEYMAP_COMPILE_NO_FLAGS);
// Clean up memory mappings immediately now that compilation is done/attempted
    munmap(map_str, size);
    close(fd);
// Check 3: Ensure the keymap actually compiled successfully
    if(!keymap) {
      E("xkb_keymap_new_from_string failed to compile the compositor keymap specification.");
    }
    aui->SetXkbKeymap(keymap);
// Allocate the tracking state using our verified keymap
    struct xkb_state* state = xkb_state_new(aui->GetXkbKeymap());
    if(!state) {
      E("Failed to allocate xkb_state engine from verified keymap.");
    }
    aui->SetXkbState(state);
  }

  UNUSED static void keyboard_key(void *data, struct wl_keyboard*, uint32_t, uint32_t, uint32_t key, uint32_t state) {
    auto* aui = static_cast<AUI*>(data);
    if(!aui->GetXkbState())
      return;
    xkb_keysym_t sym = xkb_state_key_get_one_sym(aui->GetXkbState(), key + 8);
    AUIKeyEvent ev;
    ev.pressed = (state == WL_KEYBOARD_KEY_STATE_PRESSED);
    ev.modifiers = AUIModifier::None;// you can enhance later
    AUIKeyCode code = translate_keysym_to_keycode(sym);
    if(code != AUIKeyCode::None) {
      ev.code = code;
      ev.unicode = 0;
    }
    else {
      char utf8[8];
      if(xkb_keysym_to_utf8(sym, utf8, sizeof(utf8)) > 0) {
        ev.unicode = static_cast<uint32_t>(static_cast<uint8_t>(utf8[0]));
        ev.code = AUIKeyCode::None;
      }
      else {
        ev.unicode = 0;
        ev.code = AUIKeyCode::None;
      }
    }
    AWindow* win = aui->GetFocusedWindow();
    if(win)
      win->OnKeyEvent(ev);
  }

  UNUSED static void keyboard_modifiers(void *data, struct wl_keyboard*, uint32_t, uint32_t mods_depressed,
      uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    auto* aui = static_cast<AUI*>(data);
    if(aui->GetXkbState()) {
      xkb_state_update_mask(aui->GetXkbState(), mods_depressed, mods_latched, mods_locked, 0, 0, group);
    }
  }

  static void keyboard_enter(void*, struct wl_keyboard*, uint32_t, struct wl_surface*, struct wl_array*) {
  }

  static void keyboard_leave(void*, struct wl_keyboard*, uint32_t, struct wl_surface*) {
  }

  static void keyboard_repeat_info(UNUSED void *data, UNUSED struct wl_keyboard *keyboard, UNUSED int32_t rate,
  UNUSED int32_t delay) {
  }

  static const struct wl_keyboard_listener keyboard_listener = { .keymap = keyboard_keymap, .enter = keyboard_enter,
      .leave = keyboard_leave, .key = keyboard_key, .modifiers = keyboard_modifiers, .repeat_info = keyboard_repeat_info// was nullptr
      };
// ------------------------------------------------------------------
// Wayland pointer event handlers (global for all windows)
// ------------------------------------------------------------------
  static void pointer_enter(void *data, UNUSED wl_pointer *ptr, UNUSED uint32_t serial, wl_surface *surface,
      wl_fixed_t sx, wl_fixed_t sy) {
    auto* aui = static_cast<AUI*>(data);
    uint64_t nativeId = reinterpret_cast<uint64_t>(surface);
    AWindow* win = aui->FindWindowByNativeId(nativeId);
    if(win) {
      aui->SetFocusedWindow(win);
      int32_t x = wl_fixed_to_int(sx);
      int32_t y = wl_fixed_to_int(sy);
      aui->SetLastPointerPos(x, y);
      aui->SetLastPointerSerial(serial);
      aui->ApplyDefaultCursor(ptr, serial);
    }
  }

  void AUI::ApplyDefaultCursor(wl_pointer *pointer, uint32_t serial) {
    if(!pointer || !mWaylandShm)
      return;
// 1. Lazy-init or retrieve your cursor surface
    if(!mWaylandCursorSurface && mWaylandCompositor) {
      mWaylandCursorSurface = wl_compositor_create_surface(mWaylandCompositor);
    }
    if(!mWaylandCursorSurface)
      return;
// 2. Load default theme (24px is standard, handles fallback smoothly)
    wl_cursor_theme* theme = wl_cursor_theme_load(nullptr, 24, mWaylandShm);
    if(!theme)
      return;
    wl_cursor* cursor = wl_cursor_theme_get_cursor(theme, "left_ptr");
    if(!cursor) {
      cursor = wl_cursor_theme_get_cursor(theme, "default");
    }
    if(cursor && cursor->image_count > 0) {
      wl_cursor_image* image = cursor->images[0];
      wl_buffer* buffer = wl_cursor_image_get_buffer(image);
      if(buffer) {
// Establish the client relationship with the cursor imagery
        wl_pointer_set_cursor(pointer, serial, mWaylandCursorSurface, static_cast<int32_t>(image->hotspot_x),
            static_cast<int32_t>(image->hotspot_y));
        wl_surface_attach(mWaylandCursorSurface, buffer, 0, 0);
        wl_surface_damage(mWaylandCursorSurface, 0, 0, static_cast<int32_t>(image->width),
            static_cast<int32_t>(image->height));
        wl_surface_commit(mWaylandCursorSurface);
      }
    }
    wl_cursor_theme_destroy(theme);// Safe to destroy; buffer reference stays alive in compositor
  }

  static void pointer_leave(void *data, UNUSED wl_pointer *ptr, UNUSED uint32_t serial,
  UNUSED wl_surface *surface) {
    auto* aui = static_cast<AUI*>(data);
    AWindow* win = aui->GetFocusedWindow();
    if(win) {
      win->ClearHover();
// Only clear keyboard focus if the window explicitly allows it
      if(!win->KeepFocusOnMouseLeave()) {
        aui->SetFocusedWindow(nullptr);
      }
    }
  }

  static void pointer_motion(void *data, UNUSED wl_pointer *ptr, UNUSED uint32_t time, wl_fixed_t sx, wl_fixed_t sy) {
    auto* aui = static_cast<AUI*>(data);
    int32_t x = wl_fixed_to_int(sx);
    int32_t y = wl_fixed_to_int(sy);
    aui->SetLastPointerPos(x, y);
    AWindow* win = aui->GetFocusedWindow();
    if(win) {
      win->OnMouseMove(x, y);
    }
  }

  static void pointer_button(void *data, UNUSED wl_pointer *ptr, UNUSED uint32_t serial,
  UNUSED uint32_t time, uint32_t button, uint32_t state) {
    auto* aui = static_cast<AUI*>(data);
    D3("Wayland button event: button={} state={}", button, state);
    AWindow* win = aui->GetFocusedWindow();
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
    auto* aui = static_cast<AUI*>(data);
    AWindow* win = aui->GetFocusedWindow();
    if(!win)
      return;
    if(axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
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
    auto* aui = static_cast<AUI*>(data);
    AWindow* win = aui->GetFocusedWindow();
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
    auto* aui = static_cast<AUI*>(data);
    if(capabilities & WL_SEAT_CAPABILITY_POINTER) {
      if(!aui->GetWaylandPointer()) {
        wl_pointer* ptr = wl_seat_get_pointer(seat);
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
    if(capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
      if(!aui->GetWaylandKeyboard()) {
        wl_keyboard* kbd = wl_seat_get_keyboard(seat);
        aui->SetWaylandKeyboard(kbd);
        wl_keyboard_add_listener(kbd, &keyboard_listener, aui);
      }
    }
    else {
      if(aui->GetWaylandKeyboard()) {
        wl_keyboard_destroy(aui->GetWaylandKeyboard());
        aui->SetWaylandKeyboard(nullptr);
      }
    }
  }

  static void seat_handle_name(UNUSED void *data, UNUSED wl_seat *seat, UNUSED const char *name) {
    D2("Seat name: {}", name ? name : "(null)");
  }

  static const wl_seat_listener seat_listener = { .capabilities = seat_handle_capabilities, .name = seat_handle_name, };

  static void registry_global(void *data, wl_registry *registry, uint32_t id, const char *interface,
  UNUSED uint32_t version) {
    auto* aui = static_cast<AUI*>(data);
    if(strcmp(interface, "wl_compositor") == 0) {
      auto* comp = static_cast<wl_compositor*>(wl_registry_bind(registry, id, &wl_compositor_interface, 1));
      aui->SetWaylandCompositor(comp);
    }
    else if(strcmp(interface, "wl_shm") == 0) {
      auto* shm = static_cast<wl_shm*>(wl_registry_bind(registry, id, &wl_shm_interface, 1));
      aui->SetWaylandShm(shm);
    }
    else if(strcmp(interface, "xdg_wm_base") == 0) {
      auto* base = static_cast<xdg_wm_base*>(wl_registry_bind(registry, id, &xdg_wm_base_interface, 1));
      aui->SetWaylandXdgBase(base);
      static const struct xdg_wm_base_listener wm_base_listener = { .ping = [](void*, xdg_wm_base *xdg,
          uint32_t serial) {
        D3("Ping received, serial={}", serial);
        xdg_wm_base_pong(xdg, serial);
      } };
      xdg_wm_base_add_listener(base, &wm_base_listener, aui);
      D2("Added xdg_wm_base ping listener");
    }
    else if(strcmp(interface, "zxdg_decoration_manager_v1") == 0) {
      auto* dm = static_cast<zxdg_decoration_manager_v1*>(wl_registry_bind(registry, id,
          &zxdg_decoration_manager_v1_interface, 1));
      aui->SetWaylandDecorationManager(dm);
    }
    else if(strcmp(interface, "wl_seat") == 0) {
      auto* seat = static_cast<wl_seat*>(wl_registry_bind(registry, id, &wl_seat_interface, 4));
      aui->SetWaylandSeat(seat);
      wl_seat_add_listener(seat, &seat_listener, aui);
    }
  }

  static void registry_remove(UNUSED void *data, UNUSED wl_registry *registry, UNUSED uint32_t id) {
  }

  static const wl_registry_listener registry_listener = { .global = registry_global, .global_remove = registry_remove };

  AUI* AUI::Create(const std::string &windowTitle) {
    AUI* au = new AUI();
    if(FT_Init_FreeType(&au->mFtLibrary) != 0) {
      E("FT_Init_FreeType failed");
      delete au;
      return nullptr;
    }
    FT_Error err = FT_Err_Unknown_File_Format;
// Load primary font (DejaVuSans)
    const char* fontPaths[] = { "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/DejaVuSans-Bold.ttf" };
    for(size_t i = 0; i < sizeof(fontPaths) / sizeof(fontPaths[0]); ++i) {
      err = FT_New_Face(au->mFtLibrary, fontPaths[i], 0, &au->mFtDefaultFace);
      if(err == 0) {
        D2("Loaded system font: {}", fontPaths[i]);
        break;
      }
    }
    if(err != 0) {
      if(g_embedded_font_size > 0) {
        err = FT_New_Memory_Face(au->mFtLibrary, _binary_fonts_DejaVuSans_Bold_ttf_start,
            (int64_t) g_embedded_font_size, 0, &au->mFtDefaultFace);
        if(err == 0)
          D1("Loaded embedded fallback font");
      }
    }
    if(err != 0) {
      E("No primary font");
    }
    FT_Set_Pixel_Sizes(au->mFtDefaultFace, 0, 14);
// Load fallback color emoji font (NotoColorEmoji)
    const char* emojiPath = "/usr/share/fonts/truetype/noto/NotoColorEmoji.ttf";
    err = FT_New_Face(au->mFtLibrary, emojiPath, 0, &au->mFallbackFace);
    if(err == 0) {
      D2("Loaded fallback font: {}", emojiPath);
      FT_Select_Charmap(au->mFallbackFace, FT_ENCODING_UNICODE);
      if(FT_IS_SCALABLE(au->mFallbackFace)) {
        FT_Set_Pixel_Sizes(au->mFallbackFace, 0, 14);
      }
      else {
        int32_t strike = find_closest_strike(au->mFallbackFace, 14);
        if(strike >= 0)
          FT_Select_Size(au->mFallbackFace, strike);
      }
    }
    else {
      D1("Failed to load fallback font {}", emojiPath);
      au->mFallbackFace = nullptr;
    }
// FreeType cache manager
    FT_Error error = FTC_Manager_New(au->mFtLibrary, AUI::kMaxFaces, AUI::kMaxSizes, AUI::kMaxBytes, ftc_face_requester,
        au, &au->mFTCManager);
    if(error) {
      E("FTC_Manager_New failed");
    }
    else {
      error = FTC_ImageCache_New(au->mFTCManager, &au->mFTCImageCache);
      if(error)
        E("FTC_ImageCache_New failed");
    }
    au->mFtDefaultFace->generic.data = au;
    au->PreRenderAscii(14);
// Wayland/XCB setup (unchanged)
    const char* waylandEnv = getenv("WAYLAND_DISPLAY");
    if(waylandEnv) {
      au->mWindowType = AUIWindowType::Wayland;
      au->mWaylandDisplay = wl_display_connect(nullptr);
      if(!au->mWaylandDisplay) {
        delete au;
        return nullptr;
      }
      au->mWaylandRegistry = wl_display_get_registry(au->mWaylandDisplay);
      wl_registry_add_listener(au->mWaylandRegistry, &registry_listener, au);
      wl_display_roundtrip(au->mWaylandDisplay);
      wl_display_roundtrip(au->mWaylandDisplay);
      if(!au->mWaylandCompositor || !au->mWaylandShm || !au->mWaylandXdgBase) {
        delete au;
        E("Wayland init failed")
      }
    }
    else {
      au->mWindowType = AUIWindowType::XCB;
    }
    au->mMainWnd = AWindow::AttachTo(au, windowTitle);
    if(!au->mMainWnd) {
      delete au;
      return nullptr;
    }
    au->Draw();
    return au;
  }

  void AUI::ProcessMessages() {
    mProcessingMessages = true;
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
      auto* ctx = static_cast<WaylandWindowContext*>(mMainWnd->GetBackend());
      if(ctx) {
        wl_display_roundtrip(mWaylandDisplay);
        D2("Initial configuration sync done.");
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
        wl_display_roundtrip(mWaylandDisplay);
      }
      ApplyPendingResizes();
    }
    else {
// ------------------------------------------------------------------
// Initial Synchronization for XCB (Pure XCB path)
// ------------------------------------------------------------------
      if(mXcbConnection) {
        D2("XCB: Blocking until X-server applies main's geometry changes...");
// Push all mapping and resize requests from main() to the server socket
        int32_t fle = xcb_flush(mXcbConnection);
        if(fle <= 0) {
          E("xcb flush error {}", fle)
        }
        xcb_get_input_focus_cookie_t sync_cookie = xcb_get_input_focus(mXcbConnection);
        xcb_get_input_focus_reply_t* sync_reply = xcb_get_input_focus_reply(mXcbConnection, sync_cookie, nullptr);
        if(sync_reply) {
          free(sync_reply);
        }
      }
// Now it's 100% safe to draw: the server window is already 800x600
      Draw();
    }
    auto processXcbEvents = [this]() {
      uint64_t numEventsDiscarded = 0;
      D1("processXcbEvents: enter");
      if(!mXcbConnection) {
        E("no connection before processing events")
      }
      xcb_generic_event_t* ev;
      D1("before retrieving evemt")
      while ((ev = xcb_poll_for_event(mXcbConnection))) {
        D1("processing next event")
        if(ev->response_type == 0) {
          xcb_generic_error_t* xcberror = (xcb_generic_error_t*) ev;
          E("X11 Error! Major opcode: %d, Error code: %d\n", xcberror->major_code, xcberror->error_code);
        }
        uint64_t wid = 0;
        uint8_t type = ev->response_type & 0x7F;
// Extract window ID from the event (common pattern)
        switch (type) {
          case XCB_BUTTON_PRESS: {
            auto* e = reinterpret_cast<xcb_button_press_event_t*>(ev);
            wid = e->event;
            break;
          }
          case XCB_BUTTON_RELEASE: {
            auto* e = reinterpret_cast<xcb_button_release_event_t*>(ev);
            wid = e->event;
            break;
          }
          case XCB_MOTION_NOTIFY: {
            auto* e = reinterpret_cast<xcb_motion_notify_event_t*>(ev);
            wid = e->event;
            break;
          }
          case XCB_KEY_PRESS: {
            auto* e = reinterpret_cast<xcb_key_press_event_t*>(ev);
            wid = e->event;
            break;
          }
          case XCB_KEY_RELEASE: {
            auto* e = reinterpret_cast<xcb_key_release_event_t*>(ev);
            wid = e->event;
            break;
          }
          case XCB_CONFIGURE_NOTIFY: {
            auto* e = reinterpret_cast<xcb_configure_notify_event_t*>(ev);
            wid = e->window;
            break;
          }
          case XCB_CLIENT_MESSAGE: {
            auto* e = reinterpret_cast<xcb_client_message_event_t*>(ev);
            wid = e->window;
            D1("WM_DELETE_WINDOW received for window ID {}", wid);
            ExitAUI();
            break;
          }
          case XCB_EXPOSE: {
            auto* e = reinterpret_cast<xcb_expose_event_t*>(ev);
            wid = e->window;
            break;
          }
          case XCB_MAP_NOTIFY: {
            auto* e = reinterpret_cast<xcb_map_notify_event_t*>(ev);
            wid = e->window;
            break;
          }
          case XCB_GE_GENERIC:
            D2("XCB_GE_GENERIC")
            free(ev);
            continue;
          case XCB_UNMAP_NOTIFY:
            D2("XCB_UNMAP_NOTIFY")
            free(ev);
            continue;
          case XCB_DESTROY_NOTIFY:
            D2("XCB_DESTROY_NOTIFY")
            free(ev);
            continue;
          default:
            E("Unhandled event type for window extraction: {}", type)
            free(ev);
            continue;
        }
        AWindow* win = FindWindowByNativeId(wid);
        if(win && win->GetBackend()) {
          win->GetBackend()->ProcessEvent(ev);
        }
        else {
          if(!win) {
            D2("event {} discarded - no window for wid {}", numEventsDiscarded, wid)
          }
          else if(!win->GetBackend()) {
            E("event discarded - no backend")
          }
          numEventsDiscarded++;
        }
        free(ev);
      }
      this->Draw();
      D3("processXcbEvents: exit");
    };
    int32_t xcb_fd = mXcbConnection ? xcb_get_file_descriptor(mXcbConnection) : -1;
    int32_t wl_fd = mWaylandDisplay ? wl_display_get_fd(mWaylandDisplay) : -1;
    while (!mShouldExit) {
      if(mShouldExit)
        break;
      if(mWindowType == AUIWindowType::XCB && mXcbConnection) {
        int32_t xcberrcode = xcb_connection_has_error(mXcbConnection);
        if(xcberrcode > 0) {
          switch (xcberrcode) {
            case XCB_CONN_ERROR:
              E("1Reason: Socket or pipe error. (Did the X server close?)")
              break;
            case XCB_CONN_CLOSED_PARSE_ERR:
              E("1Reason: Could not parse your DISPLAY environment variable.")
              break;
            case XCB_CONN_CLOSED_INVALID_SCREEN:
              E("1Reason: The requested X11 screen does not exist.")
              break;
            default:
              E("1Reason: Internal XCB error status {}", xcberrcode)
              break;
          }
        }
      }
      if(mXcbConnection)
        processXcbEvents();
      if(mWaylandDisplay) {
        wl_display_dispatch_pending(mWaylandDisplay);
      }
      pollfd fds[3] { };
      int32_t nfds = 0;
      int32_t xcb_idx = -1, wl_idx = -1, pipe_idx = -1;
      if(xcb_fd >= 0) {
        D("xcb fd check")
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
      D1("poll after...");
      if(mShouldExit) {
        D3("breaking from the cycle after poll()")
        break;
      }
      D3("poll after, ret={}", ret);
      if(ret < 0) {
        if(errno == EINTR) {
          if(mWaylandDisplay && wl_idx >= 0)
            wl_display_cancel_read(mWaylandDisplay);
          continue;
        }
        if(mWaylandDisplay && wl_idx >= 0)
          wl_display_cancel_read(mWaylandDisplay);
        E("poll failed")
        break;
      }
      if(mWaylandDisplay && wl_idx >= 0) {
        if(fds[wl_idx].revents & POLLIN) {
          D1("Wayland POLLIN detected");
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
        else {
          D1("flushing pongs")
          wl_display_flush(mWaylandDisplay);
        }
      }
      else {
        D1("not a wayland display")
      }
      if(mXcbConnection && xcb_idx >= 0 && (fds[xcb_idx].revents & POLLIN)) {
        D1("XCB POLLIN detected");
        processXcbEvents();
        D1("before xcb error check")
// ---- Check XCB connection error after processing events ----
        int32_t xcberrcode2 = xcb_connection_has_error(mXcbConnection);
        if(xcberrcode2 > 0) {
          switch (xcberrcode2) {
            case XCB_CONN_ERROR:
              E("2Reason: Socket or pipe error. (Did the X server close?)")
              break;
            case XCB_CONN_CLOSED_PARSE_ERR:
              E("2Reason: Could not parse your DISPLAY environment variable.")
              break;
            case XCB_CONN_CLOSED_INVALID_SCREEN:
              E("2Reason: The requested X11 screen does not exist.")
              break;
            default:
              E("2Reason: Internal XCB error status %d", xcberrcode2)
              break;
          }
        }
      }
      D1("pipe_idx {}, fd {}", pipe_idx, (fds[pipe_idx].revents & POLLIN))
      if(pipe_idx >= 0 && (fds[pipe_idx].revents & POLLIN)) {
        D1("Self-pipe wakeup read trigger");
        char buf[8];
        if(read(mSelfPipeFds[0], buf, sizeof(buf)) > 0) {
          FlushPendingDraws();// renders into buffer and enqueues command
          if(!mDrawCommands.empty()) {
            this->Draw();
          }
        }
      }
      else
        D1("no xcb events")
// Do NOT call Draw() here – it would cause unnecessary redraws.
// Widget property changes already trigger Draw() when needed.
///FlushConnection();
    }
    mProcessingMessages = false;
    D1("AUI::ProcessMessages() -> Clean exit.");
  }

  void AUI::Draw() {
    D4("cascade Draw({})", mDrawCounter++);
    D4("[AUI] Draw() called, command count = {}", mDrawCommands.size());
    UNUSED auto start = std::chrono::high_resolution_clock::now();
    if(mDrawCommands.empty()) {
      D4("zero commands");
      return;
    }
    D3("AUI::Draw: processing {} commands", mDrawCommands.size());
    std::lock_guard<std::recursive_mutex> lock(mCommandMutex);
    for(const auto& cmd : mDrawCommands) {
// --------------------------------------------------------------
//  NEW: Validate that the target window/surface still exists
// --------------------------------------------------------------
      bool valid = false;
      if(cmd.type == DrawCommandType::Xcb) {
        auto it = mXcbWindowMap.find(cmd.xcb.windowId);
        if(it != mXcbWindowMap.end())
          valid = true;
      }
      else if(cmd.type == DrawCommandType::Wayland) {
        uint64_t id = reinterpret_cast<uint64_t>(cmd.wayland.surface);
        auto it = mWaylandSurfaceMap.find(id);
        if(it != mWaylandSurfaceMap.end())
          valid = true;
      }
      if(!valid) {
        D3("Skipping draw command for destroyed window");
        continue;// skip this command, it will be cleared along with the rest
      }
// --------------------------------------------------------------
// XCB Graphics Pipeline
// --------------------------------------------------------------
      if(cmd.type == DrawCommandType::Xcb) {
        if(!mXcbConnection)
          E("Draw: XCB command but mXcbConnection is null");

        const auto& xcb = cmd.xcb;
        D3("XCB command: win={}, w={}, h={}, buffer={}", xcb.windowId, xcb.width, xcb.height, (void*)xcb.buffer);
        xcb_connection_t* conn = mXcbConnection;
        xcb_window_t win = static_cast<xcb_window_t>(xcb.windowId);
        AWindow* awin = FindWindowByNativeId(xcb.windowId);
        if(awin) {
          auto* ctx = static_cast<XcbWindowContext*>(awin->GetBackend());
          if(!ctx || !ctx->IsMapped()) {
            D1("Window not mapped, deferring draw");
            awin->Draw();// schedule later
            continue;
          }
        }
        else {
          E("Could not find AWindow for ID {}", xcb.windowId);
          continue;
        }
        xcb_get_geometry_cookie_t geom_cookie = xcb_get_geometry(conn, win);
        xcb_get_geometry_reply_t* geom = xcb_get_geometry_reply(conn, geom_cookie, nullptr);
        if(geom) {
          D3("Window geometry: depth={}, width={}, height={}", geom->depth, geom->width, geom->height);
          if(geom->width != xcb.width || geom->height != xcb.height) {
            D1("Window geometry (%dx%d) differs from command (%dx%d)", geom->width, geom->height, xcb.width, xcb.height);
            free(geom);
            // do not do any of this!!!
//            if(awin && !mShouldExit) { //
//              D1("calling additional draw")
//              awin->Draw();
//            }
            continue;
          }
          free(geom);
        }
        else {
          E("Failed to get window geometry");
        }
        xcb_gcontext_t temp_gc = xcb_generate_id(conn);
        uint32_t mask = 0;
        uint32_t value = 0;
        xcb_void_cookie_t gc_cookie = xcb_create_gc_checked(conn, temp_gc, win, mask, &value);
        xcb_generic_error_t* err = xcb_request_check(conn, gc_cookie);
        if(err) {
          E("XCB error: code=%d, major=%d, minor=%d", err->error_code, err->major_code, err->minor_code);
          free(err);
          xcb_free_gc(conn, temp_gc);
          continue;
        }
        xcb_image_t* img = xcb_image_create_native(conn, static_cast<uint16_t>(xcb.width),
            static_cast<uint16_t>(xcb.height), XCB_IMAGE_FORMAT_Z_PIXMAP, 24, nullptr, 0,
            reinterpret_cast<uint8_t*>(xcb.buffer));
        if(!img) {
          E("xcb_image_create_native failed");
          xcb_free_gc(conn, temp_gc);
          continue;
        }
        xcb_void_cookie_t put_cookie = xcb_image_put(conn, win, temp_gc, img, 0, 0, 0);
        err = xcb_request_check(conn, put_cookie);
        if(err) {
          E("XCB error: code=%d, major=%d, minor=%d", err->error_code, err->major_code, err->minor_code);
          free(err);
// The image might still be partially drawn; we skip clearing the command
// and let the window redraw later if needed.
        }
        else {
// success
        }
        xcb_free_gc(conn, temp_gc);
        xcb_image_destroy(img);
        if(xcb_flush(conn) <= 0) {
          E("xcb flush error2")
        }
      }
// --------------------------------------------------------------
// Wayland Graphics Pipeline
// --------------------------------------------------------------
      else if(cmd.type == DrawCommandType::Wayland) {
        const auto& wl = cmd.wayland;
        if(wl.surface && wl.buffer) {
// 1. Re-attach the buffer to tell the server we want to update the frame
          wl_surface_attach(wl.surface, wl.buffer, 0, 0);
// 2. Tell the compositor that the pixels inside the shared memory pool
//    have changed and it must re-read them.
          wl_surface_damage(wl.surface, 0, 0, static_cast<int32_t>(wl.width), static_cast<int32_t>(wl.height));
// 3. Submit the state and flush the connection
          wl_surface_commit(wl.surface);
        }
      }
    }
// All commands (including skipped ones) are cleared after processing
    mDrawCommands.clear();
    FlushConnection();
    UNUSED auto end = std::chrono::high_resolution_clock::now();
    D3("Backend commit took {} us", std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    D3("[AUI] Draw() finished, commands remain {}", mDrawCommands.size());
  }

  void AUI::EnqueueDrawCommand(const DrawCommand &cmd) {
    std::lock_guard<std::recursive_mutex> lock(mCommandMutex);
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
      if(xcb_flush(mXcbConnection) <= 0) {
        E("xcb flush error")
      }
    }
    else if(mWindowType == AUIWindowType::Wayland) {
      if(!mWaylandDisplay)
        E("FlushConnection: Wayland backend but mWaylandDisplay is null");
      wl_display_flush(mWaylandDisplay);
      if(mXcbConnection) {
        if(xcb_flush(mXcbConnection) <= 0) {
          E("xcb flush error")
        }
      }
    }
  }

  void AUI::ExitAUI() {
    if(mShouldExit) {
      D1("exit additional call")
      return;
    }
    D1("AUI::ExitAUI() starts")
    mShouldExit = true;
    if(mSelfPipeFds[1] >= 0) {
      UNUSED char token = 1;
      UNUSED size_t bytes = (size_t) write(mSelfPipeFds[1], &token, 1);
    }
    else {
      E("pipes are closed on exit")
    }
    if(mWaylandDisplay) {
      wl_display_flush(mWaylandDisplay);
    }
    else {
      D1("not a wayland display on exit")
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
    ClearDrawCommandsForWindow(nativeId);

    AWindow* win = nullptr;
    if(mWindowType == AUIWindowType::XCB) {
      auto it = mXcbWindowMap.find(nativeId);
      if(it == mXcbWindowMap.end()) {
        E("UnregisterWindow: XCB window not found (nativeId={})", nativeId);
        return;
      }
      win = it->second.get();
// Remove from pending draw list
      {
        std::lock_guard<std::mutex> lock(mPendingDrawMutex);
        mPendingDrawWindows.erase(std::remove(mPendingDrawWindows.begin(), mPendingDrawWindows.end(), win),
            mPendingDrawWindows.end());
      }
      if(mFocusedWindow == win) {
        mFocusedWindow = nullptr;
      }
      mXcbWindowMap.erase(it);
    }
    else {
      auto it = mWaylandSurfaceMap.find(nativeId);
      if(it == mWaylandSurfaceMap.end()) {
        E("UnregisterWindow: Wayland surface not found (nativeId={})", nativeId);
        return;
      }
      win = it->second.get();
      {
        std::lock_guard<std::mutex> lock(mPendingDrawMutex);
        mPendingDrawWindows.erase(std::remove(mPendingDrawWindows.begin(), mPendingDrawWindows.end(), win),
            mPendingDrawWindows.end());
      }
      if(mFocusedWindow == win) {
        mFocusedWindow = nullptr;
      }
      mWaylandSurfaceMap.erase(it);
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
    if(!mXcbConnection || xcb_connection_has_error(mXcbConnection)) {
      E("InitXcb: xcb_connect failed");
    }
    mXcbOwned = true;
    D2("xcb_connect returned {}", static_cast<void*>(mXcbConnection));
    const xcb_setup_t* setup = xcb_get_setup(mXcbConnection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    for(int32_t i = 0; i < screenIdx; ++i)
      xcb_screen_next(&iter);
    mXcbScreen = iter.data;
    D2("InitXcb done, screen={}", static_cast<void*>(mXcbScreen));
  }

  xcb_connection_t* AUI::GetXcbConnection() {
// we support both backends in Wayland
    if(!mXcbOwned)
      InitXcb();
    if(mXcbConnection && xcb_connection_has_error(mXcbConnection)) {
      E("XCB connection has error");
    }
    return mXcbConnection;
  }

  xcb_screen_t* AUI::GetXcbScreen() {
    if(!mXcbOwned)
      InitXcb();
    return mXcbScreen;
  }

  void AUI::ClearDrawCommandsForWindow(uint64_t nativeId) {
    std::lock_guard<std::recursive_mutex> lock(mCommandMutex);
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
    D3("Cleared draw commands for window ID {}", nativeId);
  }

  void AUI::ScheduleDraw(AWindow *win) {
    D3("[AUI] ScheduleDraw for win {}", (void*)win);
    std::lock_guard<std::mutex> lock(mPendingDrawMutex);
    if(std::find(mPendingDrawWindows.begin(), mPendingDrawWindows.end(), win) == mPendingDrawWindows.end()) {
      mPendingDrawWindows.push_back(win);
    }
    if(mSelfPipeFds[1] >= 0) {
      char c = 1;
      write(mSelfPipeFds[1], &c, 1);
      D3("[AUI] Pipe written");
    }
  }

  void AUI::FlushPendingDraws() {
    D3("[AUI] FlushPendingDraws() called");
    std::vector<AWindow*> windows;
    {
      std::lock_guard<std::mutex> lock(mPendingDrawMutex);
      windows.swap(mPendingDrawWindows);
    }
    for(AWindow* win : windows) {
      if(win && win->HasDrawPending()) {
        D3("[AUI] Flushing win {}", (void*)win);
        win->ForceDraw();// this enqueues a draw command
      }
    }
  }

  FT_Glyph AUI::GetCachedGlyph(FT_UInt glyph_index, FT_UInt font_size, FT_Int load_flags) {
    FTC_ImageTypeRec img_type;
    img_type.face_id = (FTC_FaceID) this;// unique identifier for our face
    img_type.width = font_size;
    img_type.height = font_size;
    img_type.flags = load_flags;
    FT_Glyph glyph = nullptr;
    FT_Error error = FTC_ImageCache_Lookup(mFTCImageCache, &img_type, glyph_index, &glyph, nullptr);
    if(error)
      return nullptr;
    return glyph;
  }

  FT_Fixed AUI::GetCachedAdvance(FT_UInt glyph_index, FT_UInt font_size) {
    uint64_t key = (static_cast<uint64_t>(glyph_index) << 32) | font_size;
    auto it = mAdvanceCache.find(key);
    if(it != mAdvanceCache.end())
      return it->second;
    std::unique_lock lock(GetFontMutex(), std::chrono::milliseconds(50));
    if(!lock.owns_lock()) {
      E("locked");
    }

    FT_Face face = GetDefaultFontFace();
    if(!face)
      return 0;
    FT_Set_Pixel_Sizes(face, 0, font_size);
    if(FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT) != 0)
      return 0;
    FT_Fixed advance = face->glyph->advance.x;
    mAdvanceCache[key] = advance;
    return advance;
  }

  void AUI::PreRenderAscii(uint32_t fontSize) {
    std::unique_lock lock(GetFontMutex(), std::chrono::milliseconds(50));
    if(!lock.owns_lock()) {
      E("locked");
    }

    FT_Face face = GetDefaultFontFace();
    if(!face)
      return;
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    for(int32_t ch = 32; ch <= 126; ++ch) {// use int32_t
      if(FT_Load_Char(face, (uint64_t) ch, FT_LOAD_RENDER | FT_LOAD_NO_HINTING) != 0)
        continue;
      FT_GlyphSlot slot = face->glyph;
      FT_Bitmap* src = &slot->bitmap;
      size_t size = static_cast<size_t>(src->rows) * src->width;
      uint8_t* buffer = new uint8_t[size];
      memcpy(buffer, src->buffer, size);
      CachedGlyph g;
      g.bitmap = buffer;
      g.width = static_cast<int32_t>(src->width);// cast
      g.rows = static_cast<int32_t>(src->rows);// cast
      g.left = slot->bitmap_left;
      g.top = slot->bitmap_top;
      g.asc = slot->bitmap_top;
      g.desc = (int32_t) src->rows - (int32_t) slot->bitmap_top;
      g.advance = static_cast<int32_t>(slot->advance.x >> 6);
      uint64_t key = (static_cast<uint64_t>(ch) << 32) | fontSize;
      mPreRenderedGlyphs[key] = g;
    }
  }

  const CachedGlyph* AUI::GetPreRenderedGlyph(uint32_t ch, uint32_t fontSize) const {
    uint64_t key = (static_cast<uint64_t>(ch) << 32) | fontSize;
    auto it = mPreRenderedGlyphs.find(key);
    return (it != mPreRenderedGlyphs.end()) ? &it->second : nullptr;
  }

  const uint8_t* AUI::GetScaledEmoji(FT_Face face, FT_UInt glyph_index, uint32_t size, int32_t &outWidth,
      int32_t &outHeight, int32_t &outPitch) {
    uint64_t key = (static_cast<uint64_t>(glyph_index) << 32) | size;
    auto it = mScaledEmojiCache.find(key);
    if(it != mScaledEmojiCache.end()) {
      const uint8_t* data = it->second.data();
      outWidth = *reinterpret_cast<const int32_t*>(data);
      outHeight = *reinterpret_cast<const int32_t*>(data + 4);
      outPitch = *reinterpret_cast<const int32_t*>(data + 8);
      return data + 12;
    }
// Load the glyph with color (no render to get original bitmap)
    FT_Error err = FT_Load_Glyph(face, glyph_index, FT_LOAD_COLOR | FT_LOAD_NO_HINTING);
    if(err)
      return nullptr;
    std::unique_lock lock(GetFontMutex(), std::chrono::milliseconds(50));
    if(!lock.owns_lock()) {
      E("locked");
    }

    FT_GlyphSlot slot = face->glyph;
    FT_Bitmap* bitmap = &slot->bitmap;
// If it's not a bitmap, convert it (e.g., if it's PNG format)
    if(slot->format != FT_GLYPH_FORMAT_BITMAP) {
      FT_Glyph glyph;
      if(FT_Get_Glyph(slot, &glyph) != 0)
        return nullptr;
      if(FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, 1) != 0) {
        FT_Done_Glyph(glyph);
        return nullptr;
      }
      FT_BitmapGlyph bmpGlyph = (FT_BitmapGlyph) glyph;
      bitmap = &bmpGlyph->bitmap;
// But we need to keep the glyph until we copy the data
// We'll copy the data and then free.
// However, we need the slot's metrics? We'll use the bitmap directly.
    }
// If pixel_mode is not BGRA, we cannot handle color; fallback to grayscale.
    if(bitmap->pixel_mode != FT_PIXEL_MODE_BGRA) {
// We could fallback to grayscale but we want color, so return nullptr.
      return nullptr;
    }
    int32_t targetSize = static_cast<int32_t>(size);
    int32_t dstW = targetSize;
    int32_t dstH = targetSize;
    int32_t dstPitch;
    uint8_t* scaled = scale_bgra_bitmap(bitmap->buffer, (int32_t) bitmap->width, (int32_t) bitmap->rows, dstW, dstH,
        bitmap->pitch, dstPitch);
    if(!scaled)
      return nullptr;
// Store in cache: width, height, pitch, then pixel data
    std::vector<uint8_t> cacheData;
    cacheData.reserve((size_t) (12 + dstH * dstPitch));
    const int32_t w = dstW, h = dstH, p = dstPitch;
    const uint8_t* wPtr = reinterpret_cast<const uint8_t*>(&w);
    const uint8_t* hPtr = reinterpret_cast<const uint8_t*>(&h);
    const uint8_t* pPtr = reinterpret_cast<const uint8_t*>(&p);
    cacheData.insert(cacheData.end(), wPtr, wPtr + 4);
    cacheData.insert(cacheData.end(), hPtr, hPtr + 4);
    cacheData.insert(cacheData.end(), pPtr, pPtr + 4);
    cacheData.insert(cacheData.end(), scaled, scaled + dstH * dstPitch);
    delete[] scaled;
    auto [it2, inserted] = mScaledEmojiCache.emplace(key, std::move(cacheData));
    const uint8_t* data = it2->second.data();
    outWidth = *reinterpret_cast<const int32_t*>(data);
    outHeight = *reinterpret_cast<const int32_t*>(data + 4);
    outPitch = *reinterpret_cast<const int32_t*>(data + 8);
    return data + 12;
  }

  void AUI::ApplyPendingResizes() {
    if(mWindowType == AUIWindowType::XCB) {
      for(auto& pair : mXcbWindowMap) {
        pair.second->ApplyPendingResize();
      }
    }
    else {
      for(auto& pair : mWaylandSurfaceMap) {
        pair.second->ApplyPendingResize();
      }
    }
  }

  AUI::~AUI() {
    mXcbWindowMap.clear();
    mWaylandSurfaceMap.clear();
// mMainWnd is now a dangling pointer; we set to nullptr to avoid accidental use.
    mMainWnd = nullptr;
    D3("set mMainWnd to nullptr")
    if(mXcbConnection && mXcbOwned) {
// Process any remaining events
      xcb_generic_event_t* ev;
      while ((ev = xcb_poll_for_event(mXcbConnection))) {
        free(ev);
      }
      if(xcb_flush(mXcbConnection) <= 0) {
        E("xcb flush error")
      }
      xcb_disconnect(mXcbConnection);
      mXcbConnection = nullptr;
    }
    if(mWaylandKeyboard)
      wl_keyboard_destroy(mWaylandKeyboard);
    if(mXkbState)
      xkb_state_unref(mXkbState);
    if(mXkbKeymap)
      xkb_keymap_unref(mXkbKeymap);
    if(mXkbCtx)
      xkb_context_unref(mXkbCtx);
    if(mWaylandDisplay) {
      if(mWaylandCursorSurface) {
        wl_surface_destroy(mWaylandCursorSurface);
        mWaylandCursorSurface = nullptr;
      }
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
    if(mFTCManager) {
      FTC_Manager_Done(mFTCManager);
      mFTCManager = nullptr;
// The manager has now freed the face; mark it invalid
      mFtDefaultFace = nullptr;
    }
    for(auto& pair : mPreRenderedGlyphs) {
      delete[] pair.second.bitmap;
    }
    mPreRenderedGlyphs.clear();
// Do NOT call FT_Done_Face if the manager already freed it
    if(mFtDefaultFace) {
      FT_Done_Face(mFtDefaultFace);
      mFtDefaultFace = nullptr;
    }
    if(mFtLibrary) {
      FT_Done_FreeType(mFtLibrary);
      mFtLibrary = nullptr;
    }
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

