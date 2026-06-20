#include "AUILib.h"

namespace aui {

  AComboBox *AComboBox::s_activeDropDown = nullptr;

  AComboBox::AComboBox() :
      mInputBox(nullptr), mDropList(nullptr), mParentWindow(nullptr), mButtonBGColor(0xFFCCCCCC), mButtonBorderColor(
          0xFF888888), mButtonTextColor(0xFF000000), mButtonPressed(false), mButtonHovered(false), mSelectedIndex(-1), mEditable(
          true), mDropDownOpen(false), mNeedsLayoutUpdate(true) {
    D2("AComboBox constructed");
    mSizeX = 150;
    mSizeY = 28;
    mBGColor = 0xFFCCCCCC;
    auto inputBox = std::make_unique<AInputBox>();
    mInputBox = inputBox.get();
    AddWidget(std::move(inputBox));
    mInputBox->SetBorderThickness(1);
    mInputBox->SetBGColor(0xFFFFFFFF);
    mInputBox->SetTextColor(0xFF000000);
    mInputBox->SetHAlignment(AUIHAlign::left);
    mInputBox->SetVAlignment(AUIVAlign::center);
    mInputBox->SetFocusable(true);
    mInputBox->SetEditable(mEditable);

    mInputBox->SetOnChangeCallback([this](AInputBox*, const std::string &text) {
      OnInputChanged(text);
    });
  }

  AComboBox* AComboBox::AttachTo(AWindow *parent) {
    if(!parent) {
      E("AComboBox::AttachTo: parent window is null");
      return nullptr;
    }
    D1("Attaching AComboBox to window");
    auto *box = new AComboBox();
    parent->AddWidget(std::unique_ptr<AWidget>(box));
    box->mParentWindow = parent;
    box->PropagateParentSettings();
    auto list = std::make_unique<AList>();
    box->mDropList = list.get();
    parent->AddWidget(std::move(list));
    box->mDropList->SetBGColor(0xFFFFFFFF);
    box->mDropList->SetBorderThickness(1);
    box->mDropList->SetScrollbarsEnabled(true);
    box->mDropList->SetVAlignment(AUIVAlign::top);
    box->mDropList->SetHAlignment(AUIHAlign::left);
    box->mDropList->Hide();
    box->mDropList->mEnginePtr = box->mEnginePtr;
    box->mDropList->mParentWindow = parent;
    box->mDropList->SetOnSelectionChanged([](AWindow*/*window*/, AWidget *widget, void *userData) {
      AComboBox *combo = static_cast<AComboBox*>(userData);
      AList *listWidget = static_cast<AList*>(widget);
      auto sel = listWidget->GetSelectedIndices();
      if(!sel.empty()) {
        combo->OnListSelectionChanged(static_cast<int32_t>(sel[0]));
      }
    }, box);

    box->UpdateChildrenLayout();
    return box;
  }

  AComboBox* AComboBox::AttachTo(AWidget *parent) {
    if(!parent) {
      E("AComboBox::AttachTo: parent widget is null");
      return nullptr;
    }
    D1("Attaching AComboBox to widget");
    E("AComboBox::AttachTo(AWidget*) not fully supported; use AWindow* instead");
    auto *box = new AComboBox();
    parent->AddWidget(std::unique_ptr<AWidget>(box));
    box->PropagateParentSettings();
    box->mParentWindow = nullptr;
    auto list = std::make_unique<AList>();
    box->mDropList = list.get();
    parent->AddWidget(std::move(list));
    box->mDropList->SetBGColor(0xFFFFFFFF);
    box->mDropList->SetBorderThickness(1);
    box->mDropList->SetScrollbarsEnabled(true);
    box->mDropList->SetVAlignment(AUIVAlign::top);
    box->mDropList->SetHAlignment(AUIHAlign::left);
    box->mDropList->Hide();
    box->mDropList->mEnginePtr = box->mEnginePtr;
    box->mDropList->mParentWindow = box->mParentWindow;
    box->UpdateChildrenLayout();
    return box;
  }

  void AComboBox::PropagateParentSettings() {
    D2("PropagateParentSettings");
    for(auto &child : mWidg) {
      child->mEnginePtr = mEnginePtr;
      child->mParentWindow = mParentWindow;
    }
    if(mDropList) {
      mDropList->mEnginePtr = mEnginePtr;
      mDropList->mParentWindow = mParentWindow;
      mDropList->SetFontSize(mFontSize);
    }
    mNeedsLayoutUpdate = true;
    UpdateChildrenLayout();
  }

  void AComboBox::UpdateChildrenLayout() {
    D3("UpdateChildrenLayout");
    if(!mNeedsLayoutUpdate)
      return;
    const uint32_t btnWidth = 20;
    const uint32_t inputWidth = (mSizeX > btnWidth) ? (mSizeX - btnWidth) : 0;
    mInputBox->Move(0, 0);
    mInputBox->Resize(inputWidth, mSizeY);
    UpdateListGeometry();
    mNeedsLayoutUpdate = false;
  }

  void AComboBox::UpdateListGeometry() {
    if(!mDropList)
      return;
    const uint32_t btnWidth = std::max(static_cast<uint32_t>((static_cast<uint64_t>(mSizeY) * 40) / 100), 18u);
    const uint32_t inputWidth = (mSizeX > btnWidth) ? (mSizeX - btnWidth) : 0;
    const int32_t numItems = static_cast<int32_t>(mItems.size());
    int32_t listHeight = static_cast<int32_t>(mSizeY);// fallback min
    if(numItems > 0) {
      const uint32_t contentHeight = static_cast<uint32_t>(numItems) * mDropList->GetLineHeight() + 4;// 4px border padding
      listHeight = static_cast<int32_t>(contentHeight);
    }
    uint32_t maxHeight = 200;// fallback (safe for 768p screens)
    if(mParentWindow) {
// Replace with the actual getter if available, e.g.:
// maxHeight = mParentWindow->GetClientHeight() / 2;
// maxHeight = mParentWindow->GetSizeY() / 2;
// For now we keep the fallback to avoid adding dependencies.
    }
    listHeight = std::min(listHeight, static_cast<int32_t>(maxHeight));
    listHeight = std::max(listHeight, static_cast<int32_t>(mSizeY));
    mDropList->Move(mX, mY + static_cast<int32_t>(mSizeY));
    mDropList->Resize(inputWidth, static_cast<uint32_t>(listHeight));
  }

  void AComboBox::OnParentResize(uint32_t newWidth, uint32_t newHeight) {
    D2("OnParentResize: {}x{}", newWidth, newHeight);
    AWidget::OnParentResize(newWidth, newHeight);
    mNeedsLayoutUpdate = true;
  }

  void AComboBox::ShowList() {
    if(!mDropList)
      return;
// Bring the list to the front so it draws above other widgets
    if(mParentWindow) {
      mParentWindow->BringChildToFront(mDropList);
    }
    mDropList->Show();
  }

  void AComboBox::HideList() {
    if(!mDropList)
      return;
    mDropList->Hide();
  }

// ------------------------------------------------------------------
// Items / selection
// ------------------------------------------------------------------
  void AComboBox::AddItem(const std::string &text) {
    D2("AddItem: {}", text);
    mItems.push_back(text);
    if(mDropDownOpen)
      PopulateList();
  }

  void AComboBox::InsertItem(size_t index, const std::string &text) {
    if(index > mItems.size())
      index = mItems.size();
    D2("InsertItem at {}: {}", index, text);
    mItems.insert(mItems.begin() + static_cast<ptrdiff_t>(index), text);
    if(mDropDownOpen)
      PopulateList();
  }

  void AComboBox::RemoveItem(size_t index) {
    if(index >= mItems.size())
      return;
    D2("RemoveItem at {}", index);
    mItems.erase(mItems.begin() + static_cast<ptrdiff_t>(index));
    if(mSelectedIndex >= static_cast<int32_t>(mItems.size()))
      mSelectedIndex = static_cast<int32_t>(mItems.size()) - 1;
    if(mDropDownOpen) {
      PopulateList();
      SyncInputToSelection();
    }
  }

  void AComboBox::ClearItems() {
    D2("ClearItems");
    mItems.clear();
    mSelectedIndex = -1;
    if(mDropDownOpen)
      PopulateList();
    SyncInputToSelection();
  }

  const std::string& AComboBox::GetItem(size_t index) const {
    static const std::string empty;
    if(index >= mItems.size())
      return empty;
    return mItems[index];
  }

  void AComboBox::SetItems(const std::vector<std::string> &items) {
    D2("SetItems: {} items", items.size());
    mItems = items;
    if(mSelectedIndex >= static_cast<int32_t>(mItems.size()))
      mSelectedIndex = -1;
    if(mDropDownOpen)
      PopulateList();
    SyncInputToSelection();
  }

  void AComboBox::PopulateList() {
    D2("PopulateList: {} items", mItems.size());
    if(!mDropList)
      return;
    mDropList->Clear();
    for(const auto &item : mItems) {
      mDropList->AddItem(item);
      D3("  added: {}", item);
    }
    if(mSelectedIndex >= 0 && mSelectedIndex < static_cast<int32_t>(mItems.size())) {
      mDropList->SelectIndex(static_cast<size_t>(mSelectedIndex), true);
      D2("Selected index {} in list", mSelectedIndex);
    }
  }

  void AComboBox::SyncInputToSelection() {
    if(mSelectedIndex >= 0 && mSelectedIndex < static_cast<int32_t>(mItems.size())) {
      mInputBox->SetText(mItems[static_cast<size_t>(mSelectedIndex)]);
      D2("SyncInputToSelection: '{}'", mItems[static_cast<size_t>(mSelectedIndex)]);
    }
    else {
      mInputBox->SetText("");
      D2("SyncInputToSelection: cleared");
    }
  }

  int32_t AComboBox::FindItem(const std::string &text) const {
    for(size_t i = 0; i < mItems.size(); ++i)
      if(mItems[i] == text)
        return static_cast<int32_t>(i);
    return -1;
  }

  void AComboBox::OnInputChanged(const std::string &text) {
    mSelectedIndex = FindItem(text);
    D3("OnInputChanged: '{}' -> index {}", text, mSelectedIndex);
  }

  void AComboBox::SetSelectedIndex(int32_t index) {
    if(index < -1 || index >= static_cast<int32_t>(mItems.size()))
      index = -1;
    if(mSelectedIndex != index) {
      D2("SetSelectedIndex: {} -> {}", mSelectedIndex, index);
      mSelectedIndex = index;
      SyncInputToSelection();
      if(mDropDownOpen)
        PopulateList();
      if(mParentWindow)
        mParentWindow->Draw();
    }
  }

  std::string AComboBox::GetSelectedText() const {
    if(mSelectedIndex >= 0 && mSelectedIndex < static_cast<int32_t>(mItems.size()))
      return mItems[static_cast<size_t>(mSelectedIndex)];
    return "";
  }

  void AComboBox::ClearSelection() {
    SetSelectedIndex(-1);
  }

  void AComboBox::OnListSelectionChanged(int32_t index) {
    if(index >= 0 && index < static_cast<int32_t>(mItems.size())) {
      if(index != mSelectedIndex) {
        mSelectedIndex = index;
        SyncInputToSelection();
        CloseDropDown();
        if(mParentWindow)
          mParentWindow->Draw();
      }
    }
  }

// ------------------------------------------------------------------
// Editable
// ------------------------------------------------------------------
  void AComboBox::SetEditable(bool editable) {
    D2("SetEditable: {}", editable);
    mEditable = editable;
    mInputBox->SetEditable(editable);
  }

// ------------------------------------------------------------------
// Dropdown open/close
// ------------------------------------------------------------------
  void AComboBox::OpenDropDown() {
    if(mDropDownOpen || mItems.empty())
      return;
// CLOSE ANY OTHER OPEN DROPDOWN FIRST
    if(s_activeDropDown && s_activeDropDown != this) {
      s_activeDropDown->CloseDropDown();
    }
    D2("OpenDropDown");
    s_activeDropDown = this;// register this as the active one
    mDropDownOpen = true;
    mNeedsLayoutUpdate = true;
    UpdateChildrenLayout();
    PopulateList();
    ShowList();// This will call BringChildToFront from your previous fix
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AComboBox::CloseDropDown() {
    if(!mDropDownOpen)
      return;
    D2("CloseDropDown");
    if(s_activeDropDown == this) {
      s_activeDropDown = nullptr;// clear the static tracker
    }
    mDropDownOpen = false;
    mButtonPressed = false;
    HideList();
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AComboBox::ToggleDropDown() {
    if(mDropDownOpen)
      CloseDropDown();
    else
      OpenDropDown();
  }

// ------------------------------------------------------------------
// Draw
// ------------------------------------------------------------------
  void AComboBox::Draw(uint32_t *buffer, uint32_t parentWidth, uint32_t parentHeight, int32_t offsetX,
      int32_t offsetY) const {
    D3("Draw: offset=({},{})", offsetX, offsetY);
    if(mNeedsLayoutUpdate) {
      const_cast<AComboBox*>(this)->UpdateChildrenLayout();
    }
    DrawChildren(buffer, parentWidth, parentHeight, offsetX, offsetY);
    const uint32_t btnWidth = 20;
    const uint32_t inputWidth = (mSizeX > btnWidth) ? (mSizeX - btnWidth) : 0;
    int32_t btnAbsX = offsetX + mX + static_cast<int32_t>(inputWidth);
    int32_t btnAbsY = offsetY + mY;
    int32_t btnW = static_cast<int32_t>(btnWidth);
    int32_t btnH = static_cast<int32_t>(mSizeY);
    uint32_t bg = mButtonBGColor;
    if(mButtonPressed) {
      bg = ShiftColor(mButtonBGColor, true);
    }
    else if(mButtonHovered) {
      bg = ShiftColor(mButtonBGColor, false);
    }
    if(!mEnabled) {
      bg = ShiftColor(bg, true);
    }
    FillRect(buffer, parentWidth, btnAbsX, btnAbsY, btnW, btnH, bg);
    if(mBorderThick > 0) {
      DrawRectBorder(buffer, parentWidth, btnAbsX, btnAbsY, btnW, btnH, mButtonBorderColor);
    }
    int32_t triSize = std::min(btnW, btnH) / 4;
    triSize = std::max(triSize, 3);
    int32_t cx = btnAbsX + btnW / 2;
    int32_t cy = btnAbsY + btnH / 2;
    uint32_t color = mEnabled ? mButtonTextColor : ShiftColor(mButtonTextColor, true);
    DrawThickLine(buffer, parentWidth, parentHeight, cx - triSize, cy - triSize / 2, cx, cy + triSize / 2, color, 2);
    DrawThickLine(buffer, parentWidth, parentHeight, cx, cy + triSize / 2, cx + triSize, cy - triSize / 2, color, 2);
  }

// ------------------------------------------------------------------
// Mouse events
// ------------------------------------------------------------------
  bool AComboBox::OnMouseClick(int32_t localX, int32_t localY, bool pressed) {
    if(!mEnabled)
      return false;
    D2("AComboBox::OnMouseClick: local=({},{}) pressed={}", localX, localY, pressed);
    const uint32_t btnWidth = 20;
    const uint32_t inputWidth = (mSizeX > btnWidth) ? (mSizeX - btnWidth) : 0;
    if(!pressed) {
// Do NOT clear mButtonPressed here – it stays pressed until dropdown closes.
// If release is over input area and dropdown not open, open it
      if(localX >= 0 && localX < static_cast<int32_t>(inputWidth) && localY >= 0
          && localY < static_cast<int32_t>(mSizeY)) {
        if(!mDropDownOpen) {
          OpenDropDown();
          return true;
        }
      }
      return false;
    }
// Drop button area
    if(localX >= static_cast<int32_t>(inputWidth) && localX < static_cast<int32_t>(mSizeX) && localY >= 0
        && localY < static_cast<int32_t>(mSizeY)) {
      D2("Button area clicked – toggling");
      mButtonPressed = true;
      ToggleDropDown();
      if(mParentWindow)
        mParentWindow->Draw();
      return true;
    }
// Input area – give focus, but open on release
    if(localX >= 0 && localX < static_cast<int32_t>(inputWidth) && localY >= 0
        && localY < static_cast<int32_t>(mSizeY)) {
      D2("Input area pressed");
      mInputBox->OnMouseClick(localX, localY, true);
      return true;
    }
    if(mDropDownOpen) {
      CloseDropDown();
      return true;
    }
    return false;
  }

  void AComboBox::OnMouseMove(int32_t localX, int32_t localY) {
    D3("OnMouseMove: ({},{})", localX, localY);
    const uint32_t btnWidth = 20;
    const uint32_t inputWidth = (mSizeX > btnWidth) ? (mSizeX - btnWidth) : 0;
    bool overButton = (localX >= static_cast<int32_t>(inputWidth) && localX < static_cast<int32_t>(mSizeX)
        && localY >= 0 && localY < static_cast<int32_t>(mSizeY));
    if(overButton != mButtonHovered) {
      mButtonHovered = overButton;
      if(mParentWindow)
        mParentWindow->Draw();
    }

    ForwardMoveToChildren(localX, localY);
  }

// ------------------------------------------------------------------
// Keyboard
// ------------------------------------------------------------------
  void AComboBox::OnKeyEvent(const AUIKeyEvent &event) {
    if(!mEnabled || !event.pressed)
      return;
    if(!mDropDownOpen)
      return;
    switch (event.code) {
      case AUIKeyCode::Up:
        if(mSelectedIndex > 0) {
          SetSelectedIndex(mSelectedIndex - 1);
          if(mDropDownOpen)
            PopulateList();
        }
        break;
      case AUIKeyCode::Down:
        if(mSelectedIndex < static_cast<int32_t>(mItems.size()) - 1) {
          SetSelectedIndex(mSelectedIndex + 1);
          if(mDropDownOpen)
            PopulateList();
        }
        break;
      case AUIKeyCode::Enter:
        if(mSelectedIndex >= 0) {
          SyncInputToSelection();
          CloseDropDown();
        }
        break;
      case AUIKeyCode::Escape:
        CloseDropDown();
        break;
      default:
        break;
    }
  }

  void AComboBox::OnFocusGained() {
    D2("Focus gained");
    mInputBox->OnFocusGained();
  }

  void AComboBox::OnFocusLost() {
    D2("Focus lost");
    mInputBox->OnFocusLost();
    CloseDropDown();
  }

// ------------------------------------------------------------------
// Enable / Disable
// ------------------------------------------------------------------
  void AComboBox::Enable() {
    D2("Enable");
    AWidget::Enable();
    mInputBox->Enable();
    if(mDropList)
      mDropList->Enable();
  }

  void AComboBox::Disable() {
    D2("Disable");
    AWidget::Disable();
    mInputBox->Disable();
    if(mDropList)
      mDropList->Disable();
    CloseDropDown();
  }

// ------------------------------------------------------------------
// Styling forwarders
// ------------------------------------------------------------------
  void AComboBox::SetInputBoxBGColor(uint32_t color) {
    mInputBox->SetBGColor(color);
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AComboBox::SetButtonBGColor(uint32_t color) {
    mButtonBGColor = color;
    if(mParentWindow)
      mParentWindow->Draw();
  }
  void AComboBox::SetListBGColor(uint32_t color) {
    if(mDropList)
      mDropList->SetBGColor(color);
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AComboBox::SetInputBoxTextColor(uint32_t color) {
    mInputBox->SetTextColor(color);
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AComboBox::SetButtonTextColor(uint32_t color) {
    mButtonTextColor = color;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AComboBox::SetListTextColor(uint32_t color) {
    if(mDropList)
      mDropList->SetTextColor(color);
    if(mParentWindow)
      mParentWindow->Draw();
  }

  void AComboBox::SetFontSize(uint32_t size) {
    D2("SetFontSize: {}", size);
    AWidget::SetFontSize(size);
    mInputBox->SetFontSize(size);
    if(mDropList)
      mDropList->SetFontSize(size);
    mNeedsLayoutUpdate = true;
    if(mParentWindow)
      mParentWindow->Draw();
  }

  AComboBox::~AComboBox() {
    if(s_activeDropDown == this) {
      s_activeDropDown = nullptr;
    }
  }

}// namespace aui
