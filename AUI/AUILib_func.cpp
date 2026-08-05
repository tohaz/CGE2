#include "AUILib.h"

namespace aui {

  AUIKeyCode translate_keysym_to_keycode(xcb_keysym_t sym) {
    D1("Keysym Hex: {:#06x} | Dec: {}", sym, sym);
    switch (sym) {
      case XK_Return:
        return AUIKeyCode::Enter;
      case XK_BackSpace:
        return AUIKeyCode::Backspace;
      case XK_Delete:
        return AUIKeyCode::Delete;
      case XK_Insert:
        return AUIKeyCode::Insert;
      case XK_Left:
        return AUIKeyCode::Left;
      case XK_Right:
        return AUIKeyCode::Right;
      case XK_Up:
        return AUIKeyCode::Up;
      case XK_Down:
        return AUIKeyCode::Down;
      case XK_Home:
        return AUIKeyCode::Home;
      case XK_End:
        return AUIKeyCode::End;
      case XK_Tab:
        return AUIKeyCode::Tab;
      case XK_Escape:
        return AUIKeyCode::Escape;
      case XK_space:
        return AUIKeyCode::Space;
      default:
        D("unable to translate X11 keycode")
        return AUIKeyCode::None;
    }
  }

  AUIModifier translate_modifiers(uint16_t state) {
    AUIModifier mod = AUIModifier::None;
    if(state & XCB_MOD_MASK_SHIFT)
      mod = mod | AUIModifier::Shift;
    if(state & XCB_MOD_MASK_CONTROL)
      mod = mod | AUIModifier::Ctrl;
    if(state & XCB_MOD_MASK_1)
      mod = mod | AUIModifier::Alt;
    if(state & XCB_MOD_MASK_4)
      mod = mod | AUIModifier::Super;
    return mod;
  }

  std::string NumberToBaseString(uint64_t n) {
    D3("entering with '{}', alphabet len '{}'", n, BaseAlphabet.size())
    std::string result = "";
    do {
      result += BaseAlphabet[n % BaseAlphabet.size()];
      n = n / BaseAlphabet.size();
      if(n > 0) {
        n--;
      }
      else {
        break;
      }
    } while (true);
    std::reverse(result.begin(), result.end());
    D3("'{}'", result.c_str())
    return result;
  }

  uint32_t GetRandomARGBColor() {
// Static random engine ensures seed initialization happens only once
    static std::random_device rd;
    static std::mt19937 gen(rd());
// Distribution covering the entire 24-bit RGB space (0x000000 to 0xFFFFFF)
    static std::uniform_int_distribution<uint32_t> dist(0x000000, 0xFFFFFF);
// Generate random RGB and shift Alpha (0xFF) into the highest 8 bits
    return 0xFF000000 | dist(gen);
  }

  uint32_t GetDistinctRandomARGBColor(uint32_t oldColor, uint8_t standOut) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    uint8_t oldR = (oldColor >> 16) & 0xFF;
    uint8_t oldG = (oldColor >> 8) & 0xFF;
    uint8_t oldB = oldColor & 0xFF;
    auto getComponent = [&](uint8_t oldVal) -> uint32_t {
// Calculate bounds using signed ints to avoid underflow/overflow wrapping
      int32_t lowerCount = static_cast<int32_t>(oldVal) - standOut + 1;
      int32_t upperCount = 256 - (static_cast<int32_t>(oldVal) + standOut);
      if(lowerCount < 0)
        lowerCount = 0;
      if(upperCount < 0)
        upperCount = 0;
      int32_t totalChoices = lowerCount + upperCount;
// Fallback constraint check
      if(totalChoices <= 0) {
        return (oldVal > 127) ? 0 : 255;
      }
      std::uniform_int_distribution<int32_t> dist(0, totalChoices - 1);
      int32_t choice = dist(gen);
// If our random pick falls into the lower bucket, return it directly
      if(choice < lowerCount) {
        return SafeUINT32(choice);
      }
// Otherwise, skip the middle "forbidden zone" and map it into the upper bucket
      return (static_cast<int32_t>(oldVal) + standOut) + (SafeUINT32(choice) - SafeUINT32(lowerCount));
    };
    uint32_t newR = getComponent(oldR);
    uint32_t newG = getComponent(oldG);
    uint32_t newB = getComponent(oldB);
    return 0xFF000000 | (newR << 16) | (newG << 8) | newB;
  }

  int32_t find_closest_strike(FT_Face face, int32_t target_ppem) {
    if(face->num_fixed_sizes == 0)
      return -1;
    int32_t best = 0;
    int32_t bestDiff = abs((int32_t) (face->available_sizes[0].y_ppem - target_ppem));
    for(int32_t i = 1; i < face->num_fixed_sizes; ++i) {
      int32_t diff = abs((int32_t) (face->available_sizes[i].y_ppem - target_ppem));
      if(diff < bestDiff) {
        bestDiff = diff;
        best = i;
      }
    }
    return best;
  }

  bool DetectWayland() {
    ST4("DetectWayland")
    const char* xdg = std::getenv("XDG_SESSION_TYPE");
    if(xdg && std::string(xdg) == "wayland") {
      return true;
    }
    if(!std::getenv("WAYLAND_DISPLAY")) {
      return false;
    }
    void* handle = dlopen("libwayland-client.so.0", RTLD_LAZY);
    if(!handle)
      return false;
//    typedef void* (*wl_display_connect_t)(const char*);
//    typedef void (*wl_display_disconnect_t)(void*);
//    auto wl_display_connect = reinterpret_cast<wl_display_connect_t>(dlsym(handle, "wl_display_connect"));
//    auto wl_display_disconnect = reinterpret_cast<wl_display_disconnect_t>(dlsym(handle, "wl_display_disconnect"));
//    bool connected = false;
//    if(wl_display_connect && wl_display_disconnect) {
//      void* display = wl_display_connect(nullptr);
//      if(display) {
//        connected = true;
//        wl_display_disconnect(display);
//      }
//    }
//    dlclose(handle);
//    return connected;
    return true;
  }

  bool DetectXWayland() {
    ST4("DetectXWayland")
    if(!DetectWayland()) {
      return false;
    }
    if(!std::getenv("DISPLAY")) {
      return false;
    }
    return true;
//    void* handle = dlopen("libX11.so.6", RTLD_LAZY);
//    if(!handle)
//      return false;
//    typedef void* (*XOpenDisplay_t)(const char*);
//    typedef int32_t (*XCloseDisplay_t)(void*);
//    auto XOpenDisplay = reinterpret_cast<XOpenDisplay_t>(dlsym(handle, "XOpenDisplay"));
//    auto XCloseDisplay = reinterpret_cast<XCloseDisplay_t>(dlsym(handle, "XCloseDisplay"));
//    bool has_x11_display = false;
//    if(XOpenDisplay && XCloseDisplay) {
//      ST1("XOpenDisplay")
//      void* display = XOpenDisplay(nullptr);
//      if(display) {
//        has_x11_display = true;
//        XCloseDisplay(display);
//      }
//    }
//    dlclose(handle);
//    return has_x11_display;
  }

  bool DetectX11() {
    ST4("DetectX11")
    if(DetectWayland()) {
      return false;
    }
    const char* xdg = std::getenv("XDG_SESSION_TYPE");
    if(xdg && std::string(xdg) == "x11") {
      return true;
    }
    if(!std::getenv("DISPLAY")) {
      return false;
    }
    void* handle = dlopen("libX11.so.6", RTLD_LAZY);
    if(!handle)
      return false;
    typedef void* (*XOpenDisplay_t)(const char*);
    typedef int32_t (*XCloseDisplay_t)(void*);
    auto XOpenDisplay = reinterpret_cast<XOpenDisplay_t>(dlsym(handle, "XOpenDisplay"));
    auto XCloseDisplay = reinterpret_cast<XCloseDisplay_t>(dlsym(handle, "XCloseDisplay"));
    bool connected = false;
    if(XOpenDisplay && XCloseDisplay) {
      void* display = XOpenDisplay(nullptr);
      if(display) {
        connected = true;
        XCloseDisplay(display);
      }
    }
    dlclose(handle);
    return connected;
  }

  std::string XCBConnectErrorToString(int32_t error_code) {
    switch(error_code) {
      case XCB_CONN_ERROR: {
        std::string msg = "XCB_CONN_ERROR: Socket, pipe or stream error";
        if(errno != 0) {
          msg += " (System error: " + std::string(std::strerror(errno)) + ")";
        }
        return msg;
      }
      case XCB_CONN_CLOSED_EXT_NOTSUPPORTED:
        return "XCB_CONN_CLOSED_EXT_NOT_SUPPORTED: Requested X-server extension is not supported";
      case XCB_CONN_CLOSED_MEM_INSUFFICIENT:
        return "XCB_CONN_CLOSED_MEM_INSUFFICIENT: Insufficient memory available";
      case XCB_CONN_CLOSED_REQ_LEN_EXCEED:
        return "XCB_CONN_CLOSED_REQ_LEN_EXCEED: Request length exceeded what the X-server accepts";
      case XCB_CONN_CLOSED_PARSE_ERR:
        return "XCB_CONN_CLOSED_PARSE_ERR: Error parsing display string (invalid $DISPLAY format)";
      case XCB_CONN_CLOSED_INVALID_SCREEN:
        return "XCB_CONN_CLOSED_INVALID_SCREEN: The X-server does not have a screen matching the display";
      default:
        return "Unknown XCB connection error (code " + std::to_string(error_code) + ")";
    }
  }

  uint32_t HLColor(uint32_t ci) {
    ARGBColor c;
    c.value = ci;
    uint32_t overall = SafeUINT32(c.argb.r + c.argb.g + c.argb.b);
    // 0x80 * 3 = 384
    if(overall > 384) {
      if(c.argb.r > AUI_HL_SHIFT) c.argb.r -= AUI_HL_SHIFT;
      else c.argb.r = 0;
      if(c.argb.g > AUI_HL_SHIFT) c.argb.g -= AUI_HL_SHIFT;
      else c.argb.g = 0;
      if(c.argb.b > AUI_HL_SHIFT) c.argb.b -= AUI_HL_SHIFT;
      else c.argb.b = 0;
    }
    else {
      if(c.argb.r < 255) c.argb.r += AUI_HL_SHIFT;
      else c.argb.r = 255;
      if(c.argb.g < 255) c.argb.g += AUI_HL_SHIFT;
      else c.argb.g = 255;
      if(c.argb.b < 255) c.argb.b += AUI_HL_SHIFT;
      else c.argb.b = 255;
    }
    return c.value;
  }

  uint32_t DarkenColor(uint32_t ci) {
    ARGBColor c;
    c.value = ci;
    c.argb.r = (c.argb.r > AUI_DARKEN_SHIFT) ? c.argb.r - AUI_DARKEN_SHIFT: 0;
    c.argb.g = (c.argb.g > AUI_DARKEN_SHIFT) ? c.argb.g - AUI_DARKEN_SHIFT: 0;
    c.argb.b = (c.argb.b > AUI_DARKEN_SHIFT) ? c.argb.b - AUI_DARKEN_SHIFT: 0;
    return c.value;
  }
}
