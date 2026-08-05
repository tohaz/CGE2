#ifndef ATABLE_H_
#define ATABLE_H_

namespace aui {

// Cell data structure (same as old version)
  struct AUICellData {
      std::string data = "";
      AUIHAlign hAlign = AUIHAlign::center;
      AUIVAlign vAlign = AUIVAlign::center;
  };

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

  struct ATableRangeData1;
  struct ATableRangeData2;
  class ATable: public AWidgetFactory<ATable> {
      friend class AWidgetFactory<ATable>;
    private:
      ATable();
      void DrawCells(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
          int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB, const ATableRangeData2& rowRange,
          const ATableRangeData2& colRange) const;
      void DrawColumnHeader(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
          int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB, const ATableRangeData1& colStart,
          const ATableRangeData1& colEnd) const;
      void DrawIntersectionBox(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
          int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const;
      void DrawRowHeader(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
          int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB, const ATableRangeData1& rowStart,
          const ATableRangeData1& rowEnd) const;
      std::unordered_map<uint64_t, AUICellData> mCells;
      // Helper to safely pack two 32-bit IDs into one 64-bit lookup key
      inline uint64_t MakeCellKey(int64_t row, int64_t col) const {
        return (static_cast<uint64_t>(static_cast<uint32_t>(row)) << 32) | static_cast<uint32_t>(col);
      }
      std::map<int64_t, std::pair<int64_t, std::string>> mRowH;// row -> (height, label)
      std::map<int64_t, std::pair<int64_t, std::string>> mColumnW;// col -> (width, label)
      int64_t mHOffset = 0;// horizontal scroll offset (pixels)
      int64_t mVOffset = 0;// vertical scroll offset (pixels)
      uint64_t mTotalContentWidth = 0;
      uint64_t mTotalContentHeight = 0;
      uint32_t mColumnHeaderHeight = 24;
      uint32_t mRowHeaderWidth = 60;
      int64_t mCursorRow = -1;
      int64_t mCursorCol = -1;
      int64_t mSelectedRow = -1;// used in row‑select mode
      bool mRowSelectMode = false;// if true, clicking selects whole row
      enum class ATableResizeMode {
        None, Column, Row, Header
      };
      ATableResizeMode mResizeMode = ATableResizeMode::None;
      int64_t mResizeId = -1;
      int32_t mResizeBasePos = 0;
      int64_t mResizeBaseSize = 0;
      enum class ATableScrollMode {
        None, Vertical, Horizontal
      };
      ATableScrollMode mScrollMode = ATableScrollMode::None;
      int64_t mScrollGrabOffset = 0;
      uint32_t mGridColor = 0xFFDDDDDD;
      uint32_t mHeaderBGColor = 0xFFCCCCCC;
      uint32_t mHeaderTextColor = 0xFF000000;
      uint32_t mSelectionColor = 0xCCE5FF;
      uint32_t mCursorBorderColor = 0xFF3399FF;
      uint32_t mScrollbarColor = 0xFF888888;
      uint32_t mScrollbarThumbColor = 0xFFAAAAAA;
      uint32_t mScrollbarSize = 12;
      bool mAutoWiden = false;
      bool mAllowColumnResize = true;
      bool mAllowRowResize = true;
      bool mRowHeaderResizeEnabled = true;
      bool mRowHeightResizeEnabled = true;
      ATableRangeData1 Offset2Column(int64_t offset) const;
      ATableRangeData1 Offset2Row(int64_t offset) const;
      ATableRangeData1 Offset2ColumnRange(const ATableRangeData1 &start, int64_t width) const;
      ATableRangeData1 Offset2RowRange(const ATableRangeData1 &start, int64_t height) const;
      std::pair<int64_t, int64_t> ScreenToCell(int32_t x, int32_t y, int32_t offsetX, int32_t offsetY) const;
      int32_t MeasureTextWidth(const std::string &text) const;
      std::unique_ptr<AScrollBar> mVScrollBar;
      std::unique_ptr<AScrollBar> mHScrollBar;
      AScrollBar *mDragScrollbar = nullptr;
      bool mAutoHideScrollbars = true;
      void UpdateScrollbarRanges();// after data/size changes
      mutable std::vector<int64_t> mRowPrefix;// cumulative row heights (size = R+1)
      mutable std::vector<int64_t> mRowIds;// row IDs in key order (size = R)
      mutable bool mRowPrefixDirty = true;
      mutable std::vector<int64_t> mColPrefix;// cumulative column widths (size = C+1)
      mutable std::vector<int64_t> mColIds;// column IDs in key order (size = C)
      mutable bool mColPrefixDirty = true;
      void RebuildRowPrefix() const;
      void RebuildColPrefix() const;
      int32_t mBatchDepth = 0;
      std::unordered_map<int64_t, int32_t> mBatchRowToIdx;
      std::unordered_map<int64_t, int32_t> mBatchColToIdx;
      std::vector<int64_t> mBatchIdxToRow;
      std::vector<int64_t> mBatchIdxToCol;
      std::vector<std::vector<AUICellData>> mBatchCells;
// Resizing state
      bool mResizing = false;
      bool mResizeColumn = false;// true = column, false = row
      int64_t mResizeTargetId = -1;// row or column ID being resized
      int32_t mResizeStartMouse = 0;// initial mouse coordinate (X for col, Y for row)
      int32_t mResizeStartSize = 0;// original width/height before drag
      int32_t mResizeMinSize = 10;// minimum allowed size
      bool mResizeHover = false;// cursor over a separator (for feedback)
      int64_t mResizeHoverId = -1;// which separator under cursor
      bool mResizeHoverColumn = false;// orientation
/////////////////
      bool HitTestSeparator(int32_t localX, int32_t localY, bool &isColumn, int64_t &id) const;
    public:
      virtual void OnDraw(uint32_t *buffer, uint32_t bufferW, uint32_t bufferH,
          int32_t offsetX, int32_t offsetY, int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const;
      AWidget* OnMouseDownLeft(int32_t localX, int32_t localY) override;
      AWidget* OnMouseUpLeft(int32_t localX, int32_t localY) override;
      bool OnMouseMove(int32_t localX, int32_t localY) override;
      void OnMouseWheel(int32_t delta) override;
      void OnParentResize(uint32_t newWidth, uint32_t newHeight);
      void OnMouseLeave();
      void AddRow();
      void AddColumn();
      void RemoveRow(int64_t rowIdx);
      void RemoveColumn(int64_t colIdx);
      void RemoveLastRow();
      void RemoveLastColumn();
      void Clear();
      void CellData(int64_t row, int64_t col, const std::string &text, AUIHAlign hAlign = AUIHAlign::center);
      std::string GetCellData(int64_t row, int64_t col) const;
      AUICellData& GetOrCreateCell(int64_t row, int64_t col);// returns reference, creates if missing
      void RowLabel(int64_t row, const std::string &label);
      void ColumnLabel(int64_t col, const std::string &label);
      void RowHeight(int64_t row, int64_t height);
      void ColumnWidth(int64_t col, int64_t width);
      void AutoWidenColumn(int64_t col);// adjust width to fit content + header
      void ScrollToCell(int64_t row, int64_t col);
      int64_t HOffset() const {return mHOffset;}
      int64_t VOffset() const {return mVOffset;}
      void CursorPosition(int64_t row, int64_t col);
      void RowSelectMode(bool enable);
      int64_t SelectedRow() const {return mSelectedRow;}
      int64_t CursorRow() const {return mCursorRow;}
      int64_t CursorCol() const {return mCursorCol;}
      void HeaderHeight(uint32_t h) {mColumnHeaderHeight = h;}
      uint32_t HeaderHeight() {return mColumnHeaderHeight;}
      void HeaderWidth(uint32_t w) {mRowHeaderWidth = w;}
      void AutoWiden(bool enable) {mAutoWiden = enable;}
      void ColumnResize(bool enable) {mAllowColumnResize = enable;}
      void RowResize(bool enable) {mAllowRowResize = enable;}
      void RowHeaderResize(bool enable) {mRowHeaderResizeEnabled = enable;}
      void RowHeightResize(bool enable) {mRowHeightResizeEnabled = enable;}
      size_t Rows() const {return mRowH.size();}
      size_t Columns() const {return mColumnW.size();}
      void AddColumns(uint32_t number);
      void AddRows(uint32_t number);
      void ScrollbarsToggle(bool v);
      bool AreScrollbarsEnabled() const {return mVScrollBar && mHScrollBar;}
      void SetAutoHideScrollbars(bool enable) {
        mAutoHideScrollbars = enable;
        UpdateScrollbarRanges();
      }
      AScrollBar* VScrollBar() {return mVScrollBar.get();}
      AScrollBar* HScrollBar() {return mHScrollBar.get();}
      void ScrollTo(int32_t xOffset, int32_t yOffset);
      void GridColor(uint32_t color) {mGridColor = color;}
      void SelectionColor(uint32_t color) {mSelectionColor = color;}
      void CursorBorderColor(uint32_t color) {mCursorBorderColor = color;}
      uint32_t HeaderBgColor() const {return mHeaderBGColor;}
      uint32_t HeaderTextColor() const {return mHeaderTextColor;}
      void HeaderBGColor(uint32_t color) {mHeaderBGColor = color;}
      void HeaderTextColor(uint32_t color) {mHeaderTextColor = color;}
      void BeginBatch();
      void BeginBatch(uint32_t prealloc);
      void EndBatch();
      void LayoutUpdate();
      int64_t RowHeight(int64_t row) const {
        auto it = mRowH.find(row);
        return (it != mRowH.end()) ? it->second.first : -1;
      }
      int64_t ColumnWidth(int64_t col) const {
        auto it = mColumnW.find(col);
        return (it != mColumnW.end()) ? it->second.first : -1;
      }
      uint64_t TotalContentWidth() const {return mTotalContentWidth;}
      uint64_t TotalContentHeight() const {return mTotalContentHeight;}
      uint32_t RowHeaderWidth() {return mRowHeaderWidth;}
      uint32_t ColumnHeaderHeight() {return mColumnHeaderHeight;}
      void OnResize(uint32_t newWidth, uint32_t newHeight);


  };

}// namespace aui

#endif // ATABLE_H_

