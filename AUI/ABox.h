#ifndef ABOX_H_
#define ABOX_H_

#include "AWidget.h"

namespace aui {

class ABox : public AWidget {
private:
  ABox();
public:
  static ABox* AttachTo(AWindow* parent);
  static ABox* AttachTo(AWidget* parent);
  void Draw(uint32_t* buffer, uint32_t parentWidth, uint32_t parentHeight,
            int32_t offsetX, int32_t offsetY) const override;
  void SetBorderThickness(uint32_t thick) { mBorderThick = thick; }
  void SetBorderColor(uint32_t color) { mBorderColor = color; }
};

} // namespace aui

#endif // ABOX_H_

