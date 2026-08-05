#include "AUILib.h"

using namespace aui;

int32_t main() {
//  AUI* au = AUI::Create("test", AUIWindowType::X11);
  AUI* au = AUI::Create("alignment cases");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(360,190);

  UNUSED ALabel* la = ALabel::AttachTo(w);
  la->Angle(0);
  la->Text("Hello123456789");
  la->Move(50, 50);
  la->Border(1);
  UNUSED ALabel* la2 = ALabel::AttachTo(w);
  la2->Angle(0);
  la2->Text("Hello123456789");
  la2->Move(50, 80);
  la2->Border(1);
  la2->HAlign(AUIHAlign::left);
  UNUSED ALabel* la3 = ALabel::AttachTo(w);
  la3->Angle(0);
  la3->Text("Hello123456789");
  la3->Move(50, 110);
  la3->Border(1);
  la3->HAlign(AUIHAlign::right);
  UNUSED ALabel* la4 = ALabel::AttachTo(w);
  la4->Angle(0);
  la4->Text("Hello");
  la4->Move(200, 50);
  la4->Border(1);
  UNUSED ALabel* la5 = ALabel::AttachTo(w);
  la5->Angle(0);
  la5->Text("Hello");
  la5->Move(200, 80);
  la5->Border(1);
  la5->HAlign(AUIHAlign::left);
  UNUSED ALabel* la6 = ALabel::AttachTo(w);
  la6->Angle(0);
  la6->Text("Hello");
  la6->Move(200, 110);
  la6->Border(1);
  la6->HAlign(AUIHAlign::right);

  au->ProcessMessages();
  delete au;
  return 0;
}
