#include "AUILib.h"

using namespace aui;

// Helper: simulate a key press on a widget
static void send_key(AInputBox* box, AUIKeyCode code) {
  AUIKeyEvent ev;
  ev.pressed = true;
  ev.code = code;
  ev.unicode = 0;
  ev.modifiers = AUIModifier::None;
  box->OnKeyEvent(ev);
}

static void send_char(AInputBox* box, char ch) {
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
  TEST_ASSERT_EQ(input->GetText(), std::string(""), 3);
  TEST_ASSERT_EQ(input->GetCursorPos(), 0U, 4);
  TEST_ASSERT_EQ(input->IsEditable(), true, 5);
  TEST_ASSERT_EQ(input->IsEnabled(), true, 6);
  TEST_ASSERT_EQ(input->GetMaxLength(), 255U, 7);
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
  input->SetText("Hello");
  input->SetCursorPos(2);
  send_char(input, 'x');
  TEST_ASSERT_EQ(input->GetText(), "Hexllo", 2);
  TEST_ASSERT_EQ(input->GetCursorPos(), 3U, 3);
  // Backspace deletes character before cursor → should remove 'x'
  send_key(input, AUIKeyCode::Backspace);
  TEST_ASSERT_EQ(input->GetText(), "Hello", 4);
  TEST_ASSERT_EQ(input->GetCursorPos(), 2U, 5);
  // Delete forward (Delete key) - first set cursor to 1 (after 'H')
  input->SetCursorPos(1);
  send_key(input, AUIKeyCode::Delete);
  TEST_ASSERT_EQ(input->GetText(), "Hllo", 6);
  TEST_ASSERT_EQ(input->GetCursorPos(), 1U, 7);
  // Clear
  input->SetText("");
  TEST_ASSERT_EQ(input->GetText(), std::string(""), 8);
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
  input->SetMaxLength(5);
  input->SetText("123456"); // should be truncated to 5
  TEST_ASSERT_EQ(input->GetText(), "12345", 2);
  input->SetText("abc");
  input->SetCursorPos(3);
  send_char(input, 'd');
  send_char(input, 'e');
  send_char(input, 'f'); // would exceed limit
  TEST_ASSERT_EQ(input->GetText(), "abcde", 3);
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
  input->SetInputFilter(R"(^[0-9]*$)");
  input->SetText("123");
  input->SetCursorPos(3);
  TEST_ASSERT_EQ(input->GetText(), "123", 2);
  send_char(input, 'a'); // should be rejected
  TEST_ASSERT_EQ(input->GetText(), "123", 3);
  send_char(input, '4'); // now inserted at end
  TEST_ASSERT_EQ(input->GetText(), "1234", 4);
  // Clear filter
  input->ClearInputFilter();
  input->SetCursorPos(4);
  send_char(input, 'b');
  TEST_ASSERT_EQ(input->GetText(), "1234b", 5);
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
  input->SetText("ABCDE");
  input->SetCursorPos(3);
  send_key(input, AUIKeyCode::Left);
  TEST_ASSERT_EQ(input->GetCursorPos(), 2U, 2);
  send_key(input, AUIKeyCode::Right);
  TEST_ASSERT_EQ(input->GetCursorPos(), 3U, 3);
  send_key(input, AUIKeyCode::Home);
  TEST_ASSERT_EQ(input->GetCursorPos(), 0U, 4);
  send_key(input, AUIKeyCode::End);
  TEST_ASSERT_EQ(input->GetCursorPos(), 5U, 5);
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
  input->SetText("12345");
  input->SetCursorPos(2);
  send_char(input, 'X');
  TEST_ASSERT_EQ(input->GetText(), "12X345", 2);
  // Overwrite mode
  input->SetText("12345");
  input->SetCursorPos(2);
  input->SetInsertMode(false);
  send_char(input, 'Y');
  TEST_ASSERT_EQ(input->GetText(), "12Y45", 3);
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
  input->SetText("Hello");
  input->SetEditable(false);
  input->SetCursorPos(2);
  send_char(input, 'x'); // should be ignored
  TEST_ASSERT_EQ(input->GetText(), "Hello", 2);
  send_key(input, AUIKeyCode::Backspace); // ignored
  TEST_ASSERT_EQ(input->GetText(), "Hello", 3);
  // Navigation still works
  send_key(input, AUIKeyCode::Left);
  TEST_ASSERT_EQ(input->GetCursorPos(), 1U, 4);
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
  input->SetText("Enabled");
  input->Disable();
  input->SetCursorPos(2); // should have no effect
  TEST_ASSERT_EQ(input->GetCursorPos(), 0U, 2);
  send_char(input, 'a'); // ignored
  TEST_ASSERT_EQ(input->GetText(), "Enabled", 3);
  input->Enable();
  input->SetCursorPos(1);
  send_char(input, 'X');
  TEST_ASSERT_EQ(input->GetText(), "EXnabled", 4);
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
  input->SetText("abc");
  input->SetCursorPos(3);
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
  input->SetText("Hi");
  input->Resize(200, 30);
  input->Move(0, 0);
  input->SetHAlignment(AUIHAlign::right);
  (void)input->GetCursorX(); // just ensure no crash
  TEST_ASSERT(true, 2);
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
  input->SetText("ABCD");
  input->Resize(200, 30);
  input->Move(0, 0);
  // Simulate click at approximate position of third character (index 2)
  int32_t clickX = 30; // rough estimate
  input->OnMouseClick(clickX, 15, true);
  size_t pos = input->GetCursorPos();
  TEST_ASSERT(pos <= 4, 2); // cursor must be within 0..4
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
  input->SetText("abc");
  input->SetCursorPos(0);
  send_key(input, AUIKeyCode::Backspace);
  TEST_ASSERT_EQ(input->GetText(), "abc", 2);
  TEST_ASSERT_EQ(input->GetCursorPos(), 0U, 3);
  // Case 2: Backspace on empty string (should do nothing, no crash)
  input->SetText("");
  input->SetCursorPos(0);
  send_key(input, AUIKeyCode::Backspace);
  TEST_ASSERT_EQ(input->GetText(), std::string(""), 4);
  TEST_ASSERT_EQ(input->GetCursorPos(), 0U, 5);
  // Case 3: Multiple backspaces delete all characters
  input->SetText("xyz");
  input->SetCursorPos(3);
  send_key(input, AUIKeyCode::Backspace);
  TEST_ASSERT_EQ(input->GetText(), "xy", 6);
  send_key(input, AUIKeyCode::Backspace);
  TEST_ASSERT_EQ(input->GetText(), "x", 7);
  send_key(input, AUIKeyCode::Backspace);
  TEST_ASSERT_EQ(input->GetText(), std::string(""), 8);
  TEST_ASSERT_EQ(input->GetCursorPos(), 0U, 9);
  // Case 4: Backspace in overwrite mode
  input->SetText("12345");
  input->SetInsertMode(false);
  input->SetCursorPos(3);
  send_key(input, AUIKeyCode::Backspace);
  TEST_ASSERT_EQ(input->GetText(), "1245", 10);
  TEST_ASSERT_EQ(input->GetCursorPos(), 2U, 11);
  // Case 5: Backspace respects max length and input filter
  input->SetMaxLength(5);
  input->SetInputFilter("[0-9]*");
  input->SetText("12345");
  input->SetCursorPos(5);
  send_key(input, AUIKeyCode::Backspace);
  TEST_ASSERT_EQ(input->GetText(), "1234", 12);
  TEST_ASSERT_EQ(input->GetCursorPos(), 4U, 13);
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
  input->SetText("12345");
  input->SetCursorPos(2);
  // Insert mode (default)
  send_char(input, 'X');
  TEST_ASSERT_EQ(input->GetText(), "12X345", 1);
  // Overwrite mode
  input->SetText("12345");
  input->SetCursorPos(2);
  input->SetInsertMode(false);
  send_char(input, 'Y');
  TEST_ASSERT_EQ(input->GetText(), "12Y45", 2);
  D1("test_insert_overwrite_modes passed");
  return 0;
}

// ------------------------------------------------------------------
// Main: run all tests with timed test harness
// ------------------------------------------------------------------
int main() {
  int32_t testsfailed = 0;

  testsfailed += runTimedTest("test_input_attachment", test_input_attachment, 1);
  testsfailed += runTimedTest("test_input_basic_ops", test_input_basic_ops, 1);
  testsfailed += runTimedTest("test_input_max_length", test_input_max_length, 1);
  testsfailed += runTimedTest("test_input_filter", test_input_filter, 1);
  testsfailed += runTimedTest("test_input_cursor", test_input_cursor, 1);
  testsfailed += runTimedTest("test_insert_mode", test_insert_mode, 1);
  testsfailed += runTimedTest("test_editable", test_editable, 1);
  testsfailed += runTimedTest("test_enabled", test_enabled, 1);
  testsfailed += runTimedTest("test_callbacks", test_callbacks, 1);
  testsfailed += runTimedTest("test_alignment", test_alignment, 1);
  testsfailed += runTimedTest("test_mouse_click", test_mouse_click, 1);
  testsfailed += runTimedTest("test_backspace_edge_cases", test_backspace_edge_cases, 1);
  testsfailed += runTimedTest("test_insert_overwrite_modes", test_insert_overwrite_modes, 1);

  testsfailed += runTimedTest("test_input_attachment", test_input_attachment, 200);
  testsfailed += runTimedTest("test_input_basic_ops", test_input_basic_ops, 200);
  testsfailed += runTimedTest("test_input_max_length", test_input_max_length, 200);
  testsfailed += runTimedTest("test_input_filter", test_input_filter, 200);
  testsfailed += runTimedTest("test_input_cursor", test_input_cursor, 200);
  testsfailed += runTimedTest("test_insert_mode", test_insert_mode, 200);
  testsfailed += runTimedTest("test_editable", test_editable, 200);
  testsfailed += runTimedTest("test_enabled", test_enabled, 200);
  testsfailed += runTimedTest("test_callbacks", test_callbacks, 200);
  testsfailed += runTimedTest("test_alignment", test_alignment, 200);
  testsfailed += runTimedTest("test_mouse_click", test_mouse_click, 200);
  testsfailed += runTimedTest("test_backspace_edge_cases", test_backspace_edge_cases, 200);
  testsfailed += runTimedTest("test_insert_overwrite_modes", test_insert_overwrite_modes, 200);
  
  D("test suite complete");
  return testsfailed;
}
