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
    D2("bgcolor {:x} bgcolor2 {:x}", BGColor(), BGColor2())
  }

  AButton::AButton(std::string text) :
      AButton() {
    mText = text;
  }

  void AButton::OnDraw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
      int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const {
    D2("bufferW %u bufferH %u offsetX %d offsetY %d clip=(%d,%d)-(%d,%d)", bufferW, bufferH, offsetX, offsetY, clipL,
        clipT, clipR, clipB);
    AUI* au = Wnd()->EnginePtr();
    if(!au)
      return;
// Absolute position of the button
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
// ---------------------------
// 1. Draw background
// ---------------------------
    uint32_t bgColor = HL() ? HLColor(BGColor()) : BGColor();
    D4("HL {}", HL())
    bgColor = mPressed ? BGColor2() : bgColor;
    double angleAbsDeg = AngleAbs();
    double angleRad = angleAbsDeg * M_PI / 180.0;
    double parentAngle = Parent() ? Parent()->AngleAbs() : 0.0;
    if(std::abs(angleAbsDeg) < 1e-6 && std::abs(parentAngle) < 1e-6) {
      int32_t wLeft = absX;
      int32_t wTop = absY;
      int32_t wRight = absX + static_cast<int32_t>(SizeX());
      int32_t wBottom = absY + static_cast<int32_t>(SizeY());
// Intersect with incoming clip and buffer limits
      int32_t left = std::max( { wLeft, clipL, 0 });
      int32_t top = std::max( { wTop, clipT, 0 });
      int32_t right = std::min( { wRight, clipR, static_cast<int32_t>(bufferW) });
      int32_t bottom = std::min( { wBottom, clipB, static_cast<int32_t>(bufferH) });
      int32_t drawW = right - left;
      int32_t drawH = bottom - top;
      if(drawW > 0 && drawH > 0) {
        FillRect(buffer, bufferW, left, top, drawW, drawH, bgColor);
      }
    }
    else {
      DrawRotatedRect(buffer, bufferW, 0, 0, static_cast<int32_t>(bufferW), static_cast<int32_t>(bufferH), X(), Y(),
          SizeX(), SizeY(), angleAbsDeg, Parent() ? Parent()->AbsX() : 0, Parent() ? Parent()->AbsY() : 0,
          Parent() ? Parent()->SizeX() : bufferW, Parent() ? Parent()->SizeY() : bufferH, parentAngle, bgColor);
    }
    if(Border() != 0) {
      if(std::abs(angleAbsDeg) < 1e-6 && std::abs(parentAngle) < 1e-6) {
// Use the existing non‑rotated border drawing
        int32_t effOffsetX = absX - X();
        int32_t effOffsetY = absY - Y();
        DrawBorder(buffer, bufferW, bufferH, effOffsetX, effOffsetY, clipL, clipT, clipR, clipB);
      }
      else {
// Rotated border
        DrawRotatedBorder(buffer, bufferW, bufferH, clipL, clipT, clipR, clipB, X(), Y(), SizeX(), SizeY(), Border(),
            BorderColor(), angleAbsDeg, Parent() ? Parent()->AbsX() : 0, Parent() ? Parent()->AbsY() : 0,
            Parent() ? Parent()->SizeX() : bufferW, Parent() ? Parent()->SizeY() : bufferH, parentAngle);
      }
    }
    FT_Face face = au->DefaultFontFace();
    if(!Text().empty() && face) {
      uint32_t textW = (SizeX() > 8u) ? static_cast<uint32_t>(SizeX() - 8u) : 0u;
      uint32_t textH = (SizeY() > 4u) ? static_cast<uint32_t>(SizeY() - 4u) : 0u;
      ARect textBounds { absX + 4, absY + 2, textW, textH };
      int32_t btnClipL = std::max(clipL, absX);
      int32_t btnClipR = std::min(clipR, absX + static_cast<int32_t>(SizeX()));
      int32_t btnClipT = std::max(clipT, absY);
      int32_t btnClipB = std::min(clipB, absY + static_cast<int32_t>(SizeY()));
      if(btnClipL < btnClipR && btnClipT < btnClipB) {
        ARect clipBounds { btnClipL, btnClipT, static_cast<uint32_t>(btnClipR - btnClipL),
            static_cast<uint32_t>(btnClipB - btnClipT) };
        ATextStyle textStyle { TextColor(), FontSize(), HAlign(), VAlign(), angleRad };
        DrawTextEx(buffer, bufferW, bufferH, textBounds, Text(), face, textStyle, &clipBounds);
      }
    }
  }

  void AButton::OnMouseDownLeftInternal(UNUSED int32_t localX, UNUSED int32_t localY) {
    D2()
    Pressed(true);
  }

  void AButton::OnMouseUpLeftInternal(UNUSED int32_t localX, UNUSED int32_t localY) {
    D2()
    Pressed(false);
  }

}// namespace aui

