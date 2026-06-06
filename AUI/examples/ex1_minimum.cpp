#include "AUILib.h"
//
using namespace aui;

int main() {
  AUI* au = AUI::Create("example1");
  au->ProcessMessages();

  delete au;
  au = 0; // i know
  return 0;
}


