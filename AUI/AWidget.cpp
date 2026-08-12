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
    D2("+++ incoming {} {}", x, y);
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
      D2("click on self")
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
      int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const {
    AWidget* self = const_cast<AWidget*>(this);
    self->EnsureContentUpToDate();
    self->EnsureOverlayUpToDate();
    self->Composite(buffer, bufferW, bufferH, offsetX + X(), offsetY + Y(), mAngle, clipL, clipT, clipR, clipB);
  }

  void AWidget::RenderContent() {
    D4("drawing widget {}", Text())
    if(mSizeX == 0 || mSizeY == 0)
      return;
// Ensure content buffer is allocated
    size_t sz = static_cast<size_t>(mSizeX) * mSizeY;
    if(mContentBuffer.size() != sz) {
      mContentBuffer.assign(sz, 0);
    }
// Offsets to draw at (0,0) in the local buffer
    int32_t offX = -static_cast<int32_t>(mX);
    int32_t offY = -static_cast<int32_t>(mY);
// 1. Background
    if(mDefaultFillBG) {
      uint32_t bgcolor = mHL ? HLColor(mBGColor) : mBGColor;
      FillRect(mContentBuffer.data(), mSizeX, 0, 0, SafeINT32(mSizeX), SafeINT32(mSizeY), bgcolor);
    }
// 2. Custom drawing
    OnDraw(mContentBuffer.data(), mSizeX, mSizeY, offX, offY, 0, 0, static_cast<int32_t>(mSizeX),
        static_cast<int32_t>(mSizeY));
// 3. Border before children (if !mClipChildren)
    if(!mClipChildren && mDefaultDrawBorder && Border() > 0) {
      DrawBorder(mContentBuffer.data(), mSizeX, mSizeY, offX, offY, 0, 0, static_cast<int32_t>(mSizeX),
          static_cast<int32_t>(mSizeY));
    }
// 4. Compute children clip (parent's local coordinates)
    int32_t childClipL = 0;
    int32_t childClipT = 0;
    int32_t childClipR = static_cast<int32_t>(mSizeX);
    int32_t childClipB = static_cast<int32_t>(mSizeY);
    bool parentRotated = (std::abs(AngleAbs()) > 1e-6);
    if(mClipChildren && !parentRotated) {
      int32_t thick = static_cast<int32_t>(mBorderThick);
      childClipL = thick;
      childClipT = thick;
      childClipR = static_cast<int32_t>(mSizeX) - thick;
      childClipB = static_cast<int32_t>(mSizeY) - thick;
      if(childClipL > childClipR)
        childClipL = childClipR = 0;
      if(childClipT > childClipB)
        childClipT = childClipB = 0;
    }
    if(mClipChildren) {
      for(auto it = mWidg.begin(); it != mWidg.end(); ++it) {
        AWidget* child = it->get();
        if(!child->Visible() || child->Modal())
          continue;
// Compute child's rotated AABB in parent local space
        double cx = static_cast<double>(child->X()) + child->SizeX() / 2.0;
        double cy = static_cast<double>(child->Y()) + child->SizeY() / 2.0;
        double angle = child->AngleAbs();
        double rad = angle * M_PI / 180.0;
        double cosA = std::cos(rad);
        double sinA = std::sin(rad);
        double hw = child->SizeX() / 2.0;
        double hh = child->SizeY() / 2.0;
        double cornersX[4] = { -hw, hw, hw, -hw };
        double cornersY[4] = { -hh, -hh, hh, hh };
        double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
        for(int32_t i = 0; i < 4; ++i) {
          double rx = cornersX[i] * cosA - cornersY[i] * sinA + cx;
          double ry = cornersX[i] * sinA + cornersY[i] * cosA + cy;
          minX = std::min(minX, rx);
          maxX = std::max(maxX, rx);
          minY = std::min(minY, ry);
          maxY = std::max(maxY, ry);
        }
        int32_t childL = static_cast<int32_t>(std::floor(minX));
        int32_t childR = static_cast<int32_t>(std::ceil(maxX));
        int32_t childT = static_cast<int32_t>(std::floor(minY));
        int32_t childB = static_cast<int32_t>(std::ceil(maxY));
// Intersect with parent's child clip
        int32_t finalL = std::max(childL, childClipL);
        int32_t finalR = std::min(childR, childClipR);
        int32_t finalT = std::max(childT, childClipT);
        int32_t finalB = std::min(childB, childClipB);
        if(finalL < finalR && finalT < finalB) {
// *** FIX: Ensure child's overlay is up‑to‑date before compositing ***
          child->EnsureContentUpToDate();
          child->EnsureOverlayUpToDate();

          child->Composite(mContentBuffer.data(), mSizeX, mSizeY, child->X(), child->Y(), child->Angle(), finalL,
              finalT, finalR, finalB);
        }
      }
    }
    mContentDirty = false;
  }

  void AWidget::RenderOverlay() {
    if(mSizeX == 0 || mSizeY == 0)
      return;
    size_t sz = static_cast<size_t>(mSizeX) * mSizeY;
    if(mOverlayBuffer.size() != sz) {
      mOverlayBuffer.assign(sz, 0);
    }
    else {
      std::fill(mOverlayBuffer.begin(), mOverlayBuffer.end(), 0);
    }
// Border after children (if mClipChildren)
    if(mClipChildren && mDefaultDrawBorder && Border() > 0) {
      int32_t offX = -static_cast<int32_t>(mX);
      int32_t offY = -static_cast<int32_t>(mY);
      DrawBorder(mOverlayBuffer.data(), mSizeX, mSizeY, offX, offY, 0, 0, static_cast<int32_t>(mSizeX),
          static_cast<int32_t>(mSizeY));
    }
    mOverlayDirty = false;
  }

  void AWidget::Composite(uint32_t* dst, uint32_t dstW, uint32_t dstH, int32_t dstX, int32_t dstY, double angle,
      int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) {
    if(mSizeX == 0 || mSizeY == 0 || mContentBuffer.empty())
      return;
// Content – overwrite (skipZero = false)
    BlitRotated(mContentBuffer.data(), mSizeX, mSizeY, dst, dstW, dstH, dstX, dstY, angle, clipL, clipT, clipR, clipB,
        true);
// Overlay – skip transparent pixels (skipZero = true)
    BlitRotated(mOverlayBuffer.data(), mSizeX, mSizeY, dst, dstW, dstH, dstX, dstY, angle, clipL, clipT, clipR, clipB,
        true);
// If children are NOT clipped, render them directly into dst
    if(!mClipChildren) {
      for(auto it = mWidg.begin(); it != mWidg.end(); ++it) {
        AWidget* child = it->get();
        if(!child->Visible() || child->Modal())
          continue;
        child->EnsureContentUpToDate();
        child->EnsureOverlayUpToDate();
        child->Composite(dst, dstW, dstH, dstX + child->X(), dstY + child->Y(), child->Angle(), clipL, clipT, clipR,
            clipB);
      }
    }
  }

  void AWidget::MarkContentDirty() {
    mContentDirty = true;
    if(mParent)
      mParent->MarkContentDirty();// propagate up
  }

  void AWidget::MarkOverlayDirty() {
    mOverlayDirty = true;
    if(mParent)
      mParent->MarkContentDirty();// overlay is part of child’s visual
  }

  void AWidget::AllocateBuffers() {
    size_t sz = static_cast<size_t>(mSizeX) * mSizeY;
    if(mContentBuffer.size() != sz) {
      mContentBuffer.assign(sz, 0);
      mOverlayBuffer.assign(sz, 0);
      MarkDirty();
    }
  }

  void AWidget::EnsureContentUpToDate() {
    if(mContentDirty) {
      RenderContent();
    }
  }

  void AWidget::EnsureOverlayUpToDate() {
    if(mOverlayDirty) {
      RenderOverlay();
    }
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
    if(mWnd){
      mWnd->RequestRedraw();
    }
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
    AllocateBuffers();
    OnResize(szx, szy);
    if(mWnd){
      mWnd->RequestRedraw();
    }
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
    MarkContentDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AWidget::DrawBorder(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
      int32_t clipLeft, int32_t clipTop, int32_t clipRight, int32_t clipBottom) const {
    if(mBorderThick == 0)
      return;
    int32_t rectW = static_cast<int32_t>(mSizeX);
    int32_t rectH = static_cast<int32_t>(mSizeY);
    int32_t thick = static_cast<int32_t>(mBorderThick);
// Absolute widget rectangle
    int32_t left = offsetX + X();
    int32_t top = offsetY + Y();
    int32_t right = left + rectW;
    int32_t bottom = top + rectH;
// Intersect with clip and buffer
    int32_t visL = std::max( { left, clipLeft, 0 });
    int32_t visT = std::max( { top, clipTop, 0 });
    int32_t visR = std::min( { right, clipRight, static_cast<int32_t>(bufferW) });
    int32_t visB = std::min( { bottom, clipBottom, static_cast<int32_t>(bufferH) });
    if(visL >= visR || visT >= visB)
      return;
// Helper to fill a solid rectangle with clipping already applied.
// Use parameter names different from outer rectW/rectH.
    auto fillRect = [&](int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) {
      int32_t fx = std::max(x, visL);
      int32_t fy = std::max(y, visT);
      int32_t fr = std::min(x + w, visR);
      int32_t fb = std::min(y + h, visB);
      int32_t fw = fr - fx;
      int32_t fh = fb - fy;
      if(fw > 0 && fh > 0)
        FillRect(buffer, bufferW, fx, fy, fw, fh, color);
    };
    D2("DrawBorder: style=%d, thick=%d, rect=(%d,%d %dx%d)", static_cast<int>(mBorderStyle), thick, left, top, rectW,
        rectH);
    if(mBorderStyle == AUIBorderStyle::Flat) {
// Flat: just a single rectangle with mBorderColor
      fillRect(left, top, rectW, thick, mBorderColor);// top
      fillRect(left, bottom - thick, rectW, thick, mBorderColor);// bottom
      fillRect(left, top, thick, rectH, mBorderColor);// left
      fillRect(right - thick, top, thick, rectH, mBorderColor);// right
      return;
    }
// ---- Simple3D style ----
// 1. Outer 1‑pixel black contour (if thick >= 1)
    if(thick >= 1) {
      uint32_t black = 0xFF000000;// or mBorderColor
      fillRect(left, top, rectW, 1, black);// top
      fillRect(left, bottom - 1, rectW, 1, black);// bottom
      fillRect(left, top, 1, rectH, black);// left
      fillRect(right - 1, top, 1, rectH, black);// right
    }
// 2. Shading for the remaining inner border (thickness thick-1)
//    Highlight on top & left, shadow on bottom & right.
    uint32_t bg = mHL ? HLColor(mBGColor) : mBGColor;
    uint32_t highlight = ShiftColor(bg, false);// lighter
    uint32_t shadow = ShiftColor(bg, true);// darker
    if(thick > 1) {
// Top highlight (full width, excluding the black contour)
      fillRect(left + 1, top + 1, rectW - 2, thick - 1, highlight);
// Left highlight (full height, avoid overlapping corners already drawn)
      fillRect(left + 1, top + 1, thick - 1, rectH - 2, highlight);
// Bottom shadow (full width)
      fillRect(left + 1, bottom - thick, rectW - 2, thick - 1, shadow);
// Right shadow (full height)
      fillRect(right - thick, top + 1, thick - 1, rectH - 2, shadow);
    }
    D2("DrawBorder: finished Simple3D");
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
      MarkContentDirty();
      if(mWnd)
        mWnd->RequestRedraw();
    }
  }
  void AWidget::BGColor2(uint32_t bg) {
    mBGColor2 = bg;
    if(mWnd) {
      MarkContentDirty();
      if(mWnd)
        mWnd->RequestRedraw();
    }
  }
  void AWidget::BGColor3(uint32_t bg) {
    mBGColor3 = bg;
    if(mWnd) {
      MarkContentDirty();
      if(mWnd)
        mWnd->RequestRedraw();
    }
  }
  void AWidget::BGColor4(uint32_t bg) {
    mBGColor4 = bg;
    if(mWnd) {
      MarkContentDirty();
      if(mWnd)
        mWnd->RequestRedraw();
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

  void AWidget::HL(bool v) {
    if(v != mHL) {
      mHL = v;
      MarkContentDirty();
      if(mWnd != nullptr) {
        mWnd->RequestRedraw();
      }
    }
  }

  void AWidget::Text(std::string tx) {
    D4()
    mText = tx;
    MarkContentDirty();
    if(mWnd != nullptr) {
      mWnd->RequestRedraw();
    }
  }

  void AWidget::DefaultFillBG(bool v) {
    if(v != mDefaultFillBG) {
      mDefaultFillBG = v;
      MarkContentDirty();
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
    uint32_t newHeight = static_cast<uint32_t>(textHeight) + 2 * mBorderThick + 3;
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
    if(mWnd) {
      mWnd->RequestRedraw();
    }
  }

  void AWidget::Hide() {
    mVisible = false;
    if(mWnd) {
      mWnd->RequestRedraw();
    }
  }

  void AWidget::Visible(bool v) {
    mVisible = v;
    if(mWnd) {
      mWnd->RequestRedraw();
    }
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
      AllocateBuffers();
      mInitDone = true;
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
    MarkContentDirty();
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
    if(!wndp)
      E("attempt to remove widget with no window");
    std::unique_ptr<AWidget> deadWidget;
    const auto erasedCount = std::erase_if(mWidg, [v, &deadWidget](auto& up) noexcept {
      if(up.get() == v) {
        deadWidget = std::move(up);
        return true;
      }
      return false;
    });
    if(erasedCount > 0) {
      D2("widget deleted");
      wndp->RequestRedraw();
    }
    else {
      D2("widget not deleted");
    }
  }

  void AWidget::MarkDirty() {
    MarkContentDirty();
    MarkOverlayDirty();
  }

  void AWidget::Border(uint32_t border) {
    if(mBorderThick != border) {
      mBorderThick = border;
      MarkOverlayDirty();
      if(mWnd) {
        mWnd->RequestRedraw();
      }
    }
  }

  void AWidget::BorderColor(uint32_t border) {
    if(mBorderColor != border) {
      mBorderColor = border;
      MarkOverlayDirty();
      if(mWnd) {
        mWnd->RequestRedraw();
      }
    }
  }

  void AWidget::DefaultDrawBorder(bool v) {
    if(mDefaultDrawBorder != v) {
      mDefaultDrawBorder = v;
      MarkOverlayDirty();
      if(mWnd) {
        mWnd->RequestRedraw();
      }
    }
  }

  void AWidget::ClipChildren(bool clip) {
    if(mClipChildren != clip) {
      mClipChildren = clip;
      MarkOverlayDirty();
      if(mWnd) {
        mWnd->RequestRedraw();
      }
    }
  }

  void AWidget::BorderStyle(AUIBorderStyle v) {
    mBorderStyle = v;
    MarkOverlayDirty();
    if(mWnd) {
      mWnd->RequestRedraw();
    }
  }

  void AWidget::HLToggle(bool v) {
    mHLEnabled = v;
    MarkContentDirty();
    if(mWnd){
      mWnd->RequestRedraw();
    }
  }

}// namespace aui

