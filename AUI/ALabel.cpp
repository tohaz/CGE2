#include "AUILib.h"

namespace aui {

  ALabel::ALabel() {
    D2("ALabel constructed");
    mSizeX = 100;
    mSizeY = 28;
    mWidgetType = AUIWidgetType::defaultLabel;
    SetFontSize(14);
    mAngle = 0;
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
    D2("Attaching ALabel to widget");
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
    D2("unimplemented")
    return false;
  }

  void ALabel::OnMouseMove(UNUSED int32_t localX, UNUSED int32_t localY) {
    D2("unimplemented")
  }

  void ALabel::Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY) const {
    int32_t absX = offsetX + mX;
        int32_t absY = offsetY + mY;
// 1. Strict Guard Clause against out-of-bounds or negative rendering areas
    if(absX < 0 || absY < 0 || static_cast<uint32_t>(absX) >= parentWidth
        || static_cast<uint32_t>(absY) >= parentHeight) {
      return;
    }
    D3("[LABEL] Drawing with angle = {:.2f} deg", mAngle * 180.0 / M_PI);
// =========================================================================
// FAST-PATH OPTIMIZATION: Identity Configuration (0 degree rotation)
// =========================================================================
    if(std::abs(mAngle) < 0.001) {
// Safe subtraction now that absX and absY are guaranteed within bounds
      uint32_t drawW = std::min(mSizeX, parentWidth - static_cast<uint32_t>(absX));
      uint32_t drawH = std::min(mSizeY, parentHeight - static_cast<uint32_t>(absY));
      if(drawW > 0 && drawH > 0) {
        uint32_t bgColor = mBGColor & 0x00FFFFFFU;
        for(uint32_t row = 0; row < drawH; ++row) {
          size_t lineStart = (static_cast<size_t>(absY) + row) * parentWidth + static_cast<size_t>(absX);
          for(uint32_t col = 0; col < drawW; ++col) {
            buffer[lineStart + col] = bgColor;
          }
        }
      }
      DrawBorder(buffer, parentWidth, parentHeight, offsetX, offsetY);
      if(!mText.empty()) {
        AUI *engine = static_cast<AUI*>(mEnginePtr);
        if(engine) {
          std::unique_lock lock(mEnginePtr->GetFontMutex(), std::chrono::milliseconds(50));
          if (!lock.owns_lock()) { E("locked"); }

          FT_Face face = engine->GetDefaultFontFace();
          if(face) {
            ARect textBounds { absX + 4, absY + 2, mSizeX - 8, mSizeY - 4 };
            ATextStyle textStyle { mTextColor, mFontSize, mHAlign, mVAlign, 0.0 };
            DrawTextEx(buffer, parentWidth, parentHeight, textBounds, mText, face, textStyle);
          }
        }
      }
      return;
    }
// =========================================================================
// ROTATED RENDERING ENGINE PATH (Center Pivot Inverse Coordinate Mapping)
// =========================================================================
    uint32_t bgColor = mBGColor & 0x00FFFFFFU;
    double cx = static_cast<double>(absX) + (static_cast<double>(mSizeX) / 2.0);
    double cy = static_cast<double>(absY) + (static_cast<double>(mSizeY) / 2.0);
    double cosA = std::cos(-mAngle);
    double sinA = std::sin(-mAngle);
    double rX1 = -static_cast<double>(mSizeX) / 2.0, rY1 = -static_cast<double>(mSizeY) / 2.0;
    double rX2 = static_cast<double>(mSizeX) / 2.0, rY2 = static_cast<double>(mSizeY) / 2.0;
    double cosM = std::cos(mAngle);
    double sinM = std::sin(mAngle);
// Compute boundaries manually inline to prevent lambda scope definition bugs
    double cornerX1 = cx + (rX1 * cosM - rY1 * sinM);
    double cornerY1 = cy + (rX1 * sinM + rY1 * cosM);
    double cornerX2 = cx + (rX2 * cosM - rY1 * sinM);
    double cornerY2 = cy + (rX2 * sinM + rY1 * cosM);
    double cornerX3 = cx + (rX1 * cosM - rY2 * sinM);
    double cornerY3 = cy + (rX1 * sinM + rY2 * cosM);
    double cornerX4 = cx + (rX2 * cosM - rY2 * sinM);
    double cornerY4 = cy + (rX2 * sinM + rY2 * cosM);
    int32_t minX = std::max(0, static_cast<int32_t>(std::min( { cornerX1, cornerX2, cornerX3, cornerX4 })));
    int32_t maxX = std::min(static_cast<int32_t>(parentWidth) - 1, static_cast<int32_t>(std::max( { cornerX1, cornerX2,
        cornerX3, cornerX4 })));
    int32_t minY = std::max(0, static_cast<int32_t>(std::min( { cornerY1, cornerY2, cornerY3, cornerY4 })));
    int32_t maxY = std::min(static_cast<int32_t>(parentHeight) - 1, static_cast<int32_t>(std::max( { cornerY1, cornerY2,
        cornerY3, cornerY4 })));
    for(int32_t screenY = minY; screenY <= maxY; ++screenY) {
      size_t lineStart = static_cast<size_t>(screenY) * parentWidth;
      double dy = static_cast<double>(screenY) - cy;
      for(int32_t screenX = minX; screenX <= maxX; ++screenX) {
        double dx = static_cast<double>(screenX) - cx;
        double localX = (dx * cosA - dy * sinA) + (static_cast<double>(mSizeX) / 2.0);
        double localY = (dx * sinA + dy * cosA) + (static_cast<double>(mSizeY) / 2.0);
        if(localX >= 0.0 && localX < static_cast<double>(mSizeX) && localY >= 0.0
            && localY < static_cast<double>(mSizeY)) {
// Fix sign conversion: cast explicit size_t accumulation array offset
          buffer[lineStart + static_cast<size_t>(screenX)] = bgColor;
        }
      }
    }
    if(!mText.empty()) {
      AUI *engine = static_cast<AUI*>(mEnginePtr);
      if(engine) {
        std::unique_lock lock(mEnginePtr->GetFontMutex(), std::chrono::milliseconds(50));
        if (!lock.owns_lock()) { E("locked"); }
        FT_Face face = engine->GetDefaultFontFace();
        if(face) {
          ARect textBounds { absX + 4, absY + 2, mSizeX - 8, mSizeY - 4 };
          ATextStyle textStyle { mTextColor, mFontSize, mHAlign, mVAlign, mAngle };
          DrawTextEx(buffer, parentWidth, parentHeight, textBounds, mText, face, textStyle);
        }
      }
    }
  }

}// namespace aui
