#include "AUILib.h"

using namespace aui;

int32_t main() {
  AUI* au = AUI::Create("example");
  if(!au)
    return -1;
  AWindow* win = au->MainWnd();
  win->EnableResize();
  win->Resize(400, 200);
  ALabel* label = ALabel::AttachTo(win);
  label->Move(50, 50);
  label->Resize(300, 60);
  label->Text("Hello 😀 World! 🎉");
  label->FontSize(24);
  label->BGColor(0xFFEEEEEE);
  label->Border(1);
  au->ProcessMessages();
  delete au;
  return 0;
}
