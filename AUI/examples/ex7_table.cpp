#include "AUILib.h"

using namespace aui;

int32_t main() {
//  AUI* au = AUI::Create("table rotation test", AUIWindowType::X11);
//  AUI* au = AUI::Create("table rotation test", AUIWindowType::Wayland);
  AUI* au = AUI::Create("table rotation test");
  AWindow* w = au->MainWnd();
  w->BGColor(0xFF222222);   // dark background
  w->EnableResize();
  w->Resize(500,500);
  w->DisableResize();

  ATable* ta = ATable::AttachTo(w);
  ta->Move(100, 100);
  ta->AddColumns(15);
  ta->AddRows(150);
  ta->ScrollbarsToggle(true);
  ta->CellData(0, 0, "AAA");
  ta->CellData(1, 1, "BBB");
  ta->CellData(2, 2, "CCC");
  ta->CellData(3, 3, "CCC");

  ta->Angle(30);
  au->ProcessMessages();
  delete au;
  return 0;
}
