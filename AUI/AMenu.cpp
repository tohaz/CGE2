#include "AUILib.h"

namespace aui {
  AMenu *AMenu::sActiveTopMenu = nullptr;
  AMenu *AMenu::sPermanentMenu = nullptr;

  AMenu::AMenu() {
    D3("AMenu::AMenu() this={}", (void*)this);
    mWidgetType = AUIWidgetType::defaultMenu;
    mBGColor = 0xFFEEEEEE;
    mBorderThick = 1;
    mBorderColor = 0xFF888888;
    mTextColor = 0xFF000000;
    mFontSize = 14;
    mHoverEnabled = true;
    mFocusable = false;
    mMinSizeX = 10;
    mMinSizeY = 10;
    mLayoutDirty = true;
  }

  AMenu::AMenu(const std::vector<AMenuItem> &items, AUIOrientation orient) :
      AMenu() {
    D3("AMenu::AMenu(items, orient={}) this={}", static_cast<int32_t>(orient), (void*)this);
    SetItems(items);
    SetOrientation(orient);
  }
// ------------------------------------------------------------------
// Static active‑menu management
// ------------------------------------------------------------------
  void AMenu::DismissActiveMenu() {
    if(sActiveTopMenu) {
      sActiveTopMenu->Dismiss();
      sActiveTopMenu = nullptr;
    }
  }

  bool AMenu::IsActiveMenuVisible() {
    return sActiveTopMenu != nullptr && sActiveTopMenu->IsVisible();
  }

  bool AMenu::IsPointInsideActiveMenu(int32_t x, int32_t y) {
    if(!sActiveTopMenu)
      return false;
    return sActiveTopMenu->IsPointInsideHierarchy(x, y);
  }

  bool AMenu::IsPointInsideMenuHierarchy(const AMenu *menu, int32_t x, int32_t y) {
    if(!menu)
      return false;
    return menu->IsPointInsideHierarchy(x, y);
  }

  bool AMenu::IsPointInsideHierarchy(int32_t x, int32_t y) const {
    if(x >= mX && x < mX + static_cast<int32_t>(mSizeX) && y >= mY && y < mY + static_cast<int32_t>(mSizeY)) {
      return true;
    }
    if(mActiveSubMenu && mActiveSubMenu->IsVisible()) {
      return mActiveSubMenu->IsPointInsideHierarchy(x, y);
    }
    return false;
  }
// ------------------------------------------------------------------
// Permanent menu support
// ------------------------------------------------------------------
  void AMenu::SetPermanent(bool permanent) {
    if(mIsPermanent == permanent)
      return;
    mIsPermanent = permanent;
    if(permanent) {
      sPermanentMenu = this;
    }
    else if(sPermanentMenu == this) {
      sPermanentMenu = nullptr;
    }
  }
// ------------------------------------------------------------------
// AttachTo
// ------------------------------------------------------------------
  AMenu* AMenu::AttachTo(AWindow *parent, const std::vector<AMenuItem> &items) {
    if(!parent) {
      E("AMenu::AttachTo: parent window is null");
      return nullptr;
    }

    AMenu *menu = new AMenu(items);
    if(menu->GetItemCount() == 0)
      menu->AddItem(AMenuItem("Menu"));
    menu->mParentWindow = parent;
    parent->AddWidget(std::unique_ptr<AWidget>(menu));
    D2("AMenu::AttachTo window: menu={}, items={}", (void*)menu, menu->GetItemCount());
    return menu;
  }

  AMenu* AMenu::AttachTo(AWidget *parent, const std::vector<AMenuItem> &items) {
    if(!parent) {
      E("AMenu::AttachTo: parent widget is null");
      return nullptr;
    }
    AMenu *menu = new AMenu(items);
    if(menu->GetItemCount() == 0)
      menu->AddItem(AMenuItem("Menu"));
    if(parent->GetParentWindow())
      menu->mParentWindow = parent->GetParentWindow();
    parent->AddWidget(std::unique_ptr<AWidget>(menu));
    D1("AMenu::AttachTo widget: menu={}, items={}", (void*)menu, menu->GetItemCount());
    return menu;
  }
// ------------------------------------------------------------------
// Content management
// ------------------------------------------------------------------
  void AMenu::SetItems(const std::vector<AMenuItem> &items) {
    D2("AMenu::SetItems: count={}", items.size());
    mItems = items;
    mLayoutDirty = true;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AMenu::AddItem(const AMenuItem &item) {
    D2("AMenu::AddItem: text='{}'", item.text);
    mItems.push_back(item);
    mLayoutDirty = true;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AMenu::InsertItem(size_t index, const AMenuItem &item) {
    if(index > mItems.size())
      index = mItems.size();
    mItems.insert(mItems.begin() + static_cast<ptrdiff_t>(index), item);
    mLayoutDirty = true;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AMenu::RemoveItem(size_t index) {
    if(index >= mItems.size())
      return;
    mItems.erase(mItems.begin() + static_cast<ptrdiff_t>(index));
    mLayoutDirty = true;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AMenu::ClearItems() {
    mItems.clear();
    mLayoutDirty = true;
    if(mParentWindow)
      mParentWindow->Draw();
  }
// ------------------------------------------------------------------
// Appearance
// ------------------------------------------------------------------
  void AMenu::SetOrientation(AUIOrientation orient) {
    if(mOrientation == orient)
      return;
    D2("AMenu::SetOrientation: {} -> {}", static_cast<int32_t>(mOrientation), static_cast<int32_t>(orient));
    mOrientation = orient;
    mLayoutDirty = true;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AMenu::SetItemHeight(int32_t height) {
    if(mItemHeight == height)
      return;
    D2("AMenu::SetItemHeight: {}", height);
    mItemHeight = height;
    mLayoutDirty = true;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AMenu::SetColors(uint32_t bg, uint32_t hoverBg, uint32_t text, uint32_t disabled, uint32_t separator) {
    D2("AMenu::SetColors");
    mBGColor = bg;
    mHoverBgColor = hoverBg;
    mTextColor = text;
    mDisabledColor = disabled;
    mSeparatorColor = separator;
    if(mParentWindow)
      mParentWindow->Draw();
  }
// ------------------------------------------------------------------
// Popup / Dismiss
// ------------------------------------------------------------------
  void AMenu::Popup(int32_t x, int32_t y) {
    D2("AMenu::Popup: pos=({},{}), visible={}", x, y, mVisible);
    RecalcLayout();
    int32_t w = mCachedWidth;
    int32_t h = mCachedHeight;
    if(mParentWindow) {
      int32_t winW = static_cast<int32_t>(mParentWindow->SizeX());
      int32_t winH = static_cast<int32_t>(mParentWindow->SizeY());
      if(x + w > winW)
        x = winW - w;
      if(y + h > winH)
        y = winH - h;
      if(x < 0)
        x = 0;
      if(y < 0)
        y = 0;
    }
    Move(x, y);
    Resize(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
    mVisible = true;
    if(!mParentMenu && !mIsPermanent)
      sActiveTopMenu = this;
    if(mIsPermanent)
      sPermanentMenu = this;
    mHoveredIndex = -1;
    CloseSubMenu();
    if(mParentWindow)
      mParentWindow->Draw();
    D2("AMenu::Popup: size=({},{}), pos=({},{})", w, h, mX, mY);
  }

  void AMenu::Dismiss() {
    if(!mVisible)
      return;
    CloseSubMenu();
    if(mIsPermanent) {
      if(mParentWindow)
        mParentWindow->Draw();
      return;
    }
    if(mParentMenu) {
      mParentMenu->mActiveSubMenu = nullptr;
      DetachFromParent();// deletes this submenu
      return;
    }
    mVisible = false;
    if(mParentWindow)
      mParentWindow->Draw();
  }

// ------------------------------------------------------------------
// Layout helpers
// ------------------------------------------------------------------
  int32_t AMenu::ComputeTextWidth(const std::string &text) const {
    if(!mEnginePtr) {
      D1("AMenu::ComputeTextWidth: mEnginePtr is null");
      return 0;
    }
    std::unique_lock lock(mEnginePtr->GetFontMutex(), std::chrono::milliseconds(50));
    if (!lock.owns_lock()) { E("locked"); }
    FT_Face face = mEnginePtr->GetDefaultFontFace();
    if(!face) {
      D1("AMenu::ComputeTextWidth: face is null");
      return 0;
    }
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    int32_t width = 0;
    for(char c : text) {
      if(FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_COLOR) == 0) {
        width += static_cast<int32_t>(face->glyph->advance.x >> 6);
      }
    }
    D2("AMenu::ComputeTextWidth: '{}' -> {}", text, width);
    return width;
  }

  int32_t AMenu::GetMaxTextWidth() const {
    int32_t maxW = 0;
    for(const auto &item : mItems) {
      if(item.isSeparator || !item.isVisible)
        continue;
      int32_t w = ComputeTextWidth(item.text);
      if(w > maxW)
        maxW = w;
    }
    D2("AMenu::GetMaxTextWidth: {}", maxW);
    return maxW;
  }

  void AMenu::RecalcLayout() const {
    if(!mLayoutDirty)
      return;
    D2("AMenu::RecalcLayout: this={}, items={}, orient={}", (void*)this, mItems.size(),
        static_cast<int32_t>(mOrientation));
    const size_t n = mItems.size();
    mItemX.resize(n);
    mItemY.resize(n);
    mItemW.resize(n);
    mItemH.resize(n);
    int32_t totalW = 0, totalH = 0;
    int32_t maxTextW = GetMaxTextWidth();
    int32_t itemW = maxTextW + mPadding * 2 + mSubmenuArrowWidth;
    if(mMinWidth > 0 && itemW < mMinWidth)
      itemW = mMinWidth;
    D2("  maxTextW={}, itemW={}, mMinWidth={}", maxTextW, itemW, mMinWidth);
    if(mOrientation == AUIOrientation::vertical) {
      int32_t y = 0;
      for(size_t i = 0; i < n; ++i) {
        const auto &it = mItems[i];
        int32_t h = it.isSeparator ? mSeparatorSizeY : mItemHeight;
        mItemX[i] = 0;
        mItemY[i] = y;
        mItemW[i] = itemW;
        mItemH[i] = h;
        y += h;
      }
      totalW = itemW;
      totalH = y;
    }
    else {// horizontal
      int32_t x = 0;
      for(size_t i = 0; i < n; ++i) {
        const auto &it = mItems[i];
        int32_t w;
        if(it.isSeparator) {
          w = 2;
        }
        else {
          int32_t tw = ComputeTextWidth(it.text);
          w = tw + mPadding * 2 + mSubmenuArrowWidth;
          if(w < 20)
            w = 20;
        }
        mItemX[i] = x;
        mItemY[i] = 0;
        mItemW[i] = w;
        mItemH[i] = mItemHeight;
        x += w;
      }
      totalW = x;
      totalH = mItemHeight;
    }
    mCachedWidth = totalW;
    mCachedHeight = totalH;
    mLayoutDirty = false;
    D2("AMenu::RecalcLayout: totalW={}, totalH={}", totalW, totalH);
  }

  int32_t AMenu::HitTest(int32_t localX, int32_t localY) const {
    RecalcLayout();
    for(size_t i = 0; i < mItems.size(); ++i) {
      if(!mItems[i].isVisible)
        continue;
      if(localX >= mItemX[i] && localX < mItemX[i] + mItemW[i] && localY >= mItemY[i]
          && localY < mItemY[i] + mItemH[i]) {
        return static_cast<int32_t>(i);
      }
    }
    return -1;
  }
// ------------------------------------------------------------------
// Drawing
// ------------------------------------------------------------------
  void AMenu::Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY) const {
    if(!mVisible || mItems.empty()) {
      D2("AMenu::Draw: not visible or empty (visible={}, items={})", mVisible, mItems.size());
      return;
    }
    D2("AMenu::Draw: this={}, mItems={}", (void*)this, mItems.size());
    RecalcLayout();
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    int32_t w = mCachedWidth;
    int32_t h = mCachedHeight;
    D2("  draw at ({},{}), size=({},{})", absX, absY, w, h);
    FillRect(buffer, parentWidth, absX, absY, w, h, mBGColor);
    DrawRectBorder(buffer, parentWidth, absX, absY, w, h, mBorderColor);
    for(size_t i = 0; i < mItems.size(); ++i) {
      int32_t itemX = absX + mItemX[i];
      int32_t itemY = absY + mItemY[i];
      int32_t itemW = mItemW[i];
      int32_t itemH = mItemH[i];
      DrawItem(buffer, static_cast<int32_t>(i), itemX, itemY, itemW, itemH, parentWidth, parentHeight);
    }
    if(mActiveSubMenu && mActiveSubMenu->IsVisible()) {
      D1("  drawing active submenu");
      mActiveSubMenu->Draw(buffer, parentWidth, parentHeight, 0, 0);
    }
  }

  void AMenu::DrawItem(uint32_t *buffer, int32_t idx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t parentWidth,
      uint32_t parentHeight) const {
    if(idx < 0 || idx >= static_cast<int32_t>(mItems.size()))
      return;
    const auto &item = mItems[static_cast<size_t>(idx)];
    if(item.isSeparator) {
      uint32_t color = mSeparatorColor;
      if(mOrientation == AUIOrientation::vertical) {
        int32_t lineY = y + h / 2;
        DrawHLine(buffer, parentWidth, x + mPadding, lineY, w - 2 * mPadding, color);
      }
      else {
        int32_t lineX = x + w / 2;
        DrawVLine(buffer, parentWidth, lineX, y + mPadding, h - 2 * mPadding, color);
      }
      return;
    }
    if(!item.isVisible)
      return;
    bool hovered = (idx == mHoveredIndex);
    uint32_t bg = hovered ? mHoverBgColor : mBGColor;
    FillRect(buffer, parentWidth, x, y, w, h, bg);
    uint32_t textColor = item.isEnabled ? mTextColor : mDisabledColor;
    int32_t textX = x + mPadding;
    int32_t textY = y;
    int32_t textW = w - 2 * mPadding - (item.subItems.empty() ? 0 : mSubmenuArrowWidth);
    int32_t textH = h;
    std::unique_lock lock(mEnginePtr->GetFontMutex(), std::chrono::milliseconds(50));
    if (!lock.owns_lock()) { E("locked"); }
    FT_Face face = mEnginePtr ? mEnginePtr->GetDefaultFontFace() : nullptr;
    if(face) {
      DrawTextEx(buffer, parentWidth, parentHeight, textX, textY, textW, textH, item.text, face, mFontSize,
          AUIHAlign::left, AUIVAlign::center, 0, textColor, textW);
    }
    if(item.isCheckable && item.isChecked) {
      const char *check = "✓";
      DrawTextEx(buffer, parentWidth, parentHeight, x + 2, y, 16, h, check, face, mFontSize, AUIHAlign::left,
          AUIVAlign::center, 0, mCheckMarkColor, 16);
    }
    if(!item.subItems.empty()) {
      const char *arrow = (mOrientation == AUIOrientation::vertical) ? "▶" : "▼";
      int32_t arrowX = x + w - mSubmenuArrowWidth + 2;
      int32_t arrowY = y;
      DrawTextEx(buffer, parentWidth, parentHeight, arrowX, arrowY, mSubmenuArrowWidth, h, arrow, face, mFontSize,
          AUIHAlign::left, AUIVAlign::center, 0, textColor, mSubmenuArrowWidth);
    }
  }
// ------------------------------------------------------------------
// Event handling
// ------------------------------------------------------------------
  bool AMenu::OnMouseClick(int32_t localX, int32_t localY, bool pressed) {
    if(!pressed || !mVisible)
      return false;
// If click is inside active sub-menu, forward to it
    if(mActiveSubMenu && mActiveSubMenu->IsVisible()) {
      int32_t subLocalX = localX - (mActiveSubMenu->X() - mX);
      int32_t subLocalY = localY - (mActiveSubMenu->Y() - mY);
      if(subLocalX >= 0 && subLocalX < static_cast<int32_t>(mActiveSubMenu->SizeX()) && subLocalY >= 0
          && subLocalY < static_cast<int32_t>(mActiveSubMenu->SizeY())) {
        return mActiveSubMenu->OnMouseClick(subLocalX, subLocalY, pressed);
      }
// Otherwise click is outside sub-menu; we'll handle it below
    }
    int32_t idx = HitTest(localX, localY);
    if(idx < 0) {
      Dismiss();// dismiss entire menu (closes sub-menu)
      return true;
    }
    if(idx >= static_cast<int32_t>(mItems.size()))
      return false;
    const auto &item = mItems[static_cast<size_t>(idx)];
    if(!item.isEnabled || !item.isVisible)
      return false;
// Item with sub-items
    if(!item.subItems.empty()) {
// Toggle off if the same item already has an open sub-menu
      if(mActiveSubMenu && mActiveSubMenuOwnerIndex == idx) {
        CloseSubMenu();// only close, do not reopen
        return true;
      }
      else {
        CloseSubMenu();// close any existing sub-menu
        OpenSubMenu(static_cast<size_t>(idx));// open new one
        return true;
      }
    }
// Normal item (no sub-items)
    bool isCheckable = item.isCheckable;
    auto actionCopy = item.action;
    if(isCheckable) {
      const_cast<AMenuItem&>(item).isChecked = !item.isChecked;
    }
    Dismiss();// closes sub-menu as well
    if(actionCopy)
      actionCopy();
    return true;
  }

  void AMenu::OnMouseMove(int32_t localX, int32_t localY) {
    if(!mVisible)
      return;
    D3("AMenu::OnMouseMove: ({},{}), activeSubMenu={}", localX, localY, (void*)mActiveSubMenu);
    if(mActiveSubMenu && mActiveSubMenu->IsVisible()) {
      int32_t subLocalX = localX - (mActiveSubMenu->X() - mX);
      int32_t subLocalY = localY - (mActiveSubMenu->Y() - mY);
      if(subLocalX >= 0 && subLocalX < static_cast<int32_t>(mActiveSubMenu->SizeX()) && subLocalY >= 0
          && subLocalY < static_cast<int32_t>(mActiveSubMenu->SizeY())) {
        mActiveSubMenu->OnMouseMove(subLocalX, subLocalY);
      }
    }
    int32_t newHover = HitTest(localX, localY);
    if(newHover != mHoveredIndex) {
      D2("  hover changed: {} -> {}", mHoveredIndex, newHover);
      if(mHoveredIndex >= 0 && !mItems[static_cast<size_t>(mHoveredIndex)].subItems.empty()) {
        CloseSubMenu();
      }
      mHoveredIndex = newHover;
      mLastHoverTime = std::chrono::steady_clock::now();
      if(mParentWindow)
        mParentWindow->Draw();
    }
    if(mHoveredIndex >= 0 && mHoveredIndex < static_cast<int32_t>(mItems.size())) {
      const auto &item = mItems[static_cast<size_t>(mHoveredIndex)];
      if(!item.subItems.empty() && item.isEnabled && !mActiveSubMenu) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - mLastHoverTime).count();
        if(elapsed >= mSubmenuDelayMs) {
          OpenSubMenu(static_cast<size_t>(mHoveredIndex));
        }
      }
    }
  }

  void AMenu::OnMouseLeave() {
    mHoveredIndex = -1;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AMenu::OnKeyEvent(const AUIKeyEvent &event) {
    if(!mVisible || !event.pressed)
      return;
    int32_t n = static_cast<int32_t>(mItems.size());
    if(n == 0)
      return;
    switch (event.code) {
      case AUIKeyCode::Down:
        if(mOrientation == AUIOrientation::vertical) {
          int32_t next = (mHoveredIndex + 1) % n;
          while (next != mHoveredIndex
              && (!mItems[static_cast<size_t>(next)].isEnabled || !mItems[static_cast<size_t>(next)].isVisible
                  || mItems[static_cast<size_t>(next)].isSeparator)) {
            next = (next + 1) % n;
          }
          mHoveredIndex = next;
          if(mParentWindow)
            mParentWindow->Draw();
        }
        return;
      case AUIKeyCode::Up:
        if(mOrientation == AUIOrientation::vertical) {
          int32_t prev = (mHoveredIndex - 1 + n) % n;
          while (prev != mHoveredIndex
              && (!mItems[static_cast<size_t>(prev)].isEnabled || !mItems[static_cast<size_t>(prev)].isVisible
                  || mItems[static_cast<size_t>(prev)].isSeparator)) {
            prev = (prev - 1 + n) % n;
          }
          mHoveredIndex = prev;
          if(mParentWindow)
            mParentWindow->Draw();
        }
        return;
      case AUIKeyCode::Right:
        if(mOrientation == AUIOrientation::horizontal) {
          int32_t next = (mHoveredIndex + 1) % n;
          while (next != mHoveredIndex
              && (!mItems[static_cast<size_t>(next)].isEnabled || !mItems[static_cast<size_t>(next)].isVisible
                  || mItems[static_cast<size_t>(next)].isSeparator)) {
            next = (next + 1) % n;
          }
          mHoveredIndex = next;
          if(mParentWindow)
            mParentWindow->Draw();
        }
        else {
          if(mHoveredIndex >= 0 && mHoveredIndex < n && !mItems[static_cast<size_t>(mHoveredIndex)].subItems.empty()) {
            OpenSubMenu(static_cast<size_t>(mHoveredIndex));
          }
        }
        return;
      case AUIKeyCode::Left:
        if(mOrientation == AUIOrientation::horizontal) {
          int32_t prev = (mHoveredIndex - 1 + n) % n;
          while (prev != mHoveredIndex
              && (!mItems[static_cast<size_t>(prev)].isEnabled || !mItems[static_cast<size_t>(prev)].isVisible
                  || mItems[static_cast<size_t>(prev)].isSeparator)) {
            prev = (prev - 1 + n) % n;
          }
          mHoveredIndex = prev;
          if(mParentWindow)
            mParentWindow->Draw();
        }
        else {
          if(mParentMenu)
            Dismiss();
        }
        return;
      case AUIKeyCode::Enter:
      case AUIKeyCode::Space:
        if(mHoveredIndex >= 0 && mHoveredIndex < n) {
          int32_t cx = mItemX[static_cast<size_t>(mHoveredIndex)] + mItemW[static_cast<size_t>(mHoveredIndex)] / 2;
          int32_t cy = mItemY[static_cast<size_t>(mHoveredIndex)] + mItemH[static_cast<size_t>(mHoveredIndex)] / 2;
          OnMouseClick(cx, cy, true);
        }
        return;
      case AUIKeyCode::Escape:
        Dismiss();
        return;
      default:
        break;
    }
  }

// ------------------------------------------------------------------
// Submenu management
// ------------------------------------------------------------------
  void AMenu::OpenSubMenu(size_t index) {
    if(index >= mItems.size())
      return;
    const auto &item = mItems[index];
    if(item.subItems.empty())
      return;
    if(mActiveSubMenu && mHoveredIndex == static_cast<int32_t>(index))
      return;
    CloseSubMenu();
    AMenu *sub = new AMenu(item.subItems, AUIOrientation::vertical);
    sub->mParentMenu = this;
    sub->mEnginePtr = this->mEnginePtr;
    sub->mParentWindow = this->mParentWindow;
    sub->SetColors(mBGColor, mHoverBgColor, mTextColor, mDisabledColor, mSeparatorColor);
    sub->SetItemHeight(mItemHeight);
    sub->SetPadding(mPadding);
    sub->SetMinWidth(mMinWidth);
    if(mParentWindow) {
      mParentWindow->AddWidget(std::unique_ptr<AWidget>(sub));
    }
    else {
      E("OpenSubMenu: no parent window – cannot register submenu");
      delete sub;
      return;
    }
    int32_t spawnX = mX + mItemX[index];
    int32_t spawnY = mY + mItemY[index];
    if(mOrientation == AUIOrientation::vertical) {
      spawnX += mItemW[index] - 2;
    }
    else {
      spawnY += mItemH[index];
    }
    sub->Popup(spawnX, spawnY);
    mActiveSubMenu = sub;
    mActiveSubMenuOwnerIndex = static_cast<int32_t>(index);
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AMenu::CloseSubMenu() {
    if(mActiveSubMenu) {
      mActiveSubMenu->Dismiss();// hide the submenu
// Do NOT call DetachFromParent() – let the window own and delete it.
      mActiveSubMenu = nullptr;
      mActiveSubMenuOwnerIndex = -1;
    }
  }

  void AMenu::DetachFromParent() {
    if(mParentWindow) {
      AWindow *win = mParentWindow;
      win->RemoveWidget(this);
    }
  }

  const AMenuItem& AMenu::GetItem(size_t index) const {
    static AMenuItem dummy;
    if(index >= mItems.size())
      return dummy;
    return mItems[index];
  }

  AMenu::~AMenu() {
    D2("AMenu::~AMenu() this={}", (void*)this);
    CloseSubMenu();
  }
}// namespace aui
