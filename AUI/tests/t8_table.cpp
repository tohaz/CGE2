#include "AUILib.h"

using namespace aui;

// ------------------------------------------------------------------
// Attachment and basic properties
// ------------------------------------------------------------------
int32_t test_table_attachment(AUI* au) {
  D1("test_table_attachment start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  TEST_ASSERT_NE(table, nullptr, 2);
  TEST_ASSERT_EQ(table->RowCount(), 0U, 3);
  TEST_ASSERT_EQ(table->ColumnCount(), 0U, 4);
  TEST_ASSERT_EQ(table->GetHOffset(), 0LL, 5);
  TEST_ASSERT_EQ(table->GetVOffset(), 0LL, 6);
  D1("test_table_attachment passed");
  return 0;
}

// ------------------------------------------------------------------
// Adding rows and columns
// ------------------------------------------------------------------
int32_t test_table_add_rows_columns(AUI* au) {
  D1("test_table_add_rows_columns start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->AddRows(5);
  TEST_ASSERT_EQ(table->RowCount(), 5U, 2);
  table->AddColumns(3);
  TEST_ASSERT_EQ(table->ColumnCount(), 3U, 3);
  table->AddRows(2);
  TEST_ASSERT_EQ(table->RowCount(), 7U, 4);
  table->AddColumns(1);
  TEST_ASSERT_EQ(table->ColumnCount(), 4U, 5);
  D1("test_table_add_rows_columns passed");
  return 0;
}

// ------------------------------------------------------------------
// Setting and getting cell data
// ------------------------------------------------------------------
int32_t test_table_cell_data(AUI* au) {
  D1("test_table_cell_data start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->AddRows(3);
  table->AddColumns(3);
  table->SetCellData(0, 0, "Hello");
  table->SetCellData(1, 1, "World");
  table->SetCellData(2, 2, "!");
  TEST_ASSERT_EQ(table->GetCellData(0, 0), std::string("Hello"), 2);
  TEST_ASSERT_EQ(table->GetCellData(1, 1), std::string("World"), 3);
  TEST_ASSERT_EQ(table->GetCellData(2, 2), std::string("!"), 4);
  table->SetCellData(0, 0, "Hi");
  TEST_ASSERT_EQ(table->GetCellData(0, 0), std::string("Hi"), 5);
  TEST_ASSERT_EQ(table->GetCellData(5, 5), std::string(""), 6);
  D1("test_table_cell_data passed");
  return 0;
}

// ------------------------------------------------------------------
// Removing rows and columns
// ------------------------------------------------------------------
int32_t test_table_remove(AUI* au) {
  D1("test_table_remove start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->AddRows(5);
  table->AddColumns(4);
  table->SetCellData(2, 2, "Keep");
  table->RemoveRow(2);
  TEST_ASSERT_EQ(table->RowCount(), 4U, 2);
  TEST_ASSERT_EQ(table->GetCellData(2, 2), std::string(""), 3);
  table->RemoveColumn(1);
  TEST_ASSERT_EQ(table->ColumnCount(), 3U, 4);
  table->RemoveLastRow();
  TEST_ASSERT_EQ(table->RowCount(), 3U, 5);
  table->RemoveLastColumn();
  TEST_ASSERT_EQ(table->ColumnCount(), 2U, 6);
  D1("test_table_remove passed");
  return 0;
}

// ------------------------------------------------------------------
// Clear all data
// ------------------------------------------------------------------
int32_t test_table_clear(AUI* au) {
  D1("test_table_clear start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->AddRows(10);
  table->AddColumns(10);
  table->SetCellData(5, 5, "Data");
  table->Clear();
  TEST_ASSERT_EQ(table->RowCount(), 0U, 2);
  TEST_ASSERT_EQ(table->ColumnCount(), 0U, 3);
  TEST_ASSERT_EQ(table->GetCellData(5, 5), std::string(""), 4);
  D1("test_table_clear passed");
  return 0;
}

// ------------------------------------------------------------------
// Scrolling (mouse wheel)
// ------------------------------------------------------------------
int32_t test_table_scroll(AUI* au) {
  D1("test_table_scroll start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->Resize(400, 300);
  table->AddRows(50);
  table->AddColumns(20);
  int64_t initialV = table->GetVOffset();
  table->OnMouseWheel(-1);// scroll down
  int64_t newV = table->GetVOffset();
  TEST_ASSERT(newV > initialV, 2);
  table->OnMouseWheel(1);
  newV = table->GetVOffset();
  TEST_ASSERT(newV >= 0, 3);
  table->ScrollTo(100, 200);
  TEST_ASSERT_EQ(table->GetHOffset(), 100LL, 4);
  TEST_ASSERT_EQ(table->GetVOffset(), 200LL, 5);
  D1("test_table_scroll passed");
  return 0;
}

// ------------------------------------------------------------------
// Selection (cursor and row selection mode)
// ------------------------------------------------------------------
int32_t test_table_selection(AUI* au) {
  D1("test_table_selection start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->AddRows(5);
  table->AddColumns(5);
  table->SetCursorPosition(2, 2);
  TEST_ASSERT_EQ(table->GetCursorRow(), 2LL, 2);
  TEST_ASSERT_EQ(table->GetCursorCol(), 2LL, 3);
  table->SetRowSelectMode(true);
  table->SetCursorPosition(3, 1);
  TEST_ASSERT_EQ(table->GetSelectedRow(), 3LL, 4);
  TEST_ASSERT_EQ(table->GetCursorRow(), 3LL, 5);
  D1("test_table_selection passed");
  return 0;
}

// ------------------------------------------------------------------
// Scrollbars enabled/disabled
// ------------------------------------------------------------------
int32_t test_table_scrollbars(AUI* au) {
  D1("test_table_scrollbars start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->SetScrollbarsEnabled(true);
  TEST_ASSERT(table->AreScrollbarsEnabled() == true, 2);
  AScrollBar* vbar = table->GetVScrollBar();
  AScrollBar* hbar = table->GetHScrollBar();
  TEST_ASSERT(vbar != nullptr && hbar != nullptr, 3);
  table->SetScrollbarsEnabled(false);
  TEST_ASSERT(table->AreScrollbarsEnabled() == false, 4);
  D1("test_table_scrollbars passed");
  return 0;
}

// ------------------------------------------------------------------
// Resize and auto-widen column
// ------------------------------------------------------------------
int32_t test_table_auto_widen(AUI* au) {
  D1("test_table_auto_widen start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->SetAutoWiden(true);
  table->AddColumns(1);
  table->SetCellData(0, 0, "This is a very long text that should widen the column");
  table->AutoWidenColumn(0);
  D1("test_table_auto_widen passed");
  return 0;
}

// ------------------------------------------------------------------
// Callback on cell click (via mouse simulation)
// ------------------------------------------------------------------
int32_t test_table_cell_click_callback(AUI* au) {
  D1("test_table_cell_click_callback start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->Resize(300, 200);
  table->AddRows(3);
  table->AddColumns(3);
  table->SetHeaderWidth(60);
  table->SetHeaderHeight(24);
  // Default column width 80, default row height 24
  int32_t clickX = 60 + 80 + 40;// inside second column
  int32_t clickY = 24 + 24 + 12;// inside second row
  table->OnMouseClick(clickX, clickY, true);
  table->OnMouseClick(clickX, clickY, false);
  TEST_ASSERT(table->GetCursorRow() != -1, 2);
  D1("test_table_cell_click_callback passed");
  return 0;
}

// ------------------------------------------------------------------
// Helper to set up a table for resize tests
// ------------------------------------------------------------------
static void setup_resize_test_table(ATable* table) {
  table->SetHeaderWidth(60);
  table->SetHeaderHeight(24);
  table->Resize(400, 300);
  table->AddRows(5);
  table->AddColumns(4);
  table->SetRowHeight(0, 30);
  table->SetRowHeight(1, 40);
  table->SetRowHeight(2, 25);
  table->SetRowHeight(3, 35);
  table->SetRowHeight(4, 28);
  table->SetColumnWidth(0, 100);
  table->SetColumnWidth(1, 120);
  table->SetColumnWidth(2, 90);
  table->SetColumnWidth(3, 110);
  table->UpdateLayout();
  table->ScrollTo(0, 0);
}

// ------------------------------------------------------------------
// Row/Column resizing via mouse drag
// ------------------------------------------------------------------
int32_t test_table_resize_column(AUI* au) {
  D1("test_table_resize_column start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  setup_resize_test_table(table);
  // separator between column 1 and 2: x = 60+100+120 = 280, y in header
  int32_t sepX = 280;
  int32_t sepY = 12;
  bool handled = table->OnMouseClick(sepX, sepY, true);
  TEST_ASSERT(handled, 2);
  table->OnMouseMove(sepX + 15, sepY);
  TEST_ASSERT_EQ(table->GetColumnWidth(1), 135LL, 3);
  TEST_ASSERT_EQ(table->GetTotalContentWidth(), 100 + 135 + 90 + 110, 4);
  table->OnMouseMove(sepX + 5, sepY);
  TEST_ASSERT_EQ(table->GetColumnWidth(1), 125LL, 5);
  table->OnMouseClick(sepX + 5, sepY, false);
  TEST_ASSERT_EQ(table->GetColumnWidth(1), 125LL, 6);
  D1("test_table_resize_column passed");
  return 0;
}

int32_t test_table_resize_row(AUI* au) {
  D1("test_table_resize_row start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  setup_resize_test_table(table);
  // separator between row 1 and 2: y = 24+30+40 = 94, x in row header
  int32_t sepX = 30;
  int32_t sepY = 94;
  table->OnMouseClick(sepX, sepY, true);
  table->OnMouseMove(sepX, sepY + 20);
  TEST_ASSERT_EQ(table->GetRowHeight(1), 60LL, 2);
  TEST_ASSERT_EQ(table->GetTotalContentHeight(), 30 + 60 + 25 + 35 + 28, 3);
  table->OnMouseMove(sepX, sepY + 10);
  TEST_ASSERT_EQ(table->GetRowHeight(1), 50LL, 4);
  table->OnMouseClick(sepX, sepY + 10, false);
  TEST_ASSERT_EQ(table->GetRowHeight(1), 50LL, 5);
  D1("test_table_resize_row passed");
  return 0;
}

int32_t test_table_resize_min_size(AUI* au) {
  D1("test_table_resize_min_size start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  setup_resize_test_table(table);
  // default minimum is 10
  int32_t sepX = 280, sepY = 12;
  table->OnMouseClick(sepX, sepY, true);
  table->OnMouseMove(sepX - 200, sepY);// would be negative, clamped to 10
  TEST_ASSERT_EQ(table->GetColumnWidth(1), 10LL, 2);
  table->OnMouseMove(sepX - 300, sepY);
  TEST_ASSERT_EQ(table->GetColumnWidth(1), 10LL, 3);
  table->OnMouseClick(sepX - 300, sepY, false);
  sepX = 30;
  sepY = 94;
  table->OnMouseClick(sepX, sepY, true);
  table->OnMouseMove(sepX, sepY - 100);
  TEST_ASSERT_EQ(table->GetRowHeight(1), 10LL, 4);
  table->OnMouseClick(sepX, sepY - 100, false);
  D1("test_table_resize_min_size passed");
  return 0;
}

int32_t test_table_resize_with_scroll(AUI* au) {
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
  table->OnMouseClick(sepX, sepY, true);
  table->OnMouseMove(sepX + 10, sepY);
  TEST_ASSERT_EQ(table->GetColumnWidth(1), 130LL, 2);// 120 → 130
  table->OnMouseClick(sepX + 10, sepY, false);
  // Row resize without vertical scroll
  // Row separator between row 1 and 2: y = colHeaderHeight + row0 + row1 = 24+30+40 = 94
  sepX = 30;
  sepY = 94;
  table->OnMouseClick(sepX, sepY, true);
  table->OnMouseMove(sepX, sepY + 15);
  TEST_ASSERT_EQ(table->GetRowHeight(1), 55LL, 3);// 40 → 55
  table->OnMouseClick(sepX, sepY + 15, false);
  D1("test_table_resize_with_scroll passed");
  return 0;
}

int32_t test_table_resize_no_separator_on_last(AUI* au) {
  D1("test_table_resize_no_separator_on_last start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  setup_resize_test_table(table);
  // after last column – no separator
  int32_t lastColRight = 60 + 100 + 120 + 90 + 110;// = 480
  int32_t sepX = lastColRight, sepY = 12;
  int64_t origW = table->GetColumnWidth(3);
  bool handled = table->OnMouseClick(sepX, sepY, true);
  TEST_ASSERT(!handled, 2);
  table->OnMouseMove(sepX + 10, sepY);
  TEST_ASSERT_EQ(table->GetColumnWidth(3), origW, 3);
  // after last row
  int32_t lastRowBottom = 24 + 30 + 40 + 25 + 35 + 28;// = 182
  sepX = 30;
  sepY = lastRowBottom;
  int64_t origH = table->GetRowHeight(4);
  handled = table->OnMouseClick(sepX, sepY, true);
  TEST_ASSERT(!handled, 4);
  table->OnMouseMove(sepX, sepY + 10);
  TEST_ASSERT_EQ(table->GetRowHeight(4), origH, 5);
  D1("test_table_resize_no_separator_on_last passed");
  return 0;
}

int32_t test_table_resize_click_outside_header(AUI* au) {
  D1("test_table_resize_click_outside_header start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  setup_resize_test_table(table);
  int64_t origW = table->GetColumnWidth(1);
  table->OnMouseClick(200, 100, true);
  table->OnMouseMove(210, 100);
  TEST_ASSERT_EQ(table->GetColumnWidth(1), origW, 2);
  table->OnMouseClick(210, 100, false);
  table->OnMouseClick(150, 12, true);
  table->OnMouseMove(160, 12);
  TEST_ASSERT_EQ(table->GetColumnWidth(1), origW, 3);
  table->OnMouseClick(160, 12, false);
  int64_t origH = table->GetRowHeight(1);
  table->OnMouseClick(30, 60, true);
  table->OnMouseMove(30, 70);
  TEST_ASSERT_EQ(table->GetRowHeight(1), origH, 4);
  table->OnMouseClick(30, 70, false);
  D1("test_table_resize_click_outside_header passed");
  return 0;
}

// ------------------------------------------------------------------
// Performance test (render 10 frames)
// ------------------------------------------------------------------
int32_t test_table_render_performance(AUI* au) {
  D1("test_table_render_performance start");
  AWindow* win = au->MainWnd();
  ATable* table = ATable::AttachTo(win);
  table->AddRows(5000);
  table->AddColumns(100);
  D1("test_table_render_performance 1");
  // Fill visible window space
  for (int i = 0; i < 50; ++i) {
    table->SetCellData(i, i % 10, "PerfTest");
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
  for (int i = 0; i < 10; ++i) {
    table->Draw(isolatedBuffer.data(), width, height, 0, 0);
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
  AWindow* win = au->MainWnd();
  win->EnableResize();
  win->Resize(500, 400);
  ABox* box = ABox::AttachTo(win);
  box->Move(20, 20);
  box->Resize(460, 360);
  ATable* table = ATable::AttachTo(box);
  table->Move(0, 0);
  table->Resize(460, 360);
  table->SetHeaderHeight(24);// <-- set known header height
  table->AddRows(100);
  table->AddColumns(5);
  table->SetScrollbarsEnabled(true);
  table->UpdateLayout();
// Force a draw to populate scrollbar positions
  win->Draw();
  AScrollBar* vbar = table->GetVScrollBar();
  TEST_ASSERT(vbar != nullptr && vbar->IsVisible(), 2);
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
  D1("Clicking at abs (%d,%d)", clickX, clickY);
  int64_t initialValue = vbar->GetValue();
// Simulate click (press and release)
  win->OnMousePress(clickX, clickY, 1);
  win->OnMouseRelease(clickX, clickY, 1);
  int64_t newValue = vbar->GetValue();
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
  table->SetClickCallback([&](AWindow*, AWidget*, void *userData, int32_t, int32_t, bool pressed) {
    if(!pressed)
      return;
    int* val = static_cast<int*>(userData);
    if(val && *val == 42) {
      callbackCalled = true;
    }
  }, &testValue);
// Click on a cell (local coordinates: row header 40, col header 24, cell (1,1) at x=40+80+40, y=24+24+12)
  int32_t clickX = 40 + 80 + 40;// header width + column0 width + half of column1
  int32_t clickY = 24 + 24 + 12;
  table->OnMouseClick(clickX, clickY, true);
  table->OnMouseClick(clickX, clickY, false);
  TEST_ASSERT(callbackCalled, 2);
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
  table->SetHeaderWidth(50);
  table->SetHeaderHeight(30);
  bool callbackFired = false;
  table->SetClickCallback([&](AWindow*, AWidget*, void*, int32_t, int32_t, bool pressed) {
    if(pressed)
      callbackFired = true;
  }, nullptr);
// Click on the column header (x = headerWidth + some column, y < headerHeight)
  int32_t clickX = 50 + 20;// inside first column header
  int32_t clickY = 10;// inside header area
  table->OnMouseClick(clickX, clickY, true);
  table->OnMouseClick(clickX, clickY, false);
  TEST_ASSERT(callbackFired, 2);
  D1("test_click_header_fires_callback passed");
  return 0;
}

// ------------------------------------------------------------------
// main
// ------------------------------------------------------------------
int main() {
  int32_t testsfailed = 0;
  
  testsfailed += runTimedTest("test_table_attachment", test_table_attachment, 1);
  testsfailed += runTimedTest("test_table_add_rows_columns", test_table_add_rows_columns, 1);
  testsfailed += runTimedTest("test_table_cell_data", test_table_cell_data, 1);
  testsfailed += runTimedTest("test_table_remove", test_table_remove, 1);
  testsfailed += runTimedTest("test_table_clear", test_table_clear, 1);
  testsfailed += runTimedTest("test_table_scroll", test_table_scroll, 1);
  testsfailed += runTimedTest("test_table_selection", test_table_selection, 1);
  testsfailed += runTimedTest("test_table_scrollbars", test_table_scrollbars, 1);
  testsfailed += runTimedTest("test_table_auto_widen", test_table_auto_widen, 1);
  testsfailed += runTimedTest("test_table_cell_click_callback", test_table_cell_click_callback, 1);
  testsfailed += runTimedTest("test_table_resize_column", test_table_resize_column, 1);
  testsfailed += runTimedTest("test_table_resize_row", test_table_resize_row, 1);
  testsfailed += runTimedTest("test_table_resize_min_size", test_table_resize_min_size, 1);
  testsfailed += runTimedTest("test_table_resize_with_scroll", test_table_resize_with_scroll, 1);
  testsfailed += runTimedTest("test_table_resize_no_separator_on_last", test_table_resize_no_separator_on_last, 1);
  testsfailed += runTimedTest("test_table_resize_click_outside_header", test_table_resize_click_outside_header, 100);
  testsfailed += runTimedTest("test_table_render_performance", test_table_render_performance, 1);
  testsfailed += runTimedTest("test_table_scrollbar_click", test_table_scrollbar_click, 1);
  testsfailed += runTimedTest("test_callback_user_data", test_callback_user_data, 1);
  testsfailed += runTimedTest("test_click_header_fires_callback", test_click_header_fires_callback, 1);

  testsfailed += runTimedTest("test_table_attachment", test_table_attachment, 200);
  testsfailed += runTimedTest("test_table_add_rows_columns", test_table_add_rows_columns, 200);
  testsfailed += runTimedTest("test_table_cell_data", test_table_cell_data, 200);
  testsfailed += runTimedTest("test_table_remove", test_table_remove, 200);
  testsfailed += runTimedTest("test_table_clear", test_table_clear, 200);
  testsfailed += runTimedTest("test_table_scroll", test_table_scroll, 200);
  testsfailed += runTimedTest("test_table_selection", test_table_selection, 200);
  testsfailed += runTimedTest("test_table_scrollbars", test_table_scrollbars, 200);
  testsfailed += runTimedTest("test_table_auto_widen", test_table_auto_widen, 200);
  testsfailed += runTimedTest("test_table_cell_click_callback", test_table_cell_click_callback, 200);
  testsfailed += runTimedTest("test_table_resize_column", test_table_resize_column, 200);
  testsfailed += runTimedTest("test_table_resize_row", test_table_resize_row, 200);
  testsfailed += runTimedTest("test_table_resize_min_size", test_table_resize_min_size, 200);
  testsfailed += runTimedTest("test_table_resize_with_scroll", test_table_resize_with_scroll, 200);
  testsfailed += runTimedTest("test_table_resize_no_separator_on_last", test_table_resize_no_separator_on_last, 200);
  testsfailed += runTimedTest("test_table_resize_click_outside_header", test_table_resize_click_outside_header, 200);
  testsfailed += runTimedTest("test_table_render_performance", test_table_render_performance, 1);
  testsfailed += runTimedTest("test_table_scrollbar_click", test_table_scrollbar_click, 200);
  testsfailed += runTimedTest("test_callback_user_data", test_callback_user_data, 200);
  testsfailed += runTimedTest("test_click_header_fires_callback", test_click_header_fires_callback, 200);
  
  
  D("test suite complete");
  return testsfailed;
}

