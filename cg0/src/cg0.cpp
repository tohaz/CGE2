// This example shows different widget placement and mouse routing edge cases.
// Clicking on boxes and studying code is the only way to undestand every aspect

#include "AUILib.h"

using namespace aui;

static AWidget* OnClick(UNUSED AWidget* wid, UNUSED void* data, int32_t, int32_t) {
  UNUSED ABox* bo = static_cast<ABox*>(wid);
  bo->BGColor(GetDistinctRandomARGBColor(bo->BGColor(), 64));
  return wid;
}

UNUSED static AWidget* OnClose(UNUSED AWidget* wid, UNUSED void* data, int32_t, int32_t) {
  wid->Wnd()->EnginePtr()->ExitAUI();
  return wid;
}

UNUSED static AWidget* OnReleaseDetector(AWidget* wid, void* data, int32_t x, int32_t y) {
  D2("x {} y {}", x, y)
  OnClick(wid, data, x, y);
// Convert local (x, y) to window coordinates by walking up the parent tree
  int32_t winX = x;
  int32_t winY = y;
  for(const AWidget* curr = wid; curr != nullptr; curr = curr->Parent()) {
    winX += curr->X();
    winY += curr->Y();
  }
  D2("winX {} winY {} abs({},{})", winX, winY, wid->AbsX() + x, wid->AbsY() + y)
// Find widget under cursor at release time using clean window-relative coords
  AWidget* underCursor = wid->Wnd()->FindWidgetAt(winX, winY);
  if(underCursor == wid) {
// Release on the same widget – normal click
    D("Released on myself");
  }
  else
    if(underCursor != nullptr) {
// Released over a different widget
      D("Released over widget: '{}'", underCursor->Text());
    }
    else {
// Released on empty background
      D("Released on empty space");
    }
  return wid;
}


int32_t main() {
//  AUI* au = AUI::Create("test", AUIWindowType::X11);
//  AUI* au = AUI::Create("test", AUIWindowType::Wayland);
  AUI* au = AUI::Create("test");
  AWindow* w = au->MainWnd();
  w->BGColor(0xFF222222);   // dark background
  w->EnableResize();
  w->Resize(500,500);
  w->DisableResize();

  ABox* bol = ABox::AttachTo(w);
  bol->Move(-50,-50);
  bol->Resize(100, 100);
  bol->Border(10);
  bol->BorderColor(0xFF888800);
  bol->SetMousePressLeftCallback(OnClick, nullptr);
  bol->Text("bol");

  ABox* bol3 = ABox::AttachTo(w);
  bol3->Move(450,50);
  bol3->Resize(100, 100);
  bol3->BGColor(0xFF0000FF);
  bol3->Border(5);
  bol3->SetMousePressLeftCallback(OnClick, nullptr);
  bol3->Text("bol3");

  //Negative X position box
  ABox* bol4 = ABox::AttachTo(w);
  bol4->Move(-50,200);
  bol4->Resize(100, 100);
  bol4->BGColor(0xFFFF00FF);
  bol4->SetMousePressLeftCallback(OnClick, nullptr);
  bol4->Text("bol4");

  //left bottom corner test
  ABox* bol5 = ABox::AttachTo(w);
  bol5->Move(-50,450);
  bol5->Resize(100, 100);
  bol5->BGColor(0xFFFFFF00);
  bol5->SetMousePressLeftCallback(OnClick, nullptr);
  bol5->Text("bol5");

  //right bottom corner test
  ABox* bol6 = ABox::AttachTo(w);
  bol6->Move(450,450);
  bol6->Resize(100, 100);
  bol6->BGColor(0xFF00FFFF);
  bol6->BorderColor(0x110000FF);
  bol6->Border(10);
  bol6->SetMousePressLeftCallback(OnClick, nullptr);
  bol6->Text("bol6");

  ABox* bol7 = ABox::AttachTo(w);
  bol7->Move(50,50);
  bol7->Resize(100, 100);
  bol7->BGColor(0xFF00FFFF);
  bol7->ClipChildren(true);
  bol7->SetMousePressLeftCallback(OnClick, nullptr);
  bol7->Text("bol7");

  ABox* bol8 = ABox::AttachTo(bol7);
  bol8->Move(50,50);
  bol8->Resize(100, 100);
  bol8->BGColor(0xFFFF00FF);
  bol8->SetMousePressLeftCallback(OnClick, nullptr);
  bol8->Text("bol8");

  ABox* bol9 = ABox::AttachTo(bol7);
  bol9->Move(-50, -50);
  bol9->Resize(100, 100);
  bol9->BGColor(0xFF7700FF);
  bol9->SetMousePressLeftCallback(OnClick, nullptr);
  bol9->Text("bol9");

  ABox* bol10 = ABox::AttachTo(w);
  bol10->Move(250,100);
  bol10->Resize(100, 100);
  bol10->BGColor(0xFFFF77FF);
  bol10->ClipChildren(false);
  bol10->ClipChildrenHitbox(false);
  bol10->SetMousePressLeftCallback(OnClick, nullptr);
  bol10->Text("bol10");

  ABox* bol11 = ABox::AttachTo(bol10);
  bol11->Move(-50, -50);
  bol11->Resize(100, 100);
  bol11->BGColor(0xFF7700FF);
  bol11->SetMousePressLeftCallback(OnClick, nullptr);
  bol11->Text("bol11");

  ABox* bol12 = ABox::AttachTo(bol10);
  bol12->Move(50,50);
  bol12->Resize(100, 100);
  bol12->BGColor(0xFFFF7777);
  bol12->SetMousePressLeftCallback(OnClick, nullptr);
  bol12->Text("bol12");

  ABox* bol13 = ABox::AttachTo(bol12);
  bol13->Move(10,10);
  bol13->Resize(100, 100);
  bol13->BGColor(0xFF777777);
  bol13->SetMousePressLeftCallback(OnClick, nullptr);
  bol13->Text("bol13");

  ABox* bol14 = ABox::AttachTo(w);
  bol14->Move(100,200);
  bol14->Text("parent bol14");
  bol14->Resize(100, 100);
  bol14->BGColor(0xFFFFFFFF);
  bol14->Angle(10);
  bol14->Border(0);
  bol14->SetMousePressLeftCallback(OnClick, nullptr);

  ABox* bol15 = ABox::AttachTo(bol14);
  bol15->Move(10,10);
  bol15->Resize(25, 25);
  bol15->BGColor(0xFFFF0000);
  bol15->Border(1);
  bol15->SetMousePressLeftCallback(OnClick, nullptr);
  bol15->Text("bol15");

  ABox* bol16 = ABox::AttachTo(bol14);
  bol16->Move(40,00);
  bol16->Resize(45, 45);
  bol16->Text("child bol16");
  bol16->BGColor(0xFF00FF00);
  bol16->Border(5);
  bol16->Angle(35);
  bol16->SetMousePressLeftCallback(OnClick, nullptr);

  ABox* bx1 = ABox::AttachTo(w);
  bx1->Move(50, 50);
  bx1->Resize(100, 100);
  bx1->Angle(30);
  bx1->Text("parent bx1");
  bx1->SetMousePressLeftCallback(OnClick, nullptr);

  ABox* bx2 = ABox::AttachTo(bx1);
  bx2->Move(15, 15);
  bx2->Resize(70, 70);
  bx2->Angle(30);
  bx2->Text("child bx2");
  bx2->BGColor(0xFFFFFF00);
  bx2->Border(5);
  bx2->SetMousePressLeftCallback(OnClick, nullptr);

  ABox* b = ABox::AttachTo(w);
  b->Move(230, 280);
  b->Resize(200, 200);
  b->Text("box b");
  b->SetMousePressLeftCallback(OnClick, nullptr);
  b->SetMouseReleaseLeftCallback(OnClick, nullptr);
  b->MouseLeftReleaseRequired(true);

  ABox* b2 = ABox::AttachTo(b);
  b2->Resize(150, 150);
  b2->Text("box b2");
  b2->Move(25, 25);
  b2->BGColor(0x80FF0000);
  b2->SetMousePressLeftCallback(OnClick, nullptr);
  b2->SetMouseReleaseLeftCallback(OnClick, nullptr);
  b2->MouseLeftReleaseRequired(false);

  ABox* b3 = ABox::AttachTo(b2);
  b3->Resize(100, 100);
  b3->Text("box b3");
  b3->Move(25, 25);
  b3->BGColor(0xFF0000FF);
  b3->SetMousePressLeftCallback(OnClick, nullptr);
  b3->SetMousePressLeftCallback(OnClick, nullptr);
  // this illustrates how drag and drop can be implemented. Another method would be overriding
  // window's mouse release callback.
  b3->SetMouseReleaseLeftCallback(OnReleaseDetector, nullptr);
  //
  b3->MouseLeftReleaseRequired(true);
  ALabel* ld = ALabel::AttachTo(b3, "Detector");
  ld->BGColor(0xFFFFFFFF);
  ld->Move(0, 0);
  ld->Border(0);
  ld->TrimToText();
  ld->HAlign(AUIHAlign::left);
  ld->VAlign(AUIVAlign::top);
  ld->Resize(ld->SizeX(), ld->SizeY() + 3);

  ABox* b4c = ABox::AttachTo(b3);
  b4c->Resize(20, 50);
  b4c->Text("box4consuming");
  b4c->Move(25, 25);
  b4c->BGColor(0xFFFF0000);
  b4c->ConsumeMouseEvents(true);
//   Intentionally not adding callback. This box is a 'shield'. it just consumes event
//  b4c->SetMousePressLeftCallback(OnClick, nullptr);

  ABox* b4n = ABox::AttachTo(b3);
  b4n->Resize(20, 50);
  b4n->Text("box4 not consuming");
  b4n->Move(55, 25);
  b4n->BGColor(0xFF00FF00);
  b4n->ConsumeMouseEvents(false);
  // Intentionally not adding callback. This box is transparent to mouse events, default behaviour

  AButton* bt = AButton::AttachTo(w);
  bt->Text("Close");
  bt->Move(400, 10);
  bt->SetMouseClickCallback(OnClose, bt);

  ALabel* lt = ALabel::AttachTo(w, "Transparent");
  lt->BGColor(0xFFFFFFFF);
  lt->Move(300, 380);
  lt->Border(0);
  lt->FontSize(10);
  lt->HAlign(AUIHAlign::left);
  lt->VAlign(AUIVAlign::top);
  lt->TrimToText();
  lt->Angle(90);

  ALabel* ls = ALabel::AttachTo(w, "Shield");
  ls->BGColor(0xFFFFFFFF);
  ls->Move(290, 375);
  ls->Border(0);
  ls->FontSize(10);
  ls->HAlign(AUIHAlign::left);
  ls->VAlign(AUIVAlign::top);
  ls->TrimToText();
  ls->Angle(90);

  au->ProcessMessages();

  delete au;
  return 0;
}



