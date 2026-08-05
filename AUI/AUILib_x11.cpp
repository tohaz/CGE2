#include "AUILib.h"

namespace aui {
  bool AUI::InitXCB() {
    if(mX11_Owned == true) {
      E("X11 already initialized")
    }
    D2("InitX11 called");
    int32_t screenIdx = 0;
    mX11Connection = xcb_connect(nullptr, &screenIdx);
    int32_t err = xcb_connection_has_error(mX11Connection);
    if(!mX11Connection || err) {
      E("x11_connect failed {}", XCBConnectErrorToString(err));
    }
    const xcb_setup_t* setup = xcb_get_setup(mX11Connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    for(int32_t i = 0; i < screenIdx; ++i)
      xcb_screen_next(&iter);
    mX11Screen = iter.data;
    int32_t xcb_fd = xcb_get_file_descriptor(mX11Connection);
    mFDs[AUI_XCB_FD_INDEX] = { xcb_fd, POLLIN, 0 };
    mX11_Owned = true;
    D2("InitX11 done, screen={}", static_cast<void*>(mX11Screen));
    return true;
  }

  void AUI::XCBProcessMessages() {
    ST2("")
    D4()
    if(!mX11Connection) {
      E("no connection");
      return;// Ensure we don't fall through if connection is dead
    }
    xcb_generic_event_t* ev = nullptr;
    while((ev = xcb_poll_for_event(mX11Connection)) != nullptr) {
// Correctly read and mask the response type
      const uint8_t response_type = ev->response_type & static_cast<uint8_t>(~0x80U);
      xcb_window_t window_id = 0;
      switch(response_type) {
        case XCB_EXPOSE:
          window_id = reinterpret_cast<xcb_expose_event_t*>(ev)->window;
          break;
        case XCB_KEY_PRESS:
          window_id = reinterpret_cast<xcb_key_press_event_t*>(ev)->event;
          break;
        case XCB_KEY_RELEASE:
          window_id = reinterpret_cast<xcb_key_release_event_t*>(ev)->event;
          break;
        case XCB_BUTTON_PRESS:
          D4("XCB_BUTTON_PRESS")
          window_id = reinterpret_cast<xcb_button_release_event_t*>(ev)->event;
          break;
        case XCB_BUTTON_RELEASE:
          D4("XCB_BUTTON_RELEASE")
          window_id = reinterpret_cast<xcb_button_press_event_t*>(ev)->event;
          break;
        case XCB_MOTION_NOTIFY:
          window_id = reinterpret_cast<xcb_motion_notify_event_t*>(ev)->event;
          break;
        case XCB_CONFIGURE_NOTIFY:
          window_id = reinterpret_cast<xcb_configure_notify_event_t*>(ev)->window;
          break;
        case XCB_CLIENT_MESSAGE:
          window_id = reinterpret_cast<xcb_client_message_event_t*>(ev)->window;
          break;
        case XCB_MAP_NOTIFY:
          window_id = reinterpret_cast<xcb_map_notify_event_t*>(ev)->window;
          break;
        case XCB_UNMAP_NOTIFY:
          window_id = reinterpret_cast<xcb_unmap_notify_event_t*>(ev)->window;
          break;
        case XCB_REPARENT_NOTIFY:
          D2("skipping XCB_REPARENT_NOTIFY")
          free(ev);
          continue;
        case XCB_NO_EXPOSURE:
          D2("skipping XCB_NO_EXPOSURE")
          free(ev);
          continue;
        case XCB_PROPERTY_NOTIFY:
          D2("skipping XCB_PROPERTY_NOTIFY")
          free(ev);
          continue;
        case XCB_DESTROY_NOTIFY:
          D2("skipping XCB_DESTROY_NOTIFY")
          free(ev);
          continue;
        default:
          E("Unknown event type: {} ({})", static_cast<int32_t>(response_type), XCB_EventTypeToString(response_type))
          free(ev);
          continue;
      }
      AWindow* w = X11FindWindow(window_id);
      if(w) {
        XCBWindowContext* x = static_cast<XCBWindowContext*>(w);
        x->ProcessEvent(ev);
      }
      else {
        D2("skipping event '{}' with unknown window {}", XCB_EventTypeToString(response_type), window_id)
      }
      free(ev);
    }
  }

  std::string XCB_EventTypeToString(uint8_t response_type) {
    switch(response_type) {
      case 0:
        return "Error (use xcb_error_code_to_string)";
      case 1:
        return "Reply (not an event)";
      case XCB_KEY_PRESS:
        return "KeyPress";
      case XCB_KEY_RELEASE:
        return "KeyRelease";
      case XCB_BUTTON_PRESS:
        return "ButtonPress";
      case XCB_BUTTON_RELEASE:
        return "ButtonRelease";
      case XCB_MOTION_NOTIFY:
        return "MotionNotify";
      case XCB_ENTER_NOTIFY:
        return "EnterNotify";
      case XCB_LEAVE_NOTIFY:
        return "LeaveNotify";
      case XCB_FOCUS_IN:
        return "FocusIn";
      case XCB_FOCUS_OUT:
        return "FocusOut";
      case XCB_KEYMAP_NOTIFY:
        return "KeymapNotify";
      case XCB_EXPOSE:
        return "Expose";
      case XCB_GRAPHICS_EXPOSURE:
        return "GraphicsExposure";
      case XCB_NO_EXPOSURE:
        return "NoExposure";
      case XCB_VISIBILITY_NOTIFY:
        return "VisibilityNotify";
      case XCB_CREATE_NOTIFY:
        return "CreateNotify";
      case XCB_DESTROY_NOTIFY:
        return "DestroyNotify";
      case XCB_UNMAP_NOTIFY:
        return "UnmapNotify";
      case XCB_MAP_NOTIFY:
        return "MapNotify";
      case XCB_MAP_REQUEST:
        return "MapRequest";
      case XCB_REPARENT_NOTIFY:
        return "ReparentNotify";
      case XCB_CONFIGURE_NOTIFY:
        return "ConfigureNotify";
      case XCB_CONFIGURE_REQUEST:
        return "ConfigureRequest";
      case XCB_GRAVITY_NOTIFY:
        return "GravityNotify";
      case XCB_RESIZE_REQUEST:
        return "ResizeRequest";
      case XCB_CIRCULATE_NOTIFY:
        return "CirculateNotify";
      case XCB_CIRCULATE_REQUEST:
        return "CirculateRequest";
      case XCB_PROPERTY_NOTIFY:
        return "PropertyNotify";
      case XCB_SELECTION_CLEAR:
        return "SelectionClear";
      case XCB_SELECTION_REQUEST:
        return "SelectionRequest";
      case XCB_SELECTION_NOTIFY:
        return "SelectionNotify";
      case XCB_COLORMAP_NOTIFY:
        return "ColormapNotify";
      case XCB_CLIENT_MESSAGE:
        return "ClientMessage";
      case XCB_MAPPING_NOTIFY:
        return "MappingNotify";
      case XCB_GE_GENERIC:
        return "GenericEvent (use xcb_ge_event_t)";
      default:
        return "Unknown event type (" + std::to_string((uint32_t)response_type) + ")";
    }
  }

  bool AUI::XCBUnregisterWindow(UNUSED uint64_t nativeID) {
    D1("UnregisterXCBWindow: nativeId={}", nativeID);
    UNUSED AWindow* w = nullptr;
    UNUSED auto it = mXCBWindowMap.find(nativeID);
    if(it == mXCBWindowMap.end()) {
      D("UnregisterWindow: X11 window not found (nativeId={})", nativeID);
      return false;
    }
    w = it->second.get();
    if(mFocusedWindow == w) {
      mFocusedWindow = nullptr;
      D1("removing focus from window {}", nativeID)
    }
    mXCBWindowMap.erase(it);
    return true;
  }

}// namespace aui
//
