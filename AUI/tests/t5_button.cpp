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
  TEST_ASSERT_EQ(btn->Text(), "some button", 4);
  TEST_ASSERT_EQ(btn->BGColor(), 0xFFCCCCCC, 5);
  TEST_ASSERT_EQ(btn->Border(), AUI_DEFAULT_BUTTON_BORDERW, 6);
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
  TEST_ASSERT_EQ(btn->Text(), "Click Me", 2);
  btn->Text("New Label");
  TEST_ASSERT_EQ(btn->Text(), "New Label", 3);
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
  btn->BGColor(0xFF8844CC);
  TEST_ASSERT_EQ(btn->BGColor(), 0xFF8844CC, 2);
  btn->TextColor(0xFFFF0000);
  TEST_ASSERT_EQ(btn->TextColor(), 0xFFFF0000, 3);
  btn->FontSize(20);
  TEST_ASSERT_EQ(btn->FontSize(), 20U, 4);
  btn->HAlign(AUIHAlign::right);
  TEST_ASSERT_EQ(btn->HAlign(), AUIHAlign::right, 5);
  btn->VAlign(AUIVAlign::bottom);
  TEST_ASSERT_EQ(btn->VAlign(), AUIVAlign::bottom, 6);
  btn->Border(3);
  TEST_ASSERT_EQ(btn->Border(), 3U, 7);
  btn->BorderColor(0xFF00FF00);
  TEST_ASSERT_EQ(btn->BorderColor(), 0xFF00FF00, 8);
  D1("test_button_properties passed");
  return 0;
}
//
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
  btn->SetMouseClickCallback([&callbackFired](AWidget* wi, void*, int32_t x, int32_t y) noexcept -> AWidget* {
    callbackFired = true;
    D1("Callback fired at ({},{})", x, y);
    return wi;
  },
  nullptr);
  w->OnMousePress(15, 15, BTN_LEFT);
  w->OnMouseRelease(15, 15, BTN_LEFT);
  TEST_ASSERT(callbackFired == true, 2);
  D1("test_button_click_callback passed");
  return 0;
}
//
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
  btn->SetMouseClickCallback([](AWidget* wi, void*, int32_t, int32_t) noexcept -> AWidget* {return wi;}, nullptr);
  AWidget* consumed = btn->MouseClick(15, 15);
  TEST_ASSERT_NE(consumed, nullptr, 2);
  consumed = btn->MouseClick(200, 200);
  TEST_ASSERT_EQ(consumed, nullptr, 3);
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
  btn->BGColor(0xFFCCCCCC);
  btn->TextColor(0xFF0000FF);
  btn->HAlign(AUIHAlign::center);
  btn->Border(2);
  btn->BorderColor(0xFFFFFFFF);
  w->RequestRedraw();
  D1("test_button_draw passed");
  return 0;
}

// ------------------------------------------------------------------
// AButton: text clipping bounds verification
// ------------------------------------------------------------------
int32_t test_clipping_bounds(AUI* au) {
  D1("test_button_clipping_bounds start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 2);
  // 1. Create a button constrained to a small width (50px wide)
  // with a string that extends beyond its boundary
  AButton* button = AButton::AttachTo(w, "VeryLongButtonLabelText");
  button->Move(10, 10);
  button->Resize(50, 30);
  button->TextColor(0xFF0000FFU); // Red text
  button->HAlign(AUIHAlign::left);
  // Render window buffer
  w->Draw();
  // Inspect 10px beyond the right boundary of the button (x = 10 + 50 = 60)
  // Pixels outside the scissor region MUST remain 0 (unmodified background)
  AWidgetReader<AButton> rRight(button, 80, 40);
  for (int32_t y = 10; y < 40; ++y) {
    for (int32_t x = 61; x < 75; ++x) {
      TEST_ASSERT_EQ(rRight.Pixel(x, y), 0U, 3); // Must not bleed past right edge
    }
  }
  // 2. Test Right-Aligned text inside the small button
  // Ensures pen position shifted left does not bleed past button->mX (x = 10)
  button->HAlign(AUIHAlign::right);
  w->Draw();
  // Inspect pixels to the left of the button boundary (x = 0 to 9)
  AWidgetReader<AButton> rLeft(button, 80, 40);
  for (int32_t y = 10; y < 40; ++y) {
    for (int32_t x = 0; x < 10; ++x) {
      TEST_ASSERT_EQ(rLeft.Pixel(x, y), 0U, 4); // Must not bleed past left edge
    }
  }
  D1("test_button_clipping_bounds passed");
  return 0;
}

// ------------------------------------------------------------------
// Main: run all tests with timed test harness
// ------------------------------------------------------------------
int main() {
  int32_t testsfailed = 0;
  testsfailed += runTimedTest(test_button_attachment, 1);
  testsfailed += runTimedTest(test_button_text, 1);
  testsfailed += runTimedTest(test_button_properties, 1);
  testsfailed += runTimedTest(test_button_click_callback, 1);
  testsfailed += runTimedTest(test_button_click_consumption, 1);
  testsfailed += runTimedTest(test_button_draw, 1);
  testsfailed += runTimedTest(test_clipping_bounds, 1);

  testsfailed += runTimedTest(test_button_attachment, 200);
  testsfailed += runTimedTest(test_button_text, 200);
  testsfailed += runTimedTest(test_button_properties, 200);
  testsfailed += runTimedTest(test_button_click_callback, 200);
  testsfailed += runTimedTest(test_button_click_consumption, 200);
  testsfailed += runTimedTest(test_button_draw, 200);
  testsfailed += runTimedTest(test_clipping_bounds, 200);

  D("test suite complete");
  return testsfailed;
}
