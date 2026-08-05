#ifndef AMENU_H_
#define AMENU_H_

namespace aui {

// Forward declaration
class AMenuItemWidget;

// -----------------------------------------------------------------------------
// Menu item data
// -----------------------------------------------------------------------------
struct AMenuItem {
    std::string text;
    std::function<void()> action;
    std::vector<AMenuItem> subItems;
    std::string icon;
    std::string shortcut;
    char accelerator = 0;
    bool isSeparator = false;
    bool isCheckable = false;
    bool isChecked = false;
    bool isEnabled = true;
    bool isVisible = true;
    void* userData = nullptr;

    AMenuItem() = default;
    AMenuItem(const std::string& t, std::function<void()> a = nullptr)
        : text(t), action(std::move(a)) {}
    AMenuItem(const std::string& t, std::vector<AMenuItem> subs)
        : text(t), subItems(std::move(subs)) {}
};

// -----------------------------------------------------------------------------
// AMenu – a container that manages a list of menu items as child widgets.
// Derived from AWidgetFactory so it can be attached via the static AttachTo.
// -----------------------------------------------------------------------------
class AMenu : public AWidgetFactory<AMenu> {
    friend class AWidgetFactory<AMenu>;
    friend class AMenuItemWidget;

public:
    // Constructors – used by the factory
    AMenu();
    explicit AMenu(const std::vector<AMenuItem>& items,
                   AUIOrientation orient = AUIOrientation::vertical);
    AMenu(const std::vector<AMenuItem>& items,
          int32_t x, int32_t y, uint32_t w, uint32_t h,
          AUIOrientation orient = AUIOrientation::vertical);

    ~AMenu() override;

    // -------------------------------------------------------------------------
    // Content management
    // -------------------------------------------------------------------------
    void Items(const std::vector<AMenuItem>& items);
    void AddItem(const AMenuItem& item);
    void InsertItem(size_t index, const AMenuItem& item);
    void RemoveItem(size_t index);
    void ClearItems();
    size_t ItemCount() const { return mItems.size(); }
    const AMenuItem& Item(size_t index) const;

    // -------------------------------------------------------------------------
    // Appearance & layout
    // -------------------------------------------------------------------------
    void Orientation(AUIOrientation orient);
    AUIOrientation GetOrientation() const { return mOrientation; }

    void ItemHeight(int32_t height);
    int32_t GetItemHeight() const { return mItemHeight; }

    void MinWidth(int32_t w) { mMinWidth = w; }
    int32_t MinWidth() const { return mMinWidth; }

    void Padding(int32_t pad) { mPadding = pad; }
    int32_t Padding() const { return mPadding; }

    void SubmenuDelayMs(int32_t ms) { mSubmenuDelayMs = ms; }
    int32_t SubmenuDelayMs() const { return mSubmenuDelayMs; }

    void SetColors(uint32_t bg, uint32_t hoverBg, uint32_t text,
                   uint32_t disabled, uint32_t separator);

    // -------------------------------------------------------------------------
    // Popup / Dismiss / Visibility
    // -------------------------------------------------------------------------
    void Popup(int32_t x, int32_t y);
    void Dismiss();
    bool IsVisible() const { return mVisible; }

    // -------------------------------------------------------------------------
    // Permanent menu bar support
    // -------------------------------------------------------------------------
    void SetPermanent(bool permanent);
    bool IsPermanent() const { return mIsPermanent; }

    // -------------------------------------------------------------------------
    // Submenu hierarchy
    // -------------------------------------------------------------------------
    AMenu* ParentMenu() const { return mParentMenu; }
    AMenu* ActiveSubMenu() const { return mActiveSubMenu; }
    void CloseSubMenu();

    // -------------------------------------------------------------------------
    // Overrides from AWidget
    // -------------------------------------------------------------------------
    void FontSize(uint32_t size) override;
    void OnDraw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH,
                int32_t offsetX, int32_t offsetY,
                int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const override;
    void OnResize(uint32_t newWidth, uint32_t newHeight) override;

    AWidget* OnMouseDownLeft(int32_t localX, int32_t localY) override;
    bool OnMouseMove(int32_t localX, int32_t localY) override;
    void OnKeyEvent(const AUIKeyEvent& event) override;

    void LayoutUpdate() override;

private:
    // -------------------------------------------------------------------------
    // Internal helpers
    // -------------------------------------------------------------------------
    void RecalcLayout();
    void UpdateItemWidgets();               // create/update child widgets
    void OpenSubMenu(size_t index);
    void DetachFromParent();                // remove from window
    bool IsPointInsideHierarchy(int32_t x, int32_t y) const;
    int32_t ComputeTextWidth(const std::string& text) const;
    int32_t MaxTextWidth() const;

    // Called by AMenuItemWidget when an item is clicked or hovered
    void OnItemClicked(size_t index, int32_t localX, int32_t localY);
    void OnItemHovered(size_t index);

    // -------------------------------------------------------------------------
    // Member variables
    // -------------------------------------------------------------------------
    std::vector<AMenuItem> mItems;
    std::vector<std::unique_ptr<AMenuItemWidget>> mItemWidgets;  // child widgets

    AUIOrientation mOrientation = AUIOrientation::vertical;
    int32_t mItemHeight = 24;
    int32_t mMinWidth = 120;
    int32_t mPadding = 6;
    int32_t mSeparatorSizeY = 4;
    int32_t mSubmenuArrowWidth = 16;

    // Colours
    uint32_t mHoverBgColor = 0xFFCCCCCC;
    uint32_t mDisabledColor = 0xFF888888;
    uint32_t mSeparatorColor = 0xFF666666;
    uint32_t mCheckMarkColor = 0xFF000000;

    // State
    bool mVisible = false;
    bool mIsPermanent = false;
    int32_t mHoveredIndex = -1;
    int32_t mSubmenuDelayMs = 200;
    std::chrono::steady_clock::time_point mLastHoverTime;

    // Submenu pointers
    AMenu* mActiveSubMenu = nullptr;
    AMenu* mParentMenu = nullptr;
    int32_t mActiveSubMenuOwnerIndex = -1;

    // Layout cache
    mutable bool mLayoutDirty = true;
    mutable int32_t mCachedWidth = 0;
    mutable int32_t mCachedHeight = 0;
    mutable std::vector<int32_t> mItemX;
    mutable std::vector<int32_t> mItemY;
    mutable std::vector<int32_t> mItemW;
    mutable std::vector<int32_t> mItemH;
};

// -----------------------------------------------------------------------------
// Internal widget representing a single menu item (a child of AMenu)
// -----------------------------------------------------------------------------
class AMenuItemWidget : public AWidget {
public:
    AMenuItemWidget(AMenu* parentMenu, size_t index);
    void UpdateFromItem(const AMenuItem& item);

    // Overrides
    void OnDraw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH,
                int32_t offsetX, int32_t offsetY,
                int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const override;
    AWidget* OnMouseDownLeft(int32_t localX, int32_t localY) override;
    bool OnMouseMove(int32_t localX, int32_t localY) override;

private:
    AMenu* mParentMenu = nullptr;
    size_t mIndex = 0;
    bool mHovered = false;
    bool mPressed = false;
    AMenuItem mItemData; // copy of the item data for drawing
};

} // namespace aui

#endif // AMENU_H_
