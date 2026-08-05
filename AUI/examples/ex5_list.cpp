#include "AUILib.h"

using namespace aui;

int32_t main() {
// Create the application and main window
  AUI* au = AUI::Create("AList Test");
  AWindow* win = au->MainWnd();
  win->EnableResize();
  win->Resize(500, 500);
  win->Move(100, 100);
  UNUSED AList* list = AList::AttachTo(win);
  list->Move(50, 50);
  list->Resize(380, 380);
  list->EnableVScrollbar(true);
  list->EnableHScrollbar(true);
  list->AutoHideVScrollbar(false);// always show
  list->AddItem(
      "HoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydew");
  list->AddItem("Apple");
  list->AddItem("Banana");
  list->AddItem("Cherry");
  list->AddItem("Date");
  list->AddItem("Elderberry");
  list->AddItem("Fig");
  list->AddItem("Grape");
  list->AddItem("Honeydew");
  list->AddItem(
      "HoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem(
      "HoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem(
      "HoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem(
      "HoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem(
      "HoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydewHoneydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
  list->AddItem("Honeydew");
// Bulk add more items
  std::vector<std::string> more =
      { "Ice Cream", "Jackfruit", "Kiwi", "Lemon", "Mango", "Nectarine", "Orange", "Papaya" };
  list->AddItems(more);
// Enable multi‑select
  list->MultiSelect(true);
// Select some items
  list->SelectIndex(2, true);// Cherry
  list->SelectIndex(5, true);// Fig
  list->SelectIndex(10, true);// Kiwi (index 10)
// Set custom selection colors
  list->SelectionColor(0xFF3399FF);// blue background
  list->SelectionTextColor(0xFFFFFFFF);// white text
// Set hover color (semi‑transparent)
  list->HoverColor(0x40FF0000);// red tint
// Register selection change callback
  list->SetOnSelectionChanged([](AWidget* widget, void*) {
    AList* l = static_cast<AList*>(widget);
    auto selected = l->SelectedIndices();
    std::cout << "Selection changed: " << selected.size() << " items selected\n";
    for(size_t idx : selected) {
      std::cout << "  - " << l->GetItem(idx) << " (index " << idx << ")\n";
    }
    std::cout << std::endl;
  });
// Optionally, you can scroll to a specific item
  list->ScrollToItem(15, true);// scroll to "Papaya" and center it
// Run the event loop
  D1("setup done")
  au->ProcessMessages();
  delete au;
  return 0;
}

