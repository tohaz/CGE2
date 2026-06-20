//#include "AUILib.h"
//
//using namespace aui;
//
//
//int main() {
//  AUI* au = AUI::Create("main window");
//  AWindow* w = au->MainWnd();
//  w->EnableResize();
//  w->Resize(200, 200);
//  AUIWindowType type = au->GetWindowType();
//  if(type == AUIWindowType::Wayland) {
//    AWindow *ww = AWindow::AttachTo(au, "additional window Wayland", AUIWindowType::Wayland);
//    ww->EnableResize();
//    ww->Resize(300, 300);
//  }
//  AWindow *wx = AWindow::AttachTo(au, "additional window X11", AUIWindowType::XCB);
//  wx->Move(200, 10);
//  au->ProcessMessages();
//  delete au;
//  return 0;
//}
//
//
//
//
//


//#include "AUILib.h"
//
//using namespace aui;
//
//std::string generate_random_alphanumeric(size_t length);
//std::string generate_random_alphanumeric(size_t length) {
//  const std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
//  static std::random_device rd;
//  static std::mt19937 generator(rd());
//  std::uniform_int_distribution<size_t> distribution(0, charset.size() - 1);
//  std::string random_string;
//  random_string.reserve(length);
//  for(size_t i = 0; i < length; ++i) {
//    random_string += charset[distribution(generator)];
//  }
//  return random_string;
//}
//
//int32_t main() {
//  UNUSED AUI *au = AUI::Create("Table Demo2");
//  AWindow *w = au->MainWnd();
//  w->EnableResize();
//  w->Resize(1024, 768);
//  ABox *bx = ABox::AttachTo(w);
//  bx->Move(0, 0);
//  bx->Resize(1024, 768);
//  auto start = std::chrono::steady_clock::now();
//
//  UNUSED ATable *ta = ATable::AttachTo(bx);
//  ta->AddColumns(2);
//  ta->AddRows(2);
//  ta->Resize(100, 100);
//  ta->Move(0, 0);
//  ta->Resize(1024, 768);
//  ta->SetBGColor(0xFFFFFFFF);
//  ta->SetAutoWiden(false);
//  ta->BeginBatch();
//    for(int32_t i = 0; i < 100000; i++) {
//      for(int32_t j = 0; j < 10; j++)
////        ta->SetCellData(i, j, "a");
//        ta->SetCellData(i, j, generate_random_alphanumeric(1));
//  }
//  ta->EndBatch();
//  ta->SetScrollbarsEnabled(true);
//  //1300ms for my setup for 10 random chars and million cells
//  D1("init {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count());
//  au->ProcessMessages();
//  delete au;
//  return 0;
//}

#include <cmath>
#include "AUILib.h"
using namespace aui;

#include <cmath>
#include "AUILib.h"
using namespace aui;

#include <cmath>
#include "AUILib.h"
using namespace aui;

int32_t test_scrollbar_large_range() {
    D1("test_scrollbar_large_range start");

    AScrollBar sb;
    sb.SetOrientation(AUIOrientation::vertical);
    sb.Resize(20, 200);                 // widget height = 200 px
    sb.SetRange(0, 10000000);           // huge range
    sb.SetPageStep(1000);
    sb.SetSingleStep(100);
    sb.SetShowArrows(true);             // CRITICAL: arrows ON – trackStart = 12

    // Initial geometry
    const uint32_t arrowSize = 12;       // default
    const uint32_t trackStart = arrowSize;
    uint32_t thumbPos = sb.GetThumbPosition();   // 0 at min
    uint32_t thumbLen = sb.GetThumbLength();     // min 20
    TEST_ASSERT(thumbLen >= 20, 1);
    TEST_ASSERT_EQ(thumbPos, 0U, 2);

    // ------------------------------------------------------------------
    // 1. Click on the thumb – thumb position MUST NOT change
    // ------------------------------------------------------------------
    int32_t thumbCenterY = SafeINT32(trackStart + thumbPos + thumbLen / 2);
    bool consumed = sb.OnMouseClick(10, thumbCenterY, true);
    TEST_ASSERT(consumed, 3);
    // If the bug is present, thumb will jump → this assertion fails
    TEST_ASSERT_EQ(sb.GetThumbPosition(), 0U, 4);

    // ------------------------------------------------------------------
    // 2. Drag down by exactly 50 pixels – position must match expected
    // ------------------------------------------------------------------
    int32_t newY = thumbCenterY + 50;
    sb.OnMouseMove(10, newY);

    uint32_t trackLen = sb.GetTrackLength();   // 200 - 2*12 = 176
    int32_t maxMovable = SafeINT32(trackLen - thumbLen);
    // Compute expected thumb position manually (same logic as fixed code)
    int32_t dragOffset = thumbCenterY - SafeINT32(trackStart + thumbPos);
    int32_t newThumbTop = newY - dragOffset;
    int32_t expectedThumbPos = newThumbTop - SafeINT32(trackStart);
    if (expectedThumbPos < 0) expectedThumbPos = 0;
    if (expectedThumbPos > maxMovable) expectedThumbPos = maxMovable;

    uint32_t actualThumbPos = sb.GetThumbPosition();
    // Without drag offset, this will differ → test fails
    TEST_ASSERT_EQ(actualThumbPos, static_cast<uint32_t>(expectedThumbPos), 5);

    // ------------------------------------------------------------------
    // 3. Release – position must stay the same
    // ------------------------------------------------------------------
    consumed = sb.OnMouseClick(10, newY, false);
    TEST_ASSERT(!consumed, 6);
    TEST_ASSERT_EQ(sb.GetThumbPosition(), actualThumbPos, 7);

    // ------------------------------------------------------------------
    // 4. Click in track below thumb – must jump to a higher position
    // ------------------------------------------------------------------
    sb.SetValue(500000);
    thumbPos = sb.GetThumbPosition();
    thumbLen = sb.GetThumbLength();
    int32_t clickBelow = SafeINT32(trackStart + thumbPos + thumbLen + 20);
    if (clickBelow > SafeINT32(trackStart + trackLen))
        clickBelow = SafeINT32(trackStart + trackLen) - 1;
    consumed = sb.OnMouseClick(10, clickBelow, true);
    TEST_ASSERT(consumed, 8);
    uint32_t newPos = sb.GetThumbPosition();
    TEST_ASSERT(newPos > thumbPos, 9);
    TEST_ASSERT(newPos <= trackLen - sb.GetThumbLength(), 10);

    // ------------------------------------------------------------------
    // 5. Click in track above thumb – must jump to a lower position
    // ------------------------------------------------------------------
    sb.SetValue(9000000);
    thumbPos = sb.GetThumbPosition();
    int32_t clickAbove = SafeINT32(trackStart + thumbPos) - 20;
    if (clickAbove < SafeINT32(trackStart)) clickAbove = SafeINT32(trackStart);
    consumed = sb.OnMouseClick(10, clickAbove, true);
    TEST_ASSERT(consumed, 11);
    newPos = sb.GetThumbPosition();
    TEST_ASSERT(newPos < thumbPos, 12);
    TEST_ASSERT(newPos <= trackLen - sb.GetThumbLength(), 13);

    D1("test_scrollbar_large_range passed");
    return 0;
}

int32_t test_scrollbar_regression() {
    D1("test_scrollbar_regression start");

    AScrollBar sb;
    sb.SetOrientation(AUIOrientation::vertical);
    sb.Resize(20, 200);                 // height = 200 px
    sb.SetRange(0, 10000000);           // huge range
    sb.SetPageStep(1000);
    sb.SetSingleStep(100);
    sb.SetShowArrows(true);             // CRITICAL: arrows ON – trackStart = 12

    // Get thumb geometry
    uint32_t thumbPos = sb.GetThumbPosition();   // should be 0 at min
    uint32_t thumbLen = sb.GetThumbLength();     // will be clamped to 20
    uint32_t trackStart = 12;                    // default arrowSize

    // Click on the center of the thumb (absolute coordinate)
    int32_t thumbCenterY = SafeINT32(trackStart + thumbPos + thumbLen / 2);
    bool consumed = sb.OnMouseClick(10, thumbCenterY, true);
    TEST_ASSERT(consumed, 1);

    // If the bug is present, the value will have jumped to a large number.
    // With the fix, the value remains 0.
    int32_t newValue = sb.GetValue();
    TEST_ASSERT_EQ(newValue, 0, 2);   // this will fail if the bug returns

    D1("test_scrollbar_regression passed");
    return 0;
}



int main() {
    // ...
    // ...
}










