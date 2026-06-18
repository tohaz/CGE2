
#include "AUILib.h"

using namespace aui;

int32_t main() {
  AUI *au = AUI::Create("Table Demo");
  AWindow *w = au->MainWnd();
  w->EnableResize();
  w->Resize(1024, 768);
  auto start = std::chrono::steady_clock::now();
  AComboBox* cb = AComboBox::AttachTo(w);
  cb->Move(50, 50);
  cb->Resize(200, 28);
  cb->SetFontSize(14);
  cb->SetEditable(true);  // allow typing (and auto-complete/search)
  cb->AddItem("Apple");
  cb->AddItem("Banana");
  cb->AddItem("Cherry");
  cb->SetSelectedIndex(2);
  AComboBox* cb2 = AComboBox::AttachTo(w);
  cb2->Move(50, 80);
  cb2->Resize(200, 28);
  cb2->AddItem("ZZZ");
  cb2->AddItem("ZZZ 2");
  cb2->AddItem("ZZZ 3");
  cb2->AddItem("ZZZ 4");
  cb2->SetSelectedIndex(1);
  D1("init {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count());
  au->ProcessMessages();
  delete au;
  return 0;
}



