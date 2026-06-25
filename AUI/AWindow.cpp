#include "AUILib.h"

namespace aui {

  AWindow::AWindow(std::unique_ptr<IWindowContext> backend) :
      mBackend(std::move(backend)), mBGColor(AUI_DEFAULT_WINDOW_BG) {// add this line
// The backend already has a pointer to this AWindow (set during construction).
// No additional initialization needed.
  }

  AWindow* AWindow::AttachTo(AUI *engine, const std::string &title) {
    return AttachTo(engine, title, engine->GetWindowType());
  }

  AWindow* AWindow::AttachTo(AUI *engine, const std::string &title, AUIWindowType type) {
    if(!engine)
      E("AWindow::AttachTo: engine is null");
    std::unique_ptr<IWindowContext> backend;
// Create backend according to type
    if(type == AUIWindowType::XCB) {
      backend = std::make_unique<XcbWindowContext>(engine, nullptr);
    }
    else if(type == AUIWindowType::Wayland) {
      backend = std::make_unique<WaylandWindowContext>(engine, nullptr);
    }
    else {
      E("AWindow::AttachTo: invalid window type");
    }
    AWindow *win = new AWindow(std::move(backend));
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
    win->Draw();
    return win;
  }

  void AWindow::Resize(uint32_t w, uint32_t h) {
    D3("AWindow::Resize: called with {}x{}, resizeEnabled={}", w, h, mResizeEnabled);
    if(!mResizeEnabled) {
      E("window resize is disabled");
    }
    AUI *au = mBackend ? mBackend->GetEnginePtr() : nullptr;
    if(!au || !au->IsProcessingMessages()) {
// Defer the resize until the event loop is active
      mResizePending = true;
      mPendingWidth = w;
      mPendingHeight = h;
      D3("Resize deferred");
      return;
    }
// Apply immediately (event loop is already running)
    if(mBackend)
      mBackend->Resize(w, h);
    mSizeX = w;
    mSizeY = h;
// Clear stale draw commands (same as before)
    uint64_t nativeId = mBackend->GetNativeWindowId();
    {
      std::lock_guard<std::recursive_mutex> lock(au->GetCommandMutex());
      auto &commands = au->GetDrawCommands();
      commands.erase(std::remove_if(commands.begin(), commands.end(), [nativeId](const DrawCommand &cmd) {
        return cmd.type == DrawCommandType::Xcb && cmd.xcb.windowId == nativeId;
      }),
      commands.end());
    }
// Notify child widgets
    for(auto &widget : mWidg) {
      widget->OnParentResize(w, h);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  void AWindow::Close() {
    D2("AWindow::Close entered, nativeId={}", mNativeId);
    if (mClosed) return;
    mClosed = true;
    AUI *ep = mBackend ? mBackend->GetEnginePtr() : nullptr;
    bool isMain = (ep && ep->MainWnd() == this);
    if(isMain) {
// Do NOT unregister/destroy the main window now.
// Just break the event loop; the window will be destroyed
// when the AUI is deleted in main() after the worker thread joins.

    }
    else {
// For non‑main windows, immediate destruction is acceptable.
      if(ep)
        ep->UnregisterWindow(mNativeId);
    }
  }

  void AWindow::SetTitle(const std::string &title) {
    mWindowTitle = title;
    if(mBackend)
      mBackend->SetTitle(title);
    else {
      E("set title with no backend present")
    }
  }

  void AWindow::EnableResize() {
    mResizeEnabled = true;
    if(mBackend) {
      mBackend->EnableResize();
    }
    else {
      E("enabling resize with no backend present")
    }
  }

  void AWindow::DisableResize() {
    mResizeEnabled = false;
    if(mBackend)
      mBackend->DisableResize();
    else {
      E("disabling resize with no backend present")
    }
  }

  void AWindow::OnMousePress(int32_t x, int32_t y, uint32_t button) {
    if(AMenu::IsActiveMenuVisible() && !AMenu::IsPointInsideActiveMenu(x, y))
      AMenu::DismissActiveMenu();
    AMenu *perm = AMenu::GetPermanentMenu();
    if(perm) {
      AMenu *sub = perm->GetActiveSubMenu();
      if(sub && sub->IsVisible() && !AMenu::IsPointInsideMenuHierarchy(sub, x, y))
        perm->CloseSubMenu();
    }
    uint32_t normalized = button;
    if(button == 272)
      normalized = 1;
    else if(button == 273)
      normalized = 3;
    else if(button == 274)
      normalized = 2;
    if(normalized != 1)
      return;
    if(mDragWidget) {
      int32_t localX = x - mDragWidget->X();
      int32_t localY = y - mDragWidget->Y();
      if(localX >= 0 && localX < static_cast<int32_t>(mDragWidget->SizeX()) && localY >= 0
          && localY < static_cast<int32_t>(mDragWidget->SizeY()))
        mDragWidget->OnMouseClick(localX, localY, true);
      ForceDraw();
      return;
    }
    for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
      AWidget *widget = it->get();
      if(!widget->IsVisible())
        continue;
      bool isMenu = dynamic_cast<AMenu*>(widget) != nullptr;
      bool consumed = widget->DispatchClick(x, y, true);
      if(consumed) {
        if(!mDragWidget && !isMenu)
          SetDragWidget(widget);
        break;
      }
    }
    if(mDragWidget && mDragWidget->IsFocusable())
      SetFocus(mDragWidget);
    else if(!mDragWidget)
      ClearFocus();
  }

  void AWindow::OnMouseRelease(int32_t x, int32_t y, uint32_t button) {
    if(mDragWidget) {
      int32_t localX = x - mDragWidget->X();
      int32_t localY = y - mDragWidget->Y();
      mDragWidget->OnMouseClick(localX, localY, false);
      SetDragWidget(nullptr);
    }
    else {
      uint32_t normalized = (button == 272) ? 1 : (button == 273) ? 3 : (button == 274) ? 2 : button;
      if(normalized == 1) {
        for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
          AWidget *widget = it->get();
          if(!widget->IsVisible())
            continue;
          if(widget->DispatchClick(x, y, false))
            break;
        }
      }
    }
  }

  void AWindow::Draw() {
      D3("[WIN] Draw() called, mDrawPending={}", mDrawPending);
      if(mClosed) return;
      if(mDrawPending) return;
      mDrawPending = true;
      if(mBackend) {
          AUI *aui = mBackend->GetEnginePtr();
          if(aui) {
              D3("[WIN] Scheduling draw");
              aui->ScheduleDraw(this);
          }
      }
  }

  void AWindow::ForceDraw() {
      D3("[WIN] ForceDraw() called, mDrawPending={}", mDrawPending);
      if(mClosed) return;
      mDrawPending = false;
      DoDraw();
  }

  void AWindow::AddWidget(std::unique_ptr<AWidget> widg) {
    AUI *engine = mBackend ? mBackend->GetEnginePtr() : nullptr;
    widg->InitWidgetProperties(0, engine, this, nullptr, widg->mWidgetType);
    mWidg.push_back(std::move(widg));
    Draw();
  }

  void AWindow::OnMouseMove(int32_t x, int32_t y) {
// If dragging, bypass hover tracking
    if(mDragWidget) {
      D1("we are dragging widget")
      int32_t localX = x - mDragWidget->X();
      int32_t localY = y - mDragWidget->Y();
      mDragWidget->OnMouseMove(localX, localY);
      return;
    }
    AWidget *hit = FindWidgetAt(x, y);
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
    }
    else if(mHoverWidget) {
// Same widget, just forward motion
      int32_t localX = x - mHoverWidget->X();
      int32_t localY = y - mHoverWidget->Y();
      mHoverWidget->OnMouseMove(localX, localY);
    }
  }

  AWidget* AWindow::FindWidgetAt(int32_t x, int32_t y) const {
// Iterate top‑down (reverse order)
    for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
      AWidget *w = it->get();
      if(w->IsVisible() && w->IsEnabled() && x >= w->X() && x < w->X() + static_cast<int32_t>(w->SizeX()) && y >= w->Y()
          && y < w->Y() + static_cast<int32_t>(w->SizeY())) {
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
    D2("SetDragWidget: {} -> {}", (UINT64)mDragWidget, (UINT64)widget);
    mDragWidget = widget;
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
    Draw();
  }

  void AWindow::DoDraw() {
    D3("AWindow::DoDraw: start");
    if(mClosed) return;
    if(!mBackend->EnsureBuffer(mSizeX, mSizeY)) {
      E("Failed to allocate buffer for window")
    }
    if(!mBackend) {
      DT("no backend");
      return;
    }
    uint32_t *buffer = mBackend->GetSoftwareBuffer();
    if(!buffer) {
      D2("no software buffer");
      return;
    }
    uint32_t bg = mBGColor & 0x00FFFFFF;
    for(size_t i = 0; i < static_cast<size_t>(mSizeX) * mSizeY; ++i)
      buffer[i] = bg;
    for(const auto &widget : mWidg) {
      if(widget->IsVisible()) {
        widget->Draw(buffer, mSizeX, mSizeY, 0, 0);
      }
    }
    mBackend->QueueFrameCommit();
  }

  void AWindow::SetFocus(AWidget *widget) {
    if(mFocusedWidget == widget)
      return;
    if(mFocusedWidget) {
      mFocusedWidget->OnFocusLost();
    }
    mFocusedWidget = widget;
    if(mFocusedWidget) {
      mFocusedWidget->OnFocusGained();
    }
    Draw();
  }

  void AWindow::ClearFocus() {
    SetFocus(nullptr);
  }

  void AWindow::OnKeyEvent(const AUIKeyEvent &event) {
    if(mFocusedWidget && mFocusedWidget->IsEnabled()) {
      mFocusedWidget->OnKeyEvent(event);
    }
  }

  void AWindow::SetCursor(AUICursorType type) {
    if(mBackend) {
      mBackend->SetCursor(type);
    }
    else
      E()
  }

  void AWindow::BringChildToFront(AWidget *child) {
    auto it = std::find_if(mWidg.begin(), mWidg.end(), [child](const std::unique_ptr<AWidget> &ptr) {
      return ptr.get() == child;
    });
    if(it != mWidg.end() && it != mWidg.end() - 1) {
      std::unique_ptr<AWidget> ptr = std::move(*it);
      mWidg.erase(it);
      mWidg.push_back(std::move(ptr));
    }
  }

  void AWindow::Move(int32_t x, int32_t y) {
    if(mBackend) {
      mBackend->Move(x, y);
    }
    else {
      E("backend is 0")
    }
  }

  void AWindow::RemoveWidget(AWidget *widget) {
    if(!widget)
      return;
    auto it = std::find_if(mWidg.begin(), mWidg.end(), [widget](const std::unique_ptr<AWidget> &ptr) {
      return ptr.get() == widget;
    });
    if(it == mWidg.end())
      return;// not found
// Clear any internal references to this widget
    if(mDragWidget == widget)
      mDragWidget = nullptr;
    if(mHoverWidget == widget)
      mHoverWidget = nullptr;
    if(mFocusedWidget == widget)
      mFocusedWidget = nullptr;
    mWidg.erase(it);// widget is destroyed via unique_ptr
    ForceDraw();// redraw immediately
  }

  void AWindow::ApplyPendingResize() {
    if(!mResizePending)
      return;
    if(mPendingWidth == mSizeX && mPendingHeight == mSizeY) {
      mResizePending = false;
      return;
    }
    if(!mResizeEnabled) {
// Optionally handle the case where resize is now disabled.
// You might want to still apply it, or ignore. We'll apply anyway.
      D3("Applying pending resize while resize is disabled – proceed anyway");
    }
    if(mBackend) {
      mBackend->Resize(mPendingWidth, mPendingHeight);
    }
    mSizeX = mPendingWidth;
    mSizeY = mPendingHeight;
// Clear stale draw commands (same as in Resize)
    AUI *au = mBackend ? mBackend->GetEnginePtr() : nullptr;
    if(au) {
      uint64_t nativeId = mBackend->GetNativeWindowId();
      {
        std::lock_guard<std::recursive_mutex> lock(au->GetCommandMutex());
        auto &commands = au->GetDrawCommands();
        commands.erase(std::remove_if(commands.begin(), commands.end(), [nativeId](const DrawCommand &cmd) {
          return cmd.type == DrawCommandType::Xcb && cmd.xcb.windowId == nativeId;
        }),
        commands.end());
      }
    }
// Notify child widgets
    for(auto &widget : mWidg) {
      widget->OnParentResize(mSizeX, mSizeY);
    }
    mResizePending = false;
  }

  AWindow::~AWindow() {
    D3()
  }

}// namespace aui

