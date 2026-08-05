#include "AUILib.h"

using namespace aui;

int32_t main() {
  AUI* au = AUI::Create("ComboBox Demo");
  if (!au) return -1;
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(400, 80);

  ABox *bx = ABox::AttachTo(w);
  bx->Resize(300, 50);
  bx->Move(10, 10);
  // This will allow combobox to extend it's list out of parent's bounds
  //bx->ClipChildren(false);

  AComboBox* cb = AComboBox::AttachTo(bx);
  cb->Move(10, 10);
  cb->Text("combobox");
  cb->AddItem("Red Apple");
  cb->AddItem("Green Banana");

  au->ProcessMessages();
  delete au;
  return 0;
}

