
#include "AUILib.h"

using namespace aui;

int32_t main() {
AUI *au = AUI::Create("Emoji example");
//  AUI *au = AUI::Create("Emoji example", AUIWindowType::Wayland);
  if(!au) return -1;
  AWindow *w = au->MainWnd();
  w->EnableResize();
  w->Resize(400, 200);
  //w->DisableResize();
  ALabel *lb = ALabel::AttachTo(w);
  lb->Move(50, 50);
  lb->Resize(300, 60);
  lb->Text("Hello 😀 World! 🎉");
  lb->FontSize(24);
  lb->BGColor(0xFFEEEEEE);
  lb->HAlign(AUIHAlign::center);
  lb->VAlign(AUIVAlign::center);
  lb->Border(5);
  lb->Angle(20);// Start at 0 degrees
  std::atomic<bool> running { true };
  std::mutex mtx;
  std::condition_variable cv;
  std::thread rotationThread([&]() {
    const double degreesPerSecond = 18.0;
    auto startTime = std::chrono::steady_clock::now();
    while(running) {
      auto currentTime = std::chrono::steady_clock::now();
      std::chrono::duration<double> elapsed = currentTime - startTime;
      double currentAngle = std::fmod(elapsed.count() * degreesPerSecond, 360.0);
      lb->Angle(currentAngle);
      D3("Angle updated, writing pipe");
// Wait for 16 ms or until exit signal
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait_for(lock, std::chrono::milliseconds(15), [&]() {
        return !running.load();
      });
    }
  });
  au->ProcessMessages();
  running = false;
  cv.notify_one();
  if(rotationThread.joinable())
    rotationThread.join();
  delete au;
  return 0;
}
