#include "AUILib.h"

using namespace aui;

std::string generate_random_alphanumeric(size_t length);
std::string generate_random_alphanumeric(size_t length) {
  static const std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  static std::random_device rd;
  static std::mt19937 generator(rd());
  static std::uniform_int_distribution<size_t> distribution(0, charset.size() - 1);
  std::string random_string;
  random_string.reserve(length);
  for(size_t i = 0; i < length; ++i) {
    random_string += charset[distribution(generator)];
  }
  return random_string;
}

int32_t main() {
  UNUSED AUI *au = AUI::Create("Table Demo");
  AWindow *w = au->MainWnd();
  w->EnableResize();
  w->Resize(1024, 768);
  auto start = std::chrono::steady_clock::now();
//  UNUSED ABox *bx = ABox::AttachTo(w);
//  bx->Move(0, 0);
//  bx->Resize(1024, 768);

  UNUSED AList *li = AList::AttachTo(w);

  for (int i = 0; i < 1000; ++i) {
    li->AddItem("Item " + std::to_string(i));
  }

  li->SetHAlignment(AUIHAlign::right);

  D1("init {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count());
  au->ProcessMessages();
  delete au;
  return 0;
}



