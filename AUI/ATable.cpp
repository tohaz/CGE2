#include "AUILib.h"

namespace aui {

  struct ATableRangeData1 {
      int64_t cell = -1;
      int64_t offset = -1;
  };

  struct ATableRangeData2 {
      int64_t cell = -1;
      int64_t offset = -1;
      int64_t cell2 = -1;
      int64_t offset2 = -1;
  };

  ATable::ATable() {
    mWidgetType = AUIWidgetType::defaultTable;
    mBGColor = 0xFFEEEEEE;
    mSizeX = 300;
    mSizeY = 200;
    mHAlign = AUIHAlign::left;
    mVAlign = AUIVAlign::top;
    mResizeHover = false;
    mResizeHoverId = -1;
    mResizeHoverColumn = false;
    SetFocusable(true);
  }

  ATable* ATable::AttachTo(AWindow *parent) {
    if(!parent)
      return nullptr;
    ATable *tbl = new ATable();
    parent->AddWidget(std::unique_ptr<AWidget>(tbl));
    return tbl;
  }

  ATable* ATable::AttachTo(AWidget *parent) {
    if(!parent)
      return nullptr;
    ATable *tbl = new ATable();
    parent->AddWidget(std::unique_ptr<AWidget>(tbl));
    return tbl;
  }

  int32_t ATable::MeasureTextWidth(const std::string &text) const {
    if(!mEnginePtr)
      return 0;
    std::unique_lock lock(mEnginePtr->GetFontMutex(), std::chrono::milliseconds(50));
    if (!lock.owns_lock()) { E("locked"); }
    FT_Face face = mEnginePtr->GetDefaultFontFace();
    if(!face)
      return 0;
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    int32_t width = 0;
    for(char c : text) {
      if(FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_DEFAULT) == 0)
        width += SafeINT32(face->glyph->advance.x) >> 6;
    }
    return width;
  }

  ATableRangeData1 ATable::Offset2Row(int64_t offset) const {
// Clamp negative offsets to 0
    if(offset < 0)
      offset = 0;

// If no rows, return invalid
    if(mRowH.empty())
      return {-1, -1};
// Rebuild prefix if dirty
    if(mRowPrefixDirty)
      RebuildRowPrefix();
// Binary search for first prefix > offset
    auto it = std::upper_bound(mRowPrefix.begin(), mRowPrefix.end(), offset);
// Case: offset is beyond total content height
    if(it == mRowPrefix.end()) {
// Return the last row with its full height as the inside offset
      int64_t lastId = mRowIds.back();
      auto rowIt = mRowH.find(lastId);
      if(rowIt != mRowH.end()) {
        return {lastId, rowIt->second.first};// inside offset = full height
      }
      return {-1, -1};
    }

// Normal case: find the row containing this offset
    size_t idx = static_cast<size_t>(it - mRowPrefix.begin()) - 1;
// Safety check for index bounds
    if(idx >= mRowIds.size()) {
// Fallback: linear scan or return last row
      auto last = mRowH.rbegin();
      return {last->first, last->second.first};
    }
    int64_t rowId = mRowIds[idx];
    int64_t offsetInside = offset - mRowPrefix[idx];
    return {rowId, offsetInside};
  }

  ATableRangeData1 ATable::Offset2Column(int64_t offset) const {
    if(mColumnW.empty())
      return {-1, -1};
    if(offset < 0)
      offset = 0;
    if(mColPrefixDirty)
      RebuildColPrefix();
// Binary search for first prefix > offset
    auto it = std::upper_bound(mColPrefix.begin(), mColPrefix.end(), offset);
    if(it == mColPrefix.end()) {
// offset >= total width: return last column with full offset inside it
      int64_t lastId = mColIds.back();
      return {lastId, mColumnW.find(lastId)->second.first};
    }
    size_t idx = static_cast<size_t>((it - mColPrefix.begin()) - 1);
// Safety: idx should be < mColIds.size()
    if(idx >= mColIds.size()) {
// Fallback to linear scan or return last column
      auto last = mColumnW.rbegin();
      return {last->first, last->second.first};
    }
    int64_t colId = mColIds[idx];
    int64_t offsetInside = offset - mColPrefix[idx];
    return {colId, offsetInside};
  }

  ATableRangeData1 ATable::Offset2RowRange(const ATableRangeData1 &start, int64_t height) const {
    if(start.cell < 0 || mRowH.empty())
      return start;
    if(mRowPrefixDirty)
      RebuildRowPrefix();
// Find start index in mRowIds via binary search (O(log R))
    auto idIt = std::lower_bound(mRowIds.begin(), mRowIds.end(), start.cell);
    if(idIt == mRowIds.end())
      return start;
    size_t startIdx = static_cast<size_t>(idIt - mRowIds.begin());
    int64_t startY = mRowPrefix[startIdx];
    int64_t targetY = startY + start.offset + height - mColumnHeaderHeight;
    if(targetY < startY) {
// Height is too small to even show the current cell fully? Return start as is.
      return start;
    }
// Binary search for first prefix > targetY
    auto upper = std::upper_bound(mRowPrefix.begin(), mRowPrefix.end(), targetY);
    size_t endIdx = static_cast<size_t>(upper - mRowPrefix.begin()) - 1;
    if(endIdx >= mRowIds.size())
      endIdx = mRowIds.size() - 1;
    int64_t endRowId = mRowIds[endIdx];
    return {endRowId, -1};// offset2 unused
  }

  ATableRangeData1 ATable::Offset2ColumnRange(const ATableRangeData1 &start, int64_t width) const {
    if(start.cell < 0 || mColumnW.empty())
      return start;
    if(mColPrefixDirty)
      RebuildColPrefix();
    auto idIt = std::lower_bound(mColIds.begin(), mColIds.end(), start.cell);
    if(idIt == mColIds.end())
      return start;
    size_t startIdx = static_cast<size_t>(idIt - mColIds.begin());
    int64_t startX = mColPrefix[startIdx];
    int64_t targetX = startX + start.offset + width - mRowHeaderWidth;
    if(targetX < startX)
      return start;
    auto upper = std::upper_bound(mColPrefix.begin(), mColPrefix.end(), targetX);
    size_t endIdx = static_cast<size_t>(upper - mColPrefix.begin() - 1);
    if(endIdx >= mColIds.size())
      endIdx = mColIds.size() - 1;
    int64_t endColId = mColIds[endIdx];
    return {endColId, -1};
  }

  void ATable::Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY) const {
    if(!mEnginePtr) {
      E()
      return;
    }
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    int32_t bgX = absX, bgY = absY, bgW = static_cast<int32_t>(mSizeX), bgH = static_cast<int32_t>(mSizeY);
    ClipRect(bgX, bgY, bgW, bgH, static_cast<int32_t>(parentWidth), static_cast<int32_t>(parentHeight));
    DrawBorder(buffer, parentWidth, parentHeight, offsetX, offsetY);
    ATableRangeData1 colStart = Offset2Column(mHOffset);
    ATableRangeData1 colEnd = Offset2ColumnRange(colStart, static_cast<int64_t>(mSizeX));
    ATableRangeData1 rowStart = Offset2Row(mVOffset);
    ATableRangeData1 rowEnd = Offset2RowRange(rowStart, static_cast<int64_t>(mSizeY));
    DrawCells(buffer, parentWidth, parentHeight, offsetX, offsetY,
        { rowStart.cell, rowStart.offset, rowEnd.cell, rowEnd.offset }, { colStart.cell, colStart.offset, colEnd.cell,
            colEnd.offset });
    DrawColumnHeader(buffer, parentWidth, parentHeight, offsetX, offsetY, colStart, colEnd);
    DrawRowHeader(buffer, parentWidth, parentHeight, offsetX, offsetY, rowStart, rowEnd);
    DrawIntersectionBox(buffer, parentWidth, parentHeight, offsetX, offsetY);
    if(mVScrollBar && mVScrollBar->IsVisible()) {
      int32_t sbX = absX + static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mVScrollBar->SizeX());
      int32_t sbY = absY + static_cast<int32_t>(mColumnHeaderHeight);
      int32_t sbH = static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mColumnHeaderHeight);
      if(mHScrollBar && mHScrollBar->IsVisible())
        sbH -= static_cast<int32_t>(mHScrollBar->SizeY());
      if(sbH > 0) {
        if(mVScrollBar->X() != sbX || mVScrollBar->Y() != sbY)
          const_cast<AScrollBar*>(mVScrollBar.get())->Move(sbX, sbY);
        if(mVScrollBar->SizeY() != static_cast<uint32_t>(sbH))
          const_cast<AScrollBar*>(mVScrollBar.get())->Resize(mVScrollBar->SizeX(), static_cast<uint32_t>(sbH));
        const_cast<AScrollBar*>(mVScrollBar.get())->Draw(buffer, parentWidth, parentHeight, 0, 0);
      }
    }
    if(mHScrollBar && mHScrollBar->IsVisible()) {
      int32_t sbX = absX + static_cast<int32_t>(mRowHeaderWidth);
      int32_t sbY = absY + static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mHScrollBar->SizeY());
      int32_t sbW = static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mRowHeaderWidth);
      if(mVScrollBar && mVScrollBar->IsVisible())
        sbW -= static_cast<int32_t>(mVScrollBar->SizeX());
      if(sbW > 0) {
        if(mHScrollBar->X() != sbX || mHScrollBar->Y() != sbY)
          const_cast<AScrollBar*>(mHScrollBar.get())->Move(sbX, sbY);
        if(mHScrollBar->SizeX() != static_cast<uint32_t>(sbW))
          const_cast<AScrollBar*>(mHScrollBar.get())->Resize(static_cast<uint32_t>(sbW), mHScrollBar->SizeY());
        const_cast<AScrollBar*>(mHScrollBar.get())->Draw(buffer, parentWidth, parentHeight, 0, 0);
      }
    }
  }

  void ATable::DrawCells(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY, const ATableRangeData2 &rowRange, const ATableRangeData2 &colRange) const {
    if(!mEnginePtr)
      return;
    int32_t clientAbsX = offsetX + mX + static_cast<int32_t>(mRowHeaderWidth);
    int32_t clientAbsY = offsetY + mY + static_cast<int32_t>(mColumnHeaderHeight);
    int32_t clientW = static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mRowHeaderWidth);
    int32_t clientH = static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mColumnHeaderHeight);
    if(clientW <= 0 || clientH <= 0)
      return;
    std::unique_lock lock(mEnginePtr->GetFontMutex(), std::chrono::milliseconds(50));
    if (!lock.owns_lock()) { E("locked"); }

    FT_Face face = mEnginePtr->GetDefaultFontFace();
    if(!face)
      return;
// ---- Find starting row and column indices (binary search) ----
    auto rowStartIt = std::lower_bound(mRowIds.begin(), mRowIds.end(), rowRange.cell);
    if(rowStartIt == mRowIds.end())
      return;
    size_t rowIdx = static_cast<size_t>(rowStartIt - mRowIds.begin());
    auto colStartIt = std::lower_bound(mColIds.begin(), mColIds.end(), colRange.cell);
    if(colStartIt == mColIds.end())
      return;
    size_t colStartIdx = static_cast<size_t>(colStartIt - mColIds.begin());
// ---- Initial vertical position (client space) ----
// rowRange.offset is the number of pixels from the top of the starting row
// to the top of the visible client area.
    int64_t yPos = static_cast<int64_t>(clientAbsY) - rowRange.offset;
// ... (Keep the external outer layout and vector setup identical)
// ---- Iterate over existing rows in the visible range ----
    for(size_t ri = rowIdx; ri < mRowIds.size() && mRowIds[ri] <= rowRange.cell2; ++ri) {
      int64_t rowId = mRowIds[ri];
      auto rowIt = mRowH.find(rowId);
      if(rowIt == mRowH.end())
        continue;
      int64_t rowH = rowIt->second.first;
      int64_t rowTop = yPos;
      int64_t rowBottom = yPos + rowH;
      if(rowBottom < clientAbsY || rowTop > clientAbsY + clientH) {
        yPos += rowH;
        continue;
      }
      int64_t xPos = static_cast<int64_t>(clientAbsX) - colRange.offset;
// ---- Iterate over existing columns in the visible range ----
      for(size_t ci = colStartIdx; ci < mColIds.size() && mColIds[ci] <= colRange.cell2; ++ci) {
        int64_t colId = mColIds[ci];
        auto colIt = mColumnW.find(colId);
        if(colIt == mColumnW.end())
          continue;
        int64_t colW = colIt->second.first;
        int64_t colLeft = xPos;
        int64_t colRight = xPos + colW;
        if(colRight < clientAbsX || colLeft > clientAbsX + clientW) {
          xPos += colW;
          continue;
        }
        int32_t drawX = std::max(static_cast<int32_t>(colLeft), clientAbsX);
        int32_t drawY = std::max(static_cast<int32_t>(rowTop), clientAbsY);
        int32_t drawW = std::min(static_cast<int32_t>(colRight), clientAbsX + clientW) - drawX;
        int32_t drawH = std::min(static_cast<int32_t>(rowBottom), clientAbsY + clientH) - drawY;
        if(drawW <= 0 || drawH <= 0) {
          xPos += colW;
          continue;
        }
        ClipRect(drawX, drawY, drawW, drawH, static_cast<int32_t>(parentWidth), static_cast<int32_t>(parentHeight));
        uint32_t bgColor = mBGColor;
        if(mRowSelectMode && rowId == mSelectedRow)
          bgColor = mSelectionColor;
        else if(!mRowSelectMode && rowId == mCursorRow && colId == mCursorCol)
          bgColor = mSelectionColor;
        FillRect(buffer, parentWidth, drawX, drawY, drawW, drawH, bgColor);
// ---- REFACTORED: O(1) Flat Cell Fetch ----
        uint64_t cellKey = MakeCellKey(rowId, colId);
        auto cellIt = mCells.find(cellKey);
        if(cellIt != mCells.end() && !cellIt->second.data.empty()) {
          int32_t textRectX = static_cast<int32_t>(colLeft) + 2;
          int32_t textRectY = static_cast<int32_t>(rowTop);
          int32_t textRectW = static_cast<int32_t>(colW) - 4;
          int32_t textRectH = static_cast<int32_t>(rowH);
          DrawTextEx(buffer, parentWidth, parentHeight, textRectX, textRectY, textRectW, textRectH, cellIt->second.data,
              face, mFontSize, cellIt->second.hAlign, AUIVAlign::center, 0, mTextColor, textRectW);
        }
// Grid lines and Cursor border rendering logic remains untouched...
        int32_t gridX = static_cast<int32_t>(colRight - 1);
        if(gridX >= drawX && gridX < drawX + drawW)
          DrawVLine(buffer, parentWidth, gridX, drawY, drawH, mGridColor);
        int32_t gridY = static_cast<int32_t>(rowBottom - 1);
        if(gridY >= drawY && gridY < drawY + drawH)
          DrawHLine(buffer, parentWidth, drawX, gridY, drawW, mGridColor);
        if(rowId == mCursorRow && colId == mCursorCol && !mRowSelectMode) {
          DrawRectBorder(buffer, parentWidth, drawX, drawY, drawW, drawH, mCursorBorderColor);
        }
        xPos += colW;
      }
      yPos += rowH;
    }
  }

  void ATable::OnMouseMove(int32_t localX, int32_t localY) {
// ---- Resize dragging ----
    if(mResizing) {
      int32_t delta;
      int32_t newSize;
      if(mResizeColumn) {
        delta = localX - mResizeStartMouse;
        newSize = mResizeStartSize + delta;
        newSize = std::max(newSize, mResizeMinSize);
        SetColumnWidth(mResizeTargetId, newSize);
        mResizeStartMouse = localX;
        mResizeStartSize = newSize;
      }
      else {
        delta = localY - mResizeStartMouse;
        newSize = mResizeStartSize + delta;
        newSize = std::max(newSize, mResizeMinSize);
        SetRowHeight(mResizeTargetId, newSize);
        mResizeStartMouse = localY;
        mResizeStartSize = newSize;
      }
      return;
    }

// ---- Hover feedback (cursor change) ----
    bool isCol = false;
    int64_t id = -1;
    bool hit = HitTestSeparator(localX, localY, isCol, id);
    if(hit != mResizeHover || id != mResizeHoverId || isCol != mResizeHoverColumn) {
      mResizeHover = hit;
      mResizeHoverId = id;
      mResizeHoverColumn = isCol;
      if(mParentWindow) {
        if(hit) {
          mParentWindow->SetCursor(isCol ? AUICursorType::HResize : AUICursorType::VResize);
        }
        else {
          mParentWindow->SetCursor(AUICursorType::Default);
        }
      }
    }

// ---- Forward to scrollbar drag if active ----
    if(mDragScrollbar) {
      int32_t lx = localX + mX - mDragScrollbar->X();
      int32_t ly = localY + mY - mDragScrollbar->Y();
      mDragScrollbar->OnMouseMove(lx, ly);
      if(mParentWindow) {
        mParentWindow->Draw();
      }
      return;
    }
  }

  void ATable::OnMouseWheel(int32_t delta) {
    int32_t step = 30;// pixels per wheel step
    int32_t newVOffset = static_cast<int32_t>(mVOffset) - (delta * step);
    int32_t maxV = std::max(0,
        static_cast<int32_t>(mTotalContentHeight
            - (static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mColumnHeaderHeight))));
    newVOffset = std::clamp(newVOffset, 0, maxV);
    if(newVOffset != static_cast<int32_t>(mVOffset)) {
      ScrollTo(static_cast<int32_t>(mHOffset), newVOffset);
    }
  }

  void ATable::OnParentResize(UNUSED uint32_t newWidth, UNUSED uint32_t newHeight) {
    UpdateLayout();
  }

  void ATable::AddRow() {
// Look at mRowH or mRowIds to determine the next ID index safely
    int64_t newId = mRowH.empty() ? 0 : mRowH.rbegin()->first + 1;
// Explicitly seed the default heights just like before
    mRowH[newId] = { 24, std::to_string(newId) };
    mTotalContentHeight += 24;
    mRowPrefixDirty = true;
    UpdateScrollbarRanges();
  }

  void ATable::AddColumn() {
    int64_t newId = mColumnW.empty() ? 0 : mColumnW.rbegin()->first + 1;

    mColumnW[newId] = { 80, std::to_string(newId) };
    mTotalContentWidth += 80;
    mColPrefixDirty = true;
    UpdateScrollbarRanges();
  }

  void ATable::RemoveRow(int64_t rowIdx) {
    auto rowIt = mRowH.find(rowIdx);
    if(rowIt == mRowH.end())
      return;
    mTotalContentHeight -= rowIt->second.first;
    mRowH.erase(rowIt);
// FIXED: Iterate through our flat map directly using a safe erase pattern
    for(auto it = mCells.begin(); it != mCells.end();) {
      int64_t r = static_cast<int64_t>(it->first >> 32);
      if(r == rowIdx) {
        it = mCells.erase(it);
      }
      else {
        ++it;
      }
    }
    mRowPrefixDirty = true;
    UpdateScrollbarRanges();
  }

  void ATable::RemoveColumn(int64_t colIdx) {
    auto colIt = mColumnW.find(colIdx);
    if(colIt == mColumnW.end())
      return;
    mTotalContentWidth -= colIt->second.first;
    mColumnW.erase(colIt);
// FIXED: Clear matching column entries reliably regardless of layout state
    for(auto it = mCells.begin(); it != mCells.end();) {
      int64_t c = static_cast<int64_t>(it->first & 0xFFFFFFFF);
      if(c == colIdx) {
        it = mCells.erase(it);
      }
      else {
        ++it;
      }
    }
    mColPrefixDirty = true;
    UpdateScrollbarRanges();
  }

  void ATable::Clear() {
    mCells.clear();
    mRowH.clear();
    mColumnW.clear();
    mRowIds.clear();
    mColIds.clear();
    mColPrefixDirty = true;
    mRowPrefixDirty = true;
    mTotalContentHeight = 0;
    mTotalContentWidth = 0;
    mCursorRow = mCursorCol = -1;
    mSelectedRow = -1;
    if(mParentWindow)
      mParentWindow->Draw();
  }
  std::string ATable::GetCellData(int64_t row, int64_t col) const {
    uint64_t key = MakeCellKey(row, col);
    auto it = mCells.find(key);
    if(it == mCells.end()) {
      return "";
    }
    return it->second.data;
  }

  AUICellData& ATable::GetOrCreateCell(int64_t row, int64_t col) {
// Dynamically tracking row bounds just like before
    if(mRowH.find(row) == mRowH.end()) {
      mRowH[row] = { 24, std::to_string(row) };
      mTotalContentHeight += 24;
      mRowPrefixDirty = true;
    }
    if(mColumnW.find(col) == mColumnW.end()) {
      mColumnW[col] = { 80, std::to_string(col) };
      mTotalContentWidth += 80;
      mColPrefixDirty = true;
    }
    uint64_t key = MakeCellKey(row, col);
// std::unordered_map[] constructs the value if it doesn't exist yet
    return mCells[key];
  }
//
  void ATable::AutoWidenColumn(int64_t col) {
    auto colIt = mColumnW.find(col);
    if(colIt == mColumnW.end())
      return;
    int32_t maxWidth = MeasureTextWidth(colIt->second.second);// Start with header label
    for(int64_t rowId : mRowIds) {
      uint64_t key = MakeCellKey(rowId, col);
      auto cellIt = mCells.find(key);
      if(cellIt != mCells.end() && !cellIt->second.data.empty()) {
        int32_t cellW = MeasureTextWidth(cellIt->second.data);
        if(cellW > maxWidth)
          maxWidth = cellW;
      }
    }

    int64_t newWidth = maxWidth + 15;// Padding space
    mTotalContentWidth -= colIt->second.first;
    colIt->second.first = newWidth;
    mTotalContentWidth += newWidth;
    mColPrefixDirty = true;
    UpdateScrollbarRanges();
  }

  void ATable::DrawIntersectionBox(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY) const {
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    int32_t boxW = static_cast<int32_t>(mRowHeaderWidth);
    int32_t boxH = static_cast<int32_t>(mColumnHeaderHeight);
    if(boxW <= 0 || boxH <= 0)
      return;
    int32_t drawX = absX;
    int32_t drawY = absY;
    int32_t drawW = boxW;
    int32_t drawH = boxH;
    ClipRect(drawX, drawY, drawW, drawH, static_cast<int32_t>(parentWidth), static_cast<int32_t>(parentHeight));
    if(drawW <= 0 || drawH <= 0)
      return;
    uint32_t bgColor = 0xFFCCCCCC;
    FillRect(buffer, parentWidth, drawX, drawY, drawW, drawH, bgColor);
// Draw outer border (all four sides) with proper clipping
    DrawRectBorder(buffer, parentWidth, drawX, drawY, drawW, drawH, 0xFF888888);
  }

  void ATable::DrawColumnHeader(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY, const ATableRangeData1 &colStart, const ATableRangeData1 &colEnd) const {
    if(!mEnginePtr)
      return;
    int32_t headerAbsX = offsetX + mX + static_cast<int32_t>(mRowHeaderWidth);
    int32_t headerAbsY = offsetY + mY;
    int32_t headerW = static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mRowHeaderWidth);
    int32_t headerH = static_cast<int32_t>(mColumnHeaderHeight);
    if(headerW <= 0 || headerH <= 0)
      return;
    std::unique_lock lock(mEnginePtr->GetFontMutex(), std::chrono::milliseconds(50));
    if (!lock.owns_lock()) { E("locked"); }

    FT_Face face = mEnginePtr->GetDefaultFontFace();
    if(!face)
      return;
    int64_t xPos = static_cast<int64_t>(headerAbsX) - colStart.offset;
    auto it = mColumnW.find(colStart.cell);
    if(it == mColumnW.end())
      return;
    for(int64_t col = colStart.cell; col <= colEnd.cell && it != mColumnW.end(); ++it, col = it->first) {
      int64_t colW = it->second.first;
      int64_t colLeft = xPos;
      int64_t colRight = xPos + colW;
      if(colRight < headerAbsX || colLeft >= headerAbsX + headerW) {
        xPos += colW;
        continue;
      }
      int32_t drawX = static_cast<int32_t>(std::max(colLeft, static_cast<int64_t>(headerAbsX)));
      int32_t drawW = static_cast<int32_t>(std::min(colRight, static_cast<int64_t>(headerAbsX) + headerW) - drawX);
      if(drawW <= 0) {
        xPos += colW;
        continue;
      }
      int32_t drawY = headerAbsY;
      int32_t drawH = headerH;
      ClipRect(drawX, drawY, drawW, drawH, static_cast<int32_t>(parentWidth), static_cast<int32_t>(parentHeight));
      if(drawW <= 0 || drawH <= 0) {
        xPos += colW;
        continue;
      }
// Background
      FillRect(buffer, parentWidth, drawX, drawY, drawW, drawH, mHeaderBgColor);
// Right border (if visible)
      int32_t gridX = static_cast<int32_t>(colRight - 1);
      if(gridX >= drawX && gridX < drawX + drawW) {
        DrawVLine(buffer, parentWidth, gridX, drawY, drawH, mGridColor);
      }
// Bottom border
      int32_t bottomY = headerAbsY + headerH - 1;
      if(bottomY >= drawY && bottomY < drawY + drawH) {
        DrawHLine(buffer, parentWidth, drawX, bottomY, drawW, mGridColor);
      }
// Text
      std::string label = it->second.second;
      if(!label.empty()) {
        int32_t textW = MeasureTextWidth(label);
        int32_t textX = static_cast<int32_t>(colLeft + (colW - textW) / 2);
        int32_t textY = headerAbsY + (headerH - static_cast<int32_t>(mFontSize)) / 2
            + static_cast<int32_t>(mFontSize) * 3 / 4;
        if(textX + textW > drawX && textX < drawX + drawW && textY > drawY && textY < drawY + drawH) {
          DrawTextEx(buffer, parentWidth, parentHeight, textX, textY - static_cast<int32_t>(mFontSize) * 3 / 4 + 2,
              textW, static_cast<int32_t>(mFontSize), label, face, mFontSize, AUIHAlign::left, AUIVAlign::top, 0,
              mHeaderTextColor, textW);
        }
      }
      xPos += colW;
      if(colLeft >= headerAbsX + headerW)
        break;
    }
  }

  void ATable::DrawRowHeader(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY, const ATableRangeData1 &rowStart, const ATableRangeData1 &rowEnd) const {
    if(!mEnginePtr)
      return;
    int32_t headerAbsX = offsetX + mX;
    int32_t headerAbsY = offsetY + mY + static_cast<int32_t>(mColumnHeaderHeight);
    int32_t headerW = static_cast<int32_t>(mRowHeaderWidth);
    int32_t headerH = static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mColumnHeaderHeight);
    if(headerW <= 0 || headerH <= 0)
      return;
    std::unique_lock lock(mEnginePtr->GetFontMutex(), std::chrono::milliseconds(50));
    if (!lock.owns_lock()) { E("locked"); }

    FT_Face face = mEnginePtr->GetDefaultFontFace();
    if(!face)
      return;
    int64_t yPos = static_cast<int64_t>(headerAbsY) - rowStart.offset;
    auto it = mRowH.find(rowStart.cell);
    if(it == mRowH.end())
      return;
    for(int64_t row = rowStart.cell; row <= rowEnd.cell && it != mRowH.end(); ++it, row = it->first) {
      int64_t rowH = it->second.first;
      int64_t rowTop = yPos;
      int64_t rowBottom = yPos + rowH;
      if(rowBottom < headerAbsY || rowTop >= headerAbsY + headerH) {
        yPos += rowH;
        continue;
      }
      int32_t drawY = static_cast<int32_t>(std::max(rowTop, static_cast<int64_t>(headerAbsY)));
      int32_t drawH = static_cast<int32_t>(std::min(rowBottom, static_cast<int64_t>(headerAbsY) + headerH) - drawY);
      if(drawH <= 0) {
        yPos += rowH;
        continue;
      }
      int32_t drawX = headerAbsX;
      int32_t drawW = headerW;
      ClipRect(drawX, drawY, drawW, drawH, static_cast<int32_t>(parentWidth), static_cast<int32_t>(parentHeight));
      if(drawW <= 0 || drawH <= 0) {
        yPos += rowH;
        continue;
      }
// Background
      FillRect(buffer, parentWidth, drawX, drawY, drawW, drawH, mHeaderBgColor);
// Right border (vertical line at right edge of header)
      int32_t rightX = headerAbsX + headerW - 1;
      if(rightX >= drawX && rightX < drawX + drawW) {
        DrawVLine(buffer, parentWidth, rightX, drawY, drawH, mGridColor);
      }
// Bottom border
      int32_t gridY = static_cast<int32_t>(rowBottom - 1);
      if(gridY >= drawY && gridY < drawY + drawH) {
        DrawHLine(buffer, parentWidth, drawX, gridY, drawW, mGridColor);
      }
// Text (right‑aligned)
      std::string label = it->second.second;
      if(!label.empty()) {
        int32_t textW = MeasureTextWidth(label);
        int32_t textX = headerAbsX + headerW - textW - 4;
        int32_t textY = static_cast<int32_t>(rowTop + (rowH - static_cast<int32_t>(mFontSize)) / 2
            + static_cast<int32_t>(mFontSize) * 3 / 4);
        if(textX + textW > drawX && textX < drawX + drawW && textY > drawY && textY < drawY + drawH) {
          DrawTextEx(buffer, parentWidth, parentHeight, textX, textY - static_cast<int32_t>(mFontSize) * 3 / 4 + 2,
              textW, static_cast<int32_t>(mFontSize), label, face, mFontSize, AUIHAlign::left, AUIVAlign::top, 0,
              mHeaderTextColor, textW);
        }
      }
      yPos += rowH;
      if(rowTop >= headerAbsY + headerH)
        break;
    }
  }

  void ATable::SetColumnLabel(int64_t col, const std::string &label) {
    auto it = mColumnW.find(col);
    if(it != mColumnW.end()) {
      it->second.second = label;
      if(mParentWindow)
        mParentWindow->Draw();
    }
  }

  void ATable::SetRowLabel(int64_t row, const std::string &label) {
    auto it = mRowH.find(row);
    if(it != mRowH.end()) {
      it->second.second = label;
      if(mParentWindow)
        mParentWindow->Draw();
    }
  }

  void ATable::SetRowHeight(int64_t row, int64_t height) {
    auto it = mRowH.find(row);
    if(it != mRowH.end() && height > 0) {
      int64_t delta = height - it->second.first;
      it->second.first = height;
      mTotalContentHeight += delta;
      mRowPrefixDirty = true;
      UpdateLayout();
      if(mParentWindow)
        mParentWindow->Draw();
    }
  }

  void ATable::SetColumnWidth(int64_t col, int64_t width) {
    auto it = mColumnW.find(col);
    if(it != mColumnW.end() && width > 0) {
      int64_t delta = width - it->second.first;
      it->second.first = width;
      mTotalContentWidth += delta;
      mColPrefixDirty = true;
      UpdateLayout();
      if(mParentWindow)
        mParentWindow->Draw();
    }
  }

  void ATable::AddRows(uint32_t number) {
    for(uint32_t i = 0; i < number; ++i) {
      AddRow();
    }
  }

  void ATable::AddColumns(uint32_t number) {
    for(uint32_t i = 0; i < number; ++i) {
      AddColumn();
    }
  }

  void ATable::SetScrollbarsEnabled(bool enable) {
    if(mVScrollBar && mHScrollBar && enable)
      return;
    if(!enable) {
      mVScrollBar.reset();
      mHScrollBar.reset();
      if(mParentWindow)
        mParentWindow->Draw();
      return;
    }
    mVScrollBar = std::make_unique<AScrollBar>();
    mVScrollBar->SetOrientation(AUIOrientation::vertical);
    mVScrollBar->mParentWindow = mParentWindow;
    mVScrollBar->mEnginePtr = mEnginePtr;
    mVScrollBar->Resize(24, mSizeY - mColumnHeaderHeight);// width 24 (was 16)
    mVScrollBar->ShowArrows(true);
    mVScrollBar->SetArrowSize(18);// 50% wider than default 12
    mVScrollBar->SetTrackThickness(18);// 50% wider than default 12
    mVScrollBar->SetThumbThickness(24);// 50% wider than default 24
    mVScrollBar->SetScrollCallback([this](AWindow*, AWidget*, void*, int32_t val) {
      ScrollTo(static_cast<int32_t>(mHOffset), val);
    }, nullptr);
    mHScrollBar = std::make_unique<AScrollBar>();
    mHScrollBar->SetOrientation(AUIOrientation::horizontal);
    mHScrollBar->mParentWindow = mParentWindow;// <-- add this
    mHScrollBar->mEnginePtr = mEnginePtr;
    mHScrollBar->Resize(mSizeX - mRowHeaderWidth, 24);// height 24 (was 16)
    mHScrollBar->ShowArrows(true);
    mHScrollBar->SetArrowSize(18);
    mHScrollBar->SetTrackThickness(18);
    mHScrollBar->SetThumbThickness(24);
    mHScrollBar->SetScrollCallback([this](AWindow*, AWidget*, void*, int32_t val) {
      ScrollTo(val, static_cast<int32_t>(mVOffset));
    }, nullptr);

    UpdateScrollbarRanges();
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void ATable::UpdateScrollbarRanges() {
    if(!mVScrollBar || !mHScrollBar)
      return;
    int32_t viewWidth = static_cast<int32_t>(mSizeX - mRowHeaderWidth);
    int32_t viewHeight = static_cast<int32_t>(mSizeY - mColumnHeaderHeight);
    bool needV = (mTotalContentHeight > viewHeight);
    bool needH = (mTotalContentWidth > viewWidth);
    if(needV)
      viewWidth -= static_cast<int32_t>(mVScrollBar->SizeX());
    if(needH)
      viewHeight -= static_cast<int32_t>(mHScrollBar->SizeY());
    needV = (mTotalContentHeight > viewHeight);
    needH = (mTotalContentWidth > viewWidth);
    mVScrollBar->SetVisible(mAutoHideScrollbars ? needV : true);
    mHScrollBar->SetVisible(mAutoHideScrollbars ? needH : true);
    if(needV) {
      int32_t maxV = static_cast<int32_t>(mTotalContentHeight - viewHeight);
      if(maxV < 0)
        maxV = 0;
      mVScrollBar->SetRange(0, maxV);
      mVScrollBar->SetPageStep(viewHeight);
      mVScrollBar->SetSingleStep(20);
      if(mVOffset > maxV)
        mVOffset = maxV;
      mVScrollBar->SetValue(static_cast<int32_t>(mVOffset));
    }
    if(needH) {
      int32_t maxH = static_cast<int32_t>(mTotalContentWidth - viewWidth);
      if(maxH < 0)
        maxH = 0;
      mHScrollBar->SetRange(0, maxH);
      mHScrollBar->SetPageStep(viewWidth);
      mHScrollBar->SetSingleStep(20);
      if(mHOffset > maxH)
        mHOffset = maxH;
      mHScrollBar->SetValue(static_cast<int32_t>(mHOffset));
    }
  }

  void ATable::ScrollTo(int32_t xOffset, int32_t yOffset) {
    int32_t maxX = static_cast<int32_t>(std::max<int64_t>(0,
        mTotalContentWidth - (static_cast<int64_t>(mSizeX) - static_cast<int64_t>(mRowHeaderWidth))));
    int32_t maxY = static_cast<int32_t>(std::max<int64_t>(0,
        mTotalContentHeight - (static_cast<int64_t>(mSizeY) - static_cast<int64_t>(mColumnHeaderHeight))));
    xOffset = std::clamp(xOffset, 0, maxX);
    yOffset = std::clamp(yOffset, 0, maxY);
    if(static_cast<int32_t>(mHOffset) == xOffset && static_cast<int32_t>(mVOffset) == yOffset)
      return;
    mHOffset = xOffset;
    mVOffset = yOffset;
    if(mVScrollBar)
      mVScrollBar->SetValue(yOffset);
    if(mHScrollBar)
      mHScrollBar->SetValue(xOffset);
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void ATable::RemoveLastRow() {
    if(mRowH.empty())
      return;
    int64_t lastRow = mRowH.rbegin()->first;
    RemoveRow(lastRow);
  }

  void ATable::RemoveLastColumn() {
    if(mColumnW.empty())
      return;
    int64_t lastCol = mColumnW.rbegin()->first;
    RemoveColumn(lastCol);
  }

  void ATable::SetCursorPosition(int64_t row, int64_t col) {
    mCursorRow = row;
    mCursorCol = col;
    if(mRowSelectMode) {
      mSelectedRow = row;// also update the selected row
    }
    ScrollToCell(row, col);
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void ATable::SetRowSelectMode(bool enable) {
    mRowSelectMode = enable;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void ATable::ScrollToCell(int64_t row, int64_t col) {
    if(mColumnW.empty() || mRowH.empty())
      return;
    if(mColPrefixDirty)
      RebuildColPrefix();
    if(mRowPrefixDirty)
      RebuildRowPrefix();
    auto colIt = std::lower_bound(mColIds.begin(), mColIds.end(), col);
    if(colIt == mColIds.end() || *colIt != col)
      return;
    size_t colIdx = static_cast<size_t>(colIt - mColIds.begin());
    int64_t colX = mColPrefix[colIdx];
    int64_t colWidth = mColumnW.find(col)->second.first;
    auto rowIt = std::lower_bound(mRowIds.begin(), mRowIds.end(), row);
    if(rowIt == mRowIds.end() || *rowIt != row)
      return;
    size_t rowIdx = static_cast<size_t>(rowIt - mRowIds.begin());
    int64_t rowY = mRowPrefix[rowIdx];
    int64_t rowHeight = mRowH.find(row)->second.first;
    int32_t viewWidth = static_cast<int32_t>(mSizeX - mRowHeaderWidth);
    int32_t viewHeight = static_cast<int32_t>(mSizeY - mColumnHeaderHeight);
    if(mVScrollBar && mVScrollBar->IsVisible())
      viewWidth -= static_cast<int32_t>(mVScrollBar->SizeX());
    if(mHScrollBar && mHScrollBar->IsVisible())
      viewHeight -= static_cast<int32_t>(mHScrollBar->SizeY());
    int64_t newHOffset = mHOffset;
    int64_t newVOffset = mVOffset;
    if(colX < mHOffset)
      newHOffset = colX;
    else if(colX + colWidth > mHOffset + viewWidth)
      newHOffset = colX + colWidth - viewWidth;
    if(rowY < mVOffset)
      newVOffset = rowY;
    else if(rowY + rowHeight > mVOffset + viewHeight)
      newVOffset = rowY + rowHeight - viewHeight;
    if(newHOffset != mHOffset || newVOffset != mVOffset)
      ScrollTo(static_cast<int32_t>(newHOffset), static_cast<int32_t>(newVOffset));
  }
  std::pair<int64_t, int64_t> ATable::ScreenToCell(int32_t localX, int32_t localY, int32_t, int32_t) const {
// Header area check
    if(localX < static_cast<int32_t>(mRowHeaderWidth) || localY < static_cast<int32_t>(mColumnHeaderHeight)) {
      return {-1, -1};
    }
// Get starting column/row from scroll offsets
    ATableRangeData1 colStart, rowStart;
    colStart = Offset2Column(mHOffset);
    rowStart = Offset2Row(mVOffset);
    int64_t targetRow = -1, targetCol = -1;
// Column search
    int64_t currX = static_cast<int64_t>(mRowHeaderWidth) - colStart.offset;
    auto itCol = mColumnW.lower_bound(colStart.cell);
    {
      for(; itCol != mColumnW.end(); ++itCol) {
        int64_t colW = itCol->second.first;
        if(localX >= currX && localX < currX + colW) {
          targetCol = itCol->first;
          break;
        }
        currX += colW;
        if(currX > localX)
          break;
      }
    }
// Row search
    int64_t currY = static_cast<int64_t>(mColumnHeaderHeight) - rowStart.offset;
    auto itRow = mRowH.lower_bound(rowStart.cell);
    {
      for(; itRow != mRowH.end(); ++itRow) {
        int64_t rowH = itRow->second.first;
        if(localY >= currY && localY < currY + rowH) {
          targetRow = itRow->first;
          break;
        }
        currY += rowH;
        if(currY > localY)
          break;
      }
    }
    return {targetRow, targetCol};
  }

  bool ATable::OnMouseClick(int32_t localX, int32_t localY, bool pressed) {
// ---- Start resizing on press if over a separator ----
    if(pressed) {
      bool isCol;
      int64_t id;
      if(HitTestSeparator(localX, localY, isCol, id)) {
        mResizing = true;
        mResizeColumn = isCol;
        mResizeTargetId = id;
        if(isCol) {
          mResizeStartMouse = localX;
          mResizeStartSize = static_cast<int32_t>(mColumnW[id].first);
        }
        else {
          mResizeStartMouse = localY;
          mResizeStartSize = static_cast<int32_t>(mRowH[id].first);
        }
// Optional: capture mouse to receive events even outside widget
// if (mParentWindow) mParentWindow->SetCapture(this);
        return true;
      }
    }
// ---- End resizing on release ----
    if(!pressed && mResizing) {
      mResizing = false;
      mResizeTargetId = -1;
      if(mParentWindow)
        mParentWindow->Draw();
      return true;
    }
// ---- Otherwise, handle scrollbars or cell selection (existing logic) ----
// (Keep your original implementation here; only the above parts are new)
// Forward to scrollbars...
    auto forwardToScrollbar = [&](AScrollBar *sb) -> bool {
      if(!sb || !sb->IsVisible())
        return false;
      int32_t sbX = sb->X();
      int32_t sbY = sb->Y();
      if(localX + mX >= sbX && localX + mX < sbX + static_cast<int32_t>(sb->SizeX()) && localY + mY >= sbY
          && localY + mY < sbY + static_cast<int32_t>(sb->SizeY())) {
        int32_t lx = localX + mX - sbX;
        int32_t ly = localY + mY - sbY;
        bool ret = sb->OnMouseClick(lx, ly, pressed);
        if(pressed && ret)
          mDragScrollbar = sb;
        else if(!pressed && mDragScrollbar == sb)
          mDragScrollbar = nullptr;
        return ret;
      }
      return false;
    };
    if(forwardToScrollbar(mVScrollBar.get()))
      return true;
    if(forwardToScrollbar(mHScrollBar.get()))
      return true;
    if(!pressed && mDragScrollbar) {
      int32_t sbX = mDragScrollbar->X();
      int32_t sbY = mDragScrollbar->Y();
      int32_t lx = localX + mX - sbX;
      int32_t ly = localY + mY - sbY;
      mDragScrollbar->OnMouseClick(lx, ly, false);
      mDragScrollbar = nullptr;
      return true;
    }
// Cell selection (only on press)
    if(pressed) {
      std::pair<int64_t, int64_t> cell = ScreenToCell(localX, localY, 0, 0);
      if(cell.first != -1 && cell.second != -1) {
        if(mRowSelectMode)
          mSelectedRow = cell.first;
        else {
          mCursorRow = cell.first;
          mCursorCol = cell.second;
        }
        AWidget::OnMouseClick(localX, localY, pressed);
        if(mParentWindow)
          mParentWindow->Draw();
        return true;
      }
    }
    return AWidget::OnMouseClick(localX, localY, pressed);
  }

  void ATable::RebuildColPrefix() const {
    mColPrefix.clear();
    mColIds.clear();
    mColPrefix.reserve(mColumnW.size() + 1);
    mColIds.reserve(mColumnW.size());
    mColPrefix.push_back(0);
    for(const auto& [id, pair] : mColumnW) {
      mColIds.push_back(id);
      mColPrefix.push_back(mColPrefix.back() + pair.first);
    }
    mColPrefixDirty = false;
  }

  void ATable::RebuildRowPrefix() const {
    mRowPrefix.clear();
    mRowIds.clear();
    mRowPrefix.reserve(mRowH.size() + 1);
    mRowIds.reserve(mRowH.size());
    mRowPrefix.push_back(0);
    for(const auto& [id, pair] : mRowH) {
      mRowIds.push_back(id);
      mRowPrefix.push_back(mRowPrefix.back() + pair.first);
    }
    mRowPrefixDirty = false;
  }

  void ATable::BeginBatch() {
    if(mBatchDepth++ == 0) {
      mBatchRowToIdx.clear();
      mBatchColToIdx.clear();
      mBatchIdxToRow.clear();
      mBatchIdxToCol.clear();
      mBatchCells.clear();
    }
  }

  void ATable::BeginBatch(uint32_t prealloc) {
    if(mBatchDepth++ == 0) {
      mBatchRowToIdx.clear();
      mBatchColToIdx.clear();
      mBatchIdxToRow.clear();
      mBatchIdxToCol.clear();
      mBatchCells.clear();
      mCells.reserve(prealloc);
      uint32_t estimatedRows = static_cast<uint32_t>(std::sqrt(prealloc));
      mBatchCells.reserve(prealloc);
      mBatchRowToIdx.reserve(estimatedRows);
      mBatchIdxToRow.reserve(estimatedRows);
    }
  }
  void ATable::EndBatch() {
    if(--mBatchDepth > 0)
      return;
// 1. Ensure all rows exist in the real maps
    for(int64_t row : mBatchIdxToRow) {
      if(mRowH.find(row) == mRowH.end()) {
        mRowH[row] = { 24, std::to_string(row) };
        mTotalContentHeight += 24;
      }
    }
// 2. Ensure all columns exist
    for(int64_t col : mBatchIdxToCol) {
      if(mColumnW.find(col) == mColumnW.end()) {
        mColumnW[col] = { 80, std::to_string(col) };
        mTotalContentWidth += 80;
      }
    }
    mRowPrefixDirty = true;
    mColPrefixDirty = true;
    for(size_t r = 0; r < mBatchCells.size(); ++r) {
      int64_t row = mBatchIdxToRow[r];
      if(mRowH.find(row) == mRowH.end()) {
        mRowH[row] = { 24, std::to_string(row) };
        mTotalContentHeight += 24;
        mRowPrefixDirty = true;
      }
      for(size_t c = 0; c < mBatchCells[r].size(); ++c) {
        int64_t col = mBatchIdxToCol[c];
        if(mColumnW.find(col) == mColumnW.end()) {
          mColumnW[col] = { 80, std::to_string(col) };
          mTotalContentWidth += 80;
          mColPrefixDirty = true;
        }
// Direct assignment straight to your fast O(1) cell map
        uint64_t key = MakeCellKey(row, col);
        mCells[key] = mBatchCells[r][c];
      }
    }
// Clean out batch staging containers
    mBatchCells.clear();
    mBatchRowToIdx.clear();
    mBatchColToIdx.clear();
    mBatchIdxToRow.clear();
    mBatchIdxToCol.clear();
    mRowPrefixDirty = true;
    mColPrefixDirty = true;
    UpdateScrollbarRanges();
    if(mParentWindow) {
      mParentWindow->Draw();
    }
  }

  void ATable::SetCellData(int64_t row, int64_t col, const std::string &text, AUIHAlign hAlign) {
// BATCH PATH
    if(mBatchDepth > 0) {
// Map row to contiguous index
      auto rowIt = mBatchRowToIdx.find(row);
      if(rowIt == mBatchRowToIdx.end()) {
        int32_t newIdx = static_cast<int32_t>(mBatchIdxToRow.size());
        rowIt = mBatchRowToIdx.emplace(row, newIdx).first;
        mBatchIdxToRow.push_back(row);
        mBatchCells.emplace_back();// new row vector
      }
      int32_t rowIdx = rowIt->second;
// Map column to contiguous index
      auto colIt = mBatchColToIdx.find(col);
      if(colIt == mBatchColToIdx.end()) {
        int32_t newIdx = static_cast<int32_t>(mBatchIdxToCol.size());
        colIt = mBatchColToIdx.emplace(col, newIdx).first;
        mBatchIdxToCol.push_back(col);
// Expand all existing rows to include the new column
        for(auto &rowVec : mBatchCells) {
          rowVec.resize(static_cast<size_t>(mBatchIdxToCol.size()));
        }
      }
      int32_t colIdx = colIt->second;
// Safe access with size_t casts
      size_t uRowIdx = static_cast<size_t>(rowIdx);
      size_t uColIdx = static_cast<size_t>(colIdx);
// Ensure row vector exists
      if(uRowIdx >= mBatchCells.size()) {
        mBatchCells.resize(uRowIdx + 1);
      }
// Ensure row has enough columns
      if(uColIdx >= mBatchCells[uRowIdx].size()) {
        mBatchCells[uRowIdx].resize(uColIdx + 1);
      }
      AUICellData &cell = mBatchCells[uRowIdx][uColIdx];
      cell.data = text;
      cell.hAlign = hAlign;
      return;// No redraw, no auto-widen, no map operations
    }
// ORIGINAL PATH (unchanged)
    AUICellData &cell = GetOrCreateCell(row, col);
    cell.data = text;
    cell.hAlign = hAlign;
    if(mAutoWiden) {
      AutoWidenColumn(col);
    }
    if(mParentWindow) {
      mParentWindow->Draw();
    }
  }

  void ATable::UpdateLayout() {
    if(!mVScrollBar || !mHScrollBar)
      return;
// Compute visibility based on content size
    int32_t viewWidth = static_cast<int32_t>(mSizeX - mRowHeaderWidth);
    int32_t viewHeight = static_cast<int32_t>(mSizeY - mColumnHeaderHeight);
    bool needV = (mTotalContentHeight > viewHeight);
    bool needH = (mTotalContentWidth > viewWidth);
    if(needV)
      viewWidth -= static_cast<int32_t>(mVScrollBar->SizeX());
    if(needH)
      viewHeight -= static_cast<int32_t>(mHScrollBar->SizeY());
    needV = (mTotalContentHeight > viewHeight);
    needH = (mTotalContentWidth > viewWidth);
    mVScrollBar->SetVisible(mAutoHideScrollbars ? needV : true);
    mHScrollBar->SetVisible(mAutoHideScrollbars ? needH : true);
// Position and size vertical scrollbar
    int32_t vX = mX + static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mVScrollBar->SizeX());
    int32_t vY = mY + static_cast<int32_t>(mColumnHeaderHeight);
    int32_t vH = static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mColumnHeaderHeight)
        - (mHScrollBar->IsVisible() ? static_cast<int32_t>(mHScrollBar->SizeY()) : 0);
    if(vH > 0) {
      mVScrollBar->Move(vX, vY);
      mVScrollBar->Resize(mVScrollBar->SizeX(), static_cast<uint32_t>(vH));
    }
// Position and size horizontal scrollbar
    int32_t hX = mX + static_cast<int32_t>(mRowHeaderWidth);
    int32_t hY = mY + static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mHScrollBar->SizeY());
    int32_t hW = static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mRowHeaderWidth)
        - (mVScrollBar->IsVisible() ? static_cast<int32_t>(mVScrollBar->SizeX()) : 0);
    if(hW > 0) {
      mHScrollBar->Move(hX, hY);
      mHScrollBar->Resize(static_cast<uint32_t>(hW), mHScrollBar->SizeY());
    }
// Update ranges and values
    UpdateScrollbarRanges();
  }

// Returns true if a separator is found, and fills out parameters.
  bool ATable::HitTestSeparator(int32_t localX, int32_t localY, bool &isColumn, int64_t &id) const {
// Column header hit test
    if(localY >= 0 && localY < static_cast<int32_t>(mColumnHeaderHeight)) {
// We are in the column header region (right of row header)
      if(localX > static_cast<int32_t>(mRowHeaderWidth)) {
        int64_t xOffset = mHOffset;
        auto colStart = Offset2Column(xOffset);
        if(colStart.cell < 0)
          return false;
// Iterate over columns from colStart
        int64_t currX = static_cast<int64_t>(mRowHeaderWidth) - colStart.offset;
        auto it = mColumnW.lower_bound(colStart.cell);
        for(; it != mColumnW.end(); ++it) {
          int64_t colW = it->second.first;
          int64_t rightEdge = currX + colW;
// Check if mouse is within RESIZE_THRESHOLD pixels of the right edge
          const int32_t THRESHOLD = 4;
          if(std::abs(localX - rightEdge) <= THRESHOLD) {
// The separator after this column (if there is a next column)
// We can resize this column.
            auto nextIt = std::next(it);
            if(nextIt != mColumnW.end()) {
              isColumn = true;
              id = it->first;// resize the column on the left of the line
              return true;
            }
          }
          currX += colW;
          if(currX > localX + THRESHOLD)
            break;
        }
      }
    }
// Row header hit test
    if(localX >= 0 && localX < static_cast<int32_t>(mRowHeaderWidth)) {
      if(localY > static_cast<int32_t>(mColumnHeaderHeight)) {
        int64_t yOffset = mVOffset;
        auto rowStart = Offset2Row(yOffset);
        if(rowStart.cell < 0)
          return false;
        int64_t currY = static_cast<int64_t>(mColumnHeaderHeight) - rowStart.offset;
        auto it = mRowH.lower_bound(rowStart.cell);
        for(; it != mRowH.end(); ++it) {
          int64_t rowH = it->second.first;
          int64_t bottomEdge = currY + rowH;
          const int32_t THRESHOLD = 4;
          if(std::abs(localY - bottomEdge) <= THRESHOLD) {
            auto nextIt = std::next(it);
            if(nextIt != mRowH.end()) {
              isColumn = false;
              id = it->first;
              return true;
            }
          }
          currY += rowH;
          if(currY > localY + THRESHOLD)
            break;
        }
      }
    }
    return false;
  }

  void ATable::OnMouseLeave() {
    AWidget::OnMouseLeave();
    if(mParentWindow) {
      mParentWindow->SetCursor(AUICursorType::Default);
    }
  }

}// namespace aui
