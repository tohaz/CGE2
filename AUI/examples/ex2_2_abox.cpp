#include "AUILib.h"

using namespace aui;

int main() {
// 1. Create AUI and window
  AUI* au = AUI::Create("Rotated box test");
  AWindow* win = au->MainWnd();
  win->EnableResize();
  win->Resize(1000, 1000);
  win->DisableResize();
  win->BGColor(0xFF222222);
 // Case 1 rotated boxex inside each other
  ABox* bx = ABox::AttachTo(win);
  bx->Move(20,60);
  bx->Resize(500, 500);
  bx->Border(10);
  bx->Angle(30);
  ABox* bx2 = ABox::AttachTo(bx);
  bx2->Move(120,20);
  bx2->Resize(300, 300);
  bx2->Border(10);
  bx2->Angle(30);
  bx2->BGColor(0xFF2222FF);

  ABox* bx3 = ABox::AttachTo(win);
  bx3->Move(620,20);
  bx3->Resize(100, 100);
  bx3->Border(10);
  bx3->Angle(0);
  bx3->BGColor(0xFF2222FF);
  bx3->ClipChildren(false);
  ABox* bx4 = ABox::AttachTo(bx3);
  bx4->Move(-50,-50);
  bx4->Resize(100, 100);
  bx4->Border(10);
  bx4->Angle(0);
  bx4->BGColor(0xFFFF22FF);

  au->ProcessMessages();

  delete au;
  return 0;
}
