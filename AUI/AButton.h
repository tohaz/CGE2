#ifndef ABUTTON_H_
#define ABUTTON_H_

namespace aui {
  class AButton: public AWidgetFactory<AButton> {
      friend class AWidgetFactory<AButton>;
    private:
      void OnMouseUpLeftInternal(int32_t localX, int32_t localY);
      void OnMouseDownLeftInternal(int32_t localX, int32_t localY);

    protected:
      AButton();
      AButton(std::string);
    public:
      virtual void OnDraw(uint32_t *buffer, uint32_t bufferW, uint32_t bufferH,
          int32_t offsetX, int32_t offsetY, int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const;

  };

}// namespace aui

#endif // ABUTTON_H_
