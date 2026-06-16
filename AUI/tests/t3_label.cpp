#include <chrono>
#include <future>
#include <thread>
#include <memory.h>

#include "AUILib.h"
#include "ALabel.h"
#include "ABox.h"   // for child widget test

using namespace aui;

#define TEST_ASSERT(cond, errcode) do { if(!(cond)) { E("Test failed: {}", #cond); return errcode; } } while(0)

// ------------------------------------------------------------------
// ALabel: basic attachment
// ------------------------------------------------------------------
int32_t test_label_attachment() {
  D1("test_label_attachment start");
  AUI* au = AUI::Create("LabelAttachTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT(w != nullptr, 2);
  ALabel* label = ALabel::AttachTo(w);
  TEST_ASSERT(label != nullptr, 3);
  TEST_ASSERT(label->GetText().empty(), 4);
  TEST_ASSERT(label->GetEnginePtr() != nullptr, 5);
  TEST_ASSERT(label->GetParentWindow() == w, 6);
  delete au;
  D1("test_label_attachment passed");
  return 0;
}

// ------------------------------------------------------------------
// ALabel: text and property setters
// ------------------------------------------------------------------
int32_t test_label_properties() {
  D1("test_label_properties start");
  AUI* au = AUI::Create("LabelPropTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  ALabel* label = ALabel::AttachTo(w, "Hello World");
  TEST_ASSERT(label->GetText() == "Hello World", 2);
  label->SetText("New Text");
  TEST_ASSERT(label->GetText() == "New Text", 3);
  label->SetTextColor(0xFFFF0000U);
  TEST_ASSERT(label->GetTextColor() == 0xFFFF0000U, 4);
  label->SetFontSize(20);
  TEST_ASSERT(label->GetFontSize() == 20U, 5);
  label->SetHAlignment(AUIHAlign::right);
  TEST_ASSERT(label->GetHAlignment() == AUIHAlign::right, 6);
  label->SetVAlignment(AUIVAlign::bottom);
  TEST_ASSERT(label->GetVAlignment() == AUIVAlign::bottom, 7);
  delete au;
  D1("test_label_properties passed");
  return 0;
}

// ------------------------------------------------------------------
// ALabel: border thickness and color
// ------------------------------------------------------------------
int32_t test_label_border() {
  D1("test_label_border start");
  AUI* au = AUI::Create("LabelBorderTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  ALabel* label = ALabel::AttachTo(w);
  label->SetBorderThickness(3);
  TEST_ASSERT(label->GetBorderThickness() == 3U, 2);
  label->SetBorderColor(0xFF00FF00U);
  TEST_ASSERT(label->GetBorderColor() == 0xFF00FF00U, 3);
  delete au;
  D1("test_label_border passed");
  return 0;
}

// ------------------------------------------------------------------
// ALabel: click transparency (should not consume clicks)
// ------------------------------------------------------------------
int32_t test_label_click_transparent() {
  D1("test_label_click_transparent start");
  AUI* au = AUI::Create("LabelClickTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  ALabel* label = ALabel::AttachTo(w, "Click Through");
  label->Move(10, 10);
  label->Resize(100, 30);
  bool consumed = label->DispatchClick(15, 15, true);
  TEST_ASSERT(consumed == false, 2);
  consumed = label->DispatchClick(200, 200, true);
  TEST_ASSERT(consumed == false, 3);
  delete au;
  D1("test_label_click_transparent passed");
  return 0;
}

// ------------------------------------------------------------------
// ALabel: drawing does not crash
// ------------------------------------------------------------------
int32_t test_label_draw() {
  D1("test_label_draw start");
  AUI* au = AUI::Create("LabelDrawTest");
  TEST_ASSERT(au != nullptr, 1);
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
  delete au;
  D1("test_label_draw passed");
  return 0;
}

// ------------------------------------------------------------------
// ALabel: attach to a widget (not window)
// ------------------------------------------------------------------
int32_t test_label_child_widget() {
  D1("test_label_child_widget start");
  AUI* au = AUI::Create("LabelChildTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  ABox* box = ABox::AttachTo(w);
  box->Move(20, 20);
  box->Resize(160, 60);
  ALabel* label = ALabel::AttachTo(box, "Inside Box");
  label->Move(10, 10);
  label->Resize(140, 20);
  label->SetBGColor(0xFFAAAAAA);
  TEST_ASSERT(label->GetParentWidget() == box, 2);
  TEST_ASSERT(label->GetParentWindow() == w, 3);
  w->Draw();
  delete au;
  D1("test_label_child_widget passed");
  return 0;
}

// ------------------------------------------------------------------
// ALabel: text metrics update (optional, requires public UpdateTextMetrics)
// ------------------------------------------------------------------
int32_t test_label_metrics_update() {
  D1("test_label_metrics_update start");
  AUI* au = AUI::Create("LabelMetricsTest");
  TEST_ASSERT(au != nullptr, 1);
  AWindow* w = au->MainWnd();
  ALabel* label = ALabel::AttachTo(w, "Short");
  label->UpdateTextMetrics();
  int32_t w1 = label->GetCachedTextWidth();  // requires getter; if not, skip
  label->SetText("This is a much longer text that should increase width");
  label->UpdateTextMetrics();
  int32_t w2 = label->GetCachedTextWidth();
  TEST_ASSERT(w2 > w1, 2);
  delete au;
  D1("test_label_metrics_update passed");
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

  // Run all ALabel tests
  testsfailed += test_label_attachment();
  testsfailed += test_label_properties();
  testsfailed += test_label_border();
  testsfailed += test_label_click_transparent();
  testsfailed += test_label_draw();
  testsfailed += test_label_child_widget();
  testsfailed += test_label_metrics_update();

  au->ProcessMessages();
  handle.get();
  delete au;
  au = nullptr;

  return testsfailed;
}

