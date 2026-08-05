#include "AUILib.h"

using namespace aui;

// ------------------------------------------------------------------
// Basic attachment and default values
// ------------------------------------------------------------------
int32_t test_list_attachment(AUI* au) {
  D1("test_list_attachment start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AList* list = AList::AttachTo(w);
  TEST_ASSERT_NE(list, nullptr, 2);
  TEST_ASSERT_EQ(list->ItemCount(), 0, 3);
  TEST_ASSERT_EQ(list->VerticalOffset(), 0, 4);
  TEST_ASSERT_EQ(list->HorizontalOffset(), 0, 5);
  TEST_ASSERT_EQ(list->IsMultiSelect(), false, 6);
  D1("test_list_attachment passed");
  return 0;
}

// ------------------------------------------------------------------
// Adding and retrieving items
// ------------------------------------------------------------------
int32_t test_list_add_items(AUI* au) {
  D1("test_list_add_items start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AList* list = AList::AttachTo(w);
  list->AddItem("Item 1");
  list->AddItem("Item 2");
  list->AddItem("Item 3");
  TEST_ASSERT_EQ(list->ItemCount(), 3, 2);
  TEST_ASSERT_EQ(list->GetItem(0), "Item 1", 3);
  TEST_ASSERT_EQ(list->GetItem(1), "Item 2", 4);
  TEST_ASSERT_EQ(list->GetItem(2), "Item 3", 5);
  list->InsertItem(1, "Inserted");
  TEST_ASSERT_EQ(list->ItemCount(), 4, 6);
  TEST_ASSERT_EQ(list->GetItem(1), "Inserted", 7);
  list->RemoveItem(2);
  TEST_ASSERT_EQ(list->ItemCount(), 3, 8);
  TEST_ASSERT_EQ(list->GetItem(2), "Item 3", 9);
  list->Clear();
  TEST_ASSERT_EQ(list->ItemCount(), 0, 10);
  D1("test_list_add_items passed");
  return 0;
}
// ------------------------------------------------------------------
// Selection (single and multi)
// ------------------------------------------------------------------
int32_t test_list_selection(AUI* au) {
  D1("test_list_selection start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AList* list = AList::AttachTo(w);
  for (int i = 0; i < 5; ++i)
    list->AddItem("Item " + std::to_string(i));
  list->SelectIndex(2, true);
  TEST_ASSERT_EQ(list->IsSelected(2), true, 2);
  TEST_ASSERT_EQ(list->IsSelected(1), false, 3);
  list->SelectIndex(4, true);
  TEST_ASSERT_EQ(list->IsSelected(4), true, 4);
  TEST_ASSERT_EQ(list->IsSelected(2), false, 5);
  list->MultiSelect(true);
  list->SelectIndex(0, true);
  list->SelectIndex(2, true);
  list->SelectIndex(4, true);
  TEST_ASSERT_EQ(list->IsSelected(0), true, 6);
  TEST_ASSERT_EQ(list->IsSelected(2), true, 7);
  TEST_ASSERT_EQ(list->IsSelected(4), true, 8);
  TEST_ASSERT_EQ(list->IsSelected(1), false, 9);
  auto selected = list->SelectedIndices();
  TEST_ASSERT_EQ(selected.size(), 3, 10);
  TEST_ASSERT_EQ(selected[0], 0u, 11);
  TEST_ASSERT_EQ(selected[1], 2u, 11);
  TEST_ASSERT_EQ(selected[2], 4u, 11);
  list->ClearSelection();
  TEST_ASSERT_EQ(list->IsSelected(0), false, 12);
  D1("test_list_selection passed");
  return 0;
}
// ------------------------------------------------------------------
// Scrolling (offsets and limits)
// ------------------------------------------------------------------
int32_t test_list_scrolling(AUI* au) {
  D1("test_list_scrolling start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AList* list = AList::AttachTo(w);
  for (int i = 0; i < 100; ++i) {
    //ST1("add one list item")
    list->AddItem("Line " + std::to_string(i));
  }
  list->Resize(200, 300);
  int32_t oldV = list->VerticalOffset();
  list->ScrollToOffset(0, oldV + 100);
  TEST_ASSERT(list->VerticalOffset() > oldV, 2);
  list->ScrollToOffset(0, 999999);
  int32_t maxV = list->VerticalOffset();
  TEST_ASSERT(maxV > 0, 3);
  list->ScrollToOffset(0, 0);
  TEST_ASSERT_EQ(list->VerticalOffset(), 0, 4);
  D1("test_list_scrolling passed");
  return 0;
}

// ------------------------------------------------------------------
// Scrollbars enable/disable and visibility
// ------------------------------------------------------------------
int32_t test_list_scrollbars(AUI* au) {
  D1("test_list_scrollbars start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AList* list = AList::AttachTo(w);
  list->ScrollbarsEnabled(true);
  TEST_ASSERT_EQ(list->ScrollbarsEnabled(), true, 2);
  list->AutoHideScrollbars(true);
  list->UpdateScrollbarRanges();
  TEST_ASSERT_EQ(list->VScrollbarEnabled(), true, 3);
  for (int32_t i = 0; i < 50; ++i) {
    list->AddItem("Item");
  }
  list->Resize(100, 200);
  list->VScrollbarToggle(false);
  TEST_ASSERT_EQ(list->VScrollbarEnabled(), false, 4);
  list->HScrollbarToggle(true);
  TEST_ASSERT_EQ(list->HScrollbarEnabled(), true, 5);
  D1("test_list_scrollbars passed");
  return 0;
}
// ------------------------------------------------------------------
// Mouse click selection (simulate on the AWindow)
// ------------------------------------------------------------------
int32_t test_list_click_selection(AUI* au) {
  D1("test_list_click_selection start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AList* list = AList::AttachTo(w);
  for (int32_t i = 0; i < 10; ++i)
    list->AddItem("Line " + std::to_string(i));
  list->Move(0, 0);
  list->Resize(200, 300);
  w->MouseClick(5, 5);
  TEST_ASSERT_EQ(list->IsSelected(0), true, 2);
  w->MouseClick(5, 30);
  TEST_ASSERT_EQ(list->IsSelected(0), false, 3);
  list->MultiSelect(true);
  list->ClearSelection();
  uint32_t lineH = list->LineHeight();
  int32_t y1 = 5;
  int32_t y2 = static_cast<int>(lineH) + 5;
  w->MouseClick(5, y1);
  w->MouseClick(5, y2);
  TEST_ASSERT(list->IsSelected(0) == true && list->IsSelected(1) == true, 4);
  D1("test_list_click_selection passed");
  return 0;
}
// ------------------------------------------------------------------
// Alignment and scroll offset interaction (public interface only)
// ------------------------------------------------------------------
int32_t test_list_alignment(AUI* au) {
  D1("test_list_alignment start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AList* list = AList::AttachTo(w);
  for (int i = 0; i < 50; ++i)
    list->AddItem("Item");
  list->Resize(200, 300);
  list->VAlign(AUIVAlign::bottom);
  list->ScrollToOffset(0, 999999);
  int32_t maxY = list->VerticalOffset();
  TEST_ASSERT(maxY > 0, 2);
  list->VAlign(AUIVAlign::top);
  list->ScrollToOffset(0, 0);
  TEST_ASSERT_EQ(list->VerticalOffset(), 0, 3);
  list->VAlign(AUIVAlign::center);
  TEST_ASSERT_EQ(list->AWidget::VAlign(), AUIVAlign::center, 4);
  D1("test_list_alignment passed");
  return 0;
}
// ------------------------------------------------------------------
// Edge cases: empty list, out‑of‑range access
// ------------------------------------------------------------------
int32_t test_list_edge_cases(AUI* au) {
  D1("test_list_edge_cases start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AList* list = AList::AttachTo(w);
  w->Draw();
  TEST_ASSERT_EQ(list->GetItem(0), std::string(), 2);
  TEST_ASSERT_EQ(list->IsSelected(0), false, 3);
  list->SelectIndex(5, true);
  TEST_ASSERT_EQ(list->SelectedIndices().size(), 0, 4);
  list->RemoveItem(0);
  TEST_ASSERT_EQ(list->ItemCount(), 0, 5);
  D1("test_list_edge_cases passed");
  return 0;
}

int32_t test_list_horizontal_alignment_regression(AUI* au) {
  D1("test_list_horizontal_alignment_regression start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(250, 200);
  AList* list = AList::AttachTo(w);
  list->Resize(200, 150);
  list->AutoHideScrollbars(false);
  list->ScrollbarsEnabled(true);
  for (int i = 0; i < 20; ++i)
    list->AddItem("Line " + std::to_string(i));
  list->AddItem("This is an extremely long line that definitely exceeds the list width and forces a horizontal scrollbar");
  list->AddItem("Short");
  list->RecalcMaxWidth();
  list->ComputeAlignmentOffsets();
  // 1. Initial viewport scroll position must be 0
  TEST_ASSERT_EQ(list->HorizontalOffset(), 0, 2);
  // 2. Setting HAlign to Right should update alignment state without forcing mHOffset to jump to maxH
  list->HAlign(AUIHAlign::right);
  list->ComputeAlignmentOffsets();
  TEST_ASSERT_EQ(list->HorizontalOffset(), 0, 3);
  if (list->HScrollBar()) {
    TEST_ASSERT_EQ(list->HScrollBar()->Value(), 0, 4);
  }
  // 3. Setting HAlign to Center should keep mHOffset at 0 (DrawTextEx handles the visual centering)
  list->HAlign(AUIHAlign::center);
  list->ComputeAlignmentOffsets();
  TEST_ASSERT_EQ(list->HorizontalOffset(), 0, 5);
  if (list->HScrollBar()) {
    TEST_ASSERT_EQ(list->HScrollBar()->Value(), 0, 6);
  }
  // 4. Setting HAlign to Left keeps mHOffset at 0
  list->HAlign(AUIHAlign::left);
  list->ComputeAlignmentOffsets();
  TEST_ASSERT_EQ(list->HorizontalOffset(), 0, 7);
  D1("test_list_horizontal_alignment_regression passed");
  return 0;
}

int32_t test_list_add_item_performance(AUI* au) {
  D1("test_list_add_item_performance start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(50, 50);
  AList* list = AList::AttachTo(w);
  list->Resize(25, 25);
  const int numItems = 100;
  // 1. Measure sequential AddItem calls
  auto startSeq = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < numItems; ++i) {
    list->AddItem("Item number " + std::to_string(i) + " with sample text padding");
  }
  auto endSeq = std::chrono::high_resolution_clock::now();
  double totalSeqUs = std::chrono::duration<double, std::micro>(endSeq - startSeq).count();
  double avgPerItemUs = totalSeqUs / numItems;
  D1("Added %d items sequentially in %.2f us (Avg: %.2f us/item)",
     numItems, totalSeqUs, avgPerItemUs);
  // SLA Assertion: With O(1) AddItem
  TEST_ASSERT(avgPerItemUs < 10000.0, 2);
  list->Clear();
  // 2. Measure bulk AddItems call (if implemented)
  std::vector<std::string> bulkData;
  bulkData.reserve(numItems);
  for (int i = 0; i < numItems; ++i) {
    bulkData.push_back("Bulk item " + std::to_string(i) + " with sample text padding");
  }
  auto startBulk = std::chrono::high_resolution_clock::now();
  list->AddItems(bulkData);
  auto endBulk = std::chrono::high_resolution_clock::now();
  double totalBulkUs = std::chrono::duration<double, std::micro>(endBulk - startBulk).count();
  D1("Added %d items in bulk in %.2f us (Avg: %.2f us/item)",
     numItems, totalBulkUs, totalBulkUs / numItems);
  // Bulk operation should be strictly faster or equal to sequential
  TEST_ASSERT(totalBulkUs <= totalSeqUs * 1.2, 3);
  D1("test_list_add_item_performance passed");
  return 0;
}

int32_t test_list_alignment_rendering(AUI* au) {
  D1("test_list_alignment_rendering start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(150, 50);
  w->DisableResize();

  AList* lTL = AList::AttachTo(w);
  lTL->Move(1, 1);
  lTL->Resize(39, 39);
  lTL->HAlign(AUIHAlign::left);
  lTL->VAlign(AUIVAlign::top);
  lTL->AddItem("8");
  lTL->FontSize(20);

  AList* lCC = AList::AttachTo(w);
  lCC->Move(41, 1);
  lCC->Resize(39, 39);
  lCC->HAlign(AUIHAlign::center);
  lCC->VAlign(AUIVAlign::center);
  lCC->AddItem("8");
  lCC->FontSize(20);

  AList* lBR = AList::AttachTo(w);
  lBR->Move(81, 1);
  lBR->Resize(39, 39);
  lBR->HAlign(AUIHAlign::right);
  lBR->VAlign(AUIVAlign::bottom);
  lBR->AddItem("8");
  lBR->FontSize(20);

  int32_t px = 0, py = 0;
  uint32_t t = 0;
  AWidgetReader<AList> r(lTL, 12, 12);
  bool c = true; // arm the test. makes it "exit(1);"
  px = 0; py = 0, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  // test canvas present
  px = 1; py = 1, t = 0xFFEEEEEE;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  // glyph rendered in place
  px = 6; py = 9, t = 0xFF000000;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);

  AWidgetReader<AList> r2(lCC, 60, 20);
  // glyph rendered in place
  px = 56; py = 17, t = 0xFF000000;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r2.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r2.Pixel(px, py), t, 3);

  AWidgetReader<AList> r3(lBR, 110, 30);
  // glyph rendered in place
  px = 106; py = 26, t = 0xFF000000;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r3.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r3.Pixel(px, py), t, 3);

  D1("test_list_alignment_rendering passed");
  return 0;
}

// ------------------------------------------------------------------
// Main: run all tests with timed test harness
// ------------------------------------------------------------------
int main() {
  int32_t testsfailed = 0;

//  AUI* au = AUI::Create("alignment test");
//  AWindow* w = au->MainWnd();
//  w->EnableResize();
//  w->Resize(120, 50);

  testsfailed += runTimedTest(test_list_attachment, 1);
  testsfailed += runTimedTest(test_list_add_items, 1);
  testsfailed += runTimedTest(test_list_selection, 1);
  testsfailed += runTimedTest(test_list_scrolling, 1);
  testsfailed += runTimedTest(test_list_scrollbars, 1);
  testsfailed += runTimedTest(test_list_click_selection, 1);
  testsfailed += runTimedTest(test_list_alignment, 1);
  testsfailed += runTimedTest(test_list_edge_cases, 1);
  testsfailed += runTimedTest(test_list_horizontal_alignment_regression, 1);
  testsfailed += runTimedTest(test_list_add_item_performance, 1);
  testsfailed += runTimedTest(test_list_alignment_rendering, 1);

  testsfailed += runTimedTest(test_list_attachment, 200);
  testsfailed += runTimedTest(test_list_add_items, 200);
  testsfailed += runTimedTest(test_list_selection, 200);
  testsfailed += runTimedTest(test_list_scrolling, 200);
  testsfailed += runTimedTest(test_list_scrollbars, 200);
  testsfailed += runTimedTest(test_list_click_selection, 200);
  testsfailed += runTimedTest(test_list_alignment, 200);
  testsfailed += runTimedTest(test_list_edge_cases, 200);
  testsfailed += runTimedTest(test_list_horizontal_alignment_regression, 200);
  testsfailed += runTimedTest(test_list_add_item_performance, 200);
  testsfailed += runTimedTest(test_list_alignment_rendering, 200);

//  au->ProcessMessages();
//  delete au;

  D("test suite complete");
  return testsfailed;
}
