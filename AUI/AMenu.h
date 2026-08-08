#ifndef AMENU_H_
#define AMENU_H_

namespace aui {

struct AMenuItem {
    std::string mText;
    std::function<void()> action;
    std::vector<AMenuItem> subItems;
    bool mSeparator = false;
    bool mEnabled = true;
    bool mVisible = true;
    void (*actionWithData)(AMenu* menu, void* userData) = nullptr;
    void* userData = nullptr;
    AMenuItem() = default;
    AMenuItem(const std::string& t, std::function<void()> a = nullptr)
        : mText(t), action(std::move(a)) {}
    AMenuItem(const std::string& t, std::vector<AMenuItem> subs)
        : mText(t), subItems(std::move(subs)) {}
    static AMenuItem Separator() { AMenuItem it; it.mSeparator = true; return it; }
    template <typename Callable,
              typename = std::enable_if_t<!std::is_same_v<std::decay_t<Callable>, std::vector<AMenuItem>>>>
    AMenuItem(const std::string& t, Callable&& a)
        : mText(t), action(std::forward<Callable>(a)) {}
};

class AMenu : public AWidgetFactory<AMenu> {
    friend class AWidgetFactory<AMenu>;
private:
    AMenu();
    void LayoutDirty() { mLayoutDirty = true; }
    void RecalcLayout() const;
    int32_t HitTest(int32_t x, int32_t y) const;
    void OpenSubMenu(size_t index);
    void CloseSubMenu();
    std::vector<AMenuItem> mItems;
    AUIOrientation mOrientation = AUIOrientation::vertical;
    int32_t mItemHeight = 24;
    int32_t mPadding = 6;
    int32_t mSeparatorSize = 4;
    uint32_t mHoverBg = 0xFFCCCCCC;
    uint32_t mDisabledColor = 0xFF888888;
    bool mVisible = false;
    bool mIsPermanent = false;
    int32_t mHoveredIndex = -1;
    // Submenu state
    AMenu* mActiveSubMenu = nullptr;          // child submenu (owned by this)
    int32_t mActiveSubMenuOwnerIndex = -1;
    AMenu* mParentMenu = nullptr;             // set if this is a submenu
    int32_t mSubmenuDelayMs = 200;
    std::chrono::steady_clock::time_point mLastHoverTime;
    mutable bool mLayoutDirty = true;
    mutable int32_t mCachedWidth = 0, mCachedHeight = 0;
    mutable std::vector<int32_t> mItemX, mItemY, mItemW, mItemH;
public:
    explicit AMenu(std::vector<AMenuItem>&& items,
                   AUIOrientation orient = AUIOrientation::vertical);
    ~AMenu() override;
    void SetItems(std::vector<AMenuItem>&& items);
    void AddItem(AMenuItem item);
    void ClearItems();
    void Orientation(AUIOrientation o);
    void ItemHeight(int32_t h);
    void Padding(int32_t p) { mPadding = p; LayoutDirty(); }
    void SetColors(uint32_t bg, uint32_t hoverBg, uint32_t text, uint32_t disabled);
    void Show();
    void Popup(int32_t x, int32_t y);
    void Dismiss();
    bool IsVisible() const { return mVisible; }
    void SetPermanent(bool p) { mIsPermanent = p; }
    void OnDraw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH,
                int32_t offsetX, int32_t offsetY,
                int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const override;
    AWidget* OnMouseDownLeft(int32_t localX, int32_t localY) override;
    bool OnMouseMove(int32_t localX, int32_t localY) override;
    void OnResize(uint32_t w, uint32_t h) override;
    void OnSubmenuDismissed(AMenu* submenu);
    int32_t HoveredIndex() const { return mHoveredIndex; }
    bool SubMenuOpen() const { return mActiveSubMenu != nullptr; }

};

} // namespace aui

#endif
