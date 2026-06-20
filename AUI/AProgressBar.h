#ifndef APROGRESSBAR_H_
#define APROGRESSBAR_H_

namespace aui {

  class AProgressBar: public AWidget {
    public:
      AProgressBar();
      virtual ~AProgressBar();
      static AProgressBar* AttachTo(AWindow *parent);
      static AProgressBar* AttachTo(AWidget *parent);
      void SetProgress(double progress);
      double GetProgress() const;
      void Clear();
      void SetRange(double min, double max);
      double GetMin() const {
        return mMin;
      }
      double GetMax() const {
        return mMax;
      }
      void SetIndeterminate(bool enable);
      bool IsIndeterminate() const {
        return mIndeterminate.load();
      }
      void SetShowText(bool show);
      bool IsTextVisible() const {
        return mShowText;
      }
      void SetTextFormat(const std::string &format);
      std::string GetTextFormat() const {
        return mTextFormat;
      }
      void SetBarColor(uint32_t color);
      uint32_t GetBarColor() const {
        return mBarColor;
      }
      void SetBarColor2(uint32_t color);
      uint32_t GetBarColor2() const {
        return mBarColor2;
      }
      void SetOrientation(AUIOrientation orient);
      AUIOrientation GetOrientation() const {
        return mOrientation;
      }
      void SetDirection(AUIDirection dir);
      AUIDirection GetDirection() const {
        return mDirection;
      }
      void SetStripe(bool enable);
      bool IsStripeEnabled() const {
        return mStripe;
      }
      void SetStripeColor(uint32_t color);
      uint32_t GetStripeColor() const {
        return mStripeColor;
      }
      void SetStripeWidth(uint32_t pixels);
      uint32_t GetStripeWidth() const {
        return mStripeWidth;
      }
      void SetStripeSpeed(int32_t pixelsPerUpdate);
      int32_t GetStripeSpeed() const {
        return mStripeSpeed;
      }
      void SetRoundedCorners(bool enable, uint32_t radius = 8);
      bool HasRoundedCorners() const {
        return mRoundedCorners;
      }
      uint32_t GetCornerRadius() const {
        return mCornerRadius;
      }
      using ProgressCallback = std::function<void(double)>;
      void SetOnProgressChanged(ProgressCallback cb);
      void SetOnStart(ProgressCallback cb);
      void SetOnComplete(ProgressCallback cb);
      void SetUpdateInterval(uint32_t intervalMs);
      uint32_t GetUpdateInterval() const {
        return mUpdateIntervalMs;
      }
      void SetProgressProvider(std::function<double()> provider);
      void PauseUpdates(bool pause);
      bool IsPaused() const {
        return mPaused;
      }
      virtual void Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
          int32_t offsetY) const override;
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
      uint32_t mBarColor;
      uint32_t mBarColor2;
      AUIOrientation mOrientation;
      AUIDirection mDirection;
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
      bool mStarted;
      void ThreadFunction();
      void FireCallbacks(double oldProgress, double newProgress);
      void GetFillRect(int32_t clientX, int32_t clientY, int32_t clientW, int32_t clientH, double progress,
          int32_t &outX, int32_t &outY, int32_t &outW, int32_t &outH) const;
      void DrawBar(uint32_t *buffer, uint32_t parentWidth, int32_t x, int32_t y, int32_t w, int32_t h) const;
      void DrawStripe(uint32_t *buffer, uint32_t parentWidth, int32_t x, int32_t y, int32_t w, int32_t h) const;
      void DrawBackground(uint32_t *buffer, uint32_t parentWidth, int32_t x, int32_t y, int32_t w, int32_t h) const;
      void UpdateTextCache();
  };

}// namespace aui

#endif // APROGRESSBAR_H_
