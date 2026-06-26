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
  TEST_ASSERT_EQ(list->GetItemCount(), 0, 3);
  TEST_ASSERT_EQ(list->GetVerticalOffset(), 0, 4);
  TEST_ASSERT_EQ(list->GetHorizontalOffset(), 0, 5);
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
  TEST_ASSERT_EQ(list->GetItemCount(), 3, 2);
  TEST_ASSERT_EQ(list->GetItem(0), "Item 1", 3);
  TEST_ASSERT_EQ(list->GetItem(1), "Item 2", 4);
  TEST_ASSERT_EQ(list->GetItem(2), "Item 3", 5);
  list->InsertItem(1, "Inserted");
  TEST_ASSERT_EQ(list->GetItemCount(), 4, 6);
  TEST_ASSERT_EQ(list->GetItem(1), "Inserted", 7);
  list->RemoveItem(2);
  TEST_ASSERT_EQ(list->GetItemCount(), 3, 8);
  TEST_ASSERT_EQ(list->GetItem(2), "Item 3", 9);
  list->Clear();
  TEST_ASSERT_EQ(list->GetItemCount(), 0, 10);
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
  list->SetMultiSelect(true);
  list->SelectIndex(0, true);
  list->SelectIndex(2, true);
  list->SelectIndex(4, true);
  TEST_ASSERT_EQ(list->IsSelected(0), true, 6);
  TEST_ASSERT_EQ(list->IsSelected(2), true, 7);
  TEST_ASSERT_EQ(list->IsSelected(4), true, 8);
  TEST_ASSERT_EQ(list->IsSelected(1), false, 9);
  auto selected = list->GetSelectedIndices();
  TEST_ASSERT_EQ(selected.size(), 3, 10);
  TEST_ASSERT(selected[0] == 0 && selected[1] == 2 && selected[2] == 4, 11);
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
  for (int i = 0; i < 100; ++i)
    list->AddItem("Line " + std::to_string(i));
  list->Resize(200, 300);
  int32_t oldV = list->GetVerticalOffset();
  list->ScrollToOffset(0, oldV + 100);
  TEST_ASSERT(list->GetVerticalOffset() > oldV, 2);
  list->ScrollToOffset(0, 999999);
  int32_t maxV = list->GetVerticalOffset();
  TEST_ASSERT(maxV > 0, 3);
  list->ScrollToOffset(0, 0);
  TEST_ASSERT_EQ(list->GetVerticalOffset(), 0, 4);
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
  list->SetScrollbarsEnabled(true);
  TEST_ASSERT_EQ(list->AreScrollbarsEnabled(), true, 2);
  list->SetAutoHideScrollbars(true);
  list->UpdateScrollbarRanges();
  TEST_ASSERT_EQ(list->IsVerticalScrollbarEnabled(), true, 3);
  for (int i = 0; i < 50; ++i)
    list->AddItem("Item");
  list->Resize(100, 200);
  list->SetVerticalScrollbarEnabled(false);
  TEST_ASSERT_EQ(list->IsVerticalScrollbarEnabled(), false, 4);
  list->SetHorizontalScrollbarEnabled(true);
  TEST_ASSERT_EQ(list->IsHorizontalScrollbarEnabled(), true, 5);
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
  for (int i = 0; i < 10; ++i)
    list->AddItem("Line " + std::to_string(i));
  list->Move(0, 0);
  list->Resize(200, 300);
  w->OnMousePress(5, 5, 1);
  w->OnMouseRelease(5, 5, 1);
  TEST_ASSERT_EQ(list->IsSelected(0), true, 2);
  w->OnMousePress(5, 30, 1);
  w->OnMouseRelease(5, 30, 1);
  TEST_ASSERT_EQ(list->IsSelected(0), false, 3);
  list->SetMultiSelect(true);
  list->ClearSelection();
  uint32_t lineH = list->GetLineHeight();
  int y1 = 5;
  int y2 = static_cast<int>(lineH) + 5;
  w->OnMousePress(5, y1, 1);
  w->OnMouseRelease(5, y1, 1);
  w->OnMousePress(5, y2, 1);
  w->OnMouseRelease(5, y2, 1);
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
  list->SetVAlignment(AUIVAlign::bottom);
  list->ScrollToOffset(0, 999999);
  int32_t maxY = list->GetVerticalOffset();
  TEST_ASSERT(maxY > 0, 2);
  list->SetVAlignment(AUIVAlign::top);
  list->ScrollToOffset(0, 0);
  TEST_ASSERT_EQ(list->GetVerticalOffset(), 0, 3);
  list->SetVAlignment(AUIVAlign::center);
  TEST_ASSERT_EQ(list->GetVAlignment(), AUIVAlign::center, 4);
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
  TEST_ASSERT_EQ(list->GetSelectedIndices().size(), 0, 4);
  list->RemoveItem(0);
  TEST_ASSERT_EQ(list->GetItemCount(), 0, 5);
  D1("test_list_edge_cases passed");
  return 0;
}

// ------------------------------------------------------------------
// Horizontal alignment regression test
// ------------------------------------------------------------------
int32_t test_list_horizontal_alignment_regression(AUI* au) {
  D1("test_list_horizontal_alignment_regression start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AList* list = AList::AttachTo(w);
  list->Resize(200, 150);
  list->SetAutoHideScrollbars(false);
  list->SetScrollbarsEnabled(true);
  for (int i = 0; i < 20; ++i)
    list->AddItem("Line " + std::to_string(i));
  list->AddItem("This is an extremely long line that definitely exceeds the list width and forces a horizontal scrollbar");
  list->AddItem("Short");
  list->UpdateScrollbarRanges();
  int32_t listWidth = (int32_t)list->SizeX();
  int32_t vScrollWidth = (list->IsVerticalScrollbarEnabled() && list->IsVerticalScrollbarVisible())
                         ? static_cast<int32_t>(list->GetVScrollBar()->SizeX()) : 0;
  int32_t viewWidth = listWidth - vScrollWidth;
  uint32_t maxWidth = list->GetMaxContentWidth();
  int32_t expectedMaxH = (maxWidth > static_cast<uint32_t>(viewWidth))
                         ? static_cast<int32_t>((int32_t)maxWidth - viewWidth) : 0;
  list->SetHAlignment(AUIHAlign::right);
  int32_t rightOffset = list->GetHorizontalOffset();
  TEST_ASSERT(std::abs(rightOffset - expectedMaxH) <= 1, 2);
  if (list->GetHScrollBar())
    TEST_ASSERT(std::abs(list->GetHScrollBar()->GetValue() - rightOffset) <= 1, 3);
  list->SetHAlignment(AUIHAlign::center);
  int32_t centerOffset = list->GetHorizontalOffset();
  int32_t expectedCenter = expectedMaxH / 2;
  TEST_ASSERT(std::abs(centerOffset - expectedCenter) <= 1, 4);
  if (list->GetHScrollBar())
    TEST_ASSERT(std::abs(list->GetHScrollBar()->GetValue() - centerOffset) <= 1, 5);
  D1("test_list_horizontal_alignment_regression passed");
  return 0;
}

// ------------------------------------------------------------------
// Main: run all tests with timed test harness
// ------------------------------------------------------------------
int main() {
  int32_t testsfailed = 0;
  testsfailed += runTimedTest("test_list_attachment", test_list_attachment, 1);
  testsfailed += runTimedTest("test_list_add_items", test_list_add_items, 1);
  testsfailed += runTimedTest("test_list_selection", test_list_selection, 1);
  testsfailed += runTimedTest("test_list_scrolling", test_list_scrolling, 1);
  testsfailed += runTimedTest("test_list_scrollbars", test_list_scrollbars, 1);
  testsfailed += runTimedTest("test_list_click_selection", test_list_click_selection, 1);
  testsfailed += runTimedTest("test_list_alignment", test_list_alignment, 1);
  testsfailed += runTimedTest("test_list_edge_cases", test_list_edge_cases, 1);
  testsfailed += runTimedTest("test_list_horizontal_alignment_regression", test_list_horizontal_alignment_regression, 1);

  testsfailed += runTimedTest("test_list_attachment", test_list_attachment, 200);
  testsfailed += runTimedTest("test_list_add_items", test_list_add_items, 200);
  testsfailed += runTimedTest("test_list_selection", test_list_selection, 200);
  testsfailed += runTimedTest("test_list_scrolling", test_list_scrolling, 200);
  testsfailed += runTimedTest("test_list_scrollbars", test_list_scrollbars, 200);
  testsfailed += runTimedTest("test_list_click_selection", test_list_click_selection, 200);
  testsfailed += runTimedTest("test_list_alignment", test_list_alignment, 200);
  testsfailed += runTimedTest("test_list_edge_cases", test_list_edge_cases, 200);
  testsfailed += runTimedTest("test_list_horizontal_alignment_regression", test_list_horizontal_alignment_regression, 200);

  D("test suite complete");
  return testsfailed;
}
