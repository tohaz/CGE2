#include "AUILib.h"
#include "AWindow.h"

namespace aui {

  AWindow::AWindow(std::unique_ptr<IWindowContext> backend) :
      mBackend(std::move(backend)),
      mBGColor(AUI_DEFAULT_WINDOW_BG) {   // add this line
    // The backend already has a pointer to this AWindow (set during construction).
    // No additional initialization needed.
  }

  AWindow* AWindow::AttachTo(AUI* engine, const std::string& title) {
    return AttachTo(engine, title, engine->GetWindowType());
  }

  AWindow* AWindow::AttachTo(AUI* engine, const std::string& title, AUIWindowType type) {
    if(!engine) E("AWindow::AttachTo: engine is null");
    std::unique_ptr<IWindowContext> backend;
    // Create backend according to type
    if(type == AUIWindowType::XCB) {
      backend = std::make_unique<XcbWindowContext>(engine, nullptr);
    } else if(type == AUIWindowType::Wayland) {
      backend = std::make_unique<WaylandWindowContext>(engine, nullptr);
    } else {
      E("AWindow::AttachTo: invalid window type");
    }
    AWindow* win = new AWindow(std::move(backend));
    win->mBackend->SetWindow(win);
    win->SetTitle(title);
    if(!win->mBackend->CreateFrame(500, 300, title)) {
      delete win;
      E("AWindow::AttachTo: CreateFrame failed for window '{}'", title);
    }
    win->mSizeX = 500;
    win->mSizeY = 300;
    win->mX = 0;
    win->mY = 0;
    win->mNativeId = win->mBackend->GetNativeWindowId();
    win->mWindowType = type;
    win->DisableResize();
    engine->RegisterWindow(win->mNativeId, std::unique_ptr<AWindow>(win));
    return win;
  }

  void AWindow::Resize(uint32_t w, uint32_t h) {
    D2("AWindow::Resize: called with {}x{}, resizeEnabled={}", w, h, mResizeEnabled);
    if(!mResizeEnabled) {
      E("window resize is disabled");
    }
    if(mBackend)
      mBackend->Resize(w, h);
    AUI *au = mBackend->GetEnginePtr();
    if(!au)
      E();
    uint64_t nativeId = mBackend->GetNativeWindowId();
// Clear stale draw commands for this window (XCB only) – lock is released after scope
    {
      std::lock_guard<std::mutex> lock(au->GetCommandMutex());
      auto &commands = au->GetDrawCommands();
      commands.erase(std::remove_if(commands.begin(), commands.end(), [nativeId](const DrawCommand &cmd) {
        return cmd.type == DrawCommandType::Xcb && cmd.xcb.windowId == nativeId;
      }),
      commands.end());
    }// mutex unlocked here
    mSizeX = w;
    mSizeY = h;
// Notify child widgets – this may call Draw() which will enqueue new commands
    for (auto &widget : mWidg) {
      widget->OnParentResize(w, h);
    }
  }

  void AWindow::Close() {
    D2("AWindow::Close entered, nativeId={}", mNativeId);
    AUI* ep = mBackend ? mBackend->GetEnginePtr() : nullptr;
    if(ep) {
      bool isMain = (ep->MainWnd() == this);
      D2("isMain={}, calling UnregisterWindow", isMain);
      ep->UnregisterWindow(mNativeId);
      if(isMain) {
        D2("Exiting AUI");
        ep->ExitAUI();
      }
    } else {
      DT("No engine pointer");
    }
  }

  void AWindow::SetTitle(const std::string &title) {
    if(mBackend)
      mBackend->SetTitle(title);
    mWindowTitle = title;
  }

  void AWindow::EnableResize() {
    if(mBackend) mBackend->EnableResize();
    mResizeEnabled = true;
  }

  void AWindow::DisableResize() {
    if(mBackend) mBackend->DisableResize();
    mResizeEnabled = false;
  }

  void AWindow::OnMousePress(int32_t x, int32_t y, uint32_t button) {
// Normalize Wayland button codes to XCB numbers (1 = left, 2 = middle, 3 = right)
    uint32_t normalized = button;
    if(button == 272)
      normalized = 1;
    else if(button == 273)
      normalized = 3;
    else if(button == 274)
      normalized = 2;
// Only left button can start/continue dragging
    if(normalized != 1)
      return;
// If a drag is already in progress, only the current drag widget gets the event
    if(mDragWidget) {
      int32_t localX = x - mDragWidget->X();
      int32_t localY = y - mDragWidget->Y();
// Check if the click is inside the drag widget
      if(localX >= 0 && localX < static_cast<int32_t>(mDragWidget->SizeX()) && localY >= 0
          && localY < static_cast<int32_t>(mDragWidget->SizeY())) {
        mDragWidget->OnMouseClick(localX, localY, true);
      }
// If click is outside the drag widget, ignore it (do not change drag widget)
      return;
    }
// No active drag: find the topmost widget that wants the press
    for (auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
      if((*it)->DispatchClick(x, y, true)) {
        mDragWidget = it->get();
        break;
      }
      }
  }

  void AWindow::OnMouseRelease(int32_t x, int32_t y, uint32_t button) {
    D2("OnMouseRelease: drag widget = {}", (void*)mDragWidget);
    if(mDragWidget) {
      int32_t localX = x - mDragWidget->X();
      int32_t localY = y - mDragWidget->Y();
      mDragWidget->OnMouseClick(localX, localY, false);
      mDragWidget = nullptr;
    }
    else {
      uint32_t normalized = (button == 272) ? 1 : (button == 273) ? 3 : (button == 274) ? 2 : button;
      if(normalized == 1) {
        for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
          if((*it)->DispatchClick(x, y, false)) break;
        }
      }
    }
  }

  void AWindow::Draw() {
      if (mDrawPending) return;           // already scheduled
      mDrawPending = true;
      // Register this window for a deferred draw with the AUI engine
      if (mBackend) {
          AUI* aui = mBackend->GetEnginePtr();
          if (aui) aui->ScheduleDraw(this);
      }
  }

  void AWindow::ForceDraw() {
      mDrawPending = false;                // cancel any pending draw
      DoDraw();
  }

  void AWindow::AddWidget(std::unique_ptr<AWidget> widg) {
    AUI* engine = mBackend ? mBackend->GetEnginePtr() : nullptr;
    widg->InitWidgetProperties(0, engine, this, nullptr, widg->mWidgetType);
    mWidg.push_back(std::move(widg));
    Draw();
  }

  void AWindow::OnMouseMove(int32_t x, int32_t y) {
    // If dragging, bypass hover tracking
    if(mDragWidget) {
      int32_t localX = x - mDragWidget->X();
      int32_t localY = y - mDragWidget->Y();
      mDragWidget->OnMouseMove(localX, localY);
      return;
    }
    AWidget* hit = FindWidgetAt(x, y);
    // If the widget under cursor changed
    if(hit != mHoverWidget) {
      // Notify old widget it lost hover
      if(mHoverWidget) {
        mHoverWidget->OnMouseLeave();
      }
      mHoverWidget = hit;
      // If new widget is valid, send enter event (OnMouseMove with local coords)
      if(mHoverWidget) {
        int32_t localX = x - mHoverWidget->X();
        int32_t localY = y - mHoverWidget->Y();
        mHoverWidget->OnMouseMove(localX, localY);
      }
    } else if(mHoverWidget) {
      // Same widget, just forward motion
      int32_t localX = x - mHoverWidget->X();
      int32_t localY = y - mHoverWidget->Y();
      mHoverWidget->OnMouseMove(localX, localY);
    }
  }

  AWidget* AWindow::FindWidgetAt(int32_t x, int32_t y) const {
    // Iterate top‑down (reverse order)
    for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
      AWidget* w = it->get();
      if(w->IsVisible() && w->IsEnabled() &&
         x >= w->X() && x < w->X() + static_cast<int32_t>(w->SizeX()) &&
         y >= w->Y() && y < w->Y() + static_cast<int32_t>(w->SizeY())) {
        return w;
      }
    }
    return nullptr;
  }

  void AWindow::ClearHover() {
    if(mHoverWidget) {
      mHoverWidget->OnMouseLeave();
      mHoverWidget = nullptr;
    }
  }

  void AWindow::SetDragWidget(AWidget *widget) {
    mDragWidget = widget;
    D2("SetDragWidget: widget={}, type={}", (uint64_t)widget, widget ? static_cast<int64_t>(widget->GetWidgetType()) : -1);
  }

  void AWindow::OnMouseWheel(int32_t delta) {
// If dragging, send wheel to the drag widget
    if(mDragWidget) {
      mDragWidget->OnMouseWheel(delta);
      return;
    }
// Otherwise send to the widget under cursor
    if(mHoverWidget) {
      mHoverWidget->OnMouseWheel(delta);
    }
  }

  void AWindow::DoDraw() {
      D3("AWindow::DoDraw: start");
      if(!mBackend) { DT("no backend"); return; }
      uint32_t* buffer = mBackend->GetSoftwareBuffer();
      if(!buffer) { D2("no software buffer"); return; }
      uint32_t bg = mBGColor & 0x00FFFFFF;
      for(size_t i = 0; i < static_cast<size_t>(mSizeX) * mSizeY; ++i) buffer[i] = bg;
      for(const auto& widget : mWidg) {
          if(widget->IsVisible()) {
              widget->Draw(buffer, mSizeX, mSizeY, 0, 0);
          }
      }
      mBackend->QueueFrameCommit();
  }

  AWindow::~AWindow() {
// The backend will be destroyed automatically by unique_ptr.
  }

}// namespace aui

