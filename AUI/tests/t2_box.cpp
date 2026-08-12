#include "AUILib.h"

using namespace aui;

// Refactored test: widget attachment and basic operations
int32_t test_widget_attachment(AUI *au) {
  D1("test_widget_attachment start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 2);
  ABox* box = ABox::AttachTo(w);
  TEST_ASSERT_NE(box, nullptr, 3);
  box->Move(50, 60);
  box->Resize(200, 150);
  box->BGColor(0xFF8844CC);
  D1("test_widget_attachment passed");
  return 0;
}

// Refactored test: click callback
int32_t test_click_callback(AUI *au) {
  D1("test_click_callback start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  ABox* box = ABox::AttachTo(w);
  box->Move(10, 10);
  box->Resize(100, 100);
  UNUSED bool callbackFired = false;
  box->SetMousePressLeftCallback([&callbackFired](AWidget* wid, void*, int32_t x, int32_t y) -> AWidget* {
    if(wid->MousePressedLeft()) {
      callbackFired = true;
      D1("Callback fired at ({},{})", x, y);
    }
    return wid;
  },
  nullptr);
// Simulate mouse press at (50,50) inside the box
  w->OnMousePress(50, 50, BTN_LEFT);
  TEST_ASSERT_EQ(callbackFired, true, 3);
  D1("test_click_callback passed");
  return 0;
}

int32_t test_mousemove_callback(AUI *au) {
  D1("test_mousemove_callback start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 2);
  ABox* box = ABox::AttachTo(w);
  box->Move(10, 10);
  box->Resize(60, 60);
  bool moveFired = false;
  box->SetMouseMoveCallback([&moveFired](AWidget*, void*, int32_t x, int32_t y) {
    moveFired = true;
    D1("Move callback fired at ({},{})", x, y);
  },
  nullptr);
// Simulate mouse move at (50,50)
  w->OnMouseMove(50, 50);
  TEST_ASSERT_EQ(moveFired, true, 3);
  D1("test_mousemove_callback passed");
  return 0;
}

int32_t test_border_0(AUI *au) {
  D1("test_border_0 start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 2);
  w->EnableResize();
  w->Resize(30, 30);
  w->DisableResize();
  // Testing correct rendeing for border size 0
  // Engine layout uses local coordinates for children
  // Border thickness does not affect widget coordinates.
  ABox* bx1 = ABox::AttachTo(w);
  bx1->Resize(25, 25);
  bx1->Move(1, 1);
  bx1->Border(0);
  bx1->BGColor(0xFFF0F0F0);
  bx1->BorderColor(0xFF010101);
  AWidgetReader<ABox> r (bx1, 30, 30);
  TEST_ASSERT_EQ(r.Pixel(0, 0), 0, 3); //buffer clear pixel
  TEST_ASSERT_EQ(r.Pixel(1, 0), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(0, 1), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(1, 1), 0xFFF0F0F0, 3);
  TEST_ASSERT_EQ(r.Pixel(2, 1), 0xFFF0F0F0, 3);
  TEST_ASSERT_EQ(r.Pixel(1, 2), 0xFFF0F0F0, 3);
  TEST_ASSERT_EQ(r.Pixel(24, 24), 0xFFF0F0F0, 3);
  TEST_ASSERT_EQ(r.Pixel(25, 25), 0xFFF0F0F0, 3);
  TEST_ASSERT_EQ(r.Pixel(24, 25), 0xFFF0F0F0, 3);
  TEST_ASSERT_EQ(r.Pixel(25, 24), 0xFFF0F0F0, 3);
  TEST_ASSERT_EQ(r.Pixel(25, 26), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(26, 25), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(26, 26), 0, 3);
  UNUSED int32_t px = 26, py = 26;
  D1("pixel at {} {} is {:x}", px, py, r.Pixel(px, py))
  D1("test_border_0 passed");
  return 0;
}

int32_t test_border_1(AUI *au) {
  D1("test_border_1 start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 2);
  w->EnableResize();
  w->Resize(30, 30);
  w->DisableResize();
  // Testing correct rendeing for border size 1
  // Border thickness does not affect widget coordinates.
  ABox* bx1 = ABox::AttachTo(w);
  bx1->Resize(25, 25);
  bx1->Move(1, 1);
  bx1->Border(1);
  bx1->BorderStyle(AUIBorderStyle::Flat);
  bx1->BGColor(0xFFF0F0F0);
  bx1->BorderColor(0xFF010101);
  AWidgetReader<ABox> r (bx1, 30, 30);
  TEST_ASSERT_EQ(r.Pixel(0, 0), 0, 3); //buffer clear pixel
  TEST_ASSERT_EQ(r.Pixel(1, 0), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(0, 1), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(1, 1), 0xFF010101, 3);
  TEST_ASSERT_EQ(r.Pixel(2, 1), 0xFF010101, 3);
  TEST_ASSERT_EQ(r.Pixel(1, 2), 0xFF010101, 3);
  TEST_ASSERT_EQ(r.Pixel(2, 2), 0xFFF0F0F0, 3);
  TEST_ASSERT_EQ(r.Pixel(24, 24), 0xFFF0F0F0, 3);
  TEST_ASSERT_EQ(r.Pixel(25, 25), 0xFF010101, 3);
  TEST_ASSERT_EQ(r.Pixel(26, 26), 0x0, 3);
  UNUSED int32_t px = 24, py = 24;
  D2("pixel at {} {} is {:x}", px, py, r.Pixel(px, py))
  D1("test_border_1 passed");
  return 0;
}

// 1. Border Drawn After Children
// Verifies that a parent's border is drawn OVER child pixels when they overlap.
int32_t test_border_drawn_after_children(AUI* au) {
  D1("test_'border_drawn_after_children'_starts");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(15, 15);
  w->DisableResize();

  ABox* b = ABox::AttachTo(w);
  b->Resize(5, 5);
  b->Move(1, 1);
  b->Border(1);
  b->BorderColor(0xFF010101);
  b->BGColor(0xFF01FF01);
  b->ClipChildren(true);
  b->BorderStyle(AUIBorderStyle::Flat);

  ABox* b2 = ABox::AttachTo(b);
  b2->Resize(4, 4);
  b2->Move(3, 3);
  b2->Border(0);
  b2->BGColor(0xFFFF0202);
  b2->BorderStyle(AUIBorderStyle::Flat);

  AWidgetReader<ABox> r(b, 10, 10);
  // (4,4) is inside parent bounds -> child color intact
  TEST_ASSERT_EQ(r.Pixel(4, 4), 0xFFFF0202, 3);
  // (5,5) overlaps parent border -> parent border overwrites child
  TEST_ASSERT_EQ(r.Pixel(5, 5), 0xFF010101, 3);

  D1("test_'border_drawn_after_children'_passed");
  return 0;
}

// 2. Overlapping Children with Different Z-Order
// Verifies that sibling widgets render in attachment/Z order (later attached renders on top).
int32_t test_overlapping_children_z_order(AUI* au) {
  D1("test_'overlapping_children_z_order'_starts");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(20, 20);
  w->DisableResize();

  ABox* parent = ABox::AttachTo(w);
  parent->Resize(10, 10);
  parent->Move(0, 0);
  parent->Border(0);
  parent->BGColor(0xFF000000);

  // First child (Lower Z)
  ABox* child1 = ABox::AttachTo(parent);
  child1->Move(1, 1);
  child1->Resize(5, 5);
  child1->Border(0);
  child1->BGColor(0xFF0000FF); // Red

  // Second child (Higher Z - overlaps child1 at (3,3) -> (5,5))
  ABox* child2 = ABox::AttachTo(parent);
  child2->Move(3, 3);
  child2->Resize(5, 5);
  child2->Border(0);
  child2->BGColor(0xFF00FF00); // Green

  AWidgetReader<ABox> r(parent, 10, 10);
  // Non-overlapping region of child1
  TEST_ASSERT_EQ(r.Pixel(2, 2), 0xFF0000FF, 3);
  // Overlapping region -> child2 must overwrite child1
  TEST_ASSERT_EQ(r.Pixel(4, 4), 0xFF00FF00, 3);

  D1("test_'overlapping_children_z_order'_passed");
  return 0;
}

// 3. Nested Widgets of Depth 4
// Verifies coordinate translation and color rendering through 4 hierarchy levels.
int32_t test_nested_depth_4(AUI* au) {
  D1("test_'nested_depth_4'_starts");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(30, 30);
  w->DisableResize();

  // Depth 1
  ABox* b1 = ABox::AttachTo(w);
  b1->Move(1, 1);
  b1->Resize(20, 20);
  b1->Border(0);
  b1->BGColor(0xFF111111);

  // Depth 2 (Abs Pos: 3, 3)
  ABox* b2 = ABox::AttachTo(b1);
  b2->Move(2, 2);
  b2->Resize(15, 15);
  b2->Border(0);
  b2->BGColor(0xFF222222);

  // Depth 3 (Abs Pos: 5, 5)
  ABox* b3 = ABox::AttachTo(b2);
  b3->Move(2, 2);
  b3->Resize(10, 10);
  b3->Border(0);
  b3->BGColor(0xFF333333);

  // Depth 4 (Abs Pos: 7, 7)
  ABox* b4 = ABox::AttachTo(b3);
  b4->Move(2, 2);
  b4->Resize(5, 5);
  b4->Border(0);
  b4->BGColor(0xFF444444);

  AWidgetReader<ABox> r(b1, 30, 30);
  TEST_ASSERT_EQ(r.Pixel(1, 1), 0xFF111111, 3);
  TEST_ASSERT_EQ(r.Pixel(3, 3), 0xFF222222, 3);
  TEST_ASSERT_EQ(r.Pixel(5, 5), 0xFF333333, 3);
  TEST_ASSERT_EQ(r.Pixel(7, 7), 0xFF444444, 3);

  D1("test_'nested_depth_4'_passed");
  return 0;
}

// 4. Partial Clipping Across Multiple Ancestor Rectangles
// Verifies that a leaf node is properly clipped by both its parent AND grandparent bounds.
int32_t test_multi_ancestor_clipping(AUI* au) {
  D1("test_'multi_ancestor_clipping'_starts");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(30, 30);
  w->DisableResize();

  // Grandparent: (2, 2) to (12, 12)
  ABox* gp = ABox::AttachTo(w);
  gp->Move(2, 2);
  gp->Resize(10, 10);
  gp->Border(0);
  gp->BGColor(0xFF111111);
  gp->ClipChildren(true);

  // Parent extends outside Grandparent: local (2, 2) -> absolute (4, 4) to (19, 19)
  ABox* p = ABox::AttachTo(gp);
  p->Move(2, 2);
  p->Resize(15, 15);
  p->Border(0);
  p->BGColor(0xFF222222);
  p->ClipChildren(true);

  // Child positioned inside Parent, but extending past Grandparent clipping boundary:
  // Local (5, 5) relative to Parent -> absolute (9, 9) to (18, 18)
  ABox* c = ABox::AttachTo(p);
  c->Move(5, 5);
  c->Resize(10, 10);
  c->Border(0);
  c->BGColor(0xFFFF0000);

  AWidgetReader<ABox> r(gp, 30, 30);
  // Inside all ancestors (9, 9) -> Child color visible
  TEST_ASSERT_EQ(r.Pixel(9, 9), 0xFFFF0000, 3);
  // (11, 11) is inside Grandparent (max X=11) -> Visible
  TEST_ASSERT_EQ(r.Pixel(11, 11), 0xFFFF0000, 3);
  // (12, 12) is outside Grandparent bounds -> Clipped to 0 (or window background)
  TEST_ASSERT_EQ(r.Pixel(12, 12), 0x0, 3);

  D1("test_'multi_ancestor_clipping'_passed");
  return 0;
}

// 5. Non-Standard Angles (e.g., 33.3 deg or Negative Angles)
// Verifies negative rotation angles work cleanly without underflow or rendering bugs.
int32_t test_non_standard_angles(AUI* au) {
  D1("test_'non_standard_angles'_starts");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(30, 30);
  w->DisableResize();

  ABox* b = ABox::AttachTo(w);
  b->Move(10, 10);
  b->Resize(10, 10);
  b->Border(1);
  b->BorderColor(0xFF0000FF);
  b->BGColor(0xFF00FF00);

  // Test negative angle (-45 degrees should be equivalent to 315 degrees)
  b->Angle(-45);

  AWidgetReader<ABox> r(b, 30, 30);
  // Center of box should stay green regardless of rotation
  TEST_ASSERT_EQ(r.Pixel(15, 15), 0xFF00FF00, 3);

  D1("test_'non_standard_angles'_passed");
  return 0;
}

int32_t test_border_2(AUI *au) {
  D1("test_border_size 2 start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 2);
  w->EnableResize();
  w->Resize(30, 30);
  w->DisableResize();
  // Testing correct rendeing for border size 2
  // Border thickness does not affect widget coordinates.
  ABox* bx1 = ABox::AttachTo(w);
  bx1->Resize(25, 25);
  bx1->Move(1, 1);
  bx1->Border(2);
  bx1->BGColor(0xFFF0F0F0);
  bx1->BorderColor(0xFF010101);
  bx1->BorderStyle(AUIBorderStyle::Flat);
  AWidgetReader<ABox> r (bx1, 30, 30);
  TEST_ASSERT_EQ(r.Pixel(0, 0), 0, 3); //buffer clear pixel
  TEST_ASSERT_EQ(r.Pixel(1, 0), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(0, 1), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(1, 1), 0xFF010101, 3);
  TEST_ASSERT_EQ(r.Pixel(2, 2), 0xFF010101, 3);
  TEST_ASSERT_EQ(r.Pixel(3, 3), 0xFFF0F0F0, 3);
  TEST_ASSERT_EQ(r.Pixel(3, 3), 0xFFF0F0F0, 3);
  TEST_ASSERT_EQ(r.Pixel(23, 23), 0xFFF0F0F0, 3);
  TEST_ASSERT_EQ(r.Pixel(24, 24), 0xFF010101, 3);
  TEST_ASSERT_EQ(r.Pixel(25, 25), 0xFF010101, 3);
  TEST_ASSERT_EQ(r.Pixel(26, 26), 0, 3);
  UNUSED int32_t px = 24, py = 24;
  D2("pixel at {} {} is {:x}", px, py, r.Pixel(px, py))

  D1("test_border_size 2 passed");
  return 0;
}

int32_t test_border_OB_glitch(AUI *au) {
  D1("test_'border_out of bounds' start");
  TEST_ASSERT_NE(au, nullptr, 1);
  AWindow* w = au->MainWnd();
  TEST_ASSERT_NE(w, nullptr, 2);
  w->EnableResize();
  w->Resize(30, 30);
  w->DisableResize();

  // Testing glitch of rendeing for border in nested box widget
  // Border thickness does not affect widget coordinates.
  // Engine layout uses local coordinates for child widgets
  // Border is drawn inwards
  // Unrotated path so widget Angle() is 0

  ABox* bx1 = ABox::AttachTo(w);
  bx1->Move(11, 11);
  bx1->Resize(20, 20);
  bx1->BGColor(0xFF00FFFF);
  bx1->BorderColor(0xFF010101);
  bx1->ClipChildren(true);
  ABox* bx1_1 = ABox::AttachTo(bx1);
  bx1_1->Move(-10, -10);
  bx1_1->Resize(20, 20);
  bx1_1->BGColor(0xFF7700FF);
  bx1_1->BorderColor(0xFF020202);
  D1("paremt angle {}", bx1->Angle())
  AWidgetReader<ABox> r (bx1, 40, 40);
  TEST_ASSERT_EQ(r.Pixel(0, 0), 0, 3); //buffer clear pixel
  TEST_ASSERT_EQ(r.Pixel(1, 0), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(0, 1), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(1, 1), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(2, 1), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(1, 2), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(3, 1), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(1, 3), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(2, 2), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(3, 3), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(4, 4), 0, 3);
  TEST_ASSERT_EQ(r.Pixel(5, 5), 0, 3);
  UNUSED int32_t px = 1, py = 1;
  D2("pixel at {} {} is {:x}", px, py, r.Pixel(px, py))
  D1("test_'border_out of bounds' ends");
  return 0;
}

int32_t test_nested_border(AUI *au) {
  D1("test 'nested border' start");
  UNUSED AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(60, 60);
  w->DisableResize();
  // Testing glitch of rendeing for border in nested box widget
  // Border thickness does not affect widget coordinates.
  // Engine layout uses local coordinates for child widgets
  // Border is drawn inwards.
  // Parent border overwrites after childs are rendered
  // Unrotated path so widget Angle() is 0
  // Children clipping is true by default
  ABox* bx1 = ABox::AttachTo(w);
  bx1->Resize(50, 50);
  bx1->Move(1, 1);
  bx1->Border(3);
  bx1->BorderColor(0xFF0000FF);
  bx1->BorderStyle(AUIBorderStyle::Flat);
  ABox* b1_1 = ABox::AttachTo(bx1);
  b1_1->Resize(40, 20);
  b1_1->Move(10, -10);
  b1_1->Border(5);
  b1_1->BGColor(0xFFFF0000);
  b1_1->BorderColor(0xFFFAFAFA);
  b1_1->BorderStyle(AUIBorderStyle::Flat);

  UNUSED int32_t px = 0, py = 0;
  UNUSED uint32_t t = 0;
  AWidgetReader<ABox> r (bx1, 52, 52);
  px = 0; py = 1, t = 0x0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 1; py = 0, t = 0x0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 2; py = 1, t = 0xFF0000FF;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 1; py = 2, t = 0xFF0000FF;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 3; py = 3, t = 0xFF0000FF;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 4; py = 4, t = 0xFF00FF00;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 10; py = 4, t = 0xFF00FF00;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 11; py = 4, t = 0xFFFAFAFA;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 15; py = 4, t = 0xFFFAFAFA;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 16; py = 4, t = 0xFFFF0000;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 16; py = 5, t = 0xFFFF0000;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 16; py = 6, t = 0xFFFAFAFA;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 47; py = 5, t = 0xFFFAFAFA;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 48; py = 5, t = 0xFF0000FF;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 47; py = 6, t = 0xFFFAFAFA;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 47; py = 10, t = 0xFFFAFAFA;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 47; py = 11, t = 0xFF00FF00;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 51; py = 5, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 51; py = 6, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 51; py = 10, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 51; py = 11, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 50; py = 4, t = 0xFF0000FF;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 50; py = 10, t = 0xFF0000FF;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  D1("test 'nested border' ends");
  return 0;
}

int32_t test_child_visible(AUI* au) {
  D1("test_'child_visible'_start");
  AWindow* w = au->MainWnd();
  // you can't resize without it
  w->EnableResize();
  w->Resize(50, 50);
  w->DisableResize();

  ABox* parent = ABox::AttachTo(w);
  parent->Move(20, 20);
  parent->Resize(30, 30);
  parent->BGColor(0xFF010101);
  parent->Border(1);
  parent->BorderColor(0xFF0000FF);
  parent->BorderStyle(AUIBorderStyle::Flat);
  parent->ClipChildren(true);
  ABox* child = ABox::AttachTo(parent);
  child->Move(10, 10);
  child->Resize(10, 10);
  child->BGColor(0xFFFF0000);
  child->Border(0);

  UNUSED int32_t px = 0, py = 0;
  UNUSED uint32_t t = 0;
  AWidgetReader<ABox> r (parent, 52, 52);
  px = 0; py = 0, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 20; py = 19, t = 0x0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 19; py = 20, t = 0x0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 21; py = 20, t = 0xFF0000FF;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 20; py = 21, t = 0xFF0000FF;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 0; py = 0, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 1; py = 1, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 30; py = 29, t = 0xFF010101;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 29; py = 30, t = 0xFF010101;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 30; py = 30, t = 0xFFFF0000;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 39; py = 39, t = 0xFFFF0000;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 39; py = 40, t = 0xFF010101;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 40; py = 39, t = 0xFF010101;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  D1("test_'child_visible'_passed");
  return 0;
}

int32_t test_basic_rotation(AUI* au) {
  D1("test_'basic_rotation'_starts");
  AWindow* w = au->MainWnd();
  // you can't resize without it
  w->EnableResize();
  w->Resize(20, 20);
  w->DisableResize();

  // Draws rotated box with border 1
  ABox* b = ABox::AttachTo(w);
  b->BGColor(0xFFB9F2FF);
  TEST_ASSERT_EQ(b->BGColor(), 0xFFB9F2FF, 3);
  b->Border(1);
  b->BorderColor(0xFFFF0101);
  b->Move(3, 3);
  b->Resize(5, 5);
  b->Angle(45);
  b->BorderStyle(AUIBorderStyle::Flat);

  int32_t px = 0, py = 0;
  uint32_t t = 0;
  AWidgetReader<ABox> r(b, 10, 10);
  bool c = 1;
  px = 0; py = 0, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 1; py = 1, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 2; py = 2, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 3; py = 3, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 4; py = 2, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 5; py = 2, t = 0xFFFF0101;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 5; py = 5, t = 0xFFB9F2FF;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  D1("test_'basic_rotation'_passed");
  return 0;
}

int32_t test_clipchildren_false(AUI* au) {
  D1("test_'clipchildren_false'_starts");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(15, 15);
  w->DisableResize();
  // Tests if child widget clips outside parent

  ABox* b = ABox::AttachTo(w);
  b->Resize(5, 5);
  b->Move(1, 1);
  b->Border(1);
  b->BorderColor(0xFF010101);
  b->BGColor(0xFF01FF01);
  b->ClipChildren(false);
  b->BorderStyle(AUIBorderStyle::Flat);
  ABox* b2 = ABox::AttachTo(b);
  b2->Resize(4, 4);
  b2->Move(3, 3);
  b2->Border(0);
  b2->BGColor(0xFFFF0202);
  b2->BorderStyle(AUIBorderStyle::Flat);

//  . . . . . . . .
//  . 0 0 0 0 0 . .
//  . 0 G G G 0 . .
//  . 0 G G G 0 . .
//  . 0 G G R R R R
//  . 0 0 0 R R R R
//  . . . . R R R R
//  . . . . R R R R

  int32_t px = 0, py = 0;
  uint32_t t = 0;
  AWidgetReader<ABox> r(b, 10, 10);
  bool c = true;
  px = 1; py = 0, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 0; py = 1, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 1; py = 1, t = 0xFF010101;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 2; py = 2, t = 0xFF01FF01;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 4; py = 4, t = 0xFFFF0202;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 5; py = 5, t = 0xFFFF0202;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 7; py = 7, t = 0xFFFF0202;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 8; py = 8, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);

  D1("test_'clipchildren_false'_passed");
  return 0;
}

int32_t test_clipchildren_true(AUI* au) {
  D1("test_'clipchildren_true'_starts");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(10, 10);
  w->DisableResize();

  // Tests if border is applied on child widget by parent
  ABox* b = ABox::AttachTo(w);
  b->Resize(5, 5);
  b->Move(1, 1);
  b->Border(1);
  b->BorderColor(0xFF010101);
  b->BGColor(0xFF01FF01);
  b->ClipChildren(true);
  b->BorderStyle(AUIBorderStyle::Flat);

  ABox* b2 = ABox::AttachTo(b);
  b2->Resize(4, 4);
  b2->Move(3, 3);
  b2->Border(0);
  b2->BGColor(0xFFFF0202);
  b2->BorderStyle(AUIBorderStyle::Flat);

//  . . . . . . .
//  . 0 0 0 0 0 .
//  . 0 G G G 0 .
//  . 0 G G G 0 .
//  . 0 G G R 0 .
//  . 0 0 0 0 0 .
//  . . . . . . .

  int32_t px = 0, py = 0;
  uint32_t t = 0;
  AWidgetReader<ABox> r(b, 10, 10);
  bool c = true; // arm the test. makes it "exit(1);"
  px = 0; py = 0, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 0; py = 1, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 1; py = 0, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 3; py = 3, t = 0xFF01FF01;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 4; py = 4, t = 0xFFFF0202;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 5; py = 5, t = 0xFF010101;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 6; py = 6, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);

  D1("test_'clipchildren_true'_passed");
  return 0;
}

int32_t test_clipchildren_rotated(AUI* au) {
  D1("test_'test_clipchildren_rotated'_starts");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(15, 15);
  w->DisableResize();

  // Tests if border is applied on child widget by parent
  // Widget is rotated
  ABox* b = ABox::AttachTo(w);
  b->Resize(10, 10);
  b->Move(2, 2);
  b->Border(1);
  b->BorderColor(0xFF010101);
  b->BGColor(0xFF01FF01);
  b->ClipChildren(true);
  b->BorderStyle(AUIBorderStyle::Flat);

  ABox* b2 = ABox::AttachTo(b);
  b2->Resize(9, 9);
  b2->Move(3, 3);
  b2->Border(1);
  b2->BGColor(0xFFFF0202);
  b2->BorderColor(0xFF0101FF);
  b2->Angle(45);
  b2->BorderStyle(AUIBorderStyle::Flat);

  int32_t px = 0, py = 0;
  uint32_t t = 0;
  AWidgetReader<ABox> r(b, 13, 13);
  bool c = true; // arm the test. makes it "exit(1);" on error
  px = 0; py = 0, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 12; py = 12, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 2; py = 2, t = 0xFF010101;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 11; py = 4, t = 0xFF010101;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 11; py = 5, t = 0xFF010101;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 5; py = 11, t = 0xFF010101;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 6; py = 11, t = 0xFF010101;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 5; py = 6, t = 0xFF01FF01;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 9; py = 4, t = 0xFF0101FF;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 10; py = 12, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 12; py = 10, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);

  D1("test_'test_clipchildren_rotated'_passed");
  return 0;
}

int32_t test_inner_box_content_rotated(AUI* au) {
  D1("test_'test_inner_box_content_rotated'_starts");
  AWindow* w = au->MainWnd();
  w->EnableResize();
  w->Resize(12, 12);
  w->DisableResize();
  // Tests if boxes draw with correct fills
  // Widgets are rotated
  ABox* bx1 = ABox::AttachTo(w);
  bx1->BGColor(0xFF00FF00);
  bx1->Move(2, 2);
  bx1->Resize(10, 10);
  bx1->Angle(1);
  ABox* bx2 = ABox::AttachTo(bx1);
  bx2->BGColor(0xFFFF0000);
  bx2->Move(2, 2);
  bx2->Resize(4, 4);
  bx2->Angle(1);

  int32_t px = 0, py = 0;
  uint32_t t = 0;
  AWidgetReader<ABox> r(bx1, 12, 12);
  bool c = true; // arm the test. makes it "exit(1);" on error
  px = 0; py = 0, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 1; py = 1, t = 0;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 2; py = 2, t = 0xFF000000;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 3; py = 3, t = 0xFF00FF00;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 4; py = 4, t = 0xFF000000;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 5; py = 5, t = 0xFFFF0000;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 6; py = 6, t = 0xFFFF0000;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 7; py = 7, t = 0xFF000000;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);
  px = 8; py = 8, t = 0xFF00FF00;
  D1("pixel at {} {} is {:x} should be {:x}", px, py, r.Pixel(px, py), t)
  if(c)TEST_ASSERT_EQ(r.Pixel(px, py), t, 3);

  D1("test_'test_inner_box_content_rotated'_passed");
  return 0;
}

int main() {
  //UNUSED char *qqq = new char[1]; // generate error
  UNUSED int32_t testsfailed = 0;

//  AUI* au = AUI::Create("box testing");
//  AWindow* w = au->MainWnd();
//  w->BGColor(0xFF222222);
//  w->EnableResize();
//  w->Resize(12, 12);
//  w->DisableResize();

  testsfailed += runTimedTest(test_widget_attachment, 1);
  testsfailed += runTimedTest(test_click_callback, 1);
  testsfailed += runTimedTest(test_mousemove_callback, 1);
  testsfailed += runTimedTest(test_border_0, 1);
  testsfailed += runTimedTest(test_border_1, 1);
  testsfailed += runTimedTest(test_border_2, 1);
  testsfailed += runTimedTest(test_border_OB_glitch, 1);
  testsfailed += runTimedTest(test_nested_border, 1);
  testsfailed += runTimedTest(test_child_visible, 1);
  testsfailed += runTimedTest(test_basic_rotation, 1);
  testsfailed += runTimedTest(test_clipchildren_false, 1);
  testsfailed += runTimedTest(test_clipchildren_true, 1);
  testsfailed += runTimedTest(test_border_drawn_after_children, 1);
  testsfailed += runTimedTest(test_overlapping_children_z_order, 1);
  testsfailed += runTimedTest(test_nested_depth_4, 1);
  testsfailed += runTimedTest(test_multi_ancestor_clipping, 1);
  testsfailed += runTimedTest(test_non_standard_angles, 1);
  testsfailed += runTimedTest(test_clipchildren_rotated, 1);
  testsfailed += runTimedTest(test_inner_box_content_rotated, 1);

  testsfailed += runTimedTest(test_widget_attachment, 200);
  testsfailed += runTimedTest(test_click_callback, 200);
  testsfailed += runTimedTest(test_mousemove_callback, 200);
  testsfailed += runTimedTest(test_border_0, 200);
  testsfailed += runTimedTest(test_border_1, 200);
  testsfailed += runTimedTest(test_border_2, 200);
  testsfailed += runTimedTest(test_border_OB_glitch, 200);
  testsfailed += runTimedTest(test_nested_border, 200);
  testsfailed += runTimedTest(test_child_visible, 200);
  testsfailed += runTimedTest(test_basic_rotation, 200);
  testsfailed += runTimedTest(test_clipchildren_false, 200);
  testsfailed += runTimedTest(test_clipchildren_true, 200);
  testsfailed += runTimedTest(test_border_drawn_after_children, 200);
  testsfailed += runTimedTest(test_overlapping_children_z_order, 200);
  testsfailed += runTimedTest(test_nested_depth_4, 200);
  testsfailed += runTimedTest(test_multi_ancestor_clipping, 200);
  testsfailed += runTimedTest(test_non_standard_angles, 200);
  testsfailed += runTimedTest(test_clipchildren_rotated, 200);
  testsfailed += runTimedTest(test_inner_box_content_rotated, 200);

//  au->ProcessMessages();
//  delete au;

  D("ABox test suite complete");
  return testsfailed;
}
