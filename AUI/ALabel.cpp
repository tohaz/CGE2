#include "AUILib.h"

namespace aui {

  ALabel::ALabel() {
    D2("ALabel constructed");
    mSizeX = 100;
    mSizeY = 28;
    mWidgetType = AUIWidgetType::defaultLabel;
  }

  ALabel* ALabel::AttachTo(AWindow *parent) {
    D2("Attaching ALabel to window");
    if(!parent)
      E("ALabel::AttachTo: parent window is null");
    ALabel *label = new ALabel();
    parent->AddWidget(std::unique_ptr<AWidget>(label));
    return label;
  }

  ALabel* ALabel::AttachTo(AWidget *parent) {
    D1("Attaching ALabel to widget");
    if(!parent)
      E("ALabel::AttachTo: parent widget is null");
    ALabel *label = new ALabel();
    parent->AddWidget(std::unique_ptr<AWidget>(label));
    return label;
  }

  ALabel* ALabel::AttachTo(AWindow *parent, const std::string &text, int32_t x, int32_t y, uint32_t w, uint32_t h) {
    ALabel *label = AttachTo(parent);
    label->SetText(text);
    label->Move(x, y);
    label->Resize(w, h);
    return label;
  }

  ALabel* ALabel::AttachTo(AWidget *parent, const std::string &text, int32_t x, int32_t y, uint32_t w, uint32_t h) {
    ALabel *label = AttachTo(parent);
    label->SetText(text);
    label->Move(x, y);
    label->Resize(w, h);
    return label;
  }

  ALabel* ALabel::AttachTo(AWindow *parent, const std::string &text) {
    ALabel *label = AttachTo(parent);
    label->SetText(text);
    return label;
  }

  ALabel* ALabel::AttachTo(AWidget *parent, const std::string &text) {
    ALabel *label = AttachTo(parent);
    label->SetText(text);
    return label;
  }

  bool ALabel::OnMouseClick(UNUSED int32_t localX, UNUSED int32_t localY, UNUSED bool pressed) {
    return false;
  }

  void ALabel::OnMouseMove(UNUSED int32_t localX, UNUSED int32_t localY) {
  }

  void ALabel::Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
                    int32_t offsetY) const {
      int32_t absX = offsetX + mX;
      int32_t absY = offsetY + mY;
      uint32_t drawW = std::min(mSizeX, parentWidth - static_cast<uint32_t>(absX));
      uint32_t drawH = std::min(mSizeY, parentHeight - static_cast<uint32_t>(absY));

      // ---- Draw background ----
      if (drawW > 0 && drawH > 0) {
          uint32_t bgColor = mBGColor & 0x00FFFFFFU;
          for (uint32_t row = 0; row < drawH; ++row) {
              size_t lineStart = (static_cast<size_t>(absY) + row) * parentWidth + static_cast<size_t>(absX);
              for (uint32_t col = 0; col < drawW; ++col) {
                  buffer[lineStart + col] = bgColor;
              }
          }
      }
      DrawBorder(buffer, parentWidth, parentHeight, offsetX, offsetY);

      // ---- Draw text ----
      if (!mText.empty()) {
          AUI *engine = static_cast<AUI*>(mEnginePtr);
          if (engine) {
              FT_Face face = engine->GetDefaultFontFace();
              if (face) {
                  // Add small padding
                  int32_t textX = absX + 4;
                  int32_t textY = absY + 2;
                  int32_t textW = static_cast<int32_t>(drawW) - 8;
                  int32_t textH = static_cast<int32_t>(drawH) - 4;
                  if (textW > 0 && textH > 0) {
                      DrawTextEx(buffer, parentWidth, parentHeight,
                                 textX, textY, textW, textH,
                                 mText, face, mFontSize,
                                 mHAlign, mVAlign, 0, mTextColor, textW);
                  }
              }
          }
      }
  }
}// namespace aui
