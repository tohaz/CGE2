#ifndef AMENU_H_
#define AMENU_H_

#include "AWidget.h"
#include <functional>
#include <string>
#include <vector>
#include <chrono>

namespace aui {

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
      void *userData = nullptr;
      AMenuItem() = default;
      AMenuItem(const std::string &t, std::function<void()> a = nullptr) :
          text(t), action(std::move(a)) {
      }
      AMenuItem(const std::string &t, std::vector<AMenuItem> subs) :
          text(t), subItems(std::move(subs)) {
      }
  };

  class AMenu: public AWidget {
      friend class AWindow;
    private:
      static AMenu *sActiveTopMenu;
      static AMenu *sPermanentMenu;

      std::vector<AMenuItem> mItems;
      AUIOrientation mOrientation = AUIOrientation::vertical;

// Layout
      int32_t mItemHeight = 24;
      int32_t mMinWidth = 120;
      int32_t mPadding = 6;
      int32_t mSeparatorSizeY = 4;
      int32_t mSubmenuArrowWidth = 16;
// Colors
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
      AMenu *mActiveSubMenu = nullptr;
      AMenu *mParentMenu = nullptr;
// Cached layout
      mutable bool mLayoutDirty = true;
      mutable int32_t mCachedWidth = 0;
      mutable int32_t mCachedHeight = 0;
      mutable std::vector<int32_t> mItemX;
      mutable std::vector<int32_t> mItemY;
      mutable std::vector<int32_t> mItemW;
      mutable std::vector<int32_t> mItemH;

    protected:
// Internal helpers
      void RecalcLayout() const;
      int32_t HitTest(int32_t localX, int32_t localY) const;
      void DrawItem(uint32_t *buffer, int32_t idx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t parentWidth,
          uint32_t parentHeight) const;
      void OpenSubMenu(size_t index);
      void DetachFromParent();
      bool IsPointInsideHierarchy(int32_t x, int32_t y) const;
      int32_t ComputeTextWidth(const std::string &text) const;
      int32_t GetMaxTextWidth() const;

    public:
      AMenu();
      explicit AMenu(const std::vector<AMenuItem> &items, AUIOrientation orient = AUIOrientation::vertical);
      ~AMenu() override;
      static AMenu* AttachTo(AWindow *parent, const std::vector<AMenuItem> &items = { });
      static AMenu* AttachTo(AWidget *parent, const std::vector<AMenuItem> &items = { });
// ------------------------------------------------------------------
// Content management
// ------------------------------------------------------------------
      void SetItems(const std::vector<AMenuItem> &items);
      void AddItem(const AMenuItem &item);
      void InsertItem(size_t index, const AMenuItem &item);
      void RemoveItem(size_t index);
      void ClearItems();
      size_t GetItemCount() const {
        return mItems.size();
      }
// ------------------------------------------------------------------
// Appearance and layout
// ------------------------------------------------------------------
      void SetOrientation(AUIOrientation orient);
      AUIOrientation GetOrientation() const {
        return mOrientation;
      }
      void SetItemHeight(int32_t height);
      int32_t GetItemHeight() const {
        return mItemHeight;
      }
      void SetMinWidth(int32_t w) {
        mMinWidth = w;
      }
      int32_t GetMinWidth() const {
        return mMinWidth;
      }
      void SetPadding(int32_t pad) {
        mPadding = pad;
      }
      int32_t GetPadding() const {
        return mPadding;
      }
      void SetSubmenuDelayMs(int32_t ms) {
        mSubmenuDelayMs = ms;
      }
      int32_t GetSubmenuDelayMs() const {
        return mSubmenuDelayMs;
      }
      void SetColors(uint32_t bg, uint32_t hoverBg, uint32_t text, uint32_t disabled, uint32_t separator);
// ------------------------------------------------------------------
// Popup / Dismiss / Visibility
// ------------------------------------------------------------------
      void Popup(int32_t x, int32_t y);
      void Dismiss();
      bool IsVisible() const {
        return mVisible;
      }
// ------------------------------------------------------------------
// Permanent menu bar support
// ------------------------------------------------------------------
      void SetPermanent(bool permanent);
      bool IsPermanent() const {
        return mIsPermanent;
      }
      static AMenu* GetPermanentMenu() {
        return sPermanentMenu;
      }
// ------------------------------------------------------------------
// Submenu hierarchy
// ------------------------------------------------------------------
      void SetParentMenu(AMenu *parent) {
        mParentMenu = parent;
      }
      AMenu* GetParentMenu() const {
        return mParentMenu;
      }
      AMenu* GetActiveSubMenu() const {
        return mActiveSubMenu;
      }
// ------------------------------------------------------------------
// Static active menu management (for click‑outside‑to‑dismiss)
// ------------------------------------------------------------------
      static void DismissActiveMenu();
      static bool IsActiveMenuVisible();
      static bool IsPointInsideActiveMenu(int32_t x, int32_t y);
      static bool IsPointInsideMenuHierarchy(const AMenu *menu, int32_t x, int32_t y);
// ------------------------------------------------------------------
// Event overrides
// ------------------------------------------------------------------
      void Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX, int32_t offsetY) const
          override;
      bool OnMouseClick(int32_t localX, int32_t localY, bool pressed) override;
      void OnMouseMove(int32_t localX, int32_t localY) override;
      void OnMouseLeave() override;
      void OnKeyEvent(const AUIKeyEvent &event) override;
// ------------------------------------------------------------------
// Accessors
// ------------------------------------------------------------------
      int32_t GetHoveredIndex() const {
        return mHoveredIndex;
      }
      const AMenuItem& GetItem(size_t index) const;
      void CloseSubMenu();

  };

}// namespace aui

#endif // AMENU_H_
