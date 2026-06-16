#include <chrono>
#include <future>
#include <thread>
#include "AUILib.h"
#include "AButton.h"

using namespace aui;

#define TEST_ASSERT(cond, errcode) do { if(!(cond)) { E("Test failed: {}", #cond); return errcode; } } while(0)

// ------------------------------------------------------------------
// Basic attachment and default values
// ------------------------------------------------------------------
int32_t test_button_attachment() {
  D1("test_button_attachment start");
  AUI* au = AUI::Create("ButtonAttachTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT(w != nullptr, 2);
  AButton* btn = AButton::AttachTo(w);
  TEST_ASSERT(btn != nullptr, 3);
  TEST_ASSERT(btn->GetText() == "Button", 4);
  TEST_ASSERT(btn->GetBGColor() == 0xFFCCCCCC, 5);
  TEST_ASSERT(btn->GetBorderThickness() == 2U, 6);
  delete au;
  D1("test_button_attachment passed");
  return 0;
}

// ------------------------------------------------------------------
// Button with custom text
// ------------------------------------------------------------------
int32_t test_button_text() {
  D1("test_button_text start");
  AUI* au = AUI::Create("ButtonTextTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  AButton* btn = AButton::AttachTo(w, "Click Me");
  TEST_ASSERT(btn->GetText() == "Click Me", 2);
  btn->SetText("New Label");
  TEST_ASSERT(btn->GetText() == "New Label", 3);
  delete au;
  D1("test_button_text passed");
  return 0;
}

// ------------------------------------------------------------------
// Button properties: colors, alignment, border
// ------------------------------------------------------------------
int32_t test_button_properties() {
  D1("test_button_properties start");
  AUI* au = AUI::Create("ButtonPropTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  AButton* btn = AButton::AttachTo(w);
  btn->SetBGColor(0xFF8844CC);
  TEST_ASSERT(btn->GetBGColor() == 0xFF8844CC, 2);
  btn->SetTextColor(0xFFFF0000);
  TEST_ASSERT(btn->GetTextColor() == 0xFFFF0000, 3);
  btn->SetFontSize(20);
  TEST_ASSERT(btn->GetFontSize() == 20U, 4);
  btn->SetHAlignment(AUIHAlign::right);
  TEST_ASSERT(btn->GetHAlignment() == AUIHAlign::right, 5);
  btn->SetVAlignment(AUIVAlign::bottom);
  TEST_ASSERT(btn->GetVAlignment() == AUIVAlign::bottom, 6);
  btn->SetBorderThickness(3);
  TEST_ASSERT(btn->GetBorderThickness() == 3U, 7);
  btn->SetBorderColor(0xFF00FF00);
  TEST_ASSERT(btn->GetBorderColor() == 0xFF00FF00, 8);
  delete au;
  D1("test_button_properties passed");
  return 0;
}

// ------------------------------------------------------------------
// Click callback
// ------------------------------------------------------------------
int32_t test_button_click_callback() {
  D1("test_button_click_callback start");
  AUI* au = AUI::Create("ButtonCallbackTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  AButton* btn = AButton::AttachTo(w, "Press Me");
  btn->Move(10, 10);
  btn->Resize(100, 30);
  bool callbackFired = false;
  btn->SetClickCallback(
      [&callbackFired](AWindow*, AWidget*, void*, int32_t x, int32_t y, bool pressed) {
        if(pressed) {
          callbackFired = true;
          D1("Callback fired at ({},{})", x, y);
        }
      },
      nullptr);
  w->OnMousePress(15, 15, 1);
  TEST_ASSERT(callbackFired == true, 2);
  delete au;
  D1("test_button_click_callback passed");
  return 0;
}

// ------------------------------------------------------------------
// Hover and click highlight tests are NOT included because they require
// visual inspection or access to internal mHovered/mPressed flags.
// These are already covered by the drawing functionality and can be
// manually verified. The tests below only check non‑visual behaviour.
// ------------------------------------------------------------------

// ------------------------------------------------------------------
// Click consumption (button should consume clicks when callback is set)
// ------------------------------------------------------------------
int32_t test_button_click_consumption() {
  D1("test_button_click_consumption start");
  AUI* au = AUI::Create("ButtonConsumeTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  AButton* btn = AButton::AttachTo(w, "Consume");
  btn->Move(10, 10);
  btn->Resize(100, 30);
  // Set a dummy callback so that the button consumes the click
  btn->SetClickCallback([](AWindow*, AWidget*, void*, int32_t, int32_t, bool) {}, nullptr);
  bool consumed = btn->DispatchClick(15, 15, true);
  TEST_ASSERT(consumed == true, 2);
  consumed = btn->DispatchClick(200, 200, true);
  TEST_ASSERT(consumed == false, 3);
  delete au;
  D1("test_button_click_consumption passed");
  return 0;
}

// ------------------------------------------------------------------
// Drawing does not crash
// ------------------------------------------------------------------
int32_t test_button_draw() {
  D1("test_button_draw start");
  AUI* au = AUI::Create("ButtonDrawTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  AButton* btn = AButton::AttachTo(w, "Draw Test");
  btn->Move(10, 10);
  btn->Resize(150, 30);
  btn->SetBGColor(0xFFCCCCCC);
  btn->SetTextColor(0xFF0000FF);
  btn->SetHAlignment(AUIHAlign::center);
  btn->SetBorderThickness(2);
  btn->SetBorderColor(0xFFFFFFFF);
  w->Draw();   // should not crash
  delete au;
  D1("test_button_draw passed");
  return 0;
}

// ------------------------------------------------------------------
// Main: run all tests with async exit
// ------------------------------------------------------------------
int main() {
  UINT32 delay_ms = 50;
  INT32 testsfailed = 0;
  AUI* au = AUI::Create("aui test");
  UNUSED AWindow* w = au->MainWnd();

  auto handle = std::async(std::launch::async, [=]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    au->ExitAUI();
  });

  testsfailed += test_button_attachment();
  testsfailed += test_button_text();
  testsfailed += test_button_properties();
  testsfailed += test_button_click_callback();
  testsfailed += test_button_click_consumption();
  testsfailed += test_button_draw();

  au->ProcessMessages();
  handle.get();
  delete au;
  au = nullptr;

  return testsfailed;
}

