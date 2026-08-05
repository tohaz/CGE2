#include "AUILib.h"

namespace aui {
  XCBWindowContext::XCBWindowContext(UNUSED AUI *au) {
    EnginePtr(au);
    CreateFrame();
  }

  void XCBWindowContext::Draw() {
    D4()
    AUI* au = EnginePtr();
    xcb_connection_t* conn = au->X11Connection();
    xcb_screen_t* screen = au->X11Screen();
    if(!conn || !screen || !mWindowId) {
      E("XCB context handles are invalid during Draw");
      return;
    }
    int32_t width = SafeINT32(SizeX());
    int32_t height = SafeINT32(SizeY());
    if(width <= 0 || height <= 0)
      return;
    int32_t idx = FindFreeBufferIndex();
    if(idx < 0) {
      D2("No free buffer, skipping frame");
      return;
    }
    XCBBuffer* back_buf = static_cast<XCBBuffer*>(mBuffers[(size_t) idx].get());
    back_buf->isBusy = true;
    std::fill(back_buf->data.get(), back_buf->data.get() + (width * height), BGColor());
    AWindow::Draw((void*) back_buf->data.get());
    xcb_put_image(conn, XCB_IMAGE_FORMAT_Z_PIXMAP, back_buf->pixmap, mGC, static_cast<uint16_t>(width),
        static_cast<uint16_t>(height), 0, 0, 0, 32,// Handles 16, 24, or 32-bit visual depths cleanly
        static_cast<uint32_t>(back_buf->size), reinterpret_cast<const uint8_t*>(back_buf->data.get()));
    xcb_copy_area(conn, back_buf->pixmap, mWindowId, mGC, 0, 0, 0, 0, static_cast<uint16_t>(width),
        static_cast<uint16_t>(height));
    xcb_flush(conn);
    back_buf->isBusy = false;
  }

  void XCBWindowContext::PrepareSync() {
    xcb_connection_t* conn = EnginePtr()->X11Connection();
    xcb_sync_initialize_cookie_t sync_init_cookie = xcb_sync_initialize(conn, 3, 0);
    xcb_sync_initialize_reply_t* sync_init_reply = xcb_sync_initialize_reply(conn, sync_init_cookie, NULL);
    if(!sync_init_reply) {
      E("Failed to initialize XSync extension on this connection");
      return;
    }
    free(sync_init_reply);
// 1. Fetch all 4 required atoms cleanly in one place
    xcb_intern_atom_cookie_t sync_cookie = xcb_intern_atom(conn, 0, 20, "_NET_WM_SYNC_REQUEST");
    xcb_intern_atom_cookie_t counter_cookie = xcb_intern_atom(conn, 0, 28, "_NET_WM_SYNC_REQUEST_COUNTER");
    xcb_intern_atom_cookie_t protocols_cookie = xcb_intern_atom(conn, 0, 12, "WM_PROTOCOLS");
    xcb_intern_atom_cookie_t delete_cookie = xcb_intern_atom(conn, 0, 16, "WM_DELETE_WINDOW");
    xcb_intern_atom_reply_t* sync_reply = xcb_intern_atom_reply(conn, sync_cookie, NULL);
    xcb_intern_atom_reply_t* counter_reply = xcb_intern_atom_reply(conn, counter_cookie, NULL);
    xcb_intern_atom_reply_t* protocols_reply = xcb_intern_atom_reply(conn, protocols_cookie, NULL);
    xcb_intern_atom_reply_t* delete_reply = xcb_intern_atom_reply(conn, delete_cookie, NULL);
    if(!sync_reply || !counter_reply || !protocols_reply || !delete_reply) {
      E("Failed to intern Sync atoms");
      free(sync_reply);
      free(counter_reply);
      free(protocols_reply);
      free(delete_reply);
      return;
    }
    mSyncRequestAtom = sync_reply->atom;
    mWmProtocolsAtom = protocols_reply->atom;
    mWmDeleteWindowAtom = delete_reply->atom;
// 2. Create XSync counter
    xcb_sync_int64_t initial_val { };
    xcb_sync_counter_t counter = xcb_generate_id(conn);
    xcb_sync_create_counter(conn, counter, initial_val);
    mSyncCounter = counter;
    uint32_t counter_prop[2] = { static_cast<uint32_t>(counter), 0 };
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, mWindowId, counter_reply->atom, XCB_ATOM_CARDINAL, 32, 2,
        counter_prop);
// 4. Safely apply both WM_PROTOCOLS
    xcb_atom_t protocols[2] = { mWmDeleteWindowAtom, mSyncRequestAtom };
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, mWindowId, mWmProtocolsAtom, XCB_ATOM_ATOM, 32, 2, protocols);
// Flush the changes to the XServer before mapping happens
    xcb_flush(conn);
    free(sync_reply);
    free(counter_reply);
    free(protocols_reply);
    free(delete_reply);
  }

  bool XCBWindowContext::CreateFrame() {
    D2("CreateFrame entry: width={}, height={}, title='{}'", SizeX(), SizeY(), Title());
    CreateBuffers();
    AUI* au = EnginePtr();
    UNUSED xcb_connection_t* conn = au->X11Connection();
    UNUSED xcb_screen_t* screen = au->X11Screen();
    D2("conn={}, screen={}", static_cast<void*>(conn), static_cast<void*>(screen));
    if(!conn || !screen) {
      E("xcb conn or screen null");
    }
    mWindowId = xcb_generate_id(conn);
// ==========================================
// 1. Find the 32-bit (ARGB) Visual and Depth
// ==========================================
    xcb_visualid_t visual_id = screen->root_visual;
    uint8_t depth = 32;// We explicitly want 32-bit depth
    bool found_32bit = false;
    xcb_depth_iterator_t depth_iter = xcb_screen_allowed_depths_iterator(screen);
    for(; depth_iter.rem; xcb_depth_next(&depth_iter)) {
      if(depth_iter.data->depth == 32) {
        xcb_visualtype_iterator_t visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
        if(visual_iter.rem) {
          visual_id = visual_iter.data->visual_id;
          found_32bit = true;
          break;
        }
      }
    }
// Fallback if the user's X server doesn't support a 32-bit depth
    if(!found_32bit) {
      D1("Warning: 32-bit ARGB visual not found. Falling back to default root visual.");
      depth = XCB_COPY_FROM_PARENT;
      visual_id = screen->root_visual;
    }
// ==========================================
// 2. Create a Colormap matching the 32-bit visual
// ==========================================
    xcb_colormap_t colormap = XCB_NONE;
    if(found_32bit) {
      colormap = xcb_generate_id(conn);
      xcb_create_colormap(conn, XCB_COLORMAP_ALLOC_NONE, colormap, screen->root, visual_id);
    }
// ==========================================
// 3. Set up Window Attributes (Mask & Values)
// ==========================================
// Crucial: When using a non-standard visual, you MUST supply a matching colormap and border pixel.
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL | XCB_CW_EVENT_MASK;
    if(found_32bit) {
      mask |= XCB_CW_COLORMAP;
    }
    uint32_t values[4];
    int32_t val_idx = 0;
    values[val_idx++] = AUI_DEFAULT_WINDOW_BG;// Transparent or alpha-supported background color
    values[val_idx++] = 0;// XCB_CW_BORDER_PIXEL (Required for custom visuals)
    values[val_idx++] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
        | XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE
        | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_POINTER_MOTION;
    if(found_32bit) {
      values[val_idx++] = colormap;// XCB_CW_COLORMAP
    }
// ==========================================
// 4. Create the Window with the 32-bit Configuration
// ==========================================
    xcb_void_cookie_t create_cookie = xcb_create_window_checked(conn,
        depth,// depth (32 instead of XCB_COPY_FROM_PARENT)
        mWindowId, screen->root, 0, 0, static_cast<uint16_t>(SizeX()), static_cast<uint16_t>(SizeY()), 0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT, visual_id,// 32-bit visual_id instead of screen->root_visual
        mask, values);

    xcb_generic_error_t* err = xcb_request_check(conn, create_cookie);
    if(err) {
// Log the EXACT problem before E() halts execution
      E("CRITICAL: xcb_create_window failed! Protocol Error: %d", err->error_code);
    }
    mGC = xcb_generate_id(conn);
    uint32_t gc_value = 0;
    xcb_void_cookie_t gc_cookie = xcb_create_gc_checked(conn, mGC, mWindowId, XCB_GC_FOREGROUND, &gc_value);
    err = xcb_request_check(conn, gc_cookie);
    if(err) {
      E("xcb_create_gc failed: code={}", err->error_code);
      free(err);
    }
    mKeySymbols = xcb_key_symbols_alloc(conn);
    if(!mKeySymbols) {
      D1("XCB: Failed to allocate key symbols – key translation limited");
    }
// Initialize XKB for Unicode conversion
    mXkbCtx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if(mXkbCtx) {
      uint16_t req_major = 1, req_minor = 0;
      uint16_t got_major = 0, got_minor = 0;
      if(xkb_x11_setup_xkb_extension(conn, req_major, req_minor, XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS, &got_major,
          &got_minor, nullptr, nullptr)) {
        int32_t device_id = xkb_x11_get_core_keyboard_device_id(conn);
        if(device_id != -1) {
          mXkbKeymap = xkb_x11_keymap_new_from_device(mXkbCtx, conn, device_id, XKB_KEYMAP_COMPILE_NO_FLAGS);
          if(mXkbKeymap)
            mXkbState = xkb_state_new(mXkbKeymap);
        }
      }
      if(!mXkbKeymap) {
        struct xkb_rule_names names = { };
        names.layout = "us";
        mXkbKeymap = xkb_keymap_new_from_names(mXkbCtx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
        if(mXkbKeymap)
          mXkbState = xkb_state_new(mXkbKeymap);
      }
    }
    if(!mXkbState) {
      D1("XCB: XKB state unavailable – only keysyms will work");
    }

    PrepareSync();
    UpdateSizeHints();
    xcb_void_cookie_t map_cookie = xcb_map_window_checked(conn, mWindowId);
    err = xcb_request_check(conn, map_cookie);
    if(err) {
      E("xcb_map_window failed: code=%d", err->error_code);
      free(err);
      return false;
    }
    D2("before xcb_flush")
    if(xcb_connection_has_error(conn)) {
      E("XCB connection has error before flush");
    }
    else {
      UNUSED int32_t ret = xcb_flush(conn);
      D2("xcb_flush returned {}", ret);
    }
    return true;
  }

  uint64_t XCBWindowContext::NativeWindowId() const {
    return mWindowId;
  }

  void XCBWindowContext::CreateBuffers() {
    D2("XCB buffer creation")
    AUI* au = EnginePtr();
    xcb_connection_t* conn = au->X11Connection();
    xcb_screen_t* screen = au->X11Screen();
    if(!conn || !screen) {
      E("XCB connection or screen is null");
      return;
    }
    for(size_t i = 0; i < AUI_NUM_BUFFERS; ++i) {
      if(mBuffers[i]) {
// Cast base class pointer to backend-specific class
        if(auto* xcbBuf = dynamic_cast<XCBBuffer*>(mBuffers[i].get())) {
          if(xcbBuf->pixmap) {
            xcb_free_pixmap(conn, xcbBuf->pixmap);
            xcbBuf->pixmap = 0;
          }
        }
      }
    }
    int32_t width = SafeINT32(SizeX());
    int32_t height = SafeINT32(SizeY());
    if(width <= 0 || height <= 0) {
      E("Invalid window size: {}x{}", width, height);
      return;
    }
    size_t data_size = static_cast<size_t>(width) * (size_t) height * 4;// 4 bytes per pixel
    for(size_t i = 0; i < AUI_NUM_BUFFERS; ++i) {
      auto buf = std::make_unique<XCBBuffer>();
// Allocate client‑side pixel memory
      buf->data = std::make_unique<uint32_t[]>((size_t) (width * height));
      buf->size = data_size;
// Create a pixmap of the same size and depth as the screen
      buf->pixmap = xcb_generate_id(conn);
      xcb_create_pixmap(conn, 32,// depth (typically 24 or 32)
          buf->pixmap, screen->root, static_cast<uint16_t>(width), static_cast<uint16_t>(height));
// Initially fill with background color (optional)
      std::fill(buf->data.get(), buf->data.get() + width * height, BGColor());
      buf->isBusy = false;
// Store in the buffer array
      mBuffers[i] = std::move(buf);
    }
    D2("XCB buffers created: %d buffers of size %zu bytes", AUI_NUM_BUFFERS, data_size);
  }

  void XCBWindowContext::UpdateDecorations() {
    AUI* au = EnginePtr();
    xcb_connection_t* conn = au->X11Connection();
    if(!conn || !mWindowId) {
      D1("Cannot update decorations: no connection or window");
      return;
    }
    xcb_intern_atom_cookie_t cookie_motif = xcb_intern_atom(conn, 0, 15, "_MOTIF_WM_HINTS");
    xcb_intern_atom_cookie_t cookie_type = xcb_intern_atom(conn, 0, 19, "_NET_WM_WINDOW_TYPE");
    xcb_intern_atom_cookie_t cookie_normal = xcb_intern_atom(conn, 0, 24, "_NET_WM_WINDOW_TYPE_NORMAL");
    xcb_intern_atom_cookie_t cookie_splash = xcb_intern_atom(conn, 0, 24, "_NET_WM_WINDOW_TYPE_SPLASH");
    xcb_intern_atom_reply_t* reply_motif = xcb_intern_atom_reply(conn, cookie_motif, nullptr);
    xcb_intern_atom_reply_t* reply_type = xcb_intern_atom_reply(conn, cookie_type, nullptr);
    xcb_intern_atom_reply_t* reply_normal = xcb_intern_atom_reply(conn, cookie_normal, nullptr);
    xcb_intern_atom_reply_t* reply_splash = xcb_intern_atom_reply(conn, cookie_splash, nullptr);
    if(!reply_motif || !reply_type || !reply_normal || !reply_splash) {
      D1("Failed to intern EWMH/Motif atoms");
      free(reply_motif);
      free(reply_type);
      free(reply_normal);
      free(reply_splash);
      return;
    }
    xcb_atom_t atom_motif = reply_motif->atom;
    xcb_atom_t atom_type = reply_type->atom;
    xcb_atom_t atom_normal = reply_normal->atom;
    xcb_atom_t atom_splash = reply_splash->atom;
    free(reply_motif);
    free(reply_type);
    free(reply_normal);
    free(reply_splash);
    uint32_t hints_data[5] = { 0 };
    hints_data[0] = static_cast<uint32_t>(MWM_HINTS_DECORATIONS);
    hints_data[2] = mDecorations ? static_cast<uint32_t>(MWM_DECOR_ALL) : 0u;
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, mWindowId, atom_motif, atom_motif, 32, 5, hints_data);
    xcb_atom_t chosen_type = mDecorations ? atom_normal : atom_splash;
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, mWindowId, atom_type, XCB_ATOM_ATOM, 32, 1, &chosen_type);
    xcb_flush(conn);
    D1("Decorations updated: {}", mDecorations ? "enabled" : "disabled");
  }

  void XCBWindowContext::ProcessEvent(xcb_generic_event_t* ev) {
    ST2("")
    AUI* au = EnginePtr();
    xcb_connection_t* conn = au->X11Connection();
    const uint8_t type = ev->response_type & 0x7FU;
    switch(type) {
      case XCB_EXPOSE: {
        RequestRedraw();
        break;
      }
      case XCB_CLIENT_MESSAGE: {
        auto* msg = reinterpret_cast<xcb_client_message_event_t*>(ev);
        if(msg->data.data32[0] == mSyncRequestAtom) {// _NET_WM_SYNC_REQUEST
// Extract the 64-bit serial number
          uint32_t serial_low = msg->data.data32[2];
          uint32_t serial_high = msg->data.data32[3];
          uint64_t serial = (static_cast<uint64_t>(serial_high) << 32) | serial_low;
          D2("Sync request received, serial=%llu", serial);
// Store it for use after ConfigureNotify
          mPendingSyncSerial = serial;
          mSyncPending = true;
        }
        else {
          D3("msg->data.data32[0] != mSyncRequestAtom, values {}, {}", msg->data.data32[0], mSyncRequestAtom)
        }
        if(msg->data.data32[0] == mWmDeleteWindowAtom) {
          if(EnginePtr()->MainWnd() == this) {
            EnginePtr()->ExitAUI();
          }
          else {
            Close();
          }
        }
        break;
      }
      case XCB_CONFIGURE_NOTIFY: {
        auto* cfg = reinterpret_cast<xcb_configure_notify_event_t*>(ev);
        if(cfg->width != SizeX() || cfg->height != SizeY()) {
          SizeX(cfg->width);
          SizeY(cfg->height);
          CreateBuffers();// Recreate at new size
          Draw();// Draw the new content
// If this ConfigureNotify was preceded by a sync request, acknowledge it
          if(mSyncPending) {
// Set counter to the serial number (even value = "drawing complete")
            xcb_sync_int64_t sync_val;
            sync_val.hi = static_cast<int32_t>(mPendingSyncSerial >> 32);
            sync_val.lo = static_cast<uint32_t>(mPendingSyncSerial & 0xFFFFFFFF);
// 2. Pass the struct to the function
            xcb_sync_set_counter(conn, mSyncCounter, sync_val);
            xcb_flush(conn);
            mSyncPending = false;
          }
        }
        break;
      }
      case XCB_MOTION_NOTIFY: {
        auto* motion = reinterpret_cast<xcb_motion_notify_event_t*>(ev);
        D4("Mouse motion at ({},{})", motion->event_x, motion->event_y);
        OnMouseMove(motion->event_x, motion->event_y);
        break;
      }
      case XCB_BUTTON_PRESS: {
        D4("XCB_BUTTON_PRESS");
        UNUSED auto* btn = reinterpret_cast<xcb_button_press_event_t*>(ev);
        switch(btn->detail) {
          case 1:
            OnMousePress(btn->event_x, btn->event_y, BTN_LEFT);
            break;
          default:
            D("unhandled button {} press", btn->detail)
        }
      }
        break;
      case XCB_BUTTON_RELEASE: {
        D4("XCB_BUTTON_RELEASE");
//        E("unimplemented")
        auto* btn = reinterpret_cast<xcb_button_release_event_t*>(ev);
        switch(btn->detail) {
          case 1:
            OnMouseRelease(btn->event_x, btn->event_y, BTN_LEFT);
            break;
          default:
            D("unhandled button {} release", btn->detail)
        }
      }
        break;
      case XCB_KEY_PRESS:
      case XCB_KEY_RELEASE: {
        auto* key = reinterpret_cast<xcb_key_press_event_t*>(ev);
        if(!key)
          break;
        if(key->detail < 8) {
          D1("XCB: low keycode {}", key->detail);
        }
        if(!mKeySymbols) {
          D1("XCB: key translation not initialized");
          break;
        }
// 1. Get keysyms (base for code mapping, active for printable)
        xcb_keysym_t base_keysym = xcb_key_symbols_get_keysym(mKeySymbols, key->detail, 0);
        int32_t col = (key->state & XCB_MOD_MASK_SHIFT) ? 1 : 0;
        xcb_keysym_t active_keysym = xcb_key_symbols_get_keysym(mKeySymbols, key->detail, col);
// 2. Map to internal key code (for control keys)
        AUIKeyCode code = translate_keysym_to_keycode(base_keysym);
// 3. Build the event
        AUIKeyEvent keyEvent;
        keyEvent.pressed = (type == XCB_KEY_PRESS);
        keyEvent.modifiers = translate_modifiers(key->state);
        keyEvent.code = code;
// 4. Compute Unicode from the active keysym (ONCE, no arithmetic!)
        uint32_t unicode = 0;
        if(active_keysym >= 0x20 && active_keysym <= 0x7E) {
// ASCII: keysym is the Unicode codepoint
          unicode = static_cast<uint32_t>(active_keysym);
        }
        else
          if((active_keysym & 0xFF000000) == 0x01000000) {
// Extended Unicode (X11 stores in lower 24 bits)
            unicode = active_keysym & 0x00FFFFFF;
          }
          else {
// For non‑ASCII (e.g., Cyrillic, Greek) use xkbcommon as fallback
            char utf8[8] = { 0 };
            int32_t len = xkb_keysym_to_utf8(active_keysym, utf8, sizeof(utf8));
            if(len > 0) {
              auto* u = reinterpret_cast<uint8_t*>(utf8);
              if(len == 1)
                unicode = u[0];
              else
                if(len == 2)
                  unicode = ((u[0] & 0x1F) << 6) | (u[1] & 0x3F);
                else
                  if(len == 3)
                    unicode = ((u[0] & 0x0F) << 12) | ((u[1] & 0x3F) << 6) | (u[2] & 0x3F);
                  else
                    if(len == 4)
                      unicode = ((u[0] & 0x07) << 18) | ((u[1] & 0x3F) << 12) | ((u[2] & 0x3F) << 6) | (u[3] & 0x3F);
            }
// If all else fails, unicode stays 0 (non‑printable)
          }
        keyEvent.unicode = unicode;
        D1("sending event: code {}, unicode {:x}", static_cast<int32_t>(keyEvent.code), keyEvent.unicode);
        OnKeyEvent(keyEvent);
        break;
      }
      case XCB_MAP_NOTIFY:
        D3("XCB_MAP_NOTIFY not implemented")
        break;
      default:
        D("Unknown event type: {} ({})", static_cast<int32_t>(type), XCB_EventTypeToString(type))
        break;
    }
  }

  void XCBWindowContext::BackendResize(uint32_t width, uint32_t height) {
    if(width == 0 || height == 0) {
      D1("Invalid resize dimensions: {}x{}", width, height);
      return;
    }
    if(width == SizeX() && height == SizeY()) {
      D2("Resize skipped – size already {}x{}", width, height);
      return;
    }
    SizeX(width);
    SizeY(height);
    CreateBuffers();
    AUI* au = EnginePtr();
    xcb_connection_t* conn = au->X11Connection();
    if(!conn || !mWindowId) {
      E("XCB connection or window invalid");
      return;
    }
    uint32_t values[2] = { width, height };
    xcb_configure_window(conn, mWindowId, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
    UpdateSizeHints();
    xcb_flush(conn);
    D4("Resize requested: {}x{}", width, height);
  }

  void XCBWindowContext::BackendMove(int32_t x, int32_t y) {
    AUI* au = EnginePtr();
    xcb_connection_t* conn = au->X11Connection();
    if(!conn || !mWindowId) {
      E("XCB connection or window invalid");
      return;
    }
    uint32_t values[2] = { static_cast<uint32_t>(x), static_cast<uint32_t>(y) };
    xcb_configure_window(conn, mWindowId, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
    xcb_flush(conn);
    D1("Window moved to ({}, {})", x, y);
  }

  void XCBWindowContext::BackendTitle(UNUSED std::string title) {
    AUI* au = EnginePtr();
    if(!au) return;
    xcb_connection_t *conn = au->X11Connection();
    if(!conn || !mWindowId) {E("not initialized")}
    xcb_void_cookie_t cookie = xcb_change_property_checked(conn, XCB_PROP_MODE_REPLACE,
      mWindowId, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
      static_cast<uint32_t>(title.size()), title.c_str());
    xcb_generic_error_t *err = xcb_request_check(conn, cookie);
    if (err) {
      E("xcb_change_property (title) failed: code={}", err->error_code);
    }
  }

  void XCBWindowContext::UpdateSizeHints() {
    if(!mWindowId)
      return;
    AUI* au = EnginePtr();
    xcb_connection_t* conn = au->X11Connection();
    if(!conn) {
      E("XCB connection is null");
      return;
    }
// Intern WM_NORMAL_HINTS (cache if desired)
    xcb_intern_atom_cookie_t hints_cookie = xcb_intern_atom(conn, 0, 15, "WM_NORMAL_HINTS");
    xcb_intern_atom_reply_t* hints_reply = xcb_intern_atom_reply(conn, hints_cookie, nullptr);
    if(!hints_reply) {
      E("Failed to intern WM_NORMAL_HINTS");
      return;
    }
    uint32_t min_w, min_h, max_w, max_h;
    if(mResizeEnabled) {
// Allow resizing within a large range
      min_w = 1;
      min_h = 1;
      max_w = 0x7fffffff;
      max_h = 0x7fffffff;
    }
    else {
// Lock to current size
      min_w = max_w = static_cast<uint32_t>(SizeX());
      min_h = max_h = static_cast<uint32_t>(SizeY());
    }
// Build hints array (18 x uint32_t as per ICCCM)
    uint32_t hints[18] = { 0 };
    hints[0] = 48U;// PMinSize | PMaxSize
    hints[5] = min_w;
    hints[6] = min_h;
    hints[7] = max_w;
    hints[8] = max_h;
    xcb_void_cookie_t cookie = xcb_change_property_checked(conn, XCB_PROP_MODE_REPLACE, mWindowId, hints_reply->atom,
        XCB_ATOM_WM_SIZE_HINTS,// correct type
        32, 18, hints);
    xcb_generic_error_t* err = xcb_request_check(conn, cookie);
    if(err) {
      E("xcb_change_property (size hints) failed: code=%d", err->error_code);
      free(err);
    }
    free(hints_reply);
    xcb_flush(conn);
  }

  void XCBWindowContext::BackendDisableResize() {
    D()
    mResizeEnabled = false;
    UpdateSizeHints();
  }

  void XCBWindowContext::BackendEnableResize() {
    mResizeEnabled = true;
    UpdateSizeHints();
  }

  void XCBWindowContext::BackendCursor(UNUSED AUICursorType type) {
    AUI* au;
    if(!(au = EnginePtr())) {E("engine ptr is null");}
    xcb_connection_t* conn = au->X11Connection();
    if(!conn || xcb_connection_has_error(conn)) { E("invalid or errored connection");}
    if(!mWindowId) {E("window already destroyed");}
    if(!mCursorContext) {
      xcb_screen_t* screen = au->X11Screen();
      if(!screen) {
        D1("XcbWindowContext::SetCursor: no screen");
        return;
      }
      if(xcb_cursor_context_new(conn, screen, &mCursorContext) != 0) {
        D1("XcbWindowContext::SetCursor: failed to create cursor context");
        mCursorContext = nullptr;
        return;
      }
    }
    const char* name = "left_ptr";
    switch (type) {
      case AUICursorType::HResize: name = "ew-resize"; break;
      case AUICursorType::VResize: name = "ns-resize"; break;
      default: name = "left_ptr"; break;
    }
    xcb_cursor_t new_cursor = xcb_cursor_load_cursor(mCursorContext, name);
    if(!new_cursor) {
      D1("XcbWindowContext::SetCursor: failed to load cursor '%s'", name);
      return;
    }
    xcb_void_cookie_t cookie = xcb_change_window_attributes_checked(conn, mWindowId, XCB_CW_CURSOR, &new_cursor);
    xcb_generic_error_t* err = xcb_request_check(conn, cookie);
    if(err) {
      E("xcb_change_window_attributes (cursor) failed: code=%d", err->error_code);
      free(err);
      xcb_free_cursor(conn, new_cursor);
      return;
    }
    if(mCurrentCursor != 0 && mCurrentCursor != new_cursor) {
      if(!xcb_connection_has_error(conn)) {
        xcb_free_cursor(conn, mCurrentCursor);
      } else {
        D1("XcbWindowContext::SetCursor: connection error, cannot free old cursor");
      }
    }
    mCurrentCursor = new_cursor;
    RequestRedraw();
  }

  XCBWindowContext::~XCBWindowContext() {
    if(mCursorContext) {
      xcb_cursor_context_free(mCursorContext);
      mCursorContext = nullptr;
    }
    if(mWindowId) {
      xcb_connection_t* conn = EnginePtr()->X11Connection();
      xcb_destroy_window(conn, mWindowId);
      xcb_flush(conn);
    }
    else {
      E("mWindowId unkn1own")
    }
    if(mSyncCounter) {
      xcb_sync_destroy_counter(EnginePtr()->X11Connection(), mSyncCounter);
      mSyncCounter = 0;
    }
    if(mKeySymbols) {
      xcb_key_symbols_free(mKeySymbols);
      mKeySymbols = nullptr;
    }
    if(mXkbState) {
      xkb_state_unref(mXkbState);
      mXkbState = nullptr;
    }
    if(mXkbKeymap) {
      xkb_keymap_unref(mXkbKeymap);
      mXkbKeymap = nullptr;
    }
    if(mXkbCtx) {
      xkb_context_unref(mXkbCtx);
      mXkbCtx = nullptr;
    }
  }
}
