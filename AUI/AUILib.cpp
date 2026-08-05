#include "AUILib.h"

extern "C" {
  UNUSED static FT_Error ftc_face_requester(UNUSED FTC_FaceID face_id,
  UNUSED FT_Library library, UNUSED FT_Pointer request_data, UNUSED FT_Face* aface) {
    UNUSED aui::AUI* au = static_cast<aui::AUI*>(request_data);
    *aface = au->DefaultFontFace();
    return 0;
  }
}

namespace aui {
  bool AUI::InitFreeType() {
    ST2("")
    if(FT_Init_FreeType(&mFtLibrary) != 0)
      E("FT_Init_FreeType failed")
    FT_Error err = FT_Err_Unknown_File_Format;
    for(size_t i = 0; i < sizeof(g_FontPaths) / sizeof(g_FontPaths[0]); ++i) {
      err = FT_New_Face(mFtLibrary, g_FontPaths[i], 0, &mFtDefaultFace);
      if(err == 0) {
        D2("Loaded system font: {}", g_FontPaths[i]);
        break;
      }
    }
    if(err != 0) {
      if(g_EmbeddedFontSize > 0) {
        D1("loading embedded main fallback font")
//Raleway_VariableFont_wght_ttf
        err = FT_New_Memory_Face(mFtLibrary, _binary_fonts_Raleway_VariableFont_wght_ttf_start,
            (int64_t) g_EmbeddedFontSize, 0, &mFtDefaultFace);
        if(err == 0) {
          D1("Loaded embedded fallback font");
// Adjust variable font weight on initialization
          FT_MM_Var* mmVar = nullptr;
          if(FT_Get_MM_Var(mFtDefaultFace, &mmVar) == 0) {
            std::vector<FT_Fixed> coords(mmVar->num_axis);
            for(FT_UInt i = 0; i < mmVar->num_axis; ++i) {
              coords[i] = mmVar->axis[i].def;
// 0x77676874U is the hardcoded Big-Endian calculation of 'w','g','h','t'
// This completely avoids using FT_MAKE_TAG which uses the banned 'unsigned' keyword
              if(mmVar->axis[i].tag == 0x77676874U) {
// Adjusting to bold (700). 16.16 fixed point formatting
                coords[i] = 700 * 65536;
              }
              else
                E("unexpected value in mmVar->axis[i].tag")
            }
            err = FT_Set_Var_Design_Coordinates(mFtDefaultFace, static_cast<FT_UInt>(coords.size()), coords.data());
            if(err != 0)
              E("error in FT_Set_Var_Design_Coordinates")
            err = FT_Done_MM_Var(mFtLibrary, mmVar);
            if(err != 0)
              E("error in FT_Done_MM_Var")
          }
          else
            E("error getting FT_MM_Var")
        }
        else
          E("error loadin memory fallback font")
      }
      else
        E("main fallback font is 0")
    }
    if(err != 0)
      E("No primary font")
    if(FT_Set_Pixel_Sizes(mFtDefaultFace, 0, 14))
      E("FT_Set_Pixel_Sizes")
// Load color emoji font (NotoColorEmoji)
    const char* emojiPath = "/usr/share/fonts/truetype/noto/NotoColorEmoji.ttf";
    err = FT_New_Face(mFtLibrary, emojiPath, 0, &mFallbackFace);
    if(err == 0) {
      D2("Loaded fallback font: {}", emojiPath);
      err = FT_Select_Charmap(mFallbackFace, FT_ENCODING_UNICODE);
      if(err != 0)
        E("FT_Select_Charmap failed")
      if(FT_IS_SCALABLE(mFallbackFace)) {
        err = FT_Set_Pixel_Sizes(mFallbackFace, 0, 14);
        if(err != 0)
          E("emoji FT_Set_Pixel_Sizes failed")
      }
      else {
        int32_t strike = find_closest_strike(mFallbackFace, 14);
        if(strike >= 0) {
          err = FT_Select_Size(mFallbackFace, strike);
          if(err != 0)
            E("FT_Select_Size failed")
        }
      }
    }
    else
      E("failed creating FT emoji face")
// FreeType cache manager
    err = FTC_Manager_New(mFtLibrary, AUI::kMaxFaces, AUI::kMaxSizes, AUI::kMaxBytes, ftc_face_requester, this,
        &mFTCManager);
    if(err != 0)
      E("FTC_Manager_New failed")
    else {
      err = FTC_ImageCache_New(mFTCManager, &mFTCImageCache);
      if(err != 0)
        E("FTC_ImageCache_New failed")
    }
    mFtDefaultFace->generic.data = this;
    return true;
  }

  AUI::AUI() {
    ST2("")
    {
      ST4("InitFreeType()")
      if(!InitFreeType())
        E("Freetype init failed")
    }
    DetectBackends();
    if(pipe(mSelfPipeFDs) != 0) {
      mSelfPipeFDs[0] = mSelfPipeFDs[1] = -1;
      E("pipe error")
    }
    mFDs[AUI_SELFPIPE_FD_INDEX].fd = mSelfPipeFDs[0];
    mFDs[AUI_SELFPIPE_FD_INDEX].events = POLLIN;
    int32_t flags = fcntl(mSelfPipeFDs[0], F_GETFL, 0);
    fcntl(mSelfPipeFDs[0], F_SETFL, flags | O_NONBLOCK);
    mMainThreadId = std::this_thread::get_id();
  }

  AUI* AUI::Create(UNUSED const std::string& windowTitle, UNUSED AUIWindowType btype) {
    ST4("AUI::Create true")
    AUI* au = nullptr;
    AUIWindowType wt = AUIWindowType::unset;
    {
      ST4("AUI new()")
      if(!(au = new AUI())) {E("AUI instance creation failed")}
    }
    bool winit = false;
    switch(btype) {
      case AUIWindowType::Wayland:
        D1("Wayland window mode")
        if(!au->mIsWayland) {
          E("Wayland interface is not availiable")
        }
        {
          ST4("winit = au->InitWayland();");
          winit = au->InitWayland();
        }
        if(!winit) {
          E("Wayland init failed")
        }
        wt = AUIWindowType::Wayland;
        au->mMainBackendType = wt;
        if(au->mIsX11) {
          au->InitXCB();
        }
        D2("Wayland init done")
        break;
      case AUIWindowType::X11:
        D1("X11 window mode")
        if(au->mIsX11) {
          {
            ST4("InitXCB()")
            au->InitXCB();
          }
          wt = AUIWindowType::X11;
          au->mMainBackendType = wt;
        }
        else {
          E("X11 is not avaliable")
        }
        au->mIsWayland = false;
        break;
      case AUIWindowType::unset:
        E("backend unset")
        break;
      default:
        E("unknown backend type")
    }
    D2("primary backend {}", (uint32_t)wt)
    {
      ST4("CreateFrame")
      au->CreateFrame(windowTitle);
    }
    return au;
  }

  AUI* AUI::Create(UNUSED const std::string& windowTitle) {
    AUI* au = nullptr;
    ST4("AUI::Create");
    const char* forceX11 = getenv("AUI_FORCE_X11");
    if((DetectWayland() || DetectXWayland()) && (forceX11 == nullptr)) {
      ST4("choosing default Wayland")
      au = AUI::Create(windowTitle, AUIWindowType::Wayland);
    }
    if(DetectX11() || (forceX11 != nullptr)) {
      ST4("choosing default X11")
      au = AUI::Create(windowTitle, AUIWindowType::X11);
    }
    if(!au) {E("AUI Create failed")}
    return au;
  }

  void AUI::ProcessMessages() {
    mProcessingMessages = true;
    D2("Entering hybrid loop {}", (int32_t)mMainBackendType);
    UpdateLayout();
    while(!mShouldExit) {
      if(mIsWayland) {
        while(wl_display_prepare_read(mWaylandDisplay) != 0) {
          D1()
          wl_display_dispatch_pending(mWaylandDisplay);
        }
      }
      D2("before poll")
      UNUSED int32_t ret = poll(mFDs, static_cast<nfds_t>(mNFDs), -1);
      mWakeupCounter++;
      D2("after poll {} {} {}, {} {} {}, {} {} {}", mFDs[0].fd, mFDs[1].fd, mFDs[2].fd, mFDs[0].events, mFDs[1].events,
          mFDs[2].events, mFDs[0].revents, mFDs[1].revents, mFDs[2].revents)
      D4("poll ret=%d, pipe revents=%d", ret, mFDs[AUI_SELFPIPE_FD_INDEX].revents);
      if(mShouldExit || ret < 0) {
        if(mIsWayland) {
          wl_display_cancel_read(mWaylandDisplay);// Clean up the prepared read
        }
        D2("breaking from the cycle after poll()");
        break;
      }
      if(mIsWayland) {
        if(mFDs[AUI_WAYLAND_FD_INDEX].revents & POLLIN) {
          D2("Wayland POLLIN detected");
          wl_display_read_events(mWaylandDisplay);
          wl_display_dispatch_pending(mWaylandDisplay);
          if(wl_display_get_error(mWaylandDisplay) != 0) {
            E("Wayland display error detected, crashing AUI");
          }
        }
        else {
          wl_display_cancel_read(mWaylandDisplay);
        }
      }
      if(mIsX11 && (mFDs[AUI_XCB_FD_INDEX].revents & POLLIN)) {
        XCBProcessMessages();
      }
      D2("flushing pongs")
      if(mIsWayland) {
        wl_display_flush(mWaylandDisplay);
      }
      if(HandleSelfPipe()) {
        break;// exit immediately
      }
    }
    D2("ends");
    mProcessingMessages = false;
  }

  void AUI::CreateFrame(std::string title) {
    ST4("CreateFrame")
    if(mMainWnd == nullptr)
      mMainWnd = AWindow::AttachTo(this, title);
    else
      E("main window already created")
  }

  AUIWindowType AUI::MainBackendType() const {
    D2()
    if(mMainBackendType == AUIWindowType::unset)
      E("primary backend not initialized")
    return mMainBackendType;
  }

  void AUI::ExitAUI() {
    if(mShouldExit) {
      D2("exit additional call")
      return;
    }
    D2("AUI::ExitAUI() starts")
    mShouldExit = true;
    if(mSelfPipeFDs[1] >= 0) {
      UNUSED char token = 1;
      UNUSED size_t bytes = (size_t) write(mSelfPipeFDs[1], &token, 1);
    }
    else {
      E("pipe is closed on exit")
    }
    if(mWaylandDisplay) {
      wl_display_flush(mWaylandDisplay);
    }
    else {
      D2("not a wayland display on exit")
    }
  }

  void AUI::RegisterWindow(UNUSED uint64_t nativeID, UNUSED std::unique_ptr<AWindow> w) {
    D2("incoming window ID {}", nativeID)
    if(w->Type() == AUIWindowType::X11) {
      if(mXCBWindowMap.find(nativeID) != mXCBWindowMap.end()) {
        E("RegisterWindow: duplicate X11 window ID {}", nativeID);
      }
      mXCBWindowMap[nativeID] = std::move(w);
      D2("window map size {}", mXCBWindowMap.size())
    }
    else {
      if(mWaylandSurfaceMap.find(nativeID) != mWaylandSurfaceMap.end()) {
        E("RegisterWindow: duplicate Wayland surface ID {}", nativeID);
      }
      D2("registering Wayland window")
      mWaylandSurfaceMap[nativeID] = std::move(w);
    }
  }

  AWindow* AUI::MainWnd() {
    if(mMainWnd != nullptr)
      return mMainWnd;
    E("null pointer")
  }

  bool AUI::WaylandUnregisterWindow(UNUSED uint64_t nativeID) {
    D1("UnregisterWaylandWindow: nativeId={}", nativeID);
    AWindow* w = nullptr;
    auto it = mWaylandSurfaceMap.find(nativeID);
    if(it == mWaylandSurfaceMap.end()) {
      D("UnregisterWindow: Wayland window not found (nativeId={})", nativeID);
      return false;
    }
    w = it->second.get();
    if(mFocusedWindow == w) {
      mFocusedWindow = nullptr;
      D1("removing focus from window {}", nativeID)
    }
    mWaylandSurfaceMap.erase(it);
    return true;
  }

  AWindow* AUI::X11FindWindow(uint64_t nativeId) const {
    auto it = mXCBWindowMap.find(nativeId);
    if(it != mXCBWindowMap.end()) {
      return it->second.get();
    }
    D3("window {} not found", nativeId)
    return 0;
  }

  void AUI::DetectBackends() {
    ST4("DetectBackends")
    if(mBackendsDetected) {
      D1("Backends already detected")
      return;
    }
    if(DetectWayland())
      mIsWayland = true;
    if(DetectXWayland()) {
      mIsWayland = true;
      mIsX11 = true;
    }
    if(DetectX11()) {
      mIsX11 = true;
    }
    mBackendsDetected = true;
    D2("Detected backends:Wayland {}, X11 {}", mIsWayland, mIsX11)
  }

  void AUI::RequestRedraw() {
    D4("Write token 2");
    if(mSelfPipeFDs[1] >= 0) {
      int8_t token = 2;
      write(mSelfPipeFDs[1], &token, 1);
    }
  }

  bool AUI::HandleSelfPipe() {
//ST1("")
    if(!(mFDs[AUI_SELFPIPE_FD_INDEX].revents & POLLIN))
      return false;
    int8_t buffer[32];
    bool exitRequested = false;
    bool redrawNeeded = false;
// Keep reading until pipe is empty (non‑blocking)
    ssize_t n = 0;
    while(true) {
      {
//ST1("")
        n = read(mSelfPipeFDs[0], buffer, sizeof(buffer));
      }
      if(n < 0) {
        if(errno == EAGAIN)
          break;// no more data
        break;// real error – bail
      }
      else
        if(n == 0) {
          break;// pipe closed
        }
      D3("parsing {} bytes", n)
// Process each byte in the buffer
      for(ssize_t i = 0; i < n; ++i) {
        uint8_t token = static_cast<uint8_t>(buffer[i]);
        if(token == 1) {
          exitRequested = true;
// We can stop processing further tokens – exit is final
          break;
        }
        else
          if(token == 2) {
            redrawNeeded = true;
          }
// Other tokens are ignored (but you could extend)
      }
      if(exitRequested)
        break;// stop reading, we're exiting
    }
    if(exitRequested) {
      mShouldExit = true;
      return true;
    }
// Redraw all dirty windows if any redraw token was seen
    if(redrawNeeded && mIsX11) {
      for(auto& pair : mXCBWindowMap) {
        AWindow* w = pair.second.get();
        if(w && w->NeedsRepaint()) {
          w->ClearRepaintFlag();
          w->Draw();
        }
      }
    }
    if(redrawNeeded && mIsWayland) {
      for(auto& pair : mWaylandSurfaceMap) {
        AWindow* w = pair.second.get();
        if(w && w->NeedsRepaint()) {
          w->ClearRepaintFlag();
          w->Draw();
        }
      }
    }
    return false;
  }


  AWindow* AUI::FindWindowByNativeId(uint64_t id, AUIWindowType w) {
    switch(w) {
      case AUIWindowType::Wayland:
        return WaylandFindWindow(reinterpret_cast<wl_surface*>(id));
        break;
      case AUIWindowType::X11:
        return X11FindWindow(id);
        break;
      default:
        E("unknown window type provided")
        break;
    }
    return nullptr;
  }

  void AUI::UpdateLayout() {
    AWindow* w = nullptr;
    for(auto& pair : mXCBWindowMap) {
      w = pair.second.get();
      w->LayoutUpdate();
    }
    for(auto& pair : mWaylandSurfaceMap) {
      w = pair.second.get();
      w->LayoutUpdate();
    }
  }

  AUI::~AUI() {
    ST2("")
    mXCBWindowMap.clear();
    mWaylandSurfaceMap.clear();
// mMainWnd is now a dangling pointer; we set to nullptr to avoid accidental use.
    mMainWnd = nullptr;
    if(mFTCManager) {
      FTC_Manager_Done(mFTCManager);
      mFTCManager = nullptr;
// The manager has now freed the face; mark it invalid
      mFtDefaultFace = nullptr;
    }
// Do NOT call FT_Done_Face if the manager already freed it
    if(mFtDefaultFace) {
      FT_Done_Face(mFtDefaultFace);
      mFtDefaultFace = nullptr;
    }
    if(mFtLibrary) {
      FT_Done_FreeType(mFtLibrary);
      mFtLibrary = nullptr;
    }
    if(mWaylandKeyboard)
      wl_keyboard_destroy(mWaylandKeyboard);
    if(mXkbState)
      xkb_state_unref(mXkbState);
    if(mXkbKeymap)
      xkb_keymap_unref(mXkbKeymap);
    if(mXkbCtx)
      xkb_context_unref(mXkbCtx);
    if(mWaylandDisplay) {
      if(mWaylandPointer) {
        wl_pointer_destroy(mWaylandPointer);
        mWaylandPointer = nullptr;
      }
      if(mWaylandSeat) {
        wl_seat_destroy(mWaylandSeat);
        mWaylandSeat = nullptr;
      }
      if(mWaylandDecorationManager) {
        zxdg_decoration_manager_v1_destroy(mWaylandDecorationManager);
        mWaylandDecorationManager = nullptr;
      }
      if(mWaylandShm) {
        wl_shm_destroy(mWaylandShm);
        mWaylandShm = nullptr;
      }
      if(mWaylandXdgBase) {
        xdg_wm_base_destroy(mWaylandXdgBase);
        mWaylandXdgBase = nullptr;
      }
      if(mWaylandRegistry) {
        wl_registry_destroy(mWaylandRegistry);
        mWaylandRegistry = nullptr;
      }
      if(mWaylandCompositor) {
        wl_compositor_destroy(mWaylandCompositor);
        mWaylandCompositor = nullptr;
      }
      wl_display_disconnect(mWaylandDisplay);
      mWaylandDisplay = nullptr;
    }
    if(mX11Connection) {
      xcb_disconnect(mX11Connection);
    }
  }


}// namespace aui
//
