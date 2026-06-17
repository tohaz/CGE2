#include "AUILib.h"

namespace aui {

  AList::AList() :
      mLineSpacing(0), mMultiSelect(false), mScrollbarsEnabled(false), mDragScrollbar(nullptr) {
    D2("AList constructed");
    mX = AUI_LIST_X;
    mY = AUI_LIST_Y;
    mSizeX = AUI_LIST_SZX;
    mSizeY = AUI_LIST_SZY;
    mBGColor = 0xFFCCCCCC;
    mTextColor = 0xFF000000;
    mFontSize = 14;
    mBorderThick = 1;
    mBorderColor = 0xFF888888;
    mWidgetType = AUIWidgetType::defaultList;
    mHAlign = AUIHAlign::right;
    mVAlign = AUIVAlign::top;
    RecalcLineHeight();// this will set mLineHeight
  }

  AList* AList::AttachTo(AWindow *parent) {
    D1("Attaching AList to window");
    if(!parent)
      E("AList::AttachTo: parent window is null");
    AList *list = new AList();
    parent->AddWidget(std::unique_ptr<AWidget>(list));
    return list;
  }

  AList* AList::AttachTo(AWidget *parent) {
    D1("Attaching AList to widget");
    if(!parent)
      E("AList::AttachTo: parent widget is null");
    AList *list = new AList();
    parent->AddWidget(std::unique_ptr<AWidget>(list));
    return list;
  }

  void AList::AddItem(const std::string &text) {
    mData.push_back(text);
    mTag.push_back(false);
// Measure only the new string
    uint32_t newWidth = ComputeStringWidth(text);
    if(newWidth > mMaxWidthPx) {
      mMaxWidthPx = newWidth;
    }
// Update scrollbars (vertical range changes because item count increased)
    UpdateScrollbarRanges();
    if(mParentWindow)
      mParentWindow->Draw();
  }

  uint32_t AList::ComputeStringWidth(const std::string &str) const {
    if(!mEnginePtr)
      return 0;
    FT_Face face = mEnginePtr->GetDefaultFontFace();
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

  void AList::InsertItem(size_t index, const std::string &text) {
    if(index > mData.size())
      index = mData.size();
    mData.insert(mData.begin() + static_cast<ptrdiff_t>(index), text);
    mTag.insert(mTag.begin() + static_cast<ptrdiff_t>(index), false);
    RecalcMaxWidth();
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AList::RemoveItem(size_t index) {
    if(index >= mData.size())
      return;
    mData.erase(mData.begin() + static_cast<ptrdiff_t>(index));
    mTag.erase(mTag.begin() + static_cast<ptrdiff_t>(index));
    RecalcMaxWidth();
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AList::Clear() {
    mData.clear();
    mTag.clear();
    mMaxWidthPx = 0;
    mVOffset = 0;
    mHOffset = 0;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  const std::string& AList::GetItem(size_t index) const {
    static const std::string empty;
    if(index >= mData.size())
      return empty;
    return mData[index];
  }

  void AList::SetItem(size_t index, const std::string &text) {
    if(index >= mData.size())
      return;
    mData[index] = text;
    RecalcMaxWidth();
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AList::SelectAll(bool selected) {
    for(size_t i = 0; i < mTag.size(); ++i) {
      mTag[i] = selected;
    }
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AList::SelectIndex(size_t index, bool selected) {
    if(index >= mTag.size())
      return;
    if(!mMultiSelect && selected) {
// Single‑select mode: clear all others first
      for(size_t i = 0; i < mTag.size(); ++i) {
        mTag[i] = false;
      }
    }
    mTag[index] = selected;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  bool AList::IsSelected(size_t index) const {
    if(index >= mTag.size())
      return false;
    return mTag[index];
  }

  std::vector<size_t> AList::GetSelectedIndices() const {
    std::vector<size_t> result;
    for(size_t i = 0; i < mTag.size(); ++i) {
      if(mTag[i])
        result.push_back(i);
    }
    return result;
  }

  void AList::ClearSelection() {
    for(size_t i = 0; i < mTag.size(); ++i) {
      mTag[i] = false;
    }
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AList::RecalcMaxWidth() {
    if(!mEnginePtr)
      return;
    FT_Face face = mEnginePtr->GetDefaultFontFace();
    if(!face) {
      E("no face");
    }
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    uint32_t maxWidth = 0;
    for(const auto &str : mData) {
      uint32_t width = 0;
      for(char c : str) {
        if(FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_DEFAULT) != 0)
          continue;
        width += static_cast<uint32_t>(face->glyph->advance.x >> 6);
      }
      if(width > maxWidth)
        maxWidth = width;
    }
    mMaxWidthPx = maxWidth;
    UpdateScrollbarRanges();
  }

  void AList::RecalcLineHeight() {
    if(!mEnginePtr) {
      mLineHeight = mFontSize + mLineSpacing;
      return;
    }
    FT_Face face = mEnginePtr->GetDefaultFontFace();
    if(!face) {
      mLineHeight = mFontSize + mLineSpacing;
      return;
    }
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    int32_t ascender = static_cast<int32_t>(face->size->metrics.ascender >> 6);
    int32_t descender = static_cast<int32_t>((-face->size->metrics.descender) >> 6);
    int32_t height = ascender + descender;
    if(height < static_cast<int32_t>(mFontSize))
      height = static_cast<int32_t>(mFontSize);
    mLineHeight = static_cast<uint32_t>(height) + mLineSpacing;
    if(mLineHeight < mFontSize + mLineSpacing)
      mLineHeight = mFontSize + mLineSpacing;
  }

  size_t AList::IndexFromY(int32_t y) const {
    if(y < 0)
      return 0;
    uint32_t line = static_cast<uint32_t>(y) / mLineHeight;
    if(line >= mData.size())
      return mData.size() - 1;
    return line;
  }

  int32_t AList::GetLineTop(size_t index) const {
    return static_cast<int32_t>(index * mLineHeight);
  }

  void AList::SetFontSize(uint32_t size) {
    if(mFontSize == size)
      return;
    mFontSize = size;
// Invalidate text metrics cache so that future DrawTextOffset calls recompute
    mTextMetricsValid = false;
    RecalcLineHeight();// updates mLineHeight
    RecalcMaxWidth();// updates mMaxWidthPx
// Clamp scroll offsets to new valid ranges
    int32_t maxY =
        (static_cast<int32_t>(mData.size()) * static_cast<int32_t>(mLineHeight) > static_cast<int32_t>(mSizeY)) ?
            static_cast<int32_t>(mData.size() * mLineHeight - mSizeY) : 0;
    if(mVOffset > maxY)
      mVOffset = maxY;
    int32_t maxX = (mMaxWidthPx > mSizeX) ? static_cast<int32_t>(mMaxWidthPx - mSizeX) : 0;
    if(mHOffset > maxX)
      mHOffset = maxX;
    UpdateScrollbarRanges();// updates scrollbar min/max/value
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AList::Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY) const {
// ------------------------------------------------------------------
// 1. Background
// ------------------------------------------------------------------
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    int32_t drawW = static_cast<int32_t>(mSizeX);
    int32_t drawH = static_cast<int32_t>(mSizeY);
    int32_t pW = static_cast<int32_t>(parentWidth);
    int32_t pH = static_cast<int32_t>(parentHeight);
// Clip to parent
    if(absX < 0) {
      drawW += absX;
      absX = 0;
    }
    if(absY < 0) {
      drawH += absY;
      absY = 0;
    }
    if(absX + drawW > pW)
      drawW = pW - absX;
    if(absY + drawH > pH)
      drawH = pH - absY;
    if(drawW > 0 && drawH > 0 && absX >= 0 && absY >= 0) {
      uint32_t bg = mBGColor & 0x00FFFFFFU;
      size_t maxIdx = static_cast<size_t>(pW) * static_cast<size_t>(pH);
      for(int32_t row = 0; row < drawH; ++row) {
        size_t lineStart = static_cast<size_t>(absY + row) * static_cast<size_t>(pW) + static_cast<size_t>(absX);
        for(int32_t col = 0; col < drawW; ++col) {
          size_t idx = lineStart + static_cast<size_t>(col);
          if(idx < maxIdx)
            buffer[idx] = bg;
        }
      }
    }
// ------------------------------------------------------------------
// 2. Border
// ------------------------------------------------------------------
    DrawBorder(buffer, parentWidth, parentHeight, offsetX, offsetY);
    if(mData.empty())
      return;
    int32_t clientX = absX;
    int32_t clientY = absY;
    int32_t clientW = static_cast<int32_t>(mSizeX);
    int32_t clientH = static_cast<int32_t>(mSizeY);
    if(mVScrollBar && mVScrollBar->IsVisible())
      clientW -= static_cast<int32_t>(mVScrollBar->SizeX());
    if(mHScrollBar && mHScrollBar->IsVisible())
      clientH -= static_cast<int32_t>(mHScrollBar->SizeY());
    if(clientW <= 0 || clientH <= 0)
      return;
// ------------------------------------------------------------------
// 4. Vertical alignment – only applied when content fits
// ------------------------------------------------------------------
    int32_t contentHeight = static_cast<int32_t>(mData.size() * mLineHeight);
    int32_t contentStartY = 0;
    if(contentHeight <= clientH) {
// Content fits inside client area: use static alignment
      if(mVAlign == AUIVAlign::center) {
        contentStartY = (clientH - contentHeight) / 2;
      }
      else if(mVAlign == AUIVAlign::bottom) {
        contentStartY = clientH - contentHeight;
      }
// mVOffset should be 0 here; if not, force it (optional safety)
// const_cast<AList*>(this)->mVOffset = 0;
    }
    else {
// Content overflows: no static offset – alignment is achieved via mVOffset
      contentStartY = 0;
// mVOffset must be set externally (e.g., by RecalcScrollFromAlignment)
    }
// ------------------------------------------------------------------
// 5. Get FreeType face
// ------------------------------------------------------------------
    FT_Face face = mEnginePtr ? mEnginePtr->GetDefaultFontFace() : nullptr;
// ------------------------------------------------------------------
// 6. Draw each visible line
// ------------------------------------------------------------------
    for(size_t i = 0; i < mData.size(); ++i) {
      int32_t lineTop = contentStartY + static_cast<int32_t>(i * mLineHeight) - mVOffset;
      int32_t lineBottom = lineTop + static_cast<int32_t>(mLineHeight);
      if(lineBottom < 0)
        continue;
      if(lineTop >= clientH)
        break;
// Clip visible portion
      int32_t visibleStartY = std::max(lineTop, 0);
      int32_t visibleEndY = std::min(lineBottom, clientH);
      int32_t visibleHeight = visibleEndY - visibleStartY;
      if(visibleHeight <= 0)
        continue;
// Selection background
      if(mTag[i]) {
        uint32_t selColor = ShiftColor(mBGColor, false);
        int32_t startY = clientY + visibleStartY;
        int32_t endY = clientY + visibleEndY;
        size_t maxIdx = static_cast<size_t>(pW) * static_cast<size_t>(pH);
        for(int32_t row = startY; row < endY; ++row) {
          if(row < 0 || row >= pH)
            continue;
          size_t lineStart = static_cast<size_t>(row) * static_cast<size_t>(pW) + static_cast<size_t>(clientX);
          for(int32_t col = 0; col < clientW; ++col) {
            size_t idx = lineStart + static_cast<size_t>(col);
            if(idx < maxIdx)
              buffer[idx] = selColor;
          }
        }
      }
// Draw text
      if(face) {
        int32_t effectiveWidth = (int32_t)std::max(mMaxWidthPx, static_cast<uint32_t>(clientW));
        DrawTextEx(buffer, parentWidth, parentHeight, clientX, clientY + visibleStartY, clientW, visibleHeight,
            mData[i], face, mFontSize, mHAlign, mLineTextVAlign, mHOffset, mTextColor, effectiveWidth);
      }
    }
// ------------------------------------------------------------------
// 7. Draw scrollbars
// ------------------------------------------------------------------
    if(mVScrollBar && mVScrollBar->IsVisible()) {
      int32_t sbX = absX + static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mVScrollBar->SizeX());
      int32_t sbY = absY;
      int32_t sbH = static_cast<int32_t>(mSizeY);
      if(mHScrollBar && mHScrollBar->IsVisible())
        sbH -= static_cast<int32_t>(mHScrollBar->SizeY());
      if(sbH > 0) {
        const_cast<AScrollBar*>(mVScrollBar.get())->Move(sbX, sbY);
        const_cast<AScrollBar*>(mVScrollBar.get())->Resize(mVScrollBar->SizeX(), static_cast<uint32_t>(sbH));
        const_cast<AScrollBar*>(mVScrollBar.get())->Draw(buffer, parentWidth, parentHeight, 0, 0);
      }
    }
    if(mHScrollBar && mHScrollBar->IsVisible()) {
      int32_t sbX = absX;
      int32_t sbY = absY + static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mHScrollBar->SizeY());
      int32_t sbW = static_cast<int32_t>(mSizeX);
      if(mVScrollBar && mVScrollBar->IsVisible())
        sbW -= static_cast<int32_t>(mVScrollBar->SizeX());
      if(sbW > 0) {
        const_cast<AScrollBar*>(mHScrollBar.get())->Move(sbX, sbY);
        const_cast<AScrollBar*>(mHScrollBar.get())->Resize(static_cast<uint32_t>(sbW), mHScrollBar->SizeY());
        const_cast<AScrollBar*>(mHScrollBar.get())->Draw(buffer, parentWidth, parentHeight, 0, 0);
      }
    }
  }

  void AList::OnMouseMove(int32_t localX, int32_t localY) {
// If a scrollbar is being dragged, forward all mouse moves to it
    if(mDragScrollbar) {
      int32_t sbX = mDragScrollbar->X();
      int32_t sbY = mDragScrollbar->Y();
      int32_t sbLocalX = localX + mX - sbX;
      int32_t sbLocalY = localY + mY - sbY;
      mDragScrollbar->OnMouseMove(sbLocalX, sbLocalY);
      mParentWindow->Draw();
      return;
    }

// No active drag: forward hover events only when cursor is inside a scrollbar
    if(mVScrollBar) {
      int32_t sbX = mVScrollBar->X();
      int32_t sbY = mVScrollBar->Y();
      if(localX + mX >= sbX && localX + mX < sbX + static_cast<int32_t>(mVScrollBar->SizeX()) && localY + mY >= sbY
          && localY + mY < sbY + static_cast<int32_t>(mVScrollBar->SizeY())) {
        int32_t sbLocalX = localX + mX - sbX;
        int32_t sbLocalY = localY + mY - sbY;
        mVScrollBar->OnMouseMove(sbLocalX, sbLocalY);
      }
    }
    if(mHScrollBar) {
      int32_t sbX = mHScrollBar->X();
      int32_t sbY = mHScrollBar->Y();
      if(localX + mX >= sbX && localX + mX < sbX + static_cast<int32_t>(mHScrollBar->SizeX()) && localY + mY >= sbY
          && localY + mY < sbY + static_cast<int32_t>(mHScrollBar->SizeY())) {
        int32_t sbLocalX = localX + mX - sbX;
        int32_t sbLocalY = localY + mY - sbY;
        mHScrollBar->OnMouseMove(sbLocalX, sbLocalY);
      }
    }
  }

  bool AList::OnMouseClick(int32_t localX, int32_t localY, bool pressed) {
    auto forwardToScrollbar = [&](AScrollBar *sb) -> bool {
      if(!sb || !sb->IsVisible())
        return false;// 👈 key change
      int32_t sbX = sb->X();
      int32_t sbY = sb->Y();
      if(localX + mX >= sbX && localX + mX < sbX + static_cast<int32_t>(sb->SizeX()) && localY + mY >= sbY
          && localY + mY < sbY + static_cast<int32_t>(sb->SizeY())) {
        int32_t sbLocalX = localX + mX - sbX;
        int32_t sbLocalY = localY + mY - sbY;
        bool ret = sb->OnMouseClick(sbLocalX, sbLocalY, pressed);
        if(pressed && ret) {
          mDragScrollbar = sb;
        }
        else if(!pressed && mDragScrollbar == sb) {
          mDragScrollbar = nullptr;
        }
        return ret;
      }
      return false;
    };
    if(forwardToScrollbar(mVScrollBar.get()))
      return true;
    if(forwardToScrollbar(mHScrollBar.get()))
      return true;
// 3. RELEASE case: if we have an active drag scrollbar but the release
//    happened outside its area, still forward the release to it.
    if(!pressed && mDragScrollbar) {
      int32_t sbX = mDragScrollbar->X();
      int32_t sbY = mDragScrollbar->Y();
// Use the stored scrollbar's position; coordinates may be outside, but that's fine
      int32_t sbLocalX = localX + mX - sbX;
      int32_t sbLocalY = localY + mY - sbY;
      mDragScrollbar->OnMouseClick(sbLocalX, sbLocalY, false);
      mDragScrollbar = nullptr;
      return true;
    }
// 4. No scrollbar involved – handle list item selection (only on press)
    if(!pressed)
      return false;
    int32_t y = localY + mVOffset;
    if(y < 0)
      return false;
    size_t idx = IndexFromY(y);
    if(idx >= mData.size())
      return false;
    if(mMultiSelect) {
      SelectIndex(idx, !IsSelected(idx));
    }
    else {
      SelectIndex(idx, true);
    }
    return true;
  }

  void AList::OnMouseWheel(int32_t delta) {
// delta > 0 = scroll up (away from user), delta < 0 = scroll down
    int32_t scrollAmount = delta * static_cast<int32_t>(mLineHeight);
    int32_t newVOffset = mVOffset - scrollAmount;
    ScrollToOffset(mHOffset, newVOffset);
    mParentWindow->Draw();
  }

  void AList::OnParentResize(UNUSED uint32_t newWidth, UNUSED uint32_t newHeight) {
    if(!mScrollbarsEnabled)
      return;
// Position vertical scrollbar at the right edge of the list
    if(mVScrollBar) {
      mVScrollBar->Move(mX + static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mVScrollBar->SizeX()), mY);
      mVScrollBar->Resize(mVScrollBar->SizeX(), mSizeY);
    }
// Position horizontal scrollbar at the bottom edge
    if(mHScrollBar) {
      mHScrollBar->Move(mX, mY + static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mHScrollBar->SizeY()));
      mHScrollBar->Resize(mSizeX, mHScrollBar->SizeY());
    }
    UpdateScrollbarRanges();
  }

  void AList::SetLineSpacing(uint32_t spacing) {
    if(mLineSpacing == spacing)
      return;
    mLineSpacing = spacing;
    RecalcLineHeight();
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AList::SetScrollbarsEnabled(bool enable) {
    if(mScrollbarsEnabled == enable)
      return;
    mScrollbarsEnabled = enable;
    if(enable) {
      if(!mVScrollBar) {
        mVScrollBar = std::make_unique<AScrollBar>();
        mVScrollBar->SetOrientation(AUIOrientation::vertical);
        mVScrollBar->mParentWindow = mParentWindow;
        mVScrollBar->mEnginePtr = mEnginePtr;
        mVScrollBar->Resize(16, mSizeY);
        mVScrollBar->ShowArrows(true);
        mVScrollBar->SetArrowSize(12);
        mVScrollBar->SetTrackThickness(6);
        mVScrollBar->SetThumbThickness(8);
        mVScrollBar->SetPosition(mX + static_cast<int32_t>(mSizeX) - 16, mY);
        mVScrollBar->SetScrollCallback([](AWindow*, AWidget*, void *data, int32_t val) {
          AList *list = static_cast<AList*>(data);
          D2("Scrollbar callback: val={}, current mVOffset={}", val, list->mVOffset);
          list->ScrollToOffset(list->mHOffset, val);
        }, this);
      }
      if(!mHScrollBar) {
        mHScrollBar = std::make_unique<AScrollBar>();
        mHScrollBar->SetOrientation(AUIOrientation::horizontal);
        mHScrollBar->mParentWindow = mParentWindow;
        mHScrollBar->mEnginePtr = mEnginePtr;
        mHScrollBar->Resize(mSizeX, 16);
        mHScrollBar->ShowArrows(true);
        mHScrollBar->SetArrowSize(12);
        mHScrollBar->SetTrackThickness(6);
        mHScrollBar->SetThumbThickness(8);
        mHScrollBar->SetPosition(mX, mY + static_cast<int32_t>(mSizeY) - 16);
        mHScrollBar->SetScrollCallback([](AWindow*, AWidget*, void *data, int32_t val) {
          AList *list = static_cast<AList*>(data);
          list->ScrollToOffset(val, list->mVOffset);
        }, this);
      }
    }
    else {
      mVScrollBar.reset();
      mHScrollBar.reset();
    }
    UpdateScrollbarRanges();// this will set visibility if auto‑hide is on
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AList::SetVScrollbarArrowSize(uint32_t size) {
    if(mVScrollBar) {
      mVScrollBar->SetArrowSize(size);
      if(mParentWindow)
        mParentWindow->Draw();
    }
  }

  void AList::SetHScrollbarArrowSize(uint32_t size) {
    if(mHScrollBar) {
      mHScrollBar->SetArrowSize(size);
      if(mParentWindow)
        mParentWindow->Draw();
    }
  }

  void AList::ScrollToOffset(int32_t xOffset, int32_t yOffset) {
// Compute visible width (exclude vertical scrollbar if visible)
    int32_t viewWidth = static_cast<int32_t>(mSizeX);
    if(mVScrollBar && mVScrollBar->IsVisible())
      viewWidth -= static_cast<int32_t>(mVScrollBar->SizeX());
// Compute visible height (exclude horizontal scrollbar if visible)
    int32_t viewHeight = static_cast<int32_t>(mSizeY);
    if(mHScrollBar && mHScrollBar->IsVisible())
      viewHeight -= static_cast<int32_t>(mHScrollBar->SizeY());
// Compute max horizontal scroll (pixels)
    int32_t maxX = 0;
    if(mMaxWidthPx > static_cast<uint32_t>(viewWidth))
      maxX = static_cast<int32_t>((int32_t) mMaxWidthPx - (int32_t) viewWidth);
// Compute max vertical scroll (pixels)
    uint32_t contentHeight = static_cast<uint32_t>(mData.size()) * mLineHeight;
    int32_t maxY = 0;
    if(contentHeight > static_cast<uint32_t>(viewHeight))
      maxY = static_cast<int32_t>((int32_t) contentHeight - viewHeight);
    xOffset = std::max(0, std::min(xOffset, maxX));
    yOffset = std::max(0, std::min(yOffset, maxY));
    if(mHOffset == xOffset && mVOffset == yOffset)
      return;
    mHOffset = xOffset;
    mVOffset = yOffset;
    if(mVScrollBar)
      mVScrollBar->SetValue(mVOffset);
    if(mHScrollBar)
      mHScrollBar->SetValue(mHOffset);
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AList::UpdateScrollbarRanges() {
// Start with full widget dimensions
    uint32_t viewWidth = mSizeX;
    uint32_t viewHeight = mSizeY;
// ----- Step 1: Determine scrollbar needs (first pass) -----
    uint32_t totalLines = static_cast<uint32_t>(mData.size());
    uint32_t visibleLines = (mLineHeight > 0) ? (viewHeight + mLineHeight - 1) / mLineHeight : 1;
    bool needVScroll = (totalLines > visibleLines);
    bool needHScroll = (mMaxWidthPx > viewWidth);
// Adjust view size if opposite scrollbar will be shown
    if(needVScroll && mVScrollBar)
      viewWidth -= mVScrollBar->SizeX();
    if(needHScroll && mHScrollBar)
      viewHeight -= mHScrollBar->SizeY();
// Recompute vertical need (because viewHeight may have changed)
    visibleLines = (mLineHeight > 0) ? (viewHeight + mLineHeight - 1) / mLineHeight : 1;
    needVScroll = (totalLines > visibleLines);
// (Optional: second pass for width if vertical scrollbar now needed, but omitted for brevity)
// ----- Step 2: Update visibility -----
    if(mVScrollBar) {
      mVScrollBar->ShowArrows(needVScroll);
      mVScrollBar->SetVisible(mAutoHideScrollbars ? needVScroll : true);
    }
    if(mHScrollBar) {
      mHScrollBar->ShowArrows(needHScroll);
      mHScrollBar->SetVisible(mAutoHideScrollbars ? needHScroll : true);
    }
// ----- Step 3: Set vertical scrollbar range (using effective viewHeight) -----
    if(mVScrollBar) {
      uint32_t contentHeight = totalLines * mLineHeight;
      int32_t maxV = (contentHeight > viewHeight) ? static_cast<int32_t>(contentHeight - viewHeight) : 0;
      mVScrollBar->SetRange(0, maxV);
      mVScrollBar->SetPageStep(static_cast<int32_t>(visibleLines * mLineHeight));
      mVScrollBar->SetSingleStep(static_cast<int32_t>(mLineHeight));
      if(mVOffset > maxV)
        mVOffset = maxV;
      mVScrollBar->SetValue(mVOffset);
    }
// ----- Step 4: Set horizontal scrollbar range (using effective viewWidth) -----
    if(mHScrollBar) {
      int32_t maxH = (mMaxWidthPx > viewWidth) ? static_cast<int32_t>(mMaxWidthPx - viewWidth) : 0;
      D3("UpdateRanges: viewWidth={}, maxWidth={}, maxH={}, current mHOffset={}", viewWidth, mMaxWidthPx, maxH,
          mHOffset);
      mHScrollBar->SetRange(0, maxH);
      mHScrollBar->SetPageStep(static_cast<int32_t>(viewWidth / 2));
      mHScrollBar->SetSingleStep(20);
      if(mHOffset > maxH)
        mHOffset = maxH;
      D3("Before SetValue, mHOffset={}", mHOffset);
      mHScrollBar->SetValue(mHOffset);
    }
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AList::DrawItem(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t absX, int32_t absY,
      size_t index, bool isSelected) const {
// Selection background
    if(isSelected) {
      uint32_t selColor = ShiftColor(mBGColor, false);// lighten background for selection
      uint32_t w = mSizeX;
      uint32_t h = mLineHeight;
// Clip to widget area
      int32_t startX = absX;
      int32_t startY = absY + GetLineTop(index) - mVOffset;
      if(startY + static_cast<int32_t>(h) < absY || startY > absY + static_cast<int32_t>(mSizeY))
        return;
// Draw selection background
      for(uint32_t row = 0; row < h; ++row) {
        int32_t destY = startY + static_cast<int32_t>(row);
        if(destY < absY || destY >= absY + static_cast<int32_t>(mSizeY))
          continue;
        size_t lineStart = static_cast<size_t>(destY) * parentWidth + static_cast<size_t>(startX);
        for(uint32_t col = 0; col < w; ++col) {
          size_t idx = lineStart + col;
          if(idx < static_cast<size_t>(parentWidth) * parentHeight)
            buffer[idx] = selColor;
        }
      }
    }
    std::string savedText = mText;// backup
    const_cast<AList*>(this)->mText = mData[index];
    uint32_t savedTextColor = mTextColor;
    const_cast<AList*>(this)->mTextColor = this->mTextColor;// same
    UNUSED int32_t lineAbsY = absY + GetLineTop(index) - mVOffset;
    const_cast<AList*>(this)->mText = savedText;
    const_cast<AList*>(this)->mTextColor = savedTextColor;
  }

  void AList::DrawScrollbars(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY) const {
    if(!mScrollbarsEnabled)
      return;
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    if(mVScrollBar && mVScrollBar->IsVisible()) {
      int32_t sbX = absX + static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mVScrollBar->SizeX());
      int32_t sbY = absY;
      int32_t sbH = static_cast<int32_t>(mSizeY);
      if(mHScrollBar && mHScrollBar->IsVisible())
        sbH -= static_cast<int32_t>(mHScrollBar->SizeY());
      if(sbH > 0) {
        const_cast<AScrollBar*>(mVScrollBar.get())->Move(sbX, sbY);
        const_cast<AScrollBar*>(mVScrollBar.get())->Resize(mVScrollBar->SizeX(), static_cast<uint32_t>(sbH));
        const_cast<AScrollBar*>(mVScrollBar.get())->Draw(buffer, parentWidth, parentHeight, 0, 0);
      }
    }
    if(mHScrollBar && mHScrollBar->IsVisible()) {
      int32_t sbX = absX;
      int32_t sbY = absY + static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mHScrollBar->SizeY());
      int32_t sbW = static_cast<int32_t>(mSizeX);
      if(mVScrollBar && mVScrollBar->IsVisible())
        sbW -= static_cast<int32_t>(mVScrollBar->SizeX());
      if(sbW > 0) {
        const_cast<AScrollBar*>(mHScrollBar.get())->Move(sbX, sbY);
        const_cast<AScrollBar*>(mHScrollBar.get())->Resize(static_cast<uint32_t>(sbW), mHScrollBar->SizeY());
        const_cast<AScrollBar*>(mHScrollBar.get())->Draw(buffer, parentWidth, parentHeight, 0, 0);
      }
    }
  }

  void AList::SetVerticalScrollbarEnabled(bool enable) {
    if(enable == IsVerticalScrollbarEnabled())
      return;
    if(enable) {
      if(!mVScrollBar) {
        mVScrollBar = std::make_unique<AScrollBar>();
        D2("SetVerticalScrollbarEnabled: enable={}, mVScrollBar={}", enable, (void*)mVScrollBar.get());
        if(mVScrollBar) {
          D2("  sb size: {}x{}, pos: ({},{})", mVScrollBar->SizeX(), mVScrollBar->SizeY(), mVScrollBar->X(),
              mVScrollBar->Y());
        }
        mVScrollBar->SetOrientation(AUIOrientation::vertical);
        mVScrollBar->ShowArrows(true);
        mVScrollBar->SetArrowSize(16);
        mVScrollBar->SetTrackThickness(8);
        mVScrollBar->SetThumbThickness(16);
        mVScrollBar->SetScrollCallback([](AWindow*, AWidget*, void *data, int32_t val) {
          AList *list = static_cast<AList*>(data);
          list->ScrollToOffset(list->mHOffset, val);
        }, this);
      }
      mVScrollBar->Resize(16, mSizeY);
      mVScrollBar->SetPosition(mX + static_cast<int32_t>(mSizeX) - 16, mY);
    }
    else {
      mVScrollBar.reset();
    }
    UpdateScrollbarRanges();
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AList::SetHorizontalScrollbarEnabled(bool enable) {
    if(enable == IsHorizontalScrollbarEnabled())
      return;
    if(enable) {
      if(!mHScrollBar) {
        mHScrollBar = std::make_unique<AScrollBar>();
        mHScrollBar->SetOrientation(AUIOrientation::horizontal);
        mHScrollBar->ShowArrows(true);
        mHScrollBar->SetArrowSize(16);
        mHScrollBar->SetTrackThickness(8);
        mHScrollBar->SetThumbThickness(16);
        mHScrollBar->SetScrollCallback([](AWindow*, AWidget*, void *data, int32_t val) {
          AList *list = static_cast<AList*>(data);
          list->ScrollToOffset(val, list->mVOffset);
        }, this);
      }
      mHScrollBar->Resize(mSizeX, 16);
      mHScrollBar->SetPosition(mX, mY + static_cast<int32_t>(mSizeY) - 40);
    }
    else {
      mHScrollBar.reset();
    }
    UpdateScrollbarRanges();
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AList::SetAutoHideScrollbars(bool enable) {
    if(mAutoHideScrollbars == enable)
      return;
    mAutoHideScrollbars = enable;
    UpdateScrollbarRanges();
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AList::RecalcScrollFromAlignment() {
// ----- Vertical alignment -----
    int32_t maxH = 0;
    int32_t viewHeight = static_cast<int32_t>(mSizeY);
    if(mHScrollBar && mHScrollBar->IsVisible())
      viewHeight -= static_cast<int32_t>(mHScrollBar->SizeY());
    uint32_t contentHeight = static_cast<uint32_t>(mData.size()) * mLineHeight;
    if(contentHeight <= static_cast<uint32_t>(viewHeight)) {
      mVOffset = 0;
    }
    else {
      int32_t maxV = static_cast<int32_t>((int32_t) contentHeight - viewHeight);
      switch (mVAlign) {
        case AUIVAlign::top:
          mVOffset = 0;
          break;
        case AUIVAlign::center:
          mVOffset = maxV / 2;
          break;
        case AUIVAlign::bottom:
          mVOffset = maxV;
          break;
        default:
          E("garbage")
          ;
          break;
      }
      mVOffset = std::max(0, std::min(mVOffset, maxV));
    }
    if(mVScrollBar)
      mVScrollBar->SetValue(mVOffset);
// ----- Horizontal alignment -----
    int32_t viewWidth = static_cast<int32_t>(mSizeX);
    if(mVScrollBar && mVScrollBar->IsVisible())
      viewWidth -= static_cast<int32_t>(mVScrollBar->SizeX());
    if(mMaxWidthPx <= static_cast<uint32_t>(viewWidth)) {
      mHOffset = 0;
    }
    else {
      maxH = static_cast<int32_t>((int32_t) mMaxWidthPx - viewWidth);
      switch (mHAlign) {
        case AUIHAlign::left:
          mHOffset = 0;
          break;
        case AUIHAlign::center:
          mHOffset = maxH / 2;
          break;
        case AUIHAlign::right:
          mHOffset = maxH;
          break;
        default:
          E("garbage")
          ;
          break;
      }
      mHOffset = std::max(0, std::min(mHOffset, maxH));
    }
    D2("Recalc: viewWidth={}, maxWidth={}, maxH={}, mHOffset={}", viewWidth, mMaxWidthPx, maxH, mHOffset);
    if(mHScrollBar) {
      D2("RecalcH: viewWidth={}, maxWidth={}, mHAlign={}, mHOffset={}", viewWidth, mMaxWidthPx, maxH,
          static_cast<int32_t>(mHAlign), mHOffset);
      mHScrollBar->SetValue(mHOffset);
      D2("After SetValue, scrollbar value={}", mHScrollBar->GetValue());
    }
  }

  void AList::SetVAlignment(AUIVAlign align) {
    if(mVAlign == align)
      return;
    mVAlign = align;// set directly (or call base but without drawing)
    RecalcScrollFromAlignment();
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AList::ScrollToItem(size_t index, bool alignCenter) {
    if(index >= mData.size())
      return;
// Get item's top and bottom pixel positions relative to unscrolled content
    int32_t itemTop = static_cast<int32_t>(index * mLineHeight);
    int32_t itemBottom = itemTop + static_cast<int32_t>(mLineHeight);
// Compute visible area dimensions (client area)
    int32_t viewHeight = static_cast<int32_t>(mSizeY);
    if(mHScrollBar && mHScrollBar->IsVisible())
      viewHeight -= static_cast<int32_t>(mHScrollBar->SizeY());
    if(alignCenter) {
// Center the item in the viewport
      int32_t targetOffset = itemTop - (viewHeight - static_cast<int32_t>(mLineHeight)) / 2;
      ScrollToOffset(mHOffset, targetOffset);
    }
    else {
// Ensure item is fully visible, scrolling minimally
      int32_t currentTop = mVOffset;
      int32_t currentBottom = currentTop + viewHeight;
      if(itemTop < currentTop) {
// Item is above visible area
        ScrollToOffset(mHOffset, itemTop);
      }
      else if(itemBottom > currentBottom) {
// Item is below visible area
        ScrollToOffset(mHOffset, itemBottom - viewHeight);
      }
// else already visible – no change
    }
  }

  void AList::SetVScrollbarColors(uint32_t track, uint32_t thumb) {
    if(mVScrollBar) {
      mVScrollBar->SetTrackColor(track);
      mVScrollBar->SetThumbColor(thumb);
      if(mParentWindow)
        mParentWindow->Draw();
    }
  }

  void AList::SetHScrollbarColors(uint32_t track, uint32_t thumb) {
    if(mHScrollBar) {
      mHScrollBar->SetTrackColor(track);
      mHScrollBar->SetThumbColor(thumb);
      if(mParentWindow)
        mParentWindow->Draw();
    }
  }

  void AList::SetHAlignment(AUIHAlign align) {
    D2("SetHAlignment called: old={}, new={}", static_cast<int32_t>(mHAlign), static_cast<int32_t>(align));
    if(mHAlign == align)
      return;
    AWidget::SetHAlignment(align);// stores and redraws
    RecalcScrollFromAlignment();// adjust horizontal offset
    if(mParentWindow)
      mParentWindow->Draw();
  }

  AScrollBar* AList::GetVScrollBar() {
    return mVScrollBar.get();
  }

  void AList::SetMultiSelect(bool enable) {
    if(mMultiSelect == enable) {
      E("already it that selection mode")
    }
    mMultiSelect = enable;
    ClearSelection();
  }

  AList::~AList() {
    D2("AList destructor");
  }

}// namespace aui
