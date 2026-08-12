#include "AUILib.h"

namespace aui {

  AButton::AButton() {
    D2("AButton constructing");
    mSizeX = AUI_DEFAULT_BUTTON_SZX;
    mSizeY = AUI_DEFAULT_BUTTON_SZY;
    mX = AUI_DEFAULT_BUTTON_X;
    mY = AUI_DEFAULT_BUTTON_Y;
    mBGColor = AUI_DEFAULT_BUTTON_BG;
    mTextColor = 0xFF000000;
    mText = "some button";
    mHAlign = AUIHAlign::center;
    mVAlign = AUIVAlign::center;
    mFontSize = 14;
    mBorderThick = AUI_DEFAULT_BUTTON_BORDERW;
    mBorderColor = 0xFF333333;
    mType = AUIWidgetType::defaultButton;
    DefaultFillBG(false);
    DefaultDrawBorder(false);
    HLToggle(true);
    BGColor2(DarkenColor(BGColor()));
    MouseLeftReleaseRequired(true);
    ConsumeMouseEvents(true);
    D2("bgcolor {:x} bgcolor2 {:x}", BGColor(), BGColor2())
  }

  AButton::AButton(std::string text) :
      AButton() {
    mText = text;
  }

  void AButton::OnDraw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
      int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const {
    D4("bufferW %u bufferH %u offsetX %d offsetY %d clip=(%d,%d)-(%d,%d)", bufferW, bufferH, offsetX, offsetY, clipL,
        clipT, clipR, clipB);
    AUI* au = Wnd()->EnginePtr();
    if(!au)
      return;
// 1. Background – draw at (0,0) in local buffer
    uint32_t bgColor = HL() ? HLColor(BGColor()) : BGColor();
    bgColor = mPressed ? BGColor2() : bgColor;
    FillRect(buffer, bufferW, 0, 0, SafeINT32(mSizeX), SafeINT32(mSizeY), bgColor);
// 2. Border – use the incoming offset so that offset + X() = 0
    if(Border() != 0) {
      DrawBorder(buffer, bufferW, bufferH, offsetX, offsetY, clipL, clipT, clipR, clipB);
    }
// 3. Text – draw at (0,0) with angle=0 (rotation will be applied later)
    FT_Face face = au->DefaultFontFace();
    if(!Text().empty() && face) {
      uint32_t textW = (mSizeX > 8u) ? mSizeX - 8u : 0u;
      uint32_t textH = (mSizeY > 4u) ? mSizeY - 4u : 0u;
      ARect textBounds { 4, 2, textW, textH };
      int32_t btnClipL = std::max(clipL, 0);
      int32_t btnClipR = std::min(clipR, static_cast<int32_t>(mSizeX));
      int32_t btnClipT = std::max(clipT, 0);
      int32_t btnClipB = std::min(clipB, static_cast<int32_t>(mSizeY));
      if(btnClipL < btnClipR && btnClipT < btnClipB) {
        ARect clipBounds { btnClipL, btnClipT, static_cast<uint32_t>(btnClipR - btnClipL),
            static_cast<uint32_t>(btnClipB - btnClipT) };
        ATextStyle textStyle { TextColor(), FontSize(), HAlign(), VAlign(), 0.0 };
        DrawTextEx(buffer, bufferW, bufferH, textBounds, Text(), face, textStyle, &clipBounds);
      }
    }
  }

  void AButton::OnMouseDownLeftInternal(UNUSED int32_t localX, UNUSED int32_t localY) {
    D4()
    Pressed(true);
  }

  void AButton::OnMouseUpLeftInternal(UNUSED int32_t localX, UNUSED int32_t localY) {
    D4()
    Pressed(false);
  }

}// namespace aui

