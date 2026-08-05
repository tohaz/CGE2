
#include "AUILib.h"

using namespace aui;

// Helper struct to keep both scroll values and update the label
struct ScrollValues {
  int32_t vertical = 0;
  int32_t horizontal = 0;
  ALabel* label = nullptr;
};

int32_t main() {
  // Create the UI engine and main window
  AUI* au = AUI::Create("ScrollBar Test");
  if(!au) return 1;
  AWindow* w = au->MainWnd();
  w->BGColor(0xFF222222);   // dark background
  w->EnableResize();
  w->Resize(800, 600);          // desired window size
  w->DisableResize();
  // Create a label to display the current scroll values
  ALabel* valueLabel = ALabel::AttachTo(w, "Vertical: 0  Horizontal: 0");
  valueLabel->Move(10, 10);
  valueLabel->Resize(780, 30);
  valueLabel->BGColor(0xFF444444);
  valueLabel->TextColor(0xFFFFFFFF);
  valueLabel->HAlign(AUIHAlign::center);
  // ----- Vertical ScrollBar (right side) -----
  AScrollBar* vScroll = AScrollBar::AttachTo(w, AUIOrientation::vertical);
  vScroll->Move(784, 40);               // right edge, 40px from top
  vScroll->Resize(16, 520);             // 800-600? Actually 600 - 40(top) - 40(bottom) = 520
  vScroll->Range(0, 100);
  vScroll->PageStep(20);
  vScroll->Value(50);
  vScroll->TrackThick(4);
  vScroll->ThumbThick(8);
  vScroll->TrackColor(0xFF666666);
  vScroll->ThumbColor(0xFFAAAAAA);
  vScroll->Orient(AUIOrientation::vertical);
  // ----- Horizontal ScrollBar (bottom) -----
  AScrollBar* hScroll = AScrollBar::AttachTo(w, AUIOrientation::horizontal);
  hScroll->Move(40, 584);               // 600 - 16 = 584, 40px from left
  hScroll->Resize(720, 16);             // 800 - 40(left) - 40(right) = 720
  hScroll->Range(0, 200);
  hScroll->PageStep(40);
  hScroll->Value(100);
  hScroll->TrackThick(4);
  hScroll->ThumbThick(8);
  hScroll->TrackColor(0xFF666666);
  hScroll->ThumbColor(0xFFAAAAAA);
  // Shared data for callbacks
  ScrollValues* vals = new ScrollValues;
  vals->label = valueLabel;
  vals->vertical = 50;
  vals->horizontal = 100;
  // Set callbacks to update the label
  vScroll->SetScrollCallback([](AWidget*, void* data, int32_t val) {
    ScrollValues* sv = static_cast<ScrollValues*>(data);
    sv->vertical = val;
    std::string newText = "Vertical: " + std::to_string(sv->vertical) +
                          "  Horizontal: " + std::to_string(sv->horizontal);
    sv->label->Text(newText);
  }, vals);
  hScroll->SetScrollCallback([](AWidget*, void* data, int32_t val) {
    ScrollValues* sv = static_cast<ScrollValues*>(data);
    sv->horizontal = val;
    std::string newText = "Vertical: " + std::to_string(sv->vertical) +
                          "  Horizontal: " + std::to_string(sv->horizontal);
    sv->label->Text(newText);
  }, vals);
  // Start the event loop
  au->ProcessMessages();
  // Cleanup
  delete vals;
  delete au;
  return 0;
}

