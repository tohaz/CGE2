#include "AUILib.h"

using namespace aui;

// Helper: simulate a key press on a widget
UNUSED static void send_key(AInputBox* box, AUIKeyCode code) {
  AUIKeyEvent ev;
  ev.pressed = true;
  ev.code = code;
  ev.unicode = 0;
  ev.modifiers = AUIModifier::None;
  box->OnKeyEvent(ev);
}

UNUSED static void send_char(AInputBox* box, char ch) {
  AUIKeyEvent ev;
  ev.pressed = true;
  ev.code = AUIKeyCode::None;
  ev.unicode = static_cast<uint32_t>(ch);
  ev.modifiers = AUIModifier::None;
  box->OnKeyEvent(ev);
}

// ------------------------------------------------------------------
// Attachment and default values
// ------------------------------------------------------------------
int32_t test_input_attachment(AUI* au) {
  D1("test_input_attachment start");
  AWindow* w = au->MainWnd();
  AInputBox* input = AInputBox::AttachTo(w);
  TEST_ASSERT_NE(input, nullptr, 2);
  TEST_ASSERT_EQ(input->AWidget::Text(), std::string(""), 3);
  TEST_ASSERT_EQ(input->CursorPos(), 0U, 4);
  TEST_ASSERT_EQ(input->IsEditable(), true, 5);
  TEST_ASSERT_EQ(input->Enabled(), true, 6);
  TEST_ASSERT_EQ(input->MaxLength(), 255U, 7);
  D1("test_input_attachment passed");
  return 0;
}
// ------------------------------------------------------------------
// Basic text operations (insert, backspace, delete, clear)
// ------------------------------------------------------------------
int32_t test_input_basic_ops(AUI* au) {
  D1("test_input_basic_ops start");
  AWindow* w = au->MainWnd();
  AInputBox* input = AInputBox::AttachTo(w);
  // Insert character
  input->Text("Hello");
  input->CursorPos(2);
  send_char(input, 'x');
  TEST_ASSERT_EQ(input->AWidget::Text(), "Hexllo", 2);
  TEST_ASSERT_EQ(input->CursorPos(), 3U, 3);
  // Backspace deletes character before cursor → should remove 'x'
  send_key(input, AUIKeyCode::Backspace);
  TEST_ASSERT_EQ(input->AWidget::Text(), "Hello", 4);
  TEST_ASSERT_EQ(input->CursorPos(), 2U, 5);
  // Delete forward (Delete key) - first set cursor to 1 (after 'H')
  input->CursorPos(1);
  send_key(input, AUIKeyCode::Delete);
  TEST_ASSERT_EQ(input->AWidget::Text(), "Hllo", 6);
  TEST_ASSERT_EQ(input->CursorPos(), 1U, 7);
  // Clear
  input->Text("");
  TEST_ASSERT_EQ(input->AWidget::Text(), std::string(""), 8);
  D1("test_input_basic_ops passed");
  return 0;
}
// ------------------------------------------------------------------
// Max length enforcement
// ------------------------------------------------------------------
int32_t test_input_max_length(AUI* au) {
  D1("test_input_max_length start");
  AWindow* w = au->MainWnd();
  AInputBox* input = AInputBox::AttachTo(w);
  input->MaxLength(5);
  input->Text("123456"); // should be truncated to 5
  TEST_ASSERT_EQ(input->Text(), "12345", 2);
  input->Text("abc");
  input->CursorPos(3);
  send_char(input, 'd');
  send_char(input, 'e');
  send_char(input, 'f'); // would exceed limit
  TEST_ASSERT_EQ(input->Text(), "abcde", 3);
  D1("test_input_max_length passed");
  return 0;
}
// ------------------------------------------------------------------
// Input filter (regex)
// ------------------------------------------------------------------
int32_t test_input_filter(AUI* au) {
  D1("test_input_filter start");
  AWindow* w = au->MainWnd();
  AInputBox* input = AInputBox::AttachTo(w);
  // Allow only digits
  input->InputFilter(R"(^[0-9]*$)");
  input->Text("123");
  input->CursorPos(3);
  TEST_ASSERT_EQ(input->Text(), "123", 2);
  send_char(input, 'a'); // should be rejected
  TEST_ASSERT_EQ(input->Text(), "123", 3);
  send_char(input, '4'); // now inserted at end
  TEST_ASSERT_EQ(input->Text(), "1234", 4);
  // Clear filter
  input->ClearInputFilter();
  input->CursorPos(4);
  send_char(input, 'b');
  TEST_ASSERT_EQ(input->Text(), "1234b", 5);
  D1("test_input_filter passed");
  return 0;
}
// ------------------------------------------------------------------
// Cursor movement
// ------------------------------------------------------------------
int32_t test_input_cursor(AUI* au) {
  D1("test_input_cursor start");
  AWindow* w = au->MainWnd();
  AInputBox* input = AInputBox::AttachTo(w);
  input->Text("ABCDE");
  input->CursorPos(3);
  send_key(input, AUIKeyCode::Left);
  TEST_ASSERT_EQ(input->CursorPos(), 2U, 2);
  send_key(input, AUIKeyCode::Right);
  TEST_ASSERT_EQ(input->CursorPos(), 3U, 3);
  send_key(input, AUIKeyCode::Home);
  TEST_ASSERT_EQ(input->CursorPos(), 0U, 4);
  send_key(input, AUIKeyCode::End);
  TEST_ASSERT_EQ(input->CursorPos(), 5U, 5);
  D1("test_input_cursor passed");
  return 0;
}
// ------------------------------------------------------------------
// Insert/Overwrite mode
// ------------------------------------------------------------------
int32_t test_insert_mode(AUI* au) {
  D1("test_insert_mode start");
  AWindow* w = au->MainWnd();
  AInputBox* input = AInputBox::AttachTo(w);
  // Insert mode (default)
  input->Text("12345");
  input->CursorPos(2);
  send_char(input, 'X');
  TEST_ASSERT_EQ(input->Text(), "12X345", 2);
  // Overwrite mode
  input->Text("12345");
  input->CursorPos(2);
  input->InsertMode(false);
  send_char(input, 'Y');
  TEST_ASSERT_EQ(input->Text(), "12Y45", 3);
  D1("test_insert_mode passed");
  return 0;
}
// ------------------------------------------------------------------
// Editable state (can still navigate but not modify)
// ------------------------------------------------------------------
int32_t test_editable(AUI* au) {
  D1("test_editable start");
  AWindow* w = au->MainWnd();
  AInputBox* input = AInputBox::AttachTo(w);
  input->Text("Hello");
  input->Editable(false);
  input->CursorPos(2);
  send_char(input, 'x'); // should be ignored
  TEST_ASSERT_EQ(input->Text(), "Hello", 2);
  send_key(input, AUIKeyCode::Backspace); // ignored
  TEST_ASSERT_EQ(input->Text(), "Hello", 3);
  // Navigation still works
  send_key(input, AUIKeyCode::Left);
  TEST_ASSERT_EQ(input->CursorPos(), 1U, 4);
  D1("test_editable passed");
  return 0;
}
// ------------------------------------------------------------------
// Enabled state (disabled = no interaction at all)
// ------------------------------------------------------------------
int32_t test_enabled(AUI* au) {
  D1("test_enabled start");
  AWindow* w = au->MainWnd();
  AInputBox* input = AInputBox::AttachTo(w);
  input->Text("Enabled");
  input->Disable();
  input->CursorPos(2); // should have no effect
  TEST_ASSERT_EQ(input->CursorPos(), 0U, 2);
  send_char(input, 'a'); // ignored
  TEST_ASSERT_EQ(input->AWidget::Text(), "Enabled", 3);
  input->Enable();
  input->CursorPos(1);
  send_char(input, 'X');
  TEST_ASSERT_EQ(input->Text(), "EXnabled", 4);
  D1("test_enabled passed");
  return 0;
}
// ------------------------------------------------------------------
// Callbacks (onChange and onSubmit)
// ------------------------------------------------------------------
int32_t test_callbacks(AUI* au) {
  D1("test_callbacks start");
  AWindow* w = au->MainWnd();
  AInputBox* input = AInputBox::AttachTo(w);
  std::string lastChanged;
  std::string lastSubmitted;
  input->SetOnChangeCallback([&](AInputBox*, const std::string& val) {
    lastChanged = val;
  });
  input->SetOnSubmitCallback([&](AInputBox*, const std::string& val) {
    lastSubmitted = val;
  });
  input->Text("abc");
  input->CursorPos(3);
  TEST_ASSERT_EQ(lastChanged, "abc", 2);
  send_char(input, 'd');
  TEST_ASSERT_EQ(lastChanged, "abcd", 3);
  // Submit (Enter key)
  send_key(input, AUIKeyCode::Enter);
  TEST_ASSERT_EQ(lastSubmitted, "abcd", 4);
  D1("test_callbacks passed");
  return 0;
}
// ------------------------------------------------------------------
// Text alignment (cursor position - just ensure no crash)
// ------------------------------------------------------------------
int32_t test_alignment(AUI* au) {
  D1("test_alignment start");
  AWindow* w = au->MainWnd();
  AInputBox* input = AInputBox::AttachTo(w);
  input->Text("Hi");
  input->Resize(200, 30);
  input->Move(0, 0);
  input->HAlign(AUIHAlign::right);
  (void)input->CursorX(); // just ensure no crash
  D1("test_alignment passed");
  return 0;
}
// ------------------------------------------------------------------
// Mouse click
// ------------------------------------------------------------------
int32_t test_mouse_click(AUI* au) {
  D1("test_mouse_click start");
  AWindow* w = au->MainWnd();
  AInputBox* input = AInputBox::AttachTo(w);
  input->Text("ABCD");
  input->Resize(200, 30);
  input->Move(0, 0);
  // Simulate click at approximate position of third character (index 2)
  int32_t clickX = 30; // rough estimate
  input->MouseClick(clickX, 15);
  size_t pos = input->CursorPos();
  TEST_ASSERT(pos <= 4ULL, 2); // cursor must be within 0..4
  D1("test_mouse_click passed");
  return 0;
}
// ------------------------------------------------------------------
// Backspace edge cases (beginning, empty, multiple, overwrite, with constraints)
// ------------------------------------------------------------------
int32_t test_backspace_edge_cases(AUI* au) {
  D1("test_backspace_edge_cases start");
  AWindow* w = au->MainWnd();
  AInputBox* input = AInputBox::AttachTo(w);
  // Case 1: Backspace at beginning of string (should do nothing)
  input->Text("abc");
  input->CursorPos(0);
  send_key(input, AUIKeyCode::Backspace);
  TEST_ASSERT_EQ(input->AWidget::Text(), "abc", 2);
  TEST_ASSERT_EQ(input->CursorPos(), 0U, 3);
  // Case 2: Backspace on empty string (should do nothing, no crash)
  input->Text("");
  input->CursorPos(0);
  send_key(input, AUIKeyCode::Backspace);
  TEST_ASSERT_EQ(input->AWidget::Text(), std::string(""), 4);
  TEST_ASSERT_EQ(input->CursorPos(), 0U, 5);
  // Case 3: Multiple backspaces delete all characters
  input->Text("xyz");
  input->CursorPos(3);
  send_key(input, AUIKeyCode::Backspace);
  TEST_ASSERT_EQ(input->AWidget::Text(), "xy", 6);
  send_key(input, AUIKeyCode::Backspace);
  TEST_ASSERT_EQ(input->AWidget::Text(), "x", 7);
  send_key(input, AUIKeyCode::Backspace);
  TEST_ASSERT_EQ(input->AWidget::Text(), std::string(""), 8);
  TEST_ASSERT_EQ(input->CursorPos(), 0U, 9);
  // Case 4: Backspace in overwrite mode
  input->Text("12345");
  input->InsertMode(false);
  input->CursorPos(3);
  send_key(input, AUIKeyCode::Backspace);
  TEST_ASSERT_EQ(input->AWidget::AWidget::Text(), "1245", 10);
  TEST_ASSERT_EQ(input->CursorPos(), 2U, 11);
  // Case 5: Backspace respects max length and input filter
  input->MaxLength(5);
  input->InputFilter("[0-9]*");
  input->Text("12345");
  input->CursorPos(5);
  send_key(input, AUIKeyCode::Backspace);
  TEST_ASSERT_EQ(input->AWidget::Text(), "1234", 12);
  TEST_ASSERT_EQ(input->CursorPos(), 4U, 13);
  D1("test_backspace_edge_cases passed");
  return 0;
}
// ------------------------------------------------------------------
// Insert/Overwrite modes (duplicate of test_insert_mode, but we keep it)
// ------------------------------------------------------------------
int32_t test_insert_overwrite_modes(AUI* au) {
  D1("test_insert_overwrite_modes start");
  AWindow* w = au->MainWnd();
  AInputBox* input = AInputBox::AttachTo(w);
  input->Text("12345");
  input->CursorPos(2);
  // Insert mode (default)
  send_char(input, 'X');
  TEST_ASSERT_EQ(input->Text(), "12X345", 1);
  // Overwrite mode
  input->Text("12345");
  input->CursorPos(2);
  input->InsertMode(false);
  send_char(input, 'Y');
  TEST_ASSERT_EQ(input->Text(), "12Y45", 2);
  D1("test_insert_overwrite_modes passed");
  return 0;
}

int main() {
  int32_t testsfailed = 0;

  testsfailed += runTimedTest(test_input_attachment, 1);
  testsfailed += runTimedTest(test_input_basic_ops, 1);
  testsfailed += runTimedTest(test_input_max_length, 1);
  testsfailed += runTimedTest(test_input_filter, 1);
  testsfailed += runTimedTest(test_input_cursor, 1);
  testsfailed += runTimedTest(test_insert_mode, 1);
  testsfailed += runTimedTest(test_editable, 1);
  testsfailed += runTimedTest(test_enabled, 1);
  testsfailed += runTimedTest(test_callbacks, 1);
  testsfailed += runTimedTest(test_alignment, 1);
  testsfailed += runTimedTest(test_mouse_click, 1);
  testsfailed += runTimedTest(test_backspace_edge_cases, 1);
  testsfailed += runTimedTest(test_insert_overwrite_modes, 1);

  testsfailed += runTimedTest(test_input_attachment, 200);
  testsfailed += runTimedTest(test_input_basic_ops, 200);
  testsfailed += runTimedTest(test_input_max_length, 200);
  testsfailed += runTimedTest(test_input_filter, 200);
  testsfailed += runTimedTest(test_input_cursor, 200);
  testsfailed += runTimedTest(test_insert_mode, 200);
  testsfailed += runTimedTest(test_editable, 200);
  testsfailed += runTimedTest(test_enabled, 200);
  testsfailed += runTimedTest(test_callbacks, 200);
  testsfailed += runTimedTest(test_alignment, 200);
  testsfailed += runTimedTest(test_mouse_click, 200);
  testsfailed += runTimedTest(test_backspace_edge_cases, 200);
  testsfailed += runTimedTest(test_insert_overwrite_modes, 200);

  D("test suite complete");
  return testsfailed;
}

