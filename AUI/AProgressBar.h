#ifndef APROGRESSBAR_H_
#define APROGRESSBAR_H_

namespace aui {
  using ProgressCallback = std::function<void(double)>;

  class AProgressBar: public AWidgetFactory<AProgressBar> {
      friend class AWidgetFactory<AProgressBar>;
    private:
      double mMin, mMax;
      std::atomic<double> mProgress;
      std::atomic<bool> mIndeterminate;
      std::atomic<double> mIndeterminatePhase;
      double mIndeterminateSpeed;
      std::thread mUpdateThread;
      mutable std::mutex mThreadMutex;
      std::condition_variable mThreadCv;
      std::atomic<bool> mStopThread;
      std::atomic<bool> mPaused;
      uint32_t mUpdateIntervalMs;
      std::function<double()> mProgressProvider;
      bool mShowText;
      std::string mTextFormat;
      mutable std::mutex mCacheMutex;
      std::string mCachedText;
      bool mStripe;
      uint32_t mStripeColor;
      uint32_t mStripeWidth;
      int32_t mStripeSpeed;
      std::atomic<int32_t> mStripeOffset;
      bool mRoundedCorners;
      uint32_t mCornerRadius;
      ProgressCallback mOnProgressChanged;
      ProgressCallback mOnStart;
      ProgressCallback mOnComplete;
      uint32_t mClearColor = 0;
      bool mStarted;
      void ThreadFunction();
      void FireCallbacks(double oldProgress, double newProgress);
      void GetFillRect(int32_t clientX, int32_t clientY, int32_t clientW, int32_t clientH, double progress,
          int32_t &outX, int32_t &outY, int32_t &outW, int32_t &outH) const;
      void DrawBar(uint32_t *buffer, uint32_t parentWidth, int32_t x, int32_t y, int32_t w, int32_t h) const;
      void DrawStripe(uint32_t *buffer, uint32_t parentWidth, int32_t x, int32_t y, int32_t w, int32_t h) const;
      void DrawBackground(uint32_t* buffer, uint32_t parentWidth, int32_t x, int32_t y, int32_t w, int32_t h) const;


      void UpdateTextCache();
    public:
      AProgressBar();
      virtual ~AProgressBar();
      void OnDraw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH,
                  int32_t offsetX, int32_t offsetY,
                  int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const override;
      void Progress(double progress);
      double Progress() const;
      void Clear();
      void Range(double min, double max);
      double Min() const {return mMin;}
      double Max() const {return mMax;}
      void Indeterminate(bool enable);
      bool IsIndeterminate() const {
        return mIndeterminate.load();
      }
      void ShowText(bool show);
      bool IsTextVisible() const {return mShowText;}
      void TextFormat(const std::string &format);
      std::string TextFormat() const {return mTextFormat;}
      void Stripe(bool enable);
      bool IsStripeEnabled() const {return mStripe;}
      void StripeColor(uint32_t color);
      uint32_t StripeColor() const {return mStripeColor;}
      void StripeWidth(uint32_t pixels);
      uint32_t StripeWidth() const {return mStripeWidth;}
      void StripeSpeed(int32_t pixelsPerUpdate);
      int32_t StripeSpeed() const {return mStripeSpeed;}
      void RoundedCorners(bool enable, uint32_t radius = 8);
      bool RoundedCorners() const {return mRoundedCorners;}
      uint32_t CornerRadius() const {return mCornerRadius;}
      void SetOnProgressChanged(ProgressCallback cb);
      void SetOnStart(ProgressCallback cb);
      void SetOnComplete(ProgressCallback cb);
      void UpdateInterval(uint32_t intervalMs);
      uint32_t UpdateInterval() const {return mUpdateIntervalMs;}
      void SetProgressProvider(std::function<double()> provider);
      void PauseUpdates(bool pause);
      bool IsPaused() const {return mPaused;}
      std::atomic<bool> mProviderPending{false};
      void ClearColor(uint32_t color) {mClearColor = color;};
  };

  }// namespace aui

#endif // APROGRESSBAR_H_
