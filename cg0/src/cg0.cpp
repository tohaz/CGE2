//#include "AUILib.h"
//#include <iostream>
//#include <string>
//
//using namespace aui;
//
//int32_t main() {
//  // 1. Create UI engine and main window
//  AUI* au = AUI::Create("ComboBox Demo");
//  if (!au) return -1;
//
//  AWindow* win = au->MainWnd();
//  win->EnableResize();
//  win->Resize(600, 400);
//
//  // 2. Primary combobox (fruits)
//  AComboBox* cbFruits = AComboBox::AttachTo(win);
//  cbFruits->Move(50, 50);
//  cbFruits->Resize(280, 28);
//  cbFruits->SetFontSize(14);
//  cbFruits->SetEditable(true);
//  cbFruits->AddItem("🍎 Red Apple");
//  cbFruits->AddItem("🍌 Green Banana");
//  cbFruits->AddItem("🌸 Cherry Blossom");
//  cbFruits->AddItem("🌴 Date Palm");
//  cbFruits->AddItem("🍇 Elderberry");
//  cbFruits->AddItem("🍬 Fig Newton");
//  cbFruits->AddItem("🍊 Grapefruit");
//  cbFruits->AddItem("🍈 Honeydew Melon");
//  cbFruits->AddItem("🥭 Indian Mango");
//  cbFruits->AddItem("🌶️ Jalapeño Pepper");
//  cbFruits->SetSelectedIndex(2); // "Cherry Blossom"
//
//  // 3. Secondary combobox (colors)
//  AComboBox* cbColors = AComboBox::AttachTo(win);
//  cbColors->Move(50, 100);
//  cbColors->Resize(280, 28);
//  cbColors->SetFontSize(14);
//  cbColors->SetEditable(false);
//  cbColors->AddItem("Red");
//  cbColors->AddItem("Green");
//  cbColors->AddItem("Yellow");
//  cbColors->AddItem("Orange");
//  cbColors->AddItem("Purple");
//  cbColors->SetSelectedIndex(0);
//
//  // 4. Label – using built‑in ALabel
//  ALabel* label = ALabel::AttachTo(win);
//  label->Move(50, 150);
//  label->Resize(280, 30);
//  label->SetText("Selected: " + cbFruits->GetSelectedText());
//  label->SetBGColor(0xFFEEEEEE);
//  label->SetBorderThickness(1);
//
//  // 5. Print button
//  AButton* printBtn = AButton::AttachTo(win, "Print Selection");
//  printBtn->Move(50, 200);
//  printBtn->Resize(140, 28);
//  printBtn->SetClickCallback([](AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
//    if (!pressed) return;
//    AComboBox* cb = static_cast<AComboBox*>(data);
//    std::cout << "Fruit: index=" << cb->GetSelectedIndex()
//              << ", text=\"" << cb->GetSelectedText() << "\"\n";
//  }, cbFruits);
//
//  // 6. Reset button
//  AButton* resetBtn = AButton::AttachTo(win, "Reset");
//  resetBtn->Move(200, 200);
//  resetBtn->Resize(80, 28);
//  resetBtn->SetClickCallback([](AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
//    if (!pressed) return;
//    AComboBox* cb = static_cast<AComboBox*>(data);
//    cb->ClearSelection();
//  }, cbFruits);
//
//  // 7. Live update: connect combobox input change to label
//  cbFruits->GetInputBox()->SetOnChangeCallback([cbFruits, label](AInputBox*, const std::string&) {
//    label->SetText("Selected: " + cbFruits->GetSelectedText());
//  });
//
//  au->ProcessMessages();
//
//  delete au;
//  return 0;
//}


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



