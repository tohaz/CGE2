#include "AUILib.h"

using namespace aui;

static AWidget* OnClick(AWidget* wi, void*, int32_t, int32_t) {
  D("callback fired")
  return wi;
}

int32_t main() {
  AUI* au = AUI::Create("Simple Buttons Demo");
//  AUI* au = AUI::Create("Simple Buttons Demo", AUIWindowType::X11);
  //  AUI* au = AUI::Create("Simple Buttons Demo", AUIWindowType::Wayland);
  AWindow *w = au->MainWnd();
  w->EnableResize();
  w->Resize(200, 50);
  w->BGColor(0xFF55FFFF);

  AButton* bt = AButton::AttachTo(w, "Pressed");
  bt->Move(10, 10);
  bt->Pressed(true);
  bt->Angle(6);
  // Different callbacks for buttons. This one is click callback
  bt->SetMouseClickCallback(OnClick, nullptr);

  AButton* btx = AButton::AttachTo(w, "Released");
  btx->Move(110, 10);
  btx->Pressed(false);
  btx->Angle(-8);
  btx->SetMousePressLeftCallback(OnClick, nullptr);

  au->ProcessMessages();
  delete au;
  return 0;
}
