#include "AUILib.h"

namespace aui {

  bool AWidget::IsDescendantOf(const AWidget* ancestor) const {
    const AWidget* cur = this;
    while(cur) {
      if(cur == ancestor)
        return true;
      cur = cur->Parent();
    }
    return false;
  }

  void AWidget::Modal(bool modal) {
    if(modal == mIsModal)
      return;
    if(!mWnd) {
      E("Widget has no window, cannot set modal");
      return;
    }
    if(modal) {
      mWnd->PushModal(this);
    }
    else {
      mWnd->RemoveModal(this);
    }
  }

  AWidget* AWidget::FindFirstFocusable() {
    if(mFocusable && mVisible && mEnabled)
      return this;
    for(auto& child : mWidg) {
      if(child->mVisible && child->mEnabled) {
        AWidget* found = child->FindFirstFocusable();
        if(found)
          return found;
      }
    }
    return nullptr;
  }

  AWidget* AWidget::OnMouseDownLeft(int32_t x, int32_t y) {
    D2("incoming {} {}", x, y);
    if(!mVisible || !mEnabled) {
      return nullptr;
    }
    bool selfInBounds = (x >= 0 && x < SafeINT32(mSizeX) && y >= 0 && y < SafeINT32(mSizeY));
// 1. CHILD PASS (Z-Order Reverse Traversal)
    for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
      AWidget* wid = it->get();
      if(!wid->Visible() || !wid->Enabled())
        continue;
      int32_t relX = x - wid->X();
      int32_t relY = y - wid->Y();
      auto [localX, localY] = CalculateCoordsRotatedFull(relX, relY, wid->SizeX(), wid->SizeY(), wid->Angle());
      bool childInBounds = (localX >= 0 && localX < SafeINT32(wid->SizeX()) && localY >= 0
          && localY < SafeINT32(wid->SizeY()));
      if(childInBounds || !wid->ClipChildrenHitbox()) {
        AWidget* cons = wid->OnMouseDownLeft(localX, localY);
        if(cons != nullptr) {
          return cons;// Descendant consumed the press
        }
        if(childInBounds && wid->ConsumesMouseEvents()) {
          wid->mMousePressedLeft = true;
          return wid;
        }
      }
    }
// 2. SELF PASS
    if(selfInBounds) {
// Set pressed state BEFORE firing callback so wid->MousePressedLeft() is true!
      mMousePressedLeft = true;
      OnMouseDownLeftInternal(x, y);
      if(mMousePressLeftCallback) {
        D2("Firing mMousePressLeftCallback for '[{}]'", mText.c_str());
        AWidget* handled = mMousePressLeftCallback(this, mMousePressLeftCallbackData, x, y);
        if(handled != nullptr) {
          return handled;
        }
// If callback explicitly returned nullptr, clear press state unless consuming events
        if(!mMouseClickCallback && !ConsumesMouseEvents()) {
          mMousePressedLeft = false;
          return nullptr;
        }
      }
      if(mMouseClickCallback || ConsumesMouseEvents()) {
        return this;
      }
    }
    mMousePressedLeft = false;
    return nullptr;
  }

  AWidget* AWidget::OnMouseUpLeft(int32_t x, int32_t y) {
    auto [localX, localY] = CalculateCoordsRotated(x, y);
    bool isInside = (localX >= 0 && localX < static_cast<int32_t>(mSizeX) && localY >= 0
        && localY < static_cast<int32_t>(mSizeY));
    bool wasPressed = mMousePressedLeft;
    mMousePressedLeft = false;// Always clear pressed state on release
    OnMouseUpLeftInternal(x, y);
    AWidget* consumedWidget = nullptr;
// 1. SELF PASS (Captured Widget Release Processing)
    if(wasPressed) {
// FIX: Always fire release callback if the widget was captured!
// This allows UI elements to reset visual states (e.g., button un-push).
      if(mMouseReleaseLeftCallback) {
        D2("Firing release callback for '{}' at local ({}, {})", mText.c_str(), localX, localY);
        mMouseReleaseLeftCallback(this, mMouseReleaseLeftCallbackData, localX, localY);
        consumedWidget = this;
      }
// Click callback ONLY fires if inside OR if explicit release-outside is required
      if(mMouseLeftRequireRelese || isInside) {
        if(mMouseClickCallback) {
          D2("Firing click callback on release for '{}'", mText.c_str());
          mMouseClickCallback(this, mMouseClickCallbackData, localX, localY);
          consumedWidget = this;
        }
      }
    }
// 2. CHILD PASS
    for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
      AWidget* child = it->get();
      if(!child->Visible() || !child->Enabled())
        continue;
      int32_t relX = localX - child->X();
      int32_t relY = localY - child->Y();
      auto [childLocalX, childLocalY] = CalculateCoordsRotatedFull(relX, relY, child->SizeX(), child->SizeY(),
          child->Angle());
      AWidget* childCons = child->OnMouseUpLeft(childLocalX, childLocalY);
      if(childCons != nullptr) {
        consumedWidget = childCons;
      }
    }
    if(consumedWidget != nullptr) {
      return consumedWidget;
    }
    if(isInside && ConsumesMouseEvents()) {
      return this;
    }
    return nullptr;
  }

  void AWidget::AddWidget(UNUSED std::unique_ptr<AWidget> widg) {
    mWidg.push_back(std::move(widg));
  }

  void AWidget::Wnd(AWindow* win) {
    if(win != nullptr) {
      mWnd = win;
    }
    else
      E("null reference")
  }

  void AWidget::Draw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
      int32_t clipLeft, int32_t clipTop, int32_t clipRight, int32_t clipBottom) const {
    D2("[Widget::Draw] Enter | Local Pos: (%d, %d) | Offsets: (%d, %d)", X(), Y(), offsetX, offsetY);
    D2("[Widget::Draw] Incoming Clip: L=%d, T=%d, R=%d, B=%d", clipLeft, clipTop, clipRight, clipBottom);
// 1. Background
    if(mDefaultFillBG)
      OnDrawBG(buffer, bufferW, bufferH, offsetX, offsetY, clipLeft, clipTop, clipRight, clipBottom);
// 2. Custom widget content
    OnDraw(buffer, bufferW, bufferH, offsetX, offsetY, clipLeft, clipTop, clipRight, clipBottom);
// 3. Compute children clip
    int32_t childrenClipL = clipLeft;
    int32_t childrenClipT = clipTop;
    int32_t childrenClipR = clipRight;
    int32_t childrenClipB = clipBottom;
    if(mClipChildren) {
      double absAngle = AngleAbs();
      double parentAngle = mParent ? mParent->AngleAbs() : 0.0;
      bool isRotated = std::abs(absAngle) > 1e-6 || std::abs(parentAngle) > 1e-6;
      if(!isRotated) {
        int32_t innerL = offsetX + X() + static_cast<int32_t>(mBorderThick);
        int32_t innerT = offsetY + Y() + static_cast<int32_t>(mBorderThick);
        int32_t innerR = offsetX + X() + static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mBorderThick);
        int32_t innerB = offsetY + Y() + static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mBorderThick);
        childrenClipL = std::max(childrenClipL, innerL);
        childrenClipT = std::max(childrenClipT, innerT);
        childrenClipR = std::min(childrenClipR, innerR);
        childrenClipB = std::min(childrenClipB, innerB);
        D2("[Widget::Draw] Shrank Children Clip: L=%d, T=%d, R=%d, B=%d", childrenClipL, childrenClipT, childrenClipR,
            childrenClipB);
      }
    }
    auto drawBorder = [&]() {
      if(Border() == 0)
        return;
      int32_t absX = AbsX();
      int32_t absY = AbsY();
      double absAngle = AngleAbs();
      int32_t parentAbsX = mParent ? mParent->AbsX() : 0;
      int32_t parentAbsY = mParent ? mParent->AbsY() : 0;
      uint32_t parentW = mParent ? mParent->mSizeX : bufferW;
      uint32_t parentH = mParent ? mParent->mSizeY : bufferH;
      double parentAngle = mParent ? mParent->AngleAbs() : 0.0;
      if(std::abs(absAngle) < 1e-6 && std::abs(parentAngle) < 1e-6) {
        int32_t effOffsetX = absX - X();
        int32_t effOffsetY = absY - Y();
        DrawBorder(buffer, bufferW, bufferH, effOffsetX, effOffsetY, clipLeft, clipTop, clipRight, clipBottom);
      }
      else {
        DrawRotatedBorder(buffer, bufferW, bufferH, clipLeft, clipTop, clipRight, clipBottom, X(), Y(), mSizeX, mSizeY,
            mBorderThick, mBorderColor, absAngle, parentAbsX, parentAbsY, parentW, parentH, parentAngle);
      }
    };
    if(!mClipChildren && mDefaultDrawBorder)
      drawBorder();
    DrawChildren(buffer, bufferW, bufferH, offsetX, offsetY, childrenClipL, childrenClipT, childrenClipR,
        childrenClipB);
    if(mClipChildren && mDefaultDrawBorder)
      drawBorder();
  }

  void AWidget::DrawChildren(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
      int32_t clipLeft, int32_t clipTop, int32_t clipRight, int32_t clipBottom) const {
    if(mWidg.empty()) {
      D2("    DrawChildren: no children, returning");
      return;
    }
    D2(">>> DrawChildren ENTER");
    D2("    bufferW=%u, bufferH=%u, offsetX=%d, offsetY=%d, clip=(%d,%d)-(%d,%d)", bufferW, bufferH, offsetX, offsetY,
        clipLeft, clipTop, clipRight, clipBottom);
    D2("    parent: X()=%d, Y()=%d, mSizeX=%u, mSizeY=%u, mBorderThick=%u, mClipChildren=%d", X(), Y(), mSizeX, mSizeY,
        mBorderThick, mClipChildren);
    double parentAngle = AngleAbs();
    D2("    parentAngle=%f", parentAngle);
// Parent's visible rectangle is exactly the passed clip (absolute buffer coords)
    int32_t parentVisL = clipLeft;
    int32_t parentVisT = clipTop;
    int32_t parentVisR = clipRight;
    int32_t parentVisB = clipBottom;
    D2("    Parent visible rect (absolute): (%d,%d)-(%d,%d)", parentVisL, parentVisT, parentVisR, parentVisB);
// Parent's inner rectangle (if clipping children to border)
    int32_t innerLeft = offsetX + X() + static_cast<int32_t>(mBorderThick);
    int32_t innerTop = offsetY + Y() + static_cast<int32_t>(mBorderThick);
    int32_t innerRight = offsetX + X() + static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mBorderThick);
    int32_t innerBottom = offsetY + Y() + static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mBorderThick);
    D2("    Parent inner rect (if clipping): (%d,%d)-(%d,%d)", innerLeft, innerTop, innerRight, innerBottom);
    for(const auto& child : mWidg) {
      if(child->Modal())
        continue;
      if(!child->Visible()) {
        D2("    Child 0x{:x} invisible, skipping", reinterpret_cast<uintptr_t>(child.get()));
        continue;
      }
      double childAbsAngle = child->AngleAbs();
      D2("    Child: X=%d, Y=%d, SizeX=%u, SizeY=%u, Angle=%f", child->X(), child->Y(), child->SizeX(), child->SizeY(),
          childAbsAngle);
// ------------------------------------------------------------------
// Unrotated Path (both parent and child have zero rotation)
// ------------------------------------------------------------------
      if(std::abs(parentAngle) < 0.001 && std::abs(childAbsAngle) < 0.001) {
        D2("    --- Unrotated path ---");
// Child's absolute bounding box
        int32_t childAbsX = offsetX + X() + child->X();
        int32_t childAbsY = offsetY + Y() + child->Y();
        int32_t childAbsR = childAbsX + static_cast<int32_t>(child->SizeX());
        int32_t childAbsB = childAbsY + static_cast<int32_t>(child->SizeY());
        D2("    Child absolute bounds: (%d,%d)-(%d,%d)", childAbsX, childAbsY, childAbsR, childAbsB);
// Start with parent's incoming clip rect
        int32_t clipL = parentVisL;
        int32_t clipT = parentVisT;
        int32_t clipR = parentVisR;
        int32_t clipB = parentVisB;
// If parent clips children to its inner area, shrink the clip bounds
        if(mClipChildren) {
          D2("    Parent clips children to inner area");
          clipL = std::max(clipL, innerLeft);
          clipT = std::max(clipT, innerTop);
          clipR = std::min(clipR, innerRight);
          clipB = std::min(clipB, innerBottom);
          D2("    After inner clip: (%d,%d)-(%d,%d)", clipL, clipT, clipR, clipB);
        }
// Quick visibility check: Does the child's bounding box intersect the clip rect at all?
        int32_t visL = std::max(childAbsX, clipL);
        int32_t visT = std::max(childAbsY, clipT);
        int32_t visR = std::min(childAbsR, clipR);
        int32_t visB = std::min(childAbsB, clipB);
        if(visL >= visR || visT >= visB) {
          D2("    Visible region empty, skipping child");
          continue;
        }
// IMPORTANT: Pass down 'clipL..clipB' (the container clip boundary),
// NOT 'visL..visB' (which truncates the clip to this child's geometry)!
        int32_t childOffsetX = offsetX + X();
        int32_t childOffsetY = offsetY + Y();
        D2("    Calling child->Draw(offset=(%d,%d), clip_abs=(%d,%d)-(%d,%d))", childOffsetX, childOffsetY, clipL,
            clipT, clipR, clipB);
        child->Draw(buffer, bufferW, bufferH, childOffsetX, childOffsetY, clipL, clipT, clipR, clipB);
      }
      else {
// ------------------------------------------------------------------
// Rotated Path – compute true rotated AABB for the child
// ------------------------------------------------------------------
        D2("    --- Rotated path ---");
        double parentAngleRad = parentAngle * M_PI / 180.0;
        double cosP = std::cos(parentAngleRad);
        double sinP = std::sin(parentAngleRad);
        double parentCX = static_cast<double>(offsetX + X()) + static_cast<double>(mSizeX) / 2.0;
        double parentCY = static_cast<double>(offsetY + Y()) + static_cast<double>(mSizeY) / 2.0;
        double childLocalCX = static_cast<double>(child->X()) + static_cast<double>(child->SizeX()) / 2.0;
        double childLocalCY = static_cast<double>(child->Y()) + static_cast<double>(child->SizeY()) / 2.0;
        double absChildCX0 = static_cast<double>(offsetX + X()) + childLocalCX;
        double absChildCY0 = static_cast<double>(offsetY + Y()) + childLocalCY;
        double dxp = absChildCX0 - parentCX;
        double dyp = absChildCY0 - parentCY;
// Global center of the child
        double finalChildCX = parentCX + (dxp * cosP - dyp * sinP);
        double finalChildCY = parentCY + (dxp * sinP + dyp * cosP);
// --- Compute true rotated AABB using child's absolute angle ---
        double childRad = childAbsAngle * M_PI / 180.0;
        double cosC = std::cos(childRad);
        double sinC = std::sin(childRad);
        double hw = static_cast<double>(child->SizeX()) / 2.0;
        double hh = static_cast<double>(child->SizeY()) / 2.0;
        const double cornersX[4] = { -hw, hw, hw, -hw };
        const double cornersY[4] = { -hh, -hh, hh, hh };
        double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
        for(int32_t i = 0; i < 4; ++i) {
          double rx = cornersX[i] * cosC - cornersY[i] * sinC + finalChildCX;
          double ry = cornersX[i] * sinC + cornersY[i] * cosC + finalChildCY;
          minX = std::min(minX, rx);
          maxX = std::max(maxX, rx);
          minY = std::min(minY, ry);
          maxY = std::max(maxY, ry);
        }
        int32_t childAbsX = static_cast<int32_t>(std::floor(minX));
        int32_t childAbsY = static_cast<int32_t>(std::floor(minY));
        int32_t childAbsR = static_cast<int32_t>(std::ceil(maxX));
        int32_t childAbsB = static_cast<int32_t>(std::ceil(maxY));
        D2("    Child absolute bounds (rotated AABB): (%d,%d)-(%d,%d)", childAbsX, childAbsY, childAbsR, childAbsB);
// Clip to parent's visible rectangle (and optional inner clipping)
        int32_t clipL = parentVisL;
        int32_t clipT = parentVisT;
        int32_t clipR = parentVisR;
        int32_t clipB = parentVisB;
        if(mClipChildren) {
// NOTE: If parent is rotated, shrinking by an axis-aligned inner rect
// can chop off rotated child corners. If you want rotated children
// to render fully when parent is rotated, omit inner clipping here.
          if(std::abs(parentAngle) < 0.001) {
            clipL = std::max(clipL, innerLeft);
            clipT = std::max(clipT, innerTop);
            clipR = std::min(clipR, innerRight);
            clipB = std::min(clipB, innerBottom);
          }
        }
        int32_t visL = std::max(childAbsX, clipL);
        int32_t visT = std::max(childAbsY, clipT);
        int32_t visR = std::min(childAbsR, clipR);
        int32_t visB = std::min(childAbsB, clipB);
        D2("    Visible region after intersection: (%d,%d)-(%d,%d)", visL, visT, visR, visB);
        if(visL >= visR || visT >= visB) {
          D2("    Visible region empty, skipping child");
          continue;
        }
        int32_t childOffsetX = offsetX + X();
        int32_t childOffsetY = offsetY + Y();
        D2("    Calling child->Draw(offset=(%d,%d), clip_abs=(%d,%d)-(%d,%d))", childOffsetX, childOffsetY, visL, visT,
            visR, visB);
        child->Draw(buffer, bufferW, bufferH, childOffsetX, childOffsetY, visL, visT, visR, visB);
      }
    }
  }

  void AWidget::Parent(AWidget* parent) {
    D3()
    if(parent != nullptr) {
      mParent = parent;
    }
    else
      D4("null reference")
  }

  void AWidget::Move(int32_t x, int32_t y) {
    D3()
    mX = x;
    mY = y;
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AWidget::Resize(uint32_t szx, uint32_t szy) {
    D3("Resize entry: pos=({},{}) size=({},{}) -> new size=({},{})", mX, mY, mSizeX, mSizeY, szx, szy);
    uint32_t oldW = mSizeX, oldH = mSizeY;
    mSizeX = szx;
    mSizeY = szy;
// ONLY cap the size if we have a parent AND that parent enforces child clipping!
    if(mParent && mParent->mCapSizeToParent) {
      D1("Capping size to parent for widget {}", mText)
      CapSizeToParent();
      D1("After CapSizeToParent check: pos=({},{}) size=({},{})", mX, mY, mSizeX, mSizeY);
      D3("After position restore: pos=({},{})", mX, mY);
    }
    if(oldW != mSizeX || oldH != mSizeY) {
      mTextMetricsValid = false;
    }
    OnResize(szx, szy);
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AWidget::CapSizeToParent() {
    if(!mParent)
      return;
    uint32_t origW = mSizeX;
    int32_t parentW = static_cast<int32_t>(mParent->SizeX());
    int32_t parentH = static_cast<int32_t>(mParent->SizeY());
    int32_t maxW = parentW - mX;
    int32_t maxH = parentH - mY;
    if(maxW < 0)
      maxW = 0;
    if(maxH < 0)
      maxH = 0;
    if(static_cast<int32_t>(mSizeX) > maxW)
      mSizeX = static_cast<uint32_t>(maxW);
    if(static_cast<int32_t>(mSizeY) > maxH)
      mSizeY = static_cast<uint32_t>(maxH);
    if(origW != mSizeX) {
      D1("CapSizeToParent TRUNCATED '{}': mX=%d, parentW=%d, origW=%u -> newW=%u", mText.c_str(), mX, parentW, origW,
          mSizeX);
    }
  }

  void AWidget::FontSize(uint32_t size) {
    mFontSize = size;
  }

  void AWidget::DrawBorder(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
      int32_t clipLeft, int32_t clipTop, int32_t clipRight, int32_t clipBottom) const {
    D2(">>> DrawBorder ENTER");
    D2("    bufferW=%u, bufferH=%u, offsetX=%d, offsetY=%d, clip_abs=(%d,%d)-(%d,%d)", bufferW, bufferH, offsetX,
        offsetY, clipLeft, clipTop, clipRight, clipBottom);
    D2("    widget: X()=%d, Y()=%d, mSizeX=%u, mSizeY=%u, mBorderThick=%u", X(), Y(), mSizeX, mSizeY, mBorderThick);

    if(mBorderThick == 0) {
      D2("    mBorderThick == 0, returning");
      return;
    }
// Convert to signed for safe arithmetic
    int32_t sizeX = static_cast<int32_t>(mSizeX);
    int32_t sizeY = static_cast<int32_t>(mSizeY);
    int32_t thick = static_cast<int32_t>(mBorderThick);
    int32_t bufW = static_cast<int32_t>(bufferW);
    int32_t bufH = static_cast<int32_t>(bufferH);
    D2("    sizeX=%d, sizeY=%d, thick=%d, bufW=%d, bufH=%d", sizeX, sizeY, thick, bufW, bufH);
// Widget bounds in buffer coordinates
    int32_t wLeft = offsetX + X();
    int32_t wTop = offsetY + Y();
    int32_t wRight = wLeft + sizeX;
    int32_t wBottom = wTop + sizeY;
    D2("    Widget bounds: left=%d, top=%d, right=%d, bottom=%d", wLeft, wTop, wRight, wBottom);
// Use the absolute clip directly (no offset adjustment)
    int32_t cLeft = clipLeft;
    int32_t cTop = clipTop;
    int32_t cRight = clipRight;
    int32_t cBottom = clipBottom;
    D2("    Passed clip (absolute): left=%d, top=%d, right=%d, bottom=%d", cLeft, cTop, cRight, cBottom);
// Visible region = intersection of widget bounds, clip, and buffer
    int32_t visLeft = std::max( { wLeft, cLeft, 0 });
    int32_t visTop = std::max( { wTop, cTop, 0 });
    int32_t visRight = std::min( { wRight, cRight, bufW });
    int32_t visBottom = std::min( { wBottom, cBottom, bufH });
    D2("    Visible region (after intersection): left=%d, top=%d, right=%d, bottom=%d", visLeft, visTop, visRight,
        visBottom);
    if(visLeft >= visRight || visTop >= visBottom) {
      D2("    Visible region empty, returning");
      return;
    }
// Helper lambda to draw a rectangle after clipping to the visible region
    auto drawClippedRect = [&](int32_t x, int32_t y, int32_t w, int32_t h, const char*) {
      D2("    Segment input rect (%d,%d) %dx%d", x, y, w, h);
      int32_t drawX = std::max(x, visLeft);
      int32_t drawY = std::max(y, visTop);
      int32_t drawR = std::min(x + w, visRight);
      int32_t drawB = std::min(y + h, visBottom);
      int32_t drawW = drawR - drawX;
      int32_t drawH = drawB - drawY;
      D2("        after clipping to visible: (%d,%d) %dx%d", drawX, drawY, drawW, drawH);
      if(drawW > 0 && drawH > 0) {
        D2("        Filling rect with color 0x%08X", mBorderColor);
        FillRect(buffer, bufferW, drawX, drawY, drawW, drawH, mBorderColor);
      }
      else {
        D2("        Skipped (empty rect)");
      }
    };
// Draw the four border segments
    drawClippedRect(wLeft, wTop, sizeX, thick, "top");
    drawClippedRect(wLeft, wBottom - thick, sizeX, thick, "bottom");
    drawClippedRect(wLeft, wTop, thick, sizeY, "left");
    drawClippedRect(wRight - thick, wTop, thick, sizeY, "right");
    D2("<<< DrawBorder EXIT");
  }

  void AWidget::DrawRotatedBorder(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t clipMinX,
      int32_t clipMinY, int32_t clipMaxX, int32_t clipMaxY, int32_t rectX, int32_t rectY, uint32_t rectW,
      uint32_t rectH, uint32_t borderThick, uint32_t borderColor, double angleDeg, int32_t parentX, int32_t parentY,
      uint32_t parentW, uint32_t parentH, double parentAngleDeg) const {
    if(!buffer || rectW == 0 || rectH == 0 || borderThick == 0)
      return;
    constexpr double DEG2RAD = 3.14159265358979323846 / 180.0;
// --- Parent transform parameters ---
    double pRad = parentAngleDeg * DEG2RAD;
    double pCos = std::cos(pRad);
    double pSin = std::sin(pRad);
    double phw = static_cast<double>(parentW) / 2.0;
    double phh = static_cast<double>(parentH) / 2.0;
    double pcx = static_cast<double>(parentX) + phw;
    double pcy = static_cast<double>(parentY) + phh;
// --- Child local center ---
    double childLocalCX = static_cast<double>(rectX) + static_cast<double>(rectW) / 2.0;
    double childLocalCY = static_cast<double>(rectY) + static_cast<double>(rectH) / 2.0;
    double offsetX = childLocalCX - phw;
    double offsetY = childLocalCY - phh;
    double cx = pcx + (offsetX * pCos - offsetY * pSin);
    double cy = pcy + (offsetX * pSin + offsetY * pCos);
// --- Child rotation parameters ---
    double childRad = angleDeg * DEG2RAD;
    double cosA = std::cos(childRad);
    double sinA = std::sin(childRad);
    double hw = static_cast<double>(rectW) / 2.0;
    double hh = static_cast<double>(rectH) / 2.0;
    double bt = static_cast<double>(borderThick);
// --- Compute child global rotated AABB ---
    const double cornersX[4] = { -hw, hw, hw, -hw };
    const double cornersY[4] = { -hh, -hh, hh, hh };
    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    for(int32_t i = 0; i < 4; ++i) {
      double rx = cornersX[i] * cosA - cornersY[i] * sinA + cx;
      double ry = cornersX[i] * sinA + cornersY[i] * cosA + cy;
      minX = std::min(minX, rx);
      maxX = std::max(maxX, rx);
      minY = std::min(minY, ry);
      maxY = std::max(maxY, ry);
    }
    int32_t startX = std::max(0, std::max(clipMinX, static_cast<int32_t>(std::floor(minX))));
    int32_t endX = std::min(static_cast<int32_t>(bufferW) - 1,
        std::min(clipMaxX, static_cast<int32_t>(std::ceil(maxX))));
    int32_t startY = std::max(0, std::max(clipMinY, static_cast<int32_t>(std::floor(minY))));
    int32_t endY = std::min(static_cast<int32_t>(bufferH) - 1,
        std::min(clipMaxY, static_cast<int32_t>(std::ceil(maxY))));
    if(startX > endX || startY > endY)
      return;
// --- Per-pixel parent boundary test lambda ---
// Uses the parent transform parameters already passed into DrawRotatedBorder
    auto isInsideParent = [pcx, pcy, pCos, pSin, phw, phh, parentW, parentH](double px, double py) -> bool {
      double pdx = px - pcx;
      double pdy = py - pcy;
// Inverse parent matrix transform
      double plx = (pdx * pCos + pdy * pSin) + phw;
      double ply = (-pdx * pSin + pdy * pCos) + phh;
// Test inside parent outer rectangle (change bounds if you need inner border offset)
      return (plx >= 0.0 && plx < static_cast<double>(parentW) && ply >= 0.0 && ply < static_cast<double>(parentH));
    };
// --- Rasterization ---
    for(int32_t py = startY; py <= endY; ++py) {
      double pixelY = static_cast<double>(py) + 0.5;
      double dy = pixelY - cy;
      uint32_t* row = buffer + static_cast<size_t>(py) * bufferW;
      for(int32_t px = startX; px <= endX; ++px) {
        double pixelX = static_cast<double>(px) + 0.5;
// 1. Per-pixel check against parent's rotated boundary
        if(!isInsideParent(pixelX, pixelY))
          continue;
// 2. Transform to child local space
        double dx = pixelX - cx;
        double lx = dx * cosA + dy * sinA + hw;
        double ly = -dx * sinA + dy * cosA + hh;
        if(lx >= 0.0 && lx < static_cast<double>(rectW) && ly >= 0.0 && ly < static_cast<double>(rectH)) {
// Inner border cutout check
          if(lx >= bt && lx < static_cast<double>(rectW) - bt && ly >= bt && ly < static_cast<double>(rectH) - bt)
            continue;
          row[px] = borderColor;
        }
      }
    }
  }

  bool AWidget::OnMouseMove(int32_t localX, int32_t localY) {
    D2("localX {} localY {}", localX, localY)
// The DispatchClick has already checked that we are inside and that no child consumed the event.
// Just invoke the callback (if any).
    if(mMouseMoveCallback) {
      mMouseMoveCallback(this, mMouseMoveUserData, localX, localY);
      return true;
    }
    return false;
  }

  void AWidget::BGColor(uint32_t bg) {
    mBGColor = bg;
    if(mWnd) {
      mWnd->Draw();
    }
  }
  void AWidget::BGColor2(uint32_t bg) {
    mBGColor2 = bg;
    if(mWnd) {
      mWnd->Draw();
    }
  }
  void AWidget::BGColor3(uint32_t bg) {
    mBGColor3 = bg;
    if(mWnd) {
      mWnd->Draw();
    }
  }
  void AWidget::BGColor4(uint32_t bg) {
    mBGColor4 = bg;
    if(mWnd) {
      mWnd->Draw();
    }
  }

  void AWidget::Angle(double an) {
    mAngle = an;
// 1. Calculate this widget's new absolute angle
    if(mParent == nullptr) {
      mAbsoluteAngle = an;
      D3("absolute angle is equal widget's angle")
    }
    else {
      mAbsoluteAngle = mParent->mAbsoluteAngle + an;
      D3("absolute angle is calculated to {}", mAbsoluteAngle)
    }
// 2. Notify and update all child sub-trees using a localized lambda
    auto updateChildren = [](auto& self, AWidget* parent) -> void {
      for(const auto& child : parent->mWidg) {
        if(child) {
          child->mAbsoluteAngle = parent->mAbsoluteAngle + child->mAngle;
// Recurse into the child's children
          self(self, child.get());
        }
      }
    };
    updateChildren(updateChildren, this);
// 3. Trigger the redraw
    if(mWnd)
      mWnd->RequestRedraw();
  }

  double AWidget::AngleAbs() const {
    return mAbsoluteAngle;
  }

  int32_t AWidget::AbsX() const {
    int32_t ax = mX;
    AWidget* w = const_cast<AWidget*>(this);
    while(w->mParent != nullptr) {
      w = w->mParent;
      ax += w->X();
    }
    D3("w {} X {} absX {}", Text(), mX, ax)
    return ax;
  }

  int32_t AWidget::AbsY() const {
    int32_t ay = mY;
    AWidget* w = const_cast<AWidget*>(this);
    while(w->mParent != nullptr) {
      w = w->mParent;
      ay += w->Y();
    }
    D2("w {} Y {} absY {}", Text(), mY, ay)
    return ay;
  }

  bool AWidget::ForwardMouseWheelToChildren(int32_t delta) {
// Use the last stored mouse position (local to this widget) to choose the target child
    for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
      AWidget* child = it->get();
      if(!child->Visible() || !child->Enabled())
        continue;
// Check if the stored cursor position falls inside the child
      if(mLastMouseX >= child->mX && mLastMouseX < child->mX + static_cast<int32_t>(child->mSizeX)
          && mLastMouseY >= child->mY && mLastMouseY < child->mY + static_cast<int32_t>(child->mSizeY)) {
// Forward using the child's Dispatch – this lets the child run its own visibility/enabled checks
// and forward to its own children.
        if(child->DispatchMouseWheel(mLastMouseX, mLastMouseY, delta))
          return true;// child consumed the event
      }
    }
    return false;// no child handled it
  }

  bool AWidget::ForwardMouseMoveToChildren(int32_t x, int32_t y) {
    for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
      AWidget* child = it->get();
      if(!child->Visible() || !child->Enabled())
        continue;
      if(x >= child->mX && x < child->mX + static_cast<int32_t>(child->mSizeX) && y >= child->mY
          && y < child->mY + static_cast<int32_t>(child->mSizeY)) {
        if(child->OnMouseMove(x, y))
          return true;// child consumed the event
      }
    }
    return false;// no child handled it
  }

  bool AWidget::DispatchMouseWheel(int32_t parentX, int32_t parentY, int32_t delta) {
    D2("x {} y {} delta {}", parentX, parentY, delta)
    if(!mVisible || !mEnabled)
      return false;
    int32_t localX = parentX - mX;
    int32_t localY = parentY - mY;
// Update stored cursor position (used by the delta‑only forward helper)
    mLastMouseX = localX;
    mLastMouseY = localY;
// 1. Forward to children (topmost first) using the delta‑only helper
    if(ForwardMouseWheelToChildren(delta))
      return true;
// 2. No child consumed – check if this widget itself is hit
    if(localX >= 0 && localX < static_cast<int32_t>(mSizeX) && localY >= 0 && localY < static_cast<int32_t>(mSizeY)) {
      OnMouseWheel(delta);
      return true;
    }
    return false;
  }

  void AWidget::OnMouseWheel(UNUSED int32_t delta) {
    D("default is void {}", delta)
  }

  std::pair<int32_t, int32_t> AWidget::GetAbsolutePosition() const {
    int32_t x = mX, y = mY;
    const AWidget* parent = mParent;
    while(parent) {
      x += parent->mX;
      y += parent->mY;
      parent = parent->mParent;
    }
    return {x, y};
  }

  AWidget* AWidget::HitTestLocal(int32_t x, int32_t y) {
    if(!mVisible || !mEnabled) {
      return nullptr;
    }
// 1. Convert incoming parent-relative coords to THIS widget's local space
    auto [localX, localY] = CalculateCoordsRotated(x, y);
    bool isInside = (localX >= 0 && localX < static_cast<int32_t>(mSizeX) && localY >= 0
        && localY < static_cast<int32_t>(mSizeY));
// 2. Check children first (top-to-bottom / reverse render order)
    if(!mClipChildrenHitbox || isInside) {
      for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
        AWidget* child = it->get();
        if(!child->Visible() || !child->Enabled())
          continue;
// Transform THIS widget local -> CHILD local
        int32_t childX = localX - child->X();
        int32_t childY = localY - child->Y();
        if(AWidget* found = child->HitTestLocal(childX, childY)) {
          return found;
        }
      }
    }
// 3. Self check
    if(isInside) {
      return this;
    }
    return nullptr;
  }

  void AWidget::OnDrawBG(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
      int32_t clipLeft, int32_t clipTop, int32_t clipRight, int32_t clipBottom) const {
    if(!mDefaultFillBG)
      return;
    D2("[OnDrawBG] PARAMETERS: offset=(%d,%d), clip_abs=(%d,%d)-(%d,%d), mClipChildren=%d", offsetX, offsetY, clipLeft,
        clipTop, clipRight, clipBottom, mClipChildren);
    uint32_t bgcolor = mHL ? HLColor(mBGColor) : mBGColor;
    double absAngle = AngleAbs();
    double parentAngle = mParent ? mParent->AngleAbs() : 0.0;
// Fast path if both widget and parent are unrotated
    if(std::abs(absAngle) < 1e-6 && std::abs(parentAngle) < 1e-6) {
// Widget bounds in absolute buffer coordinates (offsetX + X() gives the widget's origin)
      int32_t wLeft = offsetX + X();
      int32_t wTop = offsetY + Y();
      int32_t wRight = wLeft + static_cast<int32_t>(mSizeX);
      int32_t wBottom = wTop + static_cast<int32_t>(mSizeY);
// Intersect widget bounds with the passed clip and buffer limits
      int32_t left = std::max( { wLeft, clipLeft, 0 });
      int32_t top = std::max( { wTop, clipTop, 0 });
      int32_t right = std::min( { wRight, clipRight, static_cast<int32_t>(bufferW) });
      int32_t bottom = std::min( { wBottom, clipBottom, static_cast<int32_t>(bufferH) });
      int32_t drawW = right - left;
      int32_t drawH = bottom - top;
      D2("[OnDrawBG] draw rect: (%d,%d) %dx%d", left, top, drawW, drawH);
      if(drawW > 0 && drawH > 0) {
        FillRect(buffer, bufferW, left, top, drawW, drawH, bgcolor);
      }
      return;
    }
// Rotated path – use the existing DrawRotatedRect.
// The clip is currently not used in the rotated path (it uses full screen),
// but we keep the current behavior for now.
    D2("[OnDrawBG] ROTATED PATH: using full screen clip");
    DrawRotatedRect(buffer, bufferW, 0, 0, SafeINT32(bufferW), SafeINT32(bufferH), X(), Y(), mSizeX, mSizeY, absAngle,
        mParent ? mParent->AbsX() : 0, mParent ? mParent->AbsY() : 0, mParent ? mParent->mSizeX : bufferW,
        mParent ? mParent->mSizeY : bufferH, parentAngle, bgcolor);
  }

  void AWidget::HL(bool v) {
    if(v != mHL) {
      mHL = v;
      if(mWnd != nullptr) {
        mWnd->RequestRedraw();
      }
    }
  }

  void AWidget::Text(std::string tx) {
    D4()
    mText = tx;
    if(mWnd != nullptr) {
      mWnd->RequestRedraw();
    }
  }

  void AWidget::DefaultFillBG(bool v) {
    if(v != mDefaultFillBG) {
      mDefaultFillBG = v;
      if(mWnd != nullptr) {
        mWnd->RequestRedraw();
      }
    }
  }

  std::pair<int32_t, int32_t> AWidget::CalculateCoordsRotated(int32_t x, int32_t y) const {
    if(std::abs(mAngle) < 1e-9) {
      return {static_cast<int32_t>(x), static_cast<int32_t>(y)};
    }
    double halfW = static_cast<double>(mSizeX) / 2.0;
    double halfH = static_cast<double>(mSizeY) / 2.0;
    double dx = x - halfW;
    double dy = y - halfH;
    double radians = -mAngle * (M_PI / 180.0);
    double cosA = std::cos(radians);
    double sinA = std::sin(radians);
    double rx = dx * cosA - dy * sinA + halfW;
    double ry = dx * sinA + dy * cosA + halfH;
    return {static_cast<int32_t>(std::round(rx)), static_cast<int32_t>(std::round(ry))};
  }

  AWidget* AWidget::ProcessMouseEvent(int32_t x, int32_t y, AWidget* (AWidget::*childHandler)(int32_t, int32_t),
      MouseButtonCallback callback, void* callbackData) {
// 1. Calculate local coordinates relative to THIS widget
    auto [localX, localY] = CalculateCoordsRotated(x, y);
    bool isInside = (localX >= 0 && localX < static_cast<int32_t>(mSizeX) && localY >= 0
        && localY < static_cast<int32_t>(mSizeY));
// 2. Traversal Guard: Only check children if either:
//    - Clipping is DISABLED (!mClipChildrenHitbox), OR
//    - The event is INSIDE this parent's bounds (isInside == true)
    if(!mClipChildrenHitbox || isInside) {
      for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
        AWidget* child = it->get();
        if(!child->Visible() || !child->Enabled())
          continue;
// Pass OUR local coordinates down to child
        if(AWidget* consumed = (child->*childHandler)(localX, localY)) {
          return consumed;// Child handled it
        }
      }
    }
// 3. Self Check
    if(isInside) {
      if(callback) {
        callback(this, callbackData, localX, localY);
      }
      if(callback || mConsumeMouseEvents) {
        return this;
      }
    }
    return nullptr;
  }

// Helper for 3-parameter handlers taking a button ID (x, y, button)
  AWidget* AWidget::ProcessMouseEventEx(int32_t x, int32_t y, uint32_t button,
      AWidget* (AWidget::*childHandler)(int32_t, int32_t, uint32_t), MouseButtonCallback3 callback,
      void* callbackData) {
    if(!mVisible || !mEnabled)
      return nullptr;
    auto [localX, localY] = CalculateCoordsRotated(x, y);
    bool isInsideThisWidget = (localX >= 0 && localX < static_cast<int32_t>(mSizeX) && localY >= 0
        && localY < static_cast<int32_t>(mSizeY));
    if(mClipChildrenHitbox && !isInsideThisWidget) {
      return nullptr;
    }
    for(auto it = mWidg.rbegin(); it != mWidg.rend(); ++it) {
      AWidget* child = it->get();
      if(!child->Visible() || !child->Enabled())
        continue;
      AWidget* consumed = (child->*childHandler)(localX, localY, button);
      if(consumed != nullptr) {
        return consumed;
      }
    }
    if(isInsideThisWidget) {
      if(callback) {
        callback(this, callbackData, x, y, button);
        return this;
      }
      if(mConsumeMouseEvents) {
        return this;
      }
    }
    return nullptr;
  }

  AWidget* AWidget::OnMouseDownRight(int32_t x, int32_t y) {
    return ProcessMouseEvent(x, y, &AWidget::OnMouseDownRight, mMousePressRightCallback, mMousePressRightCallbackData);
  }

  AWidget* AWidget::OnMouseDownMiddle(int32_t x, int32_t y) {
    return ProcessMouseEvent(x, y, &AWidget::OnMouseDownMiddle, mMousePressMiddleCallback,
        mMousePressMiddleCallbackData);
  }

  AWidget* AWidget::OnMouseDownOther(int32_t x, int32_t y, uint32_t button) {
    return ProcessMouseEventEx(x, y, button, &AWidget::OnMouseDownOther, mMousePressOtherCallback,
        mMousePressOtherCallbackData);
  }
// ============================================================================
// Mouse Release Handlers (4)
// ============================================================================
  AWidget* AWidget::OnMouseUpRight(int32_t x, int32_t y) {
    return ProcessMouseEvent(x, y, &AWidget::OnMouseUpRight, mMouseReleaseRightCallback, mMouseReleaseRightCallbackData);
  }

  AWidget* AWidget::OnMouseUpMiddle(int32_t x, int32_t y) {
    return ProcessMouseEvent(x, y, &AWidget::OnMouseUpMiddle, mMouseReleaseMiddleCallback,
        mMouseReleaseMiddleCallbackData);
  }

  AWidget* AWidget::OnMouseUpOther(int32_t x, int32_t y, uint32_t button) {
    return ProcessMouseEventEx(x, y, button, &AWidget::OnMouseUpOther, mMouseReleaseOtherCallback,
        mMouseReleaseOtherCallbackData);
  }

  void AWidget::SetMousePressLeftCallback(MouseButtonCallback callback, void* anyData = nullptr) {
    mMousePressLeftCallback = std::move(callback);
    mMousePressLeftCallbackData = anyData;
    D3("userData={}", (uint64_t)anyData)
  }

  void AWidget::SetMousePressRightCallback(MouseButtonCallback callback, void* anyData = nullptr) {
    mMousePressRightCallback = std::move(callback);
    mMousePressRightCallbackData = anyData;
    D2("userData={}", (uint64_t)anyData)
  }

  void AWidget::SetMousePressMiddleCallback(MouseButtonCallback callback, void* anyData = nullptr) {
    mMousePressMiddleCallback = std::move(callback);
    mMousePressMiddleCallbackData = anyData;
    D2("userData={}", (uint64_t)anyData)
  }

  void AWidget::SetMousePressOtherCallback(MouseButtonCallback3 callback, void* anyData = nullptr) {
    mMousePressOtherCallback = std::move(callback);
    mMousePressOtherCallbackData = anyData;
    D2("userData={}", (uint64_t)anyData)
  }

  void AWidget::SetMouseReleaseLeftCallback(MouseButtonCallback callback, void* anyData = nullptr) {
    mMouseReleaseLeftCallback = std::move(callback);
    mMouseReleaseLeftCallbackData = anyData;
    D2("Data={}", (uint64_t)anyData)
  }

  void AWidget::SetMouseReleaseRightCallback(MouseButtonCallback callback, void* anyData = nullptr) {
    mMouseReleaseRightCallback = std::move(callback);
    mMouseReleaseRightCallbackData = anyData;
    D2("Data={}", (uint64_t)anyData)
  }

  void AWidget::SetMouseReleaseMiddleCallback(MouseButtonCallback callback, void* anyData = nullptr) {
    mMouseReleaseMiddleCallback = std::move(callback);
    mMouseReleaseMiddleCallbackData = anyData;
    D2("Data={}", (uint64_t)anyData)
  }

  void AWidget::SetMouseReleaseOtherCallback(MouseButtonCallback3 callback, void* anyData = nullptr) {
    mMouseReleaseOtherCallback = std::move(callback);
    mMouseReleaseOtherCallbackData = anyData;
    D2("Data={}", (uint64_t)anyData)
  }

  void AWidget::SetMouseClickCallback(MouseButtonCallback callback, void* anyData = nullptr) {
    mMouseClickCallback = std::move(callback);
    mMouseClickCallbackData = anyData;
    D2("Data={}", (uint64_t)anyData)
  }

  void AWidget::SetMouseMoveCallback(MouseMoveCallback callback, void* anyData) {
    mMouseMoveCallback = std::move(callback);
    mMouseMoveCallbackData = anyData;
    D2("Data={}", (uint64_t)anyData)
  }

  void AWidget::TrimToText() {
    if(mText.empty()) {
// Optionally set a minimum size or leave as is.
// For now, do nothing.
      D1("TrimToText: text is empty, keeping current size.");
      return;
    }
// 1. Get the font face from the engine.
    AUI* engine = mWnd ? mWnd->EnginePtr() : nullptr;
    if(!engine) {
      E("TrimToText: no valid engine or window");
      return;
    }
    FT_Face face = engine->DefaultFontFace();// assuming such a method exists
    if(!face) {
      E("TrimToText: no font face available");
      return;
    }
// 2. Set the font size (if not already set).
//    In your framework, font size is set per widget; we use mFontSize.
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
// 3. Compute the text bounding box.
//    We'll use a simple approach: sum advances for each character.
//    For more accuracy, you might want to use FT_Get_Glyph and get the bitmap size.
//    This is a basic implementation.
    int32_t textWidth = 0;
    int32_t textHeight = SafeINT32(mFontSize);// approximate height
    const char* str = mText.c_str();
    for(size_t i = 0; i < mText.length(); ++i) {
      uint32_t charCode = static_cast<uint8_t>(str[i]);
      if(FT_Load_Char(face, charCode, FT_LOAD_RENDER) != 0) {
        continue;
      }
      textWidth += SafeINT32(face->glyph->advance.x >> 6);// convert from 26.6 fixed point
    }
// 4. Add border thickness (both sides).
    uint32_t newWidth = static_cast<uint32_t>(textWidth) + 2 * mBorderThick;
    uint32_t newHeight = static_cast<uint32_t>(textHeight) + 2 * mBorderThick;
// 5. Ensure minimum size (e.g., 1x1).
    if(newWidth < 1)
      newWidth = 1;
    if(newHeight < 1)
      newHeight = 1;
    D2("TrimToText: resizing from {}x{} to {}x{} (text size {}x{}, border {})", mSizeX, mSizeY, newWidth, newHeight,
        textWidth, textHeight, mBorderThick);
// 6. Apply the new size.
    Resize(newWidth, newHeight);
  }

  AUI* AWidget::EnginePtr() {
    if(mWnd) {
      return mWnd->EnginePtr();
    }
    E("unable to retrive engine ptr");
  }

  AWidget* AWidget::MouseClick(int32_t localX, int32_t localY) {
    OnMouseDownLeft(localX, localY);
    return OnMouseUpLeft(localX, localY);
  }

  AWidget* AWidget::MouseDown(int32_t localX, int32_t localY) {
    D1("debug x{} y{}", localX, localY)
    return OnMouseDownLeft(localX, localY);
  }

  AWidget* AWidget::MouseUp(int32_t localX, int32_t localY) {
    D1("debug x{} y{}", localX, localY)
    return OnMouseUpLeft(localX, localY);
  }

  bool AWidget::MouseMove(int32_t localX, int32_t localY) {
    D1("debug x{} y{}", localX, localY)
    return OnMouseMove(localX, localY);
  }

  void AWidget::OnResize(uint32_t x, uint32_t y) {
    D2("w {} ({},{})", mText, x, y)
    if(mResizeCallback != nullptr) {
      mResizeCallback(this, mResizeCallbackData, x, y);
    }
  }

  std::pair<int32_t, int32_t> AWidget::ToLocalCoords(int32_t winX, int32_t winY) const {
    int32_t localX = winX;
    int32_t localY = winY;
// 1. Collect parent hierarchy from top root -> down to this widget
    std::vector<const AWidget*> chain;
    for(const AWidget* curr = this; curr != nullptr; curr = curr->Parent()) {
      chain.push_back(curr);
    }
// 2. Walk down from the root parent to 'this' widget
    for(auto it = chain.rbegin(); it != chain.rend(); ++it) {
      const AWidget* widget = *it;
// Subtract relative position to convert to child's coordinate space
      localX -= widget->X();
      localY -= widget->Y();
// Apply rotation/scale if this level has transformations
      std::tie(localX, localY) = widget->CalculateCoordsRotated(localX, localY);
    }
    return {localX, localY};
  }

  void AWidget::GetRotatedAABB(int32_t offsetX, int32_t offsetY, double& outMinX, double& outMaxX, double& outMinY,
      double& outMaxY) const {
    double hw = static_cast<double>(mSizeX) / 2.0;
    double hh = static_cast<double>(mSizeY) / 2.0;
// Calculate global center using your parent hierarchy math
    double cx = static_cast<double>(offsetX + X()) + hw;
    double cy = static_cast<double>(offsetY + Y()) + hh;
    double rad = AngleAbs() * (3.14159265358979323846 / 180.0);
    double cosA = std::cos(rad);
    double sinA = std::sin(rad);
    const double cornersX[4] = { -hw, hw, hw, -hw };
    const double cornersY[4] = { -hh, -hh, hh, hh };
    outMinX = 1e9;
    outMaxX = -1e9;
    outMinY = 1e9;
    outMaxY = -1e9;
    for(int32_t i = 0; i < 4; ++i) {
      double rx = cornersX[i] * cosA - cornersY[i] * sinA + cx;
      double ry = cornersX[i] * sinA + cornersY[i] * cosA + cy;
      outMinX = std::min(outMinX, rx);
      outMaxX = std::max(outMaxX, rx);
      outMinY = std::min(outMinY, ry);
      outMaxY = std::max(outMaxY, ry);
    }
  }

  void AWidget::Show() {
    mVisible = true;
    if(mWnd)
      mWnd->RequestRedraw();
  }

  void AWidget::Hide() {
    mVisible = false;
    if(mWnd)
      mWnd->RequestRedraw();
  }

  void AWidget::Visible(bool v) {
    mVisible = v;
    if(mWnd)
      mWnd->RequestRedraw();
  }

  void AWidget::LayoutUpdate() {
    D2("{}", Text())
    for(const auto& child : mWidg) {
      child->LayoutDirty();
      child->LayoutUpdate();
    }
  }

  bool AWidget::Focused() const {
    if(!Wnd())
      return false;
    return Wnd()->FocusedWidget() == this;
  }

  uint32_t AWidget::ShiftColor(uint32_t color, bool doubleShift) const {
    uint8_t a = static_cast<uint8_t>((color >> 24) & 0xFFU);
    uint8_t r = static_cast<uint8_t>((color >> 16) & 0xFFU);
    uint8_t g = static_cast<uint8_t>((color >> 8) & 0xFFU);
    uint8_t b = static_cast<uint8_t>(color & 0xFFU);
// Compute perceived luminance (simple average, or you can use weighted)
    uint32_t luminance = (static_cast<uint32_t>(r) + g + b) / 3;
// Determine shift amount and direction
    int32_t shift = static_cast<int32_t>(AUI_HL_SHIFT);
    if(doubleShift)
      shift *= 2;
// Light colors (luminance > 128) should darken (negative shift)
    if(luminance > 128)
      shift = -shift;
    auto clamp = [](int32_t val) -> uint8_t {
      if(val < 0)
        return 0;
      if(val > 255)
        return 255;
      return static_cast<uint8_t>(val);
    };
    r = clamp(static_cast<int32_t>(r) + shift);
    g = clamp(static_cast<int32_t>(g) + shift);
    b = clamp(static_cast<int32_t>(b) + shift);
    return (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8)
        | static_cast<uint32_t>(b);
  }

  void AWidget::Init() {
    if(!mInitDone) {
      mInitDone = true;
      D4("default init done")
    }
  }

  void AWidget::OnMouseDownLeftInternal(UNUSED int32_t localX, UNUSED int32_t localY) {
    D4("placeholder method")
  }

  void AWidget::OnMouseUpLeftInternal(UNUSED int32_t localX, UNUSED int32_t localY) {
    D4("placeholder method")
  }

  void AWidget::BringToFront(AWidget* child) {
    auto it = std::find_if(mWidg.begin(), mWidg.end(), [child](const std::unique_ptr<AWidget>& ptr) {
      return ptr.get() == child;
    });
    if(it != mWidg.end() && it != mWidg.end() - 1) {
      std::unique_ptr<AWidget> ptr = std::move(*it);
      mWidg.erase(it);
      mWidg.push_back(std::move(ptr));
    }
  }

  void AWidget::Pressed(bool v) {
    D2("v {}", v)
    mPressed = v;
    if(mWnd) {
      mWnd->RequestRedraw();
    }
  }

  int32_t AWidget::ComputeTextWidth(const std::string& text) const {
    AUI* engine = Wnd() ? Wnd()->EnginePtr() : nullptr;
    if(!engine)
      return 0;
    FT_Face face = engine->DefaultFontFace();
    if(!face)
      return 0;
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    int32_t width = 0;
    for(char c : text) {
      if(FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_COLOR) == 0) {
        width += static_cast<int32_t>(face->glyph->advance.x >> 6);
      }
    }
    return width;
  }

  void AWidget::RemoveWidget(AWidget* v) {
    AWindow* wndp = Wnd();
    if (!wndp) E("attempt to remove widget with no window");
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
      wndp->RequestRedraw();
    } else {
      D2("widget not deleted");
    }
  }

}// namespace aui

