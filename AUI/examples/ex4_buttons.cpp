#include "AUILib.h"
#include "AWindow.h"
#include "ALabel.h"
#include "AScrollBar.h"
#include "AButton.h"

using namespace aui;

//  ------------------------------------------------------------------
// State structure holding all dynamic data
// ------------------------------------------------------------------
struct ScrollState {
  AUI* engine = nullptr;
  AWindow* window = nullptr;
  AScrollBar* vScrollBar = nullptr;
  AScrollBar* hScrollBar = nullptr;
  AButton* btnToggleV = nullptr;
  AButton* btnToggleH = nullptr;
  AButton* btnClose = nullptr;
  ALabel* infoLabel = nullptr;
  int32_t vScroll = 0;
  int32_t hScroll = 0;
  int32_t baseX = 0;
  int32_t baseY = 0;
};

// ------------------------------------------------------------------
// Helper: update button positions (opposite to scroll offsets)
// ------------------------------------------------------------------
static void UpdateButtonPositions(ScrollState* state) {
  if(!state) return;
  int32_t newX = state->baseX - state->hScroll;
  int32_t newY = state->baseY - state->vScroll;
  if(state->btnToggleV) state->btnToggleV->Move(newX, newY);
  if(state->btnToggleH) state->btnToggleH->Move(newX, newY + 50);
  if(state->btnClose)    state->btnClose->Move(newX, newY + 100);
}

// ------------------------------------------------------------------
// Callback for vertical scrollbar
// ------------------------------------------------------------------
static void OnVerticalScroll(AWindow*, AWidget*, void* data, int32_t val) {
  ScrollState* state = static_cast<ScrollState*>(data);
  state->vScroll = val;
  UpdateButtonPositions(state);
}

// ------------------------------------------------------------------
// Callback for horizontal scrollbar
// ------------------------------------------------------------------
static void OnHorizontalScroll(AWindow*, AWidget*, void* data, int32_t val) {
  ScrollState* state = static_cast<ScrollState*>(data);
  state->hScroll = val;
  UpdateButtonPositions(state);
}

// ------------------------------------------------------------------
// Callback for "Toggle V‑Scroll" button
// ------------------------------------------------------------------
static void OnToggleVScroll(AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
  if(!pressed) return;
  ScrollState* state = static_cast<ScrollState*>(data);
  if(state->vScrollBar->IsVisible())
    state->vScrollBar->Hide();
  else
    state->vScrollBar->Show();
}

// ------------------------------------------------------------------
// Callback for "Toggle H‑Scroll" button
// ------------------------------------------------------------------
static void OnToggleHScroll(AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
  if(!pressed) return;
  ScrollState* state = static_cast<ScrollState*>(data);
  if(state->hScrollBar->IsVisible())
    state->hScrollBar->Hide();
  else
    state->hScrollBar->Show();
}

// ------------------------------------------------------------------
// Callback for "Close App" button
// ------------------------------------------------------------------
static void OnCloseApp(AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
  if(!pressed) return;
  ScrollState* state = static_cast<ScrollState*>(data);
  if(state->engine) state->engine->ExitAUI();
}

// ------------------------------------------------------------------
// Main
// ------------------------------------------------------------------
int32_t main() {
  // Create engine and window
  AUI* au = AUI::Create("ScrollBar with Buttons");
  if(!au) return 1;
  AWindow* w = au->MainWnd();
  w->SetBGColor(0xFF222222);
  w->EnableResize();
  w->Resize(800, 600);
  w->DisableResize();

  // Info label
  ALabel* info = ALabel::AttachTo(w, "Scrollbars move buttons in opposite direction", 10, 10, 780, 30);
  info->SetBGColor(0xFF444444);
  info->SetTextColor(0xFFFFFFFF);
  info->SetHAlignment(AUIHAlign::center);

  // Vertical scrollbar
  AScrollBar* vScroll = AScrollBar::AttachTo(w, AUIOrientation::vertical);
  vScroll->Move(784, 40);
  vScroll->Resize(16, 520);
  vScroll->SetRange(0, 200);
  vScroll->SetPageStep(50);
  vScroll->SetValue(0);
  vScroll->SetTrackThickness(4);
  vScroll->SetThumbThickness(8);
  vScroll->SetTrackColor(0xFF666666);
  vScroll->SetThumbColor(0xFFAAAAAA);

  // Horizontal scrollbar
  AScrollBar* hScroll = AScrollBar::AttachTo(w, AUIOrientation::horizontal);
  hScroll->Move(40, 584);
  hScroll->Resize(720, 16);
  hScroll->SetRange(0, 200);
  hScroll->SetPageStep(50);
  hScroll->SetValue(0);
  hScroll->SetTrackThickness(4);
  hScroll->SetThumbThickness(8);
  hScroll->SetTrackColor(0xFF666666);
  hScroll->SetThumbColor(0xFFAAAAAA);

  // Three buttons
  AButton* btnV = AButton::AttachTo(w, "Toggle V-Scroll");
  btnV->Resize(120, 30);
  btnV->SetBGColor(0xFF999999);
  btnV->SetTextColor(0xFFFFFFFF);
  btnV->SetBorderThickness(1);
  btnV->SetBorderColor(0xFFAAAAAA);

  AButton* btnH = AButton::AttachTo(w, "Toggle H-Scroll");
  btnH->Resize(120, 30);
  btnH->SetBGColor(0xFF44CC88);
  btnH->SetTextColor(0xFFFFFFFF);
  btnH->SetBorderThickness(1);
  btnH->SetBorderColor(0xFFAAAAAA);

  AButton* btnClose = AButton::AttachTo(w, "Close App");
  btnClose->Resize(120, 30);
  btnClose->SetBGColor(0xFFCC8844);
  btnClose->SetTextColor(0xFFFFFFFF);
  btnClose->SetBorderThickness(1);
  btnClose->SetBorderColor(0xFFAAAAAA);

  // Build state object
  ScrollState state;
  state.engine = au;
  state.window = w;
  state.vScrollBar = vScroll;
  state.hScrollBar = hScroll;
  state.btnToggleV = btnV;
  state.btnToggleH = btnH;
  state.btnClose = btnClose;
  state.infoLabel = info;
  state.baseX = 380;   // center X
  state.baseY = 280;   // center Y

  // Set initial button positions
  UpdateButtonPositions(&state);

  // Register callbacks
  vScroll->SetScrollCallback(OnVerticalScroll, &state);
  hScroll->SetScrollCallback(OnHorizontalScroll, &state);
  btnV->SetClickCallback(OnToggleVScroll, &state);
  btnH->SetClickCallback(OnToggleHScroll, &state);
  btnClose->SetClickCallback(OnCloseApp, &state);

  // Run event loop
  au->ProcessMessages();

  delete au;
  return 0;
}


