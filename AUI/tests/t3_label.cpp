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
  TEST_ASSERT_EQ(label->Text(), AUI_DEFAULT_LABEL_TEXT, 4);
  TEST_ASSERT_NE(label->EnginePtr(), nullptr, 5);
  TEST_ASSERT_EQ(label->Wnd(), w, 6);
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
  TEST_ASSERT_EQ(label->Text(), "Hello World", 2);
  label->Text("New Text");
  TEST_ASSERT_EQ(label->Text(), "New Text", 3);
  label->TextColor(0xFFFF0000U);
  TEST_ASSERT_EQ(label->TextColor(), 0xFFFF0000U, 4);
  label->FontSize(20);
  TEST_ASSERT_EQ(label->FontSize(), 20U, 5);
  label->HAlign(AUIHAlign::right);
  TEST_ASSERT_EQ(label->HAlign(), AUIHAlign::right, 6);
  label->VAlign(AUIVAlign::bottom);
  TEST_ASSERT_EQ(label->VAlign(), AUIVAlign::bottom, 7);
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
  label->Border(3);
  TEST_ASSERT_EQ(label->Border(), 3U, 2);
  label->BorderColor(0xFF00FF00U);
  label->BGColor(0xFFF1F1F1U);
  label->Move(1,1);
  TEST_ASSERT_EQ(label->BorderColor(), 0xFF00FF00U, 3);
  AWidgetReader<ALabel> r (label, 5, 5);
  TEST_ASSERT_EQ(r.Pixel(0, 0), 0, 3); //buffer clear pixel
  TEST_ASSERT_EQ(r.Pixel(1, 0), 0, 3);//buffer clear pixel
  TEST_ASSERT_EQ(r.Pixel(0, 1), 0, 3);//buffer clear pixel
  TEST_ASSERT_EQ(r.Pixel(2, 1), 0xFF00FF00U, 3); //border
  TEST_ASSERT_EQ(r.Pixel(1, 2), 0xFF00FF00U, 3); //border
  TEST_ASSERT_EQ(r.Pixel(3, 3), 0xFF00FF00U, 3); //border
  TEST_ASSERT_EQ(r.Pixel(3, 4), 0xFF00FF00U, 3); //border
  TEST_ASSERT_EQ(r.Pixel(4, 3), 0xFF00FF00U, 3); //border
  TEST_ASSERT_EQ(r.Pixel(4, 4), 0xFFF1F1F1U, 3); // widget's bgcolor
  D2("pixel at {} {} is {:x}", 0, 0, r.Pixel(0, 0))
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
  label->Resize(30, 30);
  AWidget* consumed = label->MouseClick(15, 15);
  TEST_ASSERT_EQ(consumed, nullptr, 2);
//  consumed = label->MouseClick(200, 200, true);
//  TEST_ASSERT_EQ(consumed, false, 3);
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
  label->BGColor(0xFFCCCCCC);
  label->TextColor(0xFF0000FF);
  label->HAlign(AUIHAlign::center);
  label->Border(2);
  label->BorderColor(0xFFFFFFFF);
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
  TEST_ASSERT_EQ(label->Parent(), box, 2);
  TEST_ASSERT_EQ(label->Wnd(), w, 3);
  D1("test_label_child_widget passed");
  return 0;
}

// ------------------------------------------------------------------
// ALabel: text clipping bounds verification
// ------------------------------------------------------------------
int32_t test_label_clipping_bounds(AUI* au) {
  D1("test_label_clipping_bounds start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 2);
  // 1. Create a label constrained to a small width (50px wide)
  // but give it a long string that extends far beyond 50px
  ALabel* label = ALabel::AttachTo(w, "VeryLongStringThatExceedsBounds");
  label->Move(10, 10);
  label->Resize(50, 30);
  label->TextColor(0xFF0000FFU); // Red text
  label->HAlign(AUIHAlign::left);
  // Render window buffer
  w->Draw();
  // Inspect 10px beyond the right boundary of the label (x = 10 + 50 + 10 = 70)
  // These pixels MUST remain 0 (unmodified background)
  AWidgetReader<ALabel> rRight(label, 80, 40);
  for (int32_t y = 10; y < 40; ++y) {
    for (int32_t x = 61; x < 75; ++x) {
      TEST_ASSERT_EQ(rRight.Pixel(x, y), 0U, 3); // Must not bleed past right edge
    }
  }
  // 2. Now test Right-Aligned text in a small box
  // Right alignment shifts penX left; ensure it clips at label->mX (x = 10)
  label->HAlign(AUIHAlign::right);
  w->Draw();
  // Inspect pixels to the left of the label boundary (x = 0 to 9)
  AWidgetReader<ALabel> rLeft(label, 80, 40);
  for (int32_t y = 10; y < 40; ++y) {
    for (int32_t x = 0; x < 10; ++x) {
      TEST_ASSERT_EQ(rLeft.Pixel(x, y), 0U, 4); // Must not bleed past left edge
    }
  }
  D1("test_label_clipping_bounds passed");
  return 0;
}

// ------------------------------------------------------------------
// Main: run all tests using the timed test harness
// ------------------------------------------------------------------
int main() {
  UNUSED int32_t testsfailed = 0;
//  UNUSED AUI* au = AUI::Create("test");

  testsfailed += runTimedTest(test_label_attachment, 1);
  testsfailed += runTimedTest(test_label_properties, 1);
  testsfailed += runTimedTest(test_label_border, 1);
  testsfailed += runTimedTest(test_label_click_transparent, 1);
  testsfailed += runTimedTest(test_label_draw, 1);
  testsfailed += runTimedTest(test_label_child_widget, 1);
  testsfailed += runTimedTest(test_label_clipping_bounds, 1);

  testsfailed += runTimedTest(test_label_attachment, 200);
  testsfailed += runTimedTest(test_label_properties, 200);
  testsfailed += runTimedTest(test_label_border, 200);
  testsfailed += runTimedTest(test_label_click_transparent, 200);
  testsfailed += runTimedTest(test_label_draw, 200);
  testsfailed += runTimedTest(test_label_child_widget, 200);
  testsfailed += runTimedTest(test_label_clipping_bounds, 200);

//  au->ProcessMessages();
//  delete au;

  D("test suite complete");
  return testsfailed;
}
