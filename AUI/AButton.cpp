#include "AUILib.h"

namespace aui {

  AButton::AButton() {
    D2("AButton constructed");
    mSizeX = 80;
    mSizeY = 28;
    mBGColor = 0xFFCCCCCC;
    mTextColor = 0xFF000000;
    mText = "Button";
    mHAlign = AUIHAlign::center;
    mVAlign = AUIVAlign::center;
    mFontSize = 14;
    mBorderThick = 2;
    mBorderColor = 0xFF888888;
    mWidgetType = AUIWidgetType::defaultButton;
  }

  AButton* AButton::AttachTo(AWindow *parent) {
    D1("Attaching AButton to window");
    if(!parent)
      E("AButton::AttachTo: parent window is null");
    AButton *btn = new AButton();
    parent->AddWidget(std::unique_ptr<AWidget>(btn));
    return btn;
  }

  AButton* AButton::AttachTo(AWidget *parent) {
    D1("Attaching AButton to widget");
    if(!parent)
      E("AButton::AttachTo: parent widget is null");
    AButton *btn = new AButton();
    parent->AddWidget(std::unique_ptr<AWidget>(btn));
    return btn;
  }

// ------------------------------------------------------------------
// Factory methods with text
// ------------------------------------------------------------------
  AButton* AButton::AttachTo(AWindow *parent, const std::string &text) {
    AButton *btn = AttachTo(parent);
    btn->SetText(text);
    return btn;
  }

  AButton* AButton::AttachTo(AWidget *parent, const std::string &text) {
    AButton *btn = AttachTo(parent);
    btn->SetText(text);
    return btn;
  }

  bool AButton::OnMouseClick(int32_t localX, int32_t localY, bool pressed) {
    D2("AButton click at ({},{}) pressed={}", localX, localY, pressed);
    if(pressed) {
      mPressed = true;
      if(mParentWindow)
        mParentWindow->Draw();
// Call base to invoke callback (if any)
      return AWidget::OnMouseClick(localX, localY, pressed);
    } else {
      mPressed = false;
      if(mParentWindow)
        mParentWindow->Draw();
      return false;
    }
  }

  void AButton::OnMouseMove(int32_t localX, int32_t localY) {
    bool inside = (localX >= 0 && localX < static_cast<int32_t>(mSizeX) && localY >= 0
        && localY < static_cast<int32_t>(mSizeY));
    if(mHoverEnabled && inside != mHovered) {
      mHovered = inside;
      if(mParentWindow)
        mParentWindow->Draw();
    }
  }

  void AButton::Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY) const {
// Determine background color
    uint32_t bgColor = mBGColor;
    if(mPressed && mClickHighlightEnabled) {
      bgColor = ShiftColor(mBGColor, true);// double shift for click
    } else if(mHovered && mHoverEnabled) {
      bgColor = ShiftColor(mBGColor, false);// single shift for hover
    }
// Clipping and background drawing
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    int32_t drawW = static_cast<int32_t>(mSizeX);
    int32_t drawH = static_cast<int32_t>(mSizeY);
    int32_t pW = static_cast<int32_t>(parentWidth);
    int32_t pH = static_cast<int32_t>(parentHeight);
    if(absX < 0) {
      drawW += absX;
      absX = 0;
    }
    if(absY < 0) {
      drawH += absY;
      absY = 0;
    }
    if(absX + drawW > pW)
      drawW = pW - absX;
    if(absY + drawH > pH)
      drawH = pH - absY;
    if(drawW > 0 && drawH > 0) {
      uint32_t bg = bgColor & 0x00FFFFFFU;
      size_t totalPixels = static_cast<size_t>(pW) * static_cast<size_t>(pH);
      for (int32_t row = 0; row < drawH; ++row) {
        size_t lineStart = static_cast<size_t>(absY + row) * (size_t) pW + static_cast<size_t>(absX);
        for (int32_t col = 0; col < drawW; ++col) {
          size_t idx = lineStart + static_cast<size_t>(col);
          if(idx < totalPixels)
            buffer[idx] = bg;
        }
      }
    }
    DrawBorder(buffer, parentWidth, parentHeight, offsetX, offsetY);
    DrawText(buffer, parentWidth, parentHeight, offsetX, offsetY);
  }

  void AButton::OnMouseLeave() {
    if(mHovered) {
      mHovered = false;
      if(mParentWindow)
        mParentWindow->Draw();
    }
  }

}// namespace aui

