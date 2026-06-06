#include "AUILib.h"
#include "AWindow.h"
#include "ABox.h"
#include "AScrollBar.h"

using namespace aui;

int32_t main() {
  AUI* au = AUI::Create("ScrollBar Arrows Test");
  if(!au) return 1;
  AWindow* w = au->MainWnd();
  w->SetBGColor(0xFF222222);
  w->EnableResize();
  w->Resize(600, 400);

  // Create a wide box to scroll horizontally
  ABox* box = ABox::AttachTo(w);
  box->Move(20, 20);
  box->Resize(800, 100);   // wider than window
  box->SetBGColor(0xFF8844CC);
  box->SetBorderThickness(2);
  box->SetBorderColor(0xFFFFFFFF);

  // Horizontal scrollbar at bottom
  AScrollBar* hScroll = AScrollBar::AttachTo(w, AUIOrientation::horizontal);
  hScroll->Move(20, 360);
  hScroll->Resize(560, 20);
  hScroll->SetRange(0, 600);   // 800 - (window client width) ~ 600
  hScroll->SetPageStep(100);
  hScroll->SetSingleStep(20);
  hScroll->ShowArrows(true);
  hScroll->SetArrowSize(16);
  hScroll->SetTrackThickness(6);
  hScroll->SetThumbThickness(10);
  hScroll->SetScrollCallback([](AWindow*, AWidget*, void* data, int32_t val) {
    ABox* box2 = static_cast<ABox*>(data);
    box2->Move(20 - val, 20);   // move box left as scroll increases
  }, box);

  au->ProcessMessages();
  delete au;
  return 0;
}

