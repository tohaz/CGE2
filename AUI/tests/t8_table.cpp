#include "AUILib.h"

using namespace aui;

// ------------------------------------------------------------------
// Attachment and basic properties
// ------------------------------------------------------------------
int32_t test_table_attachment(AUI *au) {
  D1("test_table_attachment start");
  AWindow* w = au->MainWnd();
  ATable* table = ATable::AttachTo(w);
  TEST_ASSERT_NE(table, nullptr, 2);
  ABox *b = ABox::AttachTo(w);
  b->Resize(100, 100);
  ATable* table2 = ATable::AttachTo(b);
  TEST_ASSERT_NE(table2, nullptr, 2);
  table2->Resize(80, 80);
  TEST_ASSERT_EQ(table->Rows(), 0U, 3);
  TEST_ASSERT_EQ(table->Columns(), 0U, 4);
  TEST_ASSERT_EQ(table->HOffset(), 0LL, 5);
  TEST_ASSERT_EQ(table->VOffset(), 0LL, 6);
  D1("test_table_attachment passed");
  return 0;
}

// ------------------------------------------------------------------
// Adding rows and columns
// ------------------------------------------------------------------
int32_t test_table_add_rows_columns(AUI *au) {
  D1("test_table_add_rows_columns start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->AddRows(5);
  TEST_ASSERT_EQ(table->Rows(), 5U, 2);
  table->AddColumns(3);
  TEST_ASSERT_EQ(table->Columns(), 3U, 3);
  table->AddRows(2);
  TEST_ASSERT_EQ(table->Rows(), 7U, 4);
  table->AddColumns(1);
  TEST_ASSERT_EQ(table->Columns(), 4U, 5);
  D1("test_table_add_rows_columns passed");
  return 0;
}

// ------------------------------------------------------------------
// Setting and getting cell data
// ------------------------------------------------------------------
int32_t test_table_cell_data(AUI *au) {
  D1("test_table_cell_data start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->AddRows(3);
  table->AddColumns(3);
  table->CellData(0, 0, "Hello");
  table->CellData(1, 1, "World");
  table->CellData(2, 2, "!");
  TEST_ASSERT_EQ(table->GetCellData(0, 0), std::string("Hello"), 2);
  TEST_ASSERT_EQ(table->GetCellData(1, 1), std::string("World"), 3);
  TEST_ASSERT_EQ(table->GetCellData(2, 2), std::string("!"), 4);
  table->CellData(0, 0, "Hi");
  TEST_ASSERT_EQ(table->GetCellData(0, 0), std::string("Hi"), 5);
  TEST_ASSERT_EQ(table->GetCellData(5, 5), std::string(""), 6);
  D1("test_table_cell_data passed");
  return 0;
}

// ------------------------------------------------------------------
// Removing rows and columns
// ------------------------------------------------------------------
int32_t test_table_remove(AUI *au) {
  D1("test_table_remove start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->AddRows(5);
  table->AddColumns(4);
  table->CellData(2, 2, "Keep");
  table->RemoveRow(2);
  TEST_ASSERT_EQ(table->Rows(), 4U, 2);
  TEST_ASSERT_EQ(table->GetCellData(2, 2), std::string(""), 3);
  table->RemoveColumn(1);
  TEST_ASSERT_EQ(table->Columns(), 3U, 4);
  table->RemoveLastRow();
  TEST_ASSERT_EQ(table->Rows(), 3U, 5);
  table->RemoveLastColumn();
  TEST_ASSERT_EQ(table->Columns(), 2U, 6);
  D1("test_table_remove passed");
  return 0;
}

// ------------------------------------------------------------------
// Clear all data
// ------------------------------------------------------------------
int32_t test_table_clear(AUI *au) {
  D1("test_table_clear start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->AddRows(10);
  table->AddColumns(10);
  table->CellData(5, 5, "Data");
  table->Clear();
  TEST_ASSERT_EQ(table->Rows(), 0U, 2);
  TEST_ASSERT_EQ(table->Columns(), 0U, 3);
  TEST_ASSERT_EQ(table->GetCellData(5, 5), std::string(""), 4);
  D1("test_table_clear passed");
  return 0;
}

// ------------------------------------------------------------------
// Scrolling (mouse wheel)
// ------------------------------------------------------------------
int32_t test_table_scroll(AUI *au) {
  D1("test_table_scroll start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->Resize(400, 300);
  table->AddRows(50);
  table->AddColumns(20);
  int64_t initialV = table->VOffset();
  table->OnMouseWheel(-1);// scroll down
  int64_t newV = table->VOffset();
  TEST_ASSERT(newV > initialV, 2);
  table->OnMouseWheel(1);
  newV = table->VOffset();
  TEST_ASSERT(newV >= 0, 3);
  table->ScrollTo(100, 200);
  TEST_ASSERT_EQ(table->HOffset(), 100LL, 4);
  TEST_ASSERT_EQ(table->VOffset(), 200LL, 5);
  D1("test_table_scroll passed");
  return 0;
}

// ------------------------------------------------------------------
// Selection (cursor and row selection mode)
// ------------------------------------------------------------------
int32_t test_table_selection(AUI *au) {
  D1("test_table_selection start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->AddRows(5);
  table->AddColumns(5);
  table->CursorPosition(2, 2);
  TEST_ASSERT_EQ(table->CursorRow(), 2LL, 2);
  TEST_ASSERT_EQ(table->CursorCol(), 2LL, 3);
  table->RowSelectMode(true);
  table->CursorPosition(3, 1);
  TEST_ASSERT_EQ(table->SelectedRow(), 3LL, 4);
  TEST_ASSERT_EQ(table->CursorRow(), 3LL, 5);
  D1("test_table_selection passed");
  return 0;
}

// ------------------------------------------------------------------
// Scrollbars enabled/disabled
// ------------------------------------------------------------------
int32_t test_table_scrollbars(AUI *au) {
  D1("test_table_scrollbars start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->ScrollbarsToggle(true);
  TEST_ASSERT(table->AreScrollbarsEnabled() == true, 2);
  AScrollBar* vbar = table->VScrollBar();
  AScrollBar* hbar = table->HScrollBar();
  TEST_ASSERT_NE(vbar, nullptr, 3);
  TEST_ASSERT_NE(hbar, nullptr, 3);
  table->ScrollbarsToggle(false);
  TEST_ASSERT(table->AreScrollbarsEnabled() == false, 4);
  D1("test_table_scrollbars passed");
  return 0;
}

// ------------------------------------------------------------------
// Resize and auto-widen column
// ------------------------------------------------------------------
int32_t test_table_auto_widen(AUI *au) {
  D1("test_table_auto_widen start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->AutoWiden(true);
  table->AddColumns(1);
  table->CellData(0, 0, "This is a very long text that should widen the column");
  table->AutoWidenColumn(0);
  D1("test_table_auto_widen passed");
  return 0;
}

// ------------------------------------------------------------------
// Callback on cell click (via mouse simulation)
// ------------------------------------------------------------------
int32_t test_table_cell_click_callback(AUI *au) {
  D1("test_table_cell_click_callback start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->Resize(300, 200);
  table->AddRows(3);
  table->AddColumns(3);
  table->HeaderWidth(60);
  table->HeaderHeight(24);
// Default column width 80, default row height 24
  int32_t clickX = 60 + 80 + 40;// inside second column
  int32_t clickY = 24 + 24 + 12;// inside second row
  table->MouseClick(clickX, clickY);
  TEST_ASSERT(table->CursorRow() != -1, 2);
  D1("test_table_cell_click_callback passed");
  return 0;
}
// ------------------------------------------------------------------
// Helper to set up a table for resize tests
// ------------------------------------------------------------------
UNUSED static void setup_resize_test_table(ATable *table) {
  table->HeaderWidth(60);
  table->HeaderHeight(24);
  table->Resize(400, 300);
  table->AddRows(5);
  table->AddColumns(4);
  table->RowHeight(0, 30);
  table->RowHeight(1, 40);
  table->RowHeight(2, 25);
  table->RowHeight(3, 35);
  table->RowHeight(4, 28);
  table->ColumnWidth(0, 100);
  table->ColumnWidth(1, 120);
  table->ColumnWidth(2, 90);
  table->ColumnWidth(3, 110);
  table->LayoutUpdate();
  table->ScrollTo(0, 0);
}
// ------------------------------------------------------------------
// Row/Column resizing via mouse drag
// ------------------------------------------------------------------
int32_t test_table_resize_column(AUI *au) {
  D1("test_table_resize_column start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  setup_resize_test_table(table);
// separator between column 1 and 2: x = 60+100+120 = 280, y in header
  int32_t sepX = 280;
  int32_t sepY = 12;
  AWidget* handled = table->MouseDown(sepX, sepY);
  TEST_ASSERT_NE(handled, nullptr, 2);
  table->MouseMove(sepX + 15, sepY);
  TEST_ASSERT_EQ(table->ColumnWidth(1), 135LL, 3);
  TEST_ASSERT_EQ(table->TotalContentWidth(), 100 + 135 + 90 + 110, 4);
  table->MouseMove(sepX + 5, sepY);
  TEST_ASSERT_EQ(table->ColumnWidth(1), 125LL, 5);
  table->MouseClick(sepX + 5, sepY);
  TEST_ASSERT_EQ(table->ColumnWidth(1), 125LL, 6);
  D1("test_table_resize_column passed");
  return 0;
}

int32_t test_table_resize_row(AUI *au) {
  D1("test_table_resize_row start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  setup_resize_test_table(table);
// separator between row 1 and 2: y = 24+30+40 = 94, x in row header
  int32_t sepX = 30;
  int32_t sepY = 94;
  table->MouseDown(sepX, sepY);
  table->MouseMove(sepX, sepY + 20);
  TEST_ASSERT_EQ(table->RowHeight(1), 60LL, 2);
  TEST_ASSERT_EQ(table->TotalContentHeight(), 30 + 60 + 25 + 35 + 28, 3);
  table->MouseMove(sepX, sepY + 10);
  TEST_ASSERT_EQ(table->RowHeight(1), 50LL, 4);
  table->MouseUp(sepX, sepY + 10);
  TEST_ASSERT_EQ(table->RowHeight(1), 50LL, 5);
  D1("test_table_resize_row passed");
  return 0;
}

int32_t test_table_resize_min_size(AUI *au) {
  D1("test_table_resize_min_size start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  setup_resize_test_table(table);
// default minimum is 10
  int32_t sepX = 280, sepY = 12;
  table->MouseDown(sepX, sepY);
  table->MouseMove(sepX - 200, sepY);// would be negative, clamped to 10
  TEST_ASSERT_EQ(table->ColumnWidth(1), 10LL, 2);
  table->MouseMove(sepX - 300, sepY);
  TEST_ASSERT_EQ(table->ColumnWidth(1), 10LL, 3);
  table->MouseUp(sepX - 300, sepY);
  sepX = 30;
  sepY = 94;
  table->MouseDown(sepX, sepY);
  table->MouseMove(sepX, sepY - 100);
  TEST_ASSERT_EQ(table->RowHeight(1), 10LL, 4);
  table->MouseUp(sepX, sepY - 100);
  D1("test_table_resize_min_size passed");
  return 0;
}

int32_t test_table_resize_with_scroll(AUI *au) {
  D1("test_table_resize_with_scroll start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  setup_resize_test_table(table);
// Use horizontal scroll only (no vertical scroll) for reliable row resizing
  table->ScrollTo(60, 0);// mHOffset = 60, mVOffset = 0
// Column resize with horizontal scroll
// Column separator between col 1 and 2:
// x = rowHeaderWidth - hOffset + col0 + col1 = 60 - 60 + 100 + 120 = 220
  int32_t sepX = 220, sepY = 12;
  table->MouseDown(sepX, sepY);
  table->MouseMove(sepX + 10, sepY);
  TEST_ASSERT_EQ(table->ColumnWidth(1), 130LL, 2);// 120 → 130
  table->MouseUp(sepX + 10, sepY);
// Row resize without vertical scroll
// Row separator between row 1 and 2: y = colHeaderHeight + row0 + row1 = 24+30+40 = 94
  sepX = 30;
  sepY = 94;
  table->MouseDown(sepX, sepY);
  table->MouseMove(sepX, sepY + 15);
  TEST_ASSERT_EQ(table->RowHeight(1), 55LL, 3);// 40 → 55
  table->MouseUp(sepX, sepY + 15);
  D1("test_table_resize_with_scroll passed");
  return 0;
}

int32_t test_table_resize_no_separator_on_last(AUI* au) {
  D1("test_table_resize_column_and_row start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  setup_resize_test_table(table);
  const int32_t rowHdrW = static_cast<int32_t>(table->RowHeaderWidth());
  const int32_t colHdrH = static_cast<int32_t>(table->ColumnHeaderHeight());
// ==========================================
// PART 1: COLUMN RESIZING
// ==========================================
// Header Y coordinate for column hit testing
  int32_t colHeaderY = colHdrH / 2;
// 1a. Valid Column Resizing (Separator 0)
  int64_t origCol0W = table->ColumnWidth(0);
  int32_t col0SepX = rowHdrW + static_cast<int32_t>(origCol0W);
  AWidget* handled = table->MouseDown(col0SepX, colHeaderY);
  TEST_ASSERT_EQ(handled, table, 101);// Table consumed mouse down
  table->MouseMove(col0SepX + 15, colHeaderY);
  TEST_ASSERT_EQ(table->ColumnWidth(0), origCol0W + 15, 102);// Width increased
  table->MouseUp(col0SepX + 15, colHeaderY);// Release drag
// 1b. No-Resize on Outer Right Edge (Last Column Boundary)
  int32_t totalColW = 0;
  int64_t lastColId = -1;
  for(size_t colIdx = 0; colIdx < table->Columns(); ++colIdx) {
    totalColW += static_cast<int32_t>(table->ColumnWidth(SafeUINT32(colIdx)));
    lastColId = SafeINT64(colIdx);
  }
  int32_t lastColRightX = rowHdrW + totalColW;
  int64_t origLastColW = table->ColumnWidth(lastColId);
  handled = table->MouseDown(lastColRightX, colHeaderY);
  TEST_ASSERT_EQ(handled, nullptr, 103);// No separator on outer edge
  table->MouseMove(lastColRightX + 15, colHeaderY);
  TEST_ASSERT_EQ(table->ColumnWidth(lastColId), origLastColW, 104);// Width unchanged
  table->MouseUp(lastColRightX + 15, colHeaderY);
// ==========================================
// PART 2: ROW RESIZING
// ==========================================
// Header X coordinate for row hit testing
  int32_t rowHeaderX = rowHdrW / 2;
// 2a. Valid Row Resizing (Separator 0)
  int64_t origRow0H = table->RowHeight(0);
  int32_t row0SepY = colHdrH + static_cast<int32_t>(origRow0H);
  handled = table->MouseDown(rowHeaderX, row0SepY);
  TEST_ASSERT_EQ(handled, table, 201);// Table consumed mouse down
  table->MouseMove(rowHeaderX, row0SepY + 20);
  TEST_ASSERT_EQ(table->RowHeight(0), origRow0H + 20, 202);// Height increased
  table->MouseUp(rowHeaderX, row0SepY + 20);// Release drag
// 2b. No-Resize on Outer Bottom Edge (Last Row Boundary)
  int32_t totalRowH = 0;
  int64_t lastRowId = -1;
  for(size_t rowIdx = 0; rowIdx < table->Rows(); ++rowIdx) {
    totalRowH += static_cast<int32_t>(table->RowHeight(SafeUINT32(rowIdx)));
    lastRowId = SafeINT64(rowIdx);
  }
  int32_t lastRowBottomY = colHdrH + totalRowH;
  int64_t origLastRowH = table->RowHeight(lastRowId);
  handled = table->MouseDown(rowHeaderX, lastRowBottomY);
  TEST_ASSERT_EQ(handled, nullptr, 203);// No separator on outer edge
  table->MouseMove(rowHeaderX, lastRowBottomY + 20);
  TEST_ASSERT_EQ(table->RowHeight(lastRowId), origLastRowH, 204);// Height unchanged
  table->MouseUp(rowHeaderX, lastRowBottomY + 20);
  D1("test_table_resize_column_and_row passed");
  return 0;
}

int32_t test_table_resize_click_outside_header(AUI *au) {
  D1("test_table_resize_click_outside_header start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  setup_resize_test_table(table);
  int64_t origW = table->ColumnWidth(1);
  table->MouseDown(200, 100);
  table->MouseMove(210, 100);
  TEST_ASSERT_EQ(table->ColumnWidth(1), origW, 2);
  table->MouseUp(210, 100);
  table->MouseDown(150, 12);
  table->MouseMove(160, 12);
  TEST_ASSERT_EQ(table->ColumnWidth(1), origW, 3);
  table->MouseUp(160, 12);
  int64_t origH = table->RowHeight(1);
  table->MouseDown(30, 60);
  table->MouseMove(30, 70);
  TEST_ASSERT_EQ(table->RowHeight(1), origH, 4);
  table->MouseUp(30, 70);
  D1("test_table_resize_click_outside_header passed");
  return 0;
}
// ------------------------------------------------------------------
// Performance test (render 10 frames)
// ------------------------------------------------------------------
int32_t test_table_render_performance(AUI *au) {
  D1("test_table_render_performance start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->AddRows(5000);
  table->AddColumns(100);
  D1("test_table_render_performance 1");
// Fill visible window space
  for(int32_t i = 0; i < 50; ++i) {
    table->CellData(i, i % 10, "PerfTest");
  }
  D1("test_table_render_performance 2");
// Define exact canvas boundary parameters
  uint32_t width = 800;
  uint32_t height = 600;
// Safely allocate a buffer matching the 800x600 resolution (480,000 pixels)
  std::vector<uint32_t> isolatedBuffer(width * height, 0xFF000000);
  D1("test_table_render_performance 3");
  auto start = std::chrono::high_resolution_clock::now();
// Simulate 10 frames of rendering/layout lookups safely
  for(int i = 0; i < 10; ++i) {
    table->Wnd()->Draw();
  }
  auto end = std::chrono::high_resolution_clock::now();
  D1("test_table_render_performance 4");
  std::chrono::duration<double, std::milli> elapsed = end - start;
  D1("Rendered 10 frames in {} ms", elapsed.count());
// Adjusted assertion to match the stress workload scale target
  TEST_ASSERT(elapsed.count() < 300, 2);
  D1("test_table_render_performance passed");
  return 0;
}
int32_t test_table_scrollbar_click(AUI *au) {
  D1("test_table_scrollbar_click start");
  // Good important test. It shows that slave scrollbars use correct coordinate system
  AWindow* win = au->MainWnd();
  win->EnableResize();
  win->Resize(500, 400);
  ABox* box = ABox::AttachTo(win);
  box->Move(20, 20);
  box->Resize(460, 360);
  ATable* table = ATable::AttachTo(box);
  table->Move(0, 0);
  table->Resize(460, 360);
  table->HeaderHeight(24);// <-- set known header height
  table->AddRows(100);
  table->AddColumns(5);
  table->ScrollbarsToggle(true);
  table->LayoutUpdate();
// Force a draw to populate scrollbar positions
  win->Draw();
  AScrollBar* vbar = table->VScrollBar();
//  TEST_ASSERT(vbar != nullptr && vbar->IsVisible(), 2);
  TEST_ASSERT_NE(vbar, nullptr, 2);
  TEST_ASSERT_EQ(vbar->Visible(), true, 2);
// Get actual scrollbar position (set during Draw)
  int32_t sbX = vbar->X();
  int32_t sbY = vbar->Y();
  int32_t sbW = (int32_t) vbar->SizeX();
  int32_t sbH = (int32_t) vbar->SizeY();
  D1("Scrollbar: pos=(%d,%d) size=(%d,%d)", sbX, sbY, sbW, sbH);
// Click near the bottom arrow (assume arrow size = 18)
  int32_t arrowSize = 18;
  int32_t clickX = sbX + sbW / 2;
  int32_t clickY = sbY + sbH - arrowSize / 2;// inside the arrow area
  D1("Clicking at local (%d,%d)", clickX, clickY);
  int64_t initialValue = vbar->Value();
// Simulate click (press and release)
  table->MouseClick(clickX, clickY);
  int64_t newValue = vbar->Value();
  D1("initial=%lld new=%lld", (long long)initialValue, (long long)newValue);
  TEST_ASSERT(newValue > initialValue, 3);
  D1("test_table_scrollbar_click passed");
  return 0;
}

int32_t test_callback_user_data(AUI *au) {
  D1("test_callback_user_data start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->Resize(200, 200);
  table->AddRows(5);
  table->AddColumns(5);
  int32_t testValue = 42;
  bool callbackCalled = false;
  table->SetMousePressLeftCallback([&](UNUSED AWidget* wi, UNUSED void *userData, UNUSED int32_t,
      UNUSED int32_t) noexcept -> AWidget* {
    int32_t* val = static_cast<int32_t*>(userData);
    if(val && *val == 42) {
      callbackCalled = true;
    }
    return wi;
  }, &testValue);
// Click on a cell (local coordinates: row header 40, col header 24, cell (1,1) at x=40+80+40, y=24+24+12)
  int32_t clickX = 40 + 80 + 40;// header width + column0 width + half of column1
  int32_t clickY = 24 + 24 + 12;
  table->MouseDown(clickX, clickY);
  table->MouseUp(clickX, clickY);
  TEST_ASSERT_EQ(callbackCalled, true, 2);
  D1("test_callback_user_data passed");
  return 0;
}

int32_t test_click_header_fires_callback(AUI *au) {
  D1("test_click_header_fires_callback start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->Resize(300, 200);
  table->AddRows(3);
  table->AddColumns(3);
  table->HeaderWidth(50);
  table->HeaderHeight(30);
  bool callbackFired = false;
  table->SetMousePressLeftCallback([&](AWidget* wi, void*, int32_t, int32_t) noexcept -> AWidget*{
    callbackFired = true;
    return wi;
  }, nullptr);
// Click on the column header (x = headerWidth + some column, y < headerHeight)
  int32_t clickX = 50 + 20;// inside first column header
  int32_t clickY = 10;// inside header area
  table->MouseClick(clickX, clickY);
  TEST_ASSERT_EQ(callbackFired, true, 2);
  D1("test_click_header_fires_callback passed");
  return 0;
}
//
// ------------------------------------------------------------------
// main
// ------------------------------------------------------------------
int main() {
  UNUSED int32_t testsfailed = 0;

  testsfailed += runTimedTest(test_table_attachment, 1);
  testsfailed += runTimedTest(test_table_add_rows_columns, 1);
  testsfailed += runTimedTest(test_table_cell_data, 1);
  testsfailed += runTimedTest(test_table_remove, 1);
  testsfailed += runTimedTest(test_table_clear, 1);
  testsfailed += runTimedTest(test_table_scroll, 1);
  testsfailed += runTimedTest(test_table_selection, 1);
  testsfailed += runTimedTest(test_table_scrollbars, 1);
  testsfailed += runTimedTest(test_table_auto_widen, 1);
  testsfailed += runTimedTest(test_table_cell_click_callback, 1);
  testsfailed += runTimedTest(test_table_resize_column, 1);
  testsfailed += runTimedTest(test_table_resize_row, 1);
  testsfailed += runTimedTest(test_table_resize_min_size, 1);
  testsfailed += runTimedTest(test_table_resize_with_scroll, 1);
  testsfailed += runTimedTest(test_table_resize_no_separator_on_last, 1);
  testsfailed += runTimedTest(test_table_resize_click_outside_header, 100);
  testsfailed += runTimedTest(test_table_render_performance, 1);
  testsfailed += runTimedTest(test_table_scrollbar_click, 1);
  testsfailed += runTimedTest(test_callback_user_data, 1);
  testsfailed += runTimedTest(test_click_header_fires_callback, 1);
//
  testsfailed += runTimedTest(test_table_attachment, 200);
  testsfailed += runTimedTest(test_table_add_rows_columns, 200);
  testsfailed += runTimedTest(test_table_cell_data, 200);
  testsfailed += runTimedTest(test_table_remove, 200);
  testsfailed += runTimedTest(test_table_clear, 200);
  testsfailed += runTimedTest(test_table_scroll, 200);
  testsfailed += runTimedTest(test_table_selection, 200);
  testsfailed += runTimedTest(test_table_scrollbars, 200);
  testsfailed += runTimedTest(test_table_auto_widen, 200);
  testsfailed += runTimedTest(test_table_cell_click_callback, 200);
  testsfailed += runTimedTest(test_table_resize_column, 200);
  testsfailed += runTimedTest(test_table_resize_row, 200);
  testsfailed += runTimedTest(test_table_resize_min_size, 200);
  testsfailed += runTimedTest(test_table_resize_with_scroll, 200);
  testsfailed += runTimedTest(test_table_resize_no_separator_on_last, 200);
  testsfailed += runTimedTest(test_table_resize_click_outside_header, 200);
  testsfailed += runTimedTest(test_table_render_performance, 1);
  testsfailed += runTimedTest(test_table_scrollbar_click, 200);
  testsfailed += runTimedTest(test_callback_user_data, 200);
  testsfailed += runTimedTest(test_click_header_fires_callback, 200);

  D("test suite complete");
  return testsfailed;
}
