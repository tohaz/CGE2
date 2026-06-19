#include "AUILib.h"

using namespace aui;

int32_t main() {
  // 1. Create UI engine and main window
  AUI* au = AUI::Create("ComboBox Demo");
  if (!au) return -1;

  AWindow* win = au->MainWnd();
  win->EnableResize();
  win->Resize(400, 300);

  // 2. Primary combobox (fruits)
  AComboBox* cbFruits = AComboBox::AttachTo(win);
  cbFruits->Move(50, 50);
  cbFruits->Resize(280, 28);
  cbFruits->SetFontSize(14);
  cbFruits->SetEditable(true);
  cbFruits->AddItem("🍎 Red Apple");
  cbFruits->AddItem("🍌 Green Banana");
  cbFruits->AddItem("🌸 Cherry Blossom");
  cbFruits->AddItem("🌴 Date Palm");
  cbFruits->AddItem("🍇 Elderberry");
  cbFruits->AddItem("🍬 Fig Newton");
  cbFruits->AddItem("🍊 Grapefruit");
  cbFruits->AddItem("🍈 Honeydew Melon");
  cbFruits->AddItem("🥭 Indian Mango");
  cbFruits->AddItem("🌶️ Jalapeño Pepper");
  cbFruits->SetSelectedIndex(2); // "Cherry Blossom"

  // 3. Secondary combobox (colors)
  AComboBox* cbColors = AComboBox::AttachTo(win);
  cbColors->Move(50, 100);
  cbColors->Resize(280, 28);
  cbColors->SetFontSize(14);
  cbColors->SetEditable(false);
  cbColors->AddItem("Red");
  cbColors->AddItem("Green");
  cbColors->AddItem("Yellow");
  cbColors->AddItem("Orange");
  cbColors->AddItem("Purple");
  cbColors->SetSelectedIndex(0);

  // 4. Label – using built‑in ALabel
  ALabel* label = ALabel::AttachTo(win);
  label->Move(50, 150);
  label->Resize(280, 30);
  label->SetText("Selected: " + cbFruits->GetSelectedText());
  label->SetBGColor(0xFFEEEEEE);
  label->SetBorderThickness(1);

  // 5. Print button
  AButton* printBtn = AButton::AttachTo(win, "Print Selection");
  printBtn->Move(50, 200);
  printBtn->Resize(140, 28);
  printBtn->SetClickCallback([](AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
    if (!pressed) return;
    AComboBox* cb = static_cast<AComboBox*>(data);
    std::cout << "Fruit: index=" << cb->GetSelectedIndex()
              << ", text=\"" << cb->GetSelectedText() << "\"\n";
  }, cbFruits);

  // 6. Reset button
  AButton* resetBtn = AButton::AttachTo(win, "Reset");
  resetBtn->Move(200, 200);
  resetBtn->Resize(80, 28);
  resetBtn->SetClickCallback([](AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
    if (!pressed) return;
    AComboBox* cb = static_cast<AComboBox*>(data);
    cb->ClearSelection();
  }, cbFruits);

  // 7. Live update: connect combobox input change to label
  cbFruits->GetInputBox()->SetOnChangeCallback([cbFruits, label](AInputBox*, const std::string&) {
    label->SetText("Selected: " + cbFruits->GetSelectedText());
  });

  au->ProcessMessages();

  delete au;
  return 0;
}

