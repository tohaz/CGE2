#include "AUILib.h"

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
static void OnVerticalScroll(AWidget*, void* data, int32_t val) {
  ScrollState* state = static_cast<ScrollState*>(data);
  state->vScroll = val;
  UpdateButtonPositions(state);
}

// ------------------------------------------------------------------
// Callback for horizontal scrollbar
// ------------------------------------------------------------------
static void OnHorizontalScroll(AWidget*, void* data, int32_t val) {
  ScrollState* state = static_cast<ScrollState*>(data);
  state->hScroll = val;
  UpdateButtonPositions(state);
}

// ------------------------------------------------------------------
// Callback for "Toggle V‑Scroll" button
// ------------------------------------------------------------------
static AWidget* OnToggleVScroll(AWidget* wi, void* data, int32_t, int32_t) {
  ScrollState* state = static_cast<ScrollState*>(data);
  if(state->vScrollBar->Visible())
    state->vScrollBar->Hide();
  else
    state->vScrollBar->Show();
  return wi;
}

// ------------------------------------------------------------------
// Callback for "Toggle H‑Scroll" button
// ------------------------------------------------------------------
static AWidget* OnToggleHScroll(AWidget* wi, void* data, int32_t, int32_t) {
  ScrollState* state = static_cast<ScrollState*>(data);
  if(state->hScrollBar->Visible())
    state->hScrollBar->Hide();
  else
    state->hScrollBar->Show();
  return wi;
}

// ------------------------------------------------------------------
// Callback for "Close App" button
// ------------------------------------------------------------------
static AWidget* OnCloseApp(AWidget* wi, void* data, int32_t, int32_t) {
  ScrollState* state = static_cast<ScrollState*>(data);
  if(state->engine) state->engine->ExitAUI();
  return wi;
}

// ------------------------------------------------------------------
// Main
// ------------------------------------------------------------------
int32_t main() {
  // Create engine and window
  AUI* au = AUI::Create("ScrollBar with Buttons");
  if(!au) return 1;
  AWindow* w = au->MainWnd();
  w->BGColor(0xFF222222);
  w->EnableResize();
  w->Resize(800, 600);
  w->DisableResize();
  // Info label
  ALabel* info = ALabel::AttachTo(w, "Scrollbars move buttons in opposite direction");
  info->Move(10, 10);
  info->Resize(780, 30);
  info->BGColor(0xFF444444);
  info->TextColor(0xFFFFFFFF);
  info->HAlign(AUIHAlign::center);
  // Vertical scrollbar
  AScrollBar* vScroll = AScrollBar::AttachTo(w, AUIOrientation::vertical);
  vScroll->Move(784, 40);
  vScroll->Resize(16, 520);
  vScroll->Range(0, 200);
  vScroll->PageStep(50);
  vScroll->Value(0);
  vScroll->TrackThick(4);
  vScroll->ThumbThick(8);
  vScroll->TrackColor(0xFF666666);
  vScroll->ThumbColor(0xFFAAAAAA);
  // Horizontal scrollbar
  AScrollBar* hScroll = AScrollBar::AttachTo(w, AUIOrientation::horizontal);
  hScroll->Move(40, 584);
  hScroll->Resize(720, 16);
  hScroll->Range(0, 200);
  hScroll->PageStep(50);
  hScroll->Value(0);
  hScroll->TrackThick(4);
  hScroll->ThumbThick(8);
  hScroll->TrackColor(0xFF666666);
  hScroll->ThumbColor(0xFFAAAAAA);
  // Three buttons
  AButton* btnV = AButton::AttachTo(w, "Toggle V-Scroll");
  btnV->Resize(120, 30);
  btnV->BGColor(0xFF999999);
  btnV->TextColor(0xFFFFFFFF);
  btnV->Border(1);
  btnV->BorderColor(0xFFAAAAAA);
  AButton* btnH = AButton::AttachTo(w, "Toggle H-Scroll");
  btnH->Resize(120, 30);
  btnH->BGColor(0xFF44CC88);
  btnH->TextColor(0xFFFFFFFF);
  btnH->Border(1);
  btnH->BorderColor(0xFFAAAAAA);
  AButton* btnClose = AButton::AttachTo(w, "Close App");
  btnClose->Resize(120, 30);
  btnClose->BGColor(0xFFCC8844);
  btnClose->TextColor(0xFFFFFFFF);
  btnClose->Border(1);
  btnClose->BorderColor(0xFFAAAAAA);
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
  btnV->SetMouseClickCallback(OnToggleVScroll, &state);
  btnH->SetMouseClickCallback(OnToggleHScroll, &state);
  btnClose->SetMouseClickCallback(OnCloseApp, &state);
  // Run event loop
  au->ProcessMessages();

  delete au;
  return 0;
}
