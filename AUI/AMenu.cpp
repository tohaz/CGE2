#include "AUILib.h"

namespace aui {

  AMenu::AMenu() {
    mType = AUIWidgetType::defaultMenu;
    mBGColor = 0xFFEEEEEE;
    mBorderThick = 1;
    mBorderColor = 0xFF888888;
    mTextColor = 0xFF000000;
    mFontSize = 14;
    mDefaultFillBG = true;
    mDefaultDrawBorder = true;
    ClipChildren(false);// we handle child drawing manually
    ClipChildrenHitbox(false);
    DefaultFillBG(false);
    DefaultDrawBorder(false);
  }

  AMenu::AMenu(std::vector<AMenuItem>&& items, AUIOrientation orient) :
      AMenu() {
    mItems = std::move(items);
    mOrientation = orient;
    LayoutDirty();
  }

  AMenu::~AMenu() {
    if(mParentMenu && mParentMenu->mActiveSubMenu == this) {
      mParentMenu->mActiveSubMenu = nullptr;
      mParentMenu->mActiveSubMenuOwnerIndex = -1;
    }
    if(Wnd()) {
      if(Wnd()->ActiveMenu() == this)
        Wnd()->ActiveMenu(nullptr);
      if(Wnd()->PermanentMenu() == this)
        Wnd()->PermanentMenu(nullptr);
    }
  }

  void AMenu::SetItems(std::vector<AMenuItem>&& items) {
    mItems = std::move(items);
    LayoutDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AMenu::AddItem(AMenuItem item) {
    mItems.push_back(std::move(item));
    LayoutDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AMenu::ClearItems() {
    mItems.clear();
    LayoutDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AMenu::Orientation(AUIOrientation o) {
    if(mOrientation == o)
      return;
    mOrientation = o;
    LayoutDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AMenu::ItemHeight(int32_t h) {
    if(mItemHeight == h)
      return;
    mItemHeight = h;
    LayoutDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AMenu::SetColors(uint32_t bg, uint32_t hoverBg, uint32_t text, uint32_t disabled) {
    mBGColor = bg;
    mHoverBg = hoverBg;
    mTextColor = text;
    mDisabledColor = disabled;
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AMenu::RecalcLayout() const {
    if(!mLayoutDirty)
      return;
    size_t n = mItems.size();
    mItemX.resize(n);
    mItemY.resize(n);
    mItemW.resize(n);
    mItemH.resize(n);
    int32_t maxTextW = 0;
    for(const auto& it : mItems) {
      if(it.mSeparator || !it.mVisible)
        continue;
      int32_t tw = ComputeTextWidth(it.mText);
      if(tw > maxTextW)
        maxTextW = tw;
    }
    int32_t itemW = maxTextW + 2 * mPadding;
    if(mOrientation == AUIOrientation::vertical) {
      int32_t y = 0;
      for(size_t i = 0; i < n; ++i) {
        int32_t h = mItems[i].mSeparator ? mSeparatorSize : mItemHeight;
        mItemX[i] = 0;
        mItemY[i] = y;
        mItemW[i] = itemW;
        mItemH[i] = h;
        y += h;
      }
      mCachedWidth = itemW;
      mCachedHeight = y;
    }
    else {
      int32_t x = 0;
      for(size_t i = 0; i < n; ++i) {
        int32_t w;
        if(mItems[i].mSeparator) {
          w = 2;
        }
        else {
          int32_t tw = ComputeTextWidth(mItems[i].mText);
          w = tw + 2 * mPadding;
          if(w < 20)
            w = 20;
        }
        mItemX[i] = x;
        mItemY[i] = 0;
        mItemW[i] = w;
        mItemH[i] = mItemHeight;
        x += w;
      }
      mCachedWidth = x;
      mCachedHeight = mItemHeight;
    }
    mLayoutDirty = false;
  }

  int32_t AMenu::HitTest(int32_t x, int32_t y) const {
    RecalcLayout();
    for(size_t i = 0; i < mItems.size(); ++i) {
      if(!mItems[i].mVisible)
        continue;
      if(x >= mItemX[i] && x < mItemX[i] + mItemW[i] && y >= mItemY[i] && y < mItemY[i] + mItemH[i]) {
        return static_cast<int32_t>(i);
      }
    }
    return -1;
  }

  void AMenu::OnDraw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
      int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const {
    (void) clipL;
    (void) clipT;
    (void) clipR;
    (void) clipB;
    if(!mVisible || mItems.empty())
      return;
    RecalcLayout();
    int32_t absX = offsetX + X();
    int32_t absY = offsetY + Y();
    FillRect(buffer, bufferW, absX, absY, mCachedWidth, mCachedHeight, mBGColor);
// Draw own items
    for(size_t i = 0; i < mItems.size(); ++i) {
      const auto& item = mItems[i];
      if(!item.mVisible)
        continue;
      int32_t x = absX + mItemX[i];
      int32_t y = absY + mItemY[i];
      int32_t w = mItemW[i];
      int32_t h = mItemH[i];
      if(item.mSeparator) {
        uint32_t color = 0xFF666666;
        if(mOrientation == AUIOrientation::vertical) {
          int32_t lineY = y + h / 2;
          DrawHLine(buffer, bufferW, x + mPadding, lineY, w - 2 * mPadding, color);
        }
        else {
          int32_t lineX = x + w / 2;
          DrawVLine(buffer, bufferW, lineX, y + mPadding, h - 2 * mPadding, color);
        }
        continue;
      }
      if(mHoveredIndex == static_cast<int32_t>(i))
        FillRect(buffer, bufferW, x, y, w, h, mHoverBg);
      uint32_t color = item.mEnabled ? mTextColor : mDisabledColor;
      FT_Face face = Wnd()->EnginePtr()->DefaultFontFace();
      if(face) {
        ARect bounds { x + mPadding, y, static_cast<uint32_t>(w - 2 * mPadding), static_cast<uint32_t>(h) };
        ATextStyle style { color, mFontSize, AUIHAlign::left, AUIVAlign::center, 0.0 };
        DrawTextEx(buffer, bufferW, bufferH, bounds, item.mText, face, style, nullptr);
      }
    }
// Draw active submenu (if any)
    if(mActiveSubMenu && mActiveSubMenu->IsVisible()) {
// Submenu is a child widget; we can call its Draw directly.
// We need to pass correct offset (which is 0,0 because it's a child of this? Actually offset should be relative to this menu's position)
// Since the submenu is a child, its position is relative to this menu, so we pass offsetX+X(), offsetY+Y()
      mActiveSubMenu->Draw(buffer, bufferW, bufferH, offsetX + X(), offsetY + Y(), clipL, clipT, clipR, clipB);
    }
  }

  AWidget* AMenu::OnMouseDownLeft(int32_t localX, int32_t localY) {
// If we have an active submenu, forward the event to it first.
    if(mActiveSubMenu && mActiveSubMenu->IsVisible()) {
// Convert to submenu's local coordinates (submenu is a child of this)
      int32_t subLocalX = localX - mActiveSubMenu->X();
      int32_t subLocalY = localY - mActiveSubMenu->Y();
      AWidget* consumed = mActiveSubMenu->OnMouseDownLeft(subLocalX, subLocalY);
      if(consumed)
        return consumed;
    }
// If click is outside this menu, dismiss (if not permanent)
    bool inside = (localX >= 0 && localX < static_cast<int32_t>(mSizeX) && localY >= 0
        && localY < static_cast<int32_t>(mSizeY));
    if(!inside) {
      if(!mIsPermanent)
        Dismiss();
      return nullptr;
    }
    int32_t idx = HitTest(localX, localY);
    if(idx < 0)
      return nullptr;
    const auto& item = mItems[static_cast<size_t>(idx)];
    if(!item.mEnabled || item.mSeparator)
      return nullptr;
// Toggle submenu
    if(!item.subItems.empty()) {
      if(mActiveSubMenu && mActiveSubMenuOwnerIndex == idx) {
        CloseSubMenu();
      }
      else {
        CloseSubMenu();
        OpenSubMenu(static_cast<size_t>(idx));
      }
      return this;
    }
// Execute action and dismiss (if not permanent)
    if (item.actionWithData) {
        item.actionWithData(this, item.userData);
    } else if(item.action)
      item.action();
    if(!mIsPermanent) {
      Dismiss();
    }
    return nullptr;
  }

  bool AMenu::OnMouseMove(int32_t localX, int32_t localY) {
// Forward to active submenu first if it exists and is visible
    if(mActiveSubMenu && mActiveSubMenu->IsVisible()) {
      int32_t subLocalX = localX - mActiveSubMenu->X();
      int32_t subLocalY = localY - mActiveSubMenu->Y();
      if(subLocalX >= 0 && subLocalX < static_cast<int32_t>(mActiveSubMenu->SizeX()) && subLocalY >= 0
          && subLocalY < static_cast<int32_t>(mActiveSubMenu->SizeY())) {
        mActiveSubMenu->OnMouseMove(subLocalX, subLocalY);
        return true;
      }
// If mouse left the submenu, we may want to close it if we move to a different root item
// But we'll handle that below when we detect hover change.
    }
    bool inside = (localX >= 0 && localX < static_cast<int32_t>(mSizeX) && localY >= 0
        && localY < static_cast<int32_t>(mSizeY));
    int32_t newHover = inside ? HitTest(localX, localY) : -1;
    if(newHover != mHoveredIndex) {
// If we were hovering over an item that had an open submenu, close it
      if(mHoveredIndex >= 0 && mActiveSubMenu && mActiveSubMenuOwnerIndex == mHoveredIndex) {
        CloseSubMenu();
      }
      mHoveredIndex = newHover;
      mLastHoverTime = std::chrono::steady_clock::now();
      if(Wnd())
        Wnd()->RequestRedraw();
    }
// Open new submenu after delay if applicable
    if(mHoveredIndex >= 0 && mHoveredIndex < static_cast<int32_t>(mItems.size())) {
      const auto& item = mItems[static_cast<size_t>(mHoveredIndex)];
      if(!item.subItems.empty() && item.mEnabled && !mActiveSubMenu) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - mLastHoverTime).count();
        if(elapsed >= mSubmenuDelayMs) {
          OpenSubMenu(static_cast<size_t>(mHoveredIndex));
        }
      }
    }
    return true;
  }

  void AMenu::Show() {
    if(mVisible)
      return;
    RecalcLayout();
    if(mCachedWidth > 0 && mCachedHeight > 0) {
      Resize(static_cast<uint32_t>(mCachedWidth), static_cast<uint32_t>(mCachedHeight));
    }
    else {
      Resize(100, SafeUINT32(mItemHeight));
    }
    mVisible = true;
    mHoveredIndex = -1;
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AMenu::Popup(int32_t x, int32_t y) {
    if(mVisible)
      return;
    RecalcLayout();
    int32_t w = mCachedWidth, h = mCachedHeight;
    if(w == 0 || h == 0) {
      w = 100;
      h = mItemHeight;
    }
    if(Wnd()) {
      int32_t winW = static_cast<int32_t>(Wnd()->SizeX());
      int32_t winH = static_cast<int32_t>(Wnd()->SizeY());
      x = std::max(0, std::min(x, winW - w));
      y = std::max(0, std::min(y, winH - h));
    }
    Move(x, y);
    Resize(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
    mVisible = true;
    mHoveredIndex = -1;
// Only push modal for top-level popups (not permanent, not a submenu)
    if(!mIsPermanent && !mParentMenu && Wnd()) {
      Wnd()->PushModal(this);
    }
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AMenu::Dismiss() {
    if (mParentMenu) {
        mParentMenu->OnSubmenuDismissed(this);
    }
    if(!mVisible)
      return;
    CloseSubMenu();
    if(Wnd()) {
      Wnd()->CapturedWidgetLeft(nullptr);
      if(Wnd()->FocusedWidget() == this)
        Wnd()->FocusedWidget(nullptr);
      if(Wnd()->ActiveMenu() == this)
        Wnd()->ActiveMenu(nullptr);
    }
    if(!mIsPermanent && !mParentMenu && Wnd()) {
      Wnd()->RemoveModal(this);
    }
    mVisible = false;
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AMenu::OpenSubMenu(size_t index) {
    if(index >= mItems.size())
      return;
    const auto& item = mItems[index];
    if(item.subItems.empty())
      return;
    CloseSubMenu();// close any existing submenu
// Create submenu as a child of this menu
    AMenu* sub = new AMenu(std::vector<AMenuItem>(item.subItems), AUIOrientation::vertical);
    sub->mParentMenu = this;// mark as submenu
    sub->Wnd(this->Wnd());
    sub->SetPermanent(false);
    sub->SetColors(mBGColor, mHoverBg, mTextColor, mDisabledColor);
    sub->ItemHeight(mItemHeight);
    sub->mPadding = mPadding;
    sub->FontSize(mFontSize);
// Position relative to parent
    int32_t spawnX = mItemX[index];
    int32_t spawnY = mItemY[index];
    if(mOrientation == AUIOrientation::vertical) {
      spawnX += mItemW[index] - 2;
    }
    else {
      spawnY += mItemH[index];
    }
    sub->Move(spawnX, spawnY);
// Compute submenu size (it will recalc its own layout)
    sub->RecalcLayout();
    int32_t sw = sub->mCachedWidth, sh = sub->mCachedHeight;
    if(sw == 0 || sh == 0) {
      sw = 100;
      sh = mItemHeight;
    }
    sub->Resize(static_cast<uint32_t>(sw), static_cast<uint32_t>(sh));
// Add as a child – this transfers ownership to this menu
    AddWidget(std::unique_ptr<AWidget>(sub));
// Set state
    sub->mVisible = true;
    mActiveSubMenu = sub;
    mActiveSubMenuOwnerIndex = static_cast<int32_t>(index);
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AMenu::CloseSubMenu() {
    if(mActiveSubMenu) {
      AMenu* subToDestroy = mActiveSubMenu;
      mActiveSubMenu = nullptr;
      mActiveSubMenuOwnerIndex = -1;
      mHoveredIndex = -1;
      if(Wnd()) {
        RemoveWidget(subToDestroy);
        Wnd()->RequestRedraw();
      }
    }
  }
  void AMenu::OnResize(uint32_t w, uint32_t h) {
    AWidget::OnResize(w, h);
    LayoutDirty();
  }

  void AMenu::OnSubmenuDismissed(AMenu* submenu) {
    if(submenu == mActiveSubMenu) {
      mActiveSubMenu = nullptr;
      mActiveSubMenuOwnerIndex = -1;
      mHoveredIndex = -1;// clear the highlight on the root item
      if(Wnd())
        Wnd()->RequestRedraw();
    }
  }

}// namespace aui
