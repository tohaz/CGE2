#ifndef AMENU_H_
#define AMENU_H_

namespace aui {
  class AMenuItem {
      std::string text;
      std::function<void()> action;
      std::vector<AMenuItem> subItems;// Recursive for submenus
      bool isSeparator = false;
      bool isCheckable = false;
      bool isChecked = false;
      bool isEnabled = true;
//? Picture icon = None;
  };
  class AMenu: public AWidget {
    private:
      AMenu(AWidget *wParent, const std::vector<AMenuItem> &inItems);
      std::vector<AMenuItem> mItems;
      AMenu *mActiveSubMenu = nullptr;
      AMenu *mParentMenu = nullptr;
      int32_t mHoveredIndex = -1;
      void CalculateLayout(int32_t &outW, int32_t &outH);
      void CloseSubMenu();
      void DrawItem(int32_t index, int32_t x, int32_t y, int32_t w, int32_t h);
      int32_t mSeparatorPadding = 10;// Horizontal inset for the separator line
      int32_t mDisabledColor = 0x888888;
      int32_t mSeparatorShadow = 0x666666;
      int32_t mSeparatorSizeY = 3;// Total vertical space for separator
      int32_t mHoverIntensity = 15;// How much to lighten/darken the background on hover
    protected:
    public:
      static AMenu* AttachTo(AWidget *wParent);
      static AMenu* AttachTo(AWindow *wParent);
      static AMenu* AttachTo(AWidget *wParent, const std::vector<AMenuItem> &inItems);
      virtual ~AMenu();
      virtual void Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
          int32_t offsetY) const override;
      void OnMouseMove(int32_t localX, int32_t localY);
      void OnMouseEnter();
      void OnMouseLeave();
      void Popup(int32_t X, int32_t Y);
      void Dismiss();
      void SetItems(const std::vector<AMenuItem> &newItems);
      void SetParentMenu(AMenu *parent) {
        mParentMenu = parent;
      }
      int32_t HoveredIndex() const {
        return mHoveredIndex;
      }
      int32_t ItemSizeY() const {
        return AWidget::mItemSizeY;
      }
      int32_t SeparatorSizeY() const {
        return mSeparatorSizeY;
      }
  };
}

#endif // AMENU_H_
