#include "AUILib.h"

using namespace aui;

// ------------------------------------------------------------------
// Helper: wait a few milliseconds
// ------------------------------------------------------------------
static void WaitForThread(uint32_t ms = 50) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// Run the event loop for a given number of milliseconds, then exit
//static void RunEventLoopFor(AUI* au, uint32_t ms) {
//  std::thread exit_thread([au, ms]() {
//    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
//    au->ExitAUI();
//  });
//  au->ProcessMessages();
//  exit_thread.join();
//}

// ------------------------------------------------------------------
// Attachment and basic properties
// ------------------------------------------------------------------
int32_t test_progressbar_attachment(AUI* au) {
  D1("test_progressbar_attachment start");
  AWindow* win = au->MainWnd();
  AProgressBar* pb = AProgressBar::AttachTo(win);
  TEST_ASSERT_NE(pb, nullptr, 2);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 0.0, 3);
  TEST_ASSERT_EQ(pb->IsIndeterminate(), false, 4);
  TEST_ASSERT_EQ(pb->IsTextVisible(), true, 5);
  TEST_ASSERT_EQ(pb->HasRoundedCorners(), false, 6);
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
  pb->SetProgress(0.5);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 0.5, 2);
  pb->SetProgress(1.2);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 1.0, 3);
  pb->SetProgress(-0.1);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 0.0, 4);
  pb->SetProgress(0.75);
  pb->Clear();
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 0.0, 5);
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
  pb->SetRange(10.0, 20.0);
  TEST_ASSERT_DOUBLE_EQ(pb->GetMin(), 10.0, 2);
  TEST_ASSERT_DOUBLE_EQ(pb->GetMax(), 20.0, 3);
  pb->SetProgress(15.0);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 0.5, 4);
  pb->SetProgress(5.0);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 0.0, 5);
  pb->SetProgress(25.0);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 1.0, 6);
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
  pb->SetIndeterminate(true);
  TEST_ASSERT_EQ(pb->IsIndeterminate(), true, 2);
  pb->SetProgress(0.5);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 0.0, 3);
  pb->SetIndeterminate(false);
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
  pb->SetShowText(false);
  TEST_ASSERT_EQ(pb->IsTextVisible(), false, 2);
  pb->SetShowText(true);
  TEST_ASSERT_EQ(pb->IsTextVisible(), true, 3);
  pb->SetTextFormat("%.2f%%");
  TEST_ASSERT_EQ(pb->GetTextFormat(), std::string("%.2f%%"), 4);
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
  pb->SetBarColor(0xFF00FF00);
  TEST_ASSERT_EQ(pb->GetBarColor(), 0xFF00FF00U, 2);
  pb->SetBarColor2(0xFFFF0000);
  TEST_ASSERT_EQ(pb->GetBarColor2(), 0xFFFF0000U, 3);
  pb->SetBarColor2(0);
  TEST_ASSERT_EQ(pb->GetBarColor2(), 0U, 4);
  D1("test_progressbar_colors passed");
  return 0;
}

// ------------------------------------------------------------------
// Orientation and direction
// ------------------------------------------------------------------
int32_t test_progressbar_orientation_direction(AUI* au) {
  D1("test_progressbar_orientation_direction start");
  AWindow* win = au->MainWnd();
  AProgressBar* pb = AProgressBar::AttachTo(win);
  pb->SetOrientation(AUIOrientation::vertical);
  TEST_ASSERT_EQ(pb->GetOrientation(), AUIOrientation::vertical, 2);
  pb->SetDirection(AUIDirection::bottom);
  TEST_ASSERT_EQ(pb->GetDirection(), AUIDirection::bottom, 3);
  pb->SetOrientation(AUIOrientation::horizontal);
  TEST_ASSERT_EQ(pb->GetOrientation(), AUIOrientation::horizontal, 4);
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
  pb->SetStripe(true);
  TEST_ASSERT_EQ(pb->IsStripeEnabled(), true, 2);
  pb->SetStripeColor(0x40FF00FF);
  TEST_ASSERT_EQ(pb->GetStripeColor(), 0x40FF00FFU, 3);
  pb->SetStripeWidth(10);
  TEST_ASSERT_EQ(pb->GetStripeWidth(), 10U, 4);
  pb->SetStripeSpeed(5);
  TEST_ASSERT_EQ(pb->GetStripeSpeed(), 5, 5);
  pb->SetStripe(false);
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
  pb->SetRoundedCorners(true, 12);
  TEST_ASSERT_EQ(pb->HasRoundedCorners(), true, 2);
  TEST_ASSERT_EQ(pb->GetCornerRadius(), 12U, 3);
  pb->SetRoundedCorners(false);
  TEST_ASSERT_EQ(pb->HasRoundedCorners(), false, 4);
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
  pb->SetProgress(0.0);
  pb->SetProgress(0.5);
  pb->SetProgress(1.0);
  TEST_ASSERT(startCalled, 2);
  TEST_ASSERT(changedCalled, 3);
  TEST_ASSERT(completeCalled, 4);
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
  pb->SetUpdateInterval(10);
  pb->SetProgress(0.0);
  pb->SetProgressProvider([]() noexcept {
    static double p = 0.0;
    p += 0.1;
    if (p > 1.0) p = 0.0;
    return p;
  });
  // Run event loop for 30ms to let the timer fire at least once
//  RunEventLoopFor(au, 30);
  std::this_thread::sleep_for(std::chrono::milliseconds(30));
  double val = pb->GetProgress();
  TEST_ASSERT(val > 0.0, 2);
  pb->SetProgressProvider(nullptr);
  double oldVal = pb->GetProgress();
//  RunEventLoopFor(au, 50);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), oldVal, 3);
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
  pb->SetUpdateInterval(10);
  pb->SetProgress(0.0);
  pb->SetProgressProvider([]() noexcept {
    static double p = 0.0;
    p += 0.1;
    if (p > 1.0) p = 0.0;
    return p;
  });
  WaitForThread(30);
  double val1 = pb->GetProgress();
  pb->PauseUpdates(true);
  double val2 = pb->GetProgress();
  WaitForThread(50);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), val2, 2);
  pb->PauseUpdates(false);
  WaitForThread(30);
  double val3 = pb->GetProgress();
  D("val3 {} > val2 {}", val3, val2)
  TEST_ASSERT(std::abs(val3 - val2) > 1e-9, 3);
  (void)val1;
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
  TEST_ASSERT(true, 2);
  D1("test_progressbar_resize passed");
  return 0;
}

// ------------------------------------------------------------------
// main
// ------------------------------------------------------------------
int main() {
  int32_t testsfailed = 0;
  testsfailed += runTimedTest("test_progressbar_attachment", test_progressbar_attachment, 1);
  testsfailed += runTimedTest("test_progressbar_progress", test_progressbar_progress, 1);
  testsfailed += runTimedTest("test_progressbar_range", test_progressbar_range, 1);
  testsfailed += runTimedTest("test_progressbar_indeterminate", test_progressbar_indeterminate, 1);
  testsfailed += runTimedTest("test_progressbar_text", test_progressbar_text, 1);
  testsfailed += runTimedTest("test_progressbar_colors", test_progressbar_colors, 1);
  testsfailed += runTimedTest("test_progressbar_orientation_direction", test_progressbar_orientation_direction, 1);
  testsfailed += runTimedTest("test_progressbar_stripes", test_progressbar_stripes, 1);
  testsfailed += runTimedTest("test_progressbar_rounded_corners", test_progressbar_rounded_corners, 1);
  testsfailed += runTimedTest("test_progressbar_callbacks", test_progressbar_callbacks, 1);
  testsfailed += runTimedTest("test_progressbar_provider", test_progressbar_provider, 1);
  testsfailed += runTimedTest("test_progressbar_pause", test_progressbar_pause, 1);
  testsfailed += runTimedTest("test_progressbar_resize", test_progressbar_resize, 1);

  testsfailed += runTimedTest("test_progressbar_attachment", test_progressbar_attachment, 200);
  testsfailed += runTimedTest("test_progressbar_progress", test_progressbar_progress, 200);
  testsfailed += runTimedTest("test_progressbar_range", test_progressbar_range, 200);
  testsfailed += runTimedTest("test_progressbar_indeterminate", test_progressbar_indeterminate, 200);
  testsfailed += runTimedTest("test_progressbar_text", test_progressbar_text, 200);
  testsfailed += runTimedTest("test_progressbar_colors", test_progressbar_colors, 200);
  testsfailed += runTimedTest("test_progressbar_orientation_direction", test_progressbar_orientation_direction, 1);
  testsfailed += runTimedTest("test_progressbar_stripes", test_progressbar_stripes, 200);
  testsfailed += runTimedTest("test_progressbar_rounded_corners", test_progressbar_rounded_corners, 200);
  testsfailed += runTimedTest("test_progressbar_callbacks", test_progressbar_callbacks, 200);
  testsfailed += runTimedTest("test_progressbar_provider", test_progressbar_provider, 200);
  testsfailed += runTimedTest("test_progressbar_pause", test_progressbar_pause, 200);
  testsfailed += runTimedTest("test_progressbar_resize", test_progressbar_resize, 200);

  D("test suite complete");
  return testsfailed;
}

