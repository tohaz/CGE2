#include "AUILib.h"

using namespace aui;

int32_t test_aui_lifecycle(UNUSED AUI *au) {
  D1("test_aui_lifecycle start");
  TEST_ASSERT_NE(au, nullptr, 1);
  D1("test_aui_lifecycle passed");
  return 0;
}

int32_t test_aui_lifecycle2(AUI *au) {
  D1("test_aui_lifecycle2 start, {}", (uint64_t)au);
  TEST_ASSERT_NE(au, nullptr, 1);
  UNUSED AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(au->MainWnd(), nullptr, 2);
  D1("test_aui_lifecycle2 passed");
  return 0;
}

int32_t test_aui_lifecycle3(AUI *au) {
  D1("test_aui_lifecycle3 start");
  TEST_ASSERT_NE(au, nullptr, 1);
  UNUSED AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(au->MainWnd(), nullptr, 2);
//  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  w->EnableResize();
  D1("test_aui_lifecycle3 passed");
  return 0;
}

int32_t test_aui_lifecycle4(AUI *au) {
  D1("test_aui_lifecycle4 start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(au->MainWnd(), nullptr, 2);
  w->EnableResize();
  std::this_thread::sleep_for(std::chrono::milliseconds(220));
  w->Resize(800, 600);
  D1("test_aui_lifecycle4 passed");
  return 0;
}

int32_t test_aui_lifecycle5(AUI *au) {
  D1("test_aui_lifecycle5 start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(au->MainWnd(), nullptr, 2);
  w->EnableResize();
  w->Resize(10, 10);
  D1("test_aui_lifecycle5 passed");
  return 0;
}

int32_t test_aui_lifecycle6(AUI *au) {
  D1("test_aui_lifecycle6 start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(au->MainWnd(), nullptr, 2);
  w->EnableResize();
  w->Resize(800, 600);
  w->Resize(10000, 10);
  D1("test_aui_lifecycle6 passed");
  return 0;
}

int32_t test_aui_lifecycle7(AUI *au) {
  D1("test_aui_lifecycle7 start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(au->MainWnd(), nullptr, 2);
  w->EnableResize();
  w->Resize(1, 1);
  w->Resize(300, 500);
  w->DisableResize();
  D1("test_aui_lifecycle7 passed");
  return 0;
}

int32_t test_aui_lifecycle8(AUI *au) {
  D1("test_aui_lifecycle8 start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(au->MainWnd(), nullptr, 2);
  w->EnableResize();
  w->Resize(1, 1);
  w->Resize(6000, 500);
  w->DisableResize();
  w->EnableResize();
  w->Resize(500, 10);
  w->Resize(10, 5000);
  w->DisableResize();
  D1("test_aui_lifecycle8 passed");
  return 0;
}

int32_t test_aui_lifecycle9(AUI *au) {
  D1("test_aui_lifecycle9 start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(au->MainWnd(), nullptr, 2);
  w->EnableResize();
  w->Resize(800, 600);
  w->DisableResize();
  ABox* b = ABox::AttachTo(w);
  b->Move(10, 10);
  b->Resize(100, 150);
  b->SetBGColor(0xFF8844CC);
  b->SetBorderThickness(5);
  D1("test_aui_lifecycle9 passed");
  return 0;
}

int32_t test_window_properties(AUI *au) {
  D1("test_window_properties start");
  AWindow* w = au->MainWnd();
  // Default size should be 500x300 (from AttachTo)
  TEST_ASSERT_EQ(w->SizeX(), (uint32_t)500, 3);
  TEST_ASSERT_EQ(w->SizeY(), (uint32_t)300, 4);
  w->EnableResize();
  TEST_ASSERT_EQ(w->IsResizeEnabled(), true, 5);
  w->Resize(800, 600);
  TEST_ASSERT_EQ(w->SizeX(), (uint32_t)800, 6);
  TEST_ASSERT_EQ(w->SizeY(), (uint32_t)600, 7);
  w->DisableResize();
  TEST_ASSERT_EQ(w->IsResizeEnabled(), false, 8);
  w->SetTitle("New Title");
  D1("test_window_properties passed");
  return 0;
}

int32_t test_backend_detection(AUI *au) {
  D1("test_backend_detection start");
  AUIWindowType type = au->GetWindowType();
  const char* waylandEnv = getenv("WAYLAND_DISPLAY");
  if(waylandEnv) {
    TEST_ASSERT(type == AUIWindowType::Wayland, 2);
  } else {
    TEST_ASSERT(type == AUIWindowType::XCB, 2);
  }
  D1("test_backend_detection passed");
  return 0;
}

int32_t test_multiple_windows(AUI *au) {
  D1("test_multiple_windows start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w1 = au->MainWnd();
  TEST_ASSERT_NE(w1, nullptr, 2);
  // Create a second window (type same as backend)
  AWindow* w2 = AWindow::AttachTo(au, "Second", au->GetWindowType());
  TEST_ASSERT_NE(w2, nullptr, 3);
  // Both should be registered
  UNUSED uint64_t id1 = w1->GetNativeId();
  UNUSED uint64_t id2 = w2->GetNativeId();
  TEST_ASSERT_NE(id1, id2, 4);
  D1("test_multiple_windows passed");
  return 0;
}

int32_t test_multiple_windows2(UNUSED AUI *au) {
  D1("test_multiple_windows2 start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w1 = au->MainWnd();
  TEST_ASSERT_NE(w1, nullptr, 2);
  AWindow* w2 = AWindow::AttachTo(au, "Second", au->GetWindowType());
  TEST_ASSERT_NE(w2, nullptr, 3);
  UNUSED uint64_t id1 = w1->GetNativeId();
  uint64_t id2 = w2->GetNativeId();
  TEST_ASSERT_NE(id1, id2, 4);
  // Close sec0ond window; should unregister
  w2->Close();
  // Now try to find by native id (should be null)
  AWindow* found = au->FindWindowByNativeId(id2);
  TEST_ASSERT_EQ(found, nullptr, 5);
  D1("test_multiple_windows2 passed");
  return 0;
}

int32_t test_draw_command_queue(AUI *au) {
  D1("test_draw_command_queue start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  ABox* box = ABox::AttachTo(w);
  box->Move(0, 0);
  box->Resize(100, 100);
  // Draw should enqueue a command for the window (Wayland or XCB)
  w->Draw();
  au->Draw();
  size_t sz = au->DrawCommands();
  D1("command_queue size {}", sz);
  TEST_ASSERT_EQ(sz, (size_t)0, 1);
  D1("test_draw_command_queue passed");
  return 0;
}

int32_t test_window_operations(AUI *au) {
  D1("test_window_operations start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow *mainWin = au->MainWnd();
  TEST_ASSERT_NE(mainWin, nullptr, 2);
  mainWin->EnableResize();
  mainWin->Resize(200, 200);
  AUIWindowType type = au->GetWindowType();
// 2. Create additional windows (if Wayland, create both)
  AWindow *ww = nullptr;
  AWindow *wx = nullptr;
  if(type == AUIWindowType::Wayland) {
    ww = AWindow::AttachTo(au, "Additional Wayland", AUIWindowType::Wayland);
    wx = AWindow::AttachTo(au, "Additional X11", AUIWindowType::XCB);
    TEST_ASSERT_NE(ww, nullptr, 3);
    TEST_ASSERT_NE(wx, nullptr, 4);
    ww->EnableResize();
    ww->Resize(300, 300);
    wx->Move(200, 10);// XCB move
    wx->EnableResize();
    wx->Resize(250, 250);
  }
  else {
// XCB-only: create one extra XCB window
    wx = AWindow::AttachTo(au, "Additional X11", AUIWindowType::XCB);
    TEST_ASSERT_NE(wx, nullptr, 5);
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
  size_t sz = au->DrawCommands();
  D1("command_queue size {}", sz);
  TEST_ASSERT_EQ(sz, (size_t)0, 1);
  D1("test_window_operations passed");
  return 0;
}

int main() {
  //UNUSED char *qqq = new char[1]; // generate error
  int32_t testsfailed = 0;

  testsfailed += runTimedTest("test_aui_lifecycle", test_aui_lifecycle, 2);
  testsfailed += runTimedTest("test_aui_lifecycle2", test_aui_lifecycle2, 2);
  testsfailed += runTimedTest("test_aui_lifecycle3", test_aui_lifecycle3, 2);
  testsfailed += runTimedTest("test_aui_lifecycle4", test_aui_lifecycle4, 2);
  testsfailed += runTimedTest("test_aui_lifecycle5", test_aui_lifecycle5, 2);
  testsfailed += runTimedTest("test_aui_lifecycle6", test_aui_lifecycle6, 2);
  testsfailed += runTimedTest("test_aui_lifecycle7", test_aui_lifecycle7, 2);
  testsfailed += runTimedTest("test_aui_lifecycle8", test_aui_lifecycle8, 2);
  testsfailed += runTimedTest("test_aui_lifecycle9", test_aui_lifecycle9, 2);
  testsfailed += runTimedTest("test_window_properties", test_window_properties, 2);
  testsfailed += runTimedTest("test_backend_detection", test_backend_detection, 2);
  testsfailed += runTimedTest("test_multiple_windows", test_multiple_windows, 2);
  testsfailed += runTimedTest("test_multiple_windows2", test_multiple_windows2, 2);
  testsfailed += runTimedTest("test_draw_command_queue", test_draw_command_queue, 2);
  testsfailed += runTimedTest("test_window_operations", test_window_operations, 2);

  testsfailed += runTimedTest("2test_aui_lifecycle", test_aui_lifecycle, 200);
  testsfailed += runTimedTest("2test_aui_lifecycle2", test_aui_lifecycle2, 200);
  testsfailed += runTimedTest("2test_aui_lifecycle3", test_aui_lifecycle3, 200);
  testsfailed += runTimedTest("2test_aui_lifecycle4", test_aui_lifecycle4, 200);
  testsfailed += runTimedTest("2test_aui_lifecycle5", test_aui_lifecycle5, 200);
  testsfailed += runTimedTest("2test_aui_lifecycle6", test_aui_lifecycle6, 200);
  testsfailed += runTimedTest("2test_aui_lifecycle7", test_aui_lifecycle7, 200);
  testsfailed += runTimedTest("2test_aui_lifecycle8", test_aui_lifecycle8, 200);
  testsfailed += runTimedTest("2test_aui_lifecycle9", test_aui_lifecycle9, 200);
  testsfailed += runTimedTest("2test_window_properties", test_window_properties, 200);
  testsfailed += runTimedTest("2test_backend_detection", test_backend_detection, 200);
  testsfailed += runTimedTest("2test_multiple_windows", test_multiple_windows, 200);
  testsfailed += runTimedTest("2test_multiple_windows2", test_multiple_windows2, 200);
  testsfailed += runTimedTest("2test_draw_command_queue", test_draw_command_queue, 200);
  testsfailed += runTimedTest("2test_window_operations", test_window_operations, 200);



  D("test suite complete")

  return testsfailed;
}



