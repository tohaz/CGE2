#ifndef AUILIB_H_
#define AUILIB_H_

#define FT_CONFIG_OPTION_CACHE
#include "Custom/obj/xdg-shell-client-protocol.h"
#include "Custom/obj/xdg-decoration-unstable-v1-client-protocol.h"
#include <X11/keysym.h>
#include <freetype/config/ftheader.h>
#include <freetype/freetype.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_cursor.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <xcb/xcb_keysyms.h>
#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <charconv>
#include <condition_variable>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <execinfo.h>
#include <flat_map>
#include <format>
#include <future>
#include <dirent.h>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <poll.h>
#include <print>
#include <random>
#include <regex>
#include <source_location>
#include <string>
#include <string_view>
#include <shared_mutex>
#include <string.h>
#include <stack>
#include <sstream>
#include <thread>
#include <type_traits>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <xcb/xcb.h>
#include "defaults.h"
#include "AWindow.h"
#include "AWidget.h"
#include "ABox.h"
#include "AButton.h"
#include "ALabel.h"
#include "AScrollBar.h"
#include "AList.h"
#include "AInputBox.h"
#include "ATable.h"
#include "AComboBox.h"
#include "AMenu.h"
#include "AProgressBar.h"
#include "IWindowContext.h"
#include "XcbWindowContext.h"
#include "WaylandWindowContext.h"

#include FT_CACHE_H

struct wl_display;
struct wl_compositor;
struct wl_shm;
struct xdg_wm_base;
struct wl_registry;
struct xcb_connection_t;
struct xcb_screen_t;
struct zxdg_decoration_manager_v1;

namespace aui {

class AWindow;

enum class DrawCommandType { Xcb, Wayland };

struct DrawCommandXcb {
  uint32_t windowId;    // xcb_window_t
  uint32_t* buffer;     // raw pixel data (XRGB)
  uint32_t width;
  uint32_t height;
};

struct DrawCommandWayland {
  wl_surface* surface;
  wl_buffer* buffer;    // pre-created SHM buffer
  uint32_t width;
  uint32_t height;
};

struct DrawCommand {
  DrawCommandType type;
  union {
    DrawCommandXcb xcb;
    DrawCommandWayland wayland;
  };
};

  struct CachedGlyph {
      uint8_t *bitmap;// null if only metrics are stored
      int32_t width;
      int32_t rows;
      int32_t left;
      int32_t top;
      int32_t advance;
      int32_t asc;// bitmap_top
      int32_t desc;// bitmap.rows - bitmap_top
  };
  struct ARect;
  struct ATextStyle;


  AUIKeyCode translate_keysym_to_keycode(xcb_keysym_t sym);
  AUIModifier translate_modifiers(uint16_t state);
  AUIKeyCode translate_keysym(xcb_keysym_t sym);
  std::string NumberToBaseString(UINT64 n);


  int32_t find_closest_strike(FT_Face face, int32_t target_ppem);
  uint8_t* scale_bgra_bitmap(const uint8_t* src, int32_t srcW, int32_t srcH,
                                    int32_t dstW, int32_t dstH, int32_t srcPitch,
                                    int32_t& dstPitch);
  void ClipRect(int32_t &x, int32_t &y, int32_t &w, int32_t &h, int32_t parentW, int32_t parentH);
  void DrawTextEx(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t absX, int32_t absY,
      int32_t drawW, int32_t drawH, const std::string &text, FT_Face face, uint32_t fontSize, AUIHAlign hAlign,
      AUIVAlign vAlign, int32_t xOffset, uint32_t textColor, int32_t maxContentWidth);

  void DrawTextEx(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, const ARect &bounds,
      const std::string &text, FT_Face face, const ATextStyle &style);

#pragma GCC push_options
#pragma GCC optimize ("O2")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"

  UNUSED static __attribute__((always_inline))  inline int16_t SafeINT16(uint16_t val) {
    if(val > static_cast<uint16_t>(std::numeric_limits<int16_t>::max()))
[[unlikely]] {
              E("UINT16 to INT16 conversion error");
    }
    return static_cast<int16_t>(val);
  }

  static __attribute__((always_inline))  inline int16_t SafeINT16(int32_t val) {
    if(val > std::numeric_limits<int16_t>::max() || val < std::numeric_limits<int16_t>::min())
[[unlikely]] {
              E("INT32 to INT16 conversion error");
    }
    return static_cast<int16_t>(val);
  }

  static __attribute__((always_inline))  inline int16_t SafeINT16(uint32_t val) {
    if(val > static_cast<uint32_t>(std::numeric_limits<int16_t>::max()))
[[unlikely]] {
              E("UINT32 to INT16 conversion error");
    }
    return static_cast<int16_t>(val);
  }

  static __attribute__((always_inline))  inline int16_t SafeINT16(int64_t val) {
    if(val > std::numeric_limits<int16_t>::max() || val < std::numeric_limits<int16_t>::min())
[[unlikely]] {
              E("INT64 to INT16 conversion error");
    }
    return static_cast<int16_t>(val);
  }

  static __attribute__((always_inline))  inline uint16_t SafeUINT16(uint32_t val) {
    if(val > std::numeric_limits<uint16_t>::max())
[[unlikely]] {
              E("UINT32 to UINT16 conversion error");
    }
    return static_cast<uint16_t>(val);
  }

  static __attribute__((always_inline))  inline uint16_t SafeUINT16(int64_t val) {
    if(val < 0 || val > static_cast<int64_t>(std::numeric_limits<uint16_t>::max()))
[[unlikely]] {
              E("INT64 to UINT16 conversion error");
    }
    return static_cast<uint16_t>(val);
  }

  static __attribute__((always_inline))  inline int32_t SafeINT32(uint32_t val) {
    if(val > static_cast<uint32_t>(std::numeric_limits<int32_t>::max()))
[[unlikely]] {
              E("UINT32 to INT32 conversion error");
    }
    return static_cast<int32_t>(val);
  }

  static __attribute__((always_inline))  inline int32_t SafeINT32(int64_t val) {
    if(val > std::numeric_limits<int32_t>::max() || val < std::numeric_limits<int32_t>::min())
[[unlikely]] {
              E("INT64 to INT32 conversion error");
    }
    return static_cast<int32_t>(val);
  }

  static __attribute__((always_inline))  inline int32_t SafeINT32(uint64_t val) {
    if(val > static_cast<uint64_t>(std::numeric_limits<int32_t>::max()))
[[unlikely]] {
              E("UINT64 to INT32 conversion error");
    }
    return static_cast<int32_t>(val);
  }

  static __attribute__((always_inline))  inline uint32_t SafeUINT32(int32_t val) {
    if(val < 0)
[[unlikely]] {
              E("INT32 to UINT32 conversion error (negative)");
    }
    return static_cast<uint32_t>(val);
  }

  static __attribute__((always_inline))  inline uint32_t SafeUINT32(int64_t val) {
    if(val > static_cast<int64_t>(std::numeric_limits<uint32_t>::max()) || val < 0)
[[unlikely]] {
              E("INT64 to UINT32 conversion error");
    }
    return static_cast<uint32_t>(val);
  }

  static __attribute__((always_inline))  inline uint32_t SafeUINT32(uint64_t val) {
    if(val > static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()))
[[unlikely]] {
              E("UINT64 to UINT32 conversion error");
    }
    return static_cast<uint32_t>(val);
  }

  static __attribute__((always_inline))  inline int64_t SafeINT64(uint64_t val) {
    if(val > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
[[unlikely]] {
              E("UINT64 to INT64 conversion error");
    }
    return static_cast<int64_t>(val);
  }

  static __attribute__((always_inline))  inline uint64_t SafeUINT64(int32_t val) {
    if(val < 0)
[[unlikely]] {
              DS();
      E("INT32 to UINT64 conversion error");
    }
    return static_cast<uint64_t>(val);
  }

  static __attribute__((always_inline))  inline uint64_t SafeUINT64(uint32_t val) {
    return static_cast<uint64_t>(val);
  }

  static __attribute__((always_inline))  inline uint64_t SafeUINT64(int64_t val) {
    if(val < 0)
[[unlikely]] {
              E("INT64 to UINT64 conversion error");
    }
    return static_cast<uint64_t>(val);
  }

  static __attribute__((always_inline))  inline int32_t SafeINT32(double val) {
    if(std::isnan(val) || val > static_cast<double>(std::numeric_limits<int32_t>::max())
        || val < static_cast<double>(std::numeric_limits<int32_t>::min()))
[[unlikely]] {
              E("double to INT32 conversion error (overflow or NaN)");
    }
    return static_cast<int32_t>(val);
  }

  static __attribute__((always_inline))  inline uint32_t SafeUINT32(double val) {
    if(std::isnan(val) || val > static_cast<double>(std::numeric_limits<uint32_t>::max()) || val < 0.0)
[[unlikely]] {
              E("double to UINT32 conversion error (overflow, negative, or NaN)");
    }
    return static_cast<uint32_t>(val);
  }

  static __attribute__((always_inline))  inline int64_t SafeINT64(double val) {
    if(std::isnan(val) || val > static_cast<double>(std::numeric_limits<int64_t>::max())
        || val < static_cast<double>(std::numeric_limits<int64_t>::min()))
[[unlikely]] {
              E("double to INT64 conversion error (overflow or NaN)");
    }
    return static_cast<int64_t>(val);
  }

#pragma GCC diagnostic pop
#pragma GCC pop_options



class AUI {
    friend class AWindow;
    friend class AWidget;
    friend class WaylandWindowContext;
private:
  AUI();   // private constructor
  AUIWindowType mWindowType = AUIWindowType::unset;
  // XCB resources
  xcb_connection_t* mXcbConnection = nullptr;
  xcb_screen_t* mXcbScreen = nullptr;
  // Windows
  AWindow* mMainWnd = nullptr;
  // Draw command queue
  std::vector<DrawCommand> mDrawCommands;
  std::mutex mCommandMutex;
  // FreeType
  FT_Library mFtLibrary = nullptr;
  FT_Face mFtDefaultFace = nullptr;
  FT_Face mFallbackFace = nullptr;
  FTC_Manager mFTCManager = nullptr;
  FTC_ImageCache mFTCImageCache = nullptr;
  FTC_CMapCache mFTCCMapCache = nullptr;
  static constexpr FT_UInt kMaxFaces = 4;
  static constexpr FT_UInt kMaxSizes = 8;
  static constexpr FT_ULong kMaxBytes = 16 * 1024 * 1024; // 10 MB
  uint64_t mNextId = 1000000U;
  // Wayland resources
  wl_display* mWaylandDisplay = nullptr;
  wl_compositor* mWaylandCompositor = nullptr;
  wl_shm* mWaylandShm = nullptr;
  xdg_wm_base* mWaylandXdgBase = nullptr;
  wl_registry* mWaylandRegistry = nullptr;
  wl_seat* mWaylandSeat = nullptr;
  wl_pointer* mWaylandPointer = nullptr;
  zxdg_decoration_manager_v1* mWaylandDecorationManager = nullptr;
  bool mShouldExit = false;
  int32_t mSelfPipeFds[2];
  std::map<uint64_t, std::unique_ptr<AWindow>> mXcbWindowMap; // key = xcb_window_t
  std::map<uint64_t, std::unique_ptr<AWindow>> mWaylandSurfaceMap;
  void RegisterWindow(uint64_t nativeId, std::unique_ptr<AWindow> win);
  bool mXcbOwned = false;
  void InitXcb();
  AWindow* mFocusedWindow = nullptr;
  int32_t mLastPointerX = 0;
  int32_t mLastPointerY = 0;
  std::vector<AWindow*> mPendingDrawWindows;
  std::mutex mPendingDrawMutex;
  std::unordered_map<uint64_t, FT_Fixed> mAdvanceCache;
  std::unordered_map<uint64_t, std::pair<int32_t, int32_t>> mMetricsCache; // key = (glyph_index<<32)|fontSize
  uint32_t mLastPointerSerial = 0;

protected:
  int32_t GetConnectionFileDescriptor() const;
  uint64_t GenerateUniqueId() { return mNextId++; }
  std::mutex& GetCommandMutex() { return mCommandMutex; }
  std::vector<DrawCommand>& GetDrawCommands() { return mDrawCommands; }
  wl_keyboard* mWaylandKeyboard = nullptr;
  xkb_context* mXkbCtx = nullptr;
  xkb_keymap* mXkbKeymap = nullptr;
  xkb_state* mXkbState = nullptr;
  UINT64 mDrawCounter = 0;
  std::unordered_map<uint64_t, CachedGlyph> mPreRenderedGlyphs;
  void PreRenderAscii(uint32_t fontSize);

public:
  void EnqueueDrawCommand(const DrawCommand& cmd);
  static AUI* Create(const std::string& windowTitle);
  ~AUI();
  AUI(const AUI&) = delete;
  AUI& operator=(const AUI&) = delete;
  void ProcessMessages();   // event loop (blocks, polls the correct connection)
  void Draw();              // executes all queued commands, then flushes
  AWindow* MainWnd() const { return mMainWnd; }
  void* GetNativeDisplay() const;                // returns xcb_connection_t* or wl_display*
  void FlushConnection();
  // XCB specific (null if not XCB)
  xcb_connection_t* GetXcbConnection() const { return mXcbConnection; }
  xcb_screen_t* GetXcbScreen() const { return mXcbScreen; }
  // Wayland specific (null if not Wayland)
  wl_display* GetWaylandDisplay() const { return mWaylandDisplay; }
  wl_compositor* GetWaylandCompositor() const { return mWaylandCompositor; }
  wl_shm* GetWaylandShm() const { return mWaylandShm; }
  xdg_wm_base* GetWaylandXdgBase() const { return mWaylandXdgBase; }
  FT_Face GetDefaultFontFace() const { return mFtDefaultFace; }
  void SetWaylandCompositor(wl_compositor* comp) { mWaylandCompositor = comp; }
  void SetWaylandShm(wl_shm* shm) { mWaylandShm = shm; }
  void SetWaylandXdgBase(xdg_wm_base* base) { mWaylandXdgBase = base; }
  zxdg_decoration_manager_v1* GetWaylandDecorationManager() const { return mWaylandDecorationManager; }
  void SetWaylandDecorationManager(zxdg_decoration_manager_v1* mgr) { mWaylandDecorationManager = mgr; }
  AUIWindowType GetWindowType() const { return mWindowType; }
  void ExitAUI();
  void UnregisterWindow(uint64_t nativeId);
  AWindow* FindWindowByNativeId(uint64_t nativeId) const;
  xcb_connection_t* GetXcbConnection();
  xcb_screen_t* GetXcbScreen();
  // public wayland getters/setters:
  wl_seat* GetWaylandSeat() const { return mWaylandSeat; }
  void SetWaylandSeat(wl_seat* seat) { mWaylandSeat = seat; }
  wl_pointer* GetWaylandPointer() const { return mWaylandPointer; }
  void SetWaylandPointer(wl_pointer* ptr) { mWaylandPointer = ptr; }
  int32_t GetLastPointerX() const { return mLastPointerX; }
  int32_t GetLastPointerY() const { return mLastPointerY; }
  void SetLastPointerPos(int32_t x, int32_t y) { mLastPointerX = x; mLastPointerY = y; }
  size_t GetDrawCommandsSize() {return mDrawCommands.size();}
  void ClearDrawCommandsForWindow(uint64_t nativeId);
  void ScheduleDraw(AWindow* win);
  void FlushPendingDraws();
  wl_keyboard* GetWaylandKeyboard() const { return mWaylandKeyboard; }
  void SetWaylandKeyboard(wl_keyboard* kbd) { mWaylandKeyboard = kbd; }
  xkb_context* GetXkbContext() const { return mXkbCtx; }
  void SetXkbContext(xkb_context* ctx) { mXkbCtx = ctx; }
  xkb_keymap* GetXkbKeymap() const { return mXkbKeymap; }
  void SetXkbKeymap(xkb_keymap* km) { mXkbKeymap = km; }
  xkb_state* GetXkbState() const { return mXkbState; }
  void SetXkbState(xkb_state* st) { mXkbState = st; }
  AWindow* GetFocusedWindow() const { return mFocusedWindow; }
  void SetFocusedWindow(AWindow* win) { mFocusedWindow = win; }
  FT_Glyph GetCachedGlyph(FT_UInt glyph_index, FT_UInt font_size, FT_Int load_flags = FT_LOAD_DEFAULT);
  FT_Fixed GetCachedAdvance(FT_UInt glyph_index, FT_UInt font_size);
  const CachedGlyph* GetPreRenderedGlyph(uint32_t ch, uint32_t fontSize) const;
  std::unordered_map<uint64_t, std::pair<int32_t, int32_t>>& GetMetricsCache() { return mMetricsCache; }
  void SetLastPointerSerial(uint32_t serial) { mLastPointerSerial = serial; }
  uint32_t GetLastPointerSerial() const { return mLastPointerSerial; }
  FT_Face GetFallbackFace() const { return mFallbackFace; }
  const uint8_t* GetScaledEmoji(FT_Face face, FT_UInt glyph_index, uint32_t size, int32_t& outWidth, int32_t& outHeight, int32_t& outPitch);
  std::unordered_map<uint64_t, std::vector<uint8_t>> mScaledEmojiCache; // key: ((glyph_index << 32) | size)
};

} // namespace aui

#endif // AUILIB_H_

