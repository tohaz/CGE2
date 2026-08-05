#include "AUILib.h"

using namespace aui;

static AWidget* OnClick(AWidget* wi, void*, int32_t, int32_t) {
  D1("callback fired")
  return wi;
}

int32_t main() {
  AUI* au = AUI::Create("Table Demo2", AUIWindowType::X11);
  //  AUI* au = AUI::Create("Table Demo2", AUIWindowType::Wayland);
  AWindow *w = au->MainWnd();
  w->EnableResize();
  w->Resize(200, 50);
  w->BGColor(0xFF55FFFF);

  AButton* bt = AButton::AttachTo(w, "Pressed");
  bt->Move(10, 10);
  bt->Pressed(true);
  bt->Angle(6);
  bt->SetMouseClickCallback(OnClick, nullptr);

  AButton* btx = AButton::AttachTo(w, "Released");
  btx->Move(110, 10);
  btx->Pressed(false);
  btx->Angle(-8);
  btx->SetMouseClickCallback(OnClick, nullptr);
  bt->SetMousePressLeftCallback(OnClick, nullptr);

  au->ProcessMessages();
  delete au;
  return 0;
}
