#include "AUILib.h"

using namespace aui;

int32_t main() {
    AUI* au = AUI::Create("Emoji example");
    if (!au) return -1;
    AWindow* win = au->MainWnd();
    win->EnableResize();
    win->Resize(400, 200);
    ALabel* label = ALabel::AttachTo(win);
    label->Move(50, 50);
    label->Resize(300, 60);
    label->SetText("Hello 😀 World! 🎉");
    label->SetFontSize(24);
    label->SetBGColor(0xFFEEEEEE);
    label->SetBorderThickness(1);
    au->ProcessMessages();
    delete au;
    return 0;
}
