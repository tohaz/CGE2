//#include "AUILib.h"
//
//using namespace aui;
//
//int32_t test_widget_attachment(AUI *au) {
//  D1("test_widget_attachment start");
//  TEST_ASSERT_NE(au, nullptr, 1);
//  AWindow* w = au->MainWnd();
//  TEST_ASSERT_NE(w, nullptr, 2);
//  ABox* box = ABox::AttachTo(w);
//  TEST_ASSERT_NE(box, nullptr, 3);
//  box->Move(50, 60);
//  box->Resize(200, 150);
//  box->SetBGColor(0xFF8844CC);
//  w->Draw();
//  D1("test_widget_attachment passed");
//  return 0;
//}
//
//int32_t test_click_callback(UNUSED AUI *au) {
//  D1("test_click_callback start");
//  TEST_ASSERT_NE(au, nullptr, 1);
//  AWindow *w = au->MainWnd();
//  TEST_ASSERT_NE(w, nullptr, 2);
//  ABox *box = ABox::AttachTo(w);
//  box->Move(10, 10);
//  box->Resize(100, 100);
//  bool callbackFired = false;
//  box->SetClickCallback([&callbackFired](AWindow*, AWidget*, void*, int32_t x, int32_t y, bool pressed) {
//    if(pressed) {
//      callbackFired = true;
//      D1("Callback fired at ({},{})", x, y);
//    }
//  },
//  nullptr);
//// Simulate mouse press at (50,50) inside the box (10,10 to 110,110)
//// DispatchClick expects coordinates relative to window
//  w->OnMousePress(50, 50, 1);// button 1 = left
//  TEST_ASSERT_EQ(callbackFired, true, 3);
//  D1("test_click_callback passed");
//  return 0;
//}
//
//int32_t test_mousemove_callback(UNUSED AUI *au) {
//  D1("test_mousemove_callback start");
//  TEST_ASSERT_NE(au, nullptr, 1);
//  AWindow* w = au->MainWnd();
//  TEST_ASSERT_NE(w, nullptr, 2);
//  ABox* box = ABox::AttachTo(w);
//  box->Move(100, 100);
//  box->Resize(30, 30);
//  bool moveFired = false;
//  w->OnMousePress(10, 10, 1);
//  w->OnMousePress(10, 10, 0);
//  box->SetMouseMoveCallback([&moveFired](AWindow*, AWidget*, void*, int32_t x, int32_t y) {
//    moveFired = true;
//    D1("Move callback at ({},{})", x, y);
//  },
//  nullptr);
//// Simulate mouse move at (50,50)
//  w->OnMouseMove(50, 50);
//  w->OnMouseMove(60, 60);
//  TEST_ASSERT_EQ(moveFired, true, 3);
//  D1("test_mousemove_callback passed");
//  return 0;
//}
//
//
//int main() {
//  //UNUSED char *qqq = new char[1]; // generate error
//  int32_t testsfailed = 0;
//
//  //testsfailed += runTimedTest("test_widget_attachment", test_widget_attachment, 222);
//  //testsfailed += runTimedTest("test_click_callback", test_click_callback, 222);
//  testsfailed += runTimedTest("test_mousemove_callback", test_mousemove_callback, 2222);
//
//  D("test suite complete")
//
//  return testsfailed;
//}
//
//



//#include "AUILib.h"
//
//using namespace aui;
//
//int main() {
//  AUI* au = AUI::Create("aaa");
//  AWindow* w = au->MainWnd();
//  w->EnableResize();
//  w->Resize(400, 400);
//  ABox* b = ABox::AttachTo(w);
//  b->Move(10, 10);
//  b->Resize(100, 100);
//  ALabel* l = ALabel::AttachTo(w, "qwe");
//  l->Move(100, 100);
//  l->Resize(100, 30);
//  au->ProcessMessages();
//  delete au;
//  return 0;
//}
//
//

#include "AUILib.h"

using namespace aui;

int32_t main() {
    AUI* au = AUI::Create("Emoji example");
    if (!au) return -1;
    AWindow* win = au->MainWnd();
    win->EnableResize();
    win->Resize(400, 200);
    ALabel* label = ALabel::AttachTo(win);
    label->Move(50, 50);
    label->Resize(300, 60);
    label->SetText("Hello 😀 World! 🎉");
    label->SetFontSize(24);
    label->SetBGColor(0xFFEEEEEE);
    label->SetBorderThickness(1);
    au->ProcessMessages();
    delete au;
    return 0;
}


