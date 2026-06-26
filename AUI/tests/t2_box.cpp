
#include "AUILib.h"

using namespace aui;

// Refactored test: widget attachment and basic operations
int32_t test_widget_attachment(AUI *au) {
  D1("test_widget_attachment start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 2);
  ABox* box = ABox::AttachTo(w);
  TEST_ASSERT_NE(box, nullptr, 3);
  box->Move(50, 60);
  box->Resize(200, 150);
  box->SetBGColor(0xFF8844CC);
  w->Draw();
// Optional: au->Draw() if needed
  D1("test_widget_attachment passed");
  return 0;
}

// Refactored test: click callback
int32_t test_click_callback(AUI *au) {
  D1("test_click_callback start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 2);
  ABox* box = ABox::AttachTo(w);
  box->Move(10, 10);
  box->Resize(100, 100);
  bool callbackFired = false;
  box->SetClickCallback([&callbackFired](AWindow*, AWidget*, void*, int32_t x, int32_t y, bool pressed) {
    if(pressed) {
      callbackFired = true;
      D1("Callback fired at ({},{})", x, y);
    }
  },
  nullptr);

// Simulate mouse press at (50,50) inside the box
  w->OnMousePress(50, 50, 1);// button 1 = left
  TEST_ASSERT_EQ(callbackFired, true, 3);
  D1("test_click_callback passed");
  return 0;
}

// Refactored test: mouse move callback
int32_t test_mousemove_callback(AUI *au) {
  D1("test_mousemove_callback start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 2);
  ABox* box = ABox::AttachTo(w);
  box->Move(10, 10);
  box->Resize(60, 60);
  bool moveFired = false;
  box->SetMouseMoveCallback([&moveFired](AWindow*, AWidget*, void*, int32_t x, int32_t y) {
    moveFired = true;
    D1("Move callback at ({},{})", x, y);
  },
  nullptr);
// Simulate mouse move at (50,50)
  w->OnMouseMove(50, 50);
  TEST_ASSERT_EQ(moveFired, true, 3);
  D1("test_mousemove_callback passed");
  return 0;
}

int main() {
  //UNUSED char *qqq = new char[1]; // generate error
  int32_t testsfailed = 0;

  testsfailed += runTimedTest("test_widget_attachment", test_widget_attachment, 1);
  testsfailed += runTimedTest("test_click_callback", test_click_callback, 1);
  testsfailed += runTimedTest("test_mousemove_callback", test_mousemove_callback, 1);

  testsfailed += runTimedTest("test_widget_attachment", test_widget_attachment, 200);
  testsfailed += runTimedTest("test_click_callback", test_click_callback, 200);
  testsfailed += runTimedTest("test_mousemove_callback", test_mousemove_callback, 200);

  D("test suite complete");
  return testsfailed;
}


