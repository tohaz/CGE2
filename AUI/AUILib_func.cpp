#include "AUILib.h"



namespace aui {

  void ClipRect(int32_t &x, int32_t &y, int32_t &w, int32_t &h, int32_t parentW, int32_t parentH) {
    if(x < 0) {
      w += x;
      x = 0;
    }
    if(y < 0) {
      h += y;
      y = 0;
    }
    if(x + w > parentW)
      w = parentW - x;
    if(y + h > parentH)
      h = parentH - y;
    if(w <= 0 || h <= 0) {
      w = h = 0;
    }
  }

  UNUSED static FT_Error ftc_face_requester(UNUSED FTC_FaceID face_id,
  UNUSED FT_Library library, FT_Pointer request_data, FT_Face *aface) {
    AUI *au = static_cast<AUI*>(request_data);
    *aface = au->GetDefaultFontFace();
    return 0;
  }

  std::string NumberToBaseString(UINT64 n) {
    D3("entering with '{}', alphabet len '{}'", n, mAlphabetLen)
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

  AUIKeyCode translate_keysym_to_keycode(xcb_keysym_t sym) {
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

  AUIKeyCode translate_keysym(xcb_keysym_t sym) {
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
        return AUIKeyCode::None;
    }
  }

}// namespace aui

