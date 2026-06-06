#ifndef ABUTTON_H_
#define ABUTTON_H_

#include "AWidget.h"

namespace aui {

class AButton : public AWidget {
private:
  bool mClickHighlightEnabled = true;   // specific to button (press highlight)
public:
  AButton();
  ~AButton() override = default;

  // AWidget overrides
  void Draw(uint32_t* buffer, uint32_t parentWidth, uint32_t parentHeight,
            int32_t offsetX, int32_t offsetY) const override;
  bool OnMouseClick(int32_t localX, int32_t localY, bool pressed) override;
  void OnMouseMove(int32_t localX, int32_t localY) override;

  // Factory methods (basic)
  static AButton* AttachTo(AWindow* parent);
  static AButton* AttachTo(AWidget* parent);

  // Factory methods with text
  static AButton* AttachTo(AWindow* parent, const std::string& text);
  static AButton* AttachTo(AWidget* parent, const std::string& text);

  // Highlight configuration (uses base class mHoverEnabled, plus click highlight)
  void SetClickHighlightEnabled(bool enable) { mClickHighlightEnabled = enable; }
  void OnMouseLeave() override;
};

} // namespace aui

#endif // ABUTTON_H_
