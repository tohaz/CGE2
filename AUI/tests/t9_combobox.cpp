#include "AUILib.h"

using namespace aui;

// ------------------------------------------------------------------
// Attachment and basic properties
// ------------------------------------------------------------------
int32_t test_combobox_attachment(AUI* au) {
  D1("test_combobox_attachment start");
  AWindow* win = au->MainWnd();
  AComboBox* cb = AComboBox::AttachTo(win);
  TEST_ASSERT_NE(cb, nullptr, 2);
  TEST_ASSERT_EQ(cb->GetItemCount(), 0U, 3);
  TEST_ASSERT_EQ(cb->GetSelectedIndex(), -1, 4);
  TEST_ASSERT_EQ(cb->IsEditable(), true, 5);
  TEST_ASSERT_EQ(cb->IsDropDownOpen(), false, 6);
  D1("test_combobox_attachment passed");
  return 0;
}

// ------------------------------------------------------------------
// Adding and removing items
// ------------------------------------------------------------------
int32_t test_combobox_add_remove(AUI* au) {
  D1("test_combobox_add_remove start");
  AWindow* win = au->MainWnd();
  AComboBox* cb = AComboBox::AttachTo(win);
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
  D1("test_combobox_add_remove passed");
  return 0;
}

// ------------------------------------------------------------------
// Selection
// ------------------------------------------------------------------
int32_t test_combobox_selection(AUI* au) {
  D1("test_combobox_selection start");
  AWindow* win = au->MainWnd();
  AComboBox* cb = AComboBox::AttachTo(win);
  cb->AddItem("A");
  cb->AddItem("B");
  cb->AddItem("C");
  cb->SetSelectedIndex(1);
  TEST_ASSERT_EQ(cb->GetSelectedIndex(), 1, 2);
  TEST_ASSERT_EQ(cb->GetSelectedText(), std::string("B"), 3);
  cb->ClearSelection();
  TEST_ASSERT_EQ(cb->GetSelectedIndex(), -1, 4);
  TEST_ASSERT_EQ(cb->GetSelectedText(), std::string(""), 5);
  D1("test_combobox_selection passed");
  return 0;
}

// ------------------------------------------------------------------
// Editable state
// ------------------------------------------------------------------
int32_t test_combobox_editable(AUI* au) {
  D1("test_combobox_editable start");
  AWindow* win = au->MainWnd();
  AComboBox* cb = AComboBox::AttachTo(win);
  cb->SetEditable(false);
  TEST_ASSERT_EQ(cb->IsEditable(), false, 2);
  AInputBox* input = cb->GetInputBox();
  TEST_ASSERT_EQ(input->IsEditable(), false, 3);
  cb->SetEditable(true);
  TEST_ASSERT_EQ(cb->IsEditable(), true, 4);
  TEST_ASSERT_EQ(input->IsEditable(), true, 5);
  D1("test_combobox_editable passed");
  return 0;
}

// ------------------------------------------------------------------
// Dropdown open/close (direct API)
// ------------------------------------------------------------------
int32_t test_combobox_dropdown_toggle(AUI* au) {
  D1("test_combobox_dropdown_toggle start");
  AWindow* win = au->MainWnd();
  AComboBox* cb = AComboBox::AttachTo(win);
  cb->Move(50, 50);
  cb->Resize(200, 28);
  cb->AddItem("One");
  cb->AddItem("Two");
  cb->AddItem("Three");
  // Open via API
  cb->OpenDropDown();
  TEST_ASSERT_EQ(cb->IsDropDownOpen(), true, 2);
  cb->CloseDropDown();
  TEST_ASSERT_EQ(cb->IsDropDownOpen(), false, 3);
  // Toggle via API
  cb->ToggleDropDown();
  TEST_ASSERT_EQ(cb->IsDropDownOpen(), true, 4);
  cb->ToggleDropDown();
  TEST_ASSERT_EQ(cb->IsDropDownOpen(), false, 5);
  // Mouse simulation on button (should toggle)
  bool handled = cb->OnMouseClick(190, 14, true);
  TEST_ASSERT(handled, 6);
  handled = cb->OnMouseClick(190, 14, false);
  TEST_ASSERT_EQ(cb->IsDropDownOpen(), true, 7);
  // Click on input area (localX=50) – should open on release
  handled = cb->OnMouseClick(50, 14, true);
  TEST_ASSERT(handled, 8);
  handled = cb->OnMouseClick(50, 14, false);
  TEST_ASSERT_EQ(cb->IsDropDownOpen(), true, 9);
  cb->CloseDropDown();
  TEST_ASSERT_EQ(cb->IsDropDownOpen(), false, 10);
  D1("test_combobox_dropdown_toggle passed");
  return 0;
}

// ------------------------------------------------------------------
// Selection via dropdown (keyboard navigation after programmatic open)
// ------------------------------------------------------------------
int32_t test_combobox_select_from_dropdown(AUI* au) {
  D1("test_combobox_select_from_dropdown start");
  AWindow* win = au->MainWnd();
  AComboBox* cb = AComboBox::AttachTo(win);
  cb->Move(50, 50);
  cb->Resize(200, 28);
  cb->AddItem("Apple");
  cb->AddItem("Banana");
  cb->AddItem("Cherry");
  // Open dropdown programmatically
  cb->OpenDropDown();
  TEST_ASSERT_EQ(cb->IsDropDownOpen(), true, 2);
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
  TEST_ASSERT_EQ(cb->IsDropDownOpen(), false, 6);
  TEST_ASSERT_EQ(cb->GetSelectedIndex(), 2, 7);
  TEST_ASSERT_EQ(cb->GetSelectedText(), std::string("Cherry"), 8);
  D1("test_combobox_select_from_dropdown passed");
  return 0;
}

// ------------------------------------------------------------------
// Keyboard navigation (with programmatic open)
// ------------------------------------------------------------------
int32_t test_combobox_keyboard_navigation(AUI* au) {
  D1("test_combobox_keyboard_navigation start");
  AWindow* win = au->MainWnd();
  AComboBox* cb = AComboBox::AttachTo(win);
  cb->Move(50, 50);
  cb->AddItem("Item1");
  cb->AddItem("Item2");
  cb->AddItem("Item3");
  cb->AddItem("Item4");
  cb->AddItem("Item5");
  // Open dropdown programmatically
  cb->OpenDropDown();
  TEST_ASSERT_EQ(cb->IsDropDownOpen(), true, 2);
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
  TEST_ASSERT_EQ(cb->IsDropDownOpen(), false, 6);
  D1("test_combobox_keyboard_navigation passed");
  return 0;
}

// ------------------------------------------------------------------
// Styling setters
// ------------------------------------------------------------------
int32_t test_combobox_styling(AUI* au) {
  D1("test_combobox_styling start");
  AWindow* win = au->MainWnd();
  AComboBox* cb = AComboBox::AttachTo(win);
  cb->SetInputBoxBGColor(0xFF0000FF);
  cb->SetButtonBGColor(0xFFFF0000);
  cb->SetListBGColor(0xFF00FF00);
  cb->SetInputBoxTextColor(0xFF00FFFF);
  cb->SetButtonTextColor(0xFF00FF00);
  cb->SetListTextColor(0xFFFF00FF);
  cb->SetFontSize(16);
  TEST_ASSERT_EQ(cb->GetFontSize(), 16U, 2);
  // We can't easily verify colors, but we can ensure no crash.
  D1("test_combobox_styling passed");
  return 0;
}

// ------------------------------------------------------------------
// Scrollbars visibility (when many items)
// ------------------------------------------------------------------
int32_t test_combobox_scrollbars(AUI* au) {
  D1("test_combobox_scrollbars start");
  AWindow* win = au->MainWnd();
  AComboBox* cb = AComboBox::AttachTo(win);
  cb->Move(50, 50);
  cb->Resize(150, 28);
  for (int i = 0; i < 50; ++i) {
    cb->AddItem("Item " + std::to_string(i));
  }
  cb->OpenDropDown();
  TEST_ASSERT_EQ(cb->IsDropDownOpen(), true, 2);
  cb->CloseDropDown();
  TEST_ASSERT_EQ(cb->IsDropDownOpen(), false, 3);
  D1("test_combobox_scrollbars passed");
  return 0;
}

// ------------------------------------------------------------------
// main
// ------------------------------------------------------------------
int main() {
  int32_t testsfailed = 0;
  testsfailed += runTimedTest("test_combobox_attachment", test_combobox_attachment, 1);
  testsfailed += runTimedTest("test_combobox_add_remove", test_combobox_add_remove, 1);
  testsfailed += runTimedTest("test_combobox_selection", test_combobox_selection, 1);
  testsfailed += runTimedTest("test_combobox_editable", test_combobox_editable, 1);
  testsfailed += runTimedTest("test_combobox_dropdown_toggle", test_combobox_dropdown_toggle, 1);
  testsfailed += runTimedTest("test_combobox_select_from_dropdown", test_combobox_select_from_dropdown, 1);
  testsfailed += runTimedTest("test_combobox_keyboard_navigation", test_combobox_keyboard_navigation, 1);
  testsfailed += runTimedTest("test_combobox_styling", test_combobox_styling, 1);
  testsfailed += runTimedTest("test_combobox_scrollbars", test_combobox_scrollbars, 1);

  testsfailed += runTimedTest("test_combobox_attachment", test_combobox_attachment, 200);
  testsfailed += runTimedTest("test_combobox_add_remove", test_combobox_add_remove, 200);
  testsfailed += runTimedTest("test_combobox_selection", test_combobox_selection, 200);
  testsfailed += runTimedTest("test_combobox_editable", test_combobox_editable, 200);
  testsfailed += runTimedTest("test_combobox_dropdown_toggle", test_combobox_dropdown_toggle, 200);
  testsfailed += runTimedTest("test_combobox_select_from_dropdown", test_combobox_select_from_dropdown, 200);
  testsfailed += runTimedTest("test_combobox_keyboard_navigation", test_combobox_keyboard_navigation, 200);
  testsfailed += runTimedTest("test_combobox_styling", test_combobox_styling, 200);
  testsfailed += runTimedTest("test_combobox_scrollbars", test_combobox_scrollbars, 200);

  D("test suite complete");
  return testsfailed;
}