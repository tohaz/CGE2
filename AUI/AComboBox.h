#ifndef ACOMBOBOX_H_
#define ACOMBOBOX_H_

#include "AUILib.h"

namespace aui {

class AComboBox : public AWidget {
  private:
    AInputBox* mInputBox;
    AList*     mDropList;          // sibling, owned by parent window
    AWindow* mParentWindow;
    // Button properties (drawn manually)
    uint32_t mButtonBGColor;
    uint32_t mButtonBorderColor;
    uint32_t mButtonTextColor;
    bool mButtonPressed;
    bool mButtonHovered;
    std::vector<std::string> mItems;
    int32_t mSelectedIndex;
    bool mEditable;
    bool mDropDownOpen;
    bool mNeedsLayoutUpdate;
    void PropagateParentSettings();
    void UpdateChildrenLayout();
    void PopulateList();
    void SyncInputToSelection();
    int32_t FindItem(const std::string& text) const;
    void OnInputChanged(const std::string& text);
    void UpdateListGeometry();
    void ShowList();
    void HideList();
    void OnListSelectionChanged(int32_t index);
    static AComboBox* s_activeDropDown; // track the one currently open
public:
  AComboBox();
  virtual ~AComboBox();
  static AComboBox* AttachTo(AWindow* parent);
  static AComboBox* AttachTo(AWidget* parent);
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
  virtual void Draw(uint32_t* buffer, uint32_t parentWidth, uint32_t parentHeight,
                    int32_t offsetX, int32_t offsetY) const override;
  virtual bool OnMouseClick(int32_t localX, int32_t localY, bool pressed) override;
  virtual void OnMouseMove(int32_t localX, int32_t localY) override;
  virtual void OnKeyEvent(const AUIKeyEvent& event) override;
  virtual void OnFocusGained() override;
  virtual void OnFocusLost() override;
  virtual void Enable() override;
  virtual void Disable() override;
  virtual void OnParentResize(uint32_t newWidth, uint32_t newHeight) override;
  void SetInputBoxBGColor(uint32_t color);
  void SetButtonBGColor(uint32_t color);
  void SetListBGColor(uint32_t color);
  void SetInputBoxTextColor(uint32_t color);
  void SetButtonTextColor(uint32_t color);
  void SetListTextColor(uint32_t color);
  virtual void SetFontSize(uint32_t size) override;

  AInputBox* GetInputBox() const { return mInputBox; }

};

} // namespace aui

#endif // ACOMBOBOX_H_
