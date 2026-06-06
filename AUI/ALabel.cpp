#include "AUILib.h"

namespace aui {

// ------------------------------------------------------------------
// Constructor
// ------------------------------------------------------------------
  ALabel::ALabel() {
    D2("ALabel constructed");
    mSizeX = 100;
    mSizeY = 28;
    mWidgetType = AUIWidgetType::defaultLabel;
  }

// ------------------------------------------------------------------
// Static factory methods (base, no text/geometry)
// ------------------------------------------------------------------
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

// ------------------------------------------------------------------
// Overloaded factory methods (with text and geometry)
// ------------------------------------------------------------------
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

// ------------------------------------------------------------------
// Event handlers (passive)
// ------------------------------------------------------------------
  bool ALabel::OnMouseClick(UNUSED int32_t localX, UNUSED int32_t localY, UNUSED bool pressed) {
// No default action
    return false;
  }

  void ALabel::OnMouseMove(UNUSED int32_t localX, UNUSED int32_t localY) {
// No default action
  }
// ------------------------------------------------------------------
// Main Draw: background, border, text
// ------------------------------------------------------------------
  void ALabel::Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY) const {
// Draw background (optional – can rely on parent? Actually label should fill its area)
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    uint32_t drawW = std::min(mSizeX, parentWidth - static_cast<uint32_t>(absX));
    uint32_t drawH = std::min(mSizeY, parentHeight - static_cast<uint32_t>(absY));
    if(drawW > 0 && drawH > 0) {
      uint32_t bgColor = mBGColor & 0x00FFFFFFU;
      for (uint32_t row = 0; row < drawH; ++row) {
        size_t lineStart = (static_cast<size_t>(absY) + row) * parentWidth + static_cast<size_t>(absX);
        for (uint32_t col = 0; col < drawW; ++col) {
          buffer[lineStart + col] = bgColor;
        }
      }
    }
    DrawBorder(buffer, parentWidth, parentHeight, offsetX, offsetY);
    DrawText(buffer, parentWidth, parentHeight, offsetX, offsetY);
  }

}// namespace aui
