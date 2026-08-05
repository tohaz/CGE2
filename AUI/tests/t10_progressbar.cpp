#include "AUILib.h"

using namespace aui;

// ------------------------------------------------------------------
// Helper: wait a few milliseconds
// ------------------------------------------------------------------
UNUSED static void WaitForThread(uint32_t ms = 50) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ------------------------------------------------------------------
// Attachment and basic properties
// ------------------------------------------------------------------
int32_t test_progressbar_attachment(AUI* au) {
  D1("test_progressbar_attachment start");
  AWindow* win = au->MainWnd();
  AProgressBar* pb = AProgressBar::AttachTo(win);
  TEST_ASSERT_NE(pb, nullptr, 2);
  TEST_ASSERT_DOUBLE_EQ(pb->Progress(), 0.0, 3);
  TEST_ASSERT_EQ(pb->IsIndeterminate(), false, 4);
  TEST_ASSERT_EQ(pb->IsTextVisible(), true, 5);
  TEST_ASSERT_EQ(pb->RoundedCorners(), false, 6);
  TEST_ASSERT_EQ(pb->IsStripeEnabled(), false, 7);
  D1("test_progressbar_attachment passed");
  return 0;
}

// ------------------------------------------------------------------
// Progress control (SetProgress, GetProgress, Clear)
// ------------------------------------------------------------------
int32_t test_progressbar_progress(AUI* au) {
  D1("test_progressbar_progress start");
  AWindow* win = au->MainWnd();
  AProgressBar* pb = AProgressBar::AttachTo(win);
  pb->Progress(0.5);
  TEST_ASSERT_DOUBLE_EQ(pb->Progress(), 0.5, 2);
  pb->Progress(1.2);
  TEST_ASSERT_DOUBLE_EQ(pb->Progress(), 1.0, 3);
  pb->Progress(-0.1);
  TEST_ASSERT_DOUBLE_EQ(pb->Progress(), 0.0, 4);
  pb->Progress(0.75);
  pb->Clear();
  TEST_ASSERT_DOUBLE_EQ(pb->Progress(), 0.0, 5);
  D1("test_progressbar_progress passed");
  return 0;
}

// ------------------------------------------------------------------
// Range
// ------------------------------------------------------------------
int32_t test_progressbar_range(AUI* au) {
  D1("test_progressbar_range start");
  AWindow* win = au->MainWnd();
  AProgressBar* pb = AProgressBar::AttachTo(win);
  pb->Range(10.0, 20.0);
  TEST_ASSERT_DOUBLE_EQ(pb->Min(), 10.0, 2);
  TEST_ASSERT_DOUBLE_EQ(pb->Max(), 20.0, 3);
  pb->Progress(15.0);
  TEST_ASSERT_DOUBLE_EQ(pb->Progress(), 0.5, 4);
  pb->Progress(5.0);
  TEST_ASSERT_DOUBLE_EQ(pb->Progress(), 0.0, 5);
  pb->Progress(25.0);
  TEST_ASSERT_DOUBLE_EQ(pb->Progress(), 1.0, 6);
  D1("test_progressbar_range passed");
  return 0;
}

// ------------------------------------------------------------------
// Indeterminate mode
// ------------------------------------------------------------------
int32_t test_progressbar_indeterminate(AUI* au) {
  D1("test_progressbar_indeterminate start");
  AWindow* win = au->MainWnd();
  AProgressBar* pb = AProgressBar::AttachTo(win);
  pb->Indeterminate(true);
  TEST_ASSERT_EQ(pb->IsIndeterminate(), true, 2);
  pb->Progress(0.5);
  TEST_ASSERT_DOUBLE_EQ(pb->Progress(), 0.0, 3);
  pb->Indeterminate(false);
  TEST_ASSERT_EQ(pb->IsIndeterminate(), false, 4);
  D1("test_progressbar_indeterminate passed");
  return 0;
}

// ------------------------------------------------------------------
// Text visibility and formatting
// ------------------------------------------------------------------
int32_t test_progressbar_text(AUI* au) {
  D1("test_progressbar_text start");
  AWindow* win = au->MainWnd();
  AProgressBar* pb = AProgressBar::AttachTo(win);
  pb->ShowText(false);
  TEST_ASSERT_EQ(pb->IsTextVisible(), false, 2);
  pb->ShowText(true);
  TEST_ASSERT_EQ(pb->IsTextVisible(), true, 3);
  pb->TextFormat("%.2f%%");
  TEST_ASSERT_EQ(pb->TextFormat(), std::string("%.2f%%"), 4);
  D1("test_progressbar_text passed");
  return 0;
}

// ------------------------------------------------------------------
// Colors and gradient
// ------------------------------------------------------------------
int32_t test_progressbar_colors(AUI* au) {
  D1("test_progressbar_colors start");
  AWindow* win = au->MainWnd();
  AProgressBar* pb = AProgressBar::AttachTo(win);
  pb->BGColor3(0xFF00FF00);
  TEST_ASSERT_EQ(pb->BGColor3(), 0xFF00FF00U, 2);
  pb->BGColor4(0xFFFF0000);
  TEST_ASSERT_EQ(pb->BGColor4(), 0xFFFF0000U, 3);
  pb->BGColor4(0);
  TEST_ASSERT_EQ(pb->BGColor4(), 0U, 4);
  D1("test_progressbar_colors passed");
  return 0;
}
//
// ------------------------------------------------------------------
// Orientation and direction
// ------------------------------------------------------------------
int32_t test_progressbar_orientation_direction(AUI* au) {
  D1("test_progressbar_orientation_direction start");
  AWindow* win = au->MainWnd();
  AProgressBar* pb = AProgressBar::AttachTo(win);
  pb->Orient(AUIOrientation::vertical);
  TEST_ASSERT_EQ(pb->Orient(), AUIOrientation::vertical, 2);
  pb->Direction(AUIDirection::bottom);
  TEST_ASSERT_EQ(pb->Direction(), AUIDirection::bottom, 3);
  pb->Orient(AUIOrientation::horizontal);
  TEST_ASSERT_EQ(pb->Orient(), AUIOrientation::horizontal, 4);
  D1("test_progressbar_orientation_direction passed");
  return 0;
}

// ------------------------------------------------------------------
// Stripes
// ------------------------------------------------------------------
int32_t test_progressbar_stripes(AUI* au) {
  D1("test_progressbar_stripes start");
  AWindow* win = au->MainWnd();
  AProgressBar* pb = AProgressBar::AttachTo(win);
  pb->Stripe(true);
  TEST_ASSERT_EQ(pb->IsStripeEnabled(), true, 2);
  pb->StripeColor(0x40FF00FF);
  TEST_ASSERT_EQ(pb->StripeColor(), 0x40FF00FFU, 3);
  pb->StripeWidth(10);
  TEST_ASSERT_EQ(pb->StripeWidth(), 10U, 4);
  pb->StripeSpeed(5);
  TEST_ASSERT_EQ(pb->StripeSpeed(), 5, 5);
  pb->Stripe(false);
  TEST_ASSERT_EQ(pb->IsStripeEnabled(), false, 6);
  D1("test_progressbar_stripes passed");
  return 0;
}

// ------------------------------------------------------------------
// Rounded corners
// ------------------------------------------------------------------
int32_t test_progressbar_rounded_corners(AUI* au) {
  D1("test_progressbar_rounded_corners start");
  AWindow* win = au->MainWnd();
  AProgressBar* pb = AProgressBar::AttachTo(win);
  pb->RoundedCorners(true, 12);
  TEST_ASSERT_EQ(pb->RoundedCorners(), true, 2);
  TEST_ASSERT_EQ(pb->CornerRadius(), 12U, 3);
  pb->RoundedCorners(false);
  TEST_ASSERT_EQ(pb->RoundedCorners(), false, 4);
  D1("test_progressbar_rounded_corners passed");
  return 0;
}

// ------------------------------------------------------------------
// Callbacks
// ------------------------------------------------------------------
int32_t test_progressbar_callbacks(AUI* au) {
  D1("test_progressbar_callbacks start");
  AWindow* win = au->MainWnd();
  AProgressBar* pb = AProgressBar::AttachTo(win);
  bool startCalled = false;
  bool changedCalled = false;
  bool completeCalled = false;
  pb->SetOnStart([&](double) noexcept { startCalled = true; });
  pb->SetOnProgressChanged([&](double) noexcept { changedCalled = true; });
  pb->SetOnComplete([&](double) noexcept { completeCalled = true; });
  pb->Progress(0.0);
  pb->Progress(0.5);
  pb->Progress(1.0);
  TEST_ASSERT_EQ(startCalled, true, 2);
  TEST_ASSERT_EQ(changedCalled, true, 3);
  TEST_ASSERT_EQ(completeCalled, true, 4);
  D1("test_progressbar_callbacks passed");
  return 0;
}
// ------------------------------------------------------------------
// Progress provider (background thread)
// ------------------------------------------------------------------
int32_t test_progressbar_provider(AUI* au) {
  D1("test_progressbar_provider start");
  AWindow* win = au->MainWnd();
  AProgressBar* pb = AProgressBar::AttachTo(win);
  pb->UpdateInterval(10);      // 10ms update interval
  pb->Progress(0.0);
  pb->SetProgressProvider([]() noexcept {
    static double p = 0.0;
    p += 0.1;
    if (p > 1.0) p = 0.0;
    return p;
  });
  // Wait 100ms for the background thread to update at least a few times
  WaitForThread(100);
  double val = pb->Progress();
  TEST_ASSERT(val > 0.0, 2);
  pb->SetProgressProvider(nullptr);
  double oldVal = pb->Progress();
  WaitForThread(100);
  TEST_ASSERT_DOUBLE_EQ(pb->Progress(), oldVal, 3);
  D1("test_progressbar_provider passed");
  return 0;
}
// ------------------------------------------------------------------
// Pause/Resume
// ------------------------------------------------------------------
int32_t test_progressbar_pause(AUI* au) {
  D1("test_progressbar_pause start");
  AWindow* win = au->MainWnd();
  AProgressBar* pb = AProgressBar::AttachTo(win);
  pb->UpdateInterval(10);
  pb->Progress(0.0);
  pb->SetProgressProvider([]() noexcept {
    static double p = 0.0;
    p += 0.1;
    if (p > 1.0) p = 0.0;
    return p;
  });
  WaitForThread(100);
//  double val1 = pb->Progress();
  pb->PauseUpdates(true);
  double val2 = pb->Progress();
  WaitForThread(100);
  TEST_ASSERT_DOUBLE_EQ(pb->Progress(), val2, 2);
  pb->PauseUpdates(false);
  WaitForThread(100);
  double val3 = pb->Progress();
  D("val3 {} > val2 {}", val3, val2)
  TEST_ASSERT(std::abs(val3 - val2) > 1e-9, 3);
//  (void)val1;
  D1("test_progressbar_pause passed");
  return 0;
}

// ------------------------------------------------------------------
// Resize (should not crash)
// ------------------------------------------------------------------
int32_t test_progressbar_resize(AUI* au) {
  D1("test_progressbar_resize start");
  AWindow* win = au->MainWnd();
  AProgressBar* pb = AProgressBar::AttachTo(win);
  pb->Resize(100, 50);
  pb->Move(10, 20);
  D1("test_progressbar_resize passed");
  return 0;
}

// ------------------------------------------------------------------
// main
// ------------------------------------------------------------------
int main() {
  int32_t testsfailed = 0;
  testsfailed += runTimedTest(test_progressbar_attachment, 1);
  testsfailed += runTimedTest(test_progressbar_progress, 1);
  testsfailed += runTimedTest(test_progressbar_range, 1);
  testsfailed += runTimedTest(test_progressbar_indeterminate, 1);
  testsfailed += runTimedTest(test_progressbar_text, 1);
  testsfailed += runTimedTest(test_progressbar_colors, 1);
  testsfailed += runTimedTest(test_progressbar_orientation_direction, 1);
  testsfailed += runTimedTest(test_progressbar_stripes, 1);
  testsfailed += runTimedTest(test_progressbar_rounded_corners, 1);
  testsfailed += runTimedTest(test_progressbar_callbacks, 1);
  testsfailed += runTimedTest(test_progressbar_provider, 200);
  testsfailed += runTimedTest(test_progressbar_pause, 200);
  testsfailed += runTimedTest(test_progressbar_resize, 1);

  testsfailed += runTimedTest(test_progressbar_attachment, 200);
  testsfailed += runTimedTest(test_progressbar_progress, 200);
  testsfailed += runTimedTest(test_progressbar_range, 200);
  testsfailed += runTimedTest(test_progressbar_indeterminate, 200);
  testsfailed += runTimedTest(test_progressbar_text, 200);
  testsfailed += runTimedTest(test_progressbar_colors, 200);
  testsfailed += runTimedTest(test_progressbar_orientation_direction, 200);
  testsfailed += runTimedTest(test_progressbar_stripes, 200);
  testsfailed += runTimedTest(test_progressbar_rounded_corners, 200);
  testsfailed += runTimedTest(test_progressbar_callbacks, 200);
  testsfailed += runTimedTest(test_progressbar_provider, 200);
  testsfailed += runTimedTest(test_progressbar_pause, 200);
  testsfailed += runTimedTest(test_progressbar_resize, 200);

  D("test suite complete");
  return testsfailed;
}
