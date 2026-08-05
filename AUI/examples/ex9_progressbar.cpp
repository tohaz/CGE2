#include "AUILib.h"

using namespace aui;

void RunManualProgress(AProgressBar *bar, std::atomic<bool> &stopFlag, AUI *au) {
  for(int i = 0; i <= 100; ++i) {
    if(stopFlag)
      break;
    D3("drawing progress %d%%", i);
    bar->Progress(static_cast<double>(i));// 0..100
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  bar->Wnd()->RequestRedraw();
  au->ExitAUI();
}

int main() {
  AUI* au = AUI::Create("ProgressBar Demo joke");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(500, 250);
  ALabel* l = ALabel::AttachTo(w, "should fill vertical", 180, 80, 1U, 1U);
  l->TrimToText();
  AProgressBar* bar = AProgressBar::AttachTo(w);
  bar->Move(50, 100);
  bar->Resize(400, 45);
  bar->BGColor3(0xFF00AA88);
  bar->Range(0, 100);
  bar->Orient(AUIOrientation::vertical);
  bar->Direction(AUIDirection::top);
  bar->TextFormat("%.0f%%");// %.0f prints the double correctly
  std::atomic<bool> stopFlag(false);
  std::thread worker(RunManualProgress, bar, std::ref(stopFlag), au);
  au->ProcessMessages();
  stopFlag = true;
  if(worker.joinable())
    worker.join();
  delete au;
  return 0;
}
