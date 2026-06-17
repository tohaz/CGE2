#ifndef DEFAULTS_H_
#define DEFAULTS_H_

#define DEBUG_LEVEL 1

// try {deference 0}
// catch(SegmentationFault:) {}

#define UNUSED [[maybe_unused]]

static auto gAUI_timer_start = std::chrono::high_resolution_clock::now();

inline void print_stack() {
  void* buffer[64];
  size_t first = 0, last = 0;
  int8_t* demangled = 0;
  std::string entry = "", mangled = "";
  int size = backtrace(buffer, 64), status = 0;
  char** symbols = backtrace_symbols(buffer, size);
  for (int i = 1; i < size; ++i) {
    entry = symbols[i];
    first = entry.find('(');
    last = entry.find('+');
    if (first != std::string::npos && last != std::string::npos && first < last) {
      mangled = entry.substr(first + 1, last - first - 1);
      demangled = (int8_t*)abi::__cxa_demangle(mangled.c_str(), nullptr, nullptr, &status);
      if (status == 0) {
        printf("  #%d %s\n", i, demangled);
        free(demangled);
        continue;
      }
    }
    printf(" #%d %s\n", i, symbols[i]);
  }
  free(symbols);
}

class ScopedTimer {
    const char* m_msg;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
public:
    ScopedTimer(const char* msg) : m_msg(msg), m_start(std::chrono::high_resolution_clock::now()) {}
    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start).count();
        std::cout << m_msg << ": " << us << " us" << std::endl;
    }
};

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  #error "Not tested on Big-endian system"
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
//  #error "Little-endian"
#else
  #error "Unknown Endianness"
#endif

static_assert(sizeof(char) == 1, "char must be 1 bytes");
static_assert(sizeof(short) == 2, "short must be 2 bytes");
static_assert(sizeof(int) == 4, "integer must be 4 bytes");
static_assert(sizeof(long) == 8, "long must be 8 bytes");
#define UCHAR8 uint8_t
#define UINT8 uint8_t
#define INT32 int32_t
#define UINT32 uint32_t
#define INT64 int64_t
#define UINT64 uint64_t

//#ifdef BUILDING_MY_STATIC_LIB
//  #pragma GCC poison short int long unsigned signed
//#endif

#ifdef BUILDING_MY_STATIC_LIB
template <int N>
struct ProhibitedType {
    static_assert(N == 0,
    "\n==================================================\n"
    " AUI LIBRARY COMPILATION ERROR:\n"
    " Standard built-in integer types are prohibited!\n"
    " Please use fixed-width types:\n"
    " - Instead of 'int'      -> use 'int32_t'\n"
    " - Instead of 'short'    -> use 'int16_t'\n"
    " - Instead of 'unsigned' -> use 'uint32_t'\n"
    "==================================================");
    using type = void;
};
#define int      typename ProhibitedType<1>::type
#define short    typename ProhibitedType<1>::type
#define long     typename ProhibitedType<1>::type
#define unsigned typename ProhibitedType<1>::type
#define signed   typename ProhibitedType<1>::type
#endif // BUILDING_MY_STATIC_LIB

#pragma pack(push, 1)
union ARGBColor {
    UINT32 value;
    struct {
        UCHAR8 b; // Least significant byte (LSB) on Little-Endian x86_64 storage layouts
        UCHAR8 g;
        UCHAR8 r;
        UCHAR8 a; // Most significant byte (MSB) on Little-Endian x86_64 storage layouts matching 0xAARRGGBB
    } argb;
    ARGBColor() :
        value(0xFF000000U) {
    }
    ARGBColor(UINT32 val) :
        value(val) {
    }
    ARGBColor(UCHAR8 red, UCHAR8 green, UCHAR8 blue, UCHAR8 alpha = 255U) {
      argb.a = alpha;
      argb.r = red;
      argb.g = green;
      argb.b = blue;
    }
    UCHAR8 getLuminance() const {
      return static_cast<UCHAR8>((static_cast<UINT32>(argb.r) + static_cast<UINT32>(argb.g) + static_cast<UINT32>(argb.b)) / 3U);
    }
    operator UINT32() const { return value; }
    void Clear() { value = 0U; }
};
#pragma pack(pop)

enum class AUIKeyCode : uint32_t {
  None = 0,
  Enter,
  Backspace,
  Delete,
  Insert,
  Left,
  Right,
  Up,
  Down,
  Home,
  End,
  Tab,
  Escape,
  Space
// Add others as needed
};

enum class AUIModifier : uint8_t {
  None = 0,
  Shift = 1 << 0,
  Ctrl = 1 << 1,
  Alt = 1 << 2,
  Super = 1 << 3,
};

inline AUIModifier operator|(AUIModifier a, AUIModifier b) {
    return static_cast<AUIModifier>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline AUIModifier operator&(AUIModifier a, AUIModifier b) {
    return static_cast<AUIModifier>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline AUIModifier& operator|=(AUIModifier& a, AUIModifier b) {
    a = a | b;
    return a;
}

struct AUIKeyEvent {
    bool pressed;               // true = key down, false = key up
    uint32_t unicode;           // Unicode code point (0 if not a printable character)
    AUIKeyCode code;            // logical key code (for non‑printable keys)
    AUIModifier modifiers;      // active modifiers
};

// AWindow
#define AUI_DEFAULT_WINDOW_TITLE "aui dummy title, set me plz"
#define AUI_DEFAULT_WINDOW_SZX 500
#define AUI_DEFAULT_WINDOW_SZY 300
#define AUI_DEFAULT_WINDOW_BG 0xFFAAAAAAU
// AButton
#define AUI_DEFAULT_BUTTON_X 10
#define AUI_DEFAULT_BUTTON_Y 10
#define AUI_DEFAULT_BUTTON_SZX 80
#define AUI_DEFAULT_BUTTON_SZY 30
// disabled for now as it conflicts with content drawing
//#define AUI_DEFAULT_BUTTON_BORDERW 3
#define AUI_DEFAULT_BUTTON_BG 0xFFCCCCCC
// AInputBox
#define AUI_DEFAULT_INPUT_X 15
#define AUI_DEFAULT_INPUT_Y 15
#define AUI_DEFAULT_INPUT_SZX 80
#define AUI_DEFAULT_INPUT_SZY 20
#define AUI_DEFAULT_INPUT_BG 0xFFAACCAA
#define AUI_DEFAULT_INPUT_FG 0xFF000000
#define AUI_DEFAULT_INPUT_CURSORW 2
#define AUI_DEFAULT_INPUT_CURSORH 10
#define AUI_DEFAULT_INPUT_BORDERW 2
// ALabel
#define AUI_DEFAULT_LABEL_BG 0xFFBBBBBB
#define AUI_DEFAULT_LABEL_SZX 120
#define AUI_DEFAULT_LABEL_SZY 30
#define AUI_DEFAULT_LABEL_BORDERW 0
// AList
#define AUI_LIST_X 20
#define AUI_LIST_Y 20
#define AUI_LIST_SZX 300
#define AUI_LIST_SZY 200
#define AUI_LIST_FG_COLOR 0xFF333333
#define AUI_LIST_BG 0xFFCCCCCC
// ABox
#define AUI_BOX_X 25
#define AUI_BOX_Y 25
#define AUI_BOX_SZX 100
#define AUI_BOX_SZY 100
// ATable
#define AUI_TABLE_X 20
#define AUI_TABLE_Y 20
#define AUI_TABLE_SZX 300
#define AUI_TABLE_SZY 200
#define AUI_TABLE_CELL_W 50
#define AUI_TABLE_CELL_H 18
#define AUI_TABLE_BG 0xFF999999
#define AUI_TABLE_INTERSEC_BG 0xFF888888
#define AUI_TABLE_SCROLL_THICK 5

// how much color will attempt to darken/lighten on highlight operation
// value must be less than 0x80
#define AUI_HL_SHIFT 20
//#define CG_DEFAULT_FONT "-*-helvetica-medium-r-*--*-120-100-100-*-*-iso8859-1"
#define AUI_DEFAULT_FONT "-*-*-*-R-Normal--18-*-100-100-*-*-iso8859-1"

enum class AUIWidgetType {
  unset = 1,
  defaultWindow = 1001,
  defaultButton = 1002,
  defaultList = 1003,
  defaultLabel = 1004,
  defaultInputBox = 1005,
  defaultTable = 1006,
  defaultPopupMenu = 1007,
  defaultModalWindow = 1008,
  defaultComboBox = 1009,
  defaultProgressBar = 1010,
  defaultScrollBar = 1011
};

enum class AUIWindowType {
  unset = 2,
  Wayland = 2001,
  XCB = 2002
};

struct AWidgetSettings {
  std::string text = "";
  INT64 x = 10;
  INT64 y = 10;
  UINT64 width = 100;
  UINT64 height = 28;
  UINT32 backgroundColor = 0xE0E0E0;
  AUIWidgetType type = AUIWidgetType::unset;
  bool startVisible = true;
};

enum class AUIOrientation {
  vertical = 1,
  horizontal = 2
};

enum class AUIHAlign {
  center = 1,
  left = 2,
  right = 3
};

enum class AUIVAlign {
  center = 1,
  top = 2,
  bottom = 3
};

enum class AUIWidgetStyle {
    Flat = 1,
    Simple3D = 2
};

enum class AUICursorType {
    Default,
    HResize,
    VResize
};

#ifndef AUI_GIT_VERSION
#define AUI_GIT_VERSION "Not a controlled build"
#endif


#define DT(fmt, ...) \
  do { \
    auto now = std::chrono::system_clock::now(); \
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000; \
    std::println("{:%H:%M:%S}.{:03d} {}|{}({}): " fmt, now, (int32_t)ms.count(), __FILE__, __func__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
  } while (0);

// DD stands for Debug Do. D1-D4 and W() are removed on release build (DEBUG_LEVEL 0)
#ifndef DEBUG_LEVEL
    #define DEBUG_LEVEL 0
#endif
#if DEBUG_LEVEL > 0
    #define INTERNAL_PRINT(prefix, lvl, fmt, ...)try { \
            std::println("{}{} {}|{}({}): " fmt, prefix, lvl, __FILE__, __func__, __LINE__ __VA_OPT__(,) __VA_ARGS__); \
        } catch (const std::exception& __debug_e) { \
            std::print("!!! DEBUG FORMAT ERROR at {}|{}({}): ", __FILE__, __func__, __LINE__); \
            std::println("{} !!!", __debug_e.what()); \
        }
    // Default debug macro (Level 1)
    #define D(fmt, ...) do { INTERNAL_PRINT("D", 1, fmt __VA_OPT__(,) __VA_ARGS__); } while (0);
    // E() - Fatal Error: Prints, dumps stack, and exits.
    #define E(fmt, ...) do {INTERNAL_PRINT("E", 1, fmt __VA_OPT__(,) __VA_ARGS__);print_stack();exit(1); \
    } while (0);
    // DD() - Executes any code only in debug builds
    #define DD(...) do { __VA_ARGS__;  } while (0);
    // W() - Lightweight marker
    #define W() do { try { std::println("W {}|{}({})", __FILE__, __func__, __LINE__); } catch(...) {} } while (0);
    // DS() - Global Stack Trace
    #define DS() do { D("\n---Trace at {}:{}---", __FILE__, __LINE__) print_stack(); } while (0);
#else
    // RELEASE MODE (DEBUG_LEVEL 0)
    #define D(...)  do {} while (0);
    #define E(...)  do {} while (0);
    #define DD(...) do {} while (0);
    #define W()     do {} while (0);
    #define DS()    do {} while (0);
#endif

// --- LEVEL 1 ---
#if DEBUG_LEVEL >= 1
    #define D1(fmt, ...) do { INTERNAL_PRINT("D", 1, fmt __VA_OPT__(,) __VA_ARGS__); } while (0);
    #define DS1() DS()
#else
    #define D1(...)  do {} while (0);
    #define DS1()    do {} while (0);
#endif

// --- LEVEL 2 ---
#if DEBUG_LEVEL >= 2
    #define D2(fmt, ...) do { INTERNAL_PRINT("D", 2, fmt __VA_OPT__(,) __VA_ARGS__); } while (0);
    #define DS2() DS()
#else
    #define D2(...)  do {} while (0);
    #define DS2()    do {} while (0);
#endif

// --- LEVEL 3 ---
#if DEBUG_LEVEL >= 3
    #define D3(fmt, ...) do { INTERNAL_PRINT("D", 3, fmt __VA_OPT__(,) __VA_ARGS__); } while (0);
    #define DS3() DS()
#else
    #define D3(...)  do {} while (0);
    #define DS3()    do {} while (0);
#endif

// --- LEVEL 4 ---
#if DEBUG_LEVEL >= 4
    #define D4(fmt, ...) do { INTERNAL_PRINT("D", 4, fmt __VA_OPT__(,) __VA_ARGS__); } while (0);
#else
    #define D4(...)  do {} while (0);
#endif

#define TEST_ASSERT(cond, errcode) do { if(!(cond)) { E("Test failed: {}", #cond); return errcode; } } while(0)

#define TEST_ASSERT_EQ(actual, expected, errcode) \
    do { \
        auto act = (actual); \
        auto exp = (expected); \
        if (act != exp) { \
            E("Test failed: {} == {} (actual: {}, expected: {})", #actual, #expected, act, exp); \
            return errcode; \
        } \
    } while(0)

#define TEST_ASSERT_NE(actual, expected, errcode) \
    do { \
        auto act = (actual); \
        auto exp = (expected); \
        if (act == exp) { \
            E("Test failed: {} != {} (actual: {}, expected: {})", #actual, #expected, act, exp); \
            return errcode; \
        } \
    } while(0)

const std::unordered_map<std::string,UINT64> string_to_case{
   {"BackSpace", 1},
   {"space", 2},
   {"Return", 3},
   {"KP_Enter", 4},
   {"Left", 5},
   {"Right", 6},
   {"Delete", 7},
   {"Insert", 8}
};

static std::string BaseAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#endif // DEFAULTS_H_
