#include "AUILib.h"

using namespace aui;

// ------------------------------------------------------------------
// Basic attachment and default values
// ------------------------------------------------------------------
int32_t test_scrollbar_attachment(AUI* au) {
  D1("test_scrollbar_attachment start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 2);
  AScrollBar* sb = AScrollBar::AttachTo(w);
  TEST_ASSERT_NE(sb, nullptr, 3);
  TEST_ASSERT_EQ(sb->GetOrientation(), AUIOrientation::vertical, 4);
  TEST_ASSERT_EQ(sb->GetMinValue(), 0, 5);
  TEST_ASSERT_EQ(sb->GetMaxValue(), 100, 6);
  TEST_ASSERT_EQ(sb->GetValue(), 0, 7);
  D1("test_scrollbar_attachment passed");
  return 0;
}
// ------------------------------------------------------------------
// Orientation and size
// ------------------------------------------------------------------
int32_t test_scrollbar_orientation(AUI* au) {
  D1("test_scrollbar_orientation start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AScrollBar* v = AScrollBar::AttachTo(w, AUIOrientation::vertical);
  TEST_ASSERT_EQ(v->GetOrientation(), AUIOrientation::vertical, 2);
  AScrollBar* h = AScrollBar::AttachTo(w, AUIOrientation::horizontal);
  TEST_ASSERT_EQ(h->GetOrientation(), AUIOrientation::horizontal, 3);
  D1("test_scrollbar_orientation passed");
  return 0;
}
// ------------------------------------------------------------------
// Range and value manipulation
// ------------------------------------------------------------------
int32_t test_scrollbar_range_and_value(AUI* au) {
  D1("test_scrollbar_range_and_value start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AScrollBar* sb = AScrollBar::AttachTo(w);
  sb->SetRange(10, 200);
  TEST_ASSERT_EQ(sb->GetMinValue(), 10, 2);
  TEST_ASSERT_EQ(sb->GetMaxValue(), 200, 3);
  sb->SetValue(150);
  TEST_ASSERT_EQ(sb->GetValue(), 150, 4);
  sb->SetValue(5);   // below min
  TEST_ASSERT_EQ(sb->GetValue(), 10, 5);
  sb->SetValue(300); // above max
  TEST_ASSERT_EQ(sb->GetValue(), 200, 6);
  D1("test_scrollbar_range_and_value passed");
  return 0;
}
// ------------------------------------------------------------------
// Page step (affects thumb length)
// ------------------------------------------------------------------
int32_t test_scrollbar_page_step(AUI* au) {
  D1("test_scrollbar_page_step start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AScrollBar* sb = AScrollBar::AttachTo(w);
  sb->SetRange(0, 100);
  sb->SetPageStep(25);
  TEST_ASSERT_EQ(sb->GetPageStep(), 25, 2);
  D1("test_scrollbar_page_step passed");
  return 0;
}
// ------------------------------------------------------------------
// Thumb and track colors
// ------------------------------------------------------------------
int32_t test_scrollbar_colors(AUI* au) {
  D1("test_scrollbar_colors start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AScrollBar* sb = AScrollBar::AttachTo(w);
  sb->SetThumbColor(0xFF123456);
  TEST_ASSERT_EQ(sb->GetThumbColor(), 0xFF123456, 2);
  sb->SetTrackColor(0xFF654321);
  TEST_ASSERT_EQ(sb->GetTrackColor(), 0xFF654321, 3);
  D1("test_scrollbar_colors passed");
  return 0;
}
// ------------------------------------------------------------------
// Scroll callback (via SetValue)
// ------------------------------------------------------------------
int32_t test_scrollbar_callback(AUI* au) {
  D1("test_scrollbar_callback start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AScrollBar* sb = AScrollBar::AttachTo(w);
  bool callbackFired = false;
  int32_t callbackValue = 0;
  sb->SetScrollCallback(
      [&](AWindow*, AWidget*, void*, int32_t val) noexcept {
        callbackFired = true;
        callbackValue = val;
      },
      nullptr);
  sb->SetValue(42);
  TEST_ASSERT(callbackFired == true, 2);
  TEST_ASSERT_EQ(callbackValue, 42, 3);
  D1("test_scrollbar_callback passed");
  return 0;
}
// ------------------------------------------------------------------
// Click in track (jump to value)
// ------------------------------------------------------------------
int32_t test_scrollbar_click_in_track(AUI* au) {
  D1("test_scrollbar_click_in_track start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(400, 300);
  AScrollBar* sb = AScrollBar::AttachTo(w, AUIOrientation::vertical);
  sb->Move(380, 40);
  sb->Resize(16, 240);
  sb->SetRange(0, 100);
  sb->SetValue(0);
  sb->SetValue(0);
  sb->OnMouseClick(5, 200, true);
  int32_t newVal = sb->GetValue();
  TEST_ASSERT(newVal > 0, 2);
  sb->OnMouseClick(5, 200, false);
  TEST_ASSERT_EQ(sb->GetValue(), newVal, 3);
  D1("test_scrollbar_click_in_track passed");
  return 0;
}
// ------------------------------------------------------------------
// Drag simulation (thumb follows mouse)
// ------------------------------------------------------------------
int32_t test_scrollbar_drag(AUI* au) {
  D1("test_scrollbar_drag start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(400, 300);
  AScrollBar* sb = AScrollBar::AttachTo(w, AUIOrientation::vertical);
  sb->Move(380, 40);
  sb->Resize(16, 240);
  sb->SetRange(0, 100);
  sb->SetValue(50);
  sb->OnMouseClick(5, 115, true);
  int32_t startVal = sb->GetValue();
  sb->OnMouseMove(5, 145);
  int32_t afterMove = sb->GetValue();
  TEST_ASSERT(afterMove > startVal, 2);
  sb->OnMouseClick(5, 145, false);
  int32_t beforeReleaseMove = sb->GetValue();
  sb->OnMouseMove(5, 175);
  TEST_ASSERT_EQ(sb->GetValue(), beforeReleaseMove, 3);
  D1("test_scrollbar_drag passed");
  return 0;
}
// ------------------------------------------------------------------
// Large range regression test
// ------------------------------------------------------------------
int32_t test_scrollbar_large_range(AUI* au) {
  D1("test_scrollbar_regression start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  AScrollBar* sb = AScrollBar::AttachTo(w);
  TEST_ASSERT_NE(sb, nullptr, 1);
  sb->SetOrientation(AUIOrientation::vertical);
  sb->Resize(20, 200);
  sb->SetRange(0, 10000000);
  sb->SetPageStep(1000);
  sb->SetSingleStep(100);
  sb->SetShowArrows(true);
  uint32_t thumbPos = sb->GetThumbPosition();
  uint32_t thumbLen = sb->GetThumbLength();
  uint32_t trackStart = 12;
  int32_t thumbCenterY = SafeINT32(trackStart + thumbPos + thumbLen / 2);
  bool consumed = sb->OnMouseClick(10, thumbCenterY, true);
  TEST_ASSERT(consumed, 1);
  int32_t newValue = sb->GetValue();
  TEST_ASSERT_EQ(newValue, 0, 2);
  D1("test_scrollbar_regression passed");
  return 0;
}
// ------------------------------------------------------------------
// Main: run all tests with timed test harness
// ------------------------------------------------------------------
int main() {
  int32_t testsfailed = 0;

  testsfailed += runTimedTest("test_scrollbar_attachment", test_scrollbar_attachment, 1);
  testsfailed += runTimedTest("test_scrollbar_orientation", test_scrollbar_orientation, 1);
  testsfailed += runTimedTest("test_scrollbar_range_and_value", test_scrollbar_range_and_value, 1);
  testsfailed += runTimedTest("test_scrollbar_page_step", test_scrollbar_page_step, 1);
  testsfailed += runTimedTest("test_scrollbar_colors", test_scrollbar_colors, 1);
  testsfailed += runTimedTest("test_scrollbar_callback", test_scrollbar_callback, 1);
  testsfailed += runTimedTest("test_scrollbar_click_in_track", test_scrollbar_click_in_track, 1);
  testsfailed += runTimedTest("test_scrollbar_drag", test_scrollbar_drag, 1);
  testsfailed += runTimedTest("test_scrollbar_large_range", test_scrollbar_large_range, 1);

  testsfailed += runTimedTest("test_scrollbar_attachment", test_scrollbar_attachment, 200);
  testsfailed += runTimedTest("test_scrollbar_orientation", test_scrollbar_orientation, 200);
  testsfailed += runTimedTest("test_scrollbar_range_and_value", test_scrollbar_range_and_value, 200);
  testsfailed += runTimedTest("test_scrollbar_page_step", test_scrollbar_page_step, 200);
  testsfailed += runTimedTest("test_scrollbar_colors", test_scrollbar_colors, 200);
  testsfailed += runTimedTest("test_scrollbar_callback", test_scrollbar_callback, 200);
  testsfailed += runTimedTest("test_scrollbar_click_in_track", test_scrollbar_click_in_track, 200);
  testsfailed += runTimedTest("test_scrollbar_drag", test_scrollbar_drag, 200);
  testsfailed += runTimedTest("test_scrollbar_large_range", test_scrollbar_large_range, 200);


  D("test suite complete");
  return testsfailed;
}
