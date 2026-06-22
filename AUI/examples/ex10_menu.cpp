#include "AUILib.h"

using namespace aui;

int32_t main() {
  AUI *au = AUI::Create("Menu Demo");
  AWindow *w = au->MainWnd();
  w->EnableResize();
  w->Resize(600, 450);
// ---------------------------------------------------------------
// 1. Horizontal menu bar
// ---------------------------------------------------------------
  std::vector<AMenuItem> barItems;
// File submenu
  AMenuItem fileItem("File");
  fileItem.subItems = { AMenuItem("New", []() {
    D1("New file");
  }),
  AMenuItem("Open", []() {
    D1("Open file");
  }),
  AMenuItem("Save", []() {
    D1("Save file");
  }),
  AMenuItem(),// separator
      AMenuItem("Exit", [au]() {
        D1("Exit called from Menu Bar – shutting down");
        au->ExitAUI();
      }) };
  fileItem.subItems[3].isSeparator = true;
  barItems.push_back(fileItem);
// Edit submenu
  AMenuItem editItem("Edit");
  editItem.subItems = { AMenuItem("Undo", []() {
    D1("Undo");
  }),
  AMenuItem("Redo", []() {
    D1("Redo");
  }),
  AMenuItem(),// separator
      AMenuItem("Cut", []() {
        D1("Cut");
      }),
      AMenuItem("Copy", []() {
        D1("Copy");
      }),
      AMenuItem("Paste", []() {
        D1("Paste");
      }) };
  editItem.subItems[2].isSeparator = true;
  barItems.push_back(editItem);

// View submenu with checkable items
  AMenuItem viewItem("View");
  viewItem.subItems = { AMenuItem("Toolbar", []() {
    D1("Toggle toolbar");
  }),
  AMenuItem("Status Bar", []() {
    D1("Toggle status bar");
  }),
  AMenuItem(),// separator
      AMenuItem("Zoom In", []() {
        D1("Zoom in");
      }),
      AMenuItem("Zoom Out", []() {
        D1("Zoom out");
      }) };
  viewItem.subItems[0].isCheckable = true;
  viewItem.subItems[0].isChecked = true;
  viewItem.subItems[1].isCheckable = true;
  viewItem.subItems[2].isSeparator = true;
  barItems.push_back(viewItem);

// Help submenu
  AMenuItem helpItem("Help");
  helpItem.subItems = { AMenuItem("About", []() {
    D1("About");
  }) };
  barItems.push_back(helpItem);

// Create and configure the horizontal menu bar
  AMenu *menuBar = AMenu::AttachTo(w, barItems);
  menuBar->SetOrientation(AUIOrientation::horizontal);
  menuBar->SetPermanent(true);
  menuBar->SetMinWidth(0);
  menuBar->SetItemHeight(28);
  menuBar->SetPadding(12);
  menuBar->SetColors(0xFFEEEEEE, 0xFFCCCCCC, 0xFF000000, 0xFF888888, 0xFF666666);
  menuBar->Move(0, 0);
  menuBar->Resize(800, 30);
  menuBar->SetSubmenuDelayMs(150);
  menuBar->Popup(0, 0);

// ---------------------------------------------------------------
// 2. Vertical context menu with "Exit" callback
// ---------------------------------------------------------------
  AButton *btn = AButton::AttachTo(w, "Click for context menu");
  btn->Move(200, 100);
  btn->Resize(200, 30);
  btn->SetBGColor(0xFFDDDDDD);

  std::vector<AMenuItem> ctxItems;
  ctxItems.push_back(AMenuItem("Copy", []() {
    D1("Copy");
  }));
  ctxItems.push_back(AMenuItem("Paste", []() {
    D1("Paste");
  }));
  AMenuItem alignItem("Align");
  alignItem.subItems = { AMenuItem("Left", []() {
    D1("Align left");
  }),
  AMenuItem("Center", []() {
    D1("Align center");
  }),
  AMenuItem("Right", []() {
    D1("Align right");
  }) };
  ctxItems.push_back(alignItem);
  ctxItems.push_back(AMenuItem());// separator
  ctxItems.back().isSeparator = true;
  ctxItems.push_back(AMenuItem("Properties", []() {
    D1("Properties");
  }));
// Add Exit item with callback to exit AUI
  ctxItems.push_back(AMenuItem("Exit", [au]() {
    D1("Exit called – shutting down");
    au->ExitAUI();
  }));

  AMenu *contextMenu = AMenu::AttachTo(w, ctxItems);
  contextMenu->SetOrientation(AUIOrientation::vertical);
  contextMenu->SetMinWidth(150);
  contextMenu->SetItemHeight(26);
  contextMenu->SetPadding(8);
  contextMenu->SetColors(0xFFF0F0F0, 0xFFCCCCCC, 0xFF000000, 0xFF888888, 0xFF666666);

  btn->SetClickCallback([](UNUSED AWindow *win, AWidget *widget, void *data,
  UNUSED int32_t x, UNUSED int32_t y, bool pressed) {
    if(!pressed)
      return;
    AMenu *menu = static_cast<AMenu*>(data);
    int32_t absX, absY;
    widget->GetAbsolutePosition(absX, absY);
    menu->Popup(absX, absY + static_cast<int32_t>(widget->SizeY()));
  }, contextMenu);

  D1("Menu demo ready. Click the button for context menu, or hover over menu bar items.");
  au->ProcessMessages();
  delete au;
  return 0;
}



