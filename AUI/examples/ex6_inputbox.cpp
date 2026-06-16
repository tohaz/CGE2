#include <chrono>
#include <thread>
#include <future>
#include "AUILib.h"
#include "AInputBox.h"
#include "AButton.h"

using namespace aui;

int main() {
    AUI *au = AUI::Create("AInputBox Demo");
    AWindow *win = au->MainWnd();
    win->EnableResize();
    win->Resize(750, 550);
    win->SetBGColor(0xFF2E3440);  // dark background

    // ----- Input boxes -----
    auto *basic = AInputBox::AttachTo(win);
    basic->Move(30, 40);
    basic->Resize(420, 40);
    basic->SetBGColor(0xFF4C566A);
    basic->SetTextColor(0xFFD8DEE9);
    basic->SetBorderThickness(2);
    basic->SetBorderColor(0xFF81A1C1);
    basic->SetInputFilter("^[A-Za-z0-9]*$");
    basic->SetMaxLength(20);
    basic->SetPlaceholder("Alphanumeric only (max 20)");
    basic->SetPlaceholderColor(0xFF8FBCBB);

    auto *numeric = AInputBox::AttachTo(win);
    numeric->Move(30, 100);
    numeric->Resize(420, 40);
    numeric->SetBGColor(0xFF434C5E);
    numeric->SetTextColor(0xFFEBCB8B);
    numeric->SetBorderThickness(2);
    numeric->SetBorderColor(0xFFD08770);
    numeric->SetHAlignment(AUIHAlign::right);
    numeric->SetInputFilter("^[0-9]*$");
    numeric->SetMaxLength(10);
    numeric->SetPlaceholder("Only numbers (right‑aligned)");
    numeric->SetPlaceholderColor(0xFFD08770);

    auto *password = AInputBox::AttachTo(win);
    password->Move(30, 160);
    password->Resize(420, 40);
    password->SetBGColor(0xFF3B4252);
    password->SetTextColor(0xFFBF616A);
    password->SetBorderThickness(2);
    password->SetBorderColor(0xFFBF616A);
    password->SetPlaceholder("Password (masked)");
    password->SetPasswordMode(true);

    auto *status = AInputBox::AttachTo(win);
    status->Move(30, 460);
    status->Resize(690, 40);
    status->SetBGColor(0xFF2E3440);
    status->SetTextColor(0xFFA3BE8C);
    status->SetBorderThickness(1);
    status->SetBorderColor(0xFFA3BE8C);
    status->SetEditable(false);
    status->SetText("Ready");

    // ----- Buttons (arranged vertically on the right side) -----
    int32_t btnX = 480;
    int32_t btnY = 40;
    int32_t btnStep = 45;
    int32_t btnW = 230;
    int32_t btnH = 35;

    // Helper to create buttons with attached callbacks
    auto addButton = [&](const char* label, std::function<void()> cb) -> AButton* {
        auto *btn = AButton::AttachTo(win, label);
        btn->Move(btnX, btnY);
        btn->Resize((uint32_t)btnW, (uint32_t)btnH);
        btn->SetClickCallback([cb](AWindow*, AWidget*, void*, int32_t, int32_t, bool pressed) {
            if (pressed && cb) cb();
        }, nullptr);
        btnY += btnStep;
        return btn;
    };

    // Toggle editable (affects basic, numeric, password; status stays read-only)
    addButton("Toggle Editable", [basic, numeric, password, status]() {
        static bool editable = true;
        editable = !editable;
        basic->SetEditable(editable);
        numeric->SetEditable(editable);
        password->SetEditable(editable);
        status->SetText(editable ? "Editable mode ON" : "Editable mode OFF");
    });

    // Toggle enabled/disabled
    addButton("Toggle Enabled", [basic, numeric, password, status]() {
        static bool enabled = true;
        enabled = !enabled;
        if (enabled) {
            basic->Enable();
            numeric->Enable();
            password->Enable();
            status->Enable();
            status->SetText("Widgets enabled");
        } else {
            basic->Disable();
            numeric->Disable();
            password->Disable();
            status->Disable();
            status->SetText("Widgets disabled");
        }
    });

    // Toggle insert/overwrite mode (basic and numeric only)
    addButton("Insert/Overwrite", [basic, numeric, status]() {
        static bool insert = true;
        insert = !insert;
        basic->SetInsertMode(insert);
        numeric->SetInsertMode(insert);
        status->SetText(insert ? "Insert mode" : "Overwrite mode");
    });

    // Toggle cursor blinking
    addButton("Blink On/Off", [basic, numeric, password, status]() {
        static bool blink = true;
        blink = !blink;
        basic->SetCursorBlinkingEnabled(blink);
        numeric->SetCursorBlinkingEnabled(blink);
        password->SetCursorBlinkingEnabled(blink);
        status->SetText(blink ? "Cursor blinking ON" : "Cursor blinking OFF");
    });

    // Toggle password masking
    addButton("Mask Password", [password, status]() {
        static bool masked = true;
        masked = !masked;
        password->SetPasswordMode(masked);
        status->SetText(masked ? "Password hidden" : "Password visible (demo)");
    });

    // ----- Callbacks to update status on submit (Enter key) -----
    basic->SetOnSubmitCallback([status](AInputBox*, const std::string& val) {
        status->SetText("Basic submitted: " + val);
    });
    numeric->SetOnSubmitCallback([status](AInputBox*, const std::string& val) {
        status->SetText("Numeric submitted: " + val);
    });
    password->SetOnSubmitCallback([status](AInputBox*, const std::string&) {
        status->SetText("Password submitted (value hidden)");
    });

    // Optional change callbacks – update status while typing
    basic->SetOnChangeCallback([status](AInputBox*, const std::string& val) {
        status->SetText("Basic changed: " + val);
    });
    numeric->SetOnChangeCallback([status](AInputBox*, const std::string& val) {
        status->SetText("Numeric changed: " + val);
    });

    au->ProcessMessages();
    delete au;
    return 0;
}


