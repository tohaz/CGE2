#include "AUILib.h"

namespace aui {

  XcbWindowContext::XcbWindowContext(AUI *aui, AWindow *window)
    : mSoftwareBuffer(std::make_shared<std::vector<uint32_t>>()){
    mWindow = window;
    mAUI = aui;
    D3("w {} aui {}", (int64_t)mWindow, (int64_t)mAUI)
  }

  bool XcbWindowContext::CreateFrame(uint32_t width, uint32_t height, const std::string &title) {
    D2("CreateFrame entry: width={}, height={}, title='{}'", width, height, title);
    if(!mAUI) {
      DT("mAUI is null");
      return false;
    }
    xcb_connection_t *conn = mAUI->GetXcbConnection();
    xcb_screen_t *screen = mAUI->GetXcbScreen();
    D2("conn={}, screen={}", static_cast<void*>(conn), static_cast<void*>(screen));
    if(!conn || !screen) {
      DT("conn or screen null");
      return false;
    }
    mWindowId = xcb_generate_id(conn);
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[2] = {
    AUI_DEFAULT_WINDOW_BG, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_BUTTON_PRESS
        | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE
        | XCB_EVENT_MASK_POINTER_MOTION };
    xcb_void_cookie_t create_cookie = xcb_create_window_checked(conn, XCB_COPY_FROM_PARENT, mWindowId, screen->root, 0, 0, static_cast<uint16_t>(width),
        static_cast<uint16_t>(height), 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, values);
    xcb_generic_error_t *err = xcb_request_check(conn, create_cookie);
    if(err) {
      E("xcb_create_window failed: code=%d", err->error_code);
      free(err);
      return false;
    }
    SetTitle(title);
    mGC = xcb_generate_id(conn);
    uint32_t gc_value = 0;
    xcb_void_cookie_t gc_cookie = xcb_create_gc_checked(conn, mGC, mWindowId, XCB_GC_FOREGROUND,
        &gc_value);
    err = xcb_request_check(conn, gc_cookie);
    if(err) {
      E("xcb_create_gc failed: code=%d", err->error_code);
      free(err);
      return false;
    }
// WM_DELETE_WINDOW protocol
    xcb_intern_atom_cookie_t protocols_cookie = xcb_intern_atom(conn, 0, 12, "WM_PROTOCOLS");
    xcb_intern_atom_cookie_t delete_cookie = xcb_intern_atom(conn, 0, 16, "WM_DELETE_WINDOW");
    xcb_intern_atom_reply_t *protocols_reply = xcb_intern_atom_reply(conn, protocols_cookie, nullptr);
    xcb_intern_atom_reply_t *delete_reply = xcb_intern_atom_reply(conn, delete_cookie, nullptr);
    if(protocols_reply && delete_reply) {
      mWmProtocolsAtom = protocols_reply->atom;
      mWmDeleteWindowAtom = delete_reply->atom;
      D2("WM_PROTOCOLS atom set to: {}", mWmProtocolsAtom);
      D2("WM_DELETE_WINDOW atom set to: {}", mWmDeleteWindowAtom);
      xcb_void_cookie_t prop_cookie = xcb_change_property_checked(conn, XCB_PROP_MODE_REPLACE, mWindowId, mWmProtocolsAtom, XCB_ATOM_ATOM, 32, 1,
          &mWmDeleteWindowAtom);
      err = xcb_request_check(conn, prop_cookie);
      if (err) {
        E("xcb_change_property for WM_PROTOCOLS failed: code=%d", err->error_code);
        free(err);
      }
      free(protocols_reply);
      free(delete_reply);
    }
    else {
      DT("Failed to intern WM_DELETE_WINDOW atom (protocols_reply={}, delete_reply={})", (void*)protocols_reply,
          (void*)delete_reply);
      if(protocols_reply)
        free(protocols_reply);
      if(delete_reply)
        free(delete_reply);
      E("XCB Fatal: Failed to retrieve window manager protocol atoms from server.");
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
    xcb_void_cookie_t map_cookie = xcb_map_window_checked(conn, mWindowId);
    err = xcb_request_check(conn, map_cookie);
    if (err) {
      E("xcb_map_window failed: code=%d", err->error_code);
      free(err);
      return false;
    }
    D2("before xcb_flush")
    if (xcb_connection_has_error(conn)) {
        E("XCB connection has error before flush");
    } else {
        UNUSED int32_t ret = xcb_flush(conn);
        D2("xcb_flush returned %d", ret);
    }
    D2("Before buffer resize: size = %zu", mSoftwareBuffer->size());
    mSoftwareBuffer->resize(width * height, 0xFFAAAAAA);
    D2("After buffer resize");
    return true;
  }

  void XcbWindowContext::Move(int32_t x, int32_t y) {
    if(!mAUI)
      return;
    xcb_connection_t *conn = mAUI->GetXcbConnection();
    if(!conn || !mWindowId)
      return;
    uint32_t values[2] = { static_cast<uint32_t>(x), static_cast<uint32_t>(y) };
    xcb_void_cookie_t cookie = xcb_configure_window_checked(conn, mWindowId, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
    xcb_generic_error_t *err = xcb_request_check(conn, cookie);
    if (err) {
      E("xcb_configure_window (Move) failed: code=%d", err->error_code);
      free(err);
    }
    mWindow->Draw();
  }

  void XcbWindowContext::Resize(uint32_t width, uint32_t height) {
    if(!mAUI)
      return;
    if(mSizeX == width && mSizeY == height)
      return;
    xcb_connection_t *conn = mAUI->GetXcbConnection();
    if(!conn || !mWindowId)
      return;
    mSizeX = width;
    mSizeY = height;
    mSoftwareBuffer->resize(width * height, 0);
    uint32_t values[2] = { width, height };
    xcb_void_cookie_t cookie = xcb_configure_window_checked(conn, mWindowId, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
    xcb_generic_error_t *err = xcb_request_check(conn, cookie);
    if (err) {
      E("xcb_configure_window (Resize) failed: code=%d", err->error_code);
      free(err);
    }
    if(mWindow && !mWindow->IsResizeEnabled()) {
      ApplySizeHints(width, height, width, height);
    }
///    xcb_flush(conn);
  }

  void XcbWindowContext::SetTitle(const std::string &title) {
    if(!mAUI)
      return;
    xcb_connection_t *conn = mAUI->GetXcbConnection();
    if(!conn || !mWindowId)
      return;
    xcb_void_cookie_t cookie = xcb_change_property_checked(conn, XCB_PROP_MODE_REPLACE, mWindowId, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
        static_cast<uint32_t>(title.size()), title.c_str());
    xcb_generic_error_t *err = xcb_request_check(conn, cookie);
    if (err) {
      E("xcb_change_property (title) failed: code=%d", err->error_code);
      free(err);
    }
  }

  uint32_t* XcbWindowContext::GetSoftwareBuffer() {
    return mSoftwareBuffer->data();
  }

  void XcbWindowContext::QueueFrameCommit() {
      if (!mAUI) return;
      // Remove any stale command for this window (ensures only one command exists)
      mAUI->ClearDrawCommandsForWindow(mWindowId);   // <-- ADD THIS LINE
      DrawCommand cmd;
      cmd.type = DrawCommandType::Xcb;
      cmd.xcb.windowId = mWindowId;
      cmd.xcb.width = mSizeX;
      cmd.xcb.height = mSizeY;
      cmd.xcb.buffer = mSoftwareBuffer->data();
      mAUI->EnqueueDrawCommand(cmd);
  }

  void XcbWindowContext::ProcessEvent(void *ev) {
    xcb_generic_event_t *event = static_cast<xcb_generic_event_t*>(ev);
    uint8_t type = event->response_type & 0x7F;
    D3("XcbWindowContext::ProcessEvent: type={}", (int32_t)type);
    switch (type) {
      case XCB_EXPOSE: {
        D2("Expose event, redrawing window");
        if(mWindow)
          mWindow->Draw();
        break;
      }
      case XCB_MOTION_NOTIFY: {
        auto *motion = reinterpret_cast<xcb_motion_notify_event_t*>(event);
        D2("Mouse motion at ({},{})", motion->event_x, motion->event_y);
        if(mWindow)
          mWindow->OnMouseMove(motion->event_x, motion->event_y);
        break;
      }
      case XCB_BUTTON_PRESS: {
        auto *btn = reinterpret_cast<xcb_button_press_event_t*>(event);
        if(mWindow) {
          if(btn->detail == 4) {// scroll up
            mWindow->OnMouseWheel(1);
          }
          else if(btn->detail == 5) {// scroll down
            mWindow->OnMouseWheel(-1);
          }
          else {
            mWindow->OnMousePress(btn->event_x, btn->event_y, btn->detail);
          }
        }
        break;
      }
      case XCB_BUTTON_RELEASE: {
        auto *btn = reinterpret_cast<xcb_button_release_event_t*>(event);
        D2("Button release {} at ({},{})", (int32_t)btn->detail, btn->event_x, btn->event_y);
        if(mWindow) {
          mWindow->OnMouseRelease(btn->event_x, btn->event_y, btn->detail);
        }
        break;
      }
      case XCB_KEY_PRESS:
      case XCB_KEY_RELEASE: {
        auto *key = reinterpret_cast<xcb_key_press_event_t*>(event);
        if(!key)
          break;
        if(key->detail < 8) {
          D1("XCB: low keycode {}", key->detail);
        }
        if(!mKeySymbols || !mXkbState) {
          D1("XCB: key translation not initialized");
          break;
        }
        xcb_keysym_t keysym = xcb_key_symbols_get_keysym(mKeySymbols, key->detail, 0);
        if(keysym == XCB_NO_SYMBOL) {
          D1("XCB: no keysym for keycode {}", key->detail);
          break;
        }
        AUIKeyEvent keyEvent;
        keyEvent.pressed = (type == XCB_KEY_PRESS);
        keyEvent.modifiers = translate_modifiers(key->state);
// *** PRIORITY: Check for known non‑printable key codes FIRST ***
        AUIKeyCode code = translate_keysym_to_keycode(keysym);
        if(code != AUIKeyCode::None) {
          keyEvent.code = code;
          keyEvent.unicode = 0;
        }
        else {
// Not a known control key – treat as printable (if any)
          char utf8[8] = { 0 };
          int32_t len = xkb_keysym_to_utf8(keysym, utf8, sizeof(utf8));
          if (len > 0) {
            // Decode up to 4 bytes into a single UTF-32 character scalar
            uint32_t cp = 0;
            auto* u = reinterpret_cast<uint8_t*>(utf8);
            if (len == 1)      cp = u[0];
            else if (len == 2) cp = ((u[0] & 0x1F) << 6)  | (u[1] & 0x3F);
            else if (len == 3) cp = ((u[0] & 0x0F) << 12) | ((u[1] & 0x3F) << 6)  | (u[2] & 0x3F);
            else if (len == 4) cp = ((u[0] & 0x07) << 18) | ((u[1] & 0x3F) << 12) | ((u[2] & 0x3F) << 6) | (u[3] & 0x3F);
            keyEvent.unicode = cp;
            keyEvent.code = AUIKeyCode::None;
          }
          else {
// No printable representation – ignore
            keyEvent.unicode = 0;
            keyEvent.code = AUIKeyCode::None;
          }
        }
        if(mWindow) {
          mWindow->OnKeyEvent(keyEvent);
        }
        break;
      }
      case XCB_LEAVE_NOTIFY: {
        if(mWindow)
          mWindow->ClearHover();
        break;
      }
      case XCB_CLIENT_MESSAGE: {
        auto *msg = reinterpret_cast<xcb_client_message_event_t*>(event);
        D2("Client message atom: {}", msg->type);
        D2("mWmProtocolsAtom: {}", mWmProtocolsAtom);
        D2("mWmDeleteWindowAtom: {}", mWmDeleteWindowAtom);
        if(msg->type == mWmProtocolsAtom && static_cast<uint32_t>(msg->data.data32[0]) == mWmDeleteWindowAtom) {
          D2("WM_DELETE_WINDOW received, calling ExitAUI");
          if(mWindow)
            mWindow->Close();
        }
        else {
          DT("Atom mismatch, ignoring");
        }
        break;
      }
      case XCB_MAP_NOTIFY:
        mMapped = true;
        if(mWindow) {
          mWindow->OnMap();// resets pending and schedules a fresh draw
        }
        break;
      case XCB_UNMAP_NOTIFY:
        mMapped = false;
        break;
      case XCB_CONFIGURE_NOTIFY: {
        auto* cfg = reinterpret_cast<xcb_configure_notify_event_t*>(event);
        mSizeX = cfg->width;
        mSizeY = cfg->height;
        uint32_t new_size = static_cast<uint32_t>(cfg->width) * static_cast<uint32_t>(cfg->height);
        if(mSoftwareBuffer->size() != new_size) {
          if(mAUI) {
                std::lock_guard<std::recursive_mutex> lock(mAUI->GetCommandMutex());
                auto &commands = mAUI->GetDrawCommands();
                uint64_t nativeId = mWindowId;
                commands.erase(std::remove_if(commands.begin(), commands.end(), [nativeId](const DrawCommand &cmd) {
                  return cmd.type == DrawCommandType::Xcb && cmd.xcb.windowId == nativeId;
                }), commands.end());
              }
          mSoftwareBuffer->resize(new_size, 0xFFAAAAAA);
        }
        if(mWindow && mWindow->IsResizeEnabled()) {
          mWindow->Resize(cfg->width, cfg->height);
        }
        break;
      }
      default:
        D2("Unhandled event type: {}", (int32_t)type)
        ;
        break;
    }
  }

  void XcbWindowContext::ApplySizeHints(uint32_t min_w, uint32_t min_h, uint32_t max_w, uint32_t max_h) {
    D2("minw {} minh {}", min_w, min_h);
    if(!mAUI || !mWindowId) {
      E()
      return;
    }
    xcb_connection_t *conn = mAUI->GetXcbConnection();
    if(!conn) {
      E()
      return;
    }
    xcb_intern_atom_cookie_t hints_cookie = xcb_intern_atom(conn, 0, 15, "WM_NORMAL_HINTS");
    xcb_intern_atom_reply_t *hints_reply = xcb_intern_atom_reply(conn, hints_cookie, nullptr);
    if(!hints_reply) {
      E("Failed to intern WM_NORMAL_HINTS");
      return;
    }
    uint32_t hints[18] = { 0 };
    hints[0] = 48U;
    hints[5] = min_w;
    hints[6] = min_h;
    hints[7] = max_w;
    hints[8] = max_h;
    xcb_void_cookie_t cookie = xcb_change_property_checked(conn, XCB_PROP_MODE_REPLACE, mWindowId, hints_reply->atom, XCB_ATOM_WM_SIZE_HINTS, 32, 18,
        hints);
    xcb_generic_error_t *err = xcb_request_check(conn, cookie);
    if (err) {
      E("xcb_change_property (size hints) failed: code=%d", err->error_code);
      free(err);
    }
    free(hints_reply);
    xcb_flush(conn);
  }

  void XcbWindowContext::EnableResize() {
    ApplySizeHints(1, 1, 65535, 65535);
  }

  bool XcbWindowContext::EnsureBuffer(uint32_t width, uint32_t height) {
    size_t needed = static_cast<size_t>(width) * height;
    if(mSoftwareBuffer->size() != needed) {
      mSoftwareBuffer->resize(needed, 0xFFAAAAAA);
    }
    return true;
  }

  void XcbWindowContext::DisableResize() {
    uint32_t w = mWindow->SizeX();
    uint32_t h = mWindow->SizeY();
    ApplySizeHints(w, h, w, h);
  }

  void XcbWindowContext::SetCursor(AUICursorType type) {
    if(!mCursorContext) {
      xcb_connection_t *conn = mAUI->GetXcbConnection();
      xcb_screen_t *screen = mAUI->GetXcbScreen();
      if(!conn || !screen) {
        D1("XCB: No connection or screen");
        return;
      }
      if(xcb_cursor_context_new(conn, screen, &mCursorContext) != 0) {
        D1("XCB: Failed to create cursor context");
        mCursorContext = nullptr;
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
    xcb_cursor_t new_cursor = xcb_cursor_load_cursor(mCursorContext, name);
    if(!new_cursor) {
      D1("XCB: Failed to load cursor '{}'", name);
      return;
    }
    xcb_connection_t *conn = mAUI->GetXcbConnection();
    if(!conn) {
// Prevent server resource leak if connection drops unexpectedly
      xcb_free_cursor(conn, new_cursor);
      return;
    }
// Apply the fresh cursor handle to our native window context
    xcb_void_cookie_t cookie = xcb_change_window_attributes_checked(conn, mWindowId, XCB_CW_CURSOR, &new_cursor);
    xcb_generic_error_t *err = xcb_request_check(conn, cookie);
    if (err) {
      E("xcb_change_window_attributes (cursor) failed: code=%d", err->error_code);
      free(err);
      // if failed, free new_cursor to avoid leak
      xcb_free_cursor(conn, new_cursor);
      return;
    }
// Free the old server-side cursor resource if one was previously allocated
    if(mCurrentCursor != 0) {
      xcb_free_cursor(conn, mCurrentCursor);
      E()
    }
// Retain tracking ownership of the new allocation
    mCurrentCursor = new_cursor;
    mWindow->Draw();
  }

  void XcbWindowContext::DestroyFrame() {
    D2("DestroyFrame called, mWindowId={}", mWindowId);
    if(!mAUI)
      return;
    xcb_connection_t *conn = mAUI->GetXcbConnection();
    if(!conn)
      return;
    if(mWindowId) {
      if(mGC) {
        xcb_void_cookie_t cookie = xcb_free_gc_checked(conn, mGC);
        xcb_request_check(conn, cookie); // ignore error
        mGC = 0;
      }
// Free the active cursor resource before the window context is eliminated
      if(mCurrentCursor != 0) {
        xcb_void_cookie_t cookie = xcb_free_cursor_checked(conn, mCurrentCursor);
        xcb_request_check(conn, cookie);
        mCurrentCursor = 0;
      }
      xcb_void_cookie_t cookie = xcb_destroy_window_checked(conn, mWindowId);
      xcb_generic_error_t *err = xcb_request_check(conn, cookie);
      if (err) {
        D1("xcb_destroy_window failed: code=%d", err->error_code);
        free(err);
      }
      xcb_flush(conn);
      mWindowId = 0;
      mGC = 0;
    }
    mSoftwareBuffer->clear();
    D2("DestroyFrame done");
  }

  XcbWindowContext::~XcbWindowContext() {
    D2("XcbWindowContext destructor");
    if(mCursorContext) {
      xcb_cursor_context_free(mCursorContext);
      mCursorContext = nullptr;
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
    DestroyFrame();
  }
}// namespace aui
