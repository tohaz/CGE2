#ifndef AINPUTBOX_H
#define AINPUTBOX_H

namespace aui {
  class AInputBox;
  using OnChangeCallback = std::function<void(AInputBox* wi, const std::string& newValue)>;
  using OnSubmitCallback = std::function<void(AInputBox* wi, const std::string& value)>;

  class AInputBox: public AWidgetFactory<AInputBox> {
      friend class AWidgetFactory<AInputBox>;
      friend class AComboBox;
    private:
      void BlinkThreadFunc();
      std::string mPlaceholder = "";
      uint32_t mPlaceholderColor = 0xFF888888;
      void InsertChar(int8_t ch);
      void DeleteChar();// Backspace
      void DeleteForwardChar();// Delete
      std::unique_ptr<std::thread> mBlinkThread;
      std::atomic<bool> mBlinkingEnabled;
      std::atomic<bool> mStopBlinkThread;
      mutable std::atomic<bool> mCursorVisible;
      size_t mCursorPos = 0;
      bool mInsertMode = 0;
      bool mEditable = false;
      size_t mMaxLength = 255;// default 255
      std::optional<std::regex> mInputFilter;
      OnChangeCallback mOnChange = nullptr;
      OnSubmitCallback mOnSubmit = nullptr;
      std::string mLastNotifiedValue = "";
      static constexpr int32_t BLINK_INTERVAL_MS = 530;
      static constexpr size_t DEFAULT_MAX_LENGTH = 255;
      std::mutex mBlinkMutex;
      std::condition_variable mBlinkCV;
      int32_t MeasureTextWidth(const std::string &text) const;
      int32_t MeasureCharWidth(char ch) const;
      size_t IndexFromX(int32_t localX) const;
      bool mPasswordMode = false;
      char mMaskChar = '*';
      std::string DisplayText() const;
    protected:
      AInputBox();
    public:
      void OnDraw(UNUSED uint32_t *buffer, UNUSED uint32_t bufferW, UNUSED uint32_t bufferH, UNUSED int32_t offsetX,
          UNUSED int32_t offsetY, UNUSED int32_t clipL, UNUSED int32_t clipT,
          UNUSED int32_t clipR, UNUSED int32_t clipB) const;
      ~AInputBox();
//// ----- Overrides -----
//      void Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX, int32_t offsetY) const
//          override;
      void OnFocusGained();
      void OnFocusLost();
      AWidget* OnMouseDownLeft(UNUSED int32_t localX, UNUSED int32_t localY) override;
      void OnKeyEvent(const AUIKeyEvent &event) override;
      void Enable() override;
      void Disable() override;
      void Editable(bool editable);
      void InsertMode(bool insert);
      bool IsEditable() const {return mEditable;}
      void MaxLength(size_t maxLen);
      size_t MaxLength() const {return mMaxLength;}
      void InputFilter(const std::string &regexPattern);
      void ClearInputFilter();
      void CursorBlinkingEnabled(bool enable);
      void CursorPos(size_t pos);
      size_t CursorPos() const;
      bool IsInsertMode() const;
      void SetOnChangeCallback(OnChangeCallback cb);
      void SetOnSubmitCallback(OnSubmitCallback cb);
      void Text(const std::string &text);// resets cursor and validates max length/filter
      std::string Text() {return AWidget::Text();}// resets cursor and validates max length/filter
      int32_t CursorX() const;
      bool InputAllowed(const std::string& newValue) const;
      bool LengthAllowed(const std::string& newValue) const;// max length check
      void SetValueAndNotify(const std::string& newValue);
      void Placeholder(const std::string &placeholder);
      const std::string& GetPlaceholder() const {return mPlaceholder;}
      void PlaceholderColor(uint32_t color);
      void PasswordMode(bool enable, int8_t maskChar = '*');
      bool PasswordMode() const {return mPasswordMode;}
  };

}// namespace aui

#endif // AINPUTBOX_H
