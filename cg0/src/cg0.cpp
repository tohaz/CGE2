#include "AUILib.h"

using namespace aui;

int32_t main() {
  AUI* au = AUI::Create("cg0");
  UNUSED AWindow* w = au->MainWnd();
  w->BGColor(0xFF222222);   // dark background
  w->EnableResize();
  w->Resize(500, 400);
  w->DisableResize();

  au->ProcessMessages();
  delete au;
  return 0;
}


