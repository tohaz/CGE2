#include "AUILib.h"

using namespace aui;

// ------------------------------------------------------------------
// Basic attachment and default values
// ------------------------------------------------------------------
int32_t test_button_attachment(AUI* au) {
  D1("test_button_attachment start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 2);
  AButton* btn = AButton::AttachTo(w);
  TEST_ASSERT_NE(btn, nullptr, 3);
  TEST_ASSERT_EQ(btn->GetText(), "Button", 4);
  TEST_ASSERT_EQ(btn->GetBGColor(), 0xFFCCCCCC, 5);
  TEST_ASSERT_EQ(btn->GetBorderThickness(), 2U, 6);
  D1("test_button_attachment passed");
  return 0;
}

// ------------------------------------------------------------------
// Button with custom text
// ------------------------------------------------------------------
int32_t test_button_text(AUI* au) {
  D1("test_button_text start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AButton* btn = AButton::AttachTo(w, "Click Me");
  TEST_ASSERT_EQ(btn->GetText(), "Click Me", 2);
  btn->SetText("New Label");
  TEST_ASSERT_EQ(btn->GetText(), "New Label", 3);
  D1("test_button_text passed");
  return 0;
}

// ------------------------------------------------------------------
// Button properties: colors, alignment, border
// ------------------------------------------------------------------
int32_t test_button_properties(AUI* au) {
  D1("test_button_properties start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AButton* btn = AButton::AttachTo(w);
  btn->SetBGColor(0xFF8844CC);
  TEST_ASSERT_EQ(btn->GetBGColor(), 0xFF8844CC, 2);
  btn->SetTextColor(0xFFFF0000);
  TEST_ASSERT_EQ(btn->GetTextColor(), 0xFFFF0000, 3);
  btn->SetFontSize(20);
  TEST_ASSERT_EQ(btn->GetFontSize(), 20U, 4);
  btn->SetHAlignment(AUIHAlign::right);
  TEST_ASSERT_EQ(btn->GetHAlignment(), AUIHAlign::right, 5);
  btn->SetVAlignment(AUIVAlign::bottom);
  TEST_ASSERT_EQ(btn->GetVAlignment(), AUIVAlign::bottom, 6);
  btn->SetBorderThickness(3);
  TEST_ASSERT_EQ(btn->GetBorderThickness(), 3U, 7);
  btn->SetBorderColor(0xFF00FF00);
  TEST_ASSERT_EQ(btn->GetBorderColor(), 0xFF00FF00, 8);
  D1("test_button_properties passed");
  return 0;
}

// ------------------------------------------------------------------
// Click callback
// ------------------------------------------------------------------
int32_t test_button_click_callback(AUI* au) {
  D1("test_button_click_callback start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AButton* btn = AButton::AttachTo(w, "Press Me");
  btn->Move(10, 10);
  btn->Resize(100, 30);
  bool callbackFired = false;
  btn->SetClickCallback(
      [&callbackFired](AWindow*, AWidget*, void*, int32_t x, int32_t y, bool pressed) {
        if (pressed) {
          callbackFired = true;
          D1("Callback fired at ({},{})", x, y);
        }
      },
      nullptr);
  w->OnMousePress(15, 15, 1);
  TEST_ASSERT(callbackFired == true, 2);
  D1("test_button_click_callback passed");
  return 0;
}

// ------------------------------------------------------------------
// Click consumption (button should consume clicks when callback is set)
// ------------------------------------------------------------------
int32_t test_button_click_consumption(AUI* au) {
  D1("test_button_click_consumption start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AButton* btn = AButton::AttachTo(w, "Consume");
  btn->Move(10, 10);
  btn->Resize(100, 30);
  btn->SetClickCallback([](AWindow*, AWidget*, void*, int32_t, int32_t, bool) {}, nullptr);
  bool consumed = btn->DispatchClick(15, 15, true);
  TEST_ASSERT(consumed == true, 2);
  consumed = btn->DispatchClick(200, 200, true);
  TEST_ASSERT(consumed == false, 3);
  D1("test_button_click_consumption passed");
  return 0;
}

// ------------------------------------------------------------------
// Drawing does not crash
// ------------------------------------------------------------------
int32_t test_button_draw(AUI* au) {
  D1("test_button_draw start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AButton* btn = AButton::AttachTo(w, "Draw Test");
  btn->Move(10, 10);
  btn->Resize(150, 30);
  btn->SetBGColor(0xFFCCCCCC);
  btn->SetTextColor(0xFF0000FF);
  btn->SetHAlignment(AUIHAlign::center);
  btn->SetBorderThickness(2);
  btn->SetBorderColor(0xFFFFFFFF);
  w->Draw();
  D1("test_button_draw passed");
  return 0;
}

// ------------------------------------------------------------------
// Main: run all tests with timed test harness
// ------------------------------------------------------------------
int main() {
  int32_t testsfailed = 0;
  testsfailed += runTimedTest("test_button_attachment", test_button_attachment, 1);
  testsfailed += runTimedTest("test_button_text", test_button_text, 1);
  testsfailed += runTimedTest("test_button_properties", test_button_properties, 1);
  testsfailed += runTimedTest("test_button_click_callback", test_button_click_callback, 1);
  testsfailed += runTimedTest("test_button_click_consumption", test_button_click_consumption, 1);
  testsfailed += runTimedTest("test_button_draw", test_button_draw, 1);
 
  testsfailed += runTimedTest("test_button_attachment", test_button_attachment, 200);
  testsfailed += runTimedTest("test_button_text", test_button_text, 200);
  testsfailed += runTimedTest("test_button_properties", test_button_properties, 200);
  testsfailed += runTimedTest("test_button_click_callback", test_button_click_callback, 200);
  testsfailed += runTimedTest("test_button_click_consumption", test_button_click_consumption, 200);
  testsfailed += runTimedTest("test_button_draw", test_button_draw, 200);


 
  D("test suite complete");
  return testsfailed;
}