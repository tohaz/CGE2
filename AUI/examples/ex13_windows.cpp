#include "AUILib.h"

using namespace aui;

static AWidget* OnClick(AWidget* wi, void* data, int32_t, int32_t) {
  D1("callback fired")
  AWindow* w2 = (AWindow*)data;
  if(w2->Visible()) {
    wi->Text("Show");
    w2->Hide();
  }
  else {
    wi->Text("Hide");
    w2->Show();
  }
  return wi;
}

int32_t main() {
//  AUI* au = AUI::Create("test", AUIWindowType::X11);
//  AUI* au = AUI::Create("test", AUIWindowType::Wayland);
  AUI* au = AUI::Create("test");
  AWindow* w = au->MainWnd();
  w->BGColor(0xFF222222);   // dark background

  UNUSED AWindow* w2 = AWindow::AttachTo(au, "w2");

  AButton *bn = AButton::AttachTo(w);
  bn->Text("Hide");
  bn->SetMouseClickCallback(OnClick, w2);
  w2->Decorations(false);

  AButton *bn2 = AButton::AttachTo(w2);
  bn2->Text("Hide");
  bn2->SetMouseClickCallback(OnClick, w);

  au->ProcessMessages();

  delete au;
  return 0;
}
