#include "AUILib.h"

namespace aui {

// ------------------------------------------------------------------
// UTF-8 code point counter (fallback to byte length for simplicity)
// ------------------------------------------------------------------
  static size_t utf8_length(const std::string &str) {
// Replace with proper UTF-8 length function if needed
    return str.length();
  }

// ------------------------------------------------------------------
// Constructor / Destructor
// ------------------------------------------------------------------
  AInputBox::AInputBox() :
      mBlinkingEnabled(true), mStopBlinkThread(false), mCursorVisible(true), mCursorPos(0), mInsertMode(true), mEditable(
          true), mMaxLength(DEFAULT_MAX_LENGTH) {
    mWidgetType = AUIWidgetType::defaultInputBox;
    mHAlign = AUIHAlign::right;
    mVAlign = AUIVAlign::center;
    mX = AUI_DEFAULT_INPUT_X;
    mY = AUI_DEFAULT_INPUT_Y;
    mSizeX = AUI_DEFAULT_INPUT_SZX;
    mSizeY = AUI_DEFAULT_INPUT_SZY;
    mBGColor = AUI_DEFAULT_INPUT_BG;
    mBorderThick = AUI_DEFAULT_INPUT_BORDERW;
    SetFocusable(true);
    SetEditable(true);
// Assume AWidget has SetFocusable(true); if not, add a flag manually
// SetFocusable(true);
    mBlinkThread = std::make_unique<std::thread>(&AInputBox::BlinkThreadFunc, this);
  }

// ------------------------------------------------------------------
// Factory methods
// ------------------------------------------------------------------
  AInputBox* AInputBox::AttachTo(AWindow *parent) {
    if(!parent) {
      E("AInputBox::AttachTo: parent window is null");
      return nullptr;
    }
    auto *box = new AInputBox();
    parent->AddWidget(std::unique_ptr<AWidget>(box));
    return box;
  }

  AInputBox* AInputBox::AttachTo(AWidget *parent) {
    if(!parent) {
      E("AInputBox::AttachTo: parent widget is null");
      return nullptr;
    }
    auto *box = new AInputBox();
    parent->AddWidget(std::unique_ptr<AWidget>(box));
    return box;
  }

// ------------------------------------------------------------------
// AWidget overrides
// ------------------------------------------------------------------
  void AInputBox::Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY) const {
// 1. Background (with disabled dimming)
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    uint32_t bgColor = mEnabled ? mBGColor : ShiftColor(mBGColor, true);
    int32_t drawW = static_cast<int32_t>(mSizeX);
    int32_t drawH = static_cast<int32_t>(mSizeY);
    int32_t pW = static_cast<int32_t>(parentWidth);
    int32_t pH = static_cast<int32_t>(parentHeight);
    if(absX < 0) {
      drawW += absX;
      absX = 0;
    }
    if(absY < 0) {
      drawH += absY;
      absY = 0;
    }
    if(absX + drawW > pW)
      drawW = pW - absX;
    if(absY + drawH > pH)
      drawH = pH - absY;
    if(drawW > 0 && drawH > 0 && absX >= 0 && absY >= 0) {
      size_t maxIdx = static_cast<size_t>(pW) * static_cast<size_t>(pH);
      for(int32_t row = 0; row < drawH; ++row) {
        size_t lineStart = static_cast<size_t>(absY + row) * static_cast<size_t>(pW) + static_cast<size_t>(absX);
        for(int32_t col = 0; col < drawW; ++col) {
          size_t idx = lineStart + static_cast<size_t>(col);
          if(idx < maxIdx)
            buffer[idx] = bgColor;
        }
      }
    }
// 2. Border
    DrawBorder(buffer, parentWidth, parentHeight, offsetX, offsetY);
// 3. Client area (excluding border)
    int32_t clientX = absX + static_cast<int32_t>(mBorderThick);
    int32_t clientY = absY + static_cast<int32_t>(mBorderThick);
    int32_t clientW = static_cast<int32_t>(mSizeX) - 2 * static_cast<int32_t>(mBorderThick);
    int32_t clientH = static_cast<int32_t>(mSizeY) - 2 * static_cast<int32_t>(mBorderThick);
    if(clientW <= 0 || clientH <= 0)
      return;
// 4. Draw text or placeholder (with dimmed colors when disabled)
    FT_Face face = mEnginePtr ? mEnginePtr->GetDefaultFontFace() : nullptr;
    if(face) {
      UNUSED uint32_t textColor = mEnabled ? mTextColor : ShiftColor(mTextColor, true);
      UNUSED uint32_t placeholderColor = mEnabled ? mPlaceholderColor : ShiftColor(mPlaceholderColor, true);

      std::string displayText = GetDisplayText();// handles password mode
      if(!displayText.empty()) {
        DrawTextEx(buffer, parentWidth, parentHeight, clientX, clientY, clientW, clientH, displayText, face, mFontSize,
            mHAlign, mVAlign, 0, textColor, clientW);
      }
      else if(!mPlaceholder.empty() && !IsFocused()) {
        DrawTextEx(buffer, parentWidth, parentHeight, clientX, clientY, clientW, clientH, mPlaceholder, face, mFontSize,
            mHAlign, mVAlign, 0, placeholderColor, clientW);
      }
    }
// 5. Draw cursor (only when enabled)
    if(mEnabled && IsFocused() && mCursorVisible) {
      int32_t cursorX = GetCursorX();
      if(cursorX >= clientX && cursorX <= clientX + clientW) {
        const uint32_t CURSOR_COLOR = 0xFFFFFF00;// bright yellow, fully opaque
        auto drawPixel = [&](int32_t x, int32_t y) {
          if(x >= 0 && x < static_cast<int32_t>(parentWidth) && y >= 0 && y < static_cast<int32_t>(parentHeight)) {
            size_t idx = static_cast<size_t>(y) * parentWidth + static_cast<size_t>(x);
            buffer[idx] = CURSOR_COLOR;
          }
        };
        if(mInsertMode) {
          for(int32_t y = clientY; y < clientY + clientH; ++y) {
            drawPixel(cursorX, y);
            if(cursorX + 1 < clientX + clientW + 1)
              drawPixel(cursorX + 1, y);
          }
        }
        else {
          int32_t lineY = clientY + clientH - 2;
          for(int32_t x = cursorX; x < cursorX + 2; ++x)
            drawPixel(x, lineY);
        }
      }
    }
  }

  void AInputBox::OnFocusGained() {
    mCursorVisible = true;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AInputBox::OnFocusLost() {
    mCursorVisible = false;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AInputBox::Enable() {
    AWidget::Enable();
    SetEditable(true);
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AInputBox::Disable() {
    D("inputbox disabled")
    AWidget::Disable();
    SetEditable(false);
    mCursorVisible = false;
    if(mParentWindow)
      mParentWindow->Draw();
  }

// ------------------------------------------------------------------
// Editable state
// ------------------------------------------------------------------
  void AInputBox::SetEditable(bool editable) {
    mEditable = editable;
    if(mParentWindow)
      mParentWindow->Draw();
  }

// ------------------------------------------------------------------
// Max length
// ------------------------------------------------------------------
  void AInputBox::SetMaxLength(size_t maxLen) {
    mMaxLength = maxLen;
    if(utf8_length(mText) > mMaxLength) {
// Truncate (simple byte truncation; may break UTF-8 – improve as needed)
      std::string truncated = mText.substr(0, mMaxLength);
      SetText(truncated);
    }
  }

// ------------------------------------------------------------------
// Input filtering (regex)
// ------------------------------------------------------------------
  void AInputBox::SetInputFilter(const std::string &regexPattern) {
    try {
      mInputFilter = std::regex(regexPattern, std::regex::ECMAScript);
    } catch (const std::regex_error &e) {
      E("Invalid regex pattern: {} - {}", regexPattern, e.what());
      mInputFilter.reset();
    }
  }

  void AInputBox::ClearInputFilter() {
    mInputFilter.reset();
  }

// ------------------------------------------------------------------
// Cursor control
// ------------------------------------------------------------------
  void AInputBox::SetCursorBlinkingEnabled(bool enable) {
    mBlinkingEnabled = enable;
    if(!enable) {
      mCursorVisible = true;
    }
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AInputBox::SetCursorPos(size_t pos) {
    if(!mEnabled) {
      D1("widget is disabled")
      return;// block programmatic changes too
    }
    mCursorPos = std::min(pos, mText.length());
    mCursorVisible = true;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  size_t AInputBox::GetCursorPos() const {
    return mCursorPos;
  }

// ------------------------------------------------------------------
// Insert/Overwrite mode
// ------------------------------------------------------------------
  void AInputBox::SetInsertMode(bool insert) {
    mInsertMode = insert;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  bool AInputBox::IsInsertMode() const {
    return mInsertMode;
  }

// ------------------------------------------------------------------
// Callbacks
// ------------------------------------------------------------------
  void AInputBox::SetOnChangeCallback(OnChangeCallback cb) {
    mOnChange = std::move(cb);
  }

  void AInputBox::SetOnSubmitCallback(OnSubmitCallback cb) {
    mOnSubmit = std::move(cb);
  }

// ------------------------------------------------------------------
// Text manipulation
// ------------------------------------------------------------------
  void AInputBox::SetText(const std::string &text) {
// First truncate to max length
    std::string newText = text;
    if(utf8_length(newText) > mMaxLength) {
      newText = newText.substr(0, mMaxLength);// byte truncation – consider UTF-8 safety
    }
// Then check filter on the truncated string
    if(!IsInputAllowed(newText))
      return;
    if(mText == newText)
      return;
    mText = newText;
    mCursorPos = std::min(mCursorPos, mText.length());
    SetValueAndNotify(mText);
  }

// ------------------------------------------------------------------
// Private helpers
// ------------------------------------------------------------------
  void AInputBox::BlinkThreadFunc() {
    std::unique_lock<std::mutex> lock(mBlinkMutex);
    while (!mStopBlinkThread) {
// Wait for BLINK_INTERVAL_MS or until stop requested
      mBlinkCV.wait_for(lock, std::chrono::milliseconds(BLINK_INTERVAL_MS), [this] {
        return mStopBlinkThread.load();
      });
      if(mStopBlinkThread)
        break;
// Toggle cursor visibility when conditions are met
      if(mBlinkingEnabled && IsFocused() && mEnabled) {
        mCursorVisible = !mCursorVisible;
        if(mParentWindow)
          mParentWindow->Draw();
      }
      else if(mCursorVisible) {
        mCursorVisible = false;
        if(mParentWindow)
          mParentWindow->Draw();
      }
//      mParentWindow->ForceDraw();
    }
  }

  bool AInputBox::IsInputAllowed(const std::string &newValue) const {
    if(!IsLengthAllowed(newValue))
      return false;
    if(mInputFilter.has_value()) {
      return std::regex_match(newValue, mInputFilter.value());
    }
    return true;
  }

  bool AInputBox::IsLengthAllowed(const std::string &newValue) const {
    return utf8_length(newValue) <= mMaxLength;
  }

  void AInputBox::SetValueAndNotify(const std::string &newValue) {
    if(newValue == mLastNotifiedValue)
      return;
    mLastNotifiedValue = newValue;
    if(mOnChange)
      mOnChange(this, newValue);
    if(mParentWindow)
      mParentWindow->Draw();
  }

  int32_t AInputBox::GetCursorX() const {
    int32_t clientX = mX + static_cast<int32_t>(mBorderThick);
    int32_t clientW = static_cast<int32_t>(mSizeX) - 2 * static_cast<int32_t>(mBorderThick);
    if(clientW <= 0)
      return clientX;
// Use displayed text (masked in password mode)
    std::string displayText = GetDisplayText();
    if(displayText.empty()) {
// Empty text: position cursor according to alignment
      switch (mHAlign) {
        case AUIHAlign::left:
          return clientX;
        case AUIHAlign::right:
          return clientX + clientW;
        case AUIHAlign::center:
          return clientX + clientW / 2;
        default:
          break;
      }
      return clientX;
    }
    int32_t totalWidth = MeasureTextWidth(displayText);
    std::string prefix = displayText.substr(0, mCursorPos);
    int32_t prefixWidth = MeasureTextWidth(prefix);
    int32_t cursorX = clientX;
    switch (mHAlign) {
      case AUIHAlign::left:
        cursorX += prefixWidth;
        break;
      case AUIHAlign::right:
        cursorX += clientW - (totalWidth - prefixWidth);
        break;
      case AUIHAlign::center:
        cursorX += (clientW - totalWidth) / 2 + prefixWidth;
        break;
      default:
        break;
    }
    return std::clamp(cursorX, clientX, clientX + clientW);
  }

  void AInputBox::InsertChar(char ch) {
    if(!mEditable)
      return;
    std::string candidate;
    if(mInsertMode) {
      candidate = mText;
      candidate.insert(mCursorPos, 1, ch);
    }
    else {
      if(mCursorPos < mText.length()) {
        candidate = mText;
        candidate[mCursorPos] = ch;
      }
      else {
        candidate = mText + ch;
      }
    }
    if(!IsInputAllowed(candidate))
      return;
    mText = candidate;
    if(mInsertMode) {
      ++mCursorPos;
    }
    else {
      if(mCursorPos < mText.length())
        ++mCursorPos;
      else
        ++mCursorPos;// at end, same as insert
    }
    mCursorPos = std::min(mCursorPos, mText.length());
    mCursorVisible = true;
    SetValueAndNotify(mText);
  }

  void AInputBox::DeleteChar() {
    if(!mEditable)
      return;
    if(mCursorPos == 0)
      return;
    std::string candidate = mText;
    candidate.erase(mCursorPos - 1, 1);
    if(!IsInputAllowed(candidate))
      return;
    mText = candidate;
    --mCursorPos;
    mCursorVisible = true;
    SetValueAndNotify(mText);
  }

  void AInputBox::DeleteForwardChar() {
    if(!mEditable)
      return;
    if(mCursorPos >= mText.length())
      return;
    std::string candidate = mText;
    candidate.erase(mCursorPos, 1);
    if(!IsInputAllowed(candidate))
      return;
    mText = candidate;
    mCursorVisible = true;
    SetValueAndNotify(mText);
  }

  bool AInputBox::OnMouseClick(int32_t localX, int32_t, bool pressed) {
    if(!mEnabled || !pressed)
      return false;
    size_t newPos = GetIndexFromX(localX);
    D1("OnMouseClick: localX={}, newPos={}, oldPos={}", localX, newPos, mCursorPos);
    if(newPos != mCursorPos) {
      mCursorPos = newPos;
      mCursorVisible = true;
      if(mParentWindow)
        mParentWindow->Draw();
    }
    return true;
  }

  void AInputBox::OnKeyEvent(const AUIKeyEvent &event) {
    if(!mEnabled || !event.pressed)
      return;
    D3("AInputBox::OnKeyEvent: code={}, unicode=0x{:X}", (int32_t)event.code, event.unicode);
    switch (event.code) {
      case AUIKeyCode::Backspace:
        DeleteChar();
        break;
      case AUIKeyCode::Delete:
        DeleteForwardChar();
        break;
      case AUIKeyCode::Left:
        if(mCursorPos > 0)
          --mCursorPos;
        break;
      case AUIKeyCode::Right:
        if(mCursorPos < mText.length())
          ++mCursorPos;
        break;
      case AUIKeyCode::Home:
        mCursorPos = 0;
        break;
      case AUIKeyCode::End:
        mCursorPos = mText.length();
        break;
      case AUIKeyCode::Enter:
        if(mOnSubmit)
          mOnSubmit(this, mText);
        break;
      case AUIKeyCode::Insert:
        SetInsertMode(!mInsertMode);
        break;
      default:
        if(event.unicode >= 32 && event.unicode <= 126) {
          InsertChar(static_cast<char>(event.unicode));
        }
        break;
    }
    mCursorVisible = true;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  int32_t AInputBox::MeasureTextWidth(const std::string &text) const {
    if(!mEnginePtr)
      return 0;
    FT_Face face = mEnginePtr->GetDefaultFontFace();
    if(!face)
      return 0;
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    int32_t width = 0;
    for(char c : text) {
      if(FT_Load_Char(face, static_cast<FT_ULong>(c), FT_LOAD_DEFAULT) == 0) {
        width += SafeINT32(face->glyph->advance.x >> 6);
      }
    }
    return width;
  }

  int32_t AInputBox::MeasureCharWidth(char ch) const {
    if(!mEnginePtr)
      return 0;
    FT_Face face = mEnginePtr->GetDefaultFontFace();
    if(!face)
      return 0;
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    if(FT_Load_Char(face, static_cast<FT_ULong>(ch), FT_LOAD_DEFAULT) == 0) {
      return SafeINT32(face->glyph->advance.x >> 6);
    }
    return 0;
  }
  size_t AInputBox::GetIndexFromX(int32_t localX) const {
    int32_t clientLeft = static_cast<int32_t>(mBorderThick);
    int32_t clientWidth = static_cast<int32_t>(mSizeX) - 2 * clientLeft;
    if(clientWidth <= 0)
      return 0;
    int32_t clickX = localX - clientLeft;
    if(clickX <= 0)
      return 0;
    if(clickX >= clientWidth)
      return mText.length();
    int32_t totalWidth = MeasureTextWidth(mText);
    if(totalWidth <= 0)
      return 0;
    int32_t textStartX = 0;
    switch (mHAlign) {
      case AUIHAlign::left:
        textStartX = 0;
        break;
      case AUIHAlign::right:
        textStartX = clientWidth - totalWidth;
        break;
      case AUIHAlign::center:
        textStartX = (clientWidth - totalWidth) / 2;
        break;
      default:
        E("garbage")
    }
// If click is before the text start, return 0
    if(clickX <= textStartX)
      return 0;
// If click is after the text end, return length
    if(clickX >= textStartX + totalWidth)
      return mText.length();
// Now find the character index using half‑width threshold
    int32_t accumulated = 0;
    for(size_t i = 0; i < mText.length(); ++i) {
      int32_t charWidth = MeasureCharWidth(mText[i]);
      int32_t charStart = textStartX + accumulated;
      int32_t charMid = charStart + charWidth / 2;
      if(clickX < charMid) {
        return i;// cursor before this character
      }
      accumulated += charWidth;
      if(clickX < textStartX + accumulated) {
        return i + 1;// cursor after this character
      }
    }
    return mText.length();
  }

  void AInputBox::SetPlaceholder(const std::string &placeholder) {
    mPlaceholder = placeholder;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AInputBox::SetPlaceholderColor(uint32_t color) {
    mPlaceholderColor = color;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AInputBox::SetPasswordMode(bool enable, char maskChar) {
    mPasswordMode = enable;
    mMaskChar = maskChar;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  std::string AInputBox::GetDisplayText() const {
    if(mPasswordMode && !mText.empty()) {
      return std::string(mText.length(), mMaskChar);
    }
    return mText;
  }

  AInputBox::~AInputBox() {
    {
      std::lock_guard<std::mutex> lock(mBlinkMutex);
      mStopBlinkThread = true;
    }
    mBlinkCV.notify_one();
    if(mBlinkThread && mBlinkThread->joinable())
      mBlinkThread->join();
  }

}// namespace aui

