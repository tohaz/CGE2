#ifndef ALABEL_H_
#define ALABEL_H_

namespace aui {

class ALabel : public AWidget {
private:
  mutable int32_t mCachedTextWidth = 0;
  mutable int32_t mCachedTextHeight = 0;
  mutable bool mTextMetricsValid = false;
public:
  ALabel();
  ~ALabel() override = default;
  void Draw(uint32_t* buffer, uint32_t parentWidth, uint32_t parentHeight,
            int32_t offsetX, int32_t offsetY) const override;
  bool OnMouseClick(int32_t localX, int32_t localY, bool pressed);
  void OnMouseMove(int32_t localX, int32_t localY);
  static ALabel* AttachTo(AWindow* parent);
  static ALabel* AttachTo(AWidget* parent);
  static ALabel* AttachTo(AWindow* parent, const std::string& text,
                          int32_t x, int32_t y, uint32_t w, uint32_t h);
  static ALabel* AttachTo(AWidget* parent, const std::string& text,
                          int32_t x, int32_t y, uint32_t w, uint32_t h);
  static ALabel* AttachTo(AWindow* parent, const std::string& text);
  static ALabel* AttachTo(AWidget* parent, const std::string& text);
};

} // namespace aui

#endif // ALABEL_H_
