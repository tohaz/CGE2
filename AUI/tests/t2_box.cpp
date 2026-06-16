#include <chrono>
#include <future>
#include <thread>
#include <memory.h>

#include "AUILib.h"

using namespace aui;

#define TEST_ASSERT(cond, errcode) do { if(!(cond)) { E("Test failed: {}", #cond); return errcode; } } while(0)

int32_t test_widget_attachment() {
  D1("test_widget_attachment start");
  AUI* au = AUI::Create("WidgetTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT(w != nullptr, 2);
  ABox* box = ABox::AttachTo(w);
  TEST_ASSERT(box != nullptr, 3);
  box->Move(50, 60);
  box->Resize(200, 150);
  box->SetBGColor(0xFF8844CC);
  w->Draw();
  delete au;
  D1("test_widget_attachment passed");
  return 0;
}

int32_t test_click_callback() {
  D1("test_click_callback start");
  AUI* au = AUI::Create("CallbackTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT(w != nullptr, 2);
  ABox* box = ABox::AttachTo(w);
  box->Move(10, 10);
  box->Resize(100, 100);
  bool callbackFired = false;
  box->SetClickCallback(
      [&callbackFired](AWindow*, AWidget*, void*, int32_t x, int32_t y, bool pressed) {
        if(pressed) {
          callbackFired = true;
          D1("Callback fired at ({},{})", x, y);
        }
      },
      nullptr);
  // Simulate mouse press at (50,50) inside the box (10,10 to 110,110)
  // DispatchClick expects coordinates relative to window
  w->OnMousePress(50, 50, 1);   // button 1 = left
  TEST_ASSERT(callbackFired == true, 3);
  delete au;
  D1("test_click_callback passed");
  return 0;
}

int32_t test_mousemove_callback() {
  D1("test_mousemove_callback start");
  AUI* au = AUI::Create("MoveTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT(w != nullptr, 2);
  ABox* box = ABox::AttachTo(w);
  box->Move(10, 10);
  box->Resize(100, 100);
  bool moveFired = false;
  box->SetMouseMoveCallback(
      [&moveFired](AWindow*, AWidget*, void*, int32_t x, int32_t y) {
        moveFired = true;
        D1("Move callback at ({},{})", x, y);
      },
      nullptr);
  // Simulate mouse move at (50,50)
  w->OnMouseMove(50, 50);
  TEST_ASSERT(moveFired == true, 3);
  delete au;
  D1("test_mousemove_callback passed");
  return 0;
}


int main() {
	//UNUSED char *qqq = new char[1]; // generate error
  UINT32 delay_ms = 50; // delay before thead calls window to close
  INT32 testsfailed = 0;
  AUI* au = AUI::Create("aui test");
  UNUSED AWindow* w = au->MainWnd();

  auto handle = std::async(std::launch::async, [=]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
  	au->ExitAUI();
  });
  
  testsfailed += test_widget_attachment();
  testsfailed += test_click_callback();
  testsfailed += test_mousemove_callback();
  
  au->ProcessMessages();
  
  handle.get();

  delete au;
  au = nullptr;

  return testsfailed;
}


