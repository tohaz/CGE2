#include "AUILib.h"
#include "XcbWindowContext.h"

namespace aui {

  XcbWindowContext::XcbWindowContext(AUI *aui, AWindow *window) {
    mWindow = window;
    mAUI = aui;
    D3("w {} aui {}", (int64_t)mWindow, (int64_t)mAUI)
  }

  bool XcbWindowContext::CreateFrame(uint32_t width, uint32_t height, const std::string& title) {
    D2("CreateFrame entry: width={}, height={}, title='{}'", width, height, title);
    if(!mAUI) { DT("mAUI is null"); return false; }
    xcb_connection_t* conn = mAUI->GetXcbConnection();
    xcb_screen_t* screen = mAUI->GetXcbScreen();
    D2("conn={}, screen={}", static_cast<void*>(conn), static_cast<void*>(screen));
    if(!conn || !screen) { DT("conn or screen null"); return false; }
    mWindowId = xcb_generate_id(conn);
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[2] = {
      AUI_DEFAULT_WINDOW_BG,
      XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
      XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
      XCB_EVENT_MASK_POINTER_MOTION
    };
    xcb_create_window(conn, XCB_COPY_FROM_PARENT, mWindowId, screen->root,
                      0, 0, static_cast<uint16_t>(width), static_cast<uint16_t>(height), 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
                      mask, values);
    SetTitle(title);
    mGC = xcb_generate_id(conn);
    uint32_t gc_value = 0;
    xcb_create_gc(conn, mGC, mWindowId, XCB_GC_FOREGROUND, &gc_value);
    // WM_DELETE_WINDOW protocol
    xcb_intern_atom_cookie_t protocols_cookie = xcb_intern_atom(conn, 0, 12, "WM_PROTOCOLS");
    xcb_intern_atom_cookie_t delete_cookie = xcb_intern_atom(conn, 0, 16, "WM_DELETE_WINDOW");
    xcb_intern_atom_reply_t* protocols_reply = xcb_intern_atom_reply(conn, protocols_cookie, nullptr);
    xcb_intern_atom_reply_t* delete_reply = xcb_intern_atom_reply(conn, delete_cookie, nullptr);
    if(protocols_reply && delete_reply) {
      mWmProtocolsAtom = protocols_reply->atom;
      mWmDeleteWindowAtom = delete_reply->atom;
      D2("WM_PROTOCOLS atom set to: {}", mWmProtocolsAtom);
      D2("WM_DELETE_WINDOW atom set to: {}", mWmDeleteWindowAtom);
      xcb_change_property(conn, XCB_PROP_MODE_REPLACE, mWindowId,
                          mWmProtocolsAtom, XCB_ATOM_ATOM, 32, 1, &mWmDeleteWindowAtom);
      free(protocols_reply);
      free(delete_reply);
    }
    else {
      DT("Failed to intern WM_DELETE_WINDOW atom (protocols_reply={}, delete_reply={})",
         (void*)protocols_reply, (void*)delete_reply);
      if(protocols_reply) free(protocols_reply);
      if(delete_reply) free(delete_reply);
    }
    xcb_map_window(conn, mWindowId);
    xcb_flush(conn);
    mSoftwareBuffer.resize(width * height, 0xFFAAAAAA);
    return true;
  }

  void XcbWindowContext::Move(int32_t x, int32_t y) {
    if(!mAUI) return;
    xcb_connection_t *conn = mAUI->GetXcbConnection();
    if(!conn || !mWindowId) return;
    uint32_t values[2] = { static_cast<uint32_t>(x), static_cast<uint32_t>(y) };
    xcb_configure_window(conn, mWindowId, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
  }

  void XcbWindowContext::Resize(uint32_t width, uint32_t height) {
    if(!mAUI) return;
    if(mSizeX == width && mSizeY == height) return;
    xcb_connection_t *conn = mAUI->GetXcbConnection();
    if(!conn || !mWindowId) return;
    mSizeX = width;
    mSizeY = height;
    mSoftwareBuffer.resize(width * height, 0);
    uint32_t values[2] = { width, height };
    xcb_configure_window(conn, mWindowId, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
    if(mWindow && !mWindow->IsResizeEnabled()) {
      ApplySizeHints(width, height, width, height);
    }
    xcb_flush(conn);
  }

  void XcbWindowContext::SetTitle(const std::string &title) {
    if(!mAUI) return;
    xcb_connection_t *conn = mAUI->GetXcbConnection();
    if(!conn || !mWindowId) return;
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, mWindowId, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
        static_cast<uint32_t>(title.size()), title.c_str());
  }

  uint32_t* XcbWindowContext::GetSoftwareBuffer() {
    return mSoftwareBuffer.data();
  }

  void XcbWindowContext::QueueFrameCommit() {
      if(!mAUI || mWindowId == 0) return;
      DrawCommand cmd;
      cmd.type = DrawCommandType::Xcb;
      cmd.xcb.windowId = mWindowId;
      cmd.xcb.buffer = mSoftwareBuffer.data();
      cmd.xcb.width = mWindow->SizeX();
      cmd.xcb.height = mWindow->SizeY();
      mAUI->EnqueueDrawCommand(cmd);
  }

  void XcbWindowContext::ProcessEvent(void* ev) {
    xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(ev);
    uint8_t type = event->response_type & 0x7F;
    D3("XcbWindowContext::ProcessEvent: type={}", (int32_t)type);
    switch(type) {
      case XCB_EXPOSE: {
        D2("Expose event, redrawing window");
        if(mWindow) mWindow->Draw();
        break;
      }
      case XCB_MOTION_NOTIFY: {
        auto* motion = reinterpret_cast<xcb_motion_notify_event_t*>(event);
        D2("Mouse motion at ({},{})", motion->event_x, motion->event_y);
        if(mWindow) mWindow->OnMouseMove(motion->event_x, motion->event_y);
        break;
      }
      case XCB_BUTTON_PRESS: {
          auto* btn = reinterpret_cast<xcb_button_press_event_t*>(event);
          if (mWindow) {
              if (btn->detail == 4) {          // scroll up
                  mWindow->OnMouseWheel(1);
              } else if (btn->detail == 5) {   // scroll down
                  mWindow->OnMouseWheel(-1);
              } else {
                  mWindow->OnMousePress(btn->event_x, btn->event_y, btn->detail);
              }
          }
          break;
      }
      case XCB_BUTTON_RELEASE: {
        auto* btn = reinterpret_cast<xcb_button_release_event_t*>(event);
        D2("Button release {} at ({},{})", (int32_t)btn->detail, btn->event_x, btn->event_y);
        if(mWindow) {
          mWindow->OnMouseRelease(btn->event_x, btn->event_y, btn->detail);
        }
        break;
      }
      
      case XCB_LEAVE_NOTIFY: {
        if(mWindow) mWindow->ClearHover();
        break;
      }
      case XCB_CLIENT_MESSAGE: {
        auto* msg = reinterpret_cast<xcb_client_message_event_t*>(event);
        D2("Client message atom: {}", msg->type);
        D2("mWmProtocolsAtom: {}", mWmProtocolsAtom);
        D2("mWmDeleteWindowAtom: {}", mWmDeleteWindowAtom);
        if(msg->type == mWmProtocolsAtom && static_cast<uint32_t>(msg->data.data32[0]) == mWmDeleteWindowAtom) {
          D2("WM_DELETE_WINDOW received, calling ExitAUI");
          if(mWindow) mWindow->Close();
        } else {
          DT("Atom mismatch, ignoring");
        }
        break;
      }
      case XCB_CONFIGURE_NOTIFY: {
        auto* cfg = reinterpret_cast<xcb_configure_notify_event_t*>(event);
        mSizeX = cfg->width;
        mSizeY = cfg->height;
        uint32_t new_size = static_cast<uint32_t>(cfg->width) * static_cast<uint32_t>(cfg->height);
        if(mSoftwareBuffer.size() != new_size) {
          mSoftwareBuffer.resize(new_size, 0xFFAAAAAA);
        }
        if(mWindow && mWindow->IsResizeEnabled()) {
          mWindow->Resize(cfg->width, cfg->height);
        }
        break;
      }
      default:
        D2("Unhandled event type: {}", (int32_t)type);
        break;
    }
  }

  void XcbWindowContext::DestroyFrame() {
    D2("DestroyFrame called, mWindowId={}", mWindowId);
    if(!mAUI) return;
    xcb_connection_t* conn = mAUI->GetXcbConnection();
    if(!conn) return;
    if(mWindowId) {
      if(mGC) xcb_free_gc(conn, mGC);
      xcb_destroy_window(conn, mWindowId);
      xcb_flush(conn);
      mWindowId = 0;
      mGC = 0;
    }
    mSoftwareBuffer.clear();
    D2("DestroyFrame done");
  }

  void XcbWindowContext::ApplySizeHints(uint32_t min_w, uint32_t min_h, uint32_t max_w, uint32_t max_h) {
    D2("minw {} minh {}", min_w, min_h);
    if(!mAUI || !mWindowId) return;
    xcb_connection_t* conn = mAUI->GetXcbConnection();
    if(!conn) return;
    xcb_intern_atom_cookie_t hints_cookie = xcb_intern_atom(conn, 0, 15, "WM_NORMAL_HINTS");
    xcb_intern_atom_reply_t* hints_reply = xcb_intern_atom_reply(conn, hints_cookie, nullptr);
    if(!hints_reply) return;
    uint32_t hints[18] = {0};
    hints[0] = 48U;
    hints[5] = min_w;
    hints[6] = min_h;
    hints[7] = max_w;
    hints[8] = max_h;
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, mWindowId,
                        hints_reply->atom, XCB_ATOM_WM_SIZE_HINTS, 32, 18, hints);
    free(hints_reply);
    xcb_flush(conn);
  }

  void XcbWindowContext::EnableResize() {
    ApplySizeHints(1, 1, 65535, 65535);
  }

  void XcbWindowContext::DisableResize() {
    uint32_t w = mWindow->SizeX();
    uint32_t h = mWindow->SizeY();
    ApplySizeHints(w, h, w, h);
  }

  XcbWindowContext::~XcbWindowContext() {
    D2("XcbWindowContext destructor");
    DestroyFrame();
  }
} // namespace aui
