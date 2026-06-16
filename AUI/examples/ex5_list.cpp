
#include "AUILib.h"
#include "AWindow.h"
#include "AList.h"
#include "AButton.h"
#include <vector>
#include <string>
#include <random>

using namespace aui;

// ------------------------------------------------------------------
// Helper: random space mission names
// ------------------------------------------------------------------
static std::vector<std::string> missionNames = {
  "Apollo 11", "Voyager 1", "Mars Rover", "Hubble Telescope",
  "Cassini-Huygens", "New Horizons", "Rosetta", "Curiosity",
  "Perseverance", "Juno", "Galileo", "Pioneer 10",
  "Sputnik 1", "Vostok 1", "Gemini 4", "Apollo 13",
  "Chandrayaan-3", "Artemis I", "Lunar Gateway", "James Webb"
};

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> dist(0, static_cast<int>(missionNames.size() - 1));
static std::uniform_int_distribution<> yearDist(1957, 2025);

std::string RandomMission() {
  int32_t idx = dist(gen);
  int32_t year = yearDist(gen);
  return missionNames[static_cast<size_t>(idx)] + " (" + std::to_string(year) + ")";
}

struct State {
  AList* list = nullptr;
  AButton* toggleModeBtn = nullptr;
  AButton* addBtn = nullptr;
  AButton* clearSelBtn = nullptr;
  AButton* printSelBtn = nullptr;
  AButton* lineHAlignBtn = nullptr;
  AButton* contentVAlignBtn = nullptr;
  AButton* lineVAlignBtn = nullptr;
  AButton* lineSpacingUpBtn = nullptr;
  AButton* lineSpacingDownBtn = nullptr;
  AButton* fontSizeUpBtn = nullptr;
  AButton* fontSizeDownBtn = nullptr;
  AButton* toggleVScrollBtn = nullptr;
  AButton* toggleHScrollBtn = nullptr;
  bool multiSelect = true;
  int lineHIdx = 0;
  int contentVIdx = 0;
  int lineVIdx = 0;
};

// Alignment cycle callbacks
static void CycleLineHAlign(AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
  if(!pressed) return;
  State* st = static_cast<State*>(data);
  st->lineHIdx = (st->lineHIdx + 1) % 3;
  AUIHAlign align = static_cast<AUIHAlign>(st->lineHIdx + 1);
  st->list->SetHAlignment(align);
  const char* names[] = {"Left", "Center", "Right"};
  st->lineHAlignBtn->SetText(std::string("Line H: ") + names[st->lineHIdx]);
}

static void CycleContentVAlign(AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
  if(!pressed) return;
  State* st = static_cast<State*>(data);
  st->contentVIdx = (st->contentVIdx + 1) % 3;
  AUIVAlign align = static_cast<AUIVAlign>(st->contentVIdx + 1);
  st->list->SetVAlignment(align);
  const char* names[] = {"Top", "Center", "Bottom"};
  st->contentVAlignBtn->SetText(std::string("Content V: ") + names[st->contentVIdx]);
}

static void CycleLineVAlign(AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
  if(!pressed) return;
  State* st = static_cast<State*>(data);
  st->lineVIdx = (st->lineVIdx + 1) % 3;
  AUIVAlign align = static_cast<AUIVAlign>(st->lineVIdx + 1);
  st->list->SetLineTextVAlign(align);
  const char* names[] = {"Top", "Center", "Bottom"};
  st->lineVAlignBtn->SetText(std::string("Line V: ") + names[st->lineVIdx]);
}

// Spacing and font controls
static void OnLineSpacingUp(AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
  if(!pressed) return;
  State* st = static_cast<State*>(data);
  st->list->SetLineSpacing(st->list->GetLineSpacing() + 1);
}

static void OnLineSpacingDown(AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
  if(!pressed) return;
  State* st = static_cast<State*>(data);
  uint32_t cur = st->list->GetLineSpacing();
  if(cur > 0) st->list->SetLineSpacing(cur - 1);
}

static void OnFontSizeUp(AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
  if(!pressed) return;
  State* st = static_cast<State*>(data);
  st->list->SetFontSize(st->list->GetFontSize() + 1);
}

static void OnFontSizeDown(AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
  if(!pressed) return;
  State* st = static_cast<State*>(data);
  uint32_t cur = st->list->GetFontSize();
  if(cur > 6) st->list->SetFontSize(cur - 1);
}

// Scrollbar toggles
static void OnToggleVScroll(AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
  if(!pressed) return;
  State* st = static_cast<State*>(data);
  bool enabled = st->list->IsVerticalScrollbarEnabled();
  st->list->SetVerticalScrollbarEnabled(!enabled);
  st->toggleVScrollBtn->SetText(enabled ? "V-Scroll: ON" : "V-Scroll: OFF");
}

static void OnToggleHScroll(AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
  if(!pressed) return;
  State* st = static_cast<State*>(data);
  bool enabled = st->list->IsHorizontalScrollbarEnabled();
  st->list->SetHorizontalScrollbarEnabled(!enabled);
  st->toggleHScrollBtn->SetText(enabled ? "H-Scroll: ON" : "H-Scroll: OFF");
}

// Standard button callbacks
static void OnToggleMode(AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
  if(!pressed) return;
  State* st = static_cast<State*>(data);
  st->multiSelect = !st->multiSelect;
  st->list->SetMultiSelect(st->multiSelect);
  st->toggleModeBtn->SetText(st->multiSelect ? "Mode: Multi-Select" : "Mode: Single-Select");
}

static void OnAddMission(AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
  if(!pressed) return;
  State* st = static_cast<State*>(data);
  st->list->AddItem(RandomMission());
}

static void OnClearSelection(AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
  if(!pressed) return;
  State* st = static_cast<State*>(data);
  st->list->ClearSelection();
}

static void OnPrintSelected(AWindow*, AWidget*, void* data, int32_t, int32_t, bool pressed) {
  if(!pressed) return;
  State* st = static_cast<State*>(data);
  std::vector<size_t> selected = st->list->GetSelectedIndices();
  if(selected.empty()) {
    std::println("No missions selected.");
  } else {
    std::println("Selected missions:");
    for(size_t idx : selected) {
      std::println("  {}: {}", idx + 1, st->list->GetItem(idx));
    }
  }
}

int32_t main() {
  AUI* au = AUI::Create("Space Missions Explorer");
  if(!au) return 1;

  AWindow* w = au->MainWnd();
  w->SetBGColor(0xFF1a1a2e);
  w->EnableResize();
  w->Resize(950, 750);

  // List widget
  AList* list = AList::AttachTo(w);
  list->Move(20, 60);
  list->Resize(550, 550);
  list->SetBGColor(0xFF16213e);
  list->SetTextColor(0xFFe0e0e0);
  list->SetBorderThickness(2);
  list->SetBorderColor(0xFF0f3460);
  list->SetLineSpacing(2);
  list->SetMultiSelect(true);
  list->SetHAlignment(AUIHAlign::left);
  list->SetVAlignment(AUIVAlign::top);
  list->SetLineTextVAlign(AUIVAlign::center);
  // Enable both scrollbars initially
  list->SetVerticalScrollbarEnabled(true);
  list->SetHorizontalScrollbarEnabled(true);

  // Add 25 random missions
  for(int i = 0; i < 44; ++i) {
    list->AddItem(RandomMission());
  }

  State state;
  state.list = list;
  state.multiSelect = true;

  // UI layout
  const int32_t btnX = 590;
  int32_t btnY = 60;
  const int32_t btnW = 180;
  const int32_t btnH = 30;
  const int32_t spacing = 10;

  // Mode toggle
  AButton* toggleBtn = AButton::AttachTo(w, "Mode: Multi-Select");
  toggleBtn->Move(btnX, btnY); btnY += btnH + spacing;
  toggleBtn->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  toggleBtn->SetBGColor(0xFF0f3460);
  toggleBtn->SetTextColor(0xFFe0e0e0);
  state.toggleModeBtn = toggleBtn;

  // Add mission
  AButton* addBtn = AButton::AttachTo(w, "Add Random Mission");
  addBtn->Move(btnX, btnY); btnY += btnH + spacing;
  addBtn->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  addBtn->SetBGColor(0xFF0f3460);
  addBtn->SetTextColor(0xFFe0e0e0);
  state.addBtn = addBtn;

  // Clear selection
  AButton* clearBtn = AButton::AttachTo(w, "Clear Selection");
  clearBtn->Move(btnX, btnY); btnY += btnH + spacing;
  clearBtn->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  clearBtn->SetBGColor(0xFF0f3460);
  clearBtn->SetTextColor(0xFFe0e0e0);
  state.clearSelBtn = clearBtn;

  // Print selected
  AButton* printBtn = AButton::AttachTo(w, "Print Selected");
  printBtn->Move(btnX, btnY); btnY += btnH + spacing;
  printBtn->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  printBtn->SetBGColor(0xFF0f3460);
  printBtn->SetTextColor(0xFFe0e0e0);
  state.printSelBtn = printBtn;

  // Separator
  ALabel* sep1 = ALabel::AttachTo(w, "--- Alignments ---", btnX, btnY, static_cast<uint32_t>(btnW), 20U);
  sep1->SetBGColor(0xFF1a1a2e);
  sep1->SetTextColor(0xFFa0a0a0);
  sep1->SetFontSize(10);
  btnY += 25;

  // Line horizontal alignment
  AButton* lineHBtn = AButton::AttachTo(w, "Line H: Left");
  lineHBtn->Move(btnX, btnY); btnY += btnH + spacing;
  lineHBtn->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  lineHBtn->SetBGColor(0xFF0f3460);
  lineHBtn->SetTextColor(0xFFe0e0e0);
  state.lineHAlignBtn = lineHBtn;

  // Content vertical alignment
  AButton* contentVBtn = AButton::AttachTo(w, "Content V: Top");
  contentVBtn->Move(btnX, btnY); btnY += btnH + spacing;
  contentVBtn->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  contentVBtn->SetBGColor(0xFF0f3460);
  contentVBtn->SetTextColor(0xFFe0e0e0);
  state.contentVAlignBtn = contentVBtn;

  // Line vertical alignment
  AButton* lineVBtn = AButton::AttachTo(w, "Line V: Center");
  lineVBtn->Move(btnX, btnY); btnY += btnH + spacing;
  lineVBtn->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  lineVBtn->SetBGColor(0xFF0f3460);
  lineVBtn->SetTextColor(0xFFe0e0e0);
  state.lineVAlignBtn = lineVBtn;

  // Separator
  ALabel* sep2 = ALabel::AttachTo(w, "--- Appearance ---", btnX, btnY, static_cast<uint32_t>(btnW), 20U);
  sep2->SetBGColor(0xFF1a1a2e);
  sep2->SetTextColor(0xFFa0a0a0);
  sep2->SetFontSize(10);
  btnY += 25;

  // Line spacing controls
  AButton* spacingUp = AButton::AttachTo(w, "Spacing +");
  spacingUp->Move(btnX, btnY); btnY += btnH + spacing;
  spacingUp->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  spacingUp->SetBGColor(0xFF0f3460);
  spacingUp->SetTextColor(0xFFe0e0e0);
  state.lineSpacingUpBtn = spacingUp;

  AButton* spacingDown = AButton::AttachTo(w, "Spacing -");
  spacingDown->Move(btnX, btnY); btnY += btnH + spacing;
  spacingDown->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  spacingDown->SetBGColor(0xFF0f3460);
  spacingDown->SetTextColor(0xFFe0e0e0);
  state.lineSpacingDownBtn = spacingDown;

  // Font size controls
  AButton* fontSizeUp = AButton::AttachTo(w, "Font Size +");
  fontSizeUp->Move(btnX, btnY); btnY += btnH + spacing;
  fontSizeUp->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  fontSizeUp->SetBGColor(0xFF0f3460);
  fontSizeUp->SetTextColor(0xFFe0e0e0);
  state.fontSizeUpBtn = fontSizeUp;

  AButton* fontSizeDown = AButton::AttachTo(w, "Font Size -");
  fontSizeDown->Move(btnX, btnY); btnY += btnH + spacing;
  fontSizeDown->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  fontSizeDown->SetBGColor(0xFF0f3460);
  fontSizeDown->SetTextColor(0xFFe0e0e0);
  state.fontSizeDownBtn = fontSizeDown;

  // Separator
  ALabel* sep3 = ALabel::AttachTo(w, "--- Scrollbars ---", btnX, btnY, static_cast<uint32_t>(btnW), 20U);
  sep3->SetBGColor(0xFF1a1a2e);
  sep3->SetTextColor(0xFFa0a0a0);
  sep3->SetFontSize(10);
  btnY += 25;

  // Toggle vertical scrollbar
  AButton* toggleV = AButton::AttachTo(w, "V-Scroll: ON");
  toggleV->Move(btnX, btnY); btnY += btnH + spacing;
  toggleV->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  toggleV->SetBGColor(0xFF0f3460);
  toggleV->SetTextColor(0xFFe0e0e0);
  state.toggleVScrollBtn = toggleV;

  // Toggle horizontal scrollbar
  AButton* toggleH = AButton::AttachTo(w, "H-Scroll: ON");
  toggleH->Move(btnX, btnY); btnY += btnH + spacing;
  toggleH->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  toggleH->SetBGColor(0xFF0f3460);
  toggleH->SetTextColor(0xFFe0e0e0);
  state.toggleHScrollBtn = toggleH;

  // Set callbacks
  toggleBtn->SetClickCallback(OnToggleMode, &state);
  addBtn->SetClickCallback(OnAddMission, &state);
  clearBtn->SetClickCallback(OnClearSelection, &state);
  printBtn->SetClickCallback(OnPrintSelected, &state);
  lineHBtn->SetClickCallback(CycleLineHAlign, &state);
  contentVBtn->SetClickCallback(CycleContentVAlign, &state);
  lineVBtn->SetClickCallback(CycleLineVAlign, &state);
  spacingUp->SetClickCallback(OnLineSpacingUp, &state);
  spacingDown->SetClickCallback(OnLineSpacingDown, &state);
  fontSizeUp->SetClickCallback(OnFontSizeUp, &state);
  fontSizeDown->SetClickCallback(OnFontSizeDown, &state);
  toggleV->SetClickCallback(OnToggleVScroll, &state);
  toggleH->SetClickCallback(OnToggleHScroll, &state);
  // Info label
  ALabel* info = ALabel::AttachTo(
      w,
      "Click items to select. Use scrollbars or wheel. Buttons change alignments, spacing, font, and scrollbars.",
      20, 20, 550U, 25U);
  info->SetBGColor(0xFF1a1a2e);
  info->SetTextColor(0xFFa0a0a0);
  info->SetFontSize(10);

  au->ProcessMessages();
  delete au;
  return 0;
}





