#include "AUILib.h"

using namespace aui;

int32_t main() {
  // 1. Create UI engine and main window
  AUI* au = AUI::Create("ComboBox Visibility Demo");
  if (!au) return -1;
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(500, 150);
  // 2. Primary combobox (fruits)
  AComboBox* cb1 = AComboBox::AttachTo(w);
  cb1->Text("Some");
  cb1->AddItem("item 1_1");
  cb1->AddItem("item 1_2");
  cb1->AddItem("item 1_3");
  cb1->Move(10, 10);
  AComboBox* cb2 = AComboBox::AttachTo(w);
  cb2->Text("Combo");
  cb2->AddItem("item 2_1");
  cb2->AddItem("item 2_2");
  cb2->AddItem("item 2_3");
  cb2->Move(180, 10);

  ABox* bx3 = ABox::AttachTo(w);
  bx3->Move(10, 50);
  bx3->Resize(200, 80);
  bx3->ClipChildren(false);

  AComboBox* cb3 = AComboBox::AttachTo(bx3);
  cb3->Text("Boxes");
  cb3->AddItem("item 3_1");
  cb3->AddItem("item 3_2");
  cb3->AddItem("item 3_3");
  cb3->Move(10, 10);

  au->ProcessMessages();
  delete au;
  return 0;
}
