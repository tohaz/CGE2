#include "AUILib.h"

using namespace aui;

//std::string generate_random_alphanumeric(size_t length);
UNUSED static std::string generate_random_alphanumeric(size_t length) {
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
  uint32_t xs = 800, ys = 600;
  auto start = std::chrono::steady_clock::now();
  AUI *au;
//  au = AUI::Create("Table Demo2", AUIWindowType::X11);
//  au = AUI::Create("Table Demo2", AUIWindowType::Wayland);
  au = AUI::Create("Table Demo2");
  AWindow *w = au->MainWnd();
  w->EnableResize();
  w->Resize(xs, ys);
  ABox *bx = ABox::AttachTo(w);
  bx->Move(10, 10);
  bx->Resize(xs - 20, ys - 20);
  D1("init {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count());
  ATable *ta = ATable::AttachTo(bx);
  ta->AddColumns(2);
  ta->AddRows(2);
  ta->Move(10, 10);
  ta->Resize(xs - 40, ys - 40);
  ta->BGColor(0xFFFFFFFF);
  ta->AutoWiden(false);
  uint32_t nr = 100, nc = 100;
  ta->BeginBatch(nr * nc);
  {
    ST("table insertion of {}", nr * nc);
    for(uint32_t i = 0; i < nr; i++) {
      for(uint32_t j = 0; j < nc; j++) {
//        ta->CellData(i, j, "a");
        ta->CellData(i, j, generate_random_alphanumeric(10));
      }
    }
    ta->EndBatch();
  }
  ta->ScrollbarsToggle(true);
//  //1150ms for my setup for 10 random chars and million cells
  au->ProcessMessages();
  delete au;
  return 0;
}