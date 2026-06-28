#include "AUILib.h"

namespace aui {

  ABox::ABox() {
    D3()
    mX = AUI_BOX_X;
    mY = AUI_BOX_Y;
    mSizeX = AUI_BOX_SZX;
    mSizeY = AUI_BOX_SZY;
    mBorderThick = 1;
  }

  ABox* ABox::AttachTo(AWindow *parent) {
    D2("attaching widget")
    if(!parent)
      E("");
    ABox *box = new ABox();
    parent->AddWidget(std::unique_ptr<AWidget>(box));
    return box;
  }

  ABox* ABox::AttachTo(AWidget *parent) {
    if(!parent)
      E("");
    auto *box = new ABox();
    parent->AddWidget(std::unique_ptr<AWidget>(box));
    return box;
  }

  void ABox::Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX, int32_t offsetY) const {
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    if(absX < 0 || absY < 0 || static_cast<uint32_t>(absX) >= parentWidth
        || static_cast<uint32_t>(absY) >= parentHeight) {
      return;
    }
    uint32_t drawW = std::min(mSizeX, parentWidth - static_cast<uint32_t>(absX));
    uint32_t drawH = std::min(mSizeY, parentHeight - static_cast<uint32_t>(absY));
    if(drawW == 0 || drawH == 0)
      return;
    uint32_t bg = mBGColor & 0x00FFFFFF;
    for(uint32_t row = 0; row < drawH; ++row) {
      size_t lineStart = (static_cast<size_t>(absY) + row) * parentWidth + static_cast<size_t>(absX);
      for(uint32_t col = 0; col < drawW; ++col) {
        buffer[lineStart + col] = bg;
      }
    }
    if(mBorderThick > 0) {
      uint32_t border = mBorderColor & 0x00FFFFFF;
      for(uint32_t t = 0; t < mBorderThick && t < drawH; ++t) {
        size_t topLine = (static_cast<size_t>(absY) + t) * parentWidth + static_cast<size_t>(absX);
        size_t bottomLine = (static_cast<size_t>(absY) + drawH - 1 - t) * parentWidth + static_cast<size_t>(absX);
        for(uint32_t col = 0; col < drawW; ++col) {
          buffer[topLine + col] = border;
          buffer[bottomLine + col] = border;
        }
      }
      for(uint32_t t = 0; t < mBorderThick && t < drawW; ++t) {
        for(uint32_t row = mBorderThick; row < drawH - mBorderThick; ++row) {
          size_t leftPixel = (static_cast<size_t>(absY) + row) * parentWidth + static_cast<size_t>(absX) + t;
          size_t rightPixel = (static_cast<size_t>(absY) + row) * parentWidth + static_cast<size_t>(absX) + drawW - 1
              - t;
          buffer[leftPixel] = border;
          buffer[rightPixel] = border;
        }
      }
    }
    DrawChildren(buffer, parentWidth, parentHeight, offsetX, offsetY);
  }

  void ABox::OnMouseWheel(int32_t delta) {
    AWidget::OnMouseWheel(delta);
    ForwardWheelToChildren(delta);
  }

  void ABox::OnMouseMove(int32_t localX, int32_t localY) {
    D2()
    AWidget::OnMouseMove(localX, localY);
    ForwardMoveToChildren(localX, localY);
  }

}// namespace aui

