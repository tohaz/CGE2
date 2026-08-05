#include "AUILib.h"

using namespace aui;

int main() {
  AUI* au = AUI::Create("AInputBox Demo");
//  AUI* au = AUI::Create("AInputBox Demo", AUIWindowType::X11);
  AWindow* win = au->MainWnd();
  win->EnableResize();
  win->Resize(750, 550);
  win->BGColor(0xFF2E3440);// dark background
  // ----- Input boxes -----
  AInputBox* basic = AInputBox::AttachTo(win);
  basic->Move(30, 40);
  basic->Resize(420, 40);
  basic->BGColor(0xFF4C566A);
  basic->TextColor(0xFFD8DEE9);
  basic->Border(2);
  basic->BorderColor(0xFF81A1C1);
  basic->InputFilter("^[A-Za-z0-9]*$");
  basic->MaxLength(20);
  basic->Placeholder("Alphanumeric only (max 20)");
  basic->PlaceholderColor(0xFF8FBCBB);
  auto* numeric = AInputBox::AttachTo(win);
  numeric->Move(30, 100);
  numeric->Resize(420, 40);
  numeric->BGColor(0xFF434C5E);
  numeric->TextColor(0xFFEBCB8B);
  numeric->Border(2);
  numeric->BorderColor(0xFFD08770);
  numeric->HAlign(AUIHAlign::right);
  numeric->InputFilter("^[0-9]*$");
  numeric->MaxLength(10);
  numeric->Placeholder("Only numbers (right‑aligned)");
  numeric->PlaceholderColor(0xFFD08770);
  auto* password = AInputBox::AttachTo(win);
  password->Move(30, 160);
  password->Resize(420, 40);
  password->BGColor(0xFF3B4252);
  password->TextColor(0xFFBF616A);
  password->Border(2);
  password->BorderColor(0xFFBF616A);
  password->Placeholder("Password (masked)");
  password->PasswordMode(true);
  auto* status = AInputBox::AttachTo(win);
  status->Move(30, 460);
  status->Resize(690, 40);
  status->BGColor(0xFF2E3440);
  status->TextColor(0xFFA3BE8C);
  status->Border(1);
  status->BorderColor(0xFFA3BE8C);
  status->Editable(false);
  status->Text("Ready");
// ----- Buttons (arranged vertically on the right side) -----
  int32_t btnX = 480;
  int32_t btnY = 40;
  int32_t btnStep = 45;
  int32_t btnW = 230;
  int32_t btnH = 35;
// Helper to create buttons with attached callbacks
  UNUSED auto addButton = [&](const char *label, std::function<void()> cb) -> AButton* {
   auto* btn = AButton::AttachTo(win, label);
    btn->Move(btnX, btnY);
    btn->Resize((uint32_t) btnW, (uint32_t) btnH);
    btn->SetMousePressLeftCallback([cb](AWidget* wi, void*, int32_t, int32_t) -> AWidget*{
      if(cb)
        cb();
      return wi;
    }, nullptr);
    btnY += btnStep;
    return btn;
  };
// Toggle editable (affects basic, numeric, password; status stays read-only)
  addButton("Toggle Editable", [basic, numeric, password, status]() {
    static bool editable = true;
    editable = !editable;
    basic->Editable(editable);
    numeric->Editable(editable);
    password->Editable(editable);
    status->Text(editable ? "Editable mode ON" : "Editable mode OFF");
  });
// Toggle enabled/disabled
  addButton("Toggle Enabled", [basic, numeric, password, status]() {
    static bool enabled = true;
    enabled = !enabled;
    if(enabled) {
      basic->Enable();
      numeric->Enable();
      password->Enable();
      status->Enable();
      status->Text("Widgets enabled");
    }
    else {
      basic->Disable();
      numeric->Disable();
      password->Disable();
      status->Disable();
      status->Text("Widgets disabled");
    }
  });
// Toggle insert/overwrite mode (basic and numeric only)
  addButton("Insert/Overwrite", [basic, numeric, status]() {
    static bool insert = true;
    insert = !insert;
    basic->InsertMode(insert);
    numeric->InsertMode(insert);
    status->Text(insert ? "Insert mode" : "Overwrite mode");
  });
// Toggle cursor blinking
  addButton("Blink On/Off", [basic, numeric, password, status]() {
    static bool blink = true;
    blink = !blink;
    basic->CursorBlinkingEnabled(blink);
    numeric->CursorBlinkingEnabled(blink);
    password->CursorBlinkingEnabled(blink);
    status->Text(blink ? "Cursor blinking ON" : "Cursor blinking OFF");
  });
// Toggle password masking
  addButton("Mask Password", [password, status]() {
    static bool masked = true;
    masked = !masked;
    password->PasswordMode(masked);
    status->Text(masked ? "Password hidden" : "Password visible (demo)");
  });
// ----- Callbacks to update status on submit (Enter key) -----
  basic->SetOnSubmitCallback([status](AInputBox*, const std::string &val) {
    status->Text("Basic submitted: " + val);
  });
  numeric->SetOnSubmitCallback([status](AInputBox*, const std::string &val) {
    status->Text("Numeric submitted: " + val);
  });
  password->SetOnSubmitCallback([status](AInputBox*, const std::string&) {
    status->Text("Password submitted (value hidden)");
  });
// Optional change callbacks – update status while typing
  basic->SetOnChangeCallback([status](AInputBox*, const std::string &val) {
    status->Text("Basic changed: " + val);
  });
  numeric->SetOnChangeCallback([status](AInputBox*, const std::string &val) {
    status->Text("Numeric changed: " + val);
  });
  au->ProcessMessages();
  delete au;
  return 0;
}
