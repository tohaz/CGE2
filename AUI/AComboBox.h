#ifndef ACOMBOBOX_H_
#define ACOMBOBOX_H_

namespace aui {

class AComboBox : public AWidgetFactory<AComboBox> {
    friend class AWidgetFactory<AComboBox>;
  private:
    void OnDraw(uint32_t* buffer, uint32_t bufferW, uint32_t bufferH,
                int32_t offsetX, int32_t offsetY,
                int32_t clipL, int32_t clipT, int32_t clipR, int32_t clipB) const override;
    AInputBox* mInputBox = nullptr;
    AList*     mDropList = nullptr;          // sibling, owned by parent window
    AButton*   mButton = nullptr;          // sibling, owned by parent window
    uint32_t mButtonBGColor = 0;
    uint32_t mButtonBorderColor = 0;
    uint32_t mButtonTextColor = 0;
    bool mButtonHovered = false;
    std::vector<std::string> mItems;
    int32_t mSelectedIndex = 0;
    bool mEditable = false;
    bool mDropDownOpen = false;
//    bool mNeedsLayoutUpdate;
    void PropagateParentSettings();
//    void UpdateChildrenLayout();
    void PopulateList();
    void SyncInputToSelection();
    int32_t FindItem(const std::string& text) const;
    void OnInputChanged(const std::string& text);
    void UpdateListGeometry();
    void ShowList();
    void HideList();
    void OnListSelectionChanged(int32_t index);
    //static AComboBox* s_activeDropDown; // track the one currently open
public:
  AComboBox();
  virtual ~AComboBox();
  void AddItem(const std::string& text);
  void InsertItem(size_t index, const std::string& text);
  void RemoveItem(size_t index);
  void ClearItems();
  size_t GetItemCount() const { return mItems.size(); }
  const std::string& GetItem(size_t index) const;
  void SetItems(const std::vector<std::string>& items);
  int32_t GetSelectedIndex() const { return mSelectedIndex; }
  void SetSelectedIndex(int32_t index);
  std::string GetSelectedText() const;
  void ClearSelection();
  void SetEditable(bool editable);
  bool IsEditable() const { return mEditable; }
  bool IsDropDownOpen() const { return mDropDownOpen; }
  void OpenDropDown();
  void CloseDropDown();
  void ToggleDropDown();
//  virtual void Draw(uint32_t* buffer, uint32_t parentWidth, uint32_t parentHeight,
//                    int32_t offsetX, int32_t offsetY) const override;
//  virtual bool OnMouseClick(int32_t localX, int32_t localY, bool pressed) override;
  virtual bool OnMouseMove(int32_t localX, int32_t localY) override;
  virtual void OnKeyEvent(const AUIKeyEvent& event) override;
  virtual void OnFocusGained() override;
  virtual void OnFocusLost() override;
  virtual void Enable() override;
  virtual void Disable() override;
  virtual void OnResize(uint32_t newWidth, uint32_t newHeight) override;
  void InputBoxBGColor(uint32_t color);
  void ButtonBGColor(uint32_t color);
  void ListBGColor(uint32_t color);
  void InputBoxTextColor(uint32_t color);
  void ButtonTextColor(uint32_t color);
  void ListTextColor(uint32_t color);
  //virtual void FontSize(uint32_t size) override;
  virtual void FontSize(uint32_t size) override;
  AInputBox* GetInputBox() const { return mInputBox; }
  void LayoutUpdate();
  void Init();
  AList* DropList() {return mDropList;}
  std::string Text() const {D4() if(mInputBox){return mInputBox->Text();} else return "";}
  void Text(std::string v) {if(mInputBox) mInputBox->Text(v);}
};

} // namespace aui

#endif // ACOMBOBOX_H_
