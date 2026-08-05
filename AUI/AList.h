#ifndef ALIST_H_
#define ALIST_H_

namespace aui {

  class AScrollBar;

  using SelectionChangedCallback = std::function<void(AWidget*, void*)>;

  class AList: public AWidgetFactory<AList>{
      friend class AWidgetFactory<AList>;
    private:
// Data
      std::vector<std::string> mData;
      std::vector<bool> mTaggedItems;// selection flags
      uint32_t mLineSpacing = 2U;// pixels between lines
      uint32_t mLineHeight = 16U;// cached (font size + spacing)
      int32_t mVOffset = 0;// vertical scroll offset (pixels)
      int32_t mHOffset = 0;// horizontal scroll offset (pixels)
      uint32_t mMaxWidthPx = 0;// maximum text width in the list
      bool mMultiSelect = false;// multi‑select mode (no modifiers)
      bool mVScrollbarEnabled = false;
      bool mHScrollbarEnabled = false;
      bool mHLOnMouseMove = true;
// Scrollbars (managed manually, not registered in AUI)
      AScrollBar* mVScrollBar = nullptr;
      AScrollBar* mHScrollBar = nullptr;      AUIVAlign mLineTextVAlign = AUIVAlign::top;
      void RecalcLineHeight();// after font size or spacing change
      size_t IndexFromY(int32_t y) const;// convert local y to item index
      int32_t LineTop(size_t index) const;// y position (pixels) of line start
      void DrawScrollbars() const;
      bool mAutoHideHScrollbar = true;
      bool mAutoHideVScrollbar = true;
      SelectionChangedCallback mOnSelectionChanged;
      void* mSelectionUserData = nullptr;
      uint32_t mSelectionColor = 0xFF3399FF;
      uint32_t mSelectionTextColor = 0xFFFFFFFF;
      uint32_t mHoverColor = 0xFF40FFFF;
      int32_t  mHoveredIndex = -1;
      // ---- internal helpers ----
      void UpdateHoveredItem(int32_t mouseX, int32_t mouseY);
      void OnScroll(int32_t value, AScrollBar* sender);
      void UpdateScrollbarVisibility();
      AWidget* OnMouseDownLeft(int32_t localX, int32_t localY) override;
    protected:
      // ---- NEW overrides ----
      bool OnMouseMove(int32_t localX, int32_t localY) override;
      void OnMouseWheel(int32_t delta) override;
      void OnResize(uint32_t w, uint32_t h) override;
      void OnDraw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH,
                  int32_t offsetX, int32_t offsetY,
                  int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const override;
      void SyncScrollbarValues();
    public:
      AList();
      ~AList() override;
      void AddItem(const std::string &text);
      void InsertItem(size_t index, const std::string &text);
      void RemoveItem(size_t index);
      void Clear();
      size_t ItemCount() const {return mData.size();}
      const std::string& GetItem(size_t index) const;
      void Item(size_t index, const std::string &text);
      void MultiSelect(bool enable);
      bool IsMultiSelect() const {return mMultiSelect;}
      void SelectAll(bool selected);
      void SelectIndex(size_t index, bool selected);
      bool IsSelected(size_t index) const;
      void ClearSelection();
      void ScrollToOffset(int32_t xOffset, int32_t yOffset);
      void ScrollToItem(size_t index, bool alignCenter = false);
      int32_t VerticalOffset() const {return mVOffset;}
      int32_t HorizontalOffset() const {return mHOffset;}
      void LineSpacing(uint32_t spacing);
      void ScrollbarsEnabled(bool enable);
      bool ScrollbarsEnabled();
      bool HScrollbarEnabled() const;
      bool VScrollbarEnabled() const;
      void VScrollbarColors(uint32_t track, uint32_t thumb);
      void HScrollbarColors(uint32_t track, uint32_t thumb);
      void VScrollbarArrowSize(uint32_t size);
      void HScrollbarArrowSize(uint32_t size);
      uint32_t LineSpacing() const {return mLineSpacing;}
      void FontSize(uint32_t fs) override;
      void VScrollbarToggle(bool v);
      void HScrollbarToggle(bool v);
//      bool VScrollbarEnabled() const {return mVScrollbarEnabled;}
//      bool HorizontalScrollbarEnabled() const {return mHScrollbarEnabled;}
      bool VScrollbarVisible() const {return mVScrollBar->Visible();}
      bool HScrollbarVisible() const {return mHScrollBar->Visible();}
      void AutoHideHScrollbars(bool v);
      void AutoHideVScrollbars(bool v);
      void HAlign(AUIHAlign align) override;
      void VAlign(AUIVAlign align) override;
      void UpdateScrollbarRanges();// after data or size changes
      int32_t HScrollBarMax() const { return mHScrollBar ? mHScrollBar->MaxValue() : 0; }
      int32_t HScrollBarValue() const { return mHScrollBar ? mHScrollBar->Value() : 0; }
      void RecalcMaxWidth();// after data changes
      AScrollBar* HScrollBar();
      AScrollBar* VScrollBar();
      uint32_t MaxContentWidth() {return mMaxWidthPx;}
      uint32_t LineHeight() {return mLineHeight;}
      uint32_t ComputeStringWidth(const std::string& str) const;
      void SetOnSelectionChanged(SelectionChangedCallback callback, void* userData = nullptr);
      // Bulk operations
      void AddItems(const std::vector<std::string>& items);
      void InsertItems(size_t index, const std::vector<std::string>& items);
      void RemoveItems(const std::vector<size_t>& indices);
      void SetItems(const std::vector<std::string>& items);
      // Selection retrieval
      std::vector<size_t> SelectedIndices() const;
      size_t GetSelectedCount() const;
      size_t GetFirstSelectedIndex() const;
      size_t GetLastSelectedIndex() const;
      bool HasSelection() const;
      // Single‑select convenience
      void SetSelectedIndex(size_t index);
      // Selection colors
      void SelectionColor(uint32_t argb);
      uint32_t SelectionColor() const { return mSelectionColor; }
      void SelectionTextColor(uint32_t argb);
      uint32_t SelectionTextColor() const { return mSelectionTextColor; }
      // Hover highlight
      void HoverColor(uint32_t argb);
      uint32_t HoverColor() const { return mHoverColor; }
      bool IsItemHovered(size_t index) const { return static_cast<int32_t>(index) == mHoveredIndex; }
      int32_t HoveredIndex() const { return mHoveredIndex; }
      // Per‑axis scrollbar controls
      void EnableVScrollbar(bool enable);
      void EnableHScrollbar(bool enable);
      bool IsVScrollbarEnabled() const { return mVScrollbarEnabled && mVScrollBar != nullptr; }
      bool IsHScrollbarEnabled() const { return mHScrollbarEnabled && mHScrollBar != nullptr; }
      void AutoHideVScrollbar(bool enable);
      void AutoHideHScrollbar(bool enable);
      bool IsVScrollbarAutoHide() const { return mAutoHideVScrollbar; }
      bool IsHScrollbarAutoHide() const { return mAutoHideHScrollbar; }
      void AutoHideScrollbars(bool enable);
      void LineTextVAlign(AUIVAlign align);
      void ComputeAlignmentOffsets();
  };

}// namespace aui

#endif // ALIST_H_
