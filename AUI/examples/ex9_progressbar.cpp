#include "AUILib.h"

using namespace aui;

void RunManualProgress(AProgressBar *bar, std::atomic<bool> &stopFlag, AUI *au) {
    for (int i = 0; i <= 100; ++i) {
        if (stopFlag) break;
        DT("drawing progress %d%%", i);
        bar->SetProgress(static_cast<double>(i));   // 0..100
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    bar->GetParentWindow()->Draw();
    au->ExitAUI();
}

int main() {
    AUI *au = AUI::Create("ProgressBar Demo");
    AWindow *win = au->MainWnd();
    win->EnableResize();
    win->Resize(500, 250);

    AProgressBar *bar = AProgressBar::AttachTo(win);
    bar->Move(50, 100);
    bar->Resize(400, 45);
    bar->SetBarColor(0xFF00AA88);

    // --- Fix: set range to 0..100 and format as floating point with 0 decimals ---
    bar->SetRange(0, 100);
    bar->SetTextFormat("%.0f%%");   // %.0f prints the double correctly

    std::atomic<bool> stopFlag(false);
    std::thread worker(RunManualProgress, bar, std::ref(stopFlag), au);

    au->ProcessMessages();

    stopFlag = true;
    if (worker.joinable())
        worker.join();

    delete au;
    return 0;
}
