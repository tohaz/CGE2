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
  w->SetBGColor(0xFF222222);   // dark background
  w->EnableResize();
  w->Resize(800, 600);          // desired window size
  w->DisableResize();

  // Create a label to display the current scroll values
  ALabel* valueLabel = ALabel::AttachTo(w, "Vertical: 0  Horizontal: 0", 10, 10, 780, 30);
  valueLabel->SetBGColor(0xFF444444);
  valueLabel->SetTextColor(0xFFFFFFFF);
  valueLabel->SetHAlignment(AUIHAlign::center);

  // ----- Vertical ScrollBar (right side) -----
  AScrollBar* vScroll = AScrollBar::AttachTo(w, AUIOrientation::vertical);
  vScroll->Move(784, 40);               // right edge, 40px from top
  vScroll->Resize(16, 520);             // 800-600? Actually 600 - 40(top) - 40(bottom) = 520
  vScroll->SetRange(0, 100);
  vScroll->SetPageStep(20);
  vScroll->SetValue(50);
  vScroll->SetTrackThickness(4);
  vScroll->SetThumbThickness(8);
  vScroll->SetTrackColor(0xFF666666);
  vScroll->SetThumbColor(0xFFAAAAAA);

  // ----- Horizontal ScrollBar (bottom) -----
  AScrollBar* hScroll = AScrollBar::AttachTo(w, AUIOrientation::horizontal);
  hScroll->Move(40, 584);               // 600 - 16 = 584, 40px from left
  hScroll->Resize(720, 16);             // 800 - 40(left) - 40(right) = 720
  hScroll->SetRange(0, 200);
  hScroll->SetPageStep(40);
  hScroll->SetValue(100);
  hScroll->SetTrackThickness(4);
  hScroll->SetThumbThickness(8);
  hScroll->SetTrackColor(0xFF666666);
  hScroll->SetThumbColor(0xFFAAAAAA);

  // Shared data for callbacks
  ScrollValues* vals = new ScrollValues;
  vals->label = valueLabel;
  vals->vertical = 50;
  vals->horizontal = 100;

  // Set callbacks to update the label
  vScroll->SetScrollCallback([](AWindow*, AWidget*, void* data, int32_t val) {
    ScrollValues* sv = static_cast<ScrollValues*>(data);
    sv->vertical = val;
    std::string newText = "Vertical: " + std::to_string(sv->vertical) +
                          "  Horizontal: " + std::to_string(sv->horizontal);
    sv->label->SetText(newText);
  }, vals);

  hScroll->SetScrollCallback([](AWindow*, AWidget*, void* data, int32_t val) {
    ScrollValues* sv = static_cast<ScrollValues*>(data);
    sv->horizontal = val;
    std::string newText = "Vertical: " + std::to_string(sv->vertical) +
                          "  Horizontal: " + std::to_string(sv->horizontal);
    sv->label->SetText(newText);
  }, vals);

  // Force an initial draw to show everything immediately
  au->Draw();

  // Start the event loop
  au->ProcessMessages();

  // Cleanup
  delete vals;
  delete au;
  return 0;
}
