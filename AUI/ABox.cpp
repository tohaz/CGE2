#include "AUILib.h"

namespace aui {

  ABox::ABox() {
    D3()
    X(AUI_BOX_X);
    Y(AUI_BOX_Y);
    SizeX(AUI_BOX_SZX);
    SizeY(AUI_BOX_SZY);
    Border(1);
    BGColor(AUI_BOX_BG);
    Type(AUIWidgetType::defaultBox);
    Text("some box");
  }

  void ABox::OnDraw(UNUSED uint32_t *buffer, UNUSED uint32_t bufferW, UNUSED uint32_t bufferH, UNUSED int32_t offsetX,
      UNUSED int32_t offsetY, UNUSED int32_t clipL, UNUSED int32_t clipT,
      UNUSED int32_t clipR, UNUSED int32_t clipB) const {
    D2("bufferW {} bufferH {} offsetX {} offsetY {} clipL {} clipT {} clipR {} clipB {}",
        bufferW, bufferH, offsetX, offsetY, clipL, clipT, clipR, clipB);
  }
}// namespace aui

