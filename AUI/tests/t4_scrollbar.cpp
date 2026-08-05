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
  TEST_ASSERT_EQ(sb->Orient(), AUIOrientation::vertical, 4);
  TEST_ASSERT_EQ(sb->MinValue(), 0, 5);
  TEST_ASSERT_EQ(sb->MaxValue(), 100, 6);
  TEST_ASSERT_EQ(sb->Value(), 0, 7);
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
  TEST_ASSERT_NE(w, nullptr, 1);
  AScrollBar* v = AScrollBar::AttachTo(w, AUIOrientation::vertical);
  TEST_ASSERT_EQ(v->Orient(), AUIOrientation::vertical, 2);
  AScrollBar* h = AScrollBar::AttachTo(w, AUIOrientation::horizontal);
  TEST_ASSERT_EQ(h->Orient(), AUIOrientation::horizontal, 3);
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
  sb->Range(10, 200);
  TEST_ASSERT_EQ(sb->MinValue(), 10, 2);
  TEST_ASSERT_EQ(sb->MaxValue(), 200, 3);
  sb->Value(150);
  TEST_ASSERT_EQ(sb->Value(), 150, 4);
  sb->Value(5);   // below min
  TEST_ASSERT_EQ(sb->Value(), 10, 5);
  sb->Value(300); // above max
  TEST_ASSERT_EQ(sb->Value(), 200 - 10 - sb->PageStep(), 6);
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
  sb->Range(0, 100);
  sb->PageStep(25);
  TEST_ASSERT_EQ(sb->PageStep(), 25, 2);
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
  sb->ThumbColor(0xFF123456);
  TEST_ASSERT_EQ(sb->ThumbColor(), 0xFF123456, 2);
  sb->TrackColor(0xFF654321);
  TEST_ASSERT_EQ(sb->TrackColor(), 0xFF654321, 3);
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
      [&](AWidget*, void*, int32_t val) noexcept {
        callbackFired = true;
        callbackValue = val;
      },
      nullptr);
  sb->Value(42);
  TEST_ASSERT(callbackFired == true, 2);
  TEST_ASSERT_EQ(callbackValue, 42, 3);
  D1("test_scrollbar_callback passed");
  return 0;
}
// ------------------------------------------------------------------
// Click in track (jump to value)
// ------------------------------------------------------------------
int32_t test_scrollbar_click_in_track(UNUSED AUI* au) {
  D1("test_scrollbar_click_in_track start");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(400, 300);
  w->DisableResize();
  AScrollBar* sb = AScrollBar::AttachTo(w, AUIOrientation::vertical);
  sb->Move(380, 40);
  sb->Resize(16, 240);
  sb->Range(0, 100);
  sb->Value(0);
  sb->MouseClick(5, 200);
  TEST_ASSERT_EQ(sb->Value(), 10, 2);
  D2("val {}", sb->Value())
  sb->MouseClick(5, 200);
  D2("val {}", sb->Value())
  TEST_ASSERT_EQ(sb->Value(), 20, 3);
  sb->MouseClick(5, 20);
  D2("val {}", sb->Value())
  TEST_ASSERT_EQ(sb->Value(), 10, 3);
  sb->MouseClick(1, 1);
  D2("val {}", sb->Value())
  TEST_ASSERT_EQ(sb->Value(), 9, 3);
  D1("test_scrollbar_click_in_track passed");
  return 0;
}
// ------------------------------------------------------------------
// Drag simulation (thumb follows mouse)
// ------------------------------------------------------------------
int32_t test_scrollbar_drag(AUI* au) {
  D1("test_scrollbar_drag start");
  TEST_ASSERT_NE(au, nullptr, 1);
  UNUSED AWindow* w = au->MainWnd();

  w->EnableResize();
  w->Resize(400, 300);
  AScrollBar* sb = AScrollBar::AttachTo(w, AUIOrientation::vertical);
  sb->Move(380, 40);
  sb->Resize(16, 240);
  sb->Range(0, 100);
  sb->Value(50);
  sb->OnMouseDownLeft(5, 125);
  UNUSED int32_t startVal = sb->Value();
  D1("val1 {}", sb->Value())
  sb->OnMouseMove(5, 145);
  sb->OnMouseUpLeft(5, 145);
  UNUSED int32_t afterMove = sb->Value();
  D1("val2 {}", sb->Value())
  TEST_ASSERT_EQ(afterMove, 59, 2);

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
  sb->Orient(AUIOrientation::vertical);
  sb->Resize(20, 200);
  sb->Range(0, 10000000);
  sb->PageStep(1000);
  sb->SingleStep(100);
  sb->Arrows(true);
  uint32_t thumbPos = sb->ThumbPosition();
  uint32_t thumbLen = sb->ThumbLength();
  uint32_t trackStart = 12;
  int32_t thumbCenterY = SafeINT32(trackStart + thumbPos + thumbLen / 2);
  AWidget* consumed = sb->MouseClick(10, thumbCenterY);
  TEST_ASSERT_EQ(consumed, sb, 1);
  int32_t newValue = sb->Value();
  TEST_ASSERT_EQ(newValue, 0, 2);
  D1("test_scrollbar_regression passed");
  return 0;
}
// ------------------------------------------------------------------
// Main: run all tests with timed test harness
// ------------------------------------------------------------------
int main() {
  UNUSED int32_t testsfailed = 0;
//  UNUSED AUI* au = AUI::Create("test");
//  UNUSED AWindow* w = au->MainWnd();

//   au->ProcessMessages();
//  delete au;

  testsfailed += runTimedTest(test_scrollbar_attachment, 1);
  testsfailed += runTimedTest(test_scrollbar_orientation, 1);
  testsfailed += runTimedTest(test_scrollbar_range_and_value, 1);
  testsfailed += runTimedTest(test_scrollbar_page_step, 1);
  testsfailed += runTimedTest(test_scrollbar_colors, 1);
  testsfailed += runTimedTest(test_scrollbar_callback, 1);
  testsfailed += runTimedTest(test_scrollbar_click_in_track, 1);
  testsfailed += runTimedTest(test_scrollbar_drag, 1);
  testsfailed += runTimedTest(test_scrollbar_large_range, 1);

  D("test suite complete");
  return testsfailed;
}
