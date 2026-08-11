#include "AUILib.h"

namespace aui {
  ATable::ATable() {
    mType = AUIWidgetType::defaultTable;
    mBGColor = 0xFFEEEEEE;
    mSizeX = 300;
    mSizeY = 200;
    mHAlign = AUIHAlign::left;
    mVAlign = AUIVAlign::top;
    mResizeHover = false;
    mResizeHoverId = -1;
    mResizeHoverColumn = false;
    Focusable(true);
    mColumnHeaderHeight = 24;// typical header height
    mRowHeaderWidth = 40;// typical row header width
    mAutoHideScrollbars = true;// sensible default
    mFontSize = 12;
    mTextColor = 0xFF000000;
    mHeaderBGColor = 0xFFE0E0E0;
    mHeaderTextColor = 0xFF000000;
    mGridColor = 0xFFCCCCCC;
    mSelectionColor = 0xFFCCE5FF;
    mCursorBorderColor = 0xFF3399FF;
    mRowSelectMode = false;
    mCursorRow = -1;
    mCursorCol = -1;
    mSelectedRow = -1;
    mHOffset = 0;
    mVOffset = 0;
    mTotalContentWidth = 0;
    mTotalContentHeight = 0;
    mResizing = false;
    mResizeColumn = false;
    mResizeTargetId = -1;
    mResizeStartMouse = 0;
    mResizeStartSize = 0;
    mResizeMinSize = 20;
    mDragScrollbar = nullptr;
    mBatchDepth = 0;
    mResizeMinSize = 10;
    Text("some table");
  }

  int32_t ATable::MeasureTextWidth(const std::string& text) const {
    if(!Wnd()->EnginePtr())
      return 0;
    FT_Face face = Wnd()->EnginePtr()->DefaultFontFace();
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

  ATableRangeData1 ATable::Offset2RowRange(const ATableRangeData1& start, int64_t height) const {
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

  ATableRangeData1 ATable::Offset2ColumnRange(const ATableRangeData1& start, int64_t width) const {
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

  void ATable::AddRow() {
// Look at mRowH or mRowIds to determine the next ID index safely
    int64_t newId = mRowH.empty() ? 0 : mRowH.rbegin()->first + 1;
// Explicitly seed the default heights just like before
    mRowH[newId] = { 24, std::to_string(newId) };
    mTotalContentHeight += 24;
    mRowPrefixDirty = true;
    LayoutDirty();
  }

  void ATable::AddColumn() {
    int64_t newId = mColumnW.empty() ? 0 : mColumnW.rbegin()->first + 1;
//    mColumnW[newId] = { AUI_TABLE_CELL_W, std::to_string(newId) };
    mColumnW[newId] = { AUI_TABLE_CELL_W, NumberToBaseString(SafeUINT64(newId)) };
    mTotalContentWidth += AUI_TABLE_CELL_W;
    mColPrefixDirty = true;
    LayoutDirty();
  }

  void ATable::RemoveRow(int64_t rowIdx) {
    auto rowIt = mRowH.find(rowIdx);
    if(rowIt == mRowH.end())
      return;
    mTotalContentHeight -= SafeUINT64(rowIt->second.first);
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
    LayoutDirty();
  }

  void ATable::RemoveColumn(int64_t colIdx) {
    auto colIt = mColumnW.find(colIdx);
    if(colIt == mColumnW.end())
      return;
    mTotalContentWidth -= SafeUINT64(colIt->second.first);
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
    LayoutDirty();
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
    MarkContentDirty();
    Wnd()->RequestRedraw();
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
      mColumnW[col] = { AUI_TABLE_CELL_W, NumberToBaseString(SafeUINT64(col)) };
      mTotalContentWidth += AUI_TABLE_CELL_W;
      mColPrefixDirty = true;
    }
    uint64_t key = MakeCellKey(row, col);
// std::unordered_map[] constructs the value if it doesn't exist yet
    return mCells[key];
  }

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
    mTotalContentWidth -= SafeUINT64(colIt->second.first);
    colIt->second.first = newWidth;
    mTotalContentWidth += SafeUINT64(newWidth);
    mColPrefixDirty = true;
    LayoutDirty();
  }

  void ATable::ColumnLabel(int64_t col, const std::string& label) {
    auto it = mColumnW.find(col);
    if(it != mColumnW.end()) {
      it->second.second = label;
      MarkContentDirty();
      Wnd()->RequestRedraw();
    }
  }

  void ATable::RowLabel(int64_t row, const std::string& label) {
    auto it = mRowH.find(row);
    if(it != mRowH.end()) {
      it->second.second = label;
      MarkContentDirty();
      Wnd()->RequestRedraw();
    }
  }

  void ATable::AddRows(uint32_t number) {
    for(uint32_t i = 0; i < number; ++i) {
      AddRow();
    }
    LayoutDirty();
  }

  void ATable::AddColumns(uint32_t number) {
    for(uint32_t i = 0; i < number; ++i) {
      AddColumn();
    }
    LayoutDirty();
  }

  void ATable::RemoveLastRow() {
    if(mRowH.empty())
      return;
    int64_t lastRow = mRowH.rbegin()->first;
    RemoveRow(lastRow);
  }

  bool ATable::HitTestSeparator(int32_t localX, int32_t localY, bool& isColumn, int64_t& id) const {
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

  void ATable::LayoutUpdate() {
    D2("starts")
    if(!LayoutIsDirty())
      return;
    else {
      LayoutDirtyToggle(false);
    }
    if(!mVScrollBar || !mHScrollBar)
      return;
// Compute visibility based on content size
    int32_t viewWidth = static_cast<int32_t>(mSizeX - mRowHeaderWidth);
    int32_t viewHeight = static_cast<int32_t>(mSizeY - mColumnHeaderHeight);
    bool needV = (SafeINT64(mTotalContentHeight) > viewHeight);
    bool needH = (SafeINT64(mTotalContentWidth) > viewWidth);
    if(needV)
      viewWidth -= static_cast<int32_t>(mVScrollBar->SizeX());
    if(needH)
      viewHeight -= static_cast<int32_t>(mHScrollBar->SizeY());
    needV = (SafeINT64(mTotalContentHeight) > viewHeight);
    needH = (SafeINT64(mTotalContentWidth) > viewWidth);
    mVScrollBar->Visible(mAutoHideScrollbars ? needV : true);
    mHScrollBar->Visible(mAutoHideScrollbars ? needH : true);
// Position and size vertical scrollbar (LOCAL to ATable)
    int32_t vX = static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mVScrollBar->SizeX());
    int32_t vY = static_cast<int32_t>(mColumnHeaderHeight);
    int32_t vH = static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mColumnHeaderHeight)
        - (mHScrollBar->Visible() ? static_cast<int32_t>(mHScrollBar->SizeY()) : 0);
    if(vH > 0) {
      mVScrollBar->Move(vX, vY);
      mVScrollBar->Resize(mVScrollBar->SizeX(), static_cast<uint32_t>(vH));
    }
// Position and size horizontal scrollbar (LOCAL to ATable)
    int32_t hX = static_cast<int32_t>(mRowHeaderWidth);
    int32_t hY = static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mHScrollBar->SizeY());
    int32_t hW = static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mRowHeaderWidth)
        - (mVScrollBar->Visible() ? static_cast<int32_t>(mVScrollBar->SizeX()) : 0);
    if(hW > 0) {
      mHScrollBar->Move(hX, hY);
      mHScrollBar->Resize(static_cast<uint32_t>(hW), mHScrollBar->SizeY());
    }
    UpdateScrollbarRanges();
  }

  void ATable::UpdateScrollbarRanges() {
    if(!mVScrollBar || !mHScrollBar)
      return;
    int32_t viewWidth = static_cast<int32_t>(mSizeX - mRowHeaderWidth);
    int32_t viewHeight = static_cast<int32_t>(mSizeY - mColumnHeaderHeight);
    bool needV = (SafeINT64(mTotalContentHeight) > viewHeight);
    bool needH = (SafeINT64(mTotalContentWidth) > viewWidth);
    if(needV)
      viewWidth -= static_cast<int32_t>(mVScrollBar->SizeX());
    if(needH)
      viewHeight -= static_cast<int32_t>(mHScrollBar->SizeY());
    needV = (SafeINT64(mTotalContentHeight) > viewHeight);
    needH = (SafeINT64(mTotalContentWidth) > viewWidth);
    mVScrollBar->Visible(mAutoHideScrollbars ? needV : true);
    mHScrollBar->Visible(mAutoHideScrollbars ? needH : true);
    if(needV) {
      int32_t maxV = static_cast<int32_t>(SafeINT32(mTotalContentHeight));
      if(maxV < 0)
        maxV = 0;
      mVScrollBar->Range(0, maxV);
      mVScrollBar->PageStep(viewHeight);
      mVScrollBar->SingleStep(20);
      if(mVOffset > maxV)
        mVOffset = maxV;
      mVScrollBar->Value(static_cast<int32_t>(mVOffset));
    }
    if(needH) {
      int32_t maxH = static_cast<int32_t>(SafeINT64(mTotalContentWidth));
      if(maxH < 0)
        maxH = 0;
      mHScrollBar->Range(0, maxH);
      mHScrollBar->PageStep(viewWidth);
      mHScrollBar->SingleStep(20);
      if(mHOffset > maxH)
        mHOffset = maxH;
      mHScrollBar->Value(static_cast<int32_t>(mHOffset));
    }
  }

  void ATable::RowHeight(int64_t row, int64_t height) {
    auto it = mRowH.find(row);
    if(it != mRowH.end() && height > 0) {
      int64_t oldHeight = it->second.first;
      int64_t delta = height - oldHeight;
      it->second.first = height;
      if(delta > 0) {
        mTotalContentHeight += SafeUINT64(delta);
      }
      else
        if(delta < 0) {
          uint64_t absDelta = SafeUINT64(-delta);
          if(mTotalContentHeight >= absDelta) {
            mTotalContentHeight -= absDelta;
          }
          else {
            mTotalContentHeight = 0;
          }
        }
      mRowPrefixDirty = true;
      MarkContentDirty();
      Wnd()->RequestRedraw();
    }
  }

  void ATable::ColumnWidth(int64_t col, int64_t width) {
    auto it = mColumnW.find(col);
    if(it != mColumnW.end() && width > 0) {
      int64_t oldWidth = it->second.first;
      int64_t delta = width - oldWidth;
      it->second.first = width;
      if(delta > 0) {
        mTotalContentWidth += SafeUINT64(delta);
      }
      else
        if(delta < 0) {
          uint64_t absDelta = SafeUINT64(-delta);
          if(mTotalContentWidth >= absDelta) {
            mTotalContentWidth -= absDelta;
          }
          else {
            mTotalContentWidth = 0;// Prevent underflow safety fallback
          }
        }
      mColPrefixDirty = true;
      MarkContentDirty();
      Wnd()->RequestRedraw();
    }
  }

  void ATable::RemoveLastColumn() {
    if(mColumnW.empty())
      return;
    int64_t lastCol = mColumnW.rbegin()->first;
    RemoveColumn(lastCol);
  }

  void ATable::CursorPosition(int64_t row, int64_t col) {
    mCursorRow = row;
    mCursorCol = col;
    if(mRowSelectMode) {
      mSelectedRow = row;// also update the selected row
    }
    ScrollToCell(row, col);
    LayoutDirty();
    Wnd()->RequestRedraw();
  }

  void ATable::OnParentResize(UNUSED uint32_t newWidth, UNUSED uint32_t newHeight) {
    LayoutUpdate();
  }

  void ATable::ScrollTo(int32_t xOffset, int32_t yOffset) {
    int32_t maxX = static_cast<int32_t>(std::max<int64_t>(0,
        SafeINT32(mTotalContentWidth) - (static_cast<int64_t>(mSizeX) - static_cast<int64_t>(mRowHeaderWidth))));
    int32_t maxY = static_cast<int32_t>(std::max<int64_t>(0,
        SafeINT32(mTotalContentHeight) - (static_cast<int64_t>(mSizeY) - static_cast<int64_t>(mColumnHeaderHeight))));
    xOffset = std::clamp(xOffset, 0, maxX);
    yOffset = std::clamp(yOffset, 0, maxY);
    if(static_cast<int32_t>(mHOffset) == xOffset && static_cast<int32_t>(mVOffset) == yOffset)
      return;
    mHOffset = xOffset;
    mVOffset = yOffset;
    if(mVScrollBar) {
      mVScrollBar->Value(yOffset);
    }
    if(mHScrollBar) {
      mHScrollBar->Value(xOffset);
    }
    LayoutDirty();
    Wnd()->RequestRedraw();
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
    if(mVScrollBar && mVScrollBar->Visible())
      viewWidth -= static_cast<int32_t>(mVScrollBar->SizeX());
    if(mHScrollBar && mHScrollBar->Visible())
      viewHeight -= static_cast<int32_t>(mHScrollBar->SizeY());
    int64_t newHOffset = mHOffset;
    int64_t newVOffset = mVOffset;
    if(colX < mHOffset)
      newHOffset = colX;
    else
      if(colX + colWidth > mHOffset + viewWidth)
        newHOffset = colX + colWidth - viewWidth;
    if(rowY < mVOffset)
      newVOffset = rowY;
    else
      if(rowY + rowHeight > mVOffset + viewHeight)
        newVOffset = rowY + rowHeight - viewHeight;
    if(newHOffset != mHOffset || newVOffset != mVOffset)
      ScrollTo(static_cast<int32_t>(newHOffset), static_cast<int32_t>(newVOffset));
    LayoutDirty();
  }

  void ATable::CellData(int64_t row, int64_t col, const std::string& text, AUIHAlign hAlign) {
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
        for(auto& rowVec : mBatchCells) {
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
      AUICellData& cell = mBatchCells[uRowIdx][uColIdx];
      cell.data = text;
      cell.hAlign = hAlign;
      return;// No redraw, no auto-widen, no map operations
    }
// ORIGINAL PATH (unchanged)
    AUICellData& cell = GetOrCreateCell(row, col);
    cell.data = text;
    cell.hAlign = hAlign;
    if(mAutoWiden) {
      AutoWidenColumn(col);
    }
    MarkContentDirty();
    Wnd()->RequestRedraw();
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

  void ATable::RowSelectMode(bool enable) {
    mRowSelectMode = enable;
    LayoutDirty();
  }

  void ATable::BeginBatch() {
    LayoutDirty();
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
        mColumnW[col] = { AUI_TABLE_CELL_W, NumberToBaseString(SafeUINT64(col)) };
        mTotalContentWidth += AUI_TABLE_CELL_W;
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
    LayoutDirty();
    Wnd()->RequestRedraw();
  }

  void ATable::OnMouseWheel(int32_t delta) {
    int32_t step = 30;
    int32_t newVOffset = static_cast<int32_t>(mVOffset) - (delta * step);
    int32_t maxV = std::max(0,
        static_cast<int32_t>(SafeINT64(mTotalContentHeight)
            - (static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mColumnHeaderHeight))));
    newVOffset = std::clamp(newVOffset, 0, maxV);
    ScrollTo(static_cast<int32_t>(mHOffset), newVOffset);
    LayoutDirty();
  }

  void ATable::OnMouseLeave() {
//AWidget::OnMouseLeave();
    LayoutDirty();
    Wnd()->BackendCursor(AUICursorType::Default);
  }

  void ATable::OnResize(uint32_t newWidth, uint32_t newHeight) {
    D2()
    AWidget::OnResize(newWidth, newHeight);// update mSizeX/mSizeY if needed
    LayoutDirty();
    LayoutUpdate();// refresh scrollbars and offsets
  }

  AWidget* ATable::OnMouseDownLeft(int32_t localX, int32_t localY) {
    LayoutUpdate();
    D2("localX {} localY {}", localX, localY);
    // ---- 1. Column / Row Separator Resizing ----
    bool isCol = false;
    int64_t id = -1;
    if (HitTestSeparator(localX, localY, isCol, id)) {
      mResizing = true;
      mResizeColumn = isCol;
      mResizeTargetId = id;
      if (isCol) {
        mResizeStartMouse = localX;
        mResizeStartSize = static_cast<int32_t>(mColumnW[id].first);
      } else {
        mResizeStartMouse = localY;
        mResizeStartSize = static_cast<int32_t>(mRowH[id].first);
      }
      return this;
    }
    // ---- 2. Scrollbar Forwarding ----
    auto tryForwardToScrollbar = [&](AScrollBar* sb) -> bool {
      if (!sb || !sb->Visible()) {
        return false;
      }
      int32_t sbX = sb->X();
      int32_t sbY = sb->Y();
      int32_t sbW = static_cast<int32_t>(sb->SizeX());
      int32_t sbH = static_cast<int32_t>(sb->SizeY());
      // Hit-test in ATable-local coordinates
      if (localX >= sbX && localX < sbX + sbW && localY >= sbY && localY < sbY + sbH) {
        if (sb->OnMouseDownLeft(localX - sbX, localY - sbY)) {
          mDragScrollbar = sb;
          return true; // Click handled by scrollbar
        }
      }
      return false;
    };
    if (tryForwardToScrollbar(mVScrollBar.get()) || tryForwardToScrollbar(mHScrollBar.get())) {
      return this; // Return 'this' to retain capture on ATable
    }
    // ---- 3. Cell Selection ----
    auto [row, col] = ScreenToCell(localX, localY, 0, 0);
    if (row != -1 && col != -1) {
      if (mRowSelectMode) {
        mSelectedRow = row;
      } else {
        mCursorRow = row;
        mCursorCol = col;
      }
      D2("Cell selected: row=%lld col=%lld", static_cast<int64_t>(row), static_cast<int64_t>(col));
      AWidget* ret = AWidget::OnMouseDownLeft(localX, localY);
      MarkContentDirty();
      Wnd()->RequestRedraw();
      return ret ? ret : this;
    }
    // ---- 4. Table Background / Non-Cell Click ----
    D2("Click on table but not on a cell – forwarding to base callback");
    return AWidget::OnMouseDownLeft(localX, localY);
  }

  AWidget* ATable::OnMouseUpLeft(UNUSED int32_t localX, UNUSED int32_t localY) {
    LayoutUpdate();
    D2("localX {} localY {}", localX, localY);
    if(mResizing) {
      mResizing = false;
      mResizeTargetId = -1;
      MarkContentDirty();
      Wnd()->RequestRedraw();
      return this;
    }
    if(mDragScrollbar) {
      AScrollBar* sb = mDragScrollbar;
      mDragScrollbar = nullptr;
      int32_t lx = localX - sb->X();
      int32_t ly = localY - sb->Y();
      return sb->OnMouseUpLeft(lx, ly);
    }
    return AWidget::OnMouseUpLeft(localX, localY);
  }

  bool ATable::OnMouseMove(int32_t localX, int32_t localY) {
    D1("MouseMove ATable raw local: %d, %d", localX, localY);
    // ---- 1. Column / Row Resize Dragging ----
    if (mResizing) {
      int32_t currentPos = mResizeColumn ? localX : localY;
      int32_t delta = currentPos - mResizeStartMouse;
      int32_t newSize = std::max(mResizeStartSize + delta, mResizeMinSize);
      if (mResizeColumn) {
        ColumnWidth(mResizeTargetId, newSize);
      } else {
        RowHeight(mResizeTargetId, newSize);
      }
      mResizeStartMouse = currentPos;
      mResizeStartSize = newSize;
      MarkContentDirty();
      Wnd()->RequestRedraw();
      return true;
    }
    // ---- 2. Scrollbar Drag Forwarding ----
    if (mDragScrollbar) {
      // Translate ATable-local coordinates into scrollbar-local coordinates
      int32_t lx = localX - mDragScrollbar->X();
      int32_t ly = localY - mDragScrollbar->Y();
      mDragScrollbar->OnMouseMove(lx, ly);
      Wnd()->RequestRedraw();
      return true;
    }
    // ---- 3. Hover Feedback (Cursor State) ----
    bool isCol = false;
    int64_t id = -1;
    bool hit = HitTestSeparator(localX, localY, isCol, id);
    if (hit != mResizeHover || id != mResizeHoverId || isCol != mResizeHoverColumn) {
      mResizeHover = hit;
      mResizeHoverId = id;
      mResizeHoverColumn = isCol;
      if (Wnd()) {
        AUICursorType cursor = hit ? (isCol ? AUICursorType::HResize : AUICursorType::VResize)
                                   : AUICursorType::Default;
        Wnd()->BackendCursor(cursor);
      }
    }
    // ---- 4. Forward to Base Class ----
    return AWidget::OnMouseMove(localX, localY);
  }

  void ATable::ScrollbarsToggle(bool enable) {
    if(mVScrollBar && mHScrollBar && enable)
      return;
    if(!enable) {
      mVScrollBar.reset();
      mHScrollBar.reset();
      MarkContentDirty();
      Wnd()->RequestRedraw();
      return;
    }
    mVScrollBar = std::unique_ptr<AScrollBar>(new AScrollBar());
    mVScrollBar->Orient(AUIOrientation::vertical);
    mVScrollBar->Wnd(Wnd());
    mVScrollBar->Resize(16, mSizeY - mColumnHeaderHeight);// width 24 (was 16)
    mVScrollBar->Arrows(true);
    mVScrollBar->ArrowSize(16);// 50% wider than default 12
    mVScrollBar->TrackThick(10);// 50% wider than default 12
    mVScrollBar->ThumbThick(16);// 50% wider than default 24
    mVScrollBar->SetScrollCallback([this](AWidget*, void*, int32_t val) {
      ScrollTo(static_cast<int32_t>(mHOffset), val);
    }, nullptr);
    mHScrollBar = std::unique_ptr<AScrollBar>(new AScrollBar());
    mHScrollBar->Orient(AUIOrientation::horizontal);
    mHScrollBar->Wnd(Wnd());
    mHScrollBar->Resize(mSizeX - mRowHeaderWidth, 16);// height 24 (was 16)
    mHScrollBar->Arrows(true);
    mHScrollBar->ArrowSize(16);
    mHScrollBar->TrackThick(10);
    mHScrollBar->ThumbThick(16);
    mHScrollBar->SetScrollCallback([this](AWidget*, void*, int32_t val) {
      ScrollTo(val, static_cast<int32_t>(mVOffset));
    }, nullptr);
    MarkContentDirty();
    Wnd()->RequestRedraw();
  }

  void ATable::OnDraw(UNUSED uint32_t* buffer, UNUSED uint32_t bufferW, UNUSED uint32_t bufferH,
  UNUSED int32_t offsetX, UNUSED int32_t offsetY,
  UNUSED int32_t clipL, UNUSED int32_t clipT, UNUSED int32_t clipR, UNUSED int32_t clipB) const {
    AUI* au = Wnd() ? Wnd()->EnginePtr() : nullptr;
    if(!au)
      return;
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    ATableRangeData1 colStart = Offset2Column(mHOffset);
    ATableRangeData1 colEnd = Offset2ColumnRange(colStart, static_cast<int64_t>(mSizeX));
    ATableRangeData1 rowStart = Offset2Row(mVOffset);
    ATableRangeData1 rowEnd = Offset2RowRange(rowStart, static_cast<int64_t>(mSizeY));
    DrawCells(buffer, bufferW, bufferH, offsetX, offsetY, clipL, clipT, clipR, clipB, { rowStart.cell, rowStart.offset,
        rowEnd.cell, rowEnd.offset }, { colStart.cell, colStart.offset, colEnd.cell, colEnd.offset });
    DrawColumnHeader(buffer, bufferW, bufferH, offsetX, offsetY, clipL, clipT, clipR, clipB, colStart, colEnd);
    DrawRowHeader(buffer, bufferW, bufferH, offsetX, offsetY, clipL, clipT, clipR, clipB, rowStart, rowEnd);
    DrawIntersectionBox(buffer, bufferW, bufferH, offsetX, offsetY, clipL, clipT, clipR, clipB);
// ---- Draw vertical scrollbar ----
    if(mVScrollBar && mVScrollBar->Visible()) {
      int32_t sbH = static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mColumnHeaderHeight);
      if(mHScrollBar && mHScrollBar->Visible())
        sbH -= static_cast<int32_t>(mHScrollBar->SizeY());
      if(sbH > 0) {
// Ensure size matches current layout (UpdateLayout should have set it, but keep safe)
        if(mVScrollBar->SizeY() != static_cast<uint32_t>(sbH))
          const_cast<AScrollBar*>(mVScrollBar.get())->Resize(mVScrollBar->SizeX(), static_cast<uint32_t>(sbH));
// Pass table absolute offset so it draws at the correct screen position
        const_cast<AScrollBar*>(mVScrollBar.get())->OnDraw(buffer, bufferW, bufferH, absX, absY, clipL, clipT, clipR,
            clipB);
      }
    }
// ---- Draw horizontal scrollbar ----
    if(mHScrollBar && mHScrollBar->Visible()) {
      int32_t sbW = static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mRowHeaderWidth);
      if(mVScrollBar && mVScrollBar->Visible())
        sbW -= static_cast<int32_t>(mVScrollBar->SizeX());
      if(sbW > 0) {
        if(mHScrollBar->SizeX() != static_cast<uint32_t>(sbW))
          const_cast<AScrollBar*>(mHScrollBar.get())->Resize(static_cast<uint32_t>(sbW), mHScrollBar->SizeY());
        const_cast<AScrollBar*>(mHScrollBar.get())->OnDraw(buffer, bufferW, bufferH, absX, absY, clipL, clipT, clipR,
            clipB);
      }
    }
  }

  void ATable::DrawCells(UNUSED uint32_t* buffer, UNUSED uint32_t bufferW, UNUSED uint32_t bufferH,
  UNUSED int32_t offsetX, UNUSED int32_t offsetY,
  UNUSED int32_t clipL, UNUSED int32_t clipT, UNUSED int32_t clipR, UNUSED int32_t clipB,
      const ATableRangeData2& rowRange, const ATableRangeData2& colRange) const {
    AUI* au = Wnd() ? Wnd()->EnginePtr() : nullptr;
    if(!au)
      return;
    FT_Face face = au->DefaultFontFace();
    if(!face)
      return;
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    int32_t clientAbsX = offsetX + mX + static_cast<int32_t>(mRowHeaderWidth);
    int32_t clientAbsY = offsetY + mY + static_cast<int32_t>(mColumnHeaderHeight);
    int32_t clientW = static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mRowHeaderWidth);
    int32_t clientH = static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mColumnHeaderHeight);
    int32_t drawL = std::max(clipL, clientAbsX);
    int32_t drawT = std::max(clipT, clientAbsY);
    int32_t drawR = std::min(clipR, clientAbsX + clientW);
    int32_t drawB = std::min(clipB, clientAbsY + clientH);
    if(drawL >= drawR || drawT >= drawB)
      return;
    auto rowStartIt = std::lower_bound(mRowIds.begin(), mRowIds.end(), rowRange.cell);
    if(rowStartIt == mRowIds.end())
      return;
    size_t rowIdx = static_cast<size_t>(rowStartIt - mRowIds.begin());
    auto colStartIt = std::lower_bound(mColIds.begin(), mColIds.end(), colRange.cell);
    if(colStartIt == mColIds.end())
      return;
    size_t colStartIdx = static_cast<size_t>(colStartIt - mColIds.begin());
    int64_t yPos = static_cast<int64_t>(clientAbsY) - rowRange.offset;
    for(size_t ri = rowIdx; ri < mRowIds.size() && mRowIds[ri] <= rowRange.cell2; ++ri) {
      int64_t rowId = mRowIds[ri];
      auto rowIt = mRowH.find(rowId);
      if(rowIt == mRowH.end())
        continue;
      int64_t rowH = rowIt->second.first;
      int64_t rowTop = yPos;
      int64_t rowBottom = yPos + rowH;
      if(rowBottom < drawT) {
        yPos += rowH;
        continue;
      }
      if(rowTop > drawB)
        break;
      int64_t xPos = static_cast<int64_t>(clientAbsX) - colRange.offset;
      for(size_t ci = colStartIdx; ci < mColIds.size() && mColIds[ci] <= colRange.cell2; ++ci) {
        int64_t colId = mColIds[ci];
        auto colIt = mColumnW.find(colId);
        if(colIt == mColumnW.end())
          continue;
        int64_t colW = colIt->second.first;
        int64_t colLeft = xPos;
        int64_t colRight = xPos + colW;
        if(colRight < drawL) {
          xPos += colW;
          continue;
        }
        if(colLeft > drawR)
          break;
        int32_t cellL = std::max(static_cast<int32_t>(colLeft), drawL);
        int32_t cellT = std::max(static_cast<int32_t>(rowTop), drawT);
        int32_t cellR = std::min(static_cast<int32_t>(colRight), drawR);
        int32_t cellB = std::min(static_cast<int32_t>(rowBottom), drawB);
        if(cellL < cellR && cellT < cellB) {
          uint32_t bgColor = mBGColor;
          if(mRowSelectMode && rowId == mSelectedRow) {
            bgColor = mSelectionColor;
          }
          else
            if(!mRowSelectMode && rowId == mCursorRow && colId == mCursorCol) {
              bgColor = mSelectionColor;
            }
          FillRect(buffer, bufferW, cellL, cellT, cellR - cellL, cellB - cellT, bgColor);
          uint64_t cellKey = MakeCellKey(rowId, colId);
          auto cellIt = mCells.find(cellKey);
          if(cellIt != mCells.end() && !cellIt->second.data.empty()) {
            int32_t tx = static_cast<int32_t>(colLeft) + 2;
            int32_t ty = static_cast<int32_t>(rowTop);
            uint32_t tw = static_cast<uint32_t>(std::max<int64_t>(0, colW - 4));
            uint32_t th = static_cast<uint32_t>(rowH);
            ARect textBounds { tx, ty, tw, th };
            int32_t cellClipL = std::max(static_cast<int32_t>(colLeft), drawL);
            int32_t cellClipT = std::max(static_cast<int32_t>(rowTop), drawT);
            int32_t cellClipR = std::min(static_cast<int32_t>(colRight), drawR);
            int32_t cellClipB = std::min(static_cast<int32_t>(rowBottom), drawB);
            if(cellClipL < cellClipR && cellClipT < cellClipB) {
              ARect cellClipBounds { cellClipL, cellClipT, static_cast<uint32_t>(cellClipR - cellClipL),
                  static_cast<uint32_t>(cellClipB - cellClipT) };
              ATextStyle style { mTextColor, mFontSize, cellIt->second.hAlign, AUIVAlign::center, 0.0 };
              DrawTextEx(buffer, bufferW, bufferH, textBounds, cellIt->second.data, face, style, &cellClipBounds);
            }
            int32_t gridX = static_cast<int32_t>(colRight - 1);
            if(gridX >= cellL && gridX < cellR) {
              DrawVLine(buffer, bufferW, gridX, cellT, cellB - cellT, mGridColor);
            }
            int32_t gridY = static_cast<int32_t>(rowBottom - 1);
            if(gridY >= cellT && gridY < cellB) {
              DrawHLine(buffer, bufferW, cellL, gridY, cellR - cellL, mGridColor);
            }

          }
        }
        xPos += colW;
      }
      yPos += rowH;
    }
  }

  void ATable::DrawColumnHeader(UNUSED uint32_t* buffer, UNUSED uint32_t bufferW, UNUSED uint32_t bufferH,
  UNUSED int32_t offsetX, UNUSED int32_t offsetY,
  UNUSED int32_t clipL, UNUSED int32_t clipT, UNUSED int32_t clipR, UNUSED int32_t clipB,
      const ATableRangeData1& colStart, const ATableRangeData1& colEnd) const {
    AUI* au = Wnd() ? Wnd()->EnginePtr() : nullptr;
    if(!au)
      return;
    FT_Face face = au->DefaultFontFace();
    if(!face)
      return;
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    int32_t headerAbsX = offsetX + mX + static_cast<int32_t>(mRowHeaderWidth);
    int32_t headerAbsY = offsetY + mY;
    int32_t headerW = static_cast<int32_t>(mSizeX) - static_cast<int32_t>(mRowHeaderWidth);
    int32_t headerH = static_cast<int32_t>(mColumnHeaderHeight);
    int32_t drawL = std::max(clipL, headerAbsX);
    int32_t drawT = std::max(clipT, headerAbsY);
    int32_t drawR = std::min(clipR, headerAbsX + headerW);
    int32_t drawB = std::min(clipB, headerAbsY + headerH);
    if(drawL >= drawR || drawT >= drawB)
      return;
    int64_t xPos = static_cast<int64_t>(headerAbsX) - colStart.offset;
    auto it = mColumnW.find(colStart.cell);
    if(it == mColumnW.end())
      return;
    for(int64_t col = colStart.cell; col <= colEnd.cell && it != mColumnW.end(); ++it, col = it->first) {
      int64_t colW = it->second.first;
      int64_t colLeft = xPos;
      int64_t colRight = xPos + colW;
      if(colRight < drawL) {
        xPos += colW;
        continue;
      }
      if(colLeft > drawR)
        break;
      int32_t cellL = std::max(static_cast<int32_t>(colLeft), drawL);
      int32_t cellT = drawT;
      int32_t cellR = std::min(static_cast<int32_t>(colRight), drawR);
      int32_t cellB = drawB;
      if(cellL < cellR && cellT < cellB) {
        FillRect(buffer, bufferW, cellL, cellT, cellR - cellL, cellB - cellT, mHeaderBGColor);
        int32_t gridX = static_cast<int32_t>(colRight - 1);
        if(gridX >= cellL && gridX < cellR) {
          DrawVLine(buffer, bufferW, gridX, cellT, cellB - cellT, mGridColor);
        }
        int32_t bottomY = headerAbsY + headerH - 1;
        if(bottomY >= cellT && bottomY < cellB) {
          DrawHLine(buffer, bufferW, cellL, bottomY, cellR - cellL, mGridColor);
        }
        std::string label = it->second.second;
        if(!label.empty()) {
          ARect textBounds { static_cast<int32_t>(colLeft), headerAbsY, static_cast<uint32_t>(colW),
              static_cast<uint32_t>(headerH) };
          ARect cellClipBounds { cellL, cellT, static_cast<uint32_t>(cellR - cellL), static_cast<uint32_t>(cellB - cellT) };
          ATextStyle style { mHeaderTextColor, mFontSize, AUIHAlign::center, AUIVAlign::center, 0.0 };
          DrawTextEx(buffer, bufferW, bufferH, textBounds, label, face, style, &cellClipBounds);
        }
      }
      xPos += colW;
    }
  }

  void ATable::DrawRowHeader(UNUSED uint32_t* buffer, UNUSED uint32_t bufferW, UNUSED uint32_t bufferH,
  UNUSED int32_t offsetX, UNUSED int32_t offsetY,
  UNUSED int32_t clipL, UNUSED int32_t clipT, UNUSED int32_t clipR, UNUSED int32_t clipB,
      const ATableRangeData1& rowStart, const ATableRangeData1& rowEnd) const {
    AUI* au = Wnd() ? Wnd()->EnginePtr() : nullptr;
    if(!au)
      return;
    FT_Face face = au->DefaultFontFace();
    if(!face)
      return;
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    int32_t headerAbsX = offsetX + mX;
    int32_t headerAbsY = offsetY + mY + static_cast<int32_t>(mColumnHeaderHeight);
    int32_t headerW = static_cast<int32_t>(mRowHeaderWidth);
    int32_t headerH = static_cast<int32_t>(mSizeY) - static_cast<int32_t>(mColumnHeaderHeight);
    int32_t drawL = std::max(clipL, headerAbsX);
    int32_t drawT = std::max(clipT, headerAbsY);
    int32_t drawR = std::min(clipR, headerAbsX + headerW);
    int32_t drawB = std::min(clipB, headerAbsY + headerH);
    if(drawL >= drawR || drawT >= drawB)
      return;
    int64_t yPos = static_cast<int64_t>(headerAbsY) - rowStart.offset;
    auto it = mRowH.find(rowStart.cell);
    if(it == mRowH.end())
      return;
    for(int64_t row = rowStart.cell; row <= rowEnd.cell && it != mRowH.end(); ++it, row = it->first) {
      int64_t rowH = it->second.first;
      int64_t rowTop = yPos;
      int64_t rowBottom = yPos + rowH;
      if(rowBottom < drawT) {
        yPos += rowH;
        continue;
      }
      if(rowTop > drawB)
        break;
      int32_t cellL = drawL;
      int32_t cellT = std::max(static_cast<int32_t>(rowTop), drawT);
      int32_t cellR = drawR;
      int32_t cellB = std::min(static_cast<int32_t>(rowBottom), drawB);
      if(cellL < cellR && cellT < cellB) {
        FillRect(buffer, bufferW, cellL, cellT, cellR - cellL, cellB - cellT, mHeaderBGColor);
        int32_t rightX = headerAbsX + headerW - 1;
        if(rightX >= cellL && rightX < cellR) {
          DrawVLine(buffer, bufferW, rightX, cellT, cellB - cellT, mGridColor);
        }
        int32_t gridY = static_cast<int32_t>(rowBottom - 1);
        if(gridY >= cellT && gridY < cellB) {
          DrawHLine(buffer, bufferW, cellL, gridY, cellR - cellL, mGridColor);
        }
        std::string label = it->second.second;
        if(!label.empty()) {
          ARect textBounds { headerAbsX, static_cast<int32_t>(rowTop), static_cast<uint32_t>(headerW - 4),
              static_cast<uint32_t>(rowH) };
          ARect clipBounds { drawL, drawT, static_cast<uint32_t>(drawR - drawL), static_cast<uint32_t>(drawB - drawT) };
          ATextStyle style { mHeaderTextColor, mFontSize, AUIHAlign::right, AUIVAlign::center, 0.0 };
          DrawTextEx(buffer, bufferW, bufferH, textBounds, label, face, style, &clipBounds);
        }
      }
      yPos += rowH;
    }
  }

  void ATable::DrawIntersectionBox(UNUSED uint32_t* buffer, UNUSED uint32_t bufferW, UNUSED uint32_t bufferH,
  UNUSED int32_t offsetX, UNUSED int32_t offsetY,
  UNUSED int32_t clipL, UNUSED int32_t clipT, UNUSED int32_t clipR, UNUSED int32_t clipB) const {
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    int32_t boxW = static_cast<int32_t>(mRowHeaderWidth);
    int32_t boxH = static_cast<int32_t>(mColumnHeaderHeight);
    int32_t drawL = std::max(clipL, absX);
    int32_t drawT = std::max(clipT, absY);
    int32_t drawR = std::min(clipR, absX + boxW);
    int32_t drawB = std::min(clipB, absY + boxH);
    if(drawL >= drawR || drawT >= drawB)
      return;
    uint32_t bgColor = 0xFFCCCCCC;
    FillRect(buffer, bufferW, drawL, drawT, drawR - drawL, drawB - drawT, bgColor);
//      DrawRectBorder(buffer, bufferW, drawL, drawT, drawR - drawL, drawB - drawT, 0xFF888888);
  }

}// namespace aui
