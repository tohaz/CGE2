#include "AUILib.h"

using namespace aui;
// ------------------------------------------------------------------
// Helper: wait a few milliseconds for the background thread to update
// ------------------------------------------------------------------
static void WaitForThread(uint32_t ms = 50) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
// ------------------------------------------------------------------
// Attachment and basic properties
// ------------------------------------------------------------------
int32_t test_progressbar_attachment() {
  D1("test_progressbar_attachment start");
  AUI *au = AUI::Create("ProgressBarAttachTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  AProgressBar *pb = AProgressBar::AttachTo(win);
  TEST_ASSERT(pb != nullptr, 2);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 0.0, 3);
  TEST_ASSERT(pb->IsIndeterminate() == false, 4);
  TEST_ASSERT(pb->IsTextVisible() == true, 5);
  TEST_ASSERT(pb->HasRoundedCorners() == false, 6);
  TEST_ASSERT(pb->IsStripeEnabled() == false, 7);
  delete au;
  D1("test_progressbar_attachment passed");
  return 0;
}

// ------------------------------------------------------------------
// Progress control (SetProgress, GetProgress, Clear)
// ------------------------------------------------------------------
int32_t test_progressbar_progress() {
  D1("test_progressbar_progress start");
  AUI *au = AUI::Create("ProgressBarProgressTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  AProgressBar *pb = AProgressBar::AttachTo(win);
  pb->SetProgress(0.5);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 0.5, 2);
  pb->SetProgress(1.2);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 1.0, 3);
  pb->SetProgress(-0.1);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 0.0, 4);
  pb->SetProgress(0.75);
  pb->Clear();
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 0.0, 5);
  delete au;
  D1("test_progressbar_progress passed");
  return 0;
}

// ------------------------------------------------------------------
// Range
// ------------------------------------------------------------------
int32_t test_progressbar_range() {
  D1("test_progressbar_range start");
  AUI *au = AUI::Create("ProgressBarRangeTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  AProgressBar *pb = AProgressBar::AttachTo(win);
  pb->SetRange(10.0, 20.0);
  TEST_ASSERT_DOUBLE_EQ(pb->GetMin(), 10.0, 2);
  TEST_ASSERT_DOUBLE_EQ(pb->GetMax(), 20.0, 3);
  pb->SetProgress(15.0);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 0.5, 4);
  pb->SetProgress(5.0);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 0.0, 5);
  pb->SetProgress(25.0);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 1.0, 6);
  delete au;
  D1("test_progressbar_range passed");
  return 0;
}

// ------------------------------------------------------------------
// Indeterminate mode
// ------------------------------------------------------------------
int32_t test_progressbar_indeterminate() {
  D1("test_progressbar_indeterminate start");
  AUI *au = AUI::Create("ProgressBarIndeterminateTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  AProgressBar *pb = AProgressBar::AttachTo(win);
  pb->SetIndeterminate(true);
  TEST_ASSERT(pb->IsIndeterminate() == true, 2);
  pb->SetProgress(0.5);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), 0.0, 3);
  pb->SetIndeterminate(false);
  TEST_ASSERT(pb->IsIndeterminate() == false, 4);
  delete au;
  D1("test_progressbar_indeterminate passed");
  return 0;
}

// ------------------------------------------------------------------
// Text visibility and formatting
// ------------------------------------------------------------------
int32_t test_progressbar_text() {
  D1("test_progressbar_text start");
  AUI *au = AUI::Create("ProgressBarTextTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  AProgressBar *pb = AProgressBar::AttachTo(win);
  pb->SetShowText(false);
  TEST_ASSERT(pb->IsTextVisible() == false, 2);
  pb->SetShowText(true);
  TEST_ASSERT(pb->IsTextVisible() == true, 3);
  pb->SetTextFormat("%.2f%%");
  TEST_ASSERT_EQ(pb->GetTextFormat(), std::string("%.2f%%"), 4);
  delete au;
  D1("test_progressbar_text passed");
  return 0;
}

// ------------------------------------------------------------------
// Colors and gradient
// ------------------------------------------------------------------
int32_t test_progressbar_colors() {
  D1("test_progressbar_colors start");
  AUI *au = AUI::Create("ProgressBarColorsTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  AProgressBar *pb = AProgressBar::AttachTo(win);
  pb->SetBarColor(0xFF00FF00);
  TEST_ASSERT_EQ(pb->GetBarColor(), 0xFF00FF00U, 2);
  pb->SetBarColor2(0xFFFF0000);
  TEST_ASSERT_EQ(pb->GetBarColor2(), 0xFFFF0000U, 3);
  pb->SetBarColor2(0);
  TEST_ASSERT_EQ(pb->GetBarColor2(), 0U, 4);
  delete au;
  D1("test_progressbar_colors passed");
  return 0;
}

// ------------------------------------------------------------------
// Orientation and direction
// ------------------------------------------------------------------
int32_t test_progressbar_orientation_direction() {
  D1("test_progressbar_orientation_direction start");
  AUI *au = AUI::Create("ProgressBarOrientTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  AProgressBar *pb = AProgressBar::AttachTo(win);
  pb->SetOrientation(AUIOrientation::vertical);
  TEST_ASSERT(pb->GetOrientation() == AUIOrientation::vertical, 2);
  pb->SetDirection(AUIDirection::bottom);
  TEST_ASSERT(pb->GetDirection() == AUIDirection::bottom, 3);
  pb->SetOrientation(AUIOrientation::horizontal);
  TEST_ASSERT(pb->GetOrientation() == AUIOrientation::horizontal, 4);
  delete au;
  D1("test_progressbar_orientation_direction passed");
  return 0;
}

// ------------------------------------------------------------------
// Stripes
// ------------------------------------------------------------------
int32_t test_progressbar_stripes() {
  D1("test_progressbar_stripes start");
  AUI *au = AUI::Create("ProgressBarStripesTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  AProgressBar *pb = AProgressBar::AttachTo(win);
  pb->SetStripe(true);
  TEST_ASSERT(pb->IsStripeEnabled() == true, 2);
  pb->SetStripeColor(0x40FF00FF);
  TEST_ASSERT_EQ(pb->GetStripeColor(), 0x40FF00FFU, 3);
  pb->SetStripeWidth(10);
  TEST_ASSERT_EQ(pb->GetStripeWidth(), 10U, 4);
  pb->SetStripeSpeed(5);
  TEST_ASSERT_EQ(pb->GetStripeSpeed(), 5, 5);
  pb->SetStripe(false);
  TEST_ASSERT(pb->IsStripeEnabled() == false, 6);
  delete au;
  D1("test_progressbar_stripes passed");
  return 0;
}

// ------------------------------------------------------------------
// Rounded corners
// ------------------------------------------------------------------
int32_t test_progressbar_rounded_corners() {
  D1("test_progressbar_rounded_corners start");
  AUI *au = AUI::Create("ProgressBarRoundedTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  AProgressBar *pb = AProgressBar::AttachTo(win);
  pb->SetRoundedCorners(true, 12);
  TEST_ASSERT(pb->HasRoundedCorners() == true, 2);
  TEST_ASSERT_EQ(pb->GetCornerRadius(), 12U, 3);
  pb->SetRoundedCorners(false);
  TEST_ASSERT(pb->HasRoundedCorners() == false, 4);
  delete au;
  D1("test_progressbar_rounded_corners passed");
  return 0;
}

// ------------------------------------------------------------------
// Callbacks
// ------------------------------------------------------------------
int32_t test_progressbar_callbacks() {
  D1("test_progressbar_callbacks start");
  AUI *au = AUI::Create("ProgressBarCallbacksTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  AProgressBar *pb = AProgressBar::AttachTo(win);
  bool startCalled = false;
  bool changedCalled = false;
  bool completeCalled = false;
  pb->SetOnStart([&](double) {
    startCalled = true;
  });
  pb->SetOnProgressChanged([&](double) {
    changedCalled = true;
  });
  pb->SetOnComplete([&](double) {
    completeCalled = true;
  });
  pb->SetProgress(0.0);
  pb->SetProgress(0.5);
  pb->SetProgress(1.0);
  TEST_ASSERT(startCalled, 2);
  TEST_ASSERT(changedCalled, 3);
  TEST_ASSERT(completeCalled, 4);
  delete au;
  D1("test_progressbar_callbacks passed");
  return 0;
}

// ------------------------------------------------------------------
// Progress provider (background thread)
// ------------------------------------------------------------------
int32_t test_progressbar_provider() {
  D1("test_progressbar_provider start");
  AUI *au = AUI::Create("ProgressBarProviderTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  AProgressBar *pb = AProgressBar::AttachTo(win);
  pb->SetUpdateInterval(10);
  pb->SetProgress(0.0);
  pb->SetProgressProvider([]() noexcept {
    static double p = 0.0;
    p += 0.1;
    if(p > 1.0)
      p = 0.0;
    return p;
  });
  WaitForThread(30);
  double val = pb->GetProgress();
  TEST_ASSERT(val > 0.0, 2);
  pb->SetProgressProvider(nullptr);
  double oldVal = pb->GetProgress();
  WaitForThread(50);
  TEST_ASSERT_DOUBLE_EQ(pb->GetProgress(), oldVal, 3);
  delete au;
  D1("test_progressbar_provider passed");
  return 0;
}

// ------------------------------------------------------------------
// Pause/Resume
// ------------------------------------------------------------------
int32_t test_progressbar_pause() {
  D1("test_progressbar_pause start");
  AUI *au = AUI::Create("ProgressBarPauseTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  AProgressBar *pb = AProgressBar::AttachTo(win);
  pb->SetUpdateInterval(10);
  pb->SetProgress(0.0);
  pb->SetProgressProvider([]() noexcept {
    static double p = 0.0;
    p += 0.1;
    if(p > 1.0)
      p = 0.0;
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
  TEST_ASSERT(val3 > val2, 3);
  (void) val1;
  delete au;
  D1("test_progressbar_pause passed");
  return 0;
}

// ------------------------------------------------------------------
// Resize (should not crash)
// ------------------------------------------------------------------
int32_t test_progressbar_resize() {
  D1("test_progressbar_resize start");
  AUI *au = AUI::Create("ProgressBarResizeTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow *win = au->MainWnd();
  AProgressBar *pb = AProgressBar::AttachTo(win);
  pb->Resize(100, 50);
  pb->Move(10, 20);
  TEST_ASSERT(true, 2);
  delete au;
  D1("test_progressbar_resize passed");
  return 0;
}

// ------------------------------------------------------------------
// main
// ------------------------------------------------------------------
int main() {
  uint32_t delay_ms = 50;
  int32_t testsfailed = 0;
  AUI *au = AUI::Create("aui progressbar test");
  UNUSED AWindow *w = au->MainWnd();

  auto handle = std::async(std::launch::async, [=]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    au->ExitAUI();
  });

  testsfailed += test_progressbar_attachment();
  testsfailed += test_progressbar_progress();
  testsfailed += test_progressbar_range();
  testsfailed += test_progressbar_indeterminate();
  testsfailed += test_progressbar_text();
  testsfailed += test_progressbar_colors();
  testsfailed += test_progressbar_orientation_direction();
  testsfailed += test_progressbar_stripes();
  testsfailed += test_progressbar_rounded_corners();
  testsfailed += test_progressbar_callbacks();
  testsfailed += test_progressbar_provider();
  testsfailed += test_progressbar_pause();
  testsfailed += test_progressbar_resize();

  au->ProcessMessages();
  handle.get();
  delete au;
  au = nullptr;

  return testsfailed;
}

