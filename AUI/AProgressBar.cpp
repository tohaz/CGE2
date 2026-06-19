#include "AProgressBar.h"
#include <algorithm>
#include <string>
#include <cmath>
#include <cstdio>

namespace aui {

static inline uint32_t LerpColor(uint32_t c1, uint32_t c2, double t) {
  if (t <= 0.0) return c1;
  if (t >= 1.0) return c2;
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

void AProgressBar::UpdateTextCache() {
  std::lock_guard<std::mutex> lock(mCacheMutex);
  if (mIndeterminate.load()) {
    mCachedText = "Loading...";
  } else {
    double progress = mProgress.load();
    double value = mMin + progress * (mMax - mMin);
    char buf[64];
    snprintf(buf, sizeof(buf), mTextFormat.c_str(), value);
    mCachedText = buf;
  }
}

AProgressBar::AProgressBar()
  : mMin(0.0), mMax(1.0)
  , mProgress(0.0)
  , mIndeterminate(false)
  , mIndeterminatePhase(0.0)
  , mIndeterminateSpeed(0.02)
  , mStopThread(false)
  , mPaused(false)
  , mUpdateIntervalMs(100)
  , mShowText(true)
  , mTextFormat("%d%%")
  , mBarColor(0xFF00AA00)
  , mBarColor2(0)
  , mOrientation(AUIOrientation::horizontal)
  , mDirection(AUIDirection::right)
  , mStripe(false)
  , mStripeColor(0x40FFFFFF)
  , mStripeWidth(5)
  , mStripeSpeed(2)
  , mStripeOffset(0)
  , mRoundedCorners(false)
  , mCornerRadius(0)
  , mStarted(false) {
  D2("AProgressBar constructed");
  mSizeX = 200;
  mSizeY = 30;
  mBGColor = 0xFFCCCCCC;
  mTextColor = 0xFF000000;
  mBorderThick = 1;
  mBorderColor = 0xFF888888;
  mWidgetType = AUIWidgetType::defaultProgressBar;
  UpdateTextCache();
  mUpdateThread = std::thread(&AProgressBar::ThreadFunction, this);
}

AProgressBar::~AProgressBar() {
  D2("AProgressBar destructor");
  mStopThread = true;
  mThreadCv.notify_all();
  if (mUpdateThread.joinable())
    mUpdateThread.join();
}

AProgressBar* AProgressBar::AttachTo(AWindow* parent) {
  if (!parent) { E("AProgressBar::AttachTo: parent window is null"); return nullptr; }
  auto* bar = new AProgressBar();
  parent->AddWidget(std::unique_ptr<AWidget>(bar));
  return bar;
}

AProgressBar* AProgressBar::AttachTo(AWidget* parent) {
  if (!parent) { E("AProgressBar::AttachTo: parent widget is null"); return nullptr; }
  auto* bar = new AProgressBar();
  parent->AddWidget(std::unique_ptr<AWidget>(bar));
  return bar;
}

void AProgressBar::SetProgress(double progress) {
  double old, newP;
  bool changed = false;
  {
    if (mIndeterminate.load()) return;
    if (progress < mMin) progress = mMin;
    if (progress > mMax) progress = mMax;
    double norm = (progress - mMin) / (mMax - mMin);
    norm = std::max(0.0, std::min(1.0, norm));
    old = mProgress.load();
    if (std::abs(old - norm) > 0.000001) {
      mProgress = norm;
      newP = norm;
      UpdateTextCache();
      changed = true;
    }
  }
  if (changed) {
    FireCallbacks(old, newP);
    if (mParentWindow) mParentWindow->Draw();
  }
}

double AProgressBar::GetProgress() const {
  return mProgress.load();
}

void AProgressBar::Clear() {
  SetProgress(mMin);
}

void AProgressBar::SetRange(double min, double max) {
  if (min >= max) { E("AProgressBar::SetRange: min must be < max"); return; }
  mMin = min;
  mMax = max;
  UpdateTextCache();
  if (mParentWindow) mParentWindow->Draw();
}

void AProgressBar::SetIndeterminate(bool enable) {
  mIndeterminate = enable;
  if (enable) mIndeterminatePhase = 0.0;
  UpdateTextCache();
  if (mParentWindow) mParentWindow->Draw();
}

void AProgressBar::SetShowText(bool show) {
  mShowText = show;
  if (mParentWindow) mParentWindow->Draw();
}

void AProgressBar::SetTextFormat(const std::string& format) {
  mTextFormat = format;
  UpdateTextCache();
  if (mParentWindow) mParentWindow->Draw();
}

void AProgressBar::SetBarColor(uint32_t color) {
  mBarColor = color;
  if (mParentWindow) mParentWindow->Draw();
}

void AProgressBar::SetBarColor2(uint32_t color) {
  mBarColor2 = color;
  if (mParentWindow) mParentWindow->Draw();
}

void AProgressBar::SetOrientation(AUIOrientation orient) {
  mOrientation = orient;
  if (mParentWindow) mParentWindow->Draw();
}

void AProgressBar::SetDirection(AUIDirection dir) {
  mDirection = dir;
  if (mParentWindow) mParentWindow->Draw();
}

void AProgressBar::SetStripe(bool enable) {
  mStripe = enable;
  if (mParentWindow) mParentWindow->Draw();
}

void AProgressBar::SetStripeColor(uint32_t color) {
  mStripeColor = color;
  if (mParentWindow) mParentWindow->Draw();
}

void AProgressBar::SetStripeWidth(uint32_t pixels) {
  if (pixels < 1) pixels = 1;
  mStripeWidth = pixels;
  if (mParentWindow) mParentWindow->Draw();
}

void AProgressBar::SetStripeSpeed(int32_t pixelsPerUpdate) {
  mStripeSpeed = pixelsPerUpdate;
}

void AProgressBar::SetRoundedCorners(bool enable, uint32_t radius) {
  mRoundedCorners = enable;
  mCornerRadius = (radius > 0) ? radius : 8;
  if (mParentWindow) mParentWindow->Draw();
}

void AProgressBar::SetOnProgressChanged(ProgressCallback cb) { mOnProgressChanged = cb; }
void AProgressBar::SetOnStart(ProgressCallback cb) { mOnStart = cb; }
void AProgressBar::SetOnComplete(ProgressCallback cb) { mOnComplete = cb; }

void AProgressBar::FireCallbacks(double oldP, double newP) {
  if (newP > 0.0 && !mStarted) {
    mStarted = true;
    if (mOnStart) mOnStart(newP);
  }
  if (mOnProgressChanged && std::abs(newP - oldP) > 0.000001)
    mOnProgressChanged(newP);
  if (newP >= 1.0 && oldP < 1.0) {
    if (mOnComplete) mOnComplete(newP);
  }
}

void AProgressBar::SetUpdateInterval(uint32_t intervalMs) {
  std::lock_guard<std::mutex> lock(mThreadMutex);
  mUpdateIntervalMs = intervalMs;
  mThreadCv.notify_all();
}

void AProgressBar::SetProgressProvider(std::function<double()> provider) {
  {
    std::lock_guard<std::mutex> lock(mThreadMutex);
    mProgressProvider = provider;
  }
  mThreadCv.notify_all();
}

void AProgressBar::PauseUpdates(bool pause) {
  mPaused = pause;
  if (!pause) mThreadCv.notify_all();
}

void AProgressBar::ThreadFunction() {
  while (true) {
    bool needRedraw = false;
    std::function<double()> provider;
    uint32_t interval;
    {
      std::unique_lock<std::mutex> lock(mThreadMutex);
      interval = mUpdateIntervalMs;
      if (mThreadCv.wait_for(lock, std::chrono::milliseconds(interval),
                             [this] { return mStopThread.load(); }))
        break;
      if (mStopThread.load() || mPaused.load()) continue;
      provider = mProgressProvider;
    }
    if (mIndeterminate.load()) {
      double phase = mIndeterminatePhase.load();
      phase += mIndeterminateSpeed;
      if (phase > 1.0) phase -= 1.0;
      mIndeterminatePhase = phase;
      if (mStripe) {
        mStripeOffset = mStripeOffset.load() + mStripeSpeed;
      }
      needRedraw = true;
    } else if (provider) {
      double raw;
      try { raw = provider(); } catch (...) { raw = mMin; }
      if (raw < mMin) raw = mMin;
      if (raw > mMax) raw = mMax;
      double norm = (raw - mMin) / (mMax - mMin);
      norm = std::max(0.0, std::min(1.0, norm));
      double old = mProgress.load();
      if (std::abs(norm - old) > 0.000001) {
        mProgress = norm;
        UpdateTextCache();
        FireCallbacks(old, norm);
        needRedraw = true;
      }
      if (mStripe) {
        mStripeOffset = mStripeOffset.load() + mStripeSpeed;
        needRedraw = true;
      }
    }
    if (needRedraw && mParentWindow)
      mParentWindow->Draw();
  }
}

void AProgressBar::GetFillRect(int32_t clientX, int32_t clientY,
                               int32_t clientW, int32_t clientH,
                               double progress,
                               int32_t& outX, int32_t& outY,
                               int32_t& outW, int32_t& outH) const {
  double p = std::max(0.0, std::min(1.0, progress));
  if (mOrientation == AUIOrientation::horizontal) {
    int32_t fill = static_cast<int32_t>(clientW * p);
    switch (mDirection) {
      case AUIDirection::right:
        outX = clientX; outY = clientY; outW = fill; outH = clientH; break;
      case AUIDirection::left:
        outX = clientX + clientW - fill; outY = clientY; outW = fill; outH = clientH; break;
      default:
        outX = clientX; outY = clientY; outW = fill; outH = clientH; break;
    }
  } else {
    int32_t fill = static_cast<int32_t>(clientH * p);
    switch (mDirection) {
      case AUIDirection::top:
        outX = clientX; outY = clientY; outW = clientW; outH = fill; break;
      case AUIDirection::bottom:
        outX = clientX; outY = clientY + clientH - fill; outW = clientW; outH = fill; break;
      default:
        outX = clientX; outY = clientY; outW = clientW; outH = fill; break;
    }
  }
}

void AProgressBar::DrawBackground(uint32_t* buffer, uint32_t parentWidth,
                                  int32_t x, int32_t y, int32_t w, int32_t h) const {
  if (w <= 0 || h <= 0) return;
  uint32_t bg = mBGColor & 0x00FFFFFF;
  bool rounded = mRoundedCorners && mCornerRadius > 0;
  int32_t r = static_cast<int32_t>(mCornerRadius);
  if (r > w / 2) r = w / 2;
  if (r > h / 2) r = h / 2;
  if (!rounded || r <= 0) {
    FillRect(buffer, parentWidth, x, y, w, h, bg);
    return;
  }
  int32_t rSq = r * r;
  for (int32_t row = 0; row < h; ++row) {
    int32_t startX = x;
    int32_t endX = x + w;
    if (row < r) {
      int32_t dy = r - row;
      int32_t dx = r - static_cast<int32_t>(std::sqrt(rSq - dy * dy));
      startX = x + dx;
      endX = x + w - dx;
    } else if (row >= h - r) {
      int32_t dy = row - (h - 1 - r);
      int32_t dx = r - static_cast<int32_t>(std::sqrt(rSq - dy * dy));
      startX = x + dx;
      endX = x + w - dx;
    }
    if (startX < endX) {
      size_t idx = static_cast<size_t>(y + row) * parentWidth + static_cast<size_t>(startX);
      std::fill_n(&buffer[idx], endX - startX, bg);
    }
  }
}

void AProgressBar::DrawBar(uint32_t* buffer, uint32_t parentWidth,
                           int32_t x, int32_t y, int32_t w, int32_t h) const {
  if (w <= 0 || h <= 0) return;
  const bool hasGradient = (mBarColor2 != 0);
  const bool horizontal = (mOrientation == AUIOrientation::horizontal);
  bool rounded = mRoundedCorners && mCornerRadius > 0;
  int32_t r = static_cast<int32_t>(mCornerRadius);
  if (r > w / 2) r = w / 2;
  if (r > h / 2) r = h / 2;
  if (!rounded || r <= 0) {
    if (hasGradient) {
      if (horizontal) {
        for (int32_t col = 0; col < w; ++col) {
          uint32_t c = LerpColor(mBarColor, mBarColor2, static_cast<double>(col) / w);
          FillRect(buffer, parentWidth, x + col, y, 1, h, c);
        }
      } else {
        for (int32_t row = 0; row < h; ++row) {
          uint32_t c = LerpColor(mBarColor, mBarColor2, static_cast<double>(row) / h);
          FillRect(buffer, parentWidth, x, y + row, w, 1, c);
        }
      }
    } else {
      FillRect(buffer, parentWidth, x, y, w, h, mBarColor);
    }
    return;
  }
  int32_t rSq = r * r;
  for (int32_t row = 0; row < h; ++row) {
    int32_t startX = x;
    int32_t endX = x + w;
    if (row < r) {
      int32_t dy = r - row;
      int32_t dx = r - static_cast<int32_t>(std::sqrt(rSq - dy * dy));
      startX = x + dx; endX = x + w - dx;
    } else if (row >= h - r) {
      int32_t dy = row - (h - 1 - r);
      int32_t dx = r - static_cast<int32_t>(std::sqrt(rSq - dy * dy));
      startX = x + dx; endX = x + w - dx;
    }
    if (startX >= endX) continue;
    size_t lineOffset = static_cast<size_t>(y + row) * parentWidth;
    size_t uCount = static_cast<size_t>(endX - startX);
    if (hasGradient) {
      if (horizontal) {
        for (int32_t px = startX; px < endX; ++px) {
          uint32_t c = LerpColor(mBarColor, mBarColor2, static_cast<double>(px - x) / w);
          buffer[lineOffset + static_cast<size_t>(px)] = c;
        }
      } else {
        uint32_t c = LerpColor(mBarColor, mBarColor2, static_cast<double>(row) / h);
        std::fill_n(&buffer[lineOffset + static_cast<size_t>(startX)], uCount, c);
      }
    } else {
      std::fill_n(&buffer[lineOffset + static_cast<size_t>(startX)], uCount, mBarColor);
    }
  }
}

void AProgressBar::DrawStripe(uint32_t* buffer, uint32_t parentWidth,
                              int32_t x, int32_t y, int32_t w, int32_t h) const {
  if (w <= 0 || h <= 0 || !mStripe) return;
  bool rounded = mRoundedCorners && mCornerRadius > 0;
  int32_t r = static_cast<int32_t>(mCornerRadius);
  if (r > w / 2) r = w / 2;
  if (r > h / 2) r = h / 2;
  int32_t stripeW = static_cast<int32_t>(mStripeWidth);
  int32_t modPeriod = stripeW * 2;
  int32_t initialOffset = mStripeOffset.load() % modPeriod;
  if (initialOffset < 0) initialOffset += modPeriod;
  int32_t rSq = r * r;
  for (int32_t row = 0; row < h; ++row) {
    int32_t startX = x;
    int32_t endX = x + w;
    if (rounded && r > 0) {
      if (row < r) {
        int32_t dy = r - row;
        int32_t dx = r - static_cast<int32_t>(std::sqrt(rSq - dy * dy));
        startX = x + dx; endX = x + w - dx;
      } else if (row >= h - r) {
        int32_t dy = row - (h - 1 - r);
        int32_t dx = r - static_cast<int32_t>(std::sqrt(rSq - dy * dy));
        startX = x + dx; endX = x + w - dx;
      }
    }
    if (startX >= endX) continue;
    int32_t relX = (startX - x - initialOffset) % modPeriod;
    if (relX < 0) relX += modPeriod;
    size_t idx = static_cast<size_t>(y + row) * parentWidth + static_cast<size_t>(startX);
    for (int32_t px = startX; px < endX; ++px) {
      if (relX < stripeW) {
        buffer[idx] = mStripeColor;
      }
      idx++;
      relX++;
      if (relX >= modPeriod) relX = 0;
    }
  }
}

void AProgressBar::Draw(uint32_t* buffer, uint32_t parentWidth, uint32_t parentHeight,
                        int32_t offsetX, int32_t offsetY) const {
  D3("AProgressBar::Draw");
  int32_t absX = offsetX + mX;
  int32_t absY = offsetY + mY;
  int32_t drawW = static_cast<int32_t>(mSizeX);
  int32_t drawH = static_cast<int32_t>(mSizeY);
  int32_t pW = static_cast<int32_t>(parentWidth);
  int32_t pH = static_cast<int32_t>(parentHeight);
  if (absX < 0) { drawW += absX; absX = 0; }
  if (absY < 0) { drawH += absY; absY = 0; }
  if (absX + drawW > pW) drawW = pW - absX;
  if (absY + drawH > pH) drawH = pH - absY;
  if (drawW > 0 && drawH > 0)
    DrawBackground(buffer, parentWidth, absX, absY, drawW, drawH);
  if (!(mRoundedCorners && mCornerRadius > 0)) {
    DrawBorder(buffer, parentWidth, parentHeight, offsetX, offsetY);
  }
  int32_t clientX = absX + static_cast<int32_t>(mBorderThick);
  int32_t clientY = absY + static_cast<int32_t>(mBorderThick);
  int32_t clientW = static_cast<int32_t>(mSizeX) - 2 * static_cast<int32_t>(mBorderThick);
  int32_t clientH = static_cast<int32_t>(mSizeY) - 2 * static_cast<int32_t>(mBorderThick);
  if (clientW <= 0 || clientH <= 0) return;

  // Always draw bar (geometry is cheap, and we must redraw every frame)
  double progress = mProgress.load();
  bool indeterminate = mIndeterminate.load();
  double phase = mIndeterminatePhase.load();
  int32_t fillX, fillY, fillW, fillH;
  if (indeterminate) {
    double blockWidth = 0.3;
    double start = phase * (1.0 - blockWidth);
    if (mOrientation == AUIOrientation::horizontal) {
      int32_t blockPix = static_cast<int32_t>(clientW * blockWidth);
      int32_t startPix = static_cast<int32_t>(clientW * start);
      fillX = clientX + startPix; fillY = clientY; fillW = blockPix; fillH = clientH;
    } else {
      int32_t blockPix = static_cast<int32_t>(clientH * blockWidth);
      int32_t startPix = static_cast<int32_t>(clientH * start);
      fillX = clientX; fillY = clientY + startPix; fillW = clientW; fillH = blockPix;
    }
  } else {
    GetFillRect(clientX, clientY, clientW, clientH, progress, fillX, fillY, fillW, fillH);
  }
  if (fillW > 0 && fillH > 0) {
    DrawBar(buffer, parentWidth, fillX, fillY, fillW, fillH);
    if (mStripe) {
      DrawStripe(buffer, parentWidth, fillX, fillY, fillW, fillH);
    }
  }

  // Draw text (cached)
  if (mShowText) {
    std::string text;
    {
      std::lock_guard<std::mutex> lock(mCacheMutex);
      text = mCachedText;
    }
    if (!text.empty()) {
      FT_Face face = mEnginePtr ? mEnginePtr->GetDefaultFontFace() : nullptr;
      if (face) {
        DrawTextEx(buffer, parentWidth, parentHeight,
                   clientX, clientY, clientW, clientH,
                   text, face, mFontSize,
                   AUIHAlign::center, AUIVAlign::center,
                   0, mTextColor, clientW);
      }
    }
  }
}

} // namespace aui
