#ifndef ALABEL_H_
#define ALABEL_H_

namespace aui {

class ALabel : public AWidgetFactory<ALabel> {
  friend class AWidgetFactory<ALabel>;
private:
  mutable int32_t mCachedTextWidth = 0;
  mutable int32_t mCachedTextHeight = 0;
  mutable bool mTextMetricsValid = false;
protected:
  ALabel();
  ALabel(const std::string& text);
  ALabel(const std::string& text, int32_t x, int32_t y, uint32_t szx, uint32_t szy);
public:
  ~ALabel() override = default;
  using AWidgetFactory<ALabel>::AttachTo;
  virtual void OnDraw(uint32_t *buffer, uint32_t bufferW, uint32_t bufferH,
      int32_t offsetX, int32_t offsetY, int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const;
};

} // namespace aui

#endif // ALABEL_H_
