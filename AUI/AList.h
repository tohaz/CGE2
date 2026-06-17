#ifndef ALIST_H_
#define ALIST_H_

namespace aui {

  class AScrollBar;

  using SelectionChangedCallback = std::function<void(AWindow*, AWidget*, void*)>;

  class AList: public AWidget {
    private:
// Data
      std::vector<std::string> mData;
      std::vector<bool> mTag;// selection flags
      uint32_t mLineSpacing = 2U;// pixels between lines
      uint32_t mLineHeight = 16U;// cached (font size + spacing)
      int32_t mVOffset = 0;// vertical scroll offset (pixels)
      int32_t mHOffset = 0;// horizontal scroll offset (pixels)
      uint32_t mMaxWidthPx = 0;// maximum text width in the list
      bool mMultiSelect = false;// multi‑select mode (no modifiers)
      bool mScrollbarsEnabled = false;
// Scrollbars (managed manually, not registered in AUI)
      std::unique_ptr<AScrollBar> mVScrollBar;
      std::unique_ptr<AScrollBar> mHScrollBar;
      AScrollBar *mDragScrollbar = nullptr;
      AUIVAlign mLineTextVAlign = AUIVAlign::center;
// Glyph cache for performance (character -> alpha bitmap)
      struct CachedGlyph {
          uint32_t width;
          uint32_t height;
          int32_t left;
          int32_t top;
          int32_t advance;
          std::vector<uint8_t> bitmap;// alpha values
      };
      mutable std::unordered_map<uint32_t, CachedGlyph> mGlyphCache;
      void CacheGlyph(uint32_t codepoint) const;
      void ClearGlyphCache();
      void RecalcLineHeight();// after font size or spacing change
      size_t IndexFromY(int32_t y) const;// convert local y to item index
      int32_t GetLineTop(size_t index) const;// y position (pixels) of line start
      void DrawItem(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t absX, int32_t absY,
          size_t index, bool isSelected) const;
      void DrawScrollbars(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
          int32_t offsetY) const;
      bool mAutoHideScrollbars = true;
      void RecalcScrollFromAlignment();
      SelectionChangedCallback mOnSelectionChanged;
      void* mSelectionUserData = nullptr;
      void NotifySelectionChanged();
    protected:
    public:
      AList();
      ~AList() override;
      void Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX, int32_t offsetY) const
          override;
      bool OnMouseClick(int32_t localX, int32_t localY, bool pressed) override;
      void OnMouseMove(int32_t localX, int32_t localY) override;
      void OnMouseWheel(int32_t delta) override;
      void OnParentResize(uint32_t newWidth, uint32_t newHeight) override;
      static AList* AttachTo(AWindow *parent);
      static AList* AttachTo(AWidget *parent);
      void AddItem(const std::string &text);
      void InsertItem(size_t index, const std::string &text);
      void RemoveItem(size_t index);
      void Clear();
      size_t GetItemCount() const {return mData.size();}
      const std::string& GetItem(size_t index) const;
      void SetItem(size_t index, const std::string &text);
      void SetMultiSelect(bool enable);
      bool IsMultiSelect() const {return mMultiSelect;}
      void SelectAll(bool selected);
      void SelectIndex(size_t index, bool selected);
      bool IsSelected(size_t index) const;
      std::vector<size_t> GetSelectedIndices() const;
      void ClearSelection();
      void ScrollToOffset(int32_t xOffset, int32_t yOffset);
      void ScrollToItem(size_t index, bool alignCenter = false);
      int32_t GetVerticalOffset() const {return mVOffset;}
      int32_t GetHorizontalOffset() const {return mHOffset;}
      void SetLineSpacing(uint32_t spacing);
      void SetScrollbarsEnabled(bool enable);
      bool AreScrollbarsEnabled() const {return mScrollbarsEnabled;}
      void SetVScrollbarColors(uint32_t track, uint32_t thumb);
      void SetHScrollbarColors(uint32_t track, uint32_t thumb);
      void SetVScrollbarArrowSize(uint32_t size);
      void SetHScrollbarArrowSize(uint32_t size);
      void SetLineTextVAlign(AUIVAlign align) {
        mLineTextVAlign = align;
        if(mParentWindow)
          mParentWindow->Draw();
      }
      uint32_t GetLineSpacing() const {return mLineSpacing;}
      uint32_t GetFontSize() const {return mFontSize;}
      void SetFontSize(uint32_t fs) override;
      void SetVerticalScrollbarEnabled(bool enable);
      void SetHorizontalScrollbarEnabled(bool enable);
      bool IsVerticalScrollbarEnabled() const {return mVScrollBar != nullptr;}
      bool IsHorizontalScrollbarEnabled() const {return mHScrollBar != nullptr;}
      void SetAutoHideScrollbars(bool enable);
      void SetVAlignment(AUIVAlign align);
      void UpdateScrollbarRanges();// after data or size changes
      void SetHAlignment(AUIHAlign align) override;
      int32_t GetHScrollBarMax() const { return mHScrollBar ? mHScrollBar->GetMaxValue() : 0; }
      int32_t GetHScrollBarValue() const { return mHScrollBar ? mHScrollBar->GetValue() : 0; }
      void RecalcMaxWidth();// after data changes
      AScrollBar* GetHScrollBar() {return mHScrollBar.get();}
      bool IsVerticalScrollbarVisible() const { return mVScrollBar && mVScrollBar->IsVisible(); }
      uint32_t GetMaxContentWidth() {return mMaxWidthPx;}
      uint32_t GetLineHeight() {return mLineHeight;}
      AScrollBar* GetVScrollBar();
      uint32_t ComputeStringWidth(const std::string& str) const;
      void SetOnSelectionChanged(SelectionChangedCallback callback, void* userData = nullptr);

  };

}// namespace aui

#endif // ALIST_H_

