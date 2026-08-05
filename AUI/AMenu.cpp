#include "AUILib.h"

namespace aui {

// -----------------------------------------------------------------------------
// AMenuItemWidget implementation
// -----------------------------------------------------------------------------
//AMenuItemWidget::AMenuItemWidget(AMenu* parentMenu, size_t index)
//    : mParentMenu(parentMenu), mIndex(index) {
//    // Inherit font size from parent menu
//    if (mParentMenu)
//        FontSize(mParentMenu->FontSize());
//    // Set a default size; will be updated by the parent layout
//    Resize(100, mParentMenu ? mParentMenu->GetItemHeight() : 24);
//    // Enable mouse events
//    mConsumeMouseEvents = true;
//}
//
//void AMenuItemWidget::UpdateFromItem(const AMenuItem& item) {
//    mItemData = item;
//    // Update visibility and enabled state
//    mVisible = item.isVisible;
//    mEnabled = item.isEnabled;
//    // If separator, maybe we don't want to draw text but we still handle it
//}
//
//void AMenuItemWidget::OnDraw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH,
//                             int32_t offsetX, int32_t offsetY,
//                             int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const {
//    if (!mVisible || mItemData.isSeparator) {
//        // For separators, draw a line (parent will handle it? Actually we can draw a line here)
//        // But for simplicity we let parent AMenu draw separators as background lines.
//        // We'll just skip drawing separator items; parent AMenu will draw them.
//        return;
//    }
//
//    // Calculate absolute position
//    int32_t absX = offsetX + X();
//    int32_t absY = offsetY + Y();
//    int32_t w = static_cast<int32_t>(SizeX());
//    int32_t h = static_cast<int32_t>(SizeY());
//
//    // Background (hover or normal)
//    uint32_t bg = (mHovered || mPressed) ? mParentMenu->mHoverBgColor : mParentMenu->BGColor();
//    FillRect(buffer, bufferW, absX, absY, w, h, bg);
//
//    // Text
//    uint32_t textColor = mItemData.isEnabled ? mParentMenu->TextColor() : mParentMenu->mDisabledColor;
//    int32_t padding = mParentMenu->Padding();
//    int32_t arrowWidth = mItemData.subItems.empty() ? 0 : mParentMenu->mSubmenuArrowWidth;
//    int32_t textW = w - 2 * padding - arrowWidth;
//    int32_t textH = h;
//
//    // Draw text (use DrawTextEx from AUILib)
//    FT_Face face = mParentMenu->EnginePtr()->GetDefaultFontFace();
//    if (face) {
//        DrawTextEx(buffer, bufferW, bufferH,
//                   absX + padding, absY,
//                   textW, textH,
//                   mItemData.text.c_str(),
//                   face, FontSize(),
//                   AUIHAlign::left, AUIVAlign::center,
//                   0, textColor, textW);
//    }
//
//    // Check mark
//    if (mItemData.isCheckable && mItemData.isChecked) {
//        const char* check = "✓";
//        DrawTextEx(buffer, bufferW, bufferH,
//                   absX + 2, absY,
//                   16, h,
//                   check, face, FontSize(),
//                   AUIHAlign::left, AUIVAlign::center,
//                   0, mParentMenu->mCheckMarkColor, 16);
//    }
//
//    // Submenu arrow
//    if (!mItemData.subItems.empty()) {
//        const char* arrow = (mParentMenu->mOrientation == AUIOrientation::vertical) ? "▶" : "▼";
//        DrawTextEx(buffer, bufferW, bufferH,
//                   absX + w - arrowWidth + 2, absY,
//                   arrowWidth, h,
//                   arrow, face, FontSize(),
//                   AUIHAlign::left, AUIVAlign::center,
//                   0, textColor, arrowWidth);
//    }
//}
//
//AWidget* AMenuItemWidget::OnMouseDownLeft(int32_t localX, int32_t localY) {
//    if (!mEnabled || !mVisible || mItemData.isSeparator)
//        return nullptr;
//
//    mPressed = true;
//    // Notify parent menu of click
//    if (mParentMenu) {
//        mParentMenu->OnItemClicked(mIndex, localX, localY);
//    }
//    return this; // consume event
//}
//
//bool AMenuItemWidget::OnMouseMove(int32_t localX, int32_t localY) {
//    if (!mEnabled || !mVisible || mItemData.isSeparator)
//        return false;
//
//    bool inside = (localX >= 0 && localX < static_cast<int32_t>(SizeX()) &&
//                   localY >= 0 && localY < static_cast<int32_t>(SizeY()));
//    if (inside != mHovered) {
//        mHovered = inside;
//        if (mParentMenu) {
//            mParentMenu->OnItemHovered(mIndex);
//        }
//        // Request redraw to update hover state
//        if (Wnd()) Wnd()->RequestRedraw();
//    }
//    return true; // we handled the move (or we can return false to let parent also handle)
//}
//
// -----------------------------------------------------------------------------
// AMenu implementation
// -----------------------------------------------------------------------------
AMenu::AMenu() {
    mType = AUIWidgetType::defaultMenu;
    mBGColor = 0xFFEEEEEE;
    mBorderThick = 1;
    mBorderColor = 0xFF888888;
    mTextColor = 0xFF000000;
    mFontSize = 14;
    Focusable(false);
    mDefaultFillBG = true;
    mDefaultDrawBorder = true;
    mCapSizeToParent = false;
    ClipChildren(true);
    ClipChildrenHitbox(true);
    LayoutDirty();
}

AMenu::AMenu(const std::vector<AMenuItem>& items, AUIOrientation orient)
    : AMenu() {
    mItems = items;
    mOrientation = orient;
    LayoutDirty();
}

//AMenu::AMenu(const std::vector<AMenuItem>& items,
//             int32_t x, int32_t y, uint32_t w, uint32_t h,
//             AUIOrientation orient)
//    : AMenu(items, orient) {
//    Move(x, y);
//    Resize(w, h);
//}

AMenu::~AMenu() {
    CloseSubMenu();
    // If this menu is permanent or active, clear the window's pointers
    if (Wnd()) {
        if (Wnd()->ActiveMenu() == this)
            Wnd()->ActiveMenu(nullptr);
        if (Wnd()->PermanentMenu() == this)
            Wnd()->PermanentMenu(nullptr);
    }
}
//
//// -------------------------------------------------------------------------
//// Content management
//// -------------------------------------------------------------------------
//void AMenu::Items(const std::vector<AMenuItem>& items) {
//    mItems = items;
//    mLayoutDirty = true;
//    if (mWnd) mWnd->RequestRedraw();
//}
//
//void AMenu::AddItem(const AMenuItem& item) {
//    mItems.push_back(item);
//    mLayoutDirty = true;
//    if (mWnd) mWnd->RequestRedraw();
//}
//
//void AMenu::InsertItem(size_t index, const AMenuItem& item) {
//    if (index > mItems.size()) index = mItems.size();
//    mItems.insert(mItems.begin() + index, item);
//    mLayoutDirty = true;
//    if (mWnd) mWnd->RequestRedraw();
//}
//
//void AMenu::RemoveItem(size_t index) {
//    if (index >= mItems.size()) return;
//    mItems.erase(mItems.begin() + index);
//    mLayoutDirty = true;
//    if (mWnd) mWnd->RequestRedraw();
//}
//
//void AMenu::ClearItems() {
//    mItems.clear();
//    mLayoutDirty = true;
//    if (mWnd) mWnd->RequestRedraw();
//}
//
//const AMenuItem& AMenu::Item(size_t index) const {
//    static AMenuItem dummy;
//    if (index >= mItems.size()) return dummy;
//    return mItems[index];
//}
//
//// -------------------------------------------------------------------------
//// Appearance
//// -------------------------------------------------------------------------
//void AMenu::Orientation(AUIOrientation orient) {
//    if (mOrientation == orient) return;
//    mOrientation = orient;
//    mLayoutDirty = true;
//    if (mWnd) mWnd->RequestRedraw();
//}
//
//void AMenu::ItemHeight(int32_t height) {
//    if (mItemHeight == height) return;
//    mItemHeight = height;
//    mLayoutDirty = true;
//    if (mWnd) mWnd->RequestRedraw();
//}
//
//void AMenu::SetColors(uint32_t bg, uint32_t hoverBg, uint32_t text,
//                      uint32_t disabled, uint32_t separator) {
//    mBGColor = bg;
//    mHoverBgColor = hoverBg;
//    mTextColor = text;
//    mDisabledColor = disabled;
//    mSeparatorColor = separator;
//    if (mWnd) mWnd->RequestRedraw();
//}
//
//// -------------------------------------------------------------------------
//// Popup / Dismiss
//// -------------------------------------------------------------------------
//void AMenu::Popup(int32_t x, int32_t y) {
//    if (mVisible) return;
//    RecalcLayout();
//    int32_t w = mCachedWidth;
//    int32_t h = mCachedHeight;
//    if (mWnd) {
//        int32_t winW = static_cast<int32_t>(mWnd->SizeX());
//        int32_t winH = static_cast<int32_t>(mWnd->SizeY());
//        if (x + w > winW) x = winW - w;
//        if (y + h > winH) y = winH - h;
//        if (x < 0) x = 0;
//        if (y < 0) y = 0;
//    }
//    Move(x, y);
//    Resize(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
//    mVisible = true;
//    if (!mParentMenu && !mIsPermanent) {
//        if (mWnd) mWnd->ActiveMenu(this);   // set as active popup
//    }
//    if (mIsPermanent) {
//        if (mWnd) mWnd->PermanentMenu(this);
//    }
//    mHoveredIndex = -1;
//    CloseSubMenu();
//    UpdateItemWidgets();
//    if (mWnd) mWnd->RequestRedraw();
//}
//
//void AMenu::Dismiss() {
//    if (!mVisible) return;
//    CloseSubMenu();
//    if (mIsPermanent) {
//        // Permanent menus never disappear
//        if (mWnd) mWnd->RequestRedraw();
//        return;
//    }
//    if (mParentMenu) {
//        mParentMenu->mActiveSubMenu = nullptr;
//        // Remove this submenu from the window
//        if (mWnd) {
//            mWnd->RemoveWidget(this);
//        }
//        return;
//    }
//    // Top-level popup
//    mVisible = false;
//    if (mWnd) {
//        if (mWnd->ActiveMenu() == this)
//            mWnd->ActiveMenu(nullptr);
//        mWnd->RequestRedraw();
//    }
//}
//
//void AMenu::SetPermanent(bool permanent) {
//    if (mIsPermanent == permanent) return;
//    mIsPermanent = permanent;
//    if (permanent) {
//        if (mWnd) mWnd->PermanentMenu(this);
//    } else {
//        if (mWnd && mWnd->PermanentMenu() == this)
//            mWnd->PermanentMenu(nullptr);
//    }
//}
//
//// -------------------------------------------------------------------------
//// Layout
//// -------------------------------------------------------------------------
//int32_t AMenu::ComputeTextWidth(const std::string& text) const {
//    AUI* engine = mWnd ? mWnd->EnginePtr() : nullptr;
//    if (!engine) return 0;
//    FT_Face face = engine->GetDefaultFontFace();
//    if (!face) return 0;
//    FT_Set_Pixel_Sizes(face, 0, mFontSize);
//    int32_t width = 0;
//    for (char c : text) {
//        if (FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_COLOR) == 0) {
//            width += static_cast<int32_t>(face->glyph->advance.x >> 6);
//        }
//    }
//    return width;
//}
//
//int32_t AMenu::MaxTextWidth() const {
//    int32_t maxW = 0;
//    for (const auto& item : mItems) {
//        if (item.isSeparator || !item.isVisible) continue;
//        int32_t w = ComputeTextWidth(item.text);
//        if (w > maxW) maxW = w;
//    }
//    return maxW;
//}
//
//void AMenu::RecalcLayout() {
//    if (!mLayoutDirty) return;
//    size_t n = mItems.size();
//    mItemX.resize(n);
//    mItemY.resize(n);
//    mItemW.resize(n);
//    mItemH.resize(n);
//
//    int32_t totalW = 0, totalH = 0;
//    int32_t maxTextW = MaxTextWidth();
//    int32_t itemW = maxTextW + 2 * mPadding + mSubmenuArrowWidth;
//    if (mMinWidth > 0 && itemW < mMinWidth) itemW = mMinWidth;
//
//    if (mOrientation == AUIOrientation::vertical) {
//        int32_t y = 0;
//        for (size_t i = 0; i < n; ++i) {
//            const auto& it = mItems[i];
//            int32_t h = it.isSeparator ? mSeparatorSizeY : mItemHeight;
//            mItemX[i] = 0;
//            mItemY[i] = y;
//            mItemW[i] = itemW;
//            mItemH[i] = h;
//            y += h;
//        }
//        totalW = itemW;
//        totalH = y;
//    } else {
//        int32_t x = 0;
//        for (size_t i = 0; i < n; ++i) {
//            const auto& it = mItems[i];
//            int32_t w;
//            if (it.isSeparator) {
//                w = 2;
//            } else {
//                int32_t tw = ComputeTextWidth(it.text);
//                w = tw + 2 * mPadding + mSubmenuArrowWidth;
//                if (w < 20) w = 20;
//            }
//            mItemX[i] = x;
//            mItemY[i] = 0;
//            mItemW[i] = w;
//            mItemH[i] = mItemHeight;
//            x += w;
//        }
//        totalW = x;
//        totalH = mItemHeight;
//    }
//
//    mCachedWidth = totalW;
//    mCachedHeight = totalH;
//    mLayoutDirty = false;
//}
//
//void AMenu::UpdateItemWidgets() {
//    // Remove old children
//    for (auto& child : mItemWidgets) {
//        if (mWnd) mWnd->RemoveWidget(child.get());
//    }
//    mItemWidgets.clear();
//
//    // Create new ones
//    for (size_t i = 0; i < mItems.size(); ++i) {
//        auto widget = std::make_unique<AMenuItemWidget>(this, i);
//        widget->UpdateFromItem(mItems[i]);
//        // Set position and size from layout cache
//        widget->Move(mItemX[i], mItemY[i]);
//        widget->Resize(static_cast<uint32_t>(mItemW[i]), static_cast<uint32_t>(mItemH[i]));
//        // Add as child
//        AddWidget(std::move(widget));
//    }
//    // The vector mItemWidgets can hold raw pointers for later updates
//    // We'll just keep them in mWidg (base class) and we can iterate.
//}
//
//void AMenu::LayoutUpdate() {
//    RecalcLayout();
//    UpdateItemWidgets();
//    // Also update submenu if active
//    if (mActiveSubMenu) {
//        // Submenu layout may be dirty; it will handle itself.
//        mActiveSubMenu->LayoutUpdate();
//    }
//}
//
//// -------------------------------------------------------------------------
//// Drawing
//// -------------------------------------------------------------------------
//void AMenu::OnDraw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH,
//                   int32_t offsetX, int32_t offsetY,
//                   int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const {
//    if (!mVisible) return;
//    // Draw background and border (handled by AWidget's Draw)
//    // But we can call the base OnDrawBG? Actually we override OnDraw, so we need to draw everything.
//    // However, AWidget::Draw will call OnDrawBG and OnDraw. So we just need to draw children.
//    // The background is drawn by OnDrawBG (which we might override if we want custom).
//    // Since we have mDefaultFillBG = true, the base will call OnDrawBG, which will fill with mBGColor.
//    // So we don't need to draw background here.
//    // But we need to draw child widgets, which are already drawn by DrawChildren.
//    // However, we also need to draw separators (lines) that are not part of child widgets?
//    // We can let child widgets draw themselves, including separators.
//    // For separators, we can have a special child widget that draws a line.
//    // We'll handle that in AMenuItemWidget's OnDraw: if separator, draw a line.
//}
//
//// -------------------------------------------------------------------------
//// Events
//// -------------------------------------------------------------------------
//void AMenu::OnItemClicked(size_t index, int32_t localX, int32_t localY) {
//    if (index >= mItems.size()) return;
//    const auto& item = mItems[index];
//    if (!item.isEnabled || !item.isVisible || item.isSeparator) return;
//
//    if (!item.subItems.empty()) {
//        // Toggle submenu
//        if (mActiveSubMenu && mActiveSubMenuOwnerIndex == static_cast<int32_t>(index)) {
//            CloseSubMenu();
//        } else {
//            CloseSubMenu();
//            OpenSubMenu(index);
//        }
//        return;
//    }
//
//    // Normal item action
//    bool isCheckable = item.isCheckable;
//    if (isCheckable) {
//        const_cast<AMenuItem&>(item).isChecked = !item.isChecked;
//    }
//    auto actionCopy = item.action;
//    Dismiss(); // closes all submenus
//    if (actionCopy) actionCopy();
//}
//
//void AMenu::OnItemHovered(size_t index) {
//    if (mHoveredIndex == static_cast<int32_t>(index)) return;
//    mHoveredIndex = static_cast<int32_t>(index);
//    mLastHoverTime = std::chrono::steady_clock::now();
//    if (mWnd) mWnd->RequestRedraw();
//
//    // If the item has subitems, schedule opening after delay
//    if (mHoveredIndex >= 0 && mHoveredIndex < static_cast<int32_t>(mItems.size())) {
//        const auto& item = mItems[mHoveredIndex];
//        if (!item.subItems.empty() && item.isEnabled) {
//            // Use a timer or check in OnMouseMove; we'll handle in OnMouseMove with delay
//        }
//    }
//}
//
//AWidget* AMenu::OnMouseDownLeft(int32_t localX, int32_t localY) {
//    // First, let children process (base class does that)
//    AWidget* consumed = AWidget::OnMouseDownLeft(localX, localY);
//    if (consumed) return consumed;
//
//    // Click on empty area of the menu? Dismiss if not permanent and not parent menu?
//    // We can dismiss if the click is on the menu's background (no child handled it)
//    // But we must not dismiss if it's a permanent menu or submenu?
//    // For submenu, we want to dismiss only if clicked outside the entire menu hierarchy.
//    // Since this menu is modal, the window will handle outside clicks.
//    // So we can just return nullptr to let parent handle.
//    return nullptr;
//}
//
//bool AMenu::OnMouseMove(int32_t localX, int32_t localY) {
//    // The base class already forwards to children. We don't need to do much.
//    // But we need to handle submenu opening delay.
//    // We can check if hovered index changed and if delay passed.
//    // Since OnMouseMove is called for the menu itself after children, we can check
//    // if we have a hovered index and no active submenu yet.
//    if (mHoveredIndex >= 0 && mHoveredIndex < static_cast<int32_t>(mItems.size())) {
//        const auto& item = mItems[mHoveredIndex];
//        if (!item.subItems.empty() && item.isEnabled && !mActiveSubMenu) {
//            auto now = std::chrono::steady_clock::now();
//            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - mLastHoverTime).count();
//            if (elapsed >= mSubmenuDelayMs) {
//                OpenSubMenu(static_cast<size_t>(mHoveredIndex));
//            }
//        }
//    }
//    return true; // handled
//}
//
//void AMenu::OnKeyEvent(const AUIKeyEvent& event) {
//    if (!mVisible || !event.pressed) return;
//    size_t n = mItems.size();
//    if (n == 0) return;
//
//    switch (event.code) {
//        case AUIKeyCode::Down:
//            if (mOrientation == AUIOrientation::vertical) {
//                int32_t next = (mHoveredIndex + 1) % n;
//                while (next != mHoveredIndex &&
//                       (!mItems[next].isEnabled || !mItems[next].isVisible || mItems[next].isSeparator)) {
//                    next = (next + 1) % n;
//                }
//                mHoveredIndex = next;
//                if (mWnd) mWnd->RequestRedraw();
//            }
//            break;
//        case AUIKeyCode::Up:
//            if (mOrientation == AUIOrientation::vertical) {
//                int32_t prev = (mHoveredIndex - 1 + n) % n;
//                while (prev != mHoveredIndex &&
//                       (!mItems[prev].isEnabled || !mItems[prev].isVisible || mItems[prev].isSeparator)) {
//                    prev = (prev - 1 + n) % n;
//                }
//                mHoveredIndex = prev;
//                if (mWnd) mWnd->RequestRedraw();
//            }
//            break;
//        case AUIKeyCode::Right:
//            if (mOrientation == AUIOrientation::horizontal) {
//                int32_t next = (mHoveredIndex + 1) % n;
//                while (next != mHoveredIndex &&
//                       (!mItems[next].isEnabled || !mItems[next].isVisible || mItems[next].isSeparator)) {
//                    next = (next + 1) % n;
//                }
//                mHoveredIndex = next;
//                if (mWnd) mWnd->RequestRedraw();
//            } else {
//                if (mHoveredIndex >= 0 && !mItems[mHoveredIndex].subItems.empty()) {
//                    OpenSubMenu(mHoveredIndex);
//                }
//            }
//            break;
//        case AUIKeyCode::Left:
//            if (mOrientation == AUIOrientation::horizontal) {
//                int32_t prev = (mHoveredIndex - 1 + n) % n;
//                while (prev != mHoveredIndex &&
//                       (!mItems[prev].isEnabled || !mItems[prev].isVisible || mItems[prev].isSeparator)) {
//                    prev = (prev - 1 + n) % n;
//                }
//                mHoveredIndex = prev;
//                if (mWnd) mWnd->RequestRedraw();
//            } else {
//                if (mParentMenu) Dismiss();
//            }
//            break;
//        case AUIKeyCode::Enter:
//        case AUIKeyCode::Space:
//            if (mHoveredIndex >= 0) {
//                int32_t cx = mItemX[mHoveredIndex] + mItemW[mHoveredIndex] / 2;
//                int32_t cy = mItemY[mHoveredIndex] + mItemH[mHoveredIndex] / 2;
//                OnItemClicked(mHoveredIndex, cx, cy);
//            }
//            break;
//        case AUIKeyCode::Escape:
//            Dismiss();
//            break;
//        default:
//            break;
//    }
//}
//
//void AMenu::OnResize(uint32_t newWidth, uint32_t newHeight) {
//    AWidget::OnResize(newWidth, newHeight);
//    mLayoutDirty = true;
//    LayoutUpdate();
//}
//
//void AMenu::FontSize(uint32_t size) {
//    AWidget::FontSize(size);
//    // Propagate to child widgets
//    for (auto& child : mWidg) {
//        child->FontSize(size);
//    }
//    mLayoutDirty = true;
//    if (mWnd) mWnd->RequestRedraw();
//}
//
//// -------------------------------------------------------------------------
//// Submenu management
//// -------------------------------------------------------------------------
//void AMenu::OpenSubMenu(size_t index) {
//    if (index >= mItems.size()) return;
//    const auto& item = mItems[index];
//    if (item.subItems.empty()) return;
//
//    CloseSubMenu();
//
//    AMenu* sub = new AMenu(item.subItems, AUIOrientation::vertical);
//    sub->mParentMenu = this;
//    sub->mWnd = mWnd;
//    // Copy appearance
//    sub->SetColors(mBGColor, mHoverBgColor, mTextColor, mDisabledColor, mSeparatorColor);
//    sub->ItemHeight(mItemHeight);
//    sub->Padding(mPadding);
//    sub->MinWidth(mMinWidth);
//    sub->FontSize(mFontSize);
//
//    // Attach to the same window
//    if (mWnd) {
//        mWnd->AddWidget(std::unique_ptr<AWidget>(sub));
//    } else {
//        delete sub;
//        return;
//    }
//
//    // Calculate position
//    int32_t spawnX = mX + mItemX[index];
//    int32_t spawnY = mY + mItemY[index];
//    if (mOrientation == AUIOrientation::vertical) {
//        spawnX += mItemW[index] - 2;
//    } else {
//        spawnY += mItemH[index];
//    }
//    sub->Popup(spawnX, spawnY);
//    mActiveSubMenu = sub;
//    mActiveSubMenuOwnerIndex = static_cast<int32_t>(index);
//    if (mWnd) mWnd->RequestRedraw();
//}
//
//void AMenu::CloseSubMenu() {
//    if (mActiveSubMenu) {
//        mActiveSubMenu->Dismiss();
//        mActiveSubMenu = nullptr;
//        mActiveSubMenuOwnerIndex = -1;
//    }
//}
//
//void AMenu::DetachFromParent() {
//    if (mWnd) {
//        mWnd->RemoveWidget(this);
//    }
//}
//
//bool AMenu::IsPointInsideHierarchy(int32_t x, int32_t y) const {
//    if (x >= mX && x < mX + static_cast<int32_t>(mSizeX) &&
//        y >= mY && y < mY + static_cast<int32_t>(mSizeY)) {
//        return true;
//    }
//    if (mActiveSubMenu && mActiveSubMenu->IsVisible()) {
//        return mActiveSubMenu->IsPointInsideHierarchy(x, y);
//    }
//    return false;
//}

} // namespace aui
