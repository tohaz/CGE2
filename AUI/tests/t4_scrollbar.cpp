#include <chrono>
#include <future>
#include <thread>
#include <memory.h>

#include "AUILib.h"
#include "AScrollBar.h"

using namespace aui;

#define TEST_ASSERT(cond, errcode) do { if(!(cond)) { E("Test failed: {}", #cond); return errcode; } } while(0)

// ------------------------------------------------------------------
// Basic attachment and default values
// ------------------------------------------------------------------
int32_t test_scrollbar_attachment() {
  D1("test_scrollbar_attachment start");
  AUI* au = AUI::Create("ScrollBarTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT(w != nullptr, 2);
  AScrollBar* sb = AScrollBar::AttachTo(w);
  TEST_ASSERT(sb != nullptr, 3);
  TEST_ASSERT(sb->GetOrientation() == AUIOrientation::vertical, 4);
  TEST_ASSERT(sb->GetMinValue() == 0, 5);
  TEST_ASSERT(sb->GetMaxValue() == 100, 6);
  TEST_ASSERT(sb->GetValue() == 0, 7);
  delete au;
  D1("test_scrollbar_attachment passed");
  return 0;
}

// ------------------------------------------------------------------
// Orientation and size
// ------------------------------------------------------------------
int32_t test_scrollbar_orientation() {
  D1("test_scrollbar_orientation start");
  AUI* au = AUI::Create("ScrollBarOrientationTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  AScrollBar* v = AScrollBar::AttachTo(w, AUIOrientation::vertical);
  TEST_ASSERT(v->GetOrientation() == AUIOrientation::vertical, 2);
  AScrollBar* h = AScrollBar::AttachTo(w, AUIOrientation::horizontal);
  TEST_ASSERT(h->GetOrientation() == AUIOrientation::horizontal, 3);
  delete au;
  D1("test_scrollbar_orientation passed");
  return 0;
}

// ------------------------------------------------------------------
// Range and value manipulation
// ------------------------------------------------------------------
int32_t test_scrollbar_range_and_value() {
  D1("test_scrollbar_range_and_value start");
  AUI* au = AUI::Create("ScrollBarRangeTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  AScrollBar* sb = AScrollBar::AttachTo(w);
  sb->SetRange(10, 200);
  TEST_ASSERT(sb->GetMinValue() == 10, 2);
  TEST_ASSERT(sb->GetMaxValue() == 200, 3);
  sb->SetValue(150);
  TEST_ASSERT(sb->GetValue() == 150, 4);
  sb->SetValue(5);   // below min
  TEST_ASSERT(sb->GetValue() == 10, 5);
  sb->SetValue(300); // above max
  TEST_ASSERT(sb->GetValue() == 200, 6);
  delete au;
  D1("test_scrollbar_range_and_value passed");
  return 0;
}

// ------------------------------------------------------------------
// Page step (affects thumb length)
// ------------------------------------------------------------------
int32_t test_scrollbar_page_step() {
  D1("test_scrollbar_page_step start");
  AUI* au = AUI::Create("ScrollBarPageStepTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  AScrollBar* sb = AScrollBar::AttachTo(w);
  sb->SetRange(0, 100);
  sb->SetPageStep(25);
  TEST_ASSERT(sb->GetPageStep() == 25, 2);
  // Thumb length depends on page step; we can't easily verify the computed length
  // but we can call SetPageStep and check it doesn't crash.
  delete au;
  D1("test_scrollbar_page_step passed");
  return 0;
}

// ------------------------------------------------------------------
// Thumb and track colors
// ------------------------------------------------------------------
int32_t test_scrollbar_colors() {
  D1("test_scrollbar_colors start");
  AUI* au = AUI::Create("ScrollBarColorsTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  AScrollBar* sb = AScrollBar::AttachTo(w);
  sb->SetThumbColor(0xFF123456);
  TEST_ASSERT(sb->GetThumbColor() == 0xFF123456, 2);
  sb->SetTrackColor(0xFF654321);
  TEST_ASSERT(sb->GetTrackColor() == 0xFF654321, 3);
  delete au;
  D1("test_scrollbar_colors passed");
  return 0;
}

// ------------------------------------------------------------------
// Scroll callback (via SetValue)
// ------------------------------------------------------------------
int32_t test_scrollbar_callback() {
  D1("test_scrollbar_callback start");
  AUI* au = AUI::Create("ScrollBarCallbackTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  AScrollBar* sb = AScrollBar::AttachTo(w);
  bool callbackFired = false;
  int32_t callbackValue = 0;
  sb->SetScrollCallback(
      [&](AWindow*, AWidget*, void*, int32_t val) {
        callbackFired = true;
        callbackValue = val;
      },
      nullptr);
  sb->SetValue(42);
  TEST_ASSERT(callbackFired == true, 2);
  TEST_ASSERT(callbackValue == 42, 3);
  delete au;
  D1("test_scrollbar_callback passed");
  return 0;
}

// ------------------------------------------------------------------
// Click in track (jump to value)
// ------------------------------------------------------------------
int32_t test_scrollbar_click_in_track() {
  D1("test_scrollbar_click_in_track start");
  AUI* au = AUI::Create("ScrollBarClickTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  w->EnableResize();          // allow resize if needed
  w->Resize(400, 300);
  AScrollBar* sb = AScrollBar::AttachTo(w, AUIOrientation::vertical);
  sb->Move(380, 40);
  sb->Resize(16, 240);
  sb->SetRange(0, 100);
  sb->SetValue(0);
  // Simulate a click in the track (y=140, where thumb is at position 0-??)
  // The thumb length for 0..100 with pageStep=10? Default pageStep is 10?
  // For simplicity, we call OnMouseClick with coordinates that should map to a new value.
  // We need to compute the thumb position first? Actually we can just click at a coordinate
  // that is definitely not in the thumb (e.g., bottom half). Use local coordinates.
  // For a vertical scrollbar of height 240, value range 0..100, pageStep=10,
  // the track length = 240. The thumb will be at some position. We can click near the bottom.
  // We'll set a known value and then click, then verify value changed.
  sb->SetValue(0);
  // Click at local y=200 (well below the thumb which is at top)
  sb->OnMouseClick(5, 200, true);
  int32_t newVal = sb->GetValue();
  TEST_ASSERT(newVal > 0, 2);
  // Also test release should not change value
  sb->OnMouseClick(5, 200, false);
  TEST_ASSERT(sb->GetValue() == newVal, 3);
  delete au;
  D1("test_scrollbar_click_in_track passed");
  return 0;
}

// ------------------------------------------------------------------
// Drag simulation (thumb follows mouse)
// ------------------------------------------------------------------
int32_t test_scrollbar_drag() {
  D1("test_scrollbar_drag start");
  AUI* au = AUI::Create("ScrollBarDragTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(400, 300);
  AScrollBar* sb = AScrollBar::AttachTo(w, AUIOrientation::vertical);
  sb->Move(380, 40);
  sb->Resize(16, 240);
  sb->SetRange(0, 100);
  sb->SetValue(50);
  // Simulate press on thumb (we need to know thumb position)
  // For simplicity, we can just simulate a sequence: press at thumb position (obtained via GetThumbPosition)
  // Since thumb position is not exposed, we'll use a known coordinate that should be inside thumb.
  // The thumb length is ~ (visibleRatio * trackLen). For pageStep=10, range=100 -> visibleRatio=10/110=0.09, thumbLen~21.6,
  // clamped to 20. Thumb position for value 50 is around 110. We'll press at (5, 115) and drag down.
  sb->OnMouseClick(5, 115, true);
  int32_t startVal = sb->GetValue();
  // Move mouse down by 30 pixels
  sb->OnMouseMove(5, 145);
  int32_t afterMove = sb->GetValue();
  TEST_ASSERT(afterMove > startVal, 2);
  // Release
  sb->OnMouseClick(5, 145, false);
  // After release, further move should not change value
  int32_t beforeReleaseMove = sb->GetValue();
  sb->OnMouseMove(5, 175);
  TEST_ASSERT(sb->GetValue() == beforeReleaseMove, 3);
  delete au;
  D1("test_scrollbar_drag passed");
  return 0;
}

int32_t test_scrollbar_large_range() {
    D1("test_scrollbar_regression start");

    AScrollBar sb;
    sb.SetOrientation(AUIOrientation::vertical);
    sb.Resize(20, 200);                 // height = 200 px
    sb.SetRange(0, 10000000);           // huge range
    sb.SetPageStep(1000);
    sb.SetSingleStep(100);
    sb.SetShowArrows(true);             // CRITICAL: arrows ON – trackStart = 12

    // Get thumb geometry
    uint32_t thumbPos = sb.GetThumbPosition();   // should be 0 at min
    uint32_t thumbLen = sb.GetThumbLength();     // will be clamped to 20
    uint32_t trackStart = 12;                    // default arrowSize

    // Click on the center of the thumb (absolute coordinate)
    int32_t thumbCenterY = SafeINT32(trackStart + thumbPos + thumbLen / 2);
    bool consumed = sb.OnMouseClick(10, thumbCenterY, true);
    TEST_ASSERT(consumed, 1);

    // If the bug is present, the value will have jumped to a large number.
    // With the fix, the value remains 0.
    int32_t newValue = sb.GetValue();
    TEST_ASSERT_EQ(newValue, 0, 2);   // this will fail if the bug returns

    D1("test_scrollbar_regression passed");
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

  testsfailed += test_scrollbar_attachment();
  testsfailed += test_scrollbar_orientation();
  testsfailed += test_scrollbar_range_and_value();
  testsfailed += test_scrollbar_page_step();
  testsfailed += test_scrollbar_colors();
  testsfailed += test_scrollbar_callback();
  testsfailed += test_scrollbar_click_in_track();
  testsfailed += test_scrollbar_drag();
  testsfailed += test_scrollbar_large_range();
  au->ProcessMessages();
  handle.get();
  delete au;
  au = nullptr;

  return testsfailed;
}

