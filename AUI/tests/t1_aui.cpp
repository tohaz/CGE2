#include <chrono>
#include <future>
#include <thread>
#include <memory.h>

#include "AUILib.h"

using namespace aui;

//#define TEST_ASSERT(cond, errcode) do { if(!(cond)) { E("Test failed: {}", #cond); return errcode; } } while(0)

int32_t test_aui_lifecycle() {
  D1("test_aui_lifecycle start");
  AUI* au = AUI::Create("TestWindow");
  TEST_ASSERT(au != nullptr, 1);
  TEST_ASSERT(au->MainWnd() != nullptr, 2);
  delete au;
  D1("test_aui_lifecycle passed");
  return 0;
}

int32_t test_aui_lifecycle2() {
  D1("test_aui_lifecycle start2");
  AUI* au = AUI::Create("TestWindow");
  delete au;
  au = AUI::Create("TestWindow");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  delete au;
  au = AUI::Create("TestWindow");
  w = au->MainWnd();
  w->EnableResize();
  w->Resize(800, 600);
  delete au;
  au = AUI::Create("TestWindow");
  w = au->MainWnd();
  w->EnableResize();
  w->Resize(800, 600);
  w->DisableResize();
  delete au;
  au = AUI::Create("TestWindow");
  w = au->MainWnd();
  w->EnableResize();
  w->Resize(800, 600);
  w->DisableResize();
  w->EnableResize();
  w->Resize(500, 300);
  w->DisableResize();
  ABox* b = ABox::AttachTo(w);
  b->Move(50, 50);
  b->Resize(200, 150);
  b->SetBGColor(0xFF8844CC);
  b->SetBorderThickness(5);
  delete au;
  D1("test_aui_lifecycle2 passed");
  return 0;
}


int32_t test_window_properties() {
  D1("test_window_properties start");
  AUI* au = AUI::Create("PropTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT(w != nullptr, 2);
  // Default size should be 500x300 (from AttachTo)
  TEST_ASSERT(w->SizeX() == 500, 3);
  TEST_ASSERT(w->SizeY() == 300, 4);
  w->EnableResize();
  TEST_ASSERT(w->IsResizeEnabled() == true, 5);
  w->Resize(800, 600);
  TEST_ASSERT(w->SizeX() == 800, 6);
  TEST_ASSERT(w->SizeY() == 600, 7);
  w->DisableResize();
  TEST_ASSERT(w->IsResizeEnabled() == false, 8);
  w->SetTitle("New Title");
  // No direct getter, but we trust SetTitle didn't crash
  delete au;
  D1("test_window_properties passed");
  return 0;
}

int32_t test_backend_detection() {
  D1("test_backend_detection start");
  AUI* au = AUI::Create("BackendTest");
  TEST_ASSERT(au != nullptr, 1);
  AUIWindowType type = au->GetWindowType();
  const char* waylandEnv = getenv("WAYLAND_DISPLAY");
  if(waylandEnv) {
    TEST_ASSERT(type == AUIWindowType::Wayland, 2);
  } else {
    TEST_ASSERT(type == AUIWindowType::XCB, 2);
  }
  delete au;
  D1("test_backend_detection passed");
  return 0;
}

int32_t test_multiple_windows() {
  D1("test_multiple_windows start");
  AUI* au = AUI::Create("MultiTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w1 = au->MainWnd();
  TEST_ASSERT(w1 != nullptr, 2);
  // Create a second window (type same as backend)
  AWindow* w2 = AWindow::AttachTo(au, "Second", au->GetWindowType());
  TEST_ASSERT(w2 != nullptr, 3);
  // Both should be registered
  uint64_t id1 = w1->GetNativeId();
  uint64_t id2 = w2->GetNativeId();
  TEST_ASSERT(id1 != id2, 4);
  // Close second window; should unregister
  w2->Close();
  // Now try to find by native id (should be null)
  AWindow* found = au->FindWindowByNativeId(id2);
  TEST_ASSERT(found == nullptr, 5);
  delete au;
  D1("test_multiple_windows passed");
  return 0;
}

int32_t test_draw_command_queue() {
  D1("test_draw_command_queue start");
  AUI* au = AUI::Create("QueueTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  ABox* box = ABox::AttachTo(w);
  box->Move(0, 0);
  box->Resize(100, 100);
  // Draw should enqueue a command for the window (Wayland or XCB)
  w->Draw();
  // We cannot inspect the command queue directly (private), but we can call Draw again
  // and ensure no crash.
  au->Draw();
  delete au;
  D1("test_draw_command_queue passed");
  return 0;
}

int32_t test_window_operations() {
  D1("test_window_operations start");
// 1. Create main window
  AUI *au = AUI::Create("MainWindow");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *mainWin = au->MainWnd();
  TEST_ASSERT(mainWin != nullptr, 2);
  mainWin->EnableResize();
  mainWin->Resize(200, 200);
  AUIWindowType type = au->GetWindowType();
// 2. Create additional windows (if Wayland, create both)
  AWindow *ww = nullptr;
  AWindow *wx = nullptr;
  if(type == AUIWindowType::Wayland) {
    ww = AWindow::AttachTo(au, "Additional Wayland", AUIWindowType::Wayland);
    wx = AWindow::AttachTo(au, "Additional X11", AUIWindowType::XCB);
    TEST_ASSERT(ww != nullptr, 3);
    TEST_ASSERT(wx != nullptr, 4);
    ww->EnableResize();
    ww->Resize(300, 300);
    wx->Move(200, 10);// XCB move
    wx->EnableResize();
    wx->Resize(250, 250);
  }
  else {
// XCB-only: create one extra XCB window
    wx = AWindow::AttachTo(au, "Additional X11", AUIWindowType::XCB);
    TEST_ASSERT(wx != nullptr, 5);
    wx->EnableResize();
    wx->Resize(300, 300);
    wx->Move(200, 10);
  }
  if(ww) {
    ww->Close();
  }
  if(wx) {
    wx->Close();
  }
  au->ExitAUI();
  delete au;
  D1("test_window_operations passed");
  return 0;
}


int main() {
	//UNUSED char *qqq = new char[1]; // generate error
  UINT32 delay_ms = 50; // delay before thead calls window to close
  INT32 testsfailed = 0;
  AUI* au = AUI::Create("aui test");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(800, 600);
  w->DisableResize();
  w->EnableResize();
  w->Resize(500, 300);
  w->DisableResize();
  ABox* b = ABox::AttachTo(w);
  b->Move(50, 50);
  b->Resize(200, 150);
  b->SetBGColor(0xFF8844CC);
  b->SetBorderThickness(5);

  auto handle = std::async(std::launch::async, [=]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
  	au->ExitAUI();
  });
  
  testsfailed += test_aui_lifecycle();
  testsfailed += test_aui_lifecycle2();
  testsfailed += test_window_properties();
  testsfailed += test_backend_detection();
  testsfailed += test_multiple_windows();
  testsfailed += test_draw_command_queue();
  testsfailed += test_window_operations();
  
  au->ProcessMessages();
  
  handle.get();

  delete au;
  au = nullptr;

  return testsfailed;
}


