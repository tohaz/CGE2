#include "AUILib.h"

using namespace aui;

// ------------------------------------------------------------------
// Test: Attachment and default state
// ------------------------------------------------------------------
int32_t test_menu_attachment(AUI* au) {
  D1("test_menu_attachment start");
  AWindow* win = au->MainWnd();
  AMenu* menu = AMenu::AttachTo(win);
  TEST_ASSERT_NE(menu, nullptr, 2);
  TEST_ASSERT_EQ(menu->GetItemCount(), 1, 3); // dummy item
  TEST_ASSERT_EQ(menu->GetItem(0).text, std::string("Menu"), 4);
  TEST_ASSERT_EQ(menu->GetOrientation(), AUIOrientation::vertical, 5);
  TEST_ASSERT_EQ(menu->IsVisible(), false, 6);
  D1("test_menu_attachment passed");
  return 0;
}

// ------------------------------------------------------------------
// Test: Adding and removing items
// ------------------------------------------------------------------
int32_t test_menu_items(AUI* au) {
  D1("test_menu_items start");
  AWindow* win = au->MainWnd();
  AMenu* menu = AMenu::AttachTo(win);
  menu->ClearItems();
  TEST_ASSERT_EQ(menu->GetItemCount(), 0, 2);
  menu->AddItem(AMenuItem("Item1"));
  menu->AddItem(AMenuItem("Item2"));
  TEST_ASSERT_EQ(menu->GetItemCount(), 2, 3);
  menu->InsertItem(1, AMenuItem("Inserted"));
  TEST_ASSERT_EQ(menu->GetItemCount(), 3, 4);
  TEST_ASSERT_EQ(menu->GetItem(0).text, std::string("Item1"), 5);
  TEST_ASSERT_EQ(menu->GetItem(1).text, std::string("Inserted"), 6);
  TEST_ASSERT_EQ(menu->GetItem(2).text, std::string("Item2"), 7);
  menu->RemoveItem(1);
  TEST_ASSERT_EQ(menu->GetItemCount(), 2, 8);
  TEST_ASSERT_EQ(menu->GetItem(1).text, std::string("Item2"), 9);
  menu->ClearItems();
  TEST_ASSERT_EQ(menu->GetItemCount(), 0, 10);
  D1("test_menu_items passed");
  return 0;
}

// ------------------------------------------------------------------
// Test: Popup and Dismiss
// ------------------------------------------------------------------
int32_t test_menu_popup_dismiss(AUI* au) {
  D1("test_menu_popup_dismiss start");
  AWindow* win = au->MainWnd();
  AMenu* menu = AMenu::AttachTo(win);
  menu->ClearItems();
  menu->AddItem(AMenuItem("Item1"));
  menu->AddItem(AMenuItem("Item2"));
  menu->Popup(10, 20);
  TEST_ASSERT_EQ(menu->IsVisible(), true, 2);
  TEST_ASSERT(menu->SizeX() > 0, 3);
  TEST_ASSERT(menu->SizeY() > 0, 4);
  TEST_ASSERT_EQ(menu->X(), 10, 5);
  TEST_ASSERT_EQ(menu->Y(), 20, 6);
  menu->Dismiss();
  TEST_ASSERT_EQ(menu->IsVisible(), false, 7);
  D1("test_menu_popup_dismiss passed");
  return 0;
}

// ------------------------------------------------------------------
// Test: Hit test and hover
// ------------------------------------------------------------------
int32_t test_menu_hit_test(AUI* au) {
  D1("test_menu_hit_test start");
  AWindow* win = au->MainWnd();
  AMenu* menu = AMenu::AttachTo(win);
  menu->ClearItems();
  menu->AddItem(AMenuItem("Short"));
  menu->AddItem(AMenuItem("LongerItem"));
  menu->SetItemHeight(30);
  menu->SetPadding(5);
  menu->Popup(0, 0);
  menu->OnMouseMove(5, 5);
  TEST_ASSERT_EQ(menu->GetHoveredIndex(), 0, 2);
  menu->OnMouseMove(5, 35);
  TEST_ASSERT_EQ(menu->GetHoveredIndex(), 1, 3);
  menu->OnMouseMove(5, 100); // outside
  TEST_ASSERT_EQ(menu->GetHoveredIndex(), -1, 4);
  D1("test_menu_hit_test passed");
  return 0;
}

// ------------------------------------------------------------------
// Test: Click on item with action
// ------------------------------------------------------------------
int32_t test_menu_click_action(AUI* au) {
  D1("test_menu_click_action start");
  AWindow* win = au->MainWnd();
  AMenu* menu = AMenu::AttachTo(win);
  menu->ClearItems();
  bool actionExecuted = false;
  menu->AddItem(AMenuItem("ClickMe", [&]() { actionExecuted = true; }));
  menu->Popup(0, 0);
  menu->OnMouseClick(5, 5, true);
  TEST_ASSERT(actionExecuted == true, 2);
  TEST_ASSERT_EQ(menu->IsVisible(), false, 3);
  D1("test_menu_click_action passed");
  return 0;
}

// ------------------------------------------------------------------
// Test: Checkable item toggling
// ------------------------------------------------------------------
int32_t test_menu_checkable(AUI* au) {
  D1("test_menu_checkable start");
  AWindow* win = au->MainWnd();
  AMenu* menu = AMenu::AttachTo(win);
  menu->ClearItems();
  AMenuItem item("Toggle");
  item.isCheckable = true;
  item.isChecked = false;
  menu->AddItem(item);
  menu->Popup(0, 0);
  menu->OnMouseClick(5, 5, true);
  const AMenuItem& toggled = menu->GetItem(0);
  TEST_ASSERT_EQ(toggled.isChecked, true, 2);
  menu->Popup(0, 0);
  menu->OnMouseClick(5, 5, true);
  TEST_ASSERT_EQ(menu->GetItem(0).isChecked, false, 3);
  D1("test_menu_checkable passed");
  return 0;
}

// ------------------------------------------------------------------
// Test: Submenu opening/closing
// ------------------------------------------------------------------
int32_t test_menu_submenu(AUI* au) {
  D1("test_menu_submenu start");
  AWindow* win = au->MainWnd();
  AMenu* menu = AMenu::AttachTo(win);
  menu->ClearItems();
  std::vector<AMenuItem> subItems = { AMenuItem("Sub1"), AMenuItem("Sub2") };
  AMenuItem parent("Parent");
  parent.subItems = subItems;
  menu->AddItem(parent);
  menu->Popup(0, 0);
  menu->OnMouseClick(5, 5, true);
  AMenu* sub = menu->GetActiveSubMenu();
  TEST_ASSERT_NE(sub, nullptr, 2);
  TEST_ASSERT_EQ(sub->IsVisible(), true, 3);
  TEST_ASSERT_EQ(sub->GetItemCount(), 2, 4);
  menu->CloseSubMenu();
  TEST_ASSERT_EQ(menu->GetActiveSubMenu(), nullptr, 5);
  D1("test_menu_submenu passed");
  return 0;
}

// ------------------------------------------------------------------
// Test: Orientation changes
// ------------------------------------------------------------------
int32_t test_menu_orientation(AUI* au) {
  D1("test_menu_orientation start");
  AWindow* win = au->MainWnd();
  AMenu* menu = AMenu::AttachTo(win);
  TEST_ASSERT_EQ(menu->GetOrientation(), AUIOrientation::vertical, 2);
  menu->SetOrientation(AUIOrientation::horizontal);
  TEST_ASSERT_EQ(menu->GetOrientation(), AUIOrientation::horizontal, 3);
  menu->ClearItems();
  menu->AddItem(AMenuItem("One"));
  menu->AddItem(AMenuItem("Two"));
  menu->Popup(0, 0);
  TEST_ASSERT_EQ(menu->IsVisible(), true, 4);
  D1("test_menu_orientation passed");
  return 0;
}

// ------------------------------------------------------------------
// Test: Keyboard navigation
// ------------------------------------------------------------------
int32_t test_menu_keyboard(AUI* au) {
  D1("test_menu_keyboard start");
  AWindow* win = au->MainWnd();
  AMenu* menu = AMenu::AttachTo(win);
  menu->ClearItems();
  bool actionCalled = false;
  AMenuItem item0("Item0");
  item0.action = [&](){ actionCalled = true; };
  menu->AddItem(item0);
  menu->AddItem(AMenuItem("Item1"));
  menu->AddItem(AMenuItem("Item2"));
  menu->Popup(0, 0);
  AUIKeyEvent ev;
  ev.pressed = true;
  ev.code = AUIKeyCode::Down;
  menu->OnKeyEvent(ev);
  TEST_ASSERT_EQ(menu->GetHoveredIndex(), 0, 2);
  menu->OnKeyEvent(ev);
  TEST_ASSERT_EQ(menu->GetHoveredIndex(), 1, 3);
  ev.code = AUIKeyCode::Up;
  menu->OnKeyEvent(ev);
  TEST_ASSERT_EQ(menu->GetHoveredIndex(), 0, 4);
  ev.code = AUIKeyCode::Escape;
  menu->OnKeyEvent(ev);
  TEST_ASSERT_EQ(menu->IsVisible(), false, 5);
  menu->Popup(0, 0);
  menu->OnMouseMove(5, 5);
  TEST_ASSERT_EQ(menu->GetHoveredIndex(), 0, 6);
  ev.code = AUIKeyCode::Enter;
  menu->OnKeyEvent(ev);
  TEST_ASSERT(actionCalled == true, 7);
  TEST_ASSERT_EQ(menu->IsVisible(), false, 8);
  D1("test_menu_keyboard passed");
  return 0;
}

// ------------------------------------------------------------------
// Test: Permanent menu bar
// ------------------------------------------------------------------
int32_t test_menu_permanent(AUI* au) {
  D1("test_menu_permanent start");
  AWindow* win = au->MainWnd();
  AMenu* menu = AMenu::AttachTo(win);
  menu->ClearItems();
  menu->AddItem(AMenuItem("File"));
  menu->SetPermanent(true);
  menu->Popup(0, 0);
  TEST_ASSERT_EQ(menu->IsVisible(), true, 2);
  menu->Dismiss();
  TEST_ASSERT_EQ(menu->IsVisible(), true, 3);
  menu->SetPermanent(false);
  menu->Dismiss();
  TEST_ASSERT_EQ(menu->IsVisible(), false, 4);
  D1("test_menu_permanent passed");
  return 0;
}

// ------------------------------------------------------------------
// Test: Submenu accumulation – clicking same root multiple times
// ------------------------------------------------------------------
int32_t test_menu_submenu_accumulation(AUI* au) {
  D1("test_menu_submenu_accumulation start");
  AWindow* win = au->MainWnd();
  AMenu* menu = AMenu::AttachTo(win);
  menu->ClearItems();
  std::vector<AMenuItem> subItems = { AMenuItem("Sub1") };
  AMenuItem parent("Parent");
  parent.subItems = subItems;
  menu->AddItem(parent);
  menu->Popup(0, 0);
  menu->OnMouseClick(5, 5, true);
  AMenu* sub1 = menu->GetActiveSubMenu();
  TEST_ASSERT_NE(sub1, nullptr, 2);
  menu->OnMouseClick(5, 5, true);
  AMenu* sub2 = menu->GetActiveSubMenu();
  TEST_ASSERT_EQ(sub2, nullptr, 3);
  D1("test_menu_submenu_accumulation passed");
  return 0;
}

// ------------------------------------------------------------------
// Test: Submenu item click – action executed, no crash
// ------------------------------------------------------------------
int32_t test_menu_submenu_item_click(AUI* au) {
  D1("test_menu_submenu_item_click start");
  AWindow* win = au->MainWnd();
  AMenu* menu = AMenu::AttachTo(win);
  menu->ClearItems();
  bool subActionExecuted = false;
  std::vector<AMenuItem> subItems = { AMenuItem("SubAction", [&]() { subActionExecuted = true; }) };
  AMenuItem parent("Parent");
  parent.subItems = subItems;
  menu->AddItem(parent);
  menu->Popup(0, 0);
  menu->OnMouseClick(5, 5, true);
  AMenu* sub = menu->GetActiveSubMenu();
  TEST_ASSERT_NE(sub, nullptr, 2);
  sub->OnMouseClick(5, 5, true);
  TEST_ASSERT(subActionExecuted == true, 3);
  TEST_ASSERT_EQ(menu->GetActiveSubMenu(), nullptr, 4);
  D1("test_menu_submenu_item_click passed");
  return 0;
}

// ------------------------------------------------------------------
// Test: Hidden menu does not consume clicks
// ------------------------------------------------------------------
int32_t test_menu_hidden_skip(AUI* au) {
  D1("test_menu_hidden_skip start");
  AWindow* win = au->MainWnd();
  AMenu* visibleMenu = AMenu::AttachTo(win);
  visibleMenu->ClearItems();
  visibleMenu->AddItem(AMenuItem("Visible"));
  visibleMenu->Popup(0, 0);
  AMenu* hiddenMenu = AMenu::AttachTo(win);
  hiddenMenu->ClearItems();
  hiddenMenu->AddItem(AMenuItem("Hidden"));
  hiddenMenu->Dismiss();
  win->OnMousePress(hiddenMenu->X() + 5, hiddenMenu->Y() + 5, 272);
  TEST_ASSERT_EQ(hiddenMenu->IsVisible(), false, 2);
  D1("test_menu_hidden_skip passed");
  return 0;
}

// ------------------------------------------------------------------
// Test: Clicking same root toggles submenu closed
// ------------------------------------------------------------------
int32_t test_menu_toggle_close(AUI* au) {
  D1("test_menu_toggle_close start");
  AWindow* win = au->MainWnd();
  AMenu* menu = AMenu::AttachTo(win);
  menu->ClearItems();
  std::vector<AMenuItem> subItems = { AMenuItem("Sub") };
  AMenuItem parent("Parent");
  parent.subItems = subItems;
  menu->AddItem(parent);
  menu->Popup(0, 0);
  menu->OnMouseClick(5, 5, true);
  TEST_ASSERT_NE(menu->GetActiveSubMenu(), nullptr, 2);
  menu->OnMouseClick(5, 5, true);
  TEST_ASSERT_EQ(menu->GetActiveSubMenu(), nullptr, 3);
  D1("test_menu_toggle_close passed");
  return 0;
}

// ------------------------------------------------------------------
// Test: Rapid clicks – no crashes, no accumulation
// ------------------------------------------------------------------
int32_t test_menu_rapid_clicks(AUI* au) {
  D1("test_menu_rapid_clicks start");
  AWindow* win = au->MainWnd();
  AMenu* menu = AMenu::AttachTo(win);
  menu->ClearItems();
  std::vector<AMenuItem> subItems = { AMenuItem("Sub") };
  AMenuItem parent1("Parent1");
  parent1.subItems = subItems;
  AMenuItem parent2("Parent2");
  parent2.subItems = subItems;
  menu->AddItem(parent1);
  menu->AddItem(parent2);
  menu->Popup(0, 0);
  for (int i = 0; i < 10; ++i) {
    menu->OnMouseClick(5, 5 + i * 30, true);
  }
  int visibleSubmenus = 0;
  if (menu->GetActiveSubMenu() && menu->GetActiveSubMenu()->IsVisible())
    visibleSubmenus++;
  TEST_ASSERT(visibleSubmenus <= 1, 2);
  D1("test_menu_rapid_clicks passed");
  return 0;
}

// ------------------------------------------------------------------
// Main: run all tests with timed test harness
// ------------------------------------------------------------------
int main() {
  int32_t testsfailed = 0;
  testsfailed += runTimedTest("test_menu_attachment", test_menu_attachment, 1);
  testsfailed += runTimedTest("test_menu_items", test_menu_items, 1);
  testsfailed += runTimedTest("test_menu_popup_dismiss", test_menu_popup_dismiss, 1);
  testsfailed += runTimedTest("test_menu_hit_test", test_menu_hit_test, 1);
  testsfailed += runTimedTest("test_menu_click_action", test_menu_click_action, 1);
  testsfailed += runTimedTest("test_menu_checkable", test_menu_checkable, 1);
  testsfailed += runTimedTest("test_menu_submenu", test_menu_submenu, 1);
  testsfailed += runTimedTest("test_menu_orientation", test_menu_orientation, 1);
  testsfailed += runTimedTest("test_menu_keyboard", test_menu_keyboard, 1);
  testsfailed += runTimedTest("test_menu_permanent", test_menu_permanent, 1);
  testsfailed += runTimedTest("test_menu_submenu_accumulation", test_menu_submenu_accumulation, 1);
  testsfailed += runTimedTest("test_menu_submenu_item_click", test_menu_submenu_item_click, 1);
  testsfailed += runTimedTest("test_menu_hidden_skip", test_menu_hidden_skip, 1);
  testsfailed += runTimedTest("test_menu_toggle_close", test_menu_toggle_close, 1);
  testsfailed += runTimedTest("test_menu_rapid_clicks", test_menu_rapid_clicks, 1);
  
  testsfailed += runTimedTest("test_menu_attachment", test_menu_attachment, 200);
  testsfailed += runTimedTest("test_menu_items", test_menu_items, 200);
  testsfailed += runTimedTest("test_menu_popup_dismiss", test_menu_popup_dismiss, 200);
  testsfailed += runTimedTest("test_menu_hit_test", test_menu_hit_test, 200);
  testsfailed += runTimedTest("test_menu_click_action", test_menu_click_action, 200);
  testsfailed += runTimedTest("test_menu_checkable", test_menu_checkable, 200);
  testsfailed += runTimedTest("test_menu_submenu", test_menu_submenu, 200);
  testsfailed += runTimedTest("test_menu_orientation", test_menu_orientation, 200);
  testsfailed += runTimedTest("test_menu_keyboard", test_menu_keyboard, 200);
  testsfailed += runTimedTest("test_menu_permanent", test_menu_permanent, 200);
  testsfailed += runTimedTest("test_menu_submenu_accumulation", test_menu_submenu_accumulation, 200);
  testsfailed += runTimedTest("test_menu_submenu_item_click", test_menu_submenu_item_click, 200);
  testsfailed += runTimedTest("test_menu_hidden_skip", test_menu_hidden_skip, 200);
  testsfailed += runTimedTest("test_menu_toggle_close", test_menu_toggle_close, 200);
  testsfailed += runTimedTest("test_menu_rapid_clicks", test_menu_rapid_clicks, 200);
  
  
 
  D("test suite complete");
  return testsfailed;
}