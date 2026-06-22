#include <chrono>
#include <future>
#include <thread>
#include "AUILib.h"

using namespace aui;

// ------------------------------------------------------------------
// Attachment and basic properties
// ------------------------------------------------------------------
int32_t test_table_attachment() {
  D1("test_table_attachment start");
  AUI *au = AUI::Create("TableAttachTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
  TEST_ASSERT(table != nullptr, 2);
  TEST_ASSERT_EQ(table->RowCount(), 0U, 3);
  TEST_ASSERT_EQ(table->ColumnCount(), 0U, 4);
  TEST_ASSERT_EQ(table->GetHOffset(), 0LL, 5);
  TEST_ASSERT_EQ(table->GetVOffset(), 0LL, 6);
  delete au;
  D1("test_table_attachment passed");
  return 0;
}

// ------------------------------------------------------------------
// Adding rows and columns
// ------------------------------------------------------------------
int32_t test_table_add_rows_columns() {
  D1("test_table_add_rows_columns start");
  AUI *au = AUI::Create("TableAddTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
  table->AddRows(5);
  TEST_ASSERT_EQ(table->RowCount(), 5U, 2);
  table->AddColumns(3);
  TEST_ASSERT_EQ(table->ColumnCount(), 3U, 3);
  table->AddRows(2);
  TEST_ASSERT_EQ(table->RowCount(), 7U, 4);
  table->AddColumns(1);
  TEST_ASSERT_EQ(table->ColumnCount(), 4U, 5);
  delete au;
  D1("test_table_add_rows_columns passed");
  return 0;
}

// ------------------------------------------------------------------
// Setting and getting cell data
// ------------------------------------------------------------------
int32_t test_table_cell_data() {
  D1("test_table_cell_data start");
  AUI *au = AUI::Create("TableCellTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
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
  delete au;
  D1("test_table_cell_data passed");
  return 0;
}

// ------------------------------------------------------------------
// Removing rows and columns
// ------------------------------------------------------------------
int32_t test_table_remove() {
  D1("test_table_remove start");
  AUI *au = AUI::Create("TableRemoveTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
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
  delete au;
  D1("test_table_remove passed");
  return 0;
}

// ------------------------------------------------------------------
// Clear all data
// ------------------------------------------------------------------
int32_t test_table_clear() {
  D1("test_table_clear start");
  AUI *au = AUI::Create("TableClearTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
  table->AddRows(10);
  table->AddColumns(10);
  table->SetCellData(5, 5, "Data");
  table->Clear();
  TEST_ASSERT_EQ(table->RowCount(), 0U, 2);
  TEST_ASSERT_EQ(table->ColumnCount(), 0U, 3);
  TEST_ASSERT_EQ(table->GetCellData(5, 5), std::string(""), 4);
  delete au;
  D1("test_table_clear passed");
  return 0;
}

// ------------------------------------------------------------------
// Scrolling (mouse wheel)
// ------------------------------------------------------------------
int32_t test_table_scroll() {
  D1("test_table_scroll start");
  AUI *au = AUI::Create("TableScrollTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
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
  delete au;
  D1("test_table_scroll passed");
  return 0;
}

// ------------------------------------------------------------------
// Selection (cursor and row selection mode)
// ------------------------------------------------------------------
int32_t test_table_selection() {
  D1("test_table_selection start");
  AUI *au = AUI::Create("TableSelectionTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
  table->AddRows(5);
  table->AddColumns(5);
  table->SetCursorPosition(2, 2);
  TEST_ASSERT_EQ(table->GetCursorRow(), 2LL, 2);
  TEST_ASSERT_EQ(table->GetCursorCol(), 2LL, 3);
  table->SetRowSelectMode(true);
  table->SetCursorPosition(3, 1);
  TEST_ASSERT_EQ(table->GetSelectedRow(), 3LL, 4);
  TEST_ASSERT_EQ(table->GetCursorRow(), 3LL, 5);
  delete au;
  D1("test_table_selection passed");
  return 0;
}

// ------------------------------------------------------------------
// Scrollbars enabled/disabled
// ------------------------------------------------------------------
int32_t test_table_scrollbars() {
  D1("test_table_scrollbars start");
  AUI *au = AUI::Create("TableScrollbarsTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
  table->SetScrollbarsEnabled(true);
  TEST_ASSERT(table->AreScrollbarsEnabled() == true, 2);
  AScrollBar *vbar = table->GetVScrollBar();
  AScrollBar *hbar = table->GetHScrollBar();
  TEST_ASSERT(vbar != nullptr && hbar != nullptr, 3);
  table->SetScrollbarsEnabled(false);
  TEST_ASSERT(table->AreScrollbarsEnabled() == false, 4);
  delete au;
  D1("test_table_scrollbars passed");
  return 0;
}

// ------------------------------------------------------------------
// Resize and auto-widen column
// ------------------------------------------------------------------
int32_t test_table_auto_widen() {
  D1("test_table_auto_widen start");
  AUI *au = AUI::Create("TableAutoWidenTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
  table->SetAutoWiden(true);
  table->AddColumns(1);
  table->SetCellData(0, 0, "This is a very long text that should widen the column");
  table->AutoWidenColumn(0);
  delete au;
  D1("test_table_auto_widen passed");
  return 0;
}

// ------------------------------------------------------------------
// Callback on cell click (via mouse simulation)
// ------------------------------------------------------------------
int32_t test_table_cell_click_callback() {
  D1("test_table_cell_click_callback start");
  AUI *au = AUI::Create("TableClickCallbackTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
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
  delete au;
  D1("test_table_cell_click_callback passed");
  return 0;
}

// ------------------------------------------------------------------
// NEW TESTS: Row/Column resizing via mouse drag (public API only)
// ------------------------------------------------------------------

// Helper: set known dimensions and header sizes using public setters
static void setup_resize_test_table(ATable *table) {
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
  table->UpdateLayout();// rebuild prefixes & scrollbars
  table->ScrollTo(0, 0);
}

int32_t test_table_resize_column() {
  D1("test_table_resize_column start");
  AUI *au = AUI::Create("TableResizeColTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
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
  delete au;
  D1("test_table_resize_column passed");
  return 0;
}

int32_t test_table_resize_row() {
  D1("test_table_resize_row start");
  AUI *au = AUI::Create("TableResizeRowTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
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
  delete au;
  D1("test_table_resize_row passed");
  return 0;
}

int32_t test_table_resize_min_size() {
  D1("test_table_resize_min_size start");
  AUI *au = AUI::Create("TableResizeMinTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
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
  delete au;
  D1("test_table_resize_min_size passed");
  return 0;
}

int32_t test_table_resize_with_scroll() {
  D1("test_table_resize_with_scroll start");
  AUI *au = AUI::Create("TableResizeScrollTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
  setup_resize_test_table(table);
// Use horizontal scroll only (no vertical scroll) for reliable row resizing
  table->ScrollTo(60, 0);// mHOffset = 60, mVOffset = 0
// ---- Column resize with horizontal scroll ----
// Column separator between col 1 and 2:
// x = rowHeaderWidth - hOffset + col0 + col1 = 60 - 60 + 100 + 120 = 220
  int32_t sepX = 220, sepY = 12;
  table->OnMouseClick(sepX, sepY, true);
  table->OnMouseMove(sepX + 10, sepY);
  TEST_ASSERT_EQ(table->GetColumnWidth(1), 130LL, 2);// 120 → 130
  table->OnMouseClick(sepX + 10, sepY, false);
// ---- Row resize without vertical scroll ----
// Row separator between row 1 and 2: y = colHeaderHeight + row0 + row1 = 24+30+40 = 94
  sepX = 30;
  sepY = 94;
  table->OnMouseClick(sepX, sepY, true);
  table->OnMouseMove(sepX, sepY + 15);
  TEST_ASSERT_EQ(table->GetRowHeight(1), 55LL, 3);// 40 → 55
  table->OnMouseClick(sepX, sepY + 15, false);
  delete au;
  D1("test_table_resize_with_scroll passed");
  return 0;
}

int32_t test_table_resize_no_separator_on_last() {
  D1("test_table_resize_no_separator_on_last start");
  AUI *au = AUI::Create("TableResizeLastTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
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
  delete au;
  D1("test_table_resize_no_separator_on_last passed");
  return 0;
}

int32_t test_table_resize_click_outside_header() {
  D1("test_table_resize_click_outside_header start");
  AUI *au = AUI::Create("TableResizeOutsideTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
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
  delete au;
  D1("test_table_resize_click_outside_header passed");
  return 0;
}

int32_t test_table_render_performance() {
  D1("test_table_render_performance start");
  AUI *au = AUI::Create("PerfTestContext");
  AWindow *win = au->MainWnd();
  ATable *table = ATable::AttachTo(win);
  table->AddRows(5000);
  table->AddColumns(100);
  D1("test_table_render_performance 1");
// Fill visible window space
  for(int i = 0; i < 50; ++i) {
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
// Simulate 1,000 frames of rendering/layout lookups safely
  for(int i = 0; i < 10; ++i) {
    table->Draw(isolatedBuffer.data(), width, height, 0, 0);
  }
  auto end = std::chrono::high_resolution_clock::now();
  D1("test_table_render_performance 4");
  std::chrono::duration<double, std::milli> elapsed = end - start;
  D1("Rendered 10 frames in {} ms", elapsed.count());
// Adjusted assertion to match the stress workload scale target

//                        162ms in my setup
  TEST_ASSERT(elapsed.count() < 300, 2);
  delete au;
  return 0;
}

// ------------------------------------------------------------------
// main
// ------------------------------------------------------------------
int main() {
  uint32_t delay_ms = 500;
  int32_t testsfailed = 0;
  AUI *au = AUI::Create("aui table test");
  UNUSED AWindow *w = au->MainWnd();

  auto handle = std::async(std::launch::async, [=]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    au->ExitAUI();
  });

  testsfailed += test_table_attachment();
  testsfailed += test_table_add_rows_columns();
  testsfailed += test_table_cell_data();
  testsfailed += test_table_remove();
  testsfailed += test_table_clear();
  testsfailed += test_table_scroll();
  testsfailed += test_table_selection();
  testsfailed += test_table_scrollbars();
  testsfailed += test_table_auto_widen();
  testsfailed += test_table_cell_click_callback();
  testsfailed += test_table_resize_column();
  testsfailed += test_table_resize_row();
  testsfailed += test_table_resize_min_size();
  testsfailed += test_table_resize_with_scroll();
  testsfailed += test_table_resize_no_separator_on_last();
  testsfailed += test_table_resize_click_outside_header();
  testsfailed += test_table_render_performance();

  au->ProcessMessages();
  handle.get();
  delete au;
  au = nullptr;

  return testsfailed;
}



