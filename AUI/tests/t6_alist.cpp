#include <chrono>
#include <future>
#include <thread>
#include "AUILib.h"
#include "AList.h"

using namespace aui;

#define TEST_ASSERT(cond, errcode) do { if(!(cond)) { E("Test failed: {}", #cond); return errcode; } } while(0)

// ------------------------------------------------------------------
// Basic attachment and default values
// ------------------------------------------------------------------
int32_t test_list_attachment() {
    D1("test_list_attachment start");
    AUI* au = AUI::Create("ListAttachTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* w = au->MainWnd();
    AList* list = AList::AttachTo(w);
    TEST_ASSERT(list != nullptr, 2);
    TEST_ASSERT(list->GetItemCount() == 0, 3);
    TEST_ASSERT(list->GetVerticalOffset() == 0, 4);
    TEST_ASSERT(list->GetHorizontalOffset() == 0, 5);
    TEST_ASSERT(list->IsMultiSelect() == false, 6);
    delete au;
    D1("test_list_attachment passed");
    return 0;
}

// ------------------------------------------------------------------
// Adding and retrieving items
// ------------------------------------------------------------------
int32_t test_list_add_items() {
    D1("test_list_add_items start");
    AUI* au = AUI::Create("ListAddTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* w = au->MainWnd();
    AList* list = AList::AttachTo(w);
    list->AddItem("Item 1");
    list->AddItem("Item 2");
    list->AddItem("Item 3");
    TEST_ASSERT(list->GetItemCount() == 3, 2);
    TEST_ASSERT(list->GetItem(0) == "Item 1", 3);
    TEST_ASSERT(list->GetItem(1) == "Item 2", 4);
    TEST_ASSERT(list->GetItem(2) == "Item 3", 5);
    list->InsertItem(1, "Inserted");
    TEST_ASSERT(list->GetItemCount() == 4, 6);
    TEST_ASSERT(list->GetItem(1) == "Inserted", 7);
    list->RemoveItem(2);
    TEST_ASSERT(list->GetItemCount() == 3, 8);
    TEST_ASSERT(list->GetItem(2) == "Item 3", 9);
    list->Clear();
    TEST_ASSERT(list->GetItemCount() == 0, 10);
    delete au;
    D1("test_list_add_items passed");
    return 0;
}

// ------------------------------------------------------------------
// Selection (single and multi)
// ------------------------------------------------------------------
int32_t test_list_selection() {
    D1("test_list_selection start");
    AUI* au = AUI::Create("ListSelectTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* w = au->MainWnd();
    AList* list = AList::AttachTo(w);
    for (int i = 0; i < 5; ++i)
        list->AddItem("Item " + std::to_string(i));
    
    // Single select mode (default)
    list->SelectIndex(2, true);
    TEST_ASSERT(list->IsSelected(2) == true, 2);
    TEST_ASSERT(list->IsSelected(1) == false, 3);
    list->SelectIndex(4, true);
    TEST_ASSERT(list->IsSelected(4) == true, 4);
    TEST_ASSERT(list->IsSelected(2) == false, 5);  // previous cleared
    
    // Multi‑select mode
    list->SetMultiSelect(true);
    list->SelectIndex(0, true);
    list->SelectIndex(2, true);
    list->SelectIndex(4, true);
    TEST_ASSERT(list->IsSelected(0) == true, 6);
    TEST_ASSERT(list->IsSelected(2) == true, 7);
    TEST_ASSERT(list->IsSelected(4) == true, 8);
    TEST_ASSERT(list->IsSelected(1) == false, 9);
    
    auto selected = list->GetSelectedIndices();
    TEST_ASSERT(selected.size() == 3, 10);
    TEST_ASSERT(selected[0] == 0 && selected[1] == 2 && selected[2] == 4, 11);
    
    list->ClearSelection();
    TEST_ASSERT(list->IsSelected(0) == false, 12);
    delete au;
    D1("test_list_selection passed");
    return 0;
}

// ------------------------------------------------------------------
// Scrolling (offsets and limits)
// ------------------------------------------------------------------
int32_t test_list_scrolling() {
    D1("test_list_scrolling start");
    AUI* au = AUI::Create("ListScrollTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* w = au->MainWnd();
    AList* list = AList::AttachTo(w);
    // Add many items to make scrolling possible
    for (int i = 0; i < 100; ++i)
        list->AddItem("Line " + std::to_string(i));
    // Force a reasonable size
    list->Resize(200, 300);
    // Simulate a few scroll steps
    int32_t oldV = list->GetVerticalOffset();
    list->ScrollToOffset(0, oldV + 100);
    TEST_ASSERT(list->GetVerticalOffset() > oldV, 2);
    // Scroll to bottom
    list->ScrollToOffset(0, 999999);
    int32_t maxV = list->GetVerticalOffset();   // will be clamped to max
    TEST_ASSERT(maxV > 0, 3);
    // Scroll back to top
    list->ScrollToOffset(0, 0);
    TEST_ASSERT(list->GetVerticalOffset() == 0, 4);
    delete au;
    D1("test_list_scrolling passed");
    return 0;
}

// ------------------------------------------------------------------
// Scrollbars enable/disable and visibility
// ------------------------------------------------------------------
int32_t test_list_scrollbars() {
    D1("test_list_scrollbars start");
    AUI* au = AUI::Create("ListScrollBarTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* w = au->MainWnd();
    AList* list = AList::AttachTo(w);
    list->SetScrollbarsEnabled(true);
    TEST_ASSERT(list->AreScrollbarsEnabled() == true, 2);
    // Initially no items – scrollbars hidden (auto‑hide)
    list->SetAutoHideScrollbars(true);
    list->UpdateScrollbarRanges();   // internal, but called automatically
    TEST_ASSERT(list->IsVerticalScrollbarEnabled() == true, 3);   // scrollbar object exists
    // Add enough items to make vertical scrollbar appear
    for (int i = 0; i < 50; ++i)
        list->AddItem("Item");
    list->Resize(100, 200);
    // After resize, the scrollbar should be visible (auto‑hide only hides if not needed)
    // We cannot easily query visibility, but we can test that disabling works
    list->SetVerticalScrollbarEnabled(false);
    TEST_ASSERT(list->IsVerticalScrollbarEnabled() == false, 4);
    list->SetHorizontalScrollbarEnabled(true);
    TEST_ASSERT(list->IsHorizontalScrollbarEnabled() == true, 5);
    delete au;
    D1("test_list_scrollbars passed");
    return 0;
}

// ------------------------------------------------------------------
// Mouse click selection (simulate on the AWindow)
// ------------------------------------------------------------------
int32_t test_list_click_selection() {
    D1("test_list_click_selection start");
    AUI* au = AUI::Create("ListClickTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* w = au->MainWnd();
    AList* list = AList::AttachTo(w);
    for (int i = 0; i < 10; ++i)
        list->AddItem("Line " + std::to_string(i));
    // Position the list at (0,0) and give it a size
    list->Move(0, 0);
    list->Resize(200, 300);
    // Simulate a click on the first item (approx line height = ~14px + spacing)
    int32_t y_click = 5;   // inside first line
    w->OnMousePress(5, y_click, 1);
    w->OnMouseRelease(5, y_click, 1);
    TEST_ASSERT(list->IsSelected(0) == true, 2);
    // Click on a different line
    y_click = 30;   // second or third line (depending on line height)
    w->OnMousePress(5, y_click, 1);
    w->OnMouseRelease(5, y_click, 1);
    // Single select mode – previous should be cleared
    TEST_ASSERT(list->IsSelected(0) == false, 3);
    // Test multi‑select (requires Ctrl or Shift? The current implementation
    // uses a flag mMultiSelect; we can test by enabling it and clicking)
    list->SetMultiSelect(true);
    list->ClearSelection();
    w->OnMousePress(5, 5, 1);
    w->OnMouseRelease(5, 5, 1);
    w->OnMousePress(5, 30, 1);
    w->OnMouseRelease(5, 30, 1);
    TEST_ASSERT(list->IsSelected(0) == true && list->IsSelected(1) == true, 4);
    delete au;
    D1("test_list_click_selection passed");
    return 0;
}

// ------------------------------------------------------------------
// Alignment and scroll offset interaction (public interface only)
// ------------------------------------------------------------------
int32_t test_list_alignment() {
    D1("test_list_alignment start");
    AUI* au = AUI::Create("ListAlignTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* w = au->MainWnd();
    AList* list = AList::AttachTo(w);
    for (int i = 0; i < 50; ++i)
        list->AddItem("Item");
    list->Resize(200, 300);
    // Set bottom alignment and scroll to bottom – we rely on the fact that
    // alignment itself does not change the offset; but we can test that
    // ScrollToOffset respects the alignment after the fact.
    list->SetVAlignment(AUIVAlign::bottom);
    int32_t maxY = 0;
    // Manually compute or just scroll to a large number
    list->ScrollToOffset(0, 999999);
    maxY = list->GetVerticalOffset();
    TEST_ASSERT(maxY > 0, 2);
    list->SetVAlignment(AUIVAlign::top);
    list->ScrollToOffset(0, 0);
    TEST_ASSERT(list->GetVerticalOffset() == 0, 3);
    // Center alignment – we can't assert a specific numeric value without knowing
    // content height, but we can check that setting alignment does not crash
    list->SetVAlignment(AUIVAlign::center);
    TEST_ASSERT(list->GetVAlignment() == AUIVAlign::center, 4);
    delete au;
    D1("test_list_alignment passed");
    return 0;
}

// ------------------------------------------------------------------
// Edge cases: empty list, out‑of‑range access
// ------------------------------------------------------------------
int32_t test_list_edge_cases() {
    D1("test_list_edge_cases start");
    AUI* au = AUI::Create("ListEdgeTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* w = au->MainWnd();
    AList* list = AList::AttachTo(w);
    // Empty list should not crash on draw
    w->Draw();
    TEST_ASSERT(list->GetItem(0).empty(), 2);
    TEST_ASSERT(list->IsSelected(0) == false, 3);
    list->SelectIndex(5, true);   // should be ignored
    TEST_ASSERT(list->GetSelectedIndices().empty(), 4);
    list->RemoveItem(0);          // no effect
    TEST_ASSERT(list->GetItemCount() == 0, 5);
    delete au;
    D1("test_list_edge_cases passed");
    return 0;
}

// ------------------------------------------------------------------
// Main: run all tests with async exit
// ------------------------------------------------------------------
int main() {
    uint32_t delay_ms = 50;
    int32_t testsfailed = 0;
    AUI* au = AUI::Create("aui list test");
    UNUSED AWindow* w = au->MainWnd();

    auto handle = std::async(std::launch::async, [=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        au->ExitAUI();
    });

    testsfailed += test_list_attachment();
    testsfailed += test_list_add_items();
    testsfailed += test_list_selection();
    testsfailed += test_list_scrolling();
    testsfailed += test_list_scrollbars();
    testsfailed += test_list_click_selection();
    testsfailed += test_list_alignment();
    testsfailed += test_list_edge_cases();

    au->ProcessMessages();
    handle.get();
    delete au;
    au = nullptr;

    return testsfailed;
}

