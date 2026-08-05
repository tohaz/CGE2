#include "AUILib.h"

namespace aui {

  static inline uint32_t LerpColor(uint32_t c1, uint32_t c2, double t) {
    if(t <= 0.0)
      return c1;
    if(t >= 1.0)
      return c2;
    uint32_t a1 = (c1 >> 24) & 0xFF;
    uint32_t r1 = (c1 >> 16) & 0xFF;
    uint32_t g1 = (c1 >> 8) & 0xFF;
    uint32_t b1 = c1 & 0xFF;
    uint32_t a2 = (c2 >> 24) & 0xFF;
    uint32_t r2 = (c2 >> 16) & 0xFF;
    uint32_t g2 = (c2 >> 8) & 0xFF;
    uint32_t b2 = c2 & 0xFF;
    uint32_t a = a1 + static_cast<uint32_t>((static_cast<int32_t>(a2) - static_cast<int32_t>(a1)) * t);
    uint32_t r = r1 + static_cast<uint32_t>((static_cast<int32_t>(r2) - static_cast<int32_t>(r1)) * t);
    uint32_t g = g1 + static_cast<uint32_t>((static_cast<int32_t>(g2) - static_cast<int32_t>(g1)) * t);
    uint32_t b = b1 + static_cast<uint32_t>((static_cast<int32_t>(b2) - static_cast<int32_t>(b1)) * t);
    return (a << 24) | (r << 16) | (g << 8) | b;
  }

  AProgressBar::AProgressBar() :
      mMin(0.0), mMax(1.0), mProgress(0.0), mIndeterminate(false), mIndeterminatePhase(0.0), mIndeterminateSpeed(0.02), mStopThread(
          false), mPaused(false), mUpdateIntervalMs(100), mShowText(true), mTextFormat("%.0f%%"), mStripe(false), mStripeColor(
          0x40FFFFFF), mStripeWidth(5), mStripeSpeed(2), mStripeOffset(0), mRoundedCorners(false), mCornerRadius(0), mStarted(
          false) {
    D2("AProgressBar constructed");
    mSizeX = 200;
    mSizeY = 30;
    mBGColor = 0xFFCCCCCC;
    mTextColor = 0xFF000000;
    mBorderColor = 0xFF888888;
    mType = AUIWidgetType::defaultProgressBar;
    UpdateTextCache();
    mUpdateThread = std::thread(&AProgressBar::ThreadFunction, this);
    mBGColor3 = 0xFF00AA00;
    mBGColor4 = 0;
    Orient(AUIOrientation::horizontal);
    Direction(AUIDirection::right);
    Text("some progressbar");
  }

  void AProgressBar::UpdateTextCache() {
    std::lock_guard < std::mutex > lock(mCacheMutex);
    if(mIndeterminate.load()) {
      mCachedText = "Loading...";
    }
    else {
      double progress = mProgress.load();
      double value = mMin + progress * (mMax - mMin);
      char buf[64];
      snprintf(buf, sizeof(buf), mTextFormat.c_str(), value);
      mCachedText = buf;
    }
  }

  void AProgressBar::Progress(double progress) {
    double old, newP;
    bool changed = false;
    {
      if(mIndeterminate.load())
        return;
      if(progress < mMin)
        progress = mMin;
      if(progress > mMax)
        progress = mMax;
      double norm = (progress - mMin) / (mMax - mMin);
      norm = std::max(0.0, std::min(1.0, norm));
      old = mProgress.load();
      if(std::abs(old - norm) > 0.000001) {
        mProgress = norm;
        newP = norm;
        UpdateTextCache();
        changed = true;
      }
    }
    if(changed) {
      FireCallbacks(old, newP);
      if(Wnd())
        Wnd()->RequestRedraw();
    }
  }

  double AProgressBar::Progress() const {
    return mProgress.load();
  }

  void AProgressBar::Clear() {
    Progress(mMin);
  }

  void AProgressBar::Range(double min, double max) {
    if(min >= max) {
      E("AProgressBar::SetRange: min must be < max");
      return;
    }
    mMin = min;
    mMax = max;
    UpdateTextCache();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AProgressBar::Indeterminate(bool enable) {
    mIndeterminate = enable;
    if(enable)
      mIndeterminatePhase = 0.0;
    UpdateTextCache();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AProgressBar::ShowText(bool show) {
    mShowText = show;
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AProgressBar::TextFormat(const std::string& format) {
    mTextFormat = format;
    UpdateTextCache();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AProgressBar::Stripe(bool enable) {
    mStripe = enable;
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AProgressBar::StripeColor(uint32_t color) {
    mStripeColor = color;
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AProgressBar::StripeWidth(uint32_t pixels) {
    if(pixels < 1)
      pixels = 1;
    mStripeWidth = pixels;
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AProgressBar::StripeSpeed(int32_t pixelsPerUpdate) {
    mStripeSpeed = pixelsPerUpdate;
  }

  void AProgressBar::RoundedCorners(bool enable, uint32_t radius) {
    mRoundedCorners = enable;
    mCornerRadius = (radius > 0) ? radius : 8;
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AProgressBar::SetOnProgressChanged(ProgressCallback cb) {
    mOnProgressChanged = cb;
  }

  void AProgressBar::SetOnStart(ProgressCallback cb) {
    mOnStart = cb;
  }

  void AProgressBar::SetOnComplete(ProgressCallback cb) {
    mOnComplete = cb;
  }

  void AProgressBar::FireCallbacks(double oldP, double newP) {
    if(newP > 0.0 && !mStarted) {
      mStarted = true;
      if(mOnStart)
        mOnStart(newP);
    }
    if(mOnProgressChanged && std::abs(newP - oldP) > 0.000001)
      mOnProgressChanged(newP);
    if(newP >= 1.0 && oldP < 1.0) {
      if(mOnComplete)
        mOnComplete(newP);
    }
  }

  void AProgressBar::UpdateInterval(uint32_t intervalMs) {
    std::lock_guard < std::mutex > lock(mThreadMutex);
    mUpdateIntervalMs = intervalMs;
    D2("SetUpdateInterval: interval=%u ms", intervalMs);
    mThreadCv.notify_all();
  }

  void AProgressBar::SetProgressProvider(std::function<double()> provider) {
    {
      std::lock_guard < std::mutex > lock(mThreadMutex);
      mProgressProvider = provider;
      mProviderPending = true;// signal that a new provider is available
      D2("SetProgressProvider: provider set, pending flag true");
    }
    mThreadCv.notify_all();
    D2("SetProgressProvider: notified condition variable");
  }

  void AProgressBar::PauseUpdates(bool pause) {
    mPaused = pause;
    if(!pause)
      mThreadCv.notify_all();
  }

  void AProgressBar::ThreadFunction() {
    D2("ThreadFunction: thread started");
    while(true) {
      bool needRedraw = false;
      std::function < double() > provider;
      uint32_t interval;
      {
        std::unique_lock < std::mutex > lock(mThreadMutex);
        interval = mUpdateIntervalMs;
        D3("ThreadFunction: waiting for condition (timeout=%u ms, stop=%d, pending=%d)", interval, mStopThread.load(),
            mProviderPending.load());
// Wake on stop OR new provider
        if(mThreadCv.wait_for(lock, std::chrono::milliseconds(interval), [this] {
          return mStopThread.load() || mProviderPending.load();
        }))
        {
          D2("ThreadFunction: condition met (stop=%d, pending=%d)", mStopThread.load(), mProviderPending.load());

          if(mStopThread.load()) {
            D2("ThreadFunction: stop requested, exiting");
            break;
          }
// New provider pending – clear the flag
          mProviderPending = false;
          D2("ThreadFunction: cleared provider pending flag");
        }
        else {
          D3("ThreadFunction: timeout, no pending provider or stop");
        }
        if(mStopThread.load() || mPaused.load()) {
          D2("ThreadFunction: paused or stopped, continuing loop");
          continue;
        }
// Copy the provider (if any) while holding the lock
        provider = mProgressProvider;
        D2("ThreadFunction: provider %s", provider ? "valid" : "null");
      }
// ---- Indeterminate mode ----
      if(mIndeterminate.load()) {
        double phase = mIndeterminatePhase.load();
        phase += mIndeterminateSpeed;
        if(phase > 1.0)
          phase -= 1.0;
        mIndeterminatePhase = phase;
        if(mStripe) {
          mStripeOffset = mStripeOffset.load() + mStripeSpeed;
        }
        needRedraw = true;
        D2("ThreadFunction: indeterminate phase=%.2f", phase);
      }
// ---- Normal mode with provider ----
      else
        if(provider) {
          double raw;
          try {
            raw = provider();
            D2("ThreadFunction: provider returned raw=%.3f", raw);
          } catch (...) {
            raw = mMin;
            D1("ThreadFunction: provider threw exception, using min");
          }
          if(raw < mMin)
            raw = mMin;
          if(raw > mMax)
            raw = mMax;
          double norm = (raw - mMin) / (mMax - mMin);
          norm = std::max(0.0, std::min(1.0, norm));
          double old = mProgress.load();
          if(std::abs(norm - old) > 0.000001) {
            mProgress = norm;
            UpdateTextCache();
            FireCallbacks(old, norm);
            needRedraw = true;
            D2("ThreadFunction: progress updated from %.3f to %.3f", old, norm);
          }
          else {
            D2("ThreadFunction: progress unchanged (%.3f)", norm);
          }
          if(mStripe) {
            mStripeOffset = mStripeOffset.load() + mStripeSpeed;
            needRedraw = true;
          }
        }
        else {
          D3("ThreadFunction: no provider and not indeterminate, skipping update");
        }
// ---- Trigger redraw ----
      if(needRedraw && Wnd()) {
        D2("ThreadFunction: requesting redraw");
        Wnd()->RequestRedraw();
      }
    }
    D2("ThreadFunction: thread exiting");
  }

  void AProgressBar::GetFillRect(int32_t clientX, int32_t clientY, int32_t clientW, int32_t clientH, double progress,
      int32_t& outX, int32_t& outY, int32_t& outW, int32_t& outH) const {
    double p = std::max(0.0, std::min(1.0, progress));
    if(Orient() == AUIOrientation::horizontal) {
      int32_t fill = static_cast<int32_t>(clientW * p);
      switch(Direction()) {
        case AUIDirection::right:
          outX = clientX;
          outY = clientY;
          outW = fill;
          outH = clientH;
          break;
        case AUIDirection::left:
          outX = clientX + clientW - fill;
          outY = clientY;
          outW = fill;
          outH = clientH;
          break;
        default:
          outX = clientX;
          outY = clientY;
          outW = fill;
          outH = clientH;
          break;
      }
    }
    else {
      int32_t fill = static_cast<int32_t>(clientH * p);
      switch(Direction()) {
        case AUIDirection::top:
          outX = clientX;
          outY = clientY + clientH - fill;
          outW = clientW;
          outH = fill;
          break;
        case AUIDirection::bottom:
          outX = clientX;
          outY = clientY;
          outW = clientW;
          outH = fill;
          break;
        default:
          outX = clientX;
          outY = clientY;
          outW = clientW;
          outH = fill;
          break;
      }
    }
  }

  void AProgressBar::DrawBackground(uint32_t* buffer, uint32_t parentWidth, int32_t x, int32_t y, int32_t w,
      int32_t h) const {
    if(w <= 0 || h <= 0)
      return;
    uint32_t bg = mBGColor;
    uint32_t clearColor = (mClearColor == 0) ? Wnd()->BGColor() : mClearColor;
    bool rounded = mRoundedCorners && mCornerRadius > 0;
    int32_t r = static_cast<int32_t>(mCornerRadius);
    if(r > w / 2)
      r = w / 2;
    if(r > h / 2)
      r = h / 2;
    if(!rounded || r <= 0) {
      FillRect(buffer, parentWidth, x, y, w, h, bg);
      return;
    }
    int32_t rSq = r * r;
    for(int32_t row = 0; row < h; ++row) {
      int32_t startX = x;
      int32_t endX = x + w;
      if(row < r) {
        int32_t dy = r - row;
        int32_t dx = r - static_cast<int32_t>(std::sqrt(rSq - dy * dy));
        startX = x + dx;
        endX = x + w - dx;
      }
      else
        if(row >= h - r) {
          int32_t dy = row - (h - 1 - r);
          int32_t dx = r - static_cast<int32_t>(std::sqrt(rSq - dy * dy));
          startX = x + dx;
          endX = x + w - dx;
        }
      size_t lineOffset = static_cast<size_t>(y + row) * parentWidth;
// 1. Clear left corner cutout (fixes the black box artifact)
      if(startX > x) {
        std::fill_n(&buffer[lineOffset + static_cast<size_t>(x)], startX - x, clearColor);
      }
// 2. Draw rounded background center
      if(startX < endX) {
        std::fill_n(&buffer[lineOffset + static_cast<size_t>(startX)], endX - startX, bg);
      }
// 3. Clear right corner cutout (fixes the black box artifact)
      if(endX < x + w) {
        std::fill_n(&buffer[lineOffset + static_cast<size_t>(endX)], (x + w) - endX, clearColor);
      }
    }
  }

  void AProgressBar::DrawBar(uint32_t* buffer, uint32_t parentWidth, int32_t x, int32_t y, int32_t w, int32_t h) const {
    if(w <= 0 || h <= 0)
      return;
    const bool hasGradient = (mBGColor4 != 0);
    const bool horizontal = (Orient() == AUIOrientation::horizontal);
    bool rounded = mRoundedCorners && mCornerRadius > 0;
    int32_t r = static_cast<int32_t>(mCornerRadius);
    if(r > w / 2)
      r = w / 2;
    if(r > h / 2)
      r = h / 2;
    if(!rounded || r <= 0) {
      if(hasGradient) {
        if(horizontal) {
          for(int32_t col = 0; col < w; ++col) {
            uint32_t c = LerpColor(mBGColor3, mBGColor4, static_cast<double>(col) / w);
            FillRect(buffer, parentWidth, x + col, y, 1, h, c);
          }
        }
        else {
          for(int32_t row = 0; row < h; ++row) {
            uint32_t c = LerpColor(mBGColor3, mBGColor4, static_cast<double>(row) / h);
            FillRect(buffer, parentWidth, x, y + row, w, 1, c);
          }
        }
      }
      else {
        FillRect(buffer, parentWidth, x, y, w, h, mBGColor3);
      }
      return;
    }
    int32_t rSq = r * r;
    for(int32_t row = 0; row < h; ++row) {
      int32_t startX = x;
      int32_t endX = x + w;
      if(row < r) {
        int32_t dy = r - row;
        int32_t dx = r - static_cast<int32_t>(std::sqrt(rSq - dy * dy));
        startX = x + dx;
        endX = x + w - dx;
      }
      else
        if(row >= h - r) {
          int32_t dy = row - (h - 1 - r);
          int32_t dx = r - static_cast<int32_t>(std::sqrt(rSq - dy * dy));
          startX = x + dx;
          endX = x + w - dx;
        }
      if(startX >= endX)
        continue;
      size_t lineOffset = static_cast<size_t>(y + row) * parentWidth;
      size_t uCount = static_cast<size_t>(endX - startX);
      if(hasGradient) {
        if(horizontal) {
          for(int32_t px = startX; px < endX; ++px) {
            uint32_t c = LerpColor(mBGColor3, mBGColor4, static_cast<double>(px - x) / w);
            buffer[lineOffset + static_cast<size_t>(px)] = c;
          }
        }
        else {
          uint32_t c = LerpColor(mBGColor3, mBGColor4, static_cast<double>(row) / h);
          std::fill_n(&buffer[lineOffset + static_cast<size_t>(startX)], uCount, c);
        }
      }
      else {
        std::fill_n(&buffer[lineOffset + static_cast<size_t>(startX)], uCount, mBGColor3);
      }
    }
  }

  void AProgressBar::DrawStripe(uint32_t* buffer, uint32_t parentWidth, int32_t x, int32_t y, int32_t w,
      int32_t h) const {
    if(w <= 0 || h <= 0 || !mStripe)
      return;
    bool rounded = mRoundedCorners && mCornerRadius > 0;
    int32_t r = static_cast<int32_t>(mCornerRadius);
    if(r > w / 2)
      r = w / 2;
    if(r > h / 2)
      r = h / 2;
    int32_t stripeW = static_cast<int32_t>(mStripeWidth);
    int32_t modPeriod = stripeW * 2;
    int32_t initialOffset = mStripeOffset.load() % modPeriod;
    if(initialOffset < 0)
      initialOffset += modPeriod;
    int32_t rSq = r * r;
    for(int32_t row = 0; row < h; ++row) {
      int32_t startX = x;
      int32_t endX = x + w;
      if(rounded && r > 0) {
        if(row < r) {
          int32_t dy = r - row;
          int32_t dx = r - static_cast<int32_t>(std::sqrt(rSq - dy * dy));
          startX = x + dx;
          endX = x + w - dx;
        }
        else
          if(row >= h - r) {
            int32_t dy = row - (h - 1 - r);
            int32_t dx = r - static_cast<int32_t>(std::sqrt(rSq - dy * dy));
            startX = x + dx;
            endX = x + w - dx;
          }
      }
      if(startX >= endX)
        continue;
      int32_t relX = (startX - x - initialOffset) % modPeriod;
      if(relX < 0)
        relX += modPeriod;
      size_t idx = static_cast<size_t>(y + row) * parentWidth + static_cast<size_t>(startX);
      for(int32_t px = startX; px < endX; ++px) {
        if(relX < stripeW) {
          buffer[idx] = mStripeColor;
        }
        idx++;
        relX++;
        if(relX >= modPeriod)
          relX = 0;
      }
    }
  }

  void AProgressBar::OnDraw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH, int32_t offsetX, int32_t offsetY,
      int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const {
// 1. Widget absolute rectangle
    int32_t absX = offsetX + mX;
    int32_t absY = offsetY + mY;
    int32_t w = static_cast<int32_t>(mSizeX);
    int32_t h = static_cast<int32_t>(mSizeY);
// 2. Clip with given region
    int32_t drawL = std::max(clipL, absX);
    int32_t drawT = std::max(clipT, absY);
    int32_t drawR = std::min(clipR, absX + w);
    int32_t drawB = std::min(clipB, absY + h);
    if(drawL >= drawR || drawT >= drawB)
      return;
// 4. Client area (inside border)
    int32_t border = static_cast<int32_t>(mBorderThick);
    int32_t clientX = absX + border;
    int32_t clientY = absY + border;
    int32_t clientW = w - 2 * border;
    int32_t clientH = h - 2 * border;
    DrawBackground(buffer, bufferW, absX, absY, w, h);
    double progress = mProgress.load();
    bool indeterminate = mIndeterminate.load();
    double phase = mIndeterminatePhase.load();
    int32_t fillX, fillY, fillW, fillH;
    if(indeterminate) {
      double blockWidth = 0.3;
      double start = phase * (1.0 - blockWidth);
      if(Orient() == AUIOrientation::horizontal) {
        int32_t blockPix = static_cast<int32_t>(clientW * blockWidth);
        int32_t startPix = static_cast<int32_t>(clientW * start);
        fillX = clientX + startPix;
        fillY = clientY;
        fillW = blockPix;
        fillH = clientH;
      }
      else {
        int32_t blockPix = static_cast<int32_t>(clientH * blockWidth);
        int32_t startPix = static_cast<int32_t>(clientH * start);
        fillX = clientX;
        fillY = clientY + startPix;
        fillW = clientW;
        fillH = blockPix;
      }
    }
    else {
      GetFillRect(clientX, clientY, clientW, clientH, progress, fillX, fillY, fillW, fillH);
    }
// 6. Intersect fill rect with visible area to avoid drawing outside
    int32_t visL = std::max(fillX, drawL);
    int32_t visT = std::max(fillY, drawT);
    int32_t visR = std::min(fillX + fillW, drawR);
    int32_t visB = std::min(fillY + fillH, drawB);
    if(visL < visR && visT < visB) {
      DrawBar(buffer, bufferW, visL, visT, visR - visL, visB - visT);
      if(mStripe) {
        DrawStripe(buffer, bufferW, visL, visT, visR - visL, visB - visT);
      }
    }
// 7. Draw centered text (cached)
    if(mShowText) {
      std::string text;
      {
        std::lock_guard < std::mutex > lock(mCacheMutex);
        text = mCachedText;
      }
      if(!text.empty()) {
        AUI* au = Wnd() ? Wnd()->EnginePtr() : nullptr;
        if(au) {
          FT_Face face = au->DefaultFontFace();
          if(face) {
            ARect bounds { clientX, clientY, static_cast<uint32_t>(clientW), static_cast<uint32_t>(clientH) };
            ARect clipBounds { drawL, drawT, static_cast<uint32_t>(drawR - drawL), static_cast<uint32_t>(drawB - drawT) };
            ATextStyle style { mTextColor, mFontSize, AUIHAlign::center, AUIVAlign::center, 0.0 };
            DrawTextEx(buffer, bufferW, bufferH, bounds, text, face, style, &clipBounds);
          }
        }
      }
    }
  }

  AProgressBar::~AProgressBar() {
    D2("AProgressBar destructor");
    mStopThread = true;
    mThreadCv.notify_all();
    if(mUpdateThread.joinable())
      mUpdateThread.join();
  }

}// namespace aui
