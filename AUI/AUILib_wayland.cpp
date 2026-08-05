#include "AUILib.h"

namespace aui {

  static const struct wl_pointer_listener pointer_listener = { .enter = wayland_pointer_handle_enter, .leave =
      wayland_pointer_handle_leave, .motion = wayland_pointer_handle_motion, .button = wayland_pointer_handle_button,
      .axis = wayland_pointer_handle_axis, .frame = NULL, .axis_source = NULL, .axis_stop = NULL, .axis_discrete = NULL,
      .axis_value120 = NULL, .axis_relative_direction = NULL, .warp = nullptr, };

  static void registry_global(UNUSED void *data, UNUSED wl_registry *registry, UNUSED uint32_t id,
      UNUSED const char *interface,
      UNUSED uint32_t version) {
    D3("interface {} version {}", interface, version)
    D3("registry_global ", version);
    auto* au = static_cast<AUI*>(data);
    if(!au) E()
    switch (hash64(interface)) {
      case hash64("wl_compositor"):
        if(strcmp(interface, "wl_compositor") != 0)
[[unlikely]] {                     E("hash collision");
        }
        if(wl_compositor* comp = static_cast<wl_compositor*>(wl_registry_bind(registry, id, &wl_compositor_interface, 4))) {
          au->WaylandCompositor(comp);
        }
        else {
          E("wl_registry_bind wl_compositor failed")
        }
        break;
      case hash64("wl_shm"):
        if(strcmp(interface, "wl_shm") != 0)
[[unlikely]] {                            E("hash collision");
        }
        if(wl_shm* shm = static_cast<wl_shm*>(wl_registry_bind(registry, id, &wl_shm_interface, 1))) {
          au->WaylandShm(shm);
        }
        else {
          E("wl_registry_bind wl_shm failed")
        }
        break;
      case hash64("xdg_wm_base"):
        if(strcmp(interface, "xdg_wm_base") != 0)
[[unlikely]] {                            E("hash collision");
        }
        if(UNUSED xdg_wm_base* base = static_cast<xdg_wm_base*>(wl_registry_bind(registry, id, &xdg_wm_base_interface, 1))) {
          au->WaylandXdgBase(base);
          static const struct xdg_wm_base_listener wm_base_listener = { .ping = [](void*, xdg_wm_base *xdg,
              uint32_t serial) {
            D2("Ping received, serial={}", serial);
            // no return value
            xdg_wm_base_pong(xdg, serial);
          } };
          if(xdg_wm_base_add_listener(base, &wm_base_listener, au) != 0) E("xdg_wm_base_add_listener failed")
        }
        else {
          E("wl_registry_xdg_wm_base failed")
        }
        break;
      case hash64("zxdg_decoration_manager_v1"):
        if(strcmp(interface, "zxdg_decoration_manager_v1") != 0)
[[unlikely]] {                                      E("hash collision");
        }
        if(zxdg_decoration_manager_v1* dm = static_cast<zxdg_decoration_manager_v1*>(wl_registry_bind(registry, id,
            &zxdg_decoration_manager_v1_interface, 1))) {
          au->WaylandDecorationManager(dm);
        }
        break;
      case hash64("wl_seat"):
        if(strcmp(interface, "wl_seat") != 0)
[[unlikely]] {                                      E("hash collision");
        }
        if(UNUSED wl_seat* seat = static_cast<wl_seat*>(wl_registry_bind(registry, id, &wl_seat_interface, 4))) {
          au->WaylandSeat(seat);
        }
        break;
      default:
        D3("unknown Wayland interface {}", interface)
        break;
    }
  }

  UNUSED static void registry_remove(UNUSED void *data, UNUSED wl_registry *registry, UNUSED uint32_t id) {
    E()
  }

  static const wl_registry_listener registry_listener = { .global = registry_global, .global_remove =
      registry_remove };

  UNUSED static void keyboard_keymap(void* data, struct wl_keyboard*, uint32_t format, int32_t fd, uint32_t size) {
    UNUSED auto* aui = static_cast<AUI*>(data);
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
//// Create a safe context if it doesn't exist yet
    if(!aui->XkbContext()) {
      auto* ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
      if(!ctx) {
        munmap(map_str, size);
        close(fd);
        E("Failed to create xkb_context.");
      }
      aui->XkbContext(ctx);
    }
// Check 2: Verify the compositor actually null-terminated the string.
// The last byte inside the size scope MUST be '\0'.
    if(map_str[size - 1] != '\0') {
      munmap(map_str, size);
      close(fd);
      E("Wayland keymap string is not null-terminated. Preventing buffer overflow.");
    }
// Compile the keymap string
    struct xkb_keymap* keymap = xkb_keymap_new_from_string(aui->XkbContext(), map_str, XKB_KEYMAP_FORMAT_TEXT_V1,
        XKB_KEYMAP_COMPILE_NO_FLAGS);
// Clean up memory mappings immediately now that compilation is done/attempted
    munmap(map_str, size);
    close(fd);
// Check 3: Ensure the keymap actually compiled successfully
    if(!keymap) {
      E("xkb_keymap_new_from_string failed to compile the compositor keymap specification.");
    }
    aui->XkbKeymap(keymap);
// Allocate the tracking state using our verified keymap
    struct xkb_state* state = xkb_state_new(aui->XkbKeymap());
    if(!state) {
      E("Failed to allocate xkb_state engine from verified keymap.");
    }
    aui->XkbState(state);
  }

  UNUSED static void keyboard_key(void* data, struct wl_keyboard*, uint32_t, uint32_t, uint32_t key, uint32_t state) {
    auto* aui = static_cast<AUI*>(data);
    if(!aui->XkbState())
      return;
    xkb_keysym_t sym = xkb_state_key_get_one_sym(aui->XkbState(), key + 8);
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
    AWindow* win = aui->FocusedWindow();
    if(win)
      win->OnKeyEvent(ev);
  }

  UNUSED static void keyboard_modifiers(void* data, struct wl_keyboard*, uint32_t, uint32_t mods_depressed,
      uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    auto* aui = static_cast<AUI*>(data);
    if(aui->XkbState()) {
      xkb_state_update_mask(aui->XkbState(), mods_depressed, mods_latched, mods_locked, 0, 0, group);
    }
  }

  static void keyboard_enter(void*, struct wl_keyboard*, uint32_t, struct wl_surface*, struct wl_array*) {
  }

  static void keyboard_leave(void*, struct wl_keyboard*, uint32_t, struct wl_surface*) {
  }

  static void keyboard_repeat_info(UNUSED void* data, UNUSED struct wl_keyboard* keyboard, UNUSED int32_t rate,
  UNUSED int32_t delay) {
  }

  static const struct wl_keyboard_listener keyboard_listener = { .keymap = keyboard_keymap, .enter = keyboard_enter,
      .leave = keyboard_leave, .key = keyboard_key, .modifiers = keyboard_modifiers, .repeat_info = keyboard_repeat_info// was nullptr
      };

  bool AUI::InitWayland() {
    ST4("InitWayland");
    if(mMainBackendType == AUIWindowType::unset) {
      mMainBackendType = AUIWindowType::Wayland;
    }
    else {
      E("primary backend type is already set");
      return false;
    }
    mWaylandDisplay = wl_display_connect(nullptr);
    if(!mWaylandDisplay) {
      E("cannot connect Wayland");
      return false;
    }
    mWaylandRegistry = wl_display_get_registry(mWaylandDisplay);
    if(!mWaylandRegistry) {
      E("wl_display_get_registry failed");
      return false;
    }
    WaylandAddRegistryListener();
    {
      ST2("WaylandRoundtrip");
// Flush request buffer before waiting on roundtrip
      wl_display_flush(mWaylandDisplay);
      WaylandRoundtrip(1);
    }
    if(!mWaylandCompositor || !mWaylandShm || !mWaylandXdgBase) {
      E("Wayland init failed: missing essential globals");
      return false;
    }
    mWaylandFD = wl_display_get_fd(mWaylandDisplay);
    if(mWaylandFD < 0) {
      E("wl_display_get_fd failed");
      return false;
    }
    mFDs[AUI_WAYLAND_FD_INDEX].fd = mWaylandFD;
    mFDs[AUI_WAYLAND_FD_INDEX].events = POLLIN;
// Dispatch remaining events from registry setup
    if(wl_display_dispatch_pending(mWaylandDisplay) < 0) {
      E("wl_display_dispatch_pending failure");
      return false;
    }
    D2("wl_display_dispatch_pending success");
    return true;
  }

  void wayland_pointer_handle_enter(UNUSED void *data, UNUSED struct wl_pointer *pointer,
      UNUSED uint32_t serial,
      UNUSED struct wl_surface *surface, wl_fixed_t sx_w, wl_fixed_t sy_w) {
    D2("sx {} sy {}", sx_w, sy_w)
    AUI* au = static_cast<AUI*>(data);
    au->WaylandPointerSerial(serial);
    au->WaylandPointer(pointer);
    AWindow* w = au->WaylandFindWindow(surface);
    if(w != nullptr) {
      au->WaylandFocusedSurface(surface);
      au->FocusedWindow(w);
//// Convert fixed-point coordinates to integer (scaled later)
      int32_t x = wl_fixed_to_int(sx_w);
      int32_t y = wl_fixed_to_int(sy_w);
      au->UpdatePointerTracking(x, y);
      w->OnMouseEnter(x, y);
    }
    else {
      D1("unknown window in pointer_enter")
    }
  }

  void wayland_pointer_handle_leave(void *data, UNUSED struct wl_pointer *pointer, UNUSED uint32_t serial,
      UNUSED struct wl_surface *surface) {
    D2("serial {}", serial)
    AUI* au = static_cast<AUI*>(data);
    AWindow* w = au->FocusedWindow();
    if(w != nullptr) {
      w->OnMouseLeave(au->LastPointerX(), au->LastPointerY());
      au->FocusedWindow(nullptr);
    }
    au->WaylandFocusedSurface(nullptr);
  }

  void wayland_pointer_handle_motion(UNUSED void *data, UNUSED struct wl_pointer *pointer, UNUSED uint32_t time,
      UNUSED wl_fixed_t sx_w, UNUSED wl_fixed_t sy_w) {
    D2("sx {} sy {} time {}", sx_w, sy_w, time)
    AUI* au = static_cast<AUI*>(data);
    AWindow* w = au->FocusedWindow();
    WaylandWindowContext* ctx = reinterpret_cast<WaylandWindowContext*>(w);
    if(w != nullptr) {
      int32_t x = wl_fixed_to_int(sx_w);
      int32_t y = wl_fixed_to_int(sy_w);
      au->UpdatePointerTracking(x, y);
      // Cheap non-atomic check: if theme just finished loading, re-apply cursor
      if (ctx->CursorNeedsApply()) {
        ctx->BackendCursor(ctx->CurrentCursorType());
//        E("updating")
      }
      w->OnMouseMove(x, y);
    }
    else { E("mouse motion on unfocused window") }
  }

  UNUSED void wayland_pointer_handle_button(UNUSED void *data, UNUSED struct wl_pointer *pointer, UNUSED uint32_t serial,
      UNUSED uint32_t time,
      UNUSED uint32_t button, UNUSED uint32_t state) {
    D2("button {} state {}", button, state)
    AUI* au = static_cast<AUI*>(data);
    AWindow* w = au->FocusedWindow();
    if(w != nullptr) {
      int32_t x = au->LastPointerX();
      int32_t y = au->LastPointerY();
      if(state == WL_POINTER_BUTTON_STATE_PRESSED) {
        w->OnMousePress(x, y, button);
      }
      else {
        w->OnMouseRelease(x, y, button);
      }
      wl_pointer_set_cursor(pointer, serial, nullptr, 0, 0);
    }
    else {E("mouse click in null focused window")}
  }

  void wayland_pointer_handle_axis(UNUSED void *data, UNUSED struct wl_pointer *pointer, UNUSED uint32_t time,
      UNUSED uint32_t axis, wl_fixed_t value) {
    D2("time {} axis {} value {}", time, axis, value)
   AUI* au = static_cast<AUI*>(data);
   AWindow* w = au->FocusedWindow();
    if(w != nullptr) {
      w->OnMouseWheel(au->LastPointerX(), au->LastPointerY(), -wl_fixed_to_int(value));
    }
  }

  void AUI::WaylandCompositor(wl_compositor *comp) {
    D4("wl_compositor {}", (void *)comp)
    mWaylandCompositor = comp;
  }

  void AUI::WaylandShm(wl_shm *shm) {
    D4("wl_shm {}", (void *)shm)
    mWaylandShm = shm;
  }

  void AUI::WaylandXdgBase(xdg_wm_base *base) {
    D4("xdg_wm_base {}", (void *)base)
    mWaylandXdgBase = base;
  }

  void AUI::WaylandDecorationManager(zxdg_decoration_manager_v1 *mgr) {
    D4("zxdg_decoration_manager_v1 {}", (void *)mgr)
    mWaylandDecorationManager = mgr;
  }

  void AUI::WaylandSeat(wl_seat *seat) {
    D3("seat {}", (void *)seat)
    mWaylandSeat = seat;
    if(mWaylandPointer == nullptr) {
      D3("binding pointer_listener")
      mWaylandPointer = wl_seat_get_pointer(seat);
      wl_pointer_add_listener(mWaylandPointer, &pointer_listener, this);
    }
    else {E("not binding pointer_listener")}
    if(mWaylandKeyboard == nullptr) {
      mWaylandKeyboard = wl_seat_get_keyboard(seat);
      if(mWaylandKeyboard) {
        wl_keyboard_add_listener(mWaylandKeyboard, &keyboard_listener, this);
        D4("keyboard listener added");
      }
      else {
        E("failed to get keyboard from seat");
      }
    }
    else {
      E("keyboard already bound");
    }
    //DS()
  }

  void AUI::WaylandRoundtrip(uint32_t passes = 1) {
    D2("starts roundtrip (passes: {})", passes);
    for (uint32_t i = 0; i < passes; i++) {
      wl_display_flush(mWaylandDisplay);
      int32_t ev = wl_display_roundtrip(mWaylandDisplay);
      if (ev == -1) {
        E("error in Wayland roundtrip on pass {}", i + 1);
        break;
      }
      D2("pass {}/{} done: {} events received", i + 1, passes, ev);
    }
    D2("ends");
  }

  void AUI::WaylandAddRegistryListener() {
    ST2("");
    if(wl_registry_add_listener(mWaylandRegistry, &registry_listener, this) != 0)
      E("wl_registry_add_listener failed")
  }

  wl_display* AUI::WaylandDisplay() const {
    if(mWaylandDisplay != nullptr) return mWaylandDisplay;
    else {
      E("null reference")
    }
    return nullptr;
  }

  wl_compositor* AUI::WaylandCompositor() const {
    if(mWaylandCompositor != nullptr) {
      D4("returning {}", (uint64_t)mWaylandCompositor)
      return mWaylandCompositor;
    }
    else {
      E("null reference")
    }
  }

  zxdg_decoration_manager_v1* AUI::WaylandDecorationManager() const {
    if(mWaylandDecorationManager) return mWaylandDecorationManager;
    else E("null reference")
  }

  xdg_wm_base* AUI::WaylandXdgBase() const {
    D2("{}", (uint64_t) mWaylandXdgBase)
    if(mWaylandXdgBase != nullptr) {
      return mWaylandXdgBase;
    }
    else E("null reference")
  }

  wl_shm* AUI::WaylandShm() const {
    if(mWaylandShm != nullptr) return mWaylandShm;
    else E("null reference")
  }

  AWindow* AUI::WaylandFindWindow(wl_surface *surface) {
    UNUSED uint64_t id = reinterpret_cast<uint64_t>(surface);
    auto it = mWaylandSurfaceMap.find(id);
    if(it != mWaylandSurfaceMap.end()) {
      return it->second.get();
    }
    D1("not found window {}", id)
    return nullptr;
  }

}// namespace aui
//
