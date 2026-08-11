#include "AUILib.h"

using namespace aui;

int main() {
  AUI* au = AUI::Create("Submenu Example");
  if(!au)
    return -1;
  
  AWindow* win = au->MainWnd();
  win->EnableResize();
  win->Resize(640, 480);
//  int counter = 0;
// Build submenu items for "File"
  std::vector<AMenuItem> fileItems;
  fileItems.emplace_back("Exit", [win]() {
    win->Close();
  });
// Main menu bar items (each with a submenu)
  std::vector<AMenuItem> mainItems;
  mainItems.emplace_back("File", std::move(fileItems));
  mainItems.emplace_back("Help", []() {
    std::cout << "About\n";
  });// simple item, no submenu
// Attach the menu as a permanent horizontal bar
  AMenu* menu = AMenu::AttachTo(win, std::move(mainItems));
  menu->Orientation(AUIOrientation::horizontal);
  menu->SetPermanent(true);
  menu->Move(0, 0);
  menu->Resize(win->SizeX(), 28);
  menu->SetColors(0xFFDDDDDD, 0xFFAAAAAA, 0xFF000000, 0xFF888888);
  menu->Show();
  au->ProcessMessages();
  delete au;
  return 0;
}
