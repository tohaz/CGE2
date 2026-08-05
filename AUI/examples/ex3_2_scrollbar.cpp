#include "AUILib.h"

using namespace aui;

// ------------------------------------------------------------
// State for one scrollable container
// ------------------------------------------------------------
struct ContainerState {
    ABox* container;
    ABox* box;
    AScrollBar* vScroll;
    AScrollBar* hScroll;
};

static AWidget* OnClick(AWidget* wid, UNUSED void* data, int32_t, int32_t) {
  ABox* bo = static_cast<ABox*>(wid);
  bo->BGColor(GetDistinctRandomARGBColor(bo->BGColor(), 64));
  return wid;
}
// ------------------------------------------------------------
// Scrollbar callback – moves the box inside its container
// ------------------------------------------------------------
static void OnScrollWidget(AWidget*/*widget*/, void *userData, int32_t/*value*/) {
  ContainerState* state = static_cast<ContainerState*>(userData);
  if(!state)
    return;
// Move box to current scroll positions
  state->box->Move(state->hScroll->Value(), state->vScroll->Value());
}

static AWidget* OnScrollWindowHoriz(AWidget *w, void *userData, int32_t/*value*/) {
  AScrollBar* s = (AScrollBar*) w;
  D2("callback H val {}", s->Value())
  ABox* b = (ABox*) userData;
  b->Move(s->Value(), b->Y());
  return w;
}

static AWidget* OnScrollWindowVert(AWidget *w, void *userData, int32_t/*value*/) {
  AScrollBar* s = (AScrollBar*) w;
  D2("callback V val {}", s->Value())
  ABox* b = (ABox*) userData;
  b->Move(b->X(), s->Value());
  return w;
}

// ------------------------------------------------------------
// Main – creates two independent scrollable areas
// ------------------------------------------------------------
int main() {
// 1. Create AUI and window
  AUI* au = AUI::Create("Scrollbar Test");
  AWindow* win = au->MainWnd();
  win->EnableResize();
  win->Resize(500, 500);
  win->DisableResize();
  win->BGColor(0xFF222222);
// ========== Container A (left) ==========
  ContainerState bst;
// Container box (viewport)
  bst.container = ABox::AttachTo(win);
  bst.container->Move(10, 10);
  bst.container->Resize(230, 230);
  bst.container->BGColor(0xFF333366);
  bst.container->ClipChildren(true);
  bst.container->Border(2);
  bst.container->BorderColor(0xFFAAAAAA);
// Movable box inside
  bst.box = ABox::AttachTo(bst.container);
  bst.box->Move(20, 20);
  bst.box->Resize(30, 30);
  bst.box->Angle(45);
  bst.box->BGColor(0xFF00FF00);
  bst.box->SetMousePressLeftCallback(OnClick, nullptr);
// Vertical scrollbar
  bst.vScroll = AScrollBar::AttachTo(bst.container);
  bst.vScroll->Move(230 - 20, 0);
  bst.vScroll->Resize(20, 220);
  bst.vScroll->Range(0, 230);
  bst.vScroll->PageStep(30);
  bst.vScroll->SetScrollCallback(OnScrollWidget, &bst);
  bst.vScroll->ThumbColor(0xFF8888FF);
  bst.vScroll->TrackColor(0xFF444444);
  bst.vScroll->Text("small vertical scrollbar");
// Horizontal scrollbar
  bst.hScroll = AScrollBar::AttachTo(bst.container);
  bst.hScroll->Move(0, 230 - 20);
  bst.hScroll->Resize(230 - 20, 20);
  bst.hScroll->Range(0, 230);
  bst.hScroll->PageStep(30);
  bst.hScroll->SetScrollCallback(OnScrollWidget, &bst);
  bst.hScroll->ThumbColor(0xFFFF8888);
  bst.hScroll->TrackColor(0xFF444444);
  bst.hScroll->Orient(AUIOrientation::horizontal);
  bst.hScroll->Text("small horizontal scrollbar");

// Initial position
  bst.vScroll->Value(20);
  bst.hScroll->Value(20);

  AScrollBar* hs = AScrollBar::AttachTo(win);
  hs->Resize(480, 20);
  hs->Move(0, 480);
  hs->Range(0, 500);
  hs->PageStep(230);
  hs->Orient(AUIOrientation::horizontal);
  hs->SetScrollCallback(OnScrollWindowHoriz, bst.container);
  hs->Text("big horizontal scrollbar");

  AScrollBar* vs = AScrollBar::AttachTo(win);
  vs->Resize(20, 480);
  vs->Move(480, 0);
  vs->Range(0, 500);
  vs->PageStep(230);
  vs->SetScrollCallback(OnScrollWindowVert, bst.container);
  vs->Text("big vertical scrollbar");
// 2. Run event loop
  au->ProcessMessages();

  delete au;
  return 0;
}

