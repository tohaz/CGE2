#include "AUILib.h"

using namespace aui;

int32_t main() {
  AUI *au = AUI::Create("Emoji example");
  if(!au) return -1;

  AWindow *w = au->MainWnd();
  w->EnableResize();
  w->Resize(400, 200);

  ALabel *lb = ALabel::AttachTo(w);
  lb->Move(50, 50);
  lb->Resize(300, 60);
  lb->SetText("Hello 😀 World! 🎉");
  lb->SetFontSize(24);
  lb->SetBGColor(0xFFEEEEEE);
  lb->SetHAlignment(AUIHAlign::center);
  lb->SetVAlignment(AUIVAlign::center);
  lb->SetBorderThickness(1);
  lb->SetAngle(0);// Start at 0 degrees

  std::atomic<bool> running { true };
  std::thread rotationThread([lb, &running]() {
    const double degreesPerSecond = 18.0;
    auto startTime = std::chrono::steady_clock::now();
    while (running) {
      auto currentTime = std::chrono::steady_clock::now();
      std::chrono::duration<double> elapsed = currentTime - startTime;
      double currentAngle = elapsed.count() * degreesPerSecond;
      currentAngle = std::fmod(currentAngle, 360.0);
      lb->SetAngle(currentAngle);
      lb->GetParentWindow()->Draw();
      std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
  });
  au->ProcessMessages();
  running = false;
  if(rotationThread.joinable()) {
    rotationThread.join();
  }
  delete au;
  return 0;
}
