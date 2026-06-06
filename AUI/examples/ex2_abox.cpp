#include "AUILib.h"
#include "AWindow.h"
#include "ABox.h"

using namespace aui;

int32_t main() {
  // Create the UI engine and main window
  AUI* au = AUI::Create("Click the Box – Color Changes");
  if(!au) return 1;

  AWindow* w = au->MainWnd();
  w->SetBGColor(0xFF222222);   // dark grey background
  w->EnableResize();
  w->Resize(400, 300);

  // Create a box that occupies most of the window
  ABox* box = ABox::AttachTo(w);
  box->Move(50, 50);
  box->Resize(300, 200);
  box->SetBGColor(0xFF8844CC);          // initial purple

  // Add a click callback that cycles through a few colors
  uint32_t colors[] = {
    0xFF8844CC,   // purple
    0xFF44CC88,   // teal
    0xFFCC8844,   // orange
    0xFF44AACC    // light blue
  };
  int colorIndex = 0;

  box->SetClickCallback([&](AWindow*, AWidget*, void*, int32_t, int32_t, bool pressed) {
    if(!pressed) return;
    colorIndex = (colorIndex + 1) % 4;
    box->SetBGColor(colors[colorIndex]);
    // No explicit Draw() needed – SetBGColor calls parent window's Draw()
  }, nullptr);

  // Start the event loop
  au->ProcessMessages();

  delete au;
  return 0;
}

