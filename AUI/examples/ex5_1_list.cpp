#include "AUILib.h"

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
UNUSED static AWidget* CycleLineHAlign(AWidget* wi, void* data, int32_t, int32_t) {
  State* st = static_cast<State*>(data);
  // Advance state
  st->lineHIdx = (st->lineHIdx + 1) % 3;
// Map index directly to your UI alignment enum (0 = Left, 1 = Center, 2 = Right)
  // Adjust this cast if AUIHAlign uses 0-based indexing:
  AUIHAlign alignMap[] = { AUIHAlign::left, AUIHAlign::center, AUIHAlign::right };
  st->list->HAlign(alignMap[st->lineHIdx]);
  const char* names[] = {"Left", "Center", "Right"};
  st->lineHAlignBtn->Text(std::string("Line H: ") + names[st->lineHIdx]);
  return wi;
}

UNUSED static AWidget* CycleContentVAlign(AWidget*wi, void* data, int32_t, int32_t) {
  State* st = static_cast<State*>(data);
  st->contentVIdx = (st->contentVIdx + 1) % 3;
  AUIVAlign alignMap[] = { AUIVAlign::top, AUIVAlign::center, AUIVAlign::bottom };
  st->list->VAlign(alignMap[st->contentVIdx]);
    const char* names[] = {"Top", "Center", "Bottom"};
  st->contentVAlignBtn->Text(std::string("Content V: ") + names[st->contentVIdx]);
  return wi;
}

UNUSED static AWidget* CycleLineVAlign(AWidget* wi, void* data, int32_t, int32_t) {
  State* st = static_cast<State*>(data);
  st->lineVIdx = (st->lineVIdx + 1) % 3;
  AUIVAlign alignMap[] = { AUIVAlign::top, AUIVAlign::center, AUIVAlign::bottom };
  st->list->LineTextVAlign(alignMap[st->lineVIdx]);
    const char* names[] = {"Top", "Center", "Bottom"};
  st->lineVAlignBtn->Text(std::string("Line V: ") + names[st->lineVIdx]);
  return wi;
}

// Spacing and font controls
UNUSED static AWidget* OnLineSpacingUp(AWidget*wi, void* data, int32_t, int32_t) {
  State* st = static_cast<State*>(data);
  st->list->LineSpacing(st->list->LineSpacing() + 1);
  return wi;
}

UNUSED static AWidget* OnLineSpacingDown(AWidget* wi, void* data, int32_t, int32_t) {
  State* st = static_cast<State*>(data);
  uint32_t cur = st->list->LineSpacing();
  if(cur > 0) st->list->LineSpacing(cur - 1);
  return wi;
}

UNUSED static AWidget* OnFontSizeUp(AWidget*wi, void* data, int32_t, int32_t) {
  State* st = static_cast<State*>(data);
  st->list->FontSize(st->list->AWidget::FontSize() + 1);
  return wi;
}

UNUSED static AWidget* OnFontSizeDown(AWidget* wi, void* data, int32_t, int32_t) {
  State* st = static_cast<State*>(data);
  uint32_t cur = st->list->AWidget::FontSize();
  if(cur > 6) st->list->FontSize(cur - 1);
  return wi;
}

// Scrollbar toggles
UNUSED static AWidget* OnToggleVScroll(AWidget*wi, void* data, int32_t, int32_t) {
  State* st = static_cast<State*>(data);
  bool enabled = st->list->VScrollbarEnabled();
  st->list->VScrollbarToggle(!enabled);
  st->toggleVScrollBtn->Text(enabled ? "V-Scroll: ON" : "V-Scroll: OFF");
  return wi;
}

UNUSED static AWidget* OnToggleHScroll(AWidget* wi, void* data, int32_t, int32_t) {
  State* st = static_cast<State*>(data);
  bool enabled = st->list->HScrollbarEnabled();
  st->list->HScrollbarToggle(!enabled);
  st->toggleHScrollBtn->Text(enabled ? "H-Scroll: ON" : "H-Scroll: OFF");
  return wi;
}

// Standard button callbacks
UNUSED static AWidget* OnToggleMode(AWidget*wi, void* data, int32_t, int32_t) {
  State* st = static_cast<State*>(data);
  st->multiSelect = !st->multiSelect;
  st->list->MultiSelect(st->multiSelect);
  st->toggleModeBtn->Text(st->multiSelect ? "Mode: Multi-Select" : "Mode: Single-Select");
  return wi;
}

UNUSED static AWidget* OnAddMission(AWidget*wi, void* data, int32_t, int32_t) {
  State* st = static_cast<State*>(data);
  st->list->AddItem(RandomMission());
  return wi;
}

UNUSED static AWidget* OnClearSelection(AWidget* wi, void* data, int32_t, int32_t) {
  State* st = static_cast<State*>(data);
  st->list->ClearSelection();
  return wi;
}

UNUSED static AWidget* OnPrintSelected(AWidget* wi, void* data, int32_t, int32_t) {
  State* st = static_cast<State*>(data);
  std::vector<size_t> selected = st->list->SelectedIndices();
  if(selected.empty()) {
    std::println("No missions selected.");
  } else {
    std::println("Selected missions:");
    for(size_t idx : selected) {
      std::println("  {}: {}", idx + 1, st->list->GetItem(idx));
    }
  }
  return wi;
}

int32_t main() {
  AUI* au = AUI::Create("Space Missions Explorer");
  if(!au) return 1;
  AWindow* w = au->MainWnd();
  w->BGColor(0xFF1a1a2e);
  w->EnableResize();
  w->Resize(950, 750);
  // List widget
  AList* list = AList::AttachTo(w);
  list->Move(20, 60);
  list->Resize(550, 550);
  list->BGColor(0xFF16213e);
  list->TextColor(0xFFe0e0e0);
  list->Border(2);
  list->BorderColor(0xFF0f3460);
  list->LineSpacing(2);
  list->MultiSelect(true);
  list->HAlign(AUIHAlign::left);
  list->VAlign(AUIVAlign::top);
  list->LineTextVAlign(AUIVAlign::center);
  // Enable both scrollbars initially
  list->VScrollbarToggle(true);
  list->HScrollbarToggle(true);
  // Add random missions
  for(int32_t i = 0; i < 22; ++i) {
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
//   Mode toggle
  AButton* toggleBtn = AButton::AttachTo(w, "Mode: Multi-Select");
  toggleBtn->Move(btnX, btnY); btnY += btnH + spacing;
  toggleBtn->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  toggleBtn->BGColor(0xFF0f3460);
  toggleBtn->TextColor(0xFFe0e0e0);
  toggleBtn->SetMouseClickCallback(OnToggleMode, &state);
  state.toggleModeBtn = toggleBtn;
  // Add mission
  AButton* addBtn = AButton::AttachTo(w, "Add Random Mission");
  addBtn->Move(btnX, btnY); btnY += btnH + spacing;
  addBtn->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  addBtn->BGColor(0xFF0f3460);
  addBtn->TextColor(0xFFe0e0e0);
  addBtn->SetMouseClickCallback(OnAddMission, &state);
  state.addBtn = addBtn;
//   Clear selection
  AButton* clearBtn = AButton::AttachTo(w, "Clear Selection");
  clearBtn->Move(btnX, btnY); btnY += btnH + spacing;
  clearBtn->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  clearBtn->BGColor(0xFF0f3460);
  clearBtn->TextColor(0xFFe0e0e0);
  clearBtn->SetMouseClickCallback(OnClearSelection, &state);
  state.clearSelBtn = clearBtn;
  // Print selected
  AButton* printBtn = AButton::AttachTo(w, "Print Selected");
  printBtn->Move(btnX, btnY); btnY += btnH + spacing;
  printBtn->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  printBtn->BGColor(0xFF0f3460);
  printBtn->TextColor(0xFFe0e0e0);
  printBtn->SetMouseClickCallback(OnPrintSelected, &state);
  state.printSelBtn = printBtn;
  // Separator
  UNUSED ALabel* sep1 = ALabel::AttachTo(w, "--- Alignments ---", btnX, btnY, static_cast<uint32_t>(btnW), 20U);
  sep1->BGColor(0xFF1a1a2e);
  sep1->TextColor(0xFFa0a0a0);
  sep1->FontSize(10);
  btnY += 25;
  // Line horizontal alignment
  AButton* lineHBtn = AButton::AttachTo(w, "Line H: Left");
  lineHBtn->Move(btnX, btnY); btnY += btnH + spacing;
  lineHBtn->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  lineHBtn->BGColor(0xFF0f3460);
  lineHBtn->TextColor(0xFFe0e0e0);
  lineHBtn->SetMouseClickCallback(CycleLineHAlign, &state);
  state.lineHAlignBtn = lineHBtn;
//   Content vertical alignment
  AButton* contentVBtn = AButton::AttachTo(w, "Content V: Top");
  contentVBtn->Move(btnX, btnY); btnY += btnH + spacing;
  contentVBtn->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  contentVBtn->BGColor(0xFF0f3460);
  contentVBtn->TextColor(0xFFe0e0e0);
  contentVBtn->SetMouseClickCallback(CycleContentVAlign, &state);
  state.contentVAlignBtn = contentVBtn;
  // Line vertical alignment
  AButton* lineVBtn = AButton::AttachTo(w, "Line V: Center");
  lineVBtn->Move(btnX, btnY); btnY += btnH + spacing;
  lineVBtn->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  lineVBtn->BGColor(0xFF0f3460);
  lineVBtn->TextColor(0xFFe0e0e0);
  lineVBtn->SetMouseClickCallback(CycleLineVAlign, &state);
  state.lineVAlignBtn = lineVBtn;
  // Separator
  ALabel* sep2 = ALabel::AttachTo(w, "--- Appearance ---", btnX, btnY, static_cast<uint32_t>(btnW), 20U);
  sep2->BGColor(0xFF1a1a2e);
  sep2->TextColor(0xFFa0a0a0);
  sep2->FontSize(10);
  btnY += 25;
  // Line spacing controls
  AButton* spacingUp = AButton::AttachTo(w, "Spacing +");
  spacingUp->Move(btnX, btnY); btnY += btnH + spacing;
  spacingUp->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  spacingUp->BGColor(0xFF0f3460);
  spacingUp->TextColor(0xFFe0e0e0);
  spacingUp->SetMouseClickCallback(OnLineSpacingUp, &state);
  state.lineSpacingUpBtn = spacingUp;
  AButton* spacingDown = AButton::AttachTo(w, "Spacing -");
  spacingDown->Move(btnX, btnY); btnY += btnH + spacing;
  spacingDown->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  spacingDown->BGColor(0xFF0f3460);
  spacingDown->TextColor(0xFFe0e0e0);
  spacingDown->SetMouseClickCallback(OnLineSpacingDown, &state);
  state.lineSpacingDownBtn = spacingDown;
  // Font size controls
  AButton* fontSizeUp = AButton::AttachTo(w, "Font Size +");
  fontSizeUp->Move(btnX, btnY); btnY += btnH + spacing;
  fontSizeUp->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  fontSizeUp->BGColor(0xFF0f3460);
  fontSizeUp->TextColor(0xFFe0e0e0);
  fontSizeUp->SetMouseClickCallback(OnFontSizeUp, &state);
  state.fontSizeUpBtn = fontSizeUp;
  AButton* fontSizeDown = AButton::AttachTo(w, "Font Size -");
  fontSizeDown->Move(btnX, btnY); btnY += btnH + spacing;
  fontSizeDown->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  fontSizeDown->BGColor(0xFF0f3460);
  fontSizeDown->TextColor(0xFFe0e0e0);
  fontSizeDown->SetMouseClickCallback(OnFontSizeDown, &state);
  state.fontSizeDownBtn = fontSizeDown;

//   Separator
  ALabel* sep3 = ALabel::AttachTo(w, "--- Scrollbars ---", btnX, btnY, static_cast<uint32_t>(btnW), 20U);
  sep3->BGColor(0xFF1a1a2e);
  sep3->TextColor(0xFFa0a0a0);
  sep3->FontSize(10);
  btnY += 25;
  // Toggle vertical scrollbar
  AButton* toggleV = AButton::AttachTo(w, "V-Scroll: ON");
  toggleV->Move(btnX, btnY); btnY += btnH + spacing;
  toggleV->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  toggleV->BGColor(0xFF0f3460);
  toggleV->TextColor(0xFFe0e0e0);
  state.toggleVScrollBtn = toggleV;
  toggleV->SetMouseClickCallback(OnToggleVScroll, &state);
  // Toggle horizontal scrollbar
  AButton* toggleH = AButton::AttachTo(w, "H-Scroll: ON");
  toggleH->Move(btnX, btnY); btnY += btnH + spacing;
  toggleH->Resize(static_cast<uint32_t>(btnW), static_cast<uint32_t>(btnH));
  toggleH->BGColor(0xFF0f3460);
  toggleH->TextColor(0xFFe0e0e0);
  toggleH->SetMouseClickCallback(OnToggleHScroll, &state);
  state.toggleHScrollBtn = toggleH;
  ALabel* info = ALabel::AttachTo(
      w,
      "Click items to select. Use scrollbars or wheel. Buttons change alignments, spacing, font, and scrollbars.",
      20, 20, 550U, 25U);
  info->BGColor(0xFF1a1a2e);
  info->TextColor(0xFFa0a0a0);
  info->FontSize(10);
  au->ProcessMessages();
  delete au;
  return 0;
}

