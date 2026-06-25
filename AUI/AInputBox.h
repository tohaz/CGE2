#ifndef AINPUTBOX_H
#define AINPUTBOX_H

namespace aui {
  class AInputBox;
  using OnChangeCallback = std::function<void(AInputBox*, const std::string& newValue)>;
  using OnSubmitCallback = std::function<void(AInputBox*, const std::string& value)>;
  class AInputBox: public AWidget {
    private:
      void BlinkThreadFunc();
      std::string mPlaceholder;
      uint32_t mPlaceholderColor = 0xFF888888;
      void InsertChar(char ch);
      void DeleteChar();// Backspace
      void DeleteForwardChar();// Delete
      std::unique_ptr<std::thread> mBlinkThread;
      std::atomic<bool> mBlinkingEnabled;
      std::atomic<bool> mStopBlinkThread;
      mutable std::atomic<bool> mCursorVisible;
      size_t mCursorPos;
      bool mInsertMode;
      bool mEditable;// separate from mEnabled
      size_t mMaxLength;// default 255
      std::optional<std::regex> mInputFilter;
      OnChangeCallback mOnChange;
      OnSubmitCallback mOnSubmit;
      std::string mLastNotifiedValue;
      static constexpr int32_t BLINK_INTERVAL_MS = 530;
      static constexpr size_t DEFAULT_MAX_LENGTH = 255;
      std::mutex mBlinkMutex;
      std::condition_variable mBlinkCV;
      int32_t MeasureTextWidth(const std::string &text) const;
      int32_t MeasureCharWidth(char ch) const;
      size_t GetIndexFromX(int32_t localX) const;
      bool mPasswordMode = false;
      char mMaskChar = '*';
      std::string GetDisplayText() const;
    public:
      AInputBox();
      ~AInputBox() override;
      static AInputBox* AttachTo(AWindow *parent);
      static AInputBox* AttachTo(AWidget *parent);
// ----- Overrides -----
      void Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX, int32_t offsetY) const
          override;
      void OnFocusGained();
      void OnFocusLost();
      bool OnMouseClick(int32_t localX, int32_t localY, bool pressed) override;
      void OnKeyEvent(const AUIKeyEvent &event) override;
      void Enable() override;
      void Disable() override;
// ----- Editable state (separate from enabled) -----
      void SetEditable(bool editable);
      bool IsEditable() const {return mEditable;}
// ----- Max length -----
      void SetMaxLength(size_t maxLen);
      size_t GetMaxLength() const {return mMaxLength;}
// ----- Input filtering (regex) -----
      void SetInputFilter(const std::string &regexPattern);
      void ClearInputFilter();
// ----- Cursor control -----
      void SetCursorBlinkingEnabled(bool enable);
      void SetCursorPos(size_t pos);
      size_t GetCursorPos() const;
// ----- Insert/Overwrite mode -----
      void SetInsertMode(bool insert);
      bool IsInsertMode() const;
// ----- Callbacks -----
      void SetOnChangeCallback(OnChangeCallback cb);
      void SetOnSubmitCallback(OnSubmitCallback cb);
// ----- Text manipulation -----
      virtual void SetText(const std::string &text);// resets cursor and validates max length/filter
      const std::string& GetText() const {return mText;}
      int32_t GetCursorX() const;
      bool IsInputAllowed(const std::string &newValue) const;
      bool IsLengthAllowed(const std::string &newValue) const;// max length check
      void SetValueAndNotify(const std::string &newValue);
      void SetPlaceholder(const std::string &placeholder);
      const std::string& GetPlaceholder() const {return mPlaceholder;}
      void SetPlaceholderColor(uint32_t color);
      void SetPasswordMode(bool enable, char maskChar = '*');
      bool IsPasswordMode() const {return mPasswordMode;}

  };

}// namespace aui

#endif // AINPUTBOX_H
