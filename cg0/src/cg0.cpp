#include "AUILib.h"

using namespace aui;


int main() {
  AUI* au = AUI::Create("main window");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(200, 200);
  AUIWindowType type = au->GetWindowType();
  if(type == AUIWindowType::Wayland) {
    AWindow *ww = AWindow::AttachTo(au, "additional window Wayland", AUIWindowType::Wayland);
    ww->EnableResize();
    ww->Resize(300, 300);
  }
  AWindow *wx = AWindow::AttachTo(au, "additional window X11", AUIWindowType::XCB);
  wx->Move(200, 10);
  au->ProcessMessages();
  delete au;
  return 0;
}





