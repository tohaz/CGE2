#include "AUILib.h"

using namespace aui;

std::string generate_random_alphanumeric(size_t length);
std::string generate_random_alphanumeric(size_t length) {
  const std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  static std::random_device rd;
  static std::mt19937 generator(rd());
  std::uniform_int_distribution<size_t> distribution(0, charset.size() - 1);
  std::string random_string;
  random_string.reserve(length);
  for(size_t i = 0; i < length; ++i) {
    random_string += charset[distribution(generator)];
  }
  return random_string;
}

int32_t main() {
  UNUSED AUI *au = AUI::Create("Table Demo2");
  AWindow *w = au->MainWnd();
  w->EnableResize();
  w->Resize(1024, 768);
  ABox *bx = ABox::AttachTo(w);
  bx->Move(0, 0);
  bx->Resize(1024, 768);
  auto start = std::chrono::steady_clock::now();

  UNUSED ATable *ta = ATable::AttachTo(bx);
  ta->AddColumns(2);
  ta->AddRows(2);
  ta->Resize(100, 100);
  ta->Move(0, 0);
  ta->Resize(1024, 768);
  ta->SetBGColor(0xFFFFFFFF);
  ta->SetAutoWiden(false);
  ta->BeginBatch();
    for(int32_t i = 0; i < 100000; i++) {
      for(int32_t j = 0; j < 10; j++)
//        ta->SetCellData(i, j, "a");
        ta->SetCellData(i, j, generate_random_alphanumeric(1));
  }
  ta->EndBatch();
  ta->SetScrollbarsEnabled(true);
  //1300ms for my setup for 10 random chars and million cells
  D1("init {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count());
  au->ProcessMessages();
  delete au;
  return 0;
}
