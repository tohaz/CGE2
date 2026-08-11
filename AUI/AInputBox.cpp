#include "AUILib.h"

namespace aui {

  static size_t GetPrevCharByteCount(const std::string& str, size_t cursorPos) {
    if(cursorPos == 0 || cursorPos > str.length())
      return 0;
    size_t count = 1;
    while(cursorPos - count > 0 && (static_cast<uint8_t>(str[cursorPos - count]) & 0xC0) == 0x80) {
      ++count;
    }
    return count;
  }

  static size_t GetNextCharByteCount(const std::string& str, size_t cursorPos) {
    if(cursorPos >= str.length())
      return 0;
    size_t count = 1;
    while(cursorPos + count < str.length() && (static_cast<uint8_t>(str[cursorPos + count]) & 0xC0) == 0x80) {
      ++count;
    }
    return count;
  }
// ------------------------------------------------------------------
// UTF-8 code point counter (fallback to byte length for simplicity)
// ------------------------------------------------------------------
  UNUSED static size_t utf8_length(const std::string& str) {
// Replace with proper UTF-8 length function if needed
    return str.length();
  }

  static uint32_t DecodeUTF8(const std::string& str, size_t pos, size_t& outByteLen) {
    uint8_t c = static_cast<uint8_t>(str[pos]);
    if((c & 0x80) == 0) {
      outByteLen = 1;
      return c;
    }
    if((c & 0xE0) == 0xC0 && pos + 1 < str.length()) {
      outByteLen = 2;
      return ((c & 0x1F) << 6) | (str[pos + 1] & 0x3F);
    }
    if((c & 0xF0) == 0xE0 && pos + 2 < str.length()) {
      outByteLen = 3;
      return ((c & 0x0F) << 12) | ((str[pos + 1] & 0x3F) << 6) | (str[pos + 2] & 0x3F);
    }
    if((c & 0xF8) == 0xF0 && pos + 3 < str.length()) {
      outByteLen = 4;
      return ((c & 0x07) << 18) | ((str[pos + 1] & 0x3F) << 12) | ((str[pos + 2] & 0x3F) << 6) | (str[pos + 3] & 0x3F);
    }
    outByteLen = 1;
    return c;
  }
//
  AInputBox::AInputBox() :
      mBlinkingEnabled(true), mStopBlinkThread(false), mCursorVisible(true), mCursorPos(0), mInsertMode(true), mEditable(
          true), mMaxLength(DEFAULT_MAX_LENGTH) {
    D4()
    mType = AUIWidgetType::defaultInputBox;
    mHAlign = AUIHAlign::right;
    mVAlign = AUIVAlign::center;
    mX = AUI_DEFAULT_INPUT_X;
    mY = AUI_DEFAULT_INPUT_Y;
    mSizeX = AUI_DEFAULT_INPUT_SZX;
    mSizeY = AUI_DEFAULT_INPUT_SZY;
    mBGColor = AUI_DEFAULT_INPUT_BG;
    mBorderThick = AUI_DEFAULT_INPUT_BORDERW;
    Focusable(true);
    Editable(true);
    mBlinkThread = std::make_unique<std::thread>(&AInputBox::BlinkThreadFunc, this);
  }

  void AInputBox::Editable(bool v) {
    mEditable = v;
    MarkDirty();
    if(Wnd()) {
      Wnd()->RequestRedraw();
    }
  }

  void AInputBox::InsertMode(bool insert) {
    mInsertMode = insert;
    MarkContentDirty();
    if(Wnd()) {
      Wnd()->RequestRedraw();
    }
  }

  void AInputBox::DeleteChar() {
    if(!mEditable || mCursorPos == 0)
      return;

// Determine how many bytes to step back (1 for ASCII, >1 for UTF-8)
    size_t charByteCount = GetPrevCharByteCount(mText, mCursorPos);

    std::string candidate = mText;
    candidate.erase(mCursorPos - charByteCount, charByteCount);

    if(!InputAllowed(candidate))
      return;

    mText = std::move(candidate);
    mCursorPos -= charByteCount;
    mCursorVisible = true;
    SetValueAndNotify(mText);
  }

  void AInputBox::DeleteForwardChar() {
    if(!mEditable || mCursorPos >= mText.length())
      return;
    size_t charByteCount = GetNextCharByteCount(mText, mCursorPos);
    std::string candidate = mText;
    candidate.erase(mCursorPos, charByteCount);
    if(!InputAllowed(candidate))
      return;
    mText = std::move(candidate);
    mCursorVisible = true;
    MarkContentDirty();
    if(Wnd()) {
      Wnd()->RequestRedraw();
    }
    SetValueAndNotify(mText);
  }

  bool AInputBox::InputAllowed(const std::string& newValue) const {
    if(!LengthAllowed(newValue))
      return false;
    if(mInputFilter.has_value()) {
      return std::regex_match(newValue, mInputFilter.value());
    }
    return true;
  }

  void AInputBox::SetValueAndNotify(const std::string& newValue) {
    if(newValue == mLastNotifiedValue)
      return;
    mLastNotifiedValue = newValue;
    if(mOnChange)
      mOnChange(this, newValue);
    MarkContentDirty();
    if(Wnd()) {
      Wnd()->RequestRedraw();
    }
  }

  void AInputBox::OnKeyEvent(const AUIKeyEvent& event) {
    if(!mEnabled || !event.pressed)
      return;
    D3("AInputBox::OnKeyEvent: code={}, unicode=0x{:X}", (int32_t)event.code, event.unicode);
    bool needsRedraw = false;
    switch(event.code) {
      case AUIKeyCode::Backspace:
        DeleteChar();
        return;
      case AUIKeyCode::Delete:
        DeleteForwardChar();
        return;
      case AUIKeyCode::Left:
        if(mCursorPos > 0) {
          mCursorPos -= GetPrevCharByteCount(mText, mCursorPos);
          needsRedraw = true;
        }
        break;
      case AUIKeyCode::Right:
        if(mCursorPos < mText.length()) {
          mCursorPos += GetNextCharByteCount(mText, mCursorPos);
          needsRedraw = true;
        }
        break;
      case AUIKeyCode::Home:
        if(mCursorPos > 0) {
          mCursorPos = 0;
          needsRedraw = true;
        }
        break;
      case AUIKeyCode::End:
        if(mCursorPos < mText.length()) {
          mCursorPos = mText.length();
          needsRedraw = true;
        }
        break;
      case AUIKeyCode::Enter:
        if(mOnSubmit)
          mOnSubmit(this, mText);
        return;
      case AUIKeyCode::Insert:
        InsertMode(!mInsertMode);
        return;// InsertMode handles its own redraw
      default:
        if(event.unicode >= 32 && event.unicode <= 126) {
          InsertChar(static_cast<char>(event.unicode));
        }
        return;
    }
    if(needsRedraw) {
      mCursorVisible = true;
      MarkContentDirty();
      if(Wnd())
        Wnd()->RequestRedraw();
    }
  }

  void AInputBox::BlinkThreadFunc() {
    std::unique_lock<std::mutex> lock(mBlinkMutex);
    while(!mStopBlinkThread) {
      mBlinkCV.wait_for(lock, std::chrono::milliseconds(BLINK_INTERVAL_MS), [this] {
        return mStopBlinkThread.load();
      });
      if(mStopBlinkThread)
        break;
      if(mBlinkingEnabled) {
        if(Focused() && mEnabled) {
          mCursorVisible = !mCursorVisible;
          MarkContentDirty();
          if(Wnd())
            Wnd()->RequestRedraw();
        }
        else {
          if(mCursorVisible) {
            mCursorVisible = false;
            MarkContentDirty();
            if(Wnd())
              Wnd()->RequestRedraw();
          }
        }
      }
    }
  }

  bool AInputBox::LengthAllowed(const std::string& newValue) const {
    return utf8_length(newValue) <= mMaxLength;
  }

  void AInputBox::InsertChar(int8_t ch) {
    if(!mEditable)
      return;
    std::string candidate = mText;
    if(mInsertMode || mCursorPos >= candidate.length()) {
// Insert mode OR appending at the very end
      candidate.insert(mCursorPos, 1, ch);
    }
    else {
// Overwrite mode inside the string bounds
      candidate[mCursorPos] = ch;
    }
    if(!InputAllowed(candidate))
      return;
    mText = std::move(candidate);
    ++mCursorPos;// Simply advance cursor by 1 byte inserted/overwritten
    mCursorPos = std::min(mCursorPos, mText.length());
    mCursorVisible = true;
    SetValueAndNotify(mText);
    D3("text is {}", mText)
  }

  void AInputBox::Text(const std::string& text) {
// First truncate to max length
    std::string newText = text;
    if(utf8_length(newText) > mMaxLength) {
      newText = newText.substr(0, mMaxLength);// byte truncation – consider UTF-8 safety
    }
// Then check filter on the truncated string
    if(!InputAllowed(newText))
      return;
    if(mText == newText)
      return;
    mText = newText;
    mCursorPos = std::min(mCursorPos, mText.length());
    SetValueAndNotify(mText);
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

  void AInputBox::OnFocusGained() {
    mCursorVisible = true;
    MarkContentDirty();
    Wnd()->RequestRedraw();
  }

  void AInputBox::OnFocusLost() {
    mCursorVisible = false;
    MarkContentDirty();
    Wnd()->RequestRedraw();
  }

  void AInputBox::Enable() {
    AWidget::Enable();
    Editable(true);
    MarkContentDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AInputBox::Disable() {
    D2("inputbox disabled")
    AWidget::Disable();
    Editable(false);
    mCursorVisible = false;
    MarkContentDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AInputBox::MaxLength(size_t maxLen) {
    mMaxLength = maxLen;
    if(utf8_length(mText) > mMaxLength) {
// Truncate (simple byte truncation; may break UTF-8 – improve as needed)
      std::string truncated = mText.substr(0, mMaxLength);
      Text(truncated);
    }
  }

  void AInputBox::InputFilter(const std::string& regexPattern) {
    try {
      mInputFilter = std::regex(regexPattern, std::regex::ECMAScript);
    } catch (const std::regex_error& e) {
      E("Invalid regex pattern: {} - {}", regexPattern, e.what());
      mInputFilter.reset();
    }
  }

  void AInputBox::ClearInputFilter() {
    mInputFilter.reset();
  }

  void AInputBox::CursorBlinkingEnabled(bool enable) {
    mBlinkingEnabled = enable;
    if(!enable) {
      mCursorVisible = true;
    }
    MarkContentDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AInputBox::CursorPos(size_t pos) {
    if(!mEnabled) {
      D1("widget is disabled")
      return;// block programmatic changes too
    }
    mCursorPos = std::min(pos, mText.length());
    mCursorVisible = true;
    MarkContentDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  size_t AInputBox::CursorPos() const {
    return mCursorPos;
  }

  bool AInputBox::IsInsertMode() const {
    return mInsertMode;
  }

  void AInputBox::SetOnChangeCallback(OnChangeCallback cb) {
    mOnChange = std::move(cb);
  }

  void AInputBox::SetOnSubmitCallback(OnSubmitCallback cb) {
    mOnSubmit = std::move(cb);
  }

  int32_t AInputBox::CursorX() const {
    int32_t clientX = mX + static_cast<int32_t>(mBorderThick);
    int32_t clientW = static_cast<int32_t>(mSizeX) - 2 * static_cast<int32_t>(mBorderThick);
    if(clientW <= 0)
      return clientX;
    std::string displayText = DisplayText();
    if(displayText.empty()) {
      switch(mHAlign) {
        case AUIHAlign::left:
          return clientX;
        case AUIHAlign::right:
          return clientX + clientW - 1;
        case AUIHAlign::center:
          return clientX + clientW / 2;
        default:
          return clientX;
      }
    }
// Ensure mCursorPos is clamped safely to displayText length
    size_t safePos = std::min(mCursorPos, displayText.length());
    std::string prefix = displayText.substr(0, safePos);
    std::string suffix = displayText.substr(safePos);
    int32_t prefixWidth = MeasureTextWidth(prefix);
    int32_t suffixWidth = MeasureTextWidth(suffix);
    int32_t cursorX = clientX;
    switch(mHAlign) {
      case AUIHAlign::left:
// Left alignment: clientX + prefix width
        cursorX = clientX + prefixWidth;
        break;
      case AUIHAlign::right:
// Right alignment: right bound minus suffix width
// This prevents totalWidth rounding mismatches!
        cursorX = (clientX + clientW) - suffixWidth;
        break;
      case AUIHAlign::center: {
        int32_t totalWidth = prefixWidth + suffixWidth;
        cursorX = clientX + (clientW - totalWidth) / 2 + prefixWidth;
        break;
      }
      default:
        break;
    }
    return std::clamp(cursorX, clientX, clientX + clientW - 1);
  }

  int32_t AInputBox::MeasureTextWidth(const std::string& text) const {
    if(!Wnd() || !Wnd()->EnginePtr())
      return 0;
    FT_Face face = Wnd()->EnginePtr()->DefaultFontFace();
    if(!face)
      return 0;
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    FT_Pos accum26_6 = 0;// Accumulate in 26.6 fixed-point
    size_t i = 0;
    while(i < text.length()) {
      size_t byteLen = 1;
      uint32_t codePoint = DecodeUTF8(text, i, byteLen);
      if(FT_Load_Char(face, codePoint, FT_LOAD_DEFAULT) == 0) {
        accum26_6 += face->glyph->advance.x;
      }
      i += byteLen;
    }
// Shift to integer pixels ONCE at the end
    return SafeINT32(accum26_6 >> 6);
  }

  std::string AInputBox::DisplayText() const {
    if(mPasswordMode && !mText.empty()) {
      return std::string(mText.length(), mMaskChar);
    }
    return mText;
  }

  int32_t AInputBox::MeasureCharWidth(char ch) const {
    if(!Wnd()->EnginePtr())
      return 0;
    FT_Face face = Wnd()->EnginePtr()->DefaultFontFace();
    ;
    if(!face)
      return 0;
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    if(FT_Load_Char(face, static_cast<FT_ULong>(ch), FT_LOAD_DEFAULT) == 0) {
      return SafeINT32(face->glyph->advance.x >> 6);
    }
    return 0;
  }

  size_t AInputBox::IndexFromX(int32_t localX) const {
    int32_t clientLeft = static_cast<int32_t>(mBorderThick);
    int32_t clientWidth = static_cast<int32_t>(mSizeX) - 2 * clientLeft;
    if(clientWidth <= 0 || mText.empty())
      return 0;
    int32_t clickX = localX - clientLeft;
    if(clickX <= 0)
      return 0;
    std::string displayText = DisplayText();
    int32_t totalWidth = MeasureTextWidth(displayText);
    if(totalWidth <= 0)
      return 0;
// Compute horizontal alignment start offset
    int32_t textStartX = 0;
    switch(mHAlign) {
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
        break;
    }
// Bounds check relative to formatted text
    if(clickX <= textStartX)
      return 0;
    if(clickX >= textStartX + totalWidth)
      return mText.length();
    FT_Face face = (Wnd() && Wnd()->EnginePtr()) ? Wnd()->EnginePtr()->DefaultFontFace() : nullptr;
    if(!face)
      return 0;
    FT_Set_Pixel_Sizes(face, 0, mFontSize);
    int32_t accumulatedWidth = 0;
    size_t byteIndex = 0;
// Iterate over full UTF-8 code points instead of raw single bytes
    while(byteIndex < displayText.length()) {
      size_t byteLen = 1;
      uint32_t codePoint = DecodeUTF8(displayText, byteIndex, byteLen);
      int32_t charWidth = 0;
      if(FT_Load_Char(face, codePoint, FT_LOAD_DEFAULT) == 0) {
        charWidth = SafeINT32(face->glyph->advance.x >> 6);
      }
      int32_t charStart = textStartX + accumulatedWidth;
      int32_t charMid = charStart + (charWidth / 2);
// If the click is on the left half of this character, place cursor before it
      if(clickX < charMid) {
        return byteIndex;
      }
      accumulatedWidth += charWidth;
// If click falls inside the right half of this character, place cursor after it
      if(clickX < textStartX + accumulatedWidth) {
        return byteIndex + byteLen;
      }
      byteIndex += byteLen;
    }
    return mText.length();
  }

  void AInputBox::Placeholder(const std::string& placeholder) {
    mPlaceholder = placeholder;
    MarkContentDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AInputBox::PlaceholderColor(uint32_t color) {
    mPlaceholderColor = color;
    MarkContentDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AInputBox::PasswordMode(bool enable, int8_t maskChar) {
    mPasswordMode = enable;
    mMaskChar = maskChar;
    MarkContentDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  AWidget* AInputBox::OnMouseDownLeft(UNUSED int32_t localX, UNUSED int32_t localY) {
    if(!mEnabled)
      return nullptr;
    size_t newPos = IndexFromX(localX);
    D2("OnMouseClick: localX={}, newPos={}, oldPos={}", localX, newPos, mCursorPos);
    if(newPos != mCursorPos) {
      mCursorPos = newPos;
      mCursorVisible = true;
      MarkContentDirty();
      if(Wnd())
        Wnd()->RequestRedraw();
    }
    return this;
  }

  void AInputBox::OnDraw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
      int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const {
    D4("OnDraw START: mText='{}', mCursorPos={}, CursorX={}", mText, mCursorPos, CursorX());
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
// 1. Background (with disabled dimming) intersected with clip region
    int32_t drawL = std::max(absX, clipL);
    int32_t drawT = std::max(absY, clipT);
    int32_t drawR = std::min(absX + static_cast<int32_t>(mSizeX), clipR);
    int32_t drawB = std::min(absY + static_cast<int32_t>(mSizeY), clipB);
    if(drawR > drawL && drawB > drawT && drawL >= 0 && drawT >= 0) {
      uint32_t bgColor = mEnabled ? mBGColor : ShiftColor(mBGColor, true);
      size_t pW = static_cast<size_t>(bufferW);
      size_t pH = static_cast<size_t>(bufferH);
      size_t maxIdx = pW * pH;
      for(int32_t y = drawT; y < drawB; ++y) {
        size_t lineStart = static_cast<size_t>(y) * pW;
        for(int32_t x = drawL; x < drawR; ++x) {
          size_t idx = lineStart + static_cast<size_t>(x);
          if(idx < maxIdx)
            buffer[idx] = bgColor;
        }
      }
    }
// 2. Client area (excluding border)
    int32_t clientX = absX + static_cast<int32_t>(mBorderThick);
    int32_t clientY = absY + static_cast<int32_t>(mBorderThick);
    int32_t clientW = static_cast<int32_t>(mSizeX) - 2 * static_cast<int32_t>(mBorderThick);
    int32_t clientH = static_cast<int32_t>(mSizeY) - 2 * static_cast<int32_t>(mBorderThick);
    if(clientW <= 0 || clientH <= 0)
      return;
// 3. Draw text or placeholder (using new DrawTextEx signature)
    FT_Face face = Wnd() && Wnd()->EnginePtr() ? Wnd()->EnginePtr()->DefaultFontFace() : nullptr;
    if(face) {
      aui::ARect clientBounds { clientX, clientY, static_cast<uint32_t>(clientW), static_cast<uint32_t>(clientH) };
// Compute effective clipping rectangle intersecting client area with screen clip region
      int32_t textClipL = std::max(clientX, clipL);
      int32_t textClipT = std::max(clientY, clipT);
      int32_t textClipR = std::min(clientX + clientW, clipR);
      int32_t textClipB = std::min(clientY + clientH, clipB);
      if(textClipR > textClipL && textClipB > textClipT) {
        aui::ARect customClip { textClipL, textClipT, static_cast<uint32_t>(textClipR - textClipL),
            static_cast<uint32_t>(textClipB - textClipT) };
        std::string displayText = DisplayText();// handles password mode
//D("text: {}", displayText)
        if(!displayText.empty()) {
          uint32_t textColor = mEnabled ? mTextColor : ShiftColor(mTextColor, true);
          aui::ATextStyle style { textColor, mFontSize, mHAlign, mVAlign, 0.0 };
          DrawTextEx(buffer, bufferW, bufferH, clientBounds, displayText, face, style, &customClip);
        }
        else
          if(!mPlaceholder.empty() && !Focused()) {
            uint32_t placeholderColor = mEnabled ? mPlaceholderColor : ShiftColor(mPlaceholderColor, true);
            aui::ATextStyle style { placeholderColor, mFontSize, mHAlign, mVAlign, 0.0 };
            DrawTextEx(buffer, bufferW, bufferH, clientBounds, mPlaceholder, face, style, &customClip);
          }
      }
    }
// 4. Draw cursor (only when enabled and focused)
    if(mEnabled && Focused() && mCursorVisible) {
      int32_t cursorX = offsetX + CursorX();
// Clamp cursorX so it stays within the last drawable column of the client area
      int32_t maxCursorX = clientX + clientW - 1;
      if(cursorX > maxCursorX) {
        cursorX = maxCursorX;
      }
      if(cursorX >= clientX) {
        const uint32_t CURSOR_COLOR = 0xFFFFFF00;// bright yellow, fully opaque
        auto drawPixel = [&](int32_t x, int32_t y) {
          if(x >= clipL && x < clipR && y >= clipT && y < clipB && x >= 0 && x < static_cast<int32_t>(bufferW) && y >= 0
              && y < static_cast<int32_t>(bufferH)) {
            size_t idx = static_cast<size_t>(y) * static_cast<size_t>(bufferW) + static_cast<size_t>(x);
            buffer[idx] = CURSOR_COLOR;
          }
        };
        if(mInsertMode) {
          for(int32_t y = clientY; y < clientY + clientH; ++y) {
            drawPixel(cursorX, y);
// Note: cursorX + 1 will automatically be clipped safely by drawPixel
// if cursorX is at maxCursorX
            drawPixel(cursorX + 1, y);
          }
        }
        else {
          int32_t lineY = clientY + clientH - 2;
          for(int32_t x = cursorX; x < cursorX + 2; ++x) {
            drawPixel(x, lineY);
          }
        }
      }
    }
  }

}// namespace aui

