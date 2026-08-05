#include "AUILib.h"

namespace aui {

  static AWidget* OnCBButton(AWidget* bv, void* d, int32_t, int32_t) {
    AButton* b = (AButton*)bv;
    AComboBox* c = (AComboBox*)d;
    D1("button {}", b->Pressed())
    if(!b->Pressed()) {
      D1("setting button pressed")
      b->Pressed(true);
      c->OpenDropDown();
    }
    else {
      D1("setting button unpressed")
      b->Pressed(false);
      c->CloseDropDown();
    }
    return bv;
  }

  AComboBox::AComboBox() :
      mInputBox(nullptr), mDropList(nullptr), mButtonBGColor(0xFFCCCCCC), mButtonBorderColor(0xFF888888), mButtonTextColor(
          0xFF000000), mButtonHovered(false), mSelectedIndex(-1), mEditable(true), mDropDownOpen(
          false) {
    mSizeX = 150;
    mSizeY = 28;
    mBGColor = 0xFFCCCCCC;
    LayoutDirty();
    ClipChildren(false);
    ClipChildrenHitbox(false);
    D2("AComboBox constructed");
  }

  void AComboBox::OnDraw(UNUSED uint32_t* buffer, UNUSED uint32_t bufferW, UNUSED uint32_t bufferH,
  UNUSED int32_t offsetX, UNUSED int32_t offsetY, UNUSED int32_t clipL, UNUSED int32_t clipT,
  UNUSED int32_t clipR, UNUSED int32_t clipB) const {
  }

  void AComboBox::CloseDropDown() {
    if(!mDropDownOpen)
      return;
    D2("CloseDropDown");
    if(!Wnd())
      return;
    if(Wnd()->ActiveDropdown() == this) {
      Wnd()->ActiveDropdown(nullptr);// clear the static tracker
    }
    mDropDownOpen = false;
    mButton->Pressed(false);
    HideList();
    Wnd()->RemoveModal(mDropList);
    Wnd()->RequestRedraw();
  }

  void AComboBox::FontSize(uint32_t size) {
    D2("SetFontSize: {}", size);
    AWidget::FontSize(size);
    mInputBox->FontSize(size);
    if(mDropList)
      mDropList->FontSize(size);
    LayoutDirty();
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AComboBox::HideList() {
    if(!mDropList)
      return;
    mDropList->Hide();
  }

  void AComboBox::OnFocusGained() {
    D1("Focus gained");
    mInputBox->OnFocusGained();
  }

  void AComboBox::OnFocusLost() {
    D1("Focus lost");
    mInputBox->OnFocusLost();
    CloseDropDown();
  }

  void AComboBox::OnKeyEvent(const AUIKeyEvent& event) {
    if(!mEnabled || !event.pressed)
      return;
    if(!mDropDownOpen)
      return;
    switch(event.code) {
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

  void AComboBox::PopulateList() {
    D1("PopulateList: {} items", mItems.size());
    if(!mDropList)
      return;
    mDropList->Clear();
    for(const auto& item : mItems) {
      mDropList->AddItem(item);
      D3("  added: {}", item);
    }
    if(mSelectedIndex >= 0 && mSelectedIndex < static_cast<int32_t>(mItems.size())) {
      mDropList->SelectIndex(static_cast<size_t>(mSelectedIndex), true);
      D2("Selected index {} in list", mSelectedIndex);
    }
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
      if(Wnd())
        Wnd()->RequestRedraw();
    }
  }

  std::string AComboBox::GetSelectedText() const {
    if(mSelectedIndex >= 0 && mSelectedIndex < static_cast<int32_t>(mItems.size()))
      return mItems[static_cast<size_t>(mSelectedIndex)];
    return "";
  }

  void AComboBox::SyncInputToSelection() {
    if(mSelectedIndex >= 0 && mSelectedIndex < static_cast<int32_t>(mItems.size())) {
      mInputBox->Text(mItems[static_cast<size_t>(mSelectedIndex)]);
      D2("SyncInputToSelection: '{}'", mItems[static_cast<size_t>(mSelectedIndex)]);
    }
    else {
      mInputBox->Text("");
      D2("SyncInputToSelection: cleared");
    }
  }

  void AComboBox::OnInputChanged(const std::string& text) {
    mSelectedIndex = FindItem(text);
    D3("OnInputChanged: '{}' -> index {}", text, mSelectedIndex);
  }

  int32_t AComboBox::FindItem(const std::string& text) const {
    for(size_t i = 0; i < mItems.size(); ++i)
      if(mItems[i] == text)
        return static_cast<int32_t>(i);
    return -1;
  }

  AComboBox::~AComboBox() {
    if(Wnd()->ActiveDropdown() == this) {
      Wnd()->ActiveDropdown(nullptr);
    }
  }

  void AComboBox::OnListSelectionChanged(int32_t index) {
    if(index >= 0 && index < static_cast<int32_t>(mItems.size())) {
      if(index != mSelectedIndex) {
        mSelectedIndex = index;
        SyncInputToSelection();
        CloseDropDown();
        if(Wnd())
          Wnd()->RequestRedraw();
      }
    }
  }

  void AComboBox::Init() {
    AWidget::Init();
    mDropList = AList::AttachTo(this);
    mDropList->FontSize(mFontSize);
    mDropList->BGColor(0xFFFFFFFF);
    mDropList->Border(1);
    mDropList->Text("combobox list");
    mDropList->ScrollbarsEnabled(false);
    mDropList->VAlign(AUIVAlign::top);
    mDropList->HAlign(AUIHAlign::left);
    mDropList->Hide();
    mDropList->SetOnSelectionChanged([](AWidget* widget, void* userData) {
      AComboBox* combo = static_cast<AComboBox*>(userData);
      AList* listWidget = static_cast<AList*>(widget);
      auto sel = listWidget->SelectedIndices();
      if(!sel.empty()) {
        combo->OnListSelectionChanged(static_cast<int32_t>(sel[0]));
      }
    }, this);
    mInputBox = AInputBox::AttachTo(this);
    mInputBox->Border(1);
    mInputBox->BGColor(0xFFFFFFFF);
    mInputBox->TextColor(0xFF000000);
    mInputBox->HAlign(AUIHAlign::left);
    mInputBox->VAlign(AUIVAlign::center);
    mInputBox->Focusable(true);
    mInputBox->Editable(mEditable);
    mInputBox->Text("");
    mInputBox->SetOnChangeCallback([this](AInputBox*, const std::string& text) {
      OnInputChanged(text);
    });
    mButton = AButton::AttachTo(this);
    mButton->Text("v");
    mButton->HAlign(AUIHAlign::center);
    mButton->VAlign(AUIVAlign::center);
    mButton->SetMouseClickCallback(OnCBButton, this);

    LayoutDirty();
    LayoutUpdate();
  }

  void AComboBox::SetEditable(bool editable) {
    D2("SetEditable: {}", editable);
    mEditable = editable;
    mInputBox->Editable(editable);
  }

  void AComboBox::AddItem(const std::string& text) {
    D2("AddItem: {}", text);
    mItems.push_back(text);
    if(mDropDownOpen)
      PopulateList();
  }

  void AComboBox::InsertItem(size_t index, const std::string& text) {
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

  void AComboBox::InputBoxBGColor(uint32_t color) {
    mInputBox->BGColor(color);
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AComboBox::ButtonBGColor(uint32_t color) {
    mButtonBGColor = color;
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AComboBox::ListBGColor(uint32_t color) {
    if(mDropList)
      mDropList->BGColor(color);
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AComboBox::InputBoxTextColor(uint32_t color) {
    mInputBox->TextColor(color);
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AComboBox::ButtonTextColor(uint32_t color) {
    mButtonTextColor = color;
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AComboBox::ListTextColor(uint32_t color) {
    if(mDropList)
      mDropList->TextColor(color);
    if(Wnd())
      Wnd()->RequestRedraw();
  }

  void AComboBox::ShowList() {
    if(!mDropList) {
      D("Droplist not initialized")
      return;
    }
// Bring the list to the front so it draws above other widgets
    if(Wnd()) {
//      Wnd()->BringToFront(mDropList);
      Wnd()->PushModal(mDropList);
      D("brought list to front")
    }
    mDropList->Show();
    D("ends")
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

  void AComboBox::ClearSelection() {
    SetSelectedIndex(-1);
  }
  void AComboBox::ToggleDropDown() {
    if(mDropDownOpen)
      CloseDropDown();
    else
      OpenDropDown();
  }

  void AComboBox::OpenDropDown() {
    if(!Wnd()) return;
    D("")
    if(mDropDownOpen || mItems.empty()) {D()return;}
    if(Wnd()->ActiveDropdown() && Wnd()->ActiveDropdown() != this) {
      Wnd()->ActiveDropdown()->CloseDropDown();
    }
    D1("OpenDropDown");
    Wnd()->ActiveDropdown(this);// register this as the active one
    mDropDownOpen = true;
    LayoutDirty();
    LayoutUpdate();
    PopulateList();
    ShowList();
    //Wnd()->BringToFront(mDropList);
    BringToFront(mDropList);
    Wnd()->RequestRedraw();
    D1("OpenDropDown ends");
  }

  void AComboBox::UpdateListGeometry() {
    if(!mDropList)
      return;
    const uint32_t btnWidth = std::max(static_cast<uint32_t>((static_cast<uint64_t>(mSizeY) * 40) / 100), 18u);
    const uint32_t inputWidth = (mSizeX > btnWidth) ? (mSizeX - btnWidth) : 0;
    const int32_t numItems = static_cast<int32_t>(mItems.size());
    int32_t listHeight = static_cast<int32_t>(mSizeY);// fallback min
    if(numItems > 0) {
      const uint32_t contentHeight = static_cast<uint32_t>(numItems) * mDropList->LineHeight() + 4;// 4px border padding
      listHeight = static_cast<int32_t>(contentHeight);
    }
    uint32_t maxHeight = 200;// fallback (safe for 768p screens)
    listHeight = std::min(listHeight, static_cast<int32_t>(maxHeight));
    listHeight = std::max(listHeight, static_cast<int32_t>(mSizeY));
    int32_t listX = 0;
    int32_t listY = static_cast<int32_t>(mSizeY);
    D2("positioning list at {} {}", listX, listY)
    mDropList->Move(listX, listY);
    mDropList->Resize(inputWidth, static_cast<uint32_t>(listHeight));
    mButton->Resize(btnWidth + 1, mSizeY);
    mButton->Move(SafeINT32(inputWidth - 1), 0);
  }

  void AComboBox::OnResize(uint32_t newWidth, uint32_t newHeight) {
    D2("OnParentResize: {}x{}", newWidth, newHeight);
    AWidget::OnResize(newWidth, newHeight);
    LayoutDirty();
    LayoutUpdate();
  }

  bool AComboBox::OnMouseMove(int32_t localX, int32_t localY) {
    D3("OnMouseMove: ({},{})", localX, localY);
    const uint32_t btnWidth = 20;
    const uint32_t inputWidth = (mSizeX > btnWidth) ? (mSizeX - btnWidth) : 0;
    bool overButton = (localX >= static_cast<int32_t>(inputWidth) && localX < static_cast<int32_t>(mSizeX)
        && localY >= 0 && localY < static_cast<int32_t>(mSizeY));
    if(overButton != mButtonHovered) {
      mButtonHovered = overButton;
      if(Wnd()) Wnd()->BringToFront(mDropList);
      return true;
    }
    return ForwardMouseMoveToChildren(localX, localY);
  }

  void AComboBox::LayoutUpdate() {
    D3("UpdateChildrenLayout");
    if(!LayoutIsDirty())
      return;
    const uint32_t btnWidth = 20;
    const uint32_t inputWidth = (mSizeX > btnWidth) ? (mSizeX - btnWidth) : 0;
    mInputBox->Move(0, 0);
    mInputBox->Resize(inputWidth, mSizeY);
    UpdateListGeometry();
  }

}// namespace aui
