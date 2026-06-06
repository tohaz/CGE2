#ifndef AUILIB_H_
#define AUILIB_H_
#include "Custom/obj/xdg-shell-client-protocol.h"
#include "Custom/obj/xdg-decoration-unstable-v1-client-protocol.h"
#include <freetype/config/ftheader.h>
#include <freetype/freetype.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <xcb/xcb_image.h>
#include <execinfo.h>
#include <cerrno>
#include <chrono>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cxxabi.h>
#include <dirent.h>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <poll.h>
#include <print>
#include <regex>
#include <stack>
#include <sstream>
#include <type_traits>
#include <string>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <wayland-client.h>
#include <xcb/xcb.h>
#include "defaults.h"
#include "AWindow.h"
#include "AWidget.h"
#include "ABox.h"
#include "AButton.h"
#include "ALabel.h"
#include "AList.h"
#include "AScrollBar.h"
#include "IWindowContext.h"
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

class AUI {
    friend class AWindow;
    friend class AWidget;
    friend class WaylandWindowContext;
private:
  AUI();   // private constructor
  AUIWindowType mWindowType = AUIWindowType::XCB;
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
//  bool mXcbInitialized = false;
  bool mXcbOwned = false;
  void InitXcb();
  AWindow* mFocusedWindow = nullptr;
  int32_t mLastPointerX = 0;
  int32_t mLastPointerY = 0;
  std::vector<AWindow*> mPendingDrawWindows;
  std::mutex mPendingDrawMutex;
protected:
  int32_t GetConnectionFileDescriptor() const;
  uint64_t GenerateUniqueId() { return mNextId++; }
  std::mutex& GetCommandMutex() { return mCommandMutex; }
  std::vector<DrawCommand>& GetDrawCommands() { return mDrawCommands; }

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
  AWindow* GetFocusedWindow() const { return mFocusedWindow; }
  void SetFocusedWindow(AWindow* win) { mFocusedWindow = win; }
  int32_t GetLastPointerX() const { return mLastPointerX; }
  int32_t GetLastPointerY() const { return mLastPointerY; }
  void SetLastPointerPos(int32_t x, int32_t y) { mLastPointerX = x; mLastPointerY = y; }
  size_t GetDrawCommandsSize() {return mDrawCommands.size();}
  void ClearDrawCommandsForWindow(uint64_t nativeId);
  void ScheduleDraw(AWindow* win);
  void FlushPendingDraws();
};

} // namespace aui

#endif // AUILIB_H_

