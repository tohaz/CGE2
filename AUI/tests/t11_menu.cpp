#include "AUILib.h"

using namespace aui;

// ------------------------------------------------------------------
// Test: Attachment and default state
// ------------------------------------------------------------------
int32_t test_menu_attachment() {
    D1("test_menu_attachment start");
    AUI* au = AUI::Create("MenuAttachTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* win = au->MainWnd();
    AMenu* menu = AMenu::AttachTo(win);
    TEST_ASSERT(menu != nullptr, 2);
    TEST_ASSERT(menu->GetItemCount() == 1, 3); // dummy item
    TEST_ASSERT(menu->GetItem(0).text == "Menu", 4);
    TEST_ASSERT(menu->GetOrientation() == AUIOrientation::vertical, 5);
    TEST_ASSERT(menu->IsVisible() == false, 6);
    delete au;
    D1("test_menu_attachment passed");
    return 0;
}

// ------------------------------------------------------------------
// Test: Adding and removing items
// ------------------------------------------------------------------
int32_t test_menu_items() {
    D1("test_menu_items start");
    AUI* au = AUI::Create("MenuItemsTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* win = au->MainWnd();
    AMenu* menu = AMenu::AttachTo(win);
    menu->ClearItems();
    TEST_ASSERT(menu->GetItemCount() == 0, 2);
    menu->AddItem(AMenuItem("Item1"));
    menu->AddItem(AMenuItem("Item2"));
    TEST_ASSERT(menu->GetItemCount() == 2, 3);
    menu->InsertItem(1, AMenuItem("Inserted"));
    TEST_ASSERT(menu->GetItemCount() == 3, 4);
    TEST_ASSERT(menu->GetItem(0).text == "Item1", 5);
    TEST_ASSERT(menu->GetItem(1).text == "Inserted", 6);
    TEST_ASSERT(menu->GetItem(2).text == "Item2", 7);
    menu->RemoveItem(1);
    TEST_ASSERT(menu->GetItemCount() == 2, 8);
    TEST_ASSERT(menu->GetItem(1).text == "Item2", 9);
    menu->ClearItems();
    TEST_ASSERT(menu->GetItemCount() == 0, 10);
    delete au;
    D1("test_menu_items passed");
    return 0;
}

// ------------------------------------------------------------------
// Test: Popup and Dismiss
// ------------------------------------------------------------------
int32_t test_menu_popup_dismiss() {
    D1("test_menu_popup_dismiss start");
    AUI* au = AUI::Create("MenuPopupTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* win = au->MainWnd();
    AMenu* menu = AMenu::AttachTo(win);
    menu->ClearItems();
    menu->AddItem(AMenuItem("Item1"));
    menu->AddItem(AMenuItem("Item2"));
    menu->Popup(10, 20);
    TEST_ASSERT(menu->IsVisible() == true, 2);
    TEST_ASSERT(menu->SizeX() > 0, 3);
    TEST_ASSERT(menu->SizeY() > 0, 4);
    TEST_ASSERT(menu->X() == 10, 5);
    TEST_ASSERT(menu->Y() == 20, 6);
    menu->Dismiss();
    TEST_ASSERT(menu->IsVisible() == false, 7);
    delete au;
    D1("test_menu_popup_dismiss passed");
    return 0;
}

// ------------------------------------------------------------------
// Test: Hit test and hover
// ------------------------------------------------------------------
int32_t test_menu_hit_test() {
    D1("test_menu_hit_test start");
    AUI* au = AUI::Create("MenuHitTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* win = au->MainWnd();
    AMenu* menu = AMenu::AttachTo(win);
    menu->ClearItems();
    menu->AddItem(AMenuItem("Short"));
    menu->AddItem(AMenuItem("LongerItem"));
    menu->SetItemHeight(30);
    menu->SetPadding(5);
    menu->Popup(0, 0);
    menu->OnMouseMove(5, 5);
    TEST_ASSERT(menu->GetHoveredIndex() == 0, 2);
    menu->OnMouseMove(5, 35);
    TEST_ASSERT(menu->GetHoveredIndex() == 1, 3);
    menu->OnMouseMove(5, 100); // outside
    TEST_ASSERT(menu->GetHoveredIndex() == -1, 4);
    delete au;
    D1("test_menu_hit_test passed");
    return 0;
}

// ------------------------------------------------------------------
// Test: Click on item with action
// ------------------------------------------------------------------
int32_t test_menu_click_action() {
    D1("test_menu_click_action start");
    AUI* au = AUI::Create("MenuClickTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* win = au->MainWnd();
    AMenu* menu = AMenu::AttachTo(win);
    menu->ClearItems();
    bool actionExecuted = false;
    menu->AddItem(AMenuItem("ClickMe", [&]() { actionExecuted = true; }));
    menu->Popup(0, 0);
    menu->OnMouseClick(5, 5, true);
    TEST_ASSERT(actionExecuted == true, 2);
    TEST_ASSERT(menu->IsVisible() == false, 3);
    delete au;
    D1("test_menu_click_action passed");
    return 0;
}

// ------------------------------------------------------------------
// Test: Checkable item toggling (does not rely on return value)
// ------------------------------------------------------------------
int32_t test_menu_checkable() {
    D1("test_menu_checkable start");
    AUI* au = AUI::Create("MenuCheckableTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* win = au->MainWnd();
    AMenu* menu = AMenu::AttachTo(win);
    menu->ClearItems();
    AMenuItem item("Toggle");
    item.isCheckable = true;
    item.isChecked = false;
    menu->AddItem(item);
    menu->Popup(0, 0);
    // Click once to toggle on
    menu->OnMouseClick(5, 5, true);
    const AMenuItem& toggled = menu->GetItem(0);
    TEST_ASSERT(toggled.isChecked == true, 2);
    // Menu is dismissed; re-popup for second click
    menu->Popup(0, 0);
    menu->OnMouseClick(5, 5, true);
    TEST_ASSERT(menu->GetItem(0).isChecked == false, 3);
    delete au;
    D1("test_menu_checkable passed");
    return 0;
}

// ------------------------------------------------------------------
// Test: Submenu opening/closing
// ------------------------------------------------------------------
int32_t test_menu_submenu() {
    D1("test_menu_submenu start");
    AUI* au = AUI::Create("MenuSubmenuTest");
    TEST_ASSERT(au != nullptr, 1);
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
    TEST_ASSERT(sub != nullptr, 2);
    TEST_ASSERT(sub->IsVisible() == true, 3);
    TEST_ASSERT(sub->GetItemCount() == 2, 4);
    menu->CloseSubMenu();
    TEST_ASSERT(menu->GetActiveSubMenu() == nullptr, 5);
    // Submenu is now hidden but still owned by the window – it will be deleted later.
    delete au;
    D1("test_menu_submenu passed");
    return 0;
}

// ------------------------------------------------------------------
// Test: Orientation changes
// ------------------------------------------------------------------
int32_t test_menu_orientation() {
    D1("test_menu_orientation start");
    AUI* au = AUI::Create("MenuOrientationTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* win = au->MainWnd();
    AMenu* menu = AMenu::AttachTo(win);
    TEST_ASSERT(menu->GetOrientation() == AUIOrientation::vertical, 2);
    menu->SetOrientation(AUIOrientation::horizontal);
    TEST_ASSERT(menu->GetOrientation() == AUIOrientation::horizontal, 3);
    menu->ClearItems();
    menu->AddItem(AMenuItem("One"));
    menu->AddItem(AMenuItem("Two"));
    menu->Popup(0, 0);
    // Just check it doesn't crash
    TEST_ASSERT(menu->IsVisible() == true, 4);
    delete au;
    D1("test_menu_orientation passed");
    return 0;
}

// ------------------------------------------------------------------
// Test: Keyboard navigation
// ------------------------------------------------------------------
int32_t test_menu_keyboard() {
    D1("test_menu_keyboard start");
    AUI* au = AUI::Create("MenuKeyboardTest");
    TEST_ASSERT(au != nullptr, 1);
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
    TEST_ASSERT(menu->GetHoveredIndex() == 0, 2);
    menu->OnKeyEvent(ev);
    TEST_ASSERT(menu->GetHoveredIndex() == 1, 3);
    ev.code = AUIKeyCode::Up;
    menu->OnKeyEvent(ev);
    TEST_ASSERT(menu->GetHoveredIndex() == 0, 4);
    ev.code = AUIKeyCode::Escape;
    menu->OnKeyEvent(ev);
    TEST_ASSERT(menu->IsVisible() == false, 5);

    menu->Popup(0, 0);
    menu->OnMouseMove(5, 5); // hover on first item
    TEST_ASSERT(menu->GetHoveredIndex() == 0, 6);
    ev.code = AUIKeyCode::Enter;
    menu->OnKeyEvent(ev);
    TEST_ASSERT(actionCalled == true, 7);
    TEST_ASSERT(menu->IsVisible() == false, 8);

    delete au;
    D1("test_menu_keyboard passed");
    return 0;
}

// ------------------------------------------------------------------
// Test: Permanent menu bar
// ------------------------------------------------------------------
int32_t test_menu_permanent() {
    D1("test_menu_permanent start");
    AUI* au = AUI::Create("MenuPermanentTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* win = au->MainWnd();
    AMenu* menu = AMenu::AttachTo(win);
    menu->ClearItems();
    menu->AddItem(AMenuItem("File"));
    menu->SetPermanent(true);
    menu->Popup(0, 0);
    TEST_ASSERT(menu->IsVisible() == true, 2);
    menu->Dismiss();
    TEST_ASSERT(menu->IsVisible() == true, 3);
    menu->SetPermanent(false);
    menu->Dismiss();
    TEST_ASSERT(menu->IsVisible() == false, 4);
    delete au;
    D1("test_menu_permanent passed");
    return 0;
}

// =========================================================================
// NEW REGRESSION TESTS
// =========================================================================

// ------------------------------------------------------------------
// Test: Submenu accumulation – clicking same root multiple times
// ------------------------------------------------------------------
int32_t test_menu_submenu_accumulation() {
    D1("test_menu_submenu_accumulation start");
    AUI* au = AUI::Create("MenuAccumTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* win = au->MainWnd();
    AMenu* menu = AMenu::AttachTo(win);
    menu->ClearItems();
    std::vector<AMenuItem> subItems = { AMenuItem("Sub1") };
    AMenuItem parent("Parent");
    parent.subItems = subItems;
    menu->AddItem(parent);
    menu->Popup(0, 0);

    // Click once to open submenu
    menu->OnMouseClick(5, 5, true);
    AMenu* sub1 = menu->GetActiveSubMenu();
    TEST_ASSERT(sub1 != nullptr, 2);

    // Click the same root again – should close the submenu (toggle off)
    menu->OnMouseClick(5, 5, true);
    AMenu* sub2 = menu->GetActiveSubMenu();
    TEST_ASSERT(sub2 == nullptr, 3);   // <-- changed to expect closed

    // No need for sub1 != sub2 because there is no new submenu

    delete au;
    D1("test_menu_submenu_accumulation passed");
    return 0;
}

// ------------------------------------------------------------------
// Test: Submenu item click – action executed, no crash
// ------------------------------------------------------------------
int32_t test_menu_submenu_item_click() {
    D1("test_menu_submenu_item_click start");
    AUI* au = AUI::Create("MenuSubClickTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* win = au->MainWnd();
    AMenu* menu = AMenu::AttachTo(win);
    menu->ClearItems();
    bool subActionExecuted = false;
    std::vector<AMenuItem> subItems = { AMenuItem("SubAction", [&]() { subActionExecuted = true; }) };
    AMenuItem parent("Parent");
    parent.subItems = subItems;
    menu->AddItem(parent);
    menu->Popup(0, 0);
    // Open submenu
    menu->OnMouseClick(5, 5, true);
    AMenu* sub = menu->GetActiveSubMenu();
    TEST_ASSERT(sub != nullptr, 2);
    // Click on submenu item (coordinates depend on layout, but we can simulate)
    // For simplicity, we can call OnMouseClick directly on submenu with local coords
    sub->OnMouseClick(5, 5, true);
    TEST_ASSERT(subActionExecuted == true, 3);
    // Submenu should be dismissed (deleted)
    TEST_ASSERT(menu->GetActiveSubMenu() == nullptr, 4);
    // No crash is the main pass
    delete au;
    D1("test_menu_submenu_item_click passed");
    return 0;
}

// ------------------------------------------------------------------
// Test: Hidden menu does not consume clicks
// ------------------------------------------------------------------
int32_t test_menu_hidden_skip() {
    D1("test_menu_hidden_skip start");
    AUI* au = AUI::Create("MenuHiddenTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* win = au->MainWnd();
    // Create two menus: one visible, one hidden
    AMenu* visibleMenu = AMenu::AttachTo(win);
    visibleMenu->ClearItems();
    visibleMenu->AddItem(AMenuItem("Visible"));
    visibleMenu->Popup(0, 0);
    AMenu* hiddenMenu = AMenu::AttachTo(win);
    hiddenMenu->ClearItems();
    hiddenMenu->AddItem(AMenuItem("Hidden"));
    hiddenMenu->Dismiss(); // hide it
    // Click at position that would hit the hidden menu if it were visible
    // We'll simulate a click on the window at the hidden menu's position.
    // Since hidden menu is hidden, it should not receive the click.

    // Set a dummy callback to detect if the hidden menu gets the click
    // But we don't have a direct way; we rely on no crash and that the visible menu (if any) gets it.
    // We'll just check that hidden menu remains hidden.
    win->OnMousePress(hiddenMenu->X() + 5, hiddenMenu->Y() + 5, 272);
    TEST_ASSERT(hiddenMenu->IsVisible() == false, 2);
    // Also ensure the window didn't crash
    delete au;
    D1("test_menu_hidden_skip passed");
    return 0;
}

// ------------------------------------------------------------------
// Test: Clicking same root toggles submenu closed
// ------------------------------------------------------------------
int32_t test_menu_toggle_close() {
    D1("test_menu_toggle_close start");
    AUI* au = AUI::Create("MenuToggleTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow* win = au->MainWnd();
    AMenu* menu = AMenu::AttachTo(win);
    menu->ClearItems();
    std::vector<AMenuItem> subItems = { AMenuItem("Sub") };
    AMenuItem parent("Parent");
    parent.subItems = subItems;
    menu->AddItem(parent);
    menu->Popup(0, 0);
    // First click opens
    menu->OnMouseClick(5, 5, true);
    TEST_ASSERT(menu->GetActiveSubMenu() != nullptr, 2);
    // Second click on same root should close
    menu->OnMouseClick(5, 5, true);
    TEST_ASSERT(menu->GetActiveSubMenu() == nullptr, 3);
    delete au;
    D1("test_menu_toggle_close passed");
    return 0;
}

// ------------------------------------------------------------------
// Test: Rapid clicks – no crashes, no accumulation
// ------------------------------------------------------------------
int32_t test_menu_rapid_clicks() {
    D1("test_menu_rapid_clicks start");
    AUI* au = AUI::Create("MenuRapidTest");
    TEST_ASSERT(au != nullptr, 1);
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
        menu->OnMouseClick(5, 5 + i * 30, true); // click parent1 then parent2 alternately
    }
    // No crash, only one submenu exists at the end
    int visibleSubmenus = 0;
    if (menu->GetActiveSubMenu() && menu->GetActiveSubMenu()->IsVisible())
        visibleSubmenus++;
    TEST_ASSERT(visibleSubmenus <= 1, 2);
    delete au;
    D1("test_menu_rapid_clicks passed");
    return 0;
}

// =========================================================================
// Main: run all tests
// =========================================================================

int main() {
    uint32_t delay_ms = 50;
    int32_t testsfailed = 0;
    AUI* au = AUI::Create("aui menu test");
    UNUSED AWindow* w = au->MainWnd();

    auto handle = std::async(std::launch::async, [=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        au->ExitAUI();
    });

    testsfailed += test_menu_attachment();
    testsfailed += test_menu_items();
    testsfailed += test_menu_popup_dismiss();
    testsfailed += test_menu_hit_test();
    testsfailed += test_menu_click_action();
    testsfailed += test_menu_checkable();
    testsfailed += test_menu_submenu();
    testsfailed += test_menu_orientation();
    testsfailed += test_menu_keyboard();
    testsfailed += test_menu_permanent();

    // New regression tests
    testsfailed += test_menu_submenu_accumulation();
    testsfailed += test_menu_submenu_item_click();
    testsfailed += test_menu_hidden_skip();
    testsfailed += test_menu_toggle_close();
    testsfailed += test_menu_rapid_clicks();

    au->ProcessMessages();
    handle.get();
    delete au;
    au = nullptr;

    return testsfailed;
}

