#include "AUILib.h"

using namespace aui;

// ------------------------------------------------------------------
// ALabel: basic attachment
// ------------------------------------------------------------------
int32_t test_label_attachment(AUI* au) {
  D1("test_label_attachment start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 2);
  ALabel* label = ALabel::AttachTo(w);
  TEST_ASSERT_NE(label, nullptr, 3);
  TEST_ASSERT_EQ(label->GetText(), std::string(), 4);
  TEST_ASSERT_NE(label->GetEnginePtr(), nullptr, 5);
  TEST_ASSERT_EQ(label->GetParentWindow(), w, 6);
  D1("test_label_attachment passed");
  return 0;
}

// ------------------------------------------------------------------
// ALabel: text and property setters
// ------------------------------------------------------------------
int32_t test_label_properties(AUI* au) {
  D1("test_label_properties start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  ALabel* label = ALabel::AttachTo(w, "Hello World");
  TEST_ASSERT_EQ(label->GetText(), "Hello World", 2);
  label->SetText("New Text");
  TEST_ASSERT_EQ(label->GetText(), "New Text", 3);
  label->SetTextColor(0xFFFF0000U);
  TEST_ASSERT_EQ(label->GetTextColor(), 0xFFFF0000U, 4);
  label->SetFontSize(20);
  TEST_ASSERT_EQ(label->GetFontSize(), 20U, 5);
  label->SetHAlignment(AUIHAlign::right);
  TEST_ASSERT_EQ(label->GetHAlignment(), AUIHAlign::right, 6);
  label->SetVAlignment(AUIVAlign::bottom);
  TEST_ASSERT_EQ(label->GetVAlignment(), AUIVAlign::bottom, 7);
  D1("test_label_properties passed");
  return 0;
}

// ------------------------------------------------------------------
// ALabel: border thickness and color
// ------------------------------------------------------------------
int32_t test_label_border(AUI* au) {
  D1("test_label_border start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  ALabel* label = ALabel::AttachTo(w);
  label->SetBorderThickness(3);
  TEST_ASSERT_EQ(label->GetBorderThickness(), 3U, 2);
  label->SetBorderColor(0xFF00FF00U);
  TEST_ASSERT_EQ(label->GetBorderColor(), 0xFF00FF00U, 3);
  D1("test_label_border passed");
  return 0;
}

// ------------------------------------------------------------------
// ALabel: click transparency (should not consume clicks)
// ------------------------------------------------------------------
int32_t test_label_click_transparent(AUI* au) {
  D1("test_label_click_transparent start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  ALabel* label = ALabel::AttachTo(w, "Click Through");
  label->Move(10, 10);
  label->Resize(100, 30);
  bool consumed = label->DispatchClick(15, 15, true);
  TEST_ASSERT_EQ(consumed, false, 2);
  consumed = label->DispatchClick(200, 200, true);
  TEST_ASSERT_EQ(consumed, false, 3);
  D1("test_label_click_transparent passed");
  return 0;
}

// ------------------------------------------------------------------
// ALabel: drawing does not crash
// ------------------------------------------------------------------
int32_t test_label_draw(AUI* au) {
  D1("test_label_draw start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  ALabel* label = ALabel::AttachTo(w, "Draw test");
  label->Move(10, 10);
  label->Resize(150, 30);
  label->SetBGColor(0xFFCCCCCC);
  label->SetTextColor(0xFF0000FF);
  label->SetHAlignment(AUIHAlign::center);
  label->SetBorderThickness(2);
  label->SetBorderColor(0xFFFFFFFF);
  w->Draw();
  D1("test_label_draw passed");
  return 0;
}

// ------------------------------------------------------------------
// ALabel: attach to a widget (not window)
// ------------------------------------------------------------------
int32_t test_label_child_widget(AUI* au) {
  D1("test_label_child_widget start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  ABox* box = ABox::AttachTo(w);
  box->Move(20, 20);
  box->Resize(160, 60);
  ALabel* label = ALabel::AttachTo(box, "Inside Box");
  label->Move(10, 10);
  label->Resize(140, 20);
  label->SetBGColor(0xFFAAAAAA);
  TEST_ASSERT_EQ(label->GetParentWidget(), box, 2);
  TEST_ASSERT_EQ(label->GetParentWindow(), w, 3);
  w->Draw();
  D1("test_label_child_widget passed");
  return 0;
}

// ------------------------------------------------------------------
// ALabel: text metrics update (optional, requires public UpdateTextMetrics)
// ------------------------------------------------------------------
int32_t test_label_metrics_update(AUI* au) {
  D1("test_label_metrics_update start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  ALabel* label = ALabel::AttachTo(w, "Short");
  label->UpdateTextMetrics();
  int32_t w1 = label->GetCachedTextWidth();
  label->SetText("This is a much longer text that should increase width");
  label->UpdateTextMetrics();
  int32_t w2 = label->GetCachedTextWidth();
  TEST_ASSERT(w2 > w1, 2); // use TEST_ASSERT for inequality
  D1("test_label_metrics_update passed");
  return 0;
}

// ------------------------------------------------------------------
// Main: run all tests using the timed test harness
// ------------------------------------------------------------------
int main() {
  int32_t testsfailed = 0;
  testsfailed += runTimedTest("test_label_attachment", test_label_attachment, 1);
  testsfailed += runTimedTest("test_label_properties", test_label_properties, 1);
  testsfailed += runTimedTest("test_label_border", test_label_border, 1);
  testsfailed += runTimedTest("test_label_click_transparent", test_label_click_transparent, 1);
  testsfailed += runTimedTest("test_label_draw", test_label_draw, 1);
  testsfailed += runTimedTest("test_label_child_widget", test_label_child_widget, 1);
  testsfailed += runTimedTest("test_label_metrics_update", test_label_metrics_update, 1);

  testsfailed += runTimedTest("test_label_attachment", test_label_attachment, 200);
  testsfailed += runTimedTest("test_label_properties", test_label_properties, 200);
  testsfailed += runTimedTest("test_label_border", test_label_border, 200);
  testsfailed += runTimedTest("test_label_click_transparent", test_label_click_transparent, 200);
  testsfailed += runTimedTest("test_label_draw", test_label_draw, 200);
  testsfailed += runTimedTest("test_label_child_widget", test_label_child_widget, 200);
  testsfailed += runTimedTest("test_label_metrics_update", test_label_metrics_update, 200);

  D("test suite complete");
  return testsfailed;
}
