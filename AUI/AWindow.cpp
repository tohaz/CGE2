#include "AUILib.h"

namespace aui {
  class AWidget;

  void AWindow::RemoveModal(AWidget* widget) {
    auto it = std::find(mModalStack.begin(), mModalStack.end(), widget);
    if(it != mModalStack.end()) {
      (*it)->mIsModal = false;
      mModalStack.erase(it);
// Optionally restore focus if this was the top
      if(mModalStack.empty()) {
        FocusedWidget(nullptr);
      }
      else {
// Similar focus restoration as in PopModal
        AWidget* newTop = mModalStack.back();
        if(newTop->Focusable())
          FocusedWidget(newTop);
        else {
          AWidget* focusable = newTop->FindFirstFocusable();
          if(focusable)
            FocusedWidget(focusable);
          else
            FocusedWidget(newTop);
        }
      }
      RequestRedraw();
    }
  }

  void AWindow::PopModal() {
    if(mModalStack.empty())
      return;
    AWidget* top = mModalStack.back();
    top->mIsModal = false;
    mModalStack.pop_back();
// Restore focus
    if(!mModalStack.empty()) {
      AWidget* newTop = mModalStack.back();
      if(newTop->Focusable()) {
        FocusedWidget(newTop);
      }
      else {
        AWidget* focusable = newTop->FindFirstFocusable();
        if(focusable)
          FocusedWidget(focusable);
        else
          FocusedWidget(newTop);
      }
    }
    else {
      FocusedWidget(nullptr);// no modal, focus goes to nothing (or a default widget)
    }
    RequestRedraw();
  }

  void AWindow::PushModal(AWidget* widget) {
    if(!widget)
      return;
// Ensure widget is attached to this window
    if(widget->Wnd() != this) {
      E("Cannot push modal: widget not attached to this window");
      return;
    }
// Remove if already in stack (so it moves to top)
    auto it = std::find(mModalStack.begin(), mModalStack.end(), widget);
    if(it != mModalStack.end())
      mModalStack.erase(it);
    widget->mIsModal = true;
    mModalStack.push_back(widget);
// Transfer focus to the new modal
    if(widget->Focusable()) {
      FocusedWidget(widget);
    }
    else {
      AWidget* focusable = widget->FindFirstFocusable();
      if(focusable)
        FocusedWidget(focusable);
      else
        FocusedWidget(widget);// fallback
    }
    RequestRedraw();
  }

  std::pair<int32_t, int32_t> CalculateCoordsRotatedFull(int32_t x, int32_t y, uint32_t sizeX, uint32_t sizeY,
      double angle) {
    if(std::abs(angle) < 1e-9) {
      return {static_cast<int32_t>(x), static_cast<int32_t>(y)};
    }
    double halfW = static_cast<double>(sizeX) / 2.0;
    double halfH = static_cast<double>(sizeY) / 2.0;
    double dx = x - halfW;
    double dy = y - halfH;
    double radians = -angle * (M_PI / 180.0);
    double cosA = std::cos(radians);
    double sinA = std::sin(radians);
    double rx = dx * cosA - dy * sinA + halfW;
    double ry = dx * sinA + dy * cosA + halfH;
    return {static_cast<int32_t>(std::round(rx)), static_cast<int32_t>(std::round(ry))};
  }

  AWindow::AWindow() {
    D2()
    mSizeX = AUI_DEFAULT_WINDOW_SZX;
    mSizeY = AUI_DEFAULT_WINDOW_SZY;
    mAUI = nullptr;
  }

  AWindow* AWindow::AttachTo(UNUSED AUI* au, UNUSED const std::string& title) {
    return AttachTo(au, title, au->MainBackendType());
  }

  AWindow* AWindow::AttachTo(UNUSED AUI* au, UNUSED const std::string& title, UNUSED AUIWindowType type) {
    D2("starts")
    AWindow* w = nullptr;
    switch(type) {
      case AUIWindowType::Wayland:
        D2("creating Wayland main window")
        w = new WaylandWindowContext(au);
        w->Type(AUIWindowType::Wayland);
        break;
      case AUIWindowType::X11:
        w = new XCBWindowContext(au);
        w->Type(AUIWindowType::X11);
        break;
      default:
        E("invalid window type")
        break;
    }
    au->RegisterWindow(w->NativeWindowId(), std::unique_ptr<AWindow>(w));
    w->Title(title);
    w->BackendCursor(AUICursorType::Default);
    return w;
  }

  void AWindow::Draw(void* buf) {
    uint32_t* buffer = static_cast<uint32_t*>(buf);
//    FillRect(buffer, mSizeX, 0, 0, mSizeX, mSizeY, mBGColor);
    for(auto& widget : mWidg) {
      if(!widget->Visible() || widget->Modal())
        continue;
      widget->EnsureContentUpToDate();
      widget->EnsureOverlayUpToDate();
      widget->Composite(buffer, mSizeX, mSizeY, widget->X(), widget->Y(), widget->Angle(), 0, 0,
          static_cast<int32_t>(mSizeX), static_cast<int32_t>(mSizeY));
    }
    for(auto* modal : mModalStack) {
      if(!modal->Visible())
        continue;
      modal->EnsureContentUpToDate();
      modal->EnsureOverlayUpToDate();
// Use absolute position for modal (it may be nested)
      modal->Composite(buffer, mSizeX, mSizeY, modal->AbsX(), modal->AbsY(), modal->Angle(), 0, 0,
          static_cast<int32_t>(mSizeX), static_cast<int32_t>(mSizeY));
    }
  }

  void AWindow::AddWidget(UNUSED std::unique_ptr<AWidget> widg) {
    if(widg) {
      widg.get()->Wnd(this);
      mWidg.push_back(std::move(widg));
    }
    else {
      E("Attempted to add a null widget to the window");
    }
  }

  void AWindow::Title(const std::string& title) {
    mTitle = title;
    BackendTitle(title);
  }

  bool AWindow::Close() {
    mClosing = true;
    if(mAUI->MainWnd() == this) {
      mAUI->ExitAUI();
      return true;
    }
    if(mType == AUIWindowType::Wayland) {
      D1("closing Wayland window")
      WaylandWindowContext* ctx = static_cast<WaylandWindowContext*>(this);
      uint64_t windowId = ctx->NativeWindowId();
      return mAUI->WaylandUnregisterWindow(windowId);
    }
    if(mType == AUIWindowType::X11) {
      D1("closing XCB window")
      XCBWindowContext* ctx = static_cast<XCBWindowContext*>(this);
      uint64_t windowId = ctx->NativeWindowId();
      return mAUI->XCBUnregisterWindow(windowId);
    }
    else {
      E("unknown window type")
    }
    return false;
  }

  int32_t AWindow::FindFreeBufferIndex() const {
    for(size_t i = 0; i < AUI_NUM_BUFFERS; ++i) {
      if(!mBuffers[i]->isBusy) {
        D3("found free buffer{}", i)
        return SafeINT32(i);
      }
    }
    D2("all buffers busy")
    return -1;
  }

  void AWindow::EnginePtr(AUI* au) {
    if(au != nullptr) {
      mAUI = au;
    }
    else {
      E("null reference passed")
    }
  }

  void AWindow::Type(AUIWindowType t) {
    mType = t;
  }

  AUI* AWindow::EnginePtr() {
    if(mAUI != nullptr)
      return mAUI;
    else {
      E("null reference")
    }
  }

  void AWindow::EnableResize() {
    if(mResize) {
      E("resize is already enabled")
    }
    mResize = true;
    BackendEnableResize();
  }

  void AWindow::DisableResize() {
    D2()
    if(!mResize) {
      E("resize is already disabled")
    }
    mResize = false;
    BackendDisableResize();
  }

  void AWindow::Resize(uint32_t x, uint32_t y) {
    if(!mResize) {
      E("Window resizing is disabled by default. Call EnableResize()")
    }
    BackendResize(x, y);
    LayoutUpdate();
  }

  void AWindow::Move(int32_t x, int32_t y) {
    BackendMove(x, y);
  }

  void AWindow::OnMousePress(int32_t x, int32_t y, UNUSED uint32_t button) {
    D2("===incoming x {} y {} button {}", x, y, button);
    if(mMousePressCallback) {
      mMousePressCallback(this, mMousePressCallbackData, x, y, button);
    }
    if(mActiveDropDown && ProcessDropdown(x, y)) {
      return;// event consumed by closing dropdown
    }
    if(!mModalStack.empty()) {
      AWidget* top = mModalStack.back();
      if(top->Visible() && top->Enabled()) {
// Convert to top‑modal local coords
        auto [localX, localY] = top->ToLocalCoords(x, y);
        AWidget* consumed = nullptr;
        switch(button) {
          case BTN_LEFT:
            consumed = top->OnMouseDownLeft(localX, localY);
            break;
          case BTN_RIGHT:
            consumed = top->OnMouseDownRight(localX, localY);
            break;
          case BTN_MIDDLE:
            consumed = top->OnMouseDownMiddle(localX, localY);
            break;
          default:
            consumed = top->OnMouseDownOther(localX, localY, button);
            break;
        }
        if(consumed) {
          if(consumed->Focusable())
            FocusedWidget(consumed);
          else
            if(top->Focusable())
              FocusedWidget(top);
          if(consumed->MouseLeftReleaseRequired() || consumed->IsPressedLeft())
            mCapturedWidgetLeft = consumed;
        }
      }
      return;// block normal widgets
    }
    switch(button) {
      case BTN_LEFT:
        D2("BTN_LEFT press")
        for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
          AWidget* wid = it->get();
          if(!wid->Visible() || !wid->Enabled())
            continue;
          D2("trying widget {}", wid->Text());
          int32_t relX = x - wid->X();
          int32_t relY = y - wid->Y();
          auto [localX, localY] = CalculateCoordsRotatedFull(relX, relY, wid->SizeX(), wid->SizeY(), wid->Angle());
          bool inBounds = (localX >= 0 && localX < SafeINT32(wid->SizeX()) && localY >= 0
              && localY < SafeINT32(wid->SizeY()));
          if(inBounds || !wid->ClipChildrenHitbox()) {
// Dispatch down into tree
            AWidget* cons = wid->OnMouseDownLeft(localX, localY);
            if(cons != nullptr) {
              D2("widget consumed event: {}", cons->Text());
// 1. Set Focus to the actual consumer (or fallback to top-level widget)
              if(cons->Focusable()) {
                FocusedWidget(cons);
              }
              else
                if(wid->Focusable()) {
                  FocusedWidget(wid);
                }
// 2. Set Mouse Capture on Window if the consumed widget requires it
              if(cons->MouseLeftReleaseRequired() || cons->IsPressedLeft()) {
                D2("capturing w {}", cons->Text());
                mCapturedWidgetLeft = cons;
              }
              break;// Stop Z-order iteration
            }
// Fallback if top-level widget consumes mouse events
            if(inBounds && wid->ConsumesMouseEvents()) {
              D1("widget consumes mouse events over its geometry, stop");
              if(wid->Focusable()) {
                FocusedWidget(wid);
              }
              if(wid->MouseLeftReleaseRequired() || wid->IsPressedLeft()) {
                mCapturedWidgetLeft = wid;
              }
              break;
            }
          }
        }
        D2("loop through widgets ends")
        break;
      default:
        D("unhandled button press")
        break;
    }
  }

  void AWindow::OnMouseRelease(UNUSED int32_t x, UNUSED int32_t y, UNUSED uint32_t button) {
    D2("x {} y {} button {}", x, y, button);
    if(mMouseReleaseCallback) {
      mMouseReleaseCallback(this, mMouseReleaseCallbackData, x, y, button);
    }
    if(!mModalStack.empty()) {
      AWidget* target = nullptr;
      if(mCapturedWidgetLeft) {
// Ensure the captured widget is still valid and part of the modal hierarchy
        if(mCapturedWidgetLeft->IsDescendantOf(mModalStack.back())) {
          target = mCapturedWidgetLeft;
        }
        mCapturedWidgetLeft = nullptr;// clear capture before dispatch
      }
      if(!target) {
        target = mModalStack.back();
      }
      if(target && target->Visible() && target->Enabled()) {
        auto [localX, localY] = target->ToLocalCoords(x, y);
        switch(button) {
          case BTN_LEFT:
            target->OnMouseUpLeft(localX, localY);
            break;
          case BTN_RIGHT:
            target->OnMouseUpRight(localX, localY);
            break;
          case BTN_MIDDLE:
            target->OnMouseUpMiddle(localX, localY);
            break;
          default:
            target->OnMouseUpOther(localX, localY, button);
            break;
        }
      }
      return;// block normal widgets
    }
    AWidget* consumed = nullptr;
    switch(button) {
      case BTN_LEFT: {
        D2("BTN_LEFT release");
// 1. CAPTURED ROUTING
        if(mCapturedWidgetLeft != nullptr) {
          AWidget* target = mCapturedWidgetLeft;
          mCapturedWidgetLeft = nullptr;// Clear capture before dispatch to prevent re-entrancy bugs
// IMPORTANT: Calculate coordinates in TARGET'S PARENT space,
// because target->OnMouseUpLeft expects parent-relative coordinates!
          int32_t parentRelX = x;
          int32_t parentRelY = y;
          if(target->Parent() != nullptr) {
            auto [pX, pY] = target->Parent()->ToLocalCoords(x, y);
            parentRelX = pX - target->X();
            parentRelY = pY - target->Y();
          }
          else {
// Target is a root-level widget directly attached to AWindow
            parentRelX = x - target->X();
            parentRelY = y - target->Y();
          }
          D2("Dispatching captured release to '[{}]' with coords ({}, {})", target->Text(), parentRelX, parentRelY);
          consumed = target->OnMouseUpLeft(parentRelX, parentRelY);
          if(consumed != nullptr) {
            D2("Release consumed by captured widget '[{}]'", consumed->Text());
          }
        }
// 2. FALLBACK ROUTING (No Captured Widget)
        else {
          for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
            AWidget* widget = it->get();
            if(!widget->Visible() || !widget->Enabled())
              continue;
// Pass window-relative offset to root widget
            int32_t localX = x - widget->X();
            int32_t localY = y - widget->Y();
// Check clipping on root container before recursing
            if(widget->ClipChildren()) {
              bool inBounds = (localX >= 0 && localX < SafeINT32(widget->SizeX()) && localY >= 0
                  && localY < SafeINT32(widget->SizeY()));
              if(!inBounds) {
                continue;// Skip root widget if mouse release is outside clipped container
              }
            }
            consumed = widget->OnMouseUpLeft(localX, localY);
            if(consumed != nullptr) {
              D2("Release consumed by '[{}]'", consumed->Text());
              break;
            }
          }
        }
        if(consumed == nullptr) {
          D2("Release not consumed");
        }
        break;
      }
      default:
        D2("Unhandled button release")
        ;
        break;
    }
  }

//  void AWindow::OnMouseMove(int32_t x, int32_t y) {
//    D3("mouse move global coords {} {}", x, y);
//    if(!mModalStack.empty()) {
//// If capture exists, route to that widget (must be within modal)
//      if(mCapturedWidgetLeft) {
//// Ensure captured widget is still valid and inside modal
//        if(mCapturedWidgetLeft->IsDescendantOf(mModalStack.back())) {
//          int32_t localX = x - mCapturedWidgetLeft->AbsX();
//          int32_t localY = y - mCapturedWidgetLeft->AbsY();
//          mCapturedWidgetLeft->OnMouseMove(localX, localY);
//        }
//        else {
//// capture widget no longer in modal, clear it
//          mCapturedWidgetLeft = nullptr;
//        }
//        return;
//      }
//      AWidget* top = mModalStack.back();
//      auto [localX, localY] = top->ToLocalCoords(x, y);
//      AWidget* hit = top->HitTestLocal(localX, localY);
//      if(hit) {
//        auto [hitLocalX, hitLocalY] = hit->ToLocalCoords(x, y);
//        hit->OnMouseMove(hitLocalX, hitLocalY);
//        if(!mHLWidget && hit->mHLEnabled) {
//          mHLWidget = hit;
//          mHLWidget->HL(true);
//        }
//      }
//      else {
//        if(mHLWidget) {
//          mHLWidget->HL(false);
//          mHLWidget = nullptr;
//        }
//      }
//      return;
//    }
//// If a left‑button capture exists, route all moves to that widget
//    if(mCapturedWidgetLeft) {
//// Convert window coords to local coords relative to the captured widget
//      int32_t absX = mCapturedWidgetLeft->AbsX();
//      int32_t absY = mCapturedWidgetLeft->AbsY();
//      int32_t localX = x - absX;
//      int32_t localY = y - absY;
//      mCapturedWidgetLeft->OnMouseMove(localX, localY);
//      return;// don't also send to hovered widget
//    }
//// Normal hit‑test path (no capture)
//    AWidget* hit = FindWidgetAt(x, y);
//    if(hit) {
//      hit->OnMouseMove(x - hit->AbsX(), y - hit->AbsY());
//      if(!mHLWidget && hit->mHLEnabled) {
//        mHLWidget = hit;
//        mHLWidget->HL(true);
//      }
//    }
//    else {
//      if(mHLWidget) {
//        mHLWidget->HL(false);
//        mHLWidget = nullptr;
//      }
//    }
//  }

  void AWindow::OnMouseMove(int32_t x, int32_t y) {
      D3("mouse move global coords {} {}", x, y);

  // ---- 1. Modal Stack Capture / Routing ----
      if(!mModalStack.empty()) {
        if(mCapturedWidgetLeft) {
          if(mCapturedWidgetLeft->IsDescendantOf(mModalStack.back())) {
            // FIX: Use ToLocalCoords to transform global (x, y) through the parent hierarchy
            auto [localX, localY] = mCapturedWidgetLeft->ToLocalCoords(x, y);
            mCapturedWidgetLeft->OnMouseMove(localX, localY);
          }
          else {
            mCapturedWidgetLeft = nullptr;
          }
          return;
        }
        AWidget* top = mModalStack.back();
        auto [localX, localY] = top->ToLocalCoords(x, y);
        AWidget* hit = top->HitTestLocal(localX, localY);
        if(hit) {
          auto [hitLocalX, hitLocalY] = hit->ToLocalCoords(x, y);
          hit->OnMouseMove(hitLocalX, hitLocalY);
          if(!mHLWidget && hit->mHLEnabled) {
            mHLWidget = hit;
            mHLWidget->HL(true);
          }
        }
        else {
          if(mHLWidget) {
            mHLWidget->HL(false);
            mHLWidget = nullptr;
          }
        }
        return;
      }

  // ---- 2. Normal Window Left-Button Capture ----
      if(mCapturedWidgetLeft) {
        // FIX: Replace "x - mCapturedWidgetLeft->AbsX()" with "ToLocalCoords(x, y)"
        // This maps global (x, y) through every parent container's rotation, scaling, and offset
        auto [localX, localY] = mCapturedWidgetLeft->ToLocalCoords(x, y);
        mCapturedWidgetLeft->OnMouseMove(localX, localY);
        return; // don't also send to hovered widget
      }

  // ---- 3. Normal Hit-Test Path (No Capture) ----
      AWidget* hit = FindWidgetAt(x, y);
      if(hit) {
        auto [hitLocalX, hitLocalY] = hit->ToLocalCoords(x, y);
        hit->OnMouseMove(hitLocalX, hitLocalY);
        if(!mHLWidget && hit->mHLEnabled) {
          mHLWidget = hit;
          mHLWidget->HL(true);
        }
      }
      else {
        if(mHLWidget) {
          mHLWidget->HL(false);
          mHLWidget = nullptr;
        }
      }
    }

  void AWindow::OnMouseEnter(UNUSED int32_t x, UNUSED int32_t y) {
    D2("mouse enter {} {}", x, y)
    BackendCursor(AUICursorType::Default);
  }

  void AWindow::OnMouseLeave(UNUSED int32_t x, UNUSED int32_t y) {
    D2("mouse leave {} {}", x, y)
    if(mHLWidget) {
      mHLWidget->HL(false);
      mHLWidget = nullptr;
    }
  }

  void AWindow::RequestRedraw() {
    D4("Window redraw request: {} done: {} wakeups {}", mRedrawCounter, mRedrawCounterDone, mAUI->WakeupCounter())
    mNeedsRepaint = true;
    mRedrawCounter++;
    mAUI->RequestRedraw();
  }

  void AWindow::OnMouseWheel(int32_t x, int32_t y, int32_t delta) {
    D2("mouse wheel {} {} delta {}", x, y, delta)
    if(!mModalStack.empty()) {
      AWidget* top = mModalStack.back();
      auto [localX, localY] = top->ToLocalCoords(x, y);
      top->DispatchMouseWheel(localX, localY, delta);
      return;
    }
// Otherwise, forward to children using the last known mouse position
    for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
      AWidget* widget = it->get();
      if(!widget->Visible())
        continue;
      if(widget->DispatchMouseWheel(x, y, delta))
        return;// stop at first consumer
    }
  }

  AWidget* AWindow::FindWidgetAt(int32_t x, int32_t y) {
// Iterate front-to-back (topmost rendered widgets first)
    for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
      AWidget* widget = it->get();
// Ignore hidden or disabled branches
      if(!widget->Visible() || !widget->Enabled()) {
        continue;
      }
// Convert Window Coords -> Root Widget Local Coords
      int32_t localX = x - widget->X();
      int32_t localY = y - widget->Y();
// Ask the root widget to search itself and its subtree using local space
      AWidget* found = widget->HitTestLocal(localX, localY);
      if(found != nullptr) {
        return found;// Found the deepest targeted widget under the cursor
      }
    }
    return nullptr;// Hovering over empty window space
  }

  void AWindow::BGColor(uint32_t v) {
    mBGColor = v;
    RequestRedraw();
  }

  void AWindow::SetMousePressCallback(WndMouseButtonCallback3 callback, void* anyData = nullptr) {
    mMousePressCallback = std::move(callback);
    mMousePressCallbackData = anyData;
    D1("userData={}", (uint64_t)anyData)
  }

  void AWindow::SetMouseReleaseCallback(WndMouseButtonCallback3 callback, void* anyData = nullptr) {
    mMouseReleaseCallback = std::move(callback);
    mMouseReleaseCallbackData = anyData;
    D1("userData={}", (uint64_t)anyData)
  }

  void AWindow::MouseClick(int32_t x, int32_t y) {
    OnMousePress(x, y, BTN_LEFT);
    OnMouseRelease(x, y, BTN_LEFT);
  }

  void AWindow::LayoutUpdate() {
    D2()
    for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
      AWidget* widget = it->get();
      widget->LayoutDirty();
      widget->LayoutUpdate();
    }
  }

  void AWindow::OnKeyEvent(const AUIKeyEvent& event) {
    if(!mModalStack.empty()) {
      AWidget* top = mModalStack.back();
// If focused widget exists and is inside the modal, use it
      if(mFocusedWidget && mFocusedWidget->IsDescendantOf(top)) {
        mFocusedWidget->OnKeyEvent(event);
      }
      else {
// Move focus to the modal (or its first focusable child)
        if(top->Focusable()) {
          FocusedWidget(top);
        }
        else {
          AWidget* focusable = top->FindFirstFocusable();
          if(focusable)
            FocusedWidget(focusable);
          else
            FocusedWidget(top);
        }
        if(mFocusedWidget) {
          mFocusedWidget->OnKeyEvent(event);
        }
      }
      return;
    }
    if(mFocusedWidget && mFocusedWidget->Enabled()) {
      mFocusedWidget->OnKeyEvent(event);
    }
  }

  void AWindow::FocusedWidget(AWidget* v) {
    if(mFocusedWidget == v)
      return;
    AWidget* oldFocused = mFocusedWidget;
    mFocusedWidget = v;
    if(oldFocused) {
      oldFocused->OnFocusLost();
    }
// Ensure mFocusedWidget wasn't reassigned inside OnFocusLost()
    if(mFocusedWidget == v && mFocusedWidget) {
      mFocusedWidget->OnFocusGained();
    }
    RequestRedraw();
  }

  void AWindow::BringToFront(AWidget* child) {
    D1("widget {}", child->Text())
    auto it = std::find_if(mWidg.begin(), mWidg.end(), [child](const std::unique_ptr<AWidget>& ptr) {
      return ptr.get() == child;
    });
    if(it != mWidg.end() && it != mWidg.end() - 1) {
      std::unique_ptr<AWidget> ptr = std::move(*it);
      mWidg.erase(it);
      mWidg.push_back(std::move(ptr));
    }
  }

  void AWindow::MouseDown(int32_t x, int32_t y) {
    OnMousePress(x, y, BTN_LEFT);
  }

  void AWindow::MouseUp(int32_t x, int32_t y) {
    OnMouseRelease(x, y, BTN_LEFT);
  }

  bool AWindow::ProcessDropdown(int32_t x, int32_t y) {
    if(!mActiveDropDown)
      return false;
    AWidget* list = mActiveDropDown->DropList();
    if(list) {
      int32_t lx = list->AbsX();
      int32_t ly = list->AbsY();
      int32_t lr = lx + static_cast<int32_t>(list->SizeX());
      int32_t lb = ly + static_cast<int32_t>(list->SizeY());
// If outside, close and return true
      if(x < lx || x >= lr || y < ly || y >= lb) {
        mActiveDropDown->CloseDropDown();
        return true;
      }
    }
    return false;// click inside list, keep dropdown open
  }

  void AWindow::RemoveWidget(AWidget* v) {
    std::unique_ptr<AWidget> deadWidget;
    const auto erasedCount = std::erase_if(mWidg, [v, &deadWidget](auto& up) noexcept {
      if (up.get() == v) {
        deadWidget = std::move(up);
        return true;
      }
      return false;
    });
    if (erasedCount > 0) {
      D2("widget deleted");
    } else {
      D2("widget not deleted");
    }
  }

  void AWindow::CapturedWidgetLeft(AWidget* v) {
    mCapturedWidgetLeft = v;
  }

  void AWindow::Show() {
    if(mHidden) {
      BackendShow();
      mHidden = false;
    }
  }

  void AWindow::Hide() {
    if(!mHidden) {
      BackendHide();
      mHidden = true;
    }
  }

}
