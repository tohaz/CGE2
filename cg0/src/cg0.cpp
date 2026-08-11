//
//#include "AUILib.h"
//
//using namespace aui;
//
//int32_t test_menu_edges(AUI* au) {
//  D1("test_menu_edges start");
//  AWindow* win = au->MainWnd();
//  win->EnableResize();
//  win->Resize(400, 200);
//// Build a simple menu with submenu
//  std::vector<AMenuItem> subItems;
//  bool subActionFired = false;
//  subItems.emplace_back("Sub Item 1", std::function<void()>([&]() noexcept {
//    subActionFired = true;
//  }));
//  subItems.emplace_back("Sub Item 2", []() {
//    D("Sub 2");
//  });
//  std::vector<AMenuItem> mainItems;
//  mainItems.emplace_back("File", std::move(subItems));
//  mainItems.emplace_back("Edit");
//  mainItems.emplace_back("View");
//  AMenu* menu = AMenu::AttachTo(win, std::move(mainItems));
//  menu->Orientation(AUIOrientation::horizontal);
//  menu->SetPermanent(true);
//  menu->Move(0, 0);
//  menu->Resize(win->SizeX(), 28);
//  menu->SetColors(0xFFDDDDDD, 0xFFAAAAAA, 0xFF000000, 0xFF888888);
//  menu->Show();
//  win->Draw();// initial render
//// -----------------------------------------------------------------
//// 1. Hover highlight test
//// -----------------------------------------------------------------
//// Mouse over "File" (first item) – its x range is roughly 0..textwidth+padding
//// We'll use pixel to verify highlight background.
//// First, read initial pixel (no hover) at (20, 14) – inside first item
//  AWidgetReader<AMenu> reader(menu, win->SizeX(), win->SizeY());
//  uint32_t initialPixel = reader.Pixel(20, 14);// should be background color
//
//// Move mouse over first item
//  win->OnMouseMove(10, 14);// x=10 inside first item
//  win->Draw();// force redraw
//  uint32_t hoverPixel = reader.Pixel(20, 14);// should be hover color
//
//// Move mouse outside menu
//  win->OnMouseMove(-10, -10);
//  win->Draw();
//  uint32_t afterLeavePixel = reader.Pixel(20, 14);
//
//  TEST_ASSERT_NE(initialPixel, hoverPixel, 100);// hover changed color
//  TEST_ASSERT_EQ(initialPixel, afterLeavePixel, 101);// back to normal
//
//// Also check internal hovered index (if getter exists)
//  TEST_ASSERT_EQ(menu->HoveredIndex(), -1, 102);
//// -----------------------------------------------------------------
//// 2. Submenu open and item selection
//// -----------------------------------------------------------------
////    bool rootClickFired = false;
//// Add a callback on root item to detect click? But we want submenu.
//// Click on "File" to open submenu
//  win->OnMousePress(10, 14, BTN_LEFT);
//  win->OnMouseRelease(10, 14, BTN_LEFT);
//  win->Draw();
//// Check submenu is open (via pixel or getter)
//  TEST_ASSERT_EQ(menu->SubMenuOpen(), true, 200);
//// Click on "Sub Item 1" – its position is determined by submenu layout.
//// The submenu is positioned to the right of "File", so we need to find its position.
//// Since we don't have direct access, we can simulate a click at a known coordinate.
//// In this test, submenu is vertical, positioned at (File width + some offset, 0).
//// Approximate: File text width ~ 30, padding 6 each => item width ~ 42, plus submenu gap ~ 2.
//// So submenu x ~ 44, y=0. Sub Item 1 y ~ 24.
//  win->OnMousePress(50, 14, BTN_LEFT);// click inside submenu first item
//  win->OnMouseRelease(50, 14, BTN_LEFT);
//// The sub action callback should have fired
//  TEST_ASSERT_EQ(subActionFired, true, 201);
//// Submenu should be dismissed after selection
//  TEST_ASSERT_EQ(menu->SubMenuOpen(), false, 202);
//// Root item highlight should be cleared
//  TEST_ASSERT_EQ(menu->HoveredIndex(), -1, 203);
//// -----------------------------------------------------------------
//// 3. Outside‑click dismissal (popup menu)
//// -----------------------------------------------------------------
//// Create a popup menu (non‑permanent)
//  std::vector<AMenuItem> popupItems;
//  popupItems.emplace_back("Popup 1");
//  popupItems.emplace_back("Popup 2");
//  AMenu* popup = AMenu::AttachTo(win, std::move(popupItems));
//  popup->Popup(100, 50);// position at (100,50)
//  win->Draw();
//// Click outside the popup (e.g., at (0,0))
//  win->OnMousePress(0, 0, BTN_LEFT);
//  win->OnMouseRelease(0, 0, BTN_LEFT);
//  win->Draw();
//// Popup should be invisible
//  TEST_ASSERT_EQ(popup->IsVisible(), false, 300);
//// -----------------------------------------------------------------
//// 4. Root item highlight cleared when submenu is dismissed by outside click
//// -----------------------------------------------------------------
//// Reopen submenu on root "File"
//  win->OnMousePress(10, 14, BTN_LEFT);
//  win->OnMouseRelease(10, 14, BTN_LEFT);
//  win->Draw();
//  TEST_ASSERT_EQ(menu->SubMenuOpen(), true, 400);
//// Click outside the entire menu (e.g., at (200, 100))
//  win->OnMousePress(200, 100, BTN_LEFT);
//  win->OnMouseRelease(200, 100, BTN_LEFT);
//  win->Draw();
//// Submenu should be closed, root highlight cleared
//  TEST_ASSERT_EQ(menu->SubMenuOpen(), false, 401);
//  TEST_ASSERT_EQ(menu->HoveredIndex(), -1, 402);
//  D1("test_menu_edges passed");
//  return 0;
//}
//
//int main() {
//  int32_t testsfailed = 0;
//
//  testsfailed += runTimedTest(test_menu_edges, 1);
//// and also for 200 iterations if you want stress
//  testsfailed += runTimedTest(test_menu_edges, 200);
//
//  D("test suite complete");
//  return testsfailed;
//}
//
//#include "AUILib.h"
//
//using namespace aui;
//
//int32_t main() {
//  AUI* au = AUI::Create("test");
//  AWindow* w = au->MainWnd();
//  w->BGColor(0xFF222222);   // dark background
//
//  UNUSED ABox *bx = ABox::AttachTo(w);
//  bx->Resize(50, 50);
//  bx->Border(10);
//
//  au->ProcessMessages();
//
//  delete au;
//  return 0;
//}

//#include "AUILib.h"
//
//using namespace aui;
//
//int32_t main() {
////  AUI* au = AUI::Create("test", AUIWindowType::X11);
////  AUI* au = AUI::Create("test", AUIWindowType::Wayland);
//  AUI* au = AUI::Create("table rotation test");
//  AWindow* w = au->MainWnd();
//  w->BGColor(0xFF222222);   // dark background
//  w->EnableResize();
//  w->Resize(500,500);
//  w->DisableResize();
//
//  ATable* ta = ATable::AttachTo(w);
//  ta->Move(0, 0);
//  ta->AddColumns(15);
//  ta->AddRows(150);
//  ta->ScrollbarsToggle(true);
//  ta->CellData(0, 0, "AAA");
//  ta->CellData(1, 1, "BBB");
//  ta->CellData(2, 2, "CCC");
//  ta->CellData(3, 3, "CCC");
//
//  //  ta->Angle(15);
//  au->ProcessMessages();
//  delete au;
//  return 0;
//}

#include "AUILib.h"

using namespace aui;

// ------------------------------------------------------------------
// Basic attachment and default values
// ------------------------------------------------------------------
int32_t test_scrollbar_attachment(AUI* au) {
  D1("test_scrollbar_attachment start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 2);
  AScrollBar* sb = AScrollBar::AttachTo(w);
  TEST_ASSERT_NE(sb, nullptr, 3);
  TEST_ASSERT_EQ(sb->Orient(), AUIOrientation::vertical, 4);
  TEST_ASSERT_EQ(sb->MinValue(), 0, 5);
  TEST_ASSERT_EQ(sb->MaxValue(), 100, 6);
  TEST_ASSERT_EQ(sb->Value(), 0, 7);
  D1("test_scrollbar_attachment passed");
  return 0;
}
// ------------------------------------------------------------------
// Orientation and size
// ------------------------------------------------------------------
int32_t test_scrollbar_orientation(AUI* au) {
  D1("test_scrollbar_orientation start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 1);
  AScrollBar* v = AScrollBar::AttachTo(w, AUIOrientation::vertical);
  TEST_ASSERT_EQ(v->Orient(), AUIOrientation::vertical, 2);
  AScrollBar* h = AScrollBar::AttachTo(w, AUIOrientation::horizontal);
  TEST_ASSERT_EQ(h->Orient(), AUIOrientation::horizontal, 3);
  D1("test_scrollbar_orientation passed");
  return 0;
}
// ------------------------------------------------------------------
// Range and value manipulation
// ------------------------------------------------------------------
int32_t test_scrollbar_range_and_value(AUI* au) {
  D1("test_scrollbar_range_and_value start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AScrollBar* sb = AScrollBar::AttachTo(w);
  sb->Range(10, 200);
  TEST_ASSERT_EQ(sb->MinValue(), 10, 2);
  TEST_ASSERT_EQ(sb->MaxValue(), 200, 3);
  sb->Value(150);
  TEST_ASSERT_EQ(sb->Value(), 150, 4);
  sb->Value(5);   // below min
  TEST_ASSERT_EQ(sb->Value(), 10, 5);
  sb->Value(300); // above max
  TEST_ASSERT_EQ(sb->Value(), 200 - 10 - sb->PageStep(), 6);
  D1("test_scrollbar_range_and_value passed");
  return 0;
}
// ------------------------------------------------------------------
// Page step (affects thumb length)
// ------------------------------------------------------------------
int32_t test_scrollbar_page_step(AUI* au) {
  D1("test_scrollbar_page_step start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AScrollBar* sb = AScrollBar::AttachTo(w);
  sb->Range(0, 100);
  sb->PageStep(25);
  TEST_ASSERT_EQ(sb->PageStep(), 25, 2);
  D1("test_scrollbar_page_step passed");
  return 0;
}
// ------------------------------------------------------------------
// Thumb and track colors
// ------------------------------------------------------------------
int32_t test_scrollbar_colors(AUI* au) {
  D1("test_scrollbar_colors start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AScrollBar* sb = AScrollBar::AttachTo(w);
  sb->ThumbColor(0xFF123456);
  TEST_ASSERT_EQ(sb->ThumbColor(), 0xFF123456, 2);
  sb->TrackColor(0xFF654321);
  TEST_ASSERT_EQ(sb->TrackColor(), 0xFF654321, 3);
  D1("test_scrollbar_colors passed");
  return 0;
}
// ------------------------------------------------------------------
// Scroll callback (via SetValue)
// ------------------------------------------------------------------
int32_t test_scrollbar_callback(AUI* au) {
  D1("test_scrollbar_callback start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AScrollBar* sb = AScrollBar::AttachTo(w);
  bool callbackFired = false;
  int32_t callbackValue = 0;
  sb->SetScrollCallback(
      [&](AWidget*, void*, int32_t val) noexcept {
        callbackFired = true;
        callbackValue = val;
      },
      nullptr);
  sb->Value(42);
  TEST_ASSERT(callbackFired == true, 2);
  TEST_ASSERT_EQ(callbackValue, 42, 3);
  D1("test_scrollbar_callback passed");
  return 0;
}
// ------------------------------------------------------------------
// Click in track (jump to value)
// ------------------------------------------------------------------
int32_t test_scrollbar_click_in_track(UNUSED AUI* au) {
  D1("test_scrollbar_click_in_track start");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(400, 300);
  w->DisableResize();
  AScrollBar* sb = AScrollBar::AttachTo(w, AUIOrientation::vertical);
  sb->Move(380, 40);
  sb->Resize(16, 240);
  sb->Range(0, 100);
  sb->Value(0);
  sb->MouseClick(5, 200);
  TEST_ASSERT_EQ(sb->Value(), 10, 2);
  D2("val {}", sb->Value())
  sb->MouseClick(5, 200);
  D2("val {}", sb->Value())
  TEST_ASSERT_EQ(sb->Value(), 20, 3);
  sb->MouseClick(5, 20);
  D2("val {}", sb->Value())
  TEST_ASSERT_EQ(sb->Value(), 10, 3);
  sb->MouseClick(1, 1);
  D2("val {}", sb->Value())
  TEST_ASSERT_EQ(sb->Value(), 9, 3);
  D1("test_scrollbar_click_in_track passed");
  return 0;
}
// ------------------------------------------------------------------
// Drag simulation (thumb follows mouse)
// ------------------------------------------------------------------
int32_t test_scrollbar_drag(AUI* au) {
  D1("test_scrollbar_drag start");
  TEST_ASSERT_NE(au, nullptr, 1);
  UNUSED AWindow* w = au->MainWnd();

  w->EnableResize();
  w->Resize(400, 300);
  AScrollBar* sb = AScrollBar::AttachTo(w, AUIOrientation::vertical);
  sb->Move(380, 40);
  sb->Resize(16, 240);
  sb->Range(0, 100);
  sb->Value(50);
  sb->OnMouseDownLeft(5, 125);
  UNUSED int32_t startVal = sb->Value();
  D1("val1 {}", sb->Value())
  sb->OnMouseMove(5, 145);
  sb->OnMouseUpLeft(5, 145);
  UNUSED int32_t afterMove = sb->Value();
  D1("val2 {}", sb->Value())
  TEST_ASSERT_EQ(afterMove, 59, 2);

  D1("test_scrollbar_drag passed");
  return 0;
}
// ------------------------------------------------------------------
// Large range regression test
// ------------------------------------------------------------------
int32_t test_scrollbar_large_range(AUI* au) {
  D1("test_scrollbar_regression start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AScrollBar* sb = AScrollBar::AttachTo(w);
  TEST_ASSERT_NE(sb, nullptr, 1);
  sb->Orient(AUIOrientation::vertical);
  sb->Resize(20, 200);
  sb->Range(0, 10000000);
  sb->PageStep(1000);
  sb->SingleStep(100);
  sb->Arrows(true);
  uint32_t thumbPos = sb->ThumbPosition();
  uint32_t thumbLen = sb->ThumbLength();
  uint32_t trackStart = 12;
  int32_t thumbCenterY = SafeINT32(trackStart + thumbPos + thumbLen / 2);
  AWidget* consumed = sb->MouseClick(10, thumbCenterY);
  TEST_ASSERT_EQ(consumed, sb, 1);
  int32_t newValue = sb->Value();
  TEST_ASSERT_EQ(newValue, 0, 2);
  D1("test_scrollbar_regression passed");
  return 0;
}
// ------------------------------------------------------------------
// Main: run all tests with timed test harness
// ------------------------------------------------------------------
int main() {
  UNUSED int32_t testsfailed = 0;
//  UNUSED AUI* au = AUI::Create("test");
//  UNUSED AWindow* w = au->MainWnd();

//   au->ProcessMessages();
//  delete au;

  testsfailed += runTimedTest(test_scrollbar_attachment, 1);
  testsfailed += runTimedTest(test_scrollbar_orientation, 1);
  testsfailed += runTimedTest(test_scrollbar_range_and_value, 1);
  testsfailed += runTimedTest(test_scrollbar_page_step, 1);
  testsfailed += runTimedTest(test_scrollbar_colors, 1);
  testsfailed += runTimedTest(test_scrollbar_callback, 1);
  testsfailed += runTimedTest(test_scrollbar_click_in_track, 1);
  testsfailed += runTimedTest(test_scrollbar_drag, 1);
  testsfailed += runTimedTest(test_scrollbar_large_range, 1);

  D("test suite complete");
  return testsfailed;
}



