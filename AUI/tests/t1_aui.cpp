#ifndef AUI_UNIT_TEST
#define AUI_UNIT_TEST
#endif

#include "AUILib.h"

using namespace aui;

int32_t test_aui_lifecycle(UNUSED AUI* au) {
  D1("test_aui_lifecycle start");
  TEST_ASSERT_NE(au, nullptr, 1);
  D1("test_aui_lifecycle passed");
  return 0;
}

int32_t test_aui_lifecycle2(AUI* au) {
  D1("test_aui_lifecycle2 start, {}", (uint64_t)au);
  TEST_ASSERT_NE(au, nullptr, 1);
  UNUSED AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(au->MainWnd(), nullptr, 2);
  D1("test_aui_lifecycle2 passed");
  return 0;
}
//
int32_t test_aui_lifecycle3(AUI* au) {
  D1("test_aui_lifecycle3 start");
  TEST_ASSERT_NE(au, nullptr, 1);
  UNUSED AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(au->MainWnd(), nullptr, 2);
//  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  w->EnableResize();
  D1("test_aui_lifecycle3 passed");
  return 0;
}

int32_t test_aui_lifecycle4(AUI* au) {
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

int32_t test_aui_lifecycle5(AUI* au) {
  D1("test_aui_lifecycle5 start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(au->MainWnd(), nullptr, 2);
  w->EnableResize();
  w->Resize(10, 10);
  D1("test_aui_lifecycle5 passed");
  return 0;
}

int32_t test_aui_lifecycle6(AUI* au) {
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

int32_t test_aui_lifecycle7(AUI* au) {
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

int32_t test_aui_lifecycle8(AUI* au) {
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

int32_t test_aui_lifecycle9(AUI* au) {
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
  b->BGColor(0xFF8844CC);
  b->Border(5);
  D1("test_aui_lifecycle9 passed");
  return 0;
}
//
int32_t test_window_properties(AUI* au) {
  D1("test_window_properties start");
  AWindow* w = au->MainWnd();
  // Default size should be 500x300 (from AttachTo)
  TEST_ASSERT_EQ(w->SizeX(), (uint32_t)AUI_DEFAULT_WINDOW_SZX, 3);
  TEST_ASSERT_EQ(w->SizeY(), (uint32_t)AUI_DEFAULT_WINDOW_SZY, 4);
  w->EnableResize();
  TEST_ASSERT_EQ(w->IsResizeEnabled(), true, 5);
  w->Resize(801, 602);
  TEST_ASSERT_EQ(w->SizeX(), (uint32_t)801, 6);
  TEST_ASSERT_EQ(w->SizeY(), (uint32_t)602, 7);
  w->DisableResize();
  TEST_ASSERT_EQ(w->IsResizeEnabled(), false, 8);
  w->Title("New Title");
  D1("test_window_properties passed");
  return 0;
}

int32_t test_multiple_windows(AUI* au) {
  D1("test_multiple_windows start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w1 = au->MainWnd();
  TEST_ASSERT_NE(w1, nullptr, 2);
  // Create a second window (type same as backend)
  AWindow* w2 = AWindow::AttachTo(au, "Second", au->MainBackendType());
  TEST_ASSERT_NE(w2, nullptr, 3);
  // Both should be registered
  UNUSED uint64_t id1 = w1->NativeWindowId();
  UNUSED uint64_t id2 = w2->NativeWindowId();
  TEST_ASSERT_NE(id1, id2, 4);
  TEST_ASSERT_NE(id1, 0, 4);
  TEST_ASSERT_NE(id2, 0, 4);
  D1("test_multiple_windows passed");
  return 0;
}

int32_t test_multiple_windows2(UNUSED AUI* au) {
  D1("test_multiple_windows2 start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w1 = au->MainWnd();
  TEST_ASSERT_NE(w1, nullptr, 2);
  AWindow* w2 = AWindow::AttachTo(au, "Second", au->MainBackendType());
  TEST_ASSERT_NE(w2, nullptr, 3);
  UNUSED uint64_t id1 = w1->NativeWindowId();
  uint64_t id2 = w2->NativeWindowId();
  TEST_ASSERT_NE(id1, id2, 4);
  // Close sec0ond window; should unregister
  w2->Close();
  // Now try to find by native id (should be null)
  AWindow* found = au->FindWindowByNativeId(id2, au->MainBackendType());
  TEST_ASSERT_EQ(found, nullptr, 5);
  D1("test_multiple_windows2 passed");
  return 0;
}

int32_t test_window_operations(AUI* au) {
  D1("test_window_operations start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow *mainWin = au->MainWnd();
  TEST_ASSERT_NE(mainWin, nullptr, 2);
  mainWin->EnableResize();
  mainWin->Resize(200, 200);
  AUIWindowType type = au->MainBackendType();
// 2. Create additional windows (if Wayland, create both)
  AWindow *ww = nullptr;
  AWindow *wx = nullptr;
  if(type == AUIWindowType::Wayland) {
    ww = AWindow::AttachTo(au, "Additional Wayland", AUIWindowType::Wayland);
    wx = AWindow::AttachTo(au, "Additional X11", AUIWindowType::X11);
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
    wx = AWindow::AttachTo(au, "Additional X11", AUIWindowType::X11);
    TEST_ASSERT_NE(wx, nullptr, 5);
    wx->EnableResize();
    wx->Resize(300, 300);
    wx->Move(200, 10);
  }
  // close one window hidden and one opened. test no crash
  if(ww)ww->Hide();
  if(ww)ww->Show();
  if(ww)ww->Hide();
  if(wx)wx->Hide();
  if(wx)wx->Show();
  if(ww) {
    ww->Close();
  }
  if(wx) {
    wx->Close();
  }
  D1("test_window_operations passed");
  return 0;
}

int32_t testNestedClick(AUI* au) {
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 1);
  w->EnableResize();
  w->Resize(400, 200);
  D1("test_nested_boxes start");
  TEST_ASSERT_NE(w, nullptr, 1);
  ABox* outer = ABox::AttachTo(w);
  outer->Move(10, 10);
  outer->Resize(300, 300);
  outer->BGColor(0xFFFF0000);
  ABox* middle = ABox::AttachTo(outer);
  middle->Move(50, 50);
  middle->Resize(200, 200);
  middle->BGColor(0xFF00FF00);
  ABox* inner = ABox::AttachTo(middle);
  inner->Move(80, 80);
  inner->Resize(100, 100);
  inner->BGColor(0xFF0000FF);
  bool innerFired = false;
  bool middleFired = false;
  bool outerFired = false;
  outer->SetMouseClickCallback([&](AWidget* wi, void*, int32_t , int32_t ) noexcept -> AWidget* {
    D2();
    outerFired = true;
    return wi;
  }, nullptr);
  middle->SetMouseClickCallback([&](AWidget* wi, void*, int32_t , int32_t ) noexcept -> AWidget* {
    D2();
    middleFired = true;
    return wi;
  }, nullptr);
  inner->SetMouseClickCallback([&](AWidget* wi, void*, int32_t , int32_t ) noexcept -> AWidget* {
    D2();
    innerFired = true;
    return wi;
  }, nullptr);
//// Click inside inner box: absolute coords = 10+50+80 = 140
  w->OnMousePress(11, 11, BTN_LEFT);
  w->OnMouseRelease(11, 11, BTN_LEFT);
  w->OnMousePress(140, 140, BTN_LEFT);
  w->OnMouseRelease(140, 140, BTN_LEFT);
  D2("states outerFired {:s} middleFired {:s} innerFired {:s}", outerFired, middleFired, innerFired)
  TEST_ASSERT_EQ(innerFired, true, 2);
  TEST_ASSERT_EQ(middleFired, false, 3);
  TEST_ASSERT_EQ(outerFired, true, 4);
  D2("states ok")
  D1("test_nested_boxes passed");
  return 0;
}
int32_t test_mouse_wheel_propagation(UNUSED AUI *au) {
  D("test_mouse_wheel_propagation not implemented because atable missing yet");
  AWindow* win = au->MainWnd();
  win->EnableResize();
  win->Resize(400, 300);
  ABox* box = ABox::AttachTo(win);
  box->Move(10, 10);
  box->Resize(380, 280);
  D("ATable disabled in this test")
  ATable* table = ATable::AttachTo(box);
  table->Move(0, 0);
  table->Resize(380, 280);
  table->AddRows(50);
  table->AddColumns(10);
  table->ScrollbarsToggle(true);
  table->LayoutUpdate();// ensure scrollbars appear
  int64_t initialV = table->VOffset();
// Simulate mouse wheel over the table (coordinates relative to window)
// The table is at (10,10) inside box at (10,10) -> absolute (20,20)
  win->OnMouseMove(20, 20);// set cursor position (so wheel knows where)
  win->OnMouseWheel(20, 20, -3);// scroll down 3 steps
  int64_t newV = table->VOffset();
  TEST_ASSERT(newV > initialV, 2);
// Scroll up
  win->OnMouseWheel(20, 20, 1);
  TEST_ASSERT(table->VOffset() < newV, 3);
  D1("test_mouse_wheel_propagation passed");
  return 0;
}

int32_t test_gui_edge_cases(AUI* au) {
  D1("test_gui_edge_cases start");
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 1);
  w->EnableResize();
  w->Resize(800, 600);
  // =========================================================================
  // EDGE CASE 1: Drag-Release Outside Bounds (Capture Verification)
  // =========================================================================
  {
    ABox* btn = ABox::AttachTo(w);
    btn->Move(10, 10);
    btn->Resize(100, 100);
    bool releaseFired = false;
    bool clickFired = false;
    btn->SetMouseReleaseLeftCallback([&](AWidget*, void*, int32_t, int32_t) noexcept -> AWidget* {
      releaseFired = true;
      return nullptr;
    }, nullptr);
    btn->SetMouseClickCallback([&](AWidget*, void*, int32_t, int32_t) noexcept -> AWidget* {
      clickFired = true;
      return nullptr;
    }, nullptr);
    // Press inside (10..110)
    w->OnMousePress(50, 50, BTN_LEFT);
    TEST_ASSERT_EQ(btn->MousePressedLeft(), true, 101);
    // Release far outside (500, 500)
    w->OnMouseRelease(500, 500, BTN_LEFT);
    TEST_ASSERT_EQ(btn->MousePressedLeft(), false, 102);
    TEST_ASSERT_EQ(releaseFired, true, 103); // Release callback SHOULD fire for cleanup
    TEST_ASSERT_EQ(clickFired, false, 104);   // Click callback MUST NOT fire outside
  }
  // =========================================================================
  // EDGE CASE 2: Disabled Parent Blocking Child Events
  // =========================================================================
  {
    w->Clear();
    ABox* parent = ABox::AttachTo(w);
    parent->Move(200, 10);
    parent->Resize(200, 200);
    ABox* child = ABox::AttachTo(parent);
    child->Move(10, 10);
    child->Resize(100, 100);
    bool childFired = false;
    child->SetMouseClickCallback([&](AWidget*, void*, int32_t, int32_t) noexcept -> AWidget* {
      childFired = true;
      return nullptr;
    }, nullptr);
    // Disable parent container
    parent->Enabled(false);
    // Click inside child absolute coords: (200 + 10 + 5) = 215
    w->OnMousePress(215, 25, BTN_LEFT);
    w->OnMouseRelease(215, 25, BTN_LEFT);
    TEST_ASSERT_EQ(childFired, false, 201); // Disabled parent must block event completely
    // Re-enable and verify
    parent->Enabled(true);
    w->OnMousePress(215, 25, BTN_LEFT);
    w->OnMouseRelease(215, 25, BTN_LEFT);
    TEST_ASSERT_EQ(childFired, true, 202);
  }
  // =========================================================================
  // EDGE CASE 3: Z-Order Overlap & Top-Most Interception
  // =========================================================================
  {
    w->Clear();
    ABox* boxA = ABox::AttachTo(w); // Bottom sibling
    boxA->Move(10, 200);
    boxA->Resize(100, 100);
    ABox* boxB = ABox::AttachTo(w); // Top sibling (attached second)
    boxB->Move(10, 200);
    boxB->Resize(100, 100);
    bool boxAFired = false;
    bool boxBFired = false;
    boxA->SetMouseClickCallback([&](AWidget*, void*, int32_t, int32_t) noexcept -> AWidget* {
      boxAFired = true;
      return nullptr;
    }, nullptr);
    boxB->SetMouseClickCallback([&](AWidget*, void*, int32_t, int32_t) noexcept -> AWidget* {
      boxBFired = true;
      return nullptr;
    }, nullptr);
    w->OnMousePress(50, 250, BTN_LEFT);
    w->OnMouseRelease(50, 250, BTN_LEFT);
    TEST_ASSERT_EQ(boxBFired, true, 301);  // Top widget must handle
    TEST_ASSERT_EQ(boxAFired, false, 302); // Bottom widget must be occluded
  }
  // =========================================================================
  // EDGE CASE 4: Non-Consuming Overlay Pass-Through
  // =========================================================================
  {
    w->Clear();
    ABox* buttonUnder = ABox::AttachTo(w);
    buttonUnder->Move(200, 200);
    buttonUnder->Resize(100, 100);
    ALabel* overlayOnTop = ALabel::AttachTo(w);
    overlayOnTop->Move(200, 200);
    overlayOnTop->Resize(100, 100);
    overlayOnTop->ConsumeMouseEvents(false); // Enable click pass-through
    bool buttonFired = false;
    buttonUnder->SetMouseClickCallback([&](AWidget*, void*, int32_t, int32_t) noexcept -> AWidget* {
      buttonFired = true;
      return nullptr;
    }, nullptr);
    w->OnMousePress(250, 250, BTN_LEFT);
    w->OnMouseRelease(250, 250, BTN_LEFT);
    TEST_ASSERT_EQ(buttonFired, true, 401); // Event must pass through overlay
  }
  // =========================================================================
  // EDGE CASE 5: Deep Hierarchy Local Coordinate Transformation
  // =========================================================================
  {
    w->Clear();
    ABox* panel = ABox::AttachTo(w);     // Pos: (400, 10)
    panel->Move(400, 10);
    panel->Resize(300, 300);
    ABox* group = ABox::AttachTo(panel);  // Rel Pos: (50, 50) -> Abs: (450, 60)
    group->Move(50, 50);
    group->Resize(200, 200);
    ABox* target = ABox::AttachTo(group); // Rel Pos: (30, 30) -> Abs: (480, 90)
    target->Move(30, 30);
    target->Resize(50, 50);
    int32_t receivedX = -1;
    int32_t receivedY = -1;
    target->SetMouseClickCallback([&](AWidget*, void*, int32_t lx, int32_t ly) noexcept -> AWidget* {
      receivedX = lx;
      receivedY = ly;
      return nullptr;
    }, nullptr);
    // Absolute press at (480 + 15, 90 + 20) = (495, 110)
    // Expected target-local coordinates = (15, 20)
    w->OnMousePress(495, 110, BTN_LEFT);
    w->OnMouseRelease(495, 110, BTN_LEFT);
    TEST_ASSERT_EQ(receivedX, 15, 501);
    TEST_ASSERT_EQ(receivedY, 20, 502);
  }
  D1("test_gui_edge_cases passed");
  return 0;
}

int32_t main() {
  //UNUSED char *qqq = new char[1]; // generate error
  int32_t testsfailed = 0;
  testsfailed += runTimedTest(test_aui_lifecycle, 1);
  testsfailed += runTimedTest(test_aui_lifecycle2, 1);
  testsfailed += runTimedTest(test_aui_lifecycle3, 1);
  testsfailed += runTimedTest(test_aui_lifecycle4, 1);
  testsfailed += runTimedTest(test_aui_lifecycle5, 1);
  testsfailed += runTimedTest(test_aui_lifecycle6, 1);
  testsfailed += runTimedTest(test_aui_lifecycle7, 1);
  testsfailed += runTimedTest(test_aui_lifecycle8, 1);
  testsfailed += runTimedTest(test_aui_lifecycle9, 1);
  testsfailed += runTimedTest(test_window_properties, 1);
  testsfailed += runTimedTest(test_multiple_windows, 1);
  testsfailed += runTimedTest(test_multiple_windows2, 1);
  testsfailed += runTimedTest(test_window_operations, 1);
  testsfailed += runTimedTest(testNestedClick, 1);
  testsfailed += runTimedTest(test_mouse_wheel_propagation, 1);
  testsfailed += runTimedTest(test_gui_edge_cases, 1);

  testsfailed += runTimedTest(test_aui_lifecycle, 200);
  testsfailed += runTimedTest(test_aui_lifecycle2, 100);
  testsfailed += runTimedTest(test_aui_lifecycle3, 100);
  testsfailed += runTimedTest(test_aui_lifecycle4, 100);
  testsfailed += runTimedTest(test_aui_lifecycle5, 100);
  testsfailed += runTimedTest(test_aui_lifecycle6, 100);
  testsfailed += runTimedTest(test_aui_lifecycle7, 100);
  testsfailed += runTimedTest(test_aui_lifecycle8, 100);
  testsfailed += runTimedTest(test_aui_lifecycle9, 100);
  testsfailed += runTimedTest(test_window_properties, 100);
  testsfailed += runTimedTest(test_multiple_windows, 100);
  testsfailed += runTimedTest(test_multiple_windows2, 100);
  testsfailed += runTimedTest(test_window_operations, 100);
  testsfailed += runTimedTest(testNestedClick, 100);
  testsfailed += runTimedTest(test_mouse_wheel_propagation, 100);
  testsfailed += runTimedTest(test_gui_edge_cases, 100);

  D("test suite complete")

  return testsfailed;
}

