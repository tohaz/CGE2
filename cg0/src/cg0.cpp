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

#include "AUILib.h"

using namespace aui;

static AWidget* OnClick(AWidget* wi, void* data, int32_t, int32_t) {
  D1("callback fired")
  AWindow* w2 = (AWindow*)data;
  if(w2->Visible()) {
    wi->Text("Show");
    w2->Hide();
  }
  else {
    wi->Text("Hide");
    w2->Show();
  }
  return wi;
}

int32_t main() {
//  AUI* au = AUI::Create("test", AUIWindowType::X11);
//  AUI* au = AUI::Create("test", AUIWindowType::Wayland);
  AUI* au = AUI::Create("test");
  AWindow* w = au->MainWnd();
  w->BGColor(0xFF222222);   // dark background

  UNUSED AWindow* w2 = AWindow::AttachTo(au, "w2");

  AButton *bn = AButton::AttachTo(w);
  bn->Text("Hide");
  bn->SetMouseClickCallback(OnClick, w2);
  w2->Decorations(false);

  AButton *bn2 = AButton::AttachTo(w2);
  bn2->Text("Hide");
  bn2->SetMouseClickCallback(OnClick, w);

  au->ProcessMessages();

  delete au;
  return 0;
}


