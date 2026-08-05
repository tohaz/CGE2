#ifndef ABOX_H_
#define ABOX_H_

namespace aui {

class ABox : public AWidgetFactory<ABox> {
    friend class AWidget;
    friend class AWidgetFactory<ABox>;
    private:
    protected:
      ABox();
    public:
      void OnDraw(UNUSED uint32_t *buffer, UNUSED uint32_t bufferW, UNUSED uint32_t bufferH, UNUSED int32_t offsetX,
          UNUSED int32_t offsetY, UNUSED int32_t clipL, UNUSED int32_t clipT,
          UNUSED int32_t clipR, UNUSED int32_t clipB) const;
};

} // namespace aui

#endif // ABOX_H_

