#ifndef IWINDOWCONTEXT_H_
#define IWINDOWCONTEXT_H_

namespace aui {
  class AUI;
  class AWindow;
class IWindowContext {
  protected:
    AUI* mAUI;
    AWindow* mWindow;           // owning window (to get size/position)
  public:
    virtual ~IWindowContext() = default;
    virtual bool CreateFrame(uint32_t width, uint32_t height, const std::string& title) = 0;
    virtual void DestroyFrame() = 0;
    virtual void Move(int32_t x, int32_t y) = 0;
    virtual void Resize(uint32_t width, uint32_t height) = 0;
    virtual void SetTitle(const std::string& title) = 0;
    virtual uint64_t GetNativeWindowId() const = 0;
    virtual uint32_t* GetSoftwareBuffer() = 0;
    virtual void QueueFrameCommit() = 0;
    AUI* GetEnginePtr() const {return mAUI;}
    virtual void ProcessEvent(void* ev) = 0;
    void SetWindow(AWindow* win) { mWindow = win; }
    AWindow* Wnd() {return mWindow;}
    virtual void EnableResize() = 0;
    virtual void DisableResize() = 0;
    virtual void SetCursor(AUICursorType type) = 0;
    virtual bool EnsureBuffer(uint32_t width, uint32_t height) = 0;
};

} // namespace aui

#endif // IWINDOWCONTEXT_H_

