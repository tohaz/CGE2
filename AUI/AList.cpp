#include "AUILib.h"

namespace aui {

//// ----------------------------------------------------------------------
//// Constructor / Destructor
//// ----------------------------------------------------------------------
  AList::AList() {
    mSizeX = 200;
    mSizeY = 150;
    mDefaultFillBG = true;
    mBGColor = 0xFFEEEEEE;
    mHAlign = AUIHAlign::left;
    mVAlign = AUIVAlign::top;
    RecalcLineHeight();
// Scrollbars are created lazily in VScrollBar()/HScrollBar().
  }

  AList::~AList() = default;
//// ----------------------------------------------------------------------
//// Internal helpers (existing)
//// ----------------------------------------------------------------------
  void AList::RecalcLineHeight() {
    mLineHeight = mFontSize + mLineSpacing;
  }

  size_t AList::IndexFromY(int32_t y) const {
    if(mLineHeight == 0 || y < 0)
      return 0;
    size_t idx = static_cast<size_t>(y / static_cast<int32_t>(mLineHeight));
    return (idx >= mData.size()) ? mData.size() : idx;
  }

  int32_t AList::LineTop(size_t index) const {
    return static_cast<int32_t>(index * mLineHeight);
  }

  void AList::RecalcMaxWidth() {
    mMaxWidthPx = 0;
    for(const auto& str : mData) {
      uint32_t w = ComputeStringWidth(str);
      if(w > mMaxWidthPx)
        mMaxWidthPx = w;
    }
  }
//
  uint32_t AList::ComputeStringWidth(const std::string& str) const {
    AUI* au = Wnd() ? Wnd()->EnginePtr() : nullptr;
    if(!au)
      return 0;
    FT_Face face = au->DefaultFontFace();
    if(!face)
      return 0;
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    uint32_t width = 0;
    for(char c : str) {
      if(FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_DEFAULT) == 0)
        width += static_cast<uint32_t>(face->glyph->advance.x >> 6);
    }
    return width;
  }

  void AList::UpdateScrollbarRanges() {
// Get actual scrollbar dimensions
    int32_t vScrollWidth = (mVScrollBar && mVScrollBar->Visible()) ? static_cast<int32_t>(mVScrollBar->SizeX()) : 0;
    int32_t hScrollHeight = (mHScrollBar && mHScrollBar->Visible()) ? static_cast<int32_t>(mHScrollBar->SizeY()) : 0;
    int32_t totalH = static_cast<int32_t>(mData.size() * mLineHeight);
    int32_t totalW = static_cast<int32_t>(mMaxWidthPx);
    int32_t viewH = static_cast<int32_t>(mSizeY) - hScrollHeight;
    int32_t viewW = static_cast<int32_t>(mSizeX) - vScrollWidth;
    if(mVScrollBar) {
// Document range: max = total document height, pageStep = viewport height
      int32_t maxV = totalH;//
      int32_t maxOffset = std::max(0, totalH - viewH);
      mVScrollBar->Range(0, maxV);
      mVScrollBar->PageStep(viewH);
// Clamp current offset to valid scroll range [0, maxOffset]
      if(mVOffset > maxOffset)
        mVOffset = maxOffset;
      if(mVOffset < 0)
        mVOffset = 0;
      mVScrollBar->Value(mVOffset);
    }
    if(mHScrollBar) {
      int32_t maxH = totalW;// <-- CHANGED
      int32_t maxOffset = std::max(0, totalW - viewW);
      mHScrollBar->Range(0, maxH);
      mHScrollBar->PageStep(viewW);
      if(mHOffset > maxOffset)
        mHOffset = maxOffset;
      if(mHOffset < 0)
        mHOffset = 0;
      mHScrollBar->Value(mHOffset);
    }
  }

//// ----------------------------------------------------------------------
//// NEW internal helpers
//// ----------------------------------------------------------------------
  void AList::UpdateHoveredItem(int32_t mouseX, int32_t mouseY) {
    int32_t newHover = -1;
    int32_t contentW = static_cast<int32_t>(mSizeX) - (IsVScrollbarEnabled() ? 20 : 0);
    int32_t contentH = static_cast<int32_t>(mSizeY) - (IsHScrollbarEnabled() ? 20 : 0);
    if(mouseX >= 0 && mouseX < contentW && mouseY >= 0 && mouseY < contentH) {
      size_t idx = IndexFromY(mouseY + mVOffset);
      if(idx < mData.size())
        newHover = static_cast<int32_t>(idx);
    }
    if(newHover != mHoveredIndex) {
      mHoveredIndex = newHover;
      if(Wnd())
        Wnd()->RequestRedraw();
    }
  }

  void AList::OnScroll(int32_t value, AScrollBar* sender) {
    if(sender == mVScrollBar) {
      mVOffset = value;
    }
    else
      if(sender == mHScrollBar) {
        mHOffset = value;
      }
    if(Wnd())
      Wnd()->RequestRedraw();
  }
//// ----------------------------------------------------------------------
//// Overrides
//// ----------------------------------------------------------------------
  bool AList::OnMouseMove(int32_t localX, int32_t localY) {
    if(mHLOnMouseMove) {
      UpdateHoveredItem(localX, localY);
    }
    return AWidget::OnMouseMove(localX, localY);
  }

  void AList::OnMouseWheel(int32_t delta) {
    int32_t newOffset = mVOffset - delta * 20;
    if(mVScrollBar) {
      mVScrollBar->Value(newOffset);
    }
    else {
      int32_t maxV = std::max(0,
          static_cast<int32_t>(mData.size() * mLineHeight - (mSizeY - (IsHScrollbarEnabled() ? 20 : 0))));
      mVOffset = std::clamp(newOffset, 0, maxV);
      if(Wnd())
        Wnd()->RequestRedraw();
    }
  }

  void AList::OnResize(uint32_t w, uint32_t h) {
    AWidget::OnResize(w, h);
    int32_t border = static_cast<int32_t>(mBorderThick);
    int32_t scrollSize = 20;// you can make this configurable
    if(mVScrollBar) {
      int32_t vw = scrollSize;
      int32_t vh = static_cast<int32_t>(h) - 2 * border - (IsHScrollbarEnabled() ? scrollSize : 0);
      if(vh < 0)
        vh = 0;
      mVScrollBar->Move(static_cast<int32_t>(w) - border - vw, border);
      mVScrollBar->Resize(static_cast<uint32_t>(vw), static_cast<uint32_t>(vh));
    }
    if(mHScrollBar) {
      int32_t hh = scrollSize;
      int32_t hw = static_cast<int32_t>(w) - 2 * border - (IsVScrollbarEnabled() ? scrollSize : 0);
      if(hw < 0)
        hw = 0;
      mHScrollBar->Move(border, static_cast<int32_t>(h) - border - hh);
      mHScrollBar->Resize(static_cast<uint32_t>(hw), static_cast<uint32_t>(hh));
    }
    UpdateScrollbarRanges();
    UpdateScrollbarVisibility();
  }

  void AList::OnDraw(UNUSED uint32_t* buffer, UNUSED uint32_t bufferW, UNUSED uint32_t bufferH,
  UNUSED int32_t offsetX, UNUSED int32_t offsetY,
  UNUSED int32_t clipL, UNUSED int32_t clipT, UNUSED int32_t clipR, UNUSED int32_t clipB) const {
    int32_t border = static_cast<int32_t>(mBorderThick);
    int32_t vw = (mVScrollBar && mVScrollBar->Visible()) ? static_cast<int32_t>(mVScrollBar->SizeX()) : 0;
// Content area inside border, excluding vertical scrollbar
    int32_t contentW = static_cast<int32_t>(mSizeX) - 2 * border - vw;
// Content origin (shifted by scroll offset)
    int32_t left = offsetX + mX + border - mHOffset;
    int32_t top = offsetY + mY + border;
// Visible clip region (list viewport)
    int32_t drawL = std::max(clipL, offsetX + mX + border);
    int32_t drawT = std::max(clipT, top);
    int32_t drawR = std::min(clipR, offsetX + mX + static_cast<int32_t>(mSizeX) - border);
    int32_t drawB = std::min(clipB, offsetY + mY + static_cast<int32_t>(mSizeY) - border);
    if(drawL >= drawR || drawT >= drawB)
      return;
    AUI* au = Wnd() ? Wnd()->EnginePtr() : nullptr;
    if(!au)
      return;
    FT_Face face = au->DefaultFontFace();
    if(!face)
      return;
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
// Vertical visible range
    int32_t startY = drawT - top + mVOffset;
    int32_t endY = drawB - top + mVOffset;
    size_t first = IndexFromY(startY);
    size_t last = IndexFromY(endY);
    if(first >= mData.size())
      return;
    if(last >= mData.size())
      last = mData.size() - 1;
    for(size_t i = first; i <= last; ++i) {
      int32_t lineY = top + LineTop(i) - mVOffset;
      if(lineY + static_cast<int32_t>(mLineHeight) < drawT)
        continue;
      if(lineY > drawB)
        break;
      bool selected = mTaggedItems[i];
      bool hovered = (static_cast<int32_t>(i) == mHoveredIndex);
      uint32_t bg = selected ? mSelectionColor : (hovered ? mHoverColor : mBGColor);
// Background fill
      int32_t rL, rR;
      if(selected) {
        rL = drawL;
        rR = drawR;
      }
      else {
        rL = std::max(left, drawL);
        rR = std::min(left + contentW, drawR);
      }
      int32_t rT = std::max(lineY, drawT);
      int32_t rB = std::min(lineY + static_cast<int32_t>(mLineHeight), drawB);
      if(rL < rR && rT < rB) {
        FillRect(buffer, bufferW, rL, rT, rR - rL, rB - rT, bg);
      }
// Text drawing
      uint32_t textColor = (selected && mSelectionTextColor != 0) ? mSelectionTextColor : mTextColor;
      if(!mData[i].empty()) {
        int32_t tx = left + 2;
        int32_t ty = lineY + (static_cast<int32_t>(mLineHeight) - static_cast<int32_t>(mFontSize)) / 2;
// Use the full document width (or viewport if larger) for alignment
        uint32_t docWidth = std::max(mMaxWidthPx, static_cast<uint32_t>(contentW - 4));
        ARect textBounds { tx, ty, docWidth, mFontSize };
        ARect clipBounds { drawL, drawT, static_cast<uint32_t>(drawR - drawL), static_cast<uint32_t>(drawB - drawT) };
        ATextStyle style { textColor, mFontSize, AWidget::HAlign(), mLineTextVAlign, 0.0 };
        DrawTextEx(buffer, bufferW, bufferH, textBounds, mData[i], face, style, &clipBounds);
      }
    }
  }
//// ----------------------------------------------------------------------
//// Item management
//// ----------------------------------------------------------------------
  void AList::AddItem(const std::string& text) {
    mData.push_back(text);
    mTaggedItems.push_back(false);
// O(1) width update instead of O(N) full rescan
    uint32_t newWidth = ComputeStringWidth(text);
    if(newWidth > mMaxWidthPx) {
      mMaxWidthPx = newWidth;
    }
    UpdateScrollbarRanges();// updates min/max/page
    UpdateScrollbarVisibility();// updates show/hide, recomputes offsets, syncs values
    if(Wnd())
      Wnd()->RequestRedraw();
  }
//
  void AList::InsertItem(size_t index, const std::string& text) {
    if(index > mData.size())
      index = mData.size();
    auto idx = static_cast<std::ptrdiff_t>(index);
    mData.insert(mData.begin() + idx, text);
    mTaggedItems.insert(mTaggedItems.begin() + idx, false);
    RecalcMaxWidth();
    UpdateScrollbarRanges();
    UpdateScrollbarVisibility();
    if(Wnd())
      Wnd()->RequestRedraw();
  }
//
  void AList::RemoveItem(size_t index) {
    if(index >= mData.size())
      return;
    auto idx = static_cast<std::ptrdiff_t>(index);
    mData.erase(mData.begin() + idx);
    mTaggedItems.erase(mTaggedItems.begin() + idx);
    RecalcMaxWidth();
    UpdateScrollbarRanges();
    UpdateScrollbarVisibility();
    if(Wnd())
      Wnd()->RequestRedraw();
  }
//
  void AList::Clear() {
    mData.clear();
    mTaggedItems.clear();
    mMaxWidthPx = 0;
    mVOffset = 0;
    mHOffset = 0;
    UpdateScrollbarRanges();
    UpdateScrollbarVisibility();
    if(Wnd())
      Wnd()->RequestRedraw();
  }
//
  const std::string& AList::GetItem(size_t index) const {
    static const std::string empty;
    if(index >= mData.size())
      return empty;
    return mData[index];
  }

  void AList::Item(size_t index, const std::string& text) {
    if(index >= mData.size())
      return;
    mData[index] = text;
    RecalcMaxWidth();
    UpdateScrollbarRanges();
    if(Wnd())
      Wnd()->RequestRedraw();
  }
//
//// ----------------------------------------------------------------------
//// Bulk operations
//// ----------------------------------------------------------------------
  void AList::AddItems(const std::vector<std::string>& texts) {
    if(texts.empty())
      return;
    mData.reserve(mData.size() + texts.size());
    mTaggedItems.reserve(mTaggedItems.size() + texts.size());
    for(const auto& text : texts) {
      mData.push_back(text);
      mTaggedItems.push_back(false);
      uint32_t w = ComputeStringWidth(text);
      if(w > mMaxWidthPx) {
        mMaxWidthPx = w;
      }
    }
// Perform a single layout and redraw pass after ALL items are added
    UpdateScrollbarRanges();
    UpdateScrollbarVisibility();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AList::InsertItems(size_t index, const std::vector<std::string>& items) {
    if(items.empty())
      return;
    if(index > mData.size())
      index = mData.size();
    auto idx = static_cast<std::ptrdiff_t>(index);
    mData.insert(mData.begin() + idx, items.begin(), items.end());
    mTaggedItems.insert(mTaggedItems.begin() + idx, items.size(), false);
    RecalcMaxWidth();
    UpdateScrollbarRanges();
    UpdateScrollbarVisibility();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AList::RemoveItems(const std::vector<size_t>& indices) {
    if(indices.empty())
      return;
    std::vector<size_t> sorted = indices;
    std::sort(sorted.begin(), sorted.end(), std::greater<size_t>());
    for(size_t idx : sorted) {
      if(idx < mData.size()) {
        auto pos = static_cast<std::ptrdiff_t>(idx);
        mData.erase(mData.begin() + pos);
        mTaggedItems.erase(mTaggedItems.begin() + pos);
      }
    }
    RecalcMaxWidth();
    UpdateScrollbarRanges();
    UpdateScrollbarVisibility();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AList::SetItems(const std::vector<std::string>& items) {
    mData = items;
    mTaggedItems.assign(mData.size(), false);
    RecalcMaxWidth();
    UpdateScrollbarRanges();
    UpdateScrollbarVisibility();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

//// ----------------------------------------------------------------------
//// Selection
//// ----------------------------------------------------------------------
  void AList::MultiSelect(bool enable) {
    mMultiSelect = enable;
  }

  void AList::SelectAll(bool selected) {
    std::fill(mTaggedItems.begin(), mTaggedItems.end(), selected);
    if(mOnSelectionChanged)
      mOnSelectionChanged(this, mSelectionUserData);
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AList::SelectIndex(size_t index, bool selected) {
    if(index >= mData.size())
      return;
    if(!mMultiSelect) {
      std::fill(mTaggedItems.begin(), mTaggedItems.end(), false);
    }
    mTaggedItems[index] = selected;
    if(mOnSelectionChanged)
      mOnSelectionChanged(this, mSelectionUserData);
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  bool AList::IsSelected(size_t index) const {
    if(index >= mData.size())
      return false;
    return mTaggedItems[index];
  }

  void AList::ClearSelection() {
    std::fill(mTaggedItems.begin(), mTaggedItems.end(), false);
    if(mOnSelectionChanged)
      mOnSelectionChanged(this, mSelectionUserData);
    if(Wnd())
      Wnd()->RequestRedraw();
  }

//// ----------------------------------------------------------------------
//// Selection retrieval
//// ----------------------------------------------------------------------
  std::vector<size_t> AList::SelectedIndices() const {
    std::vector<size_t> result;
    for(size_t i = 0; i < mTaggedItems.size(); ++i)
      if(mTaggedItems[i])
        result.push_back(i);
    return result;
  }

  size_t AList::GetSelectedCount() const {
    return static_cast<size_t>(std::count(mTaggedItems.begin(), mTaggedItems.end(), true));
  }

  size_t AList::GetFirstSelectedIndex() const {
    auto it = std::find(mTaggedItems.begin(), mTaggedItems.end(), true);
    return
        it == mTaggedItems.end() ?
            static_cast<size_t>(-1) : static_cast<size_t>(std::distance(mTaggedItems.begin(), it));
  }

  size_t AList::GetLastSelectedIndex() const {
    for(size_t i = mTaggedItems.size(); i-- > 0;)
      if(mTaggedItems[i])
        return i;
    return static_cast<size_t>(-1);
  }

  bool AList::HasSelection() const {
    return std::any_of(mTaggedItems.begin(), mTaggedItems.end(), [](bool b) {
      return b;
    });
  }

  void AList::SetSelectedIndex(size_t index) {
    if(index >= mData.size())
      return;
    std::fill(mTaggedItems.begin(), mTaggedItems.end(), false);
    mTaggedItems[index] = true;
    if(mOnSelectionChanged)
      mOnSelectionChanged(this, mSelectionUserData);
    if(Wnd())
      Wnd()->RequestRedraw();
  }

//// ----------------------------------------------------------------------
//// Selection colors
//// ----------------------------------------------------------------------
  void AList::SelectionColor(uint32_t argb) {
    mSelectionColor = argb;
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AList::SelectionTextColor(uint32_t argb) {
    mSelectionTextColor = argb;
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AList::HoverColor(uint32_t argb) {
    mHoverColor = argb;
    if(Wnd())
      Wnd()->RequestRedraw();
  }
//// ----------------------------------------------------------------------
//// Scrollbar enable/disable
//// ----------------------------------------------------------------------
  void AList::EnableVScrollbar(bool v) {
    D2("setting vscrollbar {}", v)
    mVScrollbarEnabled = v;
    if(v && !mVScrollBar) {
      VScrollBar();// force creation
    }
    else {
      if(!v && mVScrollBar) {
        mVScrollBar->Hide();
      }
    }
    UpdateScrollbarRanges();
    UpdateScrollbarVisibility();
    if(Wnd()) {
      Wnd()->RequestRedraw();
    }
    D2("vscrollbar {}", v)
  }

  void AList::EnableHScrollbar(bool enable) {
    mHScrollbarEnabled = enable;
    if(enable && !mHScrollBar) {
      HScrollBar();// force creation
    }
    else
      if(!enable && mHScrollBar) {
        mHScrollBar->Hide();
      }
    UpdateScrollbarRanges();
    UpdateScrollbarVisibility();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AList::AutoHideVScrollbar(bool enable) {
    mAutoHideVScrollbar = enable;
    UpdateScrollbarVisibility();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AList::AutoHideHScrollbar(bool enable) {
    mAutoHideHScrollbar = enable;
    UpdateScrollbarVisibility();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AList::AutoHideScrollbars(bool enable) {
    AutoHideVScrollbar(enable);
    AutoHideHScrollbar(enable);
  }

//// ----------------------------------------------------------------------
//// Other public methods
//// ----------------------------------------------------------------------
  void AList::ScrollToOffset(int32_t xOffset, int32_t yOffset) {
    if(mVScrollBar)
      mVScrollBar->Value(yOffset);
    else
      mVOffset = yOffset;
    if(mHScrollBar)
      mHScrollBar->Value(xOffset);
    else
      mHOffset = xOffset;
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AList::ScrollToItem(size_t index, bool alignCenter) {
    if(index >= mData.size())
      return;
    int32_t targetY = static_cast<int32_t>(index * mLineHeight);
    if(alignCenter) {
      int32_t viewH = static_cast<int32_t>(mSizeY) - (IsHScrollbarEnabled() ? 20 : 0);
      targetY -= viewH / 2;
    }
    ScrollToOffset(0, targetY);
  }

  void AList::LineSpacing(uint32_t spacing) {
    if(mLineSpacing == spacing)
      return;
    mLineSpacing = spacing;
    RecalcLineHeight();
    UpdateScrollbarVisibility();// updates ranges, recomputes offsets, syncs values, and redraws
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AList::FontSize(uint32_t fs) {
    if(mFontSize == fs)
      return;
    AWidget::FontSize(fs);
    RecalcLineHeight();
    RecalcMaxWidth();
    UpdateScrollbarVisibility();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AList::VScrollbarColors(uint32_t track, uint32_t thumb) {
    if(mVScrollBar) {
      mVScrollBar->TrackColor(track);
      mVScrollBar->ThumbColor(thumb);
    }
  }

  void AList::HScrollbarColors(uint32_t track, uint32_t thumb) {
    if(mHScrollBar) {
      mHScrollBar->TrackColor(track);
      mHScrollBar->ThumbColor(thumb);
    }
  }

  void AList::VScrollbarArrowSize(uint32_t size) {
    if(mVScrollBar)
      mVScrollBar->ArrowSize(size);
  }

  void AList::HScrollbarArrowSize(uint32_t size) {
    if(mHScrollBar)
      mHScrollBar->ArrowSize(size);
  }

  void AList::VScrollbarToggle(bool v) {
    EnableVScrollbar(v);
  }

  void AList::HScrollbarToggle(bool v) {
    EnableHScrollbar(v);
  }

  void AList::AutoHideHScrollbars(bool v) {
    AutoHideHScrollbar(v);
  }

  void AList::AutoHideVScrollbars(bool v) {
    AutoHideVScrollbar(v);
  }
//// ----------------------------------------------------------------------
//// Scrollbar accessors (lazy creation)
//// ----------------------------------------------------------------------
  AScrollBar* AList::VScrollBar() {
    if(!mVScrollBar) {
      mVScrollBar = AScrollBar::AttachTo(this, AUIOrientation::vertical);
      mVScrollBar->Visible(false);// manual drawing
      mVScrollBar->SetScrollCallback([this](AWidget*, void*, int32_t val) noexcept {
        this->OnScroll(val, mVScrollBar);
      });
      int32_t border = static_cast<int32_t>(mBorderThick);
      int32_t w = static_cast<int32_t>(mSizeX);
      int32_t h = static_cast<int32_t>(mSizeY);
      int32_t vw = 20;// or use a configurable scrollbar width
      int32_t vh = h - 2 * border - (IsHScrollbarEnabled() ? 20 : 0);
      if(vh < 0)
        vh = 0;
      mVScrollBar->Move(w - border - vw, border);
      mVScrollBar->Resize(static_cast<uint32_t>(vw), static_cast<uint32_t>(vh));
      UpdateScrollbarVisibility();
      if(Wnd())
        Wnd()->RequestRedraw();
    }
    return mVScrollBar;
  }

  AScrollBar* AList::HScrollBar() {
    if(!mHScrollBar) {
      mHScrollBar = AScrollBar::AttachTo(this, AUIOrientation::horizontal);
      mHScrollBar->Visible(false);
      mHScrollBar->SetScrollCallback([this](AWidget*, void*, int32_t val) noexcept {
        this->OnScroll(val, mHScrollBar);
      });
      int32_t border = static_cast<int32_t>(mBorderThick);
      int32_t w = static_cast<int32_t>(mSizeX);
      int32_t h = static_cast<int32_t>(mSizeY);
      int32_t hh = 20;
      int32_t hw = w - 2 * border - (IsVScrollbarEnabled() ? 20 : 0);
      if(hw < 0)
        hw = 0;
      mHScrollBar->Move(border, h - border - hh);
      mHScrollBar->Resize(static_cast<uint32_t>(hw), static_cast<uint32_t>(hh));
      UpdateScrollbarVisibility();
      if(Wnd())
        Wnd()->RequestRedraw();
    }
    return mHScrollBar;
  }

  void AList::ScrollbarsEnabled(bool enable) {
    EnableHScrollbar(enable);
    EnableVScrollbar(enable);
  }

  bool AList::ScrollbarsEnabled() {
    D1("{} {}", mVScrollbarEnabled, mHScrollbarEnabled)
    return (mVScrollbarEnabled && mHScrollbarEnabled);
  }

  bool AList::HScrollbarEnabled() const {
    return mHScrollbarEnabled;
  }

  bool AList::VScrollbarEnabled() const {
    return mVScrollbarEnabled;
  }

  void AList::SetOnSelectionChanged(SelectionChangedCallback callback, void* userData) {
    mOnSelectionChanged = std::move(callback);
    mSelectionUserData = userData;
  }
//// ----------------------------------------------------------------------
//// Update scrollbar visibility (calls ranges and shows/hides)
//// ----------------------------------------------------------------------
  void AList::UpdateScrollbarVisibility() {
    bool showV = false, showH = false;
    const int32_t scrollSize = 20;
    if(mVScrollbarEnabled && mVScrollBar) {
      int32_t totalH = static_cast<int32_t>(mData.size() * mLineHeight);
      int32_t viewH = static_cast<int32_t>(mSizeY) - (IsHScrollbarEnabled() ? scrollSize : 0);
      showV = !(mAutoHideVScrollbar && totalH <= viewH);
    }
    if(mHScrollbarEnabled && mHScrollBar) {
      int32_t totalW = static_cast<int32_t>(mMaxWidthPx);
      int32_t viewW = static_cast<int32_t>(mSizeX) - (IsVScrollbarEnabled() ? scrollSize : 0);
      showH = !(mAutoHideHScrollbar && totalW <= viewW);
    }
    if(mVScrollBar) {
      if(showV)
        mVScrollBar->Show();
      else
        mVScrollBar->Hide();
    }
    if(mHScrollBar) {
      if(showH)
        mHScrollBar->Show();
      else
        mHScrollBar->Hide();
    }
// Recompute ranges (viewport may have changed)
    UpdateScrollbarRanges();
// Recompute alignment offsets and push to scrollbars
    ComputeAlignmentOffsets();
    SyncScrollbarValues();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  AWidget* AList::OnMouseDownLeft(int32_t localX, int32_t localY) {
// 1. Give children (scrollbars) a chance to consume the event
    AWidget* consumed = AWidget::OnMouseDownLeft(localX, localY);
    if(consumed) {
      return consumed;// scrollbar handled it
    }
// 2. No child consumed – handle item selection
    const int32_t scrollSize = 20;
    int32_t contentW = static_cast<int32_t>(mSizeX) - (mVScrollBar && mVScrollBar->Visible() ? scrollSize : 0);
    int32_t contentH = static_cast<int32_t>(mSizeY) - (mHScrollBar && mHScrollBar->Visible() ? scrollSize : 0);
    if(localX < 0 || localX >= contentW || localY < 0 || localY >= contentH)
      return nullptr;
    int32_t y = localY + mVOffset;
    size_t idx = IndexFromY(y);
    if(idx >= mData.size())
      return nullptr;
    if(mMultiSelect) {
      SelectIndex(idx, !IsSelected(idx));
    }
    else {
      SetSelectedIndex(idx);
    }
    return this;// consume the event (so it doesn't propagate further)
  }

  void AList::ComputeAlignmentOffsets() {
    int32_t vScrollWidth = (mVScrollBar && mVScrollBar->Visible()) ? static_cast<int32_t>(mVScrollBar->SizeX()) : 0;
    int32_t hScrollHeight = (mHScrollBar && mHScrollBar->Visible()) ? static_cast<int32_t>(mHScrollBar->SizeY()) : 0;
// ----- Vertical Alignment / Scroll -----
    int32_t viewHeight = static_cast<int32_t>(mSizeY) - hScrollHeight;
    int32_t contentHeight = static_cast<int32_t>(mData.size() * mLineHeight);
    if(contentHeight <= viewHeight) {
      switch(mVAlign) {
        case AUIVAlign::top:
          mVOffset = 0;
          break;
        case AUIVAlign::center:
          mVOffset = -(viewHeight - contentHeight) / 2;
          break;
        case AUIVAlign::bottom:
          mVOffset = -(viewHeight - contentHeight);
          break;
        default:
          mVOffset = 0;
          break;
      }
    }
    else {
      int32_t maxV = contentHeight - viewHeight;
      mVOffset = std::max(0, std::min(mVOffset, maxV));
    }
    // ----- Horizontal Alignment / Scroll -----
    int32_t viewWidth = static_cast<int32_t>(mSizeX) - vScrollWidth;
    int32_t contentWidth = static_cast<int32_t>(mMaxWidthPx);
    if (contentWidth <= viewWidth) {
        mHOffset = 0;
    } else {
        // Overflows – preserve current user scroll position, clamped to [0, maxH].
        // DO NOT override mHOffset using mHAlign here!
        int32_t maxH = contentWidth - viewWidth;
        mHOffset = std::max(0, std::min(mHOffset, maxH));
    }
  }

  void AList::HAlign(AUIHAlign align) {
    if(mHAlign == align)
      return;
    AWidget::HAlign(align);// stores alignment
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AList::VAlign(AUIVAlign align) {
    if(mVAlign == align)
      return;
    AWidget::VAlign(align);
    ComputeAlignmentOffsets();
    SyncScrollbarValues();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AList::LineTextVAlign(AUIVAlign align) {
    mLineTextVAlign = align;
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AList::SyncScrollbarValues() {
    if(mVScrollBar)
      mVScrollBar->Value(mVOffset);
    if(mHScrollBar)
      mHScrollBar->Value(mHOffset);
  }
}// namespace aui
