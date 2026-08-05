#include "AUILib.h"

using namespace aui;

static AWidget* OnClick(UNUSED AWidget* wid, UNUSED void* data, int32_t, int32_t) {
  UNUSED ABox* bo = static_cast<ABox*>(wid);
  bo->BGColor(GetDistinctRandomARGBColor(bo->BGColor(), 64));
  return wid;
}


int32_t main() {
  AUI* au = AUI::Create("ComboBox Demo");
  if (!au) return -1;
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(400, 300);
  w->ClipChildrenHitbox(true);

  UNUSED ABox* bx2 = ABox::AttachTo(w);
  bx2->Resize(150, 150);
  bx2->Angle(0);
  bx2->SetMousePressLeftCallback(OnClick, nullptr);
  bx2->Text("bx2");

  ABox* bx3 = ABox::AttachTo(bx2);
  bx3->BGColor(0xFFFF0000);
//  bx3->ConsumeMouseEvents(true);
  bx3->Resize(100, 100);
  bx3->Move(30, 30);
  bx3->SetMousePressLeftCallback(OnClick, nullptr);
  bx3->Text("bx3");

  ABox* bx4 = ABox::AttachTo(bx3);
  bx4->BGColor(0xFF0000FF);
  bx4->Resize(250, 250);
  bx4->Move(30, 30);
  bx4->SetMousePressLeftCallback(OnClick, nullptr);
  bx4->Text("bx4");

  ABox* bx5 = ABox::AttachTo(w);
  bx5->Resize(100, 100);
  bx5->Move(250, 50);
  bx5->ClipChildren(false);
  bx5->ClipChildrenHitbox(false);
  bx5->SetMousePressLeftCallback(OnClick, nullptr);
  bx5->Text("bx5");

  ABox* bx6 = ABox::AttachTo(bx5);
  bx6->Resize(100, 100);
  bx6->BGColor(0xFF0000FF);
  bx6->Move(50, 50);
  bx6->SetMousePressLeftCallback(OnClick, nullptr);
  bx6->Text("bx6");

  ABox* bx7 = ABox::AttachTo(bx5);
  bx7->Resize(100, 100);
  bx7->BGColor(0xFFFF00FF);
  bx7->Move(-50, -50);
  bx7->SetMousePressLeftCallback(OnClick, nullptr);
  bx7->Text("bx7");

  au->ProcessMessages();
  delete au;
  return 0;
}
