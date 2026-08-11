#include "AUILib.h"

namespace aui {

  ALabel::ALabel() {
    D3("ALabel default constructor bgcolor {:x}", mBGColor);
    BGColor(AUI_DEFAULT_LABEL_BG);
    if(mX == 0 && mY == 0) {
      Move(AUI_DEFAULT_LABEL_X, AUI_DEFAULT_LABEL_Y);
    }
    if(mSizeX == 0) SizeX(AUI_DEFAULT_LABEL_SZX);
    if(mSizeY == 0) SizeY(AUI_DEFAULT_LABEL_SZY);
    Type(AUIWidgetType::defaultLabel);
    FontSize(14);
    Angle(0.0);
    Border(0);
    Text(AUI_DEFAULT_LABEL_TEXT);
  }

  ALabel::ALabel(const std::string& text) :
      ALabel() {
    D3("ALabel::ALabel(text)")
    Text(text);
  }

  ALabel::ALabel(const std::string& text, int32_t x, int32_t y, uint32_t szx, uint32_t szy) : ALabel(text) {
    D3("ALabel::ALabel(text, x, y, szx, szy)")
    Move(x, y);
    Resize(szx, szy);
    D3("ALabel::ALabel(text, x, y, szx, szy) done")
  }

  void ALabel::OnDraw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t, int32_t, int32_t clipL,
      int32_t clipT, int32_t clipR, int32_t clipB) const {
    D2("bufferW %u bufferH %u offsetX %d offsetY %d clip=(%d,%d)-(%d,%d)", bufferW, bufferH, offsetX, offsetY, clipL,
        clipT, clipR, clipB);
    AUI* au = Wnd()->EnginePtr();
    if(!au)
      return;
// Use local coordinates – we render into the widget's own buffer.
    int32_t localX = 0;
    int32_t localY = 0;
    FT_Face face = au->DefaultFontFace();
    if(!Text().empty() && face) {
      ARect textBounds { localX, localY, mSizeX, mSizeY };
// Clip is already in local coordinates (provided by RenderContent)
      int32_t labelClipL = std::max(clipL, localX);
      int32_t labelClipR = std::min(clipR, localX + static_cast<int32_t>(mSizeX));
      int32_t labelClipT = std::max(clipT, localY);
      int32_t labelClipB = std::min(clipB, localY + static_cast<int32_t>(mSizeY));
      if(labelClipL < labelClipR && labelClipT < labelClipB) {
        ARect clipBounds { labelClipL, labelClipT, static_cast<uint32_t>(labelClipR - labelClipL),
            static_cast<uint32_t>(labelClipB - labelClipT) };
// No rotation inside the local buffer – angle = 0
        ATextStyle textStyle { TextColor(), FontSize(), HAlign(), VAlign(), 0.0 };
        DrawTextEx(buffer, bufferW, bufferH, textBounds, Text(), face, textStyle, &clipBounds);
      }
    }
  }

}// namespace aui
