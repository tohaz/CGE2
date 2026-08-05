#include "AUILib.h"

using namespace aui;

int32_t main() {
  AUI* au = AUI::Create("main window");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(400, 300);
  w->Decorations(true);
  ABox* bo = ABox::AttachTo(w);
  bo->BGColor(0xFF00FFFF);
  bo->Move(0,0);
  ABox* bo2 = ABox::AttachTo(bo);
  bo2->BGColor(0xFFFFFF00);
  ABox* bo3 = ABox::AttachTo(w);
  bo3->Move(290,0);
  bo3->ClipChildren(false);
  ABox* bo4 = ABox::AttachTo(bo3);
  bo4->BGColor(0xFFFF00FF);
  ABox* bo5 = ABox::AttachTo(w);
  bo5->BGColor(0xFF0000FF);
  bo5->Move(50,50);
  bo5->Resize(300,25);

  AWindow *w2 = AWindow::AttachTo(au, "w2");
  w2->EnableResize();
  w2->Resize(600, 400);
  w2->Decorations(false);
  ABox* bo6 = ABox::AttachTo(w2);
  bo6->Move(20, 20);
//  UNUSED AWindow *w3 = AWindow::AttachTo(au, "w3");
  ALabel* l = ALabel::AttachTo(w2, "those are two windows, you can drag them apart");
  l->HAlign(AUIHAlign::left);
  l->Move(10, 150);
  l->Resize(390, 40);

  au->ProcessMessages();
  delete au;
  D1("ends")
  return 0;
}
