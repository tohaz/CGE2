#include "AUILib.h"

using namespace aui;

int main() {
  AUI* au = AUI::Create("ProgressBar Demo");
  AWindow* win = au->MainWnd();
  win->EnableResize();
  win->Resize(600, 350);
  // ---- 1: Horizontal (sausage) ----
  AProgressBar* p1 = AProgressBar::AttachTo(win);
  p1->Move(50, 50);
  p1->Resize(300, 30);
  p1->SetRange(0, 100);
  p1->SetBarColor(0xFF00AA00);
  p1->SetTextFormat("%.0f%%");
  p1->SetRoundedCorners(true, 15);
  p1->SetProgressProvider([]() noexcept {
    static double p = 0.0;
    p += 1.0;
    if (p > 100.0) p = 0.0;
    return p;
  });
  // ---- 2: Horizontal right-to-left, stripes ----
  AProgressBar* p2 = AProgressBar::AttachTo(win);
  p2->Move(50, 100);
  p2->Resize(300, 30);
  p2->SetRange(0, 100);
  p2->SetDirection(AUIDirection::left);
  p2->SetBarColor(0xFF0000FF);
  p2->SetStripe(true);
  p2->SetStripeColor(0x40FFFFFF);
  p2->SetStripeWidth(5);
  p2->SetStripeSpeed(-2);
  p2->SetTextFormat("%.0f%%");
  p2->SetRoundedCorners(true, 15);
  p2->SetProgressProvider([]() noexcept {
    static double p = 0.0;
    p += 1.5;
    if (p > 100.0) p = 0.0;
    return p;
  });
  // ---- 3: Vertical top ----
  AProgressBar* p3 = AProgressBar::AttachTo(win);
  p3->Move(400, 50);
  p3->Resize(30, 200);
  p3->SetOrientation(AUIOrientation::vertical);
  p3->SetDirection(AUIDirection::top);
  p3->SetBarColor(0xFFFF8800);
  p3->SetRoundedCorners(true, 15);
  p3->SetTextFormat("%.1f");
  p3->SetProgressProvider([]() noexcept {
    static double p = 0.0;
    p += 0.02;
    if (p > 1.0) p = 0.0;
    return p;
  });
  // ---- 4: Vertical bottom, gradient ----
  AProgressBar* p4 = AProgressBar::AttachTo(win);
  p4->Move(460, 50);
  p4->Resize(30, 200);
  p4->SetOrientation(AUIOrientation::vertical);
  p4->SetDirection(AUIDirection::bottom);
  p4->SetBarColor(0xFFAA44FF);
  p4->SetBarColor2(0xFFFF44AA);
  p4->SetRoundedCorners(true, 15);
  p4->SetTextFormat("%.1f");
  p4->SetProgressProvider([]() noexcept {
    static double p = 0.0;
    p += 0.025;
    if (p > 1.0) p = 0.0;
    return p;
  });
  // ---- 5: Indeterminate ----
  AProgressBar* p5 = AProgressBar::AttachTo(win);
  p5->Move(50, 150);
  p5->Resize(300, 30);
  p5->SetBarColor(0xFFFF4444);
  p5->SetIndeterminate(true);
  p5->SetStripe(true);
  p5->SetStripeColor(0x30FFFFFF);
  p5->SetStripeSpeed(5);
  p5->SetRoundedCorners(false, 15);
  p5->SetShowText(true);
  // ---- 6: Horizontal gradient, no text ----
  AProgressBar* p6 = AProgressBar::AttachTo(win);
  p6->Move(50, 200);
  p6->Resize(300, 30);
  p6->SetRange(0, 100);
  p6->SetBarColor(0xFF00CC88);
  p6->SetBarColor2(0xFFCC8800);
  p6->SetShowText(false);
  p6->SetRoundedCorners(true, 15);
  p6->SetProgressProvider([]() noexcept {
    static double p = 0.0;
    p += 1.0;
    if (p > 100.0) p = 0.0;
    return p;
  });
  // ---- 7: Horizontal with stripes, rounded ----
  AProgressBar* p7 = AProgressBar::AttachTo(win);
  p7->Move(50, 250);
  p7->Resize(300, 50);
  p7->SetRange(0, 100);
  p7->SetDirection(AUIDirection::right);
  p7->SetBarColor(0xFF0088FF);
  p7->SetStripe(true);
  p7->SetStripeColor(0x50FFFFFF);
  p7->SetStripeWidth(8);
  p7->SetStripeSpeed(3);
  p7->SetRoundedCorners(false, 1);
  p7->SetTextFormat("%.0f%%");
  p7->SetProgressProvider([]() noexcept {
    static double p = 0.0;
    p += 1.8;
    if (p > 100.0) p = 0.0;
    return p;
  });
  // ---- 8: Vertical top, no text ----
  AProgressBar* p8 = AProgressBar::AttachTo(win);
  p8->Move(520, 50);
  p8->Resize(15, 200);
  p8->SetOrientation(AUIOrientation::vertical);
  p8->SetDirection(AUIDirection::top);
  p8->SetBarColor(0xFFFF44AA);
  p8->SetRoundedCorners(true, 15);
  p8->SetShowText(false);
  p8->SetProgressProvider([]() noexcept {
    static double p = 0.0;
    p += 0.03;
    if (p > 1.0) p = 0.0;
    return p;
  });
  // ---- Callbacks on p1 ----
  p1->SetOnStart([](double v) noexcept { printf("Started: %.0f%%\n", v*100); });
  p1->SetOnComplete([](double) noexcept { printf("Complete!\n"); });
  p1->SetOnProgressChanged([](double v) noexcept {
    static int last = -1;
    int pct = static_cast<int>(v*100);
    if (pct != last) { last = pct; printf("Progress: %d%%\n", pct); }
  });
  au->ProcessMessages();
  delete au;
  return 0;
}
