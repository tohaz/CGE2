#ifndef AUILIB_H_
#define AUILIB_H_

#define FT_CONFIG_OPTION_CACHE

#include "Custom/obj/xdg-shell-client-protocol.h"
#include "Custom/obj/xdg-decoration-unstable-v1-client-protocol.h"
#include <X11/keysym.h>
#include <freetype/config/ftheader.h>
#include <freetype/freetype.h>
#include <freetype/ftmm.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <xcb/sync.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_cursor.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-x11.h>
#include <xcb/xcb_keysyms.h>
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <poll.h>
#include <print>
#include <random>
#include <regex>
#include <source_location>
#include <shared_mutex>
#include <unordered_map>
#include <vector>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include "defaults.h"
#include "AWidget.h"
#include "AWindow.h"
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
#include "XcbWindowContext.h"
#include "WaylandWindowContext.h"
struct wl_display;
struct wl_compositor;
struct wl_shm;
struct xdg_wm_base;
struct wl_registry;
struct xcb_connection_t;
struct xcb_screen_t;
struct zxdg_decoration_manager_v1;
#include FT_CACHE_H

namespace aui {
  UNUSED static const char* g_FontPaths[] = {
      "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
      "/usr/share/fonts/DejaVuSans-Bold.ttf" };
  //Raleway-VariableFont_wght.ttf
  extern "C" const uint8_t _binary_fonts_Raleway_VariableFont_wght_ttf_start[];
  extern "C" const uint8_t _binary_fonts_Raleway_VariableFont_wght_ttf_end[];
  extern "C" const uint8_t _binary_fonts_mousepointer1_png_start[];
  extern "C" const uint8_t _binary_fonts_mousepointer1_png_end[];
  extern "C" const uint8_t _binary_fonts_mousepointerH_png_start[];
  extern "C" const uint8_t _binary_fonts_mousepointerH_png_end[];
  extern "C" const uint8_t _binary_fonts_mousepointerV_png_start[];
  extern "C" const uint8_t _binary_fonts_mousepointerV_png_end[];
  static size_t g_EmbeddedFontSize = (size_t) _binary_fonts_Raleway_VariableFont_wght_ttf_end
      - (size_t) _binary_fonts_Raleway_VariableFont_wght_ttf_start;
  static size_t g_EmbeddedCursorSize =
      static_cast<size_t>(reinterpret_cast<uintptr_t>(_binary_fonts_mousepointer1_png_end)
          - reinterpret_cast<uintptr_t>(_binary_fonts_mousepointer1_png_start));
  constexpr uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325ULL;
  constexpr uint64_t FNV_PRIME = 0x00000100000001B3ULL;
// Generates a 64-bit hash from a string_view at compile time or runtime
  constexpr uint64_t hash64(std::string_view str) noexcept {
    uint64_t hash = FNV_OFFSET_BASIS;
    for(char c : str) {
      hash ^= static_cast<uint64_t>(c);
      hash *= FNV_PRIME;
    }
    return hash;
  }
  constexpr uint64_t operator""_hash(const char *str, std::size_t len) noexcept {
    return hash64(std::string_view(str, len));
  }

  bool DetectWayland();
  bool DetectXWayland();
  bool DetectX11();
  std::string XCBConnectErrorToString(int32_t error_code);
  std::string XCB_EventTypeToString(uint8_t response_type);
  uint32_t GetRandomARGBColor();
  uint32_t GetDistinctRandomARGBColor(uint32_t oldColor, uint8_t standOut);
  uint32_t HLColor(uint32_t ci);
  uint32_t DarkenColor(uint32_t ci);
  AUIKeyCode translate_keysym_to_keycode(xcb_keysym_t sym);
  AUIModifier translate_modifiers(uint16_t state);
  AUIKeyCode translate_keysym(xcb_keysym_t sym);
  std::string NumberToBaseString(uint64_t n);

  void wayland_pointer_handle_enter(void *data, struct wl_pointer *pointer,
      uint32_t serial, struct wl_surface *surface, wl_fixed_t sx_w, wl_fixed_t sy_w);
  void wayland_pointer_handle_leave(void *data, struct wl_pointer *pointer, uint32_t serial,
      struct wl_surface *surface);
  void wayland_pointer_handle_motion(void *data, struct wl_pointer *pointer, uint32_t time,
      wl_fixed_t sx_w, wl_fixed_t sy_w);
  void wayland_pointer_handle_button(void *data, struct wl_pointer *pointer, uint32_t serial,
      uint32_t time,
      uint32_t button, uint32_t state);
  void wayland_pointer_handle_axis(void *data, struct wl_pointer *pointer, uint32_t time,
      uint32_t axis, wl_fixed_t value);


class AUI;
//
int32_t find_closest_strike(FT_Face face, int32_t target_ppem);

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
//
class AUI {
    friend class AWindow;
    friend class XCBWindowContext;
    private:
      AUI();
      bool InitFreeType();
      bool InitWayland();
      bool InitXCB();
//      void WaylandRoundtrip();
      void WaylandRoundtrip(uint32_t);
      void CreateFrame(std::string title);
      pollfd mFDs[AUI_FD_NUM] { -1, -1, -1 };
      int32_t mNFDs = AUI_FD_NUM;
      int32_t mSelfPipeFDs[AUI_FD_NUM - 1] { -1, -1 };
      int32_t mWaylandFD = -1;
      int32_t mXCBFD = -1;
      bool mX11_Owned = false;
      AWindow* mMainWnd = nullptr;
      AUIWindowType mMainBackendType = AUIWindowType::unset;
      std::map<uint64_t, std::unique_ptr<AWindow>> mXCBWindowMap;// key = xcb_window_t
      std::map<uint64_t, std::unique_ptr<AWindow>> mWaylandSurfaceMap;
//  // XCB resources
      xcb_connection_t* mX11Connection = nullptr;
      xcb_screen_t* mX11Screen = nullptr;
      FT_Library mFtLibrary = nullptr;
      FT_Face mFtDefaultFace = nullptr;
      FT_Face mFallbackFace = nullptr;
      FTC_Manager mFTCManager = nullptr;
      FTC_ImageCache mFTCImageCache = nullptr;
      FTC_CMapCache mFTCCMapCache = nullptr;
      static constexpr FT_UInt kMaxFaces = 4;
      static constexpr FT_UInt kMaxSizes = 8;
      static constexpr FT_ULong kMaxBytes = 16 * 1024 * 1024;// 16 MB
      wl_display* mWaylandDisplay = nullptr;
      wl_compositor* mWaylandCompositor = nullptr;
      wl_shm* mWaylandShm = nullptr;
      xdg_wm_base* mWaylandXdgBase = nullptr;
      wl_registry* mWaylandRegistry = nullptr;
      wl_seat* mWaylandSeat = nullptr;
      wl_keyboard* mWaylandKeyboard = nullptr;
      wl_surface* mWaylandFocusedSurface = nullptr;
      wl_pointer* mWaylandPointer = nullptr;
      zxdg_decoration_manager_v1* mWaylandDecorationManager = nullptr;
      uint32_t mWaylandPointerSerial = 0;
      int32_t mLastPointerX = 0;
      int32_t mLastPointerY = 0;
      bool mShouldExit = false;
      bool mProcessingMessages = false;
      bool mIsWayland = false;
      bool mIsX11 = false;
      bool mBackendsDetected = false;
      AWindow* mFocusedWindow = nullptr;
      void XCBProcessMessages();
      bool XCBUnregisterWindow(UNUSED uint64_t nativeID);
      std::thread::id mMainThreadId;
      std::thread::id MainThreadId() const {return mMainThreadId;}
      bool HandleSelfPipe();
      // TODO move to instrumentation too
      uint64_t mWakeupCounter = 0;
      xkb_context* mXkbCtx = nullptr;
      xkb_keymap* mXkbKeymap = nullptr;
      xkb_state* mXkbState = nullptr;
    protected:
      void RegisterWindow(uint64_t nativeId, std::unique_ptr<AWindow> win);
      xcb_connection_t* X11Connection() {return mX11Connection;};
      xcb_screen_t* X11Screen() {return mX11Screen;};
    public:
      ~AUI();
      void ExitAUI();
      static AUI* Create(const std::string &windowTitle);
      static AUI* Create(const std::string &windowTitle, AUIWindowType backendType);
      void ProcessMessages();
      bool IsProcessingMessages() const { D1() return mProcessingMessages; }
      AUIWindowType MainBackendType() const;
      void WaylandSeat(wl_seat* seat);
      void WaylandXdgBase(xdg_wm_base* base);
      void WaylandCompositor(wl_compositor* comp);
      void WaylandShm(wl_shm* shm);
      void WaylandDecorationManager(zxdg_decoration_manager_v1* mgr);
      bool WaylandUnregisterWindow(uint64_t nativeId);
      void WaylandAddRegistryListener();
      AWindow* WaylandFindWindow(wl_surface *surface);
      wl_display* WaylandDisplay() const;
      wl_compositor* WaylandCompositor() const;
      zxdg_decoration_manager_v1* WaylandDecorationManager() const;
      xdg_wm_base* WaylandXdgBase() const;
      wl_shm* WaylandShm() const;
      wl_surface* WaylandFocusedSurface() {return mWaylandFocusedSurface;}
      void WaylandFocusedSurface(wl_surface* s) {mWaylandFocusedSurface = s;}
      void WaylandPointerSerial(uint32_t s) {mWaylandPointerSerial = s;}
      uint32_t WaylandPointerSerial() {return mWaylandPointerSerial;}
      wl_pointer* WaylandPointer() {return mWaylandPointer;}
      void WaylandPointer(wl_pointer* v) {mWaylandPointer = v;}
      FT_Face DefaultFontFace() const {D4() return mFtDefaultFace;}
      bool IsWayland() {return mIsWayland;}
      bool IsX11() {return mIsX11;}
      void DetectBackends();
      AWindow* X11FindWindow(uint64_t nativeId) const;
      AWindow* FindWindowByNativeId(uint64_t id, AUIWindowType w);
      AWindow* MainWnd();
      FT_Face FallbackFace() const {return mFallbackFace;}
      void RequestRedraw();
      AWindow* FocusedWindow() {return mFocusedWindow;}
      void FocusedWindow(AWindow* w) {mFocusedWindow = w;}
      void UpdatePointerTracking(int32_t x, int32_t y) {mLastPointerX = x; mLastPointerY = y;}
      int32_t LastPointerX() {return mLastPointerX;}
      int32_t LastPointerY() {return mLastPointerY;}
      uint64_t WakeupCounter() {return mWakeupCounter;};
      void UpdateLayout();
      xkb_context* XkbContext() const {return mXkbCtx;}
      void XkbContext(xkb_context* v) {mXkbCtx = v;}
      xkb_keymap* XkbKeymap() const {return mXkbKeymap;}
      void XkbKeymap(xkb_keymap* v) {mXkbKeymap = v;}
      xkb_state* XkbState() const {return mXkbState;}
      void XkbState(xkb_state* v) {mXkbState = v;}


#ifdef AUI_UNIT_TEST
private:
//    int32_t mScheduleDrawCount = 0;
public:
    FT_Library* FtLibrary() {D4() return &mFtLibrary;}
//    int32_t GetScheduleDrawCount() const { return mScheduleDrawCount; }
//    void ResetScheduleDrawCount() { mScheduleDrawCount = 0; }

#endif
};
//
// Fill a rectangle with a solid color.
// All coordinates and sizes are assumed to be already clipped to the buffer bounds.
  inline void FillRect(uint32_t *buffer, uint32_t bufferWidth, int32_t x, int32_t y, int32_t w, int32_t h,
      uint32_t color) {
    D4("bufferWidth {} x {} y {} w {} h {} color {}", bufferWidth, x, y, w, h, color);
    if(w <= 0 || h <= 0)
      return;
    for(int32_t row = 0; row < h; ++row) {
      uint32_t* line = buffer + static_cast<size_t>(y + row) * bufferWidth + static_cast<size_t>(x);
      std::fill(line, line + w, color);
    }
  }

  struct ThreadData {
    std::function<int32_t(AUI*)> func;
    std::string name;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    int32_t result = 0;
    bool done = false;
  };

  static void* thread_routine(void *arg) {
    auto data = std::unique_ptr<std::shared_ptr<ThreadData>>(
        static_cast<std::shared_ptr<ThreadData>*>(arg)
    );
    auto shared_data = *data;
    // Force cancellation to happen immediately at any cancellation point
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, nullptr); // Cancel anywhere
    std::unique_ptr<AUI> au;
    {
    ST1("{}", shared_data->name);
      au.reset(AUI::Create(shared_data->name));
    }
    int32_t ret = shared_data->func(au.get());
    pthread_mutex_lock(&shared_data->mutex);
    shared_data->result = ret;
    shared_data->done = true;
    pthread_cond_signal(&shared_data->cond);
    pthread_mutex_unlock(&shared_data->mutex);
    return nullptr;
  }

  template<typename Func>
  int32_t run_with_timeout(const char *name, Func &&func, int32_t timeout_ms) {
    auto data = std::make_shared<ThreadData>();
    data->func = std::forward<Func>(func);
    data->name = name;
    auto thread_arg = new std::shared_ptr<ThreadData>(data);
    pthread_t thread;
    if (pthread_create(&thread, nullptr, thread_routine, thread_arg) != 0) {
      delete thread_arg;
      return 1;
    }
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
      ts.tv_sec += 1;
      ts.tv_nsec -= 1000000000;
    }
    pthread_mutex_lock(&data->mutex);
    while (!data->done) {
      if (pthread_cond_timedwait(&data->cond, &data->mutex, &ts) == ETIMEDOUT) {
        break;
      }
    }
    bool completed = data->done;
    pthread_mutex_unlock(&data->mutex);
    if (!completed) {
      D("+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=++=+=+=+=++=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=++=+=+=+=++=+=Function {} timed out – aborting.", name);
      // 1. Try to cancel it
      pthread_cancel(thread);
      // 2. Detach it so we DO NOT block waiting for it to respond
      pthread_detach(thread);
      exit(1); // Returns instantly, never hangs
    }
    pthread_join(thread, nullptr);
    return data->result;
  }
#define RUN_WITH_TIMEOUT(func, timeout) run_with_timeout(#func, func, timeout)

  template <typename TFunc>
  int32_t runTimedTestImpl(const char* testname, TFunc&& testlogic, uint32_t rtime) {
    int32_t testresult = 0;
    D1("====================================================={}, watchdog {} ms", testname, rtime);
    ST("{}", testname);
    AUI* au = AUI::Create(reinterpret_cast<const char*>(testname));
    if (!au) return -1;
    std::mutex cv_mtx;
    std::condition_variable cv;
    bool test_finished = false;
    std::thread worker([au, rtime, &cv, &cv_mtx, &test_finished]() {
      std::unique_lock<std::mutex> lock(cv_mtx);
      bool timed_out = !cv.wait_for(lock, std::chrono::milliseconds(rtime), [&] { return test_finished; });
      if (timed_out && au) {
        D3("ExitAUI() starts");
        au->ExitAUI();
        D3("ExitAUI() ends");
      }
    });
    testresult = testlogic(au);
    D2("test func exited");
    au->ProcessMessages();
    D2("ProcessMessages() exited");
    {
      std::lock_guard<std::mutex> lock(cv_mtx);
      test_finished = true;
    }
    D2("lock passed");
    cv.notify_one();
    worker.join();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    delete au;
    return testresult;
  }
  #define runTimedTest(func, rtime) aui::runTimedTestImpl(#func, func, rtime)

} // namespace aui

#endif // AUILIB_H_

