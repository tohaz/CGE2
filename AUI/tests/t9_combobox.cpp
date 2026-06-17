#include <chrono>
#include <future>
#include <thread>
#include "AUILib.h"

using namespace aui;

// ------------------------------------------------------------------
// Attachment and basic properties
// ------------------------------------------------------------------
int32_t test_combobox_attachment() {
    D1("test_combobox_attachment start");
    AUI *au = AUI::Create("ComboBoxAttachTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow *win = au->MainWnd();
    AComboBox *cb = AComboBox::AttachTo(win);
    TEST_ASSERT(cb != nullptr, 2);
    TEST_ASSERT_EQ(cb->GetItemCount(), 0U, 3);
    TEST_ASSERT_EQ(cb->GetSelectedIndex(), -1, 4);
    TEST_ASSERT(cb->IsEditable() == true, 5);
    TEST_ASSERT(cb->IsDropDownOpen() == false, 6);
    delete au;
    D1("test_combobox_attachment passed");
    return 0;
}

// ------------------------------------------------------------------
// Adding and removing items
// ------------------------------------------------------------------
int32_t test_combobox_add_remove() {
    D1("test_combobox_add_remove start");
    AUI *au = AUI::Create("ComboBoxAddRemoveTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow *win = au->MainWnd();
    AComboBox *cb = AComboBox::AttachTo(win);
    cb->AddItem("Item1");
    cb->AddItem("Item2");
    cb->AddItem("Item3");
    TEST_ASSERT_EQ(cb->GetItemCount(), 3U, 2);
    TEST_ASSERT_EQ(cb->GetItem(0), std::string("Item1"), 3);
    TEST_ASSERT_EQ(cb->GetItem(1), std::string("Item2"), 4);
    TEST_ASSERT_EQ(cb->GetItem(2), std::string("Item3"), 5);
    cb->InsertItem(1, "Inserted");
    TEST_ASSERT_EQ(cb->GetItem(1), std::string("Inserted"), 6);
    TEST_ASSERT_EQ(cb->GetItem(2), std::string("Item2"), 7);
    cb->RemoveItem(2);
    TEST_ASSERT_EQ(cb->GetItem(2), std::string("Item3"), 8);
    TEST_ASSERT_EQ(cb->GetItemCount(), 3U, 9);
    cb->ClearItems();
    TEST_ASSERT_EQ(cb->GetItemCount(), 0U, 10);
    delete au;
    D1("test_combobox_add_remove passed");
    return 0;
}

// ------------------------------------------------------------------
// Selection
// ------------------------------------------------------------------
int32_t test_combobox_selection() {
    D1("test_combobox_selection start");
    AUI *au = AUI::Create("ComboBoxSelectionTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow *win = au->MainWnd();
    AComboBox *cb = AComboBox::AttachTo(win);
    cb->AddItem("A");
    cb->AddItem("B");
    cb->AddItem("C");
    cb->SetSelectedIndex(1);
    TEST_ASSERT_EQ(cb->GetSelectedIndex(), 1, 2);
    TEST_ASSERT_EQ(cb->GetSelectedText(), std::string("B"), 3);
    cb->ClearSelection();
    TEST_ASSERT_EQ(cb->GetSelectedIndex(), -1, 4);
    TEST_ASSERT_EQ(cb->GetSelectedText(), std::string(""), 5);
    delete au;
    D1("test_combobox_selection passed");
    return 0;
}

// ------------------------------------------------------------------
// Editable state
// ------------------------------------------------------------------
int32_t test_combobox_editable() {
    D1("test_combobox_editable start");
    AUI *au = AUI::Create("ComboBoxEditableTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow *win = au->MainWnd();
    AComboBox *cb = AComboBox::AttachTo(win);
    cb->SetEditable(false);
    TEST_ASSERT(cb->IsEditable() == false, 2);
    AInputBox *input = cb->GetInputBox();
    TEST_ASSERT(input->IsEditable() == false, 3);
    cb->SetEditable(true);
    TEST_ASSERT(cb->IsEditable() == true, 4);
    TEST_ASSERT(input->IsEditable() == true, 5);
    delete au;
    D1("test_combobox_editable passed");
    return 0;
}

// ------------------------------------------------------------------
// Dropdown open/close (direct API)
// ------------------------------------------------------------------
int32_t test_combobox_dropdown_toggle() {
    D1("test_combobox_dropdown_toggle start");
    AUI *au = AUI::Create("ComboBoxDropdownTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow *win = au->MainWnd();
    AComboBox *cb = AComboBox::AttachTo(win);
    cb->Move(50, 50);
    cb->Resize(200, 28);
    cb->AddItem("One");
    cb->AddItem("Two");
    cb->AddItem("Three");

    // Open via API
    cb->OpenDropDown();
    TEST_ASSERT(cb->IsDropDownOpen() == true, 2);
    cb->CloseDropDown();
    TEST_ASSERT(cb->IsDropDownOpen() == false, 3);

    // Toggle via API
    cb->ToggleDropDown();
    TEST_ASSERT(cb->IsDropDownOpen() == true, 4);
    cb->ToggleDropDown();
    TEST_ASSERT(cb->IsDropDownOpen() == false, 5);

    // Mouse simulation on button (should toggle)
    bool handled = cb->OnMouseClick(190, 14, true);
    TEST_ASSERT(handled, 6);
    handled = cb->OnMouseClick(190, 14, false);
    TEST_ASSERT(cb->IsDropDownOpen() == true, 7);

    // Click on input area (localX=50) – should open on release
    handled = cb->OnMouseClick(50, 14, true);
    TEST_ASSERT(handled, 8);
    handled = cb->OnMouseClick(50, 14, false);
    TEST_ASSERT(cb->IsDropDownOpen() == true, 9);

    cb->CloseDropDown();
    TEST_ASSERT(cb->IsDropDownOpen() == false, 10);

    delete au;
    D1("test_combobox_dropdown_toggle passed");
    return 0;
}

// ------------------------------------------------------------------
// Selection via dropdown (keyboard navigation after programmatic open)
// ------------------------------------------------------------------
int32_t test_combobox_select_from_dropdown() {
    D1("test_combobox_select_from_dropdown start");
    AUI *au = AUI::Create("ComboBoxSelectTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow *win = au->MainWnd();
    AComboBox *cb = AComboBox::AttachTo(win);
    cb->Move(50, 50);
    cb->Resize(200, 28);
    cb->AddItem("Apple");
    cb->AddItem("Banana");
    cb->AddItem("Cherry");

    // Open dropdown programmatically
    cb->OpenDropDown();
    TEST_ASSERT(cb->IsDropDownOpen() == true, 2);

    AUIKeyEvent ev;
    ev.pressed = true;
    ev.code = AUIKeyCode::Down;
    cb->OnKeyEvent(ev); // selects first item
    TEST_ASSERT_EQ(cb->GetSelectedIndex(), 0, 3);
    cb->OnKeyEvent(ev); // selects second item
    TEST_ASSERT_EQ(cb->GetSelectedIndex(), 1, 4);
    cb->OnKeyEvent(ev); // selects third item
    TEST_ASSERT_EQ(cb->GetSelectedIndex(), 2, 5);

    ev.code = AUIKeyCode::Enter;
    cb->OnKeyEvent(ev);
    TEST_ASSERT(cb->IsDropDownOpen() == false, 6);
    TEST_ASSERT_EQ(cb->GetSelectedIndex(), 2, 7);
    TEST_ASSERT_EQ(cb->GetSelectedText(), std::string("Cherry"), 8);

    delete au;
    D1("test_combobox_select_from_dropdown passed");
    return 0;
}

// ------------------------------------------------------------------
// Keyboard navigation (with programmatic open)
// ------------------------------------------------------------------
int32_t test_combobox_keyboard_navigation() {
    D1("test_combobox_keyboard_navigation start");
    AUI *au = AUI::Create("ComboBoxKeyNavTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow *win = au->MainWnd();
    AComboBox *cb = AComboBox::AttachTo(win);
    cb->Move(50, 50);
    cb->AddItem("Item1");
    cb->AddItem("Item2");
    cb->AddItem("Item3");
    cb->AddItem("Item4");
    cb->AddItem("Item5");

    // Open dropdown programmatically
    cb->OpenDropDown();
    TEST_ASSERT(cb->IsDropDownOpen() == true, 2);

    AUIKeyEvent ev;
    ev.pressed = true;

    // Down twice
    ev.code = AUIKeyCode::Down;
    cb->OnKeyEvent(ev);
    TEST_ASSERT_EQ(cb->GetSelectedIndex(), 0, 3);
    cb->OnKeyEvent(ev);
    TEST_ASSERT_EQ(cb->GetSelectedIndex(), 1, 4);

    // Up once
    ev.code = AUIKeyCode::Up;
    cb->OnKeyEvent(ev);
    TEST_ASSERT_EQ(cb->GetSelectedIndex(), 0, 5);

    // Escape closes
    ev.code = AUIKeyCode::Escape;
    cb->OnKeyEvent(ev);
    TEST_ASSERT(cb->IsDropDownOpen() == false, 6);

    delete au;
    D1("test_combobox_keyboard_navigation passed");
    return 0;
}

// ------------------------------------------------------------------
// Styling setters
// ------------------------------------------------------------------
int32_t test_combobox_styling() {
    D1("test_combobox_styling start");
    AUI *au = AUI::Create("ComboBoxStyleTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow *win = au->MainWnd();
    AComboBox *cb = AComboBox::AttachTo(win);
    cb->SetInputBoxBGColor(0xFF0000FF);
    cb->SetButtonBGColor(0xFFFF0000);
    cb->SetListBGColor(0xFF00FF00);
    cb->SetInputBoxTextColor(0xFF00FFFF);
    cb->SetButtonTextColor(0xFF00FF00);
    cb->SetListTextColor(0xFFFF00FF);
    cb->SetFontSize(16);
    TEST_ASSERT_EQ(cb->GetFontSize(), 16U, 2);
    // We can't easily verify colors, but we can ensure no crash.
    delete au;
    D1("test_combobox_styling passed");
    return 0;
}

// ------------------------------------------------------------------
// Scrollbars visibility (when many items)
// ------------------------------------------------------------------
int32_t test_combobox_scrollbars() {
    D1("test_combobox_scrollbars start");
    AUI *au = AUI::Create("ComboBoxScrollTest");
    TEST_ASSERT(au != nullptr, 1);
    AWindow *win = au->MainWnd();
    AComboBox *cb = AComboBox::AttachTo(win);
    cb->Move(50, 50);
    cb->Resize(150, 28);
    for (int i = 0; i < 50; ++i) {
        cb->AddItem("Item " + std::to_string(i));
    }
    cb->OpenDropDown();
    TEST_ASSERT(cb->IsDropDownOpen() == true, 2);
    cb->CloseDropDown();
    TEST_ASSERT(cb->IsDropDownOpen() == false, 3);
    delete au;
    D1("test_combobox_scrollbars passed");
    return 0;
}

// ------------------------------------------------------------------
// main
// ------------------------------------------------------------------
int main() {
    uint32_t delay_ms = 50;
    int32_t testsfailed = 0;
    AUI *au = AUI::Create("aui combobox test");
    UNUSED AWindow *w = au->MainWnd();

    auto handle = std::async(std::launch::async, [=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        au->ExitAUI();
    });

    testsfailed += test_combobox_attachment();
    testsfailed += test_combobox_add_remove();
    testsfailed += test_combobox_selection();
    testsfailed += test_combobox_editable();
    testsfailed += test_combobox_dropdown_toggle();
    testsfailed += test_combobox_select_from_dropdown();
    testsfailed += test_combobox_keyboard_navigation();
    testsfailed += test_combobox_styling();
    testsfailed += test_combobox_scrollbars();

    au->ProcessMessages();
    handle.get();
    delete au;
    au = nullptr;

    return testsfailed;
}
