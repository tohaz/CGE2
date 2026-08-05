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

  void ALabel::OnDraw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
      int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const {
    D2("bufferW %u bufferH %u offsetX %d offsetY %d clip=(%d,%d)-(%d,%d)", bufferW, bufferH, offsetX, offsetY, clipL,
        clipT, clipR, clipB);
    AUI* au = Wnd()->EnginePtr();
    if(!au)
      return;
// Use absolute tree angle (matches AButton)
    double angleAbsDeg = AngleAbs();
    double angleRad = angleAbsDeg * M_PI / 180.0;// radians
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    FT_Face face = au->DefaultFontFace();
    if(!Text().empty() && face) {
      D2("draw text %s", Text().c_str());
      ARect textBounds { absX, absY, static_cast<uint32_t>(mSizeX), static_cast<uint32_t>(mSizeY) };
// Calculate intersection of parent clip and label bounds
      int32_t labelClipL = std::max(clipL, absX);
      int32_t labelClipR = std::min(clipR, absX + static_cast<int32_t>(mSizeX));
      int32_t labelClipT = std::max(clipT, absY);
      int32_t labelClipB = std::min(clipB, absY + static_cast<int32_t>(mSizeY));
      if(labelClipL < labelClipR && labelClipT < labelClipB) {
        ARect clipBounds { labelClipL, labelClipT, static_cast<uint32_t>(labelClipR - labelClipL),
            static_cast<uint32_t>(labelClipB - labelClipT) };
        ATextStyle textStyle { TextColor(), FontSize(), HAlign(), VAlign(), angleRad };
        DrawTextEx(buffer, bufferW, bufferH, textBounds, Text(), face, textStyle, &clipBounds);
      }
    }
  }


}// namespace aui
