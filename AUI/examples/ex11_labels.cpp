#include "AUILib.h"

using namespace aui;

int32_t main() {
  AUI* au = AUI::Create("label test");
  if(!au) return 1;
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(420, 220);

// 1. Default label
  D1("before")
  ALabel* l1 = ALabel::AttachTo(w, "Default Label");
  l1->Move(10, 10);
  l1->Resize(150, 30);

// 2. Colored text (red)
  ALabel* l2 = ALabel::AttachTo(w, "Colored Text");
  l2->Move(10, 50);
  l2->Resize(150, 30);
  l2->TextColor(0xFFFF0000);

// 3. Right‑aligned
  ALabel* l3 = ALabel::AttachTo(w, "Right Aligned");
  l3->Move(10, 90);
  l3->Resize(150, 30);
  l3->HAlign(AUIHAlign::right);

// 4. With border (blue)
  ALabel* l4 = ALabel::AttachTo(w, "With Border");
  l4->Move(10, 130);
  l4->Resize(150, 30);
  l4->Border(2);
  l4->BorderColor(0xFF0000FF);

// 5. Larger font
  ALabel* l5 = ALabel::AttachTo(w, "Big Font, left");
  l5->Move(10, 170);
  l5->Resize(150, 40);
  l5->FontSize(18);
  l5->HAlign(AUIHAlign::left);

// 6. Label inside a box
  ABox* box = ABox::AttachTo(w);
  box->Move(200, 10);
  box->Resize(200, 100);
  box->BGColor(0xFFCCCCCC);
  ALabel* l6 = ALabel::AttachTo(box, "Inside Box");
  l6->Move(40, 30);
  l6->Resize(120, 30);
  l6->Border(1);
  l6->Angle(5);
  l6->TextColor(0xFF00AA00);// green

  au->ProcessMessages();
  delete au;
  return 0;
}
