#include "AUILib.h"

using namespace aui;

int32_t main() {
  // 1. Create UI engine and main window
  AUI* au = AUI::Create("ComboBox Demo");
  if (!au) return -1;
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(400, 300);
  // 2. Primary combobox (fruits)
  UNUSED AComboBox* cbFruits = AComboBox::AttachTo(w);
  cbFruits->Move(50, 50);
  cbFruits->Resize(280, 28);
  cbFruits->FontSize(14);
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
  AComboBox* cbColors = AComboBox::AttachTo(w);
  cbColors->Move(50, 100);
  cbColors->Resize(280, 28);
  cbColors->FontSize(14);
  cbColors->SetEditable(false);
  cbColors->AddItem("Red");
  cbColors->AddItem("Green");
  cbColors->AddItem("Yellow");
  cbColors->AddItem("Orange");
  cbColors->AddItem("Purple");
  cbColors->SetSelectedIndex(0);
  // 4. Label – using built‑in ALabel
  ALabel* label = ALabel::AttachTo(w);
  label->Move(50, 150);
  label->Resize(280, 30);
  label->Text("Selected: " + cbFruits->GetSelectedText());
  label->BGColor(0xFFEEEEEE);
  label->Border(1);
  // 5. Print button
  AButton* printBtn = AButton::AttachTo(w, "Print Selection");
  printBtn->Move(50, 200);
  printBtn->Resize(140, 28);
  printBtn->SetMouseClickCallback([](AWidget* wi, void* data, int32_t, int32_t) -> AWidget* {
    AComboBox* cb = static_cast<AComboBox*>(data);
    std::cout << "Fruit: index=" << cb->GetSelectedIndex()
              << ", text=\"" << cb->GetSelectedText() << "\"\n";
    return wi;
  }, cbFruits);
  // 6. Reset button
  AButton* resetBtn = AButton::AttachTo(w, "Reset");
  resetBtn->Move(200, 200);
  resetBtn->Resize(80, 28);
  resetBtn->SetMouseClickCallback([](AWidget* wi, void* data, int32_t, int32_t) -> AWidget* {
    AComboBox* cb = static_cast<AComboBox*>(data);
    cb->ClearSelection();
    return wi;
  }, cbFruits);
  // 7. Live update: connect combobox input change to label
  cbFruits->GetInputBox()->SetOnChangeCallback([cbFruits, label](AInputBox*, const std::string&) {
    label->Text("Selected: " + cbFruits->GetSelectedText());
  });

  au->ProcessMessages();
  delete au;
  return 0;
}
