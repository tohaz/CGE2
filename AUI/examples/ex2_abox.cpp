#include "AUILib.h"

using namespace aui;

int32_t main() {
  // Create the UI engine and main window
  AUI* au = AUI::Create("Click the Box – Color Changes");
  if(!au) return 1;
  AWindow* w = au->MainWnd();
  w->BGColor(0xFF222222);   // dark grey background
  w->EnableResize();
  w->Resize(400, 300);
  // Create a box that occupies most of the window
  ABox* box = ABox::AttachTo(w);
  box->Move(50, 50);
  box->Resize(300, 200);
  box->BGColor(0xFF8844CC);          // initial purple
  // Add a click callback that cycles through a few colors
  uint32_t colors[] = {
    0xFF8844CC,   // purple
    0xFF44CC88,   // teal
    0xFFCC8844,   // orange
    0xFF44AACC    // light blue
  };
  int32_t colorIndex = 0;
  int32_t colorIndex2 = 0;
  int32_t colorIndex3 = 0;
  int32_t colorIndex4 = 0;
  box->SetMouseClickCallback([&](AWidget*, void*, int32_t, int32_t) -> AWidget* {
    colorIndex = (colorIndex + 1) % 4;
    box->BGColor(colors[colorIndex]);
    return box;
  }, nullptr);
  ABox* box2 = ABox::AttachTo(box);
  box2->Move(50, 50);
  box2->Resize(50, 50);
  box2->BGColor(0xFFFF44CC);
  box2->SetMouseClickCallback([&](AWidget*, void*, int32_t, int32_t) -> AWidget* {
    colorIndex2 = (colorIndex2 + 1) % 4;
    box2->BGColor(colors[colorIndex2]);
    return box2;
  }, nullptr);
  ABox* box3 = ABox::AttachTo(box);
  box3->Move(250, 50);
  box3->Resize(50, 50);
  box3->BGColor(0xFFFF44CC);
  box3->SetMouseClickCallback([&](AWidget*, void*, int32_t, int32_t) -> AWidget* {
    colorIndex3 = (colorIndex3 + 1) % 4;
    box3->BGColor(colors[colorIndex3]);
    return box3;
  }, nullptr);
  ABox* box4 = ABox::AttachTo(box);
  box4->Move(75, 75);
  box4->Resize(150, 50);
  box4->BGColor(0xFFFF44CC);
  box4->SetMouseClickCallback([&](AWidget*, void*, int32_t, int32_t) -> AWidget* {
    colorIndex4 = (colorIndex4 + 1) % 4;
    box4->BGColor(colors[colorIndex4]);
    return box4;
  }, nullptr);
  // Start the event loop
  au->ProcessMessages();
  delete au;
  return 0;
}

