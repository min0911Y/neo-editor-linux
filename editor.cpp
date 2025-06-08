/* Editor.cpp : 文本编辑器*/
// Copyright (C) 2024 min0911
// #include <mouse.h>
// #include <mst.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
// #include <syscall.h>

#include <iostream> // For std::cout, std::endl
#include <cstdio>   // For printf, putchar, remove, rename
#include <cstdint>  // For uintptr_t
#include <fstream>  // For std::ifstream, std::ofstream
#include <vector>   // For std::vector (used in new filesize)
#include <string>   // For std::string (used in new Edit_File)
#include <ncurses.h> // For ncurses integration
#include <cctype>    // For isprint
#include <cstdlib>   // For strtol, malloc, realloc, free (already used, but good to ensure)

// --- Basic Console I/O Stubs/Placeholders ---

// Forward declarations for functions that might be used by these stubs
// (if they were originally in the removed headers)
// We'll define these properly or replace them later.
// For now, this helps reduce immediate "undeclared identifier" errors.

// extern "C" int tty_get_xsize(); // Declaration will be kept if needed, definition changes
// extern "C" int tty_get_ysize(); // Declaration will be kept if needed, definition changes

// Ncurses versions of console I/O
extern "C" int tty_get_xsize() { return COLS; } // Added extern "C"
extern "C" int tty_get_ysize() { return LINES; } // Added extern "C"

void goto_xy(int x, int y) {
    move(y, x); // Ncurses: move(row, col)
}

// Renamed original putch to avoid conflict
void editor_putch(char ch) {
    addch(ch);
}

void print(const char* str) {
    addstr(str);
}

void editor_clear() { // Renamed from clear()
    erase(); // Ncurses function to clear screen
}

unsigned short get_cons_color() {
    // This function's return value was used to restore color.
    // With ncurses, we explicitly set colors.
    // So, returning a "default" DOS color is mainly for placeholder.
    return 0x07; // Default DOS white on black
}

void set_cons_color(unsigned short color) {
    if (!has_colors()) return;

    // attr_t new_attr = A_NORMAL; // new_attr not used
    int pair_number = 0;

    // Basic mapping - can be expanded
    // DOS colors: Lower nibble is foreground, upper is background.
    // 0=black, 1=blue, 2=green, 3=cyan, 4=red, 5=magenta, 6=yellow, 7=white
    // We've defined pairs based on common usage rather than direct bit-mapping for now.
    switch (color) {
        case 0x07: // White on Black (normal)
            pair_number = 1;
            break;
        case 0x70: // Black on White (status bar)
            pair_number = 2;
            break;
        // Add more cases if other specific DOS color attributes are used elsewhere.
        // For syntax highlighting, we might need a more dynamic mapping or more pairs.
        // Example: if original code used 0x0C for bright red on black
        case 0x0C: // Bright Red on Black
        case 0x04: // Red on Black
            pair_number = 3;
            break;
        case 0x02: // Green on Black
            pair_number = 4;
            break;
        case 0x01: // Blue on Black
            pair_number = 5;
            break;
        case 0x03: // Cyan on Black
            pair_number = 6;
            break;
        case 0x05: // Magenta on Black
            pair_number = 7;
            break;
        case 0x06: // Yellow on Black
             pair_number = 8;
             break;
        default:   // Default to normal
            pair_number = 1; // Or 0 if pair 0 is A_NORMAL (pair 0 is special in ncurses)
            break;
    }
    
    // Turn off all previous attributes, then turn on the new one.
    // A_NORMAL might not be enough if other attributes (bold, etc.) were used.
    attroff(A_COLOR); // Turn off color attributes specifically
    // bkgd(COLOR_PAIR(0)); // Reset background to default pair 0 if needed (optional)
    if (pair_number > 0) {
         attron(COLOR_PAIR(pair_number));
    }
    // else if (pair_number == 0) { attron(A_NORMAL); } // if pair_number can be 0 for default
}

void tty_stop_cur_moving() {
    curs_set(0); // Make cursor invisible
}

void tty_start_cur_moving() {
    curs_set(1); // Make cursor visible (normal)
}


// --- Stubs for other missing functions from DOS headers ---
// Add stubs for functions that were likely in mouse.h, mst.h, or syscall.h
// to reduce compilation errors.
// We will replace these with functional equivalents or remove them later.

// Old Mouse Stubs (Removed)
// int mouse_support() { return 0; }
// int get_mouse() { return 0; }
// int GetMouse_btn(int mouse_event) { (void)mouse_event; return 0; }
// int GetMouse_x(int mouse_event) { (void)mouse_event; return 0; }
// int GetMouse_y(int mouse_event) { (void)mouse_event; return 0; }

// From syscall.h (custom DOS file operations) - Replaced with C++ implementations
long filesize(const char* filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return -1; // Error or file not found
    }
    long size = file.tellg();
    file.close();
    return size == -1 ? -1 : size; // tellg can return -1 on error
}

void api_ReadFile(const char* filename, char* buffer) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open() || !buffer) {
        if (buffer) buffer[0] = '\0';
        return;
    }

    // Get file size to know how much to read
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::ios_base::beg); // Corrected std::ios::beg

    if (size > 0) {
        file.read(buffer, size);
        if (file.gcount() < size) { // If read less than expected
             // Handle error or partial read if necessary, for now, null terminate what was read
             buffer[file.gcount()] = '\0';
        } else {
            buffer[size] = '\0'; // Ensure null termination
        }
    } else {
        buffer[0] = '\0'; // Empty file or error
    }
    file.close();
}

void mkfile(const char* filename) {
    std::ofstream file(filename, std::ios::binary); // Just creating it is enough
    if (file.is_open()) {
        file.close();
    }
    // No explicit error handling here, matches original stub's lack of return.
    // Downstream code expects filesize to be -1 if it doesn't exist.
}

void Edit_File(const char* filename, const char* buffer, int len, int unknown_flag) {
    (void)unknown_flag; // Original flag likely DOS specific, ignore.

    // The original main function uses Edit_File like this:
    // Edit_File(argv[1], bb, strlen(c) + mLine(c, l), 0);
    // It seems to imply that 'len' is the correct length of 'bb'.
    // However, 'bb' is constructed by adding \r for every \n in 'c'.
    // strlen(c) + mLine(c,l) IS the correct length of bb.

    // Atomicity: Write to a temporary file then rename to avoid data loss on crash
    std::string temp_filename = std::string(filename) + ".tmp";
    std::ofstream file(temp_filename, std::ios::binary | std::ios::trunc);
    if (!file.is_open() || !buffer) {
        return; // Cannot open temp file or buffer is null
    }

    file.write(buffer, len);
    file.close();

    if (file.fail()) { // Check if write or close failed
        std::remove(temp_filename.c_str()); // Clean up temp file
        return;
    }

    // Replace original file with temp file
    // std::rename (from <cstdio>) is preferred.
    // Need to remove the old file first on some systems for rename to work if it exists.
    std::remove(filename); // Remove original, C++17 std::filesystem::rename is better but sticking to C++11 for now
    if (std::rename(temp_filename.c_str(), filename) != 0) {
        // Handle rename error, e.g., print an error, try to restore backup, etc.
        // For now, just clean up temp file if rename failed.
        std::remove(temp_filename.c_str());
    }
}

// From mst.h (likely related to syntax highlighting config)
// These are more complex and involve custom types.
// For now, just declare the types as incomplete and stub functions.
struct MST_Object; // Defined as global variable later
struct SPACE;     // Defined as global variable later
struct Array;     // Defined as global variable later // This might conflict with std::array if <array> is included.
              // The original code doesn't use std::array.

// MST_Object* mst_obj = nullptr; // Global variable from original code - defined later
// SPACE* comment_config = nullptr; // Defined later
// SPACE* number_config = nullptr; // Defined later
// Array* keymap = nullptr; // Defined later
// Array* string_style = nullptr; // Defined later

// Example stub for a function that might have been in mst.h
// This is a guess, actual functions will be clear from compiler errors.
// MST_space_get_integer, MST_get_var, MST_space_get_str would also need stubs
// once they appear as undefined. For now, the global nullptrs might be enough
// to resolve some issues.

// Old Threading stub (Removed)
// void AddThread(const char* name, void* func_ptr, void* stack_ptr) {
//     (void)name;
//     (void)func_ptr;
//     (void)stack_ptr;
// }

// Text_Draw_Box - this was a macro T_DrawBox in the original code.
// It's used for drawing boxes, possibly for UI elements or highlighting.
void Text_Draw_Box(int r1, int c1, int r2, int c2, int dos_color) {
    if (!has_colors() && dos_color != 0x07) { // If no colors, only draw if it's default color
        // Or decide how to handle colors on non-color terminals.
        // For now, we'll just use the default attributes if no colors.
    } else {
         set_cons_color(dos_color); // Set the color for the box
    }

    // Draw corners
    mvaddch(r1, c1, ACS_ULCORNER);
    mvaddch(r1, c2, ACS_URCORNER);
    mvaddch(r2, c1, ACS_LLCORNER);
    mvaddch(r2, c2, ACS_LRCORNER);

    // Draw horizontal lines
    for (int x = c1 + 1; x < c2; ++x) {
        mvaddch(r1, x, ACS_HLINE); // Top
        mvaddch(r2, x, ACS_HLINE); // Bottom
    }

    // Draw vertical lines
    for (int y = r1 + 1; y < r2; ++y) {
        mvaddch(y, c1, ACS_VLINE); // Left
        mvaddch(y, c2, ACS_VLINE); // Right
    }
    
    // It's generally the caller's responsibility to reset color after the box
    // or if Text_Draw_Box is part of a larger drawing operation, the color
    // set here will persist for subsequent drawing commands in that operation
    // until set_cons_color is called again.
    // For safety, if this function is called standalone, one might want to reset
    // to a default color, but given its usage, it's likely part of render::showAll
    // which manages colors.
    // set_cons_color(0x07); // Optionally reset to default color if needed immediately
}

int get_xy() { return 0; } // Added stub - Note: ncurses has getyx(stdscr, y, x);

// Removed: int getch() { return getchar(); } // Ncurses getch() will be used

void scan(char* buf, int n) { // Added stub - Note: ncurses has getstr() or variants
    (void)n; // Suppress unused parameter warning for now
    scanf("%s", buf); // This will be replaced by ncurses input like getnstr
}

// #define CLICK_LEFT 1 // Removed - ncurses uses event.bstate & BUTTON1_CLICKED etc.

#define VIEW_LINE 0 // 有bug，暂时不开启
int mLine(char *buffer, int len);
#define T_DrawBox(x, y, w, h, c) Text_Draw_Box((y), (x), (h) + y, (w) + x, (c))
// extern "C" int tty_get_xsize(); // Removed, definition is now extern "C"
// extern "C" int tty_get_ysize(); // Removed, definition is now extern "C"
int max_line, max_char_a_line;
#define MAX_LINE max_line
#define MAX_CHAR_A_LINE max_char_a_line
// Editor处理超过最大行的文件的方式：
/*
  利用这个Camera结构体中的y来处理当前的位置
  比喻为摄像机（其实是一个base结构体，存储着一些信息：比如buffer之类的）
 */
struct Camera {
  int y;                          // 摄像机高度
  int curser_pos_x, curser_pos_y; // 光标位置
  int index;                      // 光标指向位置的index
  char *buffer;                   // 缓冲区
  int array_len;                  // 缓冲区长度（已malloc） =>
                 // 如果字符数量超过这个数，就会调用realloc
  int len; // 字符总长
#if VIEW_LINE
  int ml; // 行数
#endif
};
// Editor利用这两个结构体描述当前渲染的结果，以便render类输出到屏幕
// 由Parse类控制这两个结构体
struct Char {
  int index; // 在buffer中的index
  unsigned char ch;
};
struct Line {
  int line_flag;   // 是不是回车敲出的行
  Char *line;      // 一整行
  int len;         // 这个行有几个字符
  int start_index; // 行首索引
};
struct StringStyle {
  char *start;
  char *end;
};
/* 如果要实现高亮，需要使用这些变量 */
MST_Object *mst_obj;
SPACE *comment_config;
SPACE *number_config;
Array *keymap;
Array *string_style;
bool number;
int number_color;
struct km_style {
  char *key;
  int color;
};
///////////////高亮依赖///////////////
// 转换大写
void strtoupper(char *str) {
  while (*str != '\0') {
    if (*str >= 'a' && *str <= 'z') {
      *str -= 32;
    }
    str++;
  }
}
void get_next(char *T, int next[]) {
  int strs_len = strlen(T);
  int i = 0, j = -1;
  next[0] = -1;
  while (i < strs_len) { // 遍历
    if (j == -1 || T[i] == T[j]) {
      i++;
      j++;
      next[i] = j;
    } else {
      j = next[j];
    }
  }
}

int kmp(char *str_main, char *str_branch) {
  int next[100];
  int str_main_len = strlen(str_main);
  int str_branch_len = strlen(str_branch);
  int i, j;
  get_next(str_branch, next);
  i = 0;
  j = 0;
  /*kmp 核心
      当主串i和子串j相等的时候，i++,j++;
      反之 j = next[j];
      当 j==-1 的时候，i++,j++
  */
  while (i < str_main_len && j < str_branch_len) {
    if (str_main[i] == str_branch[j]) {
      i++;
      j++;
    } else {
      j = next[j];
      if (j == -1) {
        j++;
        i++;
      }
    }
  }
  if (j >= str_branch_len) {
    /*此时说明在主串中找到了子串*/
    return i - str_branch_len;
  } else {
    return -1; // 没有找到的话返回-1
  }
}
// 处理转义字符
void convert(char *str) {
  int i, j;
  for (i = 0, j = 0; str[i] != '\0'; i++, j++) {
    if (str[i] == '\\') {
      i++;
      switch (str[i]) {
      case 'n':
        str[j] = '\n';
        break;
      case 't':
        str[j] = '\t';
        break;
      case 'r':
        str[j] = '\r';
        break;
      case '\"':
        str[j] = '\"';
        break;
      case '\\':
        str[j] = '\\';
        break;
      default:
        str[j] = str[i];
      }
    } else
      str[j] = str[i];
  }
  str[j] = '\0';
}

// EVector C++-style的数组实现
template <typename T> class EVector {
public:
  EVector() : _size(0), _capacity(1), _data(new T[1]) {}

  EVector(int capacity)
      : _size(0), _capacity(capacity), _data(new T[capacity]) {}

  ~EVector() {
    clear();
    delete[] _data;
  }

  int size() const { return _size; }

  bool empty() const { return _size == 0; }

  T &operator[](int index) {
    // assert(index >= 0 && index < _size);
    return _data[index];
  }

  const T &operator[](int index) const {
    //  assert(index >= 0 && index < _size);
    return _data[index];
  }

  void clear() {
    for (int i = 0; i < _size; i++) {
      _data[i].~T();
    }
    _size = 0;
  }

  void push_back(const T &value) {
    if (_size >= _capacity) {
      reserve(_capacity * 2);
    }
    new (_data + _size) T(value);
    _size++;
  }

  void pop_back() {
    // assert(_size > 0);
    _data[_size - 1].~T();
    _size--;
  }

  void insert(int index, const T &value) {
    // assert(index >= 0 && index <= _size);
    if (_size >= _capacity) {
      reserve(_capacity * 2);
    }
    for (int i = _size; i > index; i--) {
      new (_data + i) T(_data[i - 1]);
      _data[i - 1].~T();
    }
    new (_data + index) T(value);
    _size++;
  }

  void erase(int index) {
    // assert(index >= 0 && index < _size);
    _data[index].~T();
    for (int i = index; i < _size - 1; i++) {
      new (_data + i) T(_data[i + 1]);
      _data[i + 1].~T();
    }
    _size--;
  }

  void reserve(int capacity) {
    if (capacity <= _capacity) {
      return;
    }
    T *newData = new T[capacity];
    for (int i = 0; i < _size; i++) {
      new (newData + i) T(_data[i]);
      _data[i].~T();
    }
    delete[] _data;
    _data = newData;
    _capacity = capacity;
  }

private:
  int _size;
  int _capacity;
  T *_data;
};
EVector<StringStyle> string_vector;
EVector<km_style> key_map;

////////////////高亮结束/////////////////////

// 用“ ”画一个长方形
void putSpace(int x, int y, int w, int h) {
  goto_xy(x, y);
  for (int i = 0; i < h; i++) {
    goto_xy(x, y + i);
    for (int j = 0; j < w; j++) {
      editor_putch(' '); // Changed to editor_putch
    }
  }
}
// 显示状态栏
void setState(const char *msg) { // Changed to const char*
  short bx, by;
  (void)bx; (void)by; // Mark as unused
  unsigned cons = get_cons_color();
  set_cons_color(0x70);
  putSpace(0, MAX_LINE, MAX_CHAR_A_LINE, 1); // 先清空最下面那行
  goto_xy(0, MAX_LINE);                      // 然后控制光标到那边
  print(msg); // 然后咱们把信息打印出来
  set_cons_color(cons);
}
// 插入一个字符
void insert_char(char *str, int pos, char ch, Camera *c) {
  // 如果我们已经malloc的字节数小于添加字符后字符总数的大小（存不下了），我们就要进行realloc
  // realloc需要干两件事：1. realloc本身 2. 重新设置c->buffer和array_len
  if (c->len + 1 > c->array_len) {
    str = (char *)realloc(
        c->buffer,
        c->array_len + 100); // 多malloc 100还是别的 可以自行选择，我选100
    c->buffer = str;
    c->array_len += 100;
  }
  // 好了，没有后顾之忧了，咱们插入字符
  int i;
  for (i = c->len; i >= pos; i--) {
    str[i + 1] = str[i];
  }
  str[pos] = ch;
  c->len++;
}
// 插入一个字符串，就是循环调用上面这个函数
void insert_str(char *str, int pos, Camera *c) {
  for (int i = 0; i < c->len; i++) {
    insert_char(c->buffer, pos++, str[i], c);
  }
}
// 删除某个字符串
void delete_char(char *str, int pos, Camera *c) {
  int i;
  int l = c->len;
  if (l == 0) { // 你没事吧？
    return;
  }
  for (i = pos; i < c->len; i++) {
    str[i] = str[i + 1];
  }
  str[l - 1] = 0; // 设置一下字符串结束符
  c->len--;
}
// 获取向上n行的第一个字符的索引
int get_index_of_nth_last_line(int n, char *buf, int pos, int len) {
  int line_count = 0;
  int i = pos - 1;
  int start_of_line = -1;
  if (buf[i] == '\n') {
    i--;
  }
  while (i >= 0) {
    if (buf[i] == '\n') {
      line_count++;
      if (line_count == n) {
        start_of_line = i + 1;
        break;
      }
    }
    i--;
  }
  if (start_of_line == -1) {
    int s;
    start_of_line = 0;
    s = start_of_line;
    for (; start_of_line < pos - 1; start_of_line++) {
      if (buf[start_of_line] == '\n') {
        break;
      }
    }
    // printf("r = %d\n", start_of_line - s);
    if (start_of_line - s > MAX_CHAR_A_LINE) {
      s = start_of_line - (start_of_line - s) % MAX_CHAR_A_LINE;
    }
    return s;
  } else {
    int s;
    s = start_of_line;
    for (; start_of_line < pos - 1; start_of_line++) {
      if (buf[start_of_line] == '\n') {
        break;
      }
    }
    // printf("r = %d\n", start_of_line - s);
    if (start_of_line - s > MAX_CHAR_A_LINE) {
      s = start_of_line - (start_of_line - s) % MAX_CHAR_A_LINE;
    }
    return s;
  }
}
// 获取向下n个字符的索引
int get_index_of_nth_next_line(int n, char *buf, int pos, int len) {
  int i = 0;
  int j = 0;
  int f_flag = 0;
  for (; pos < len; pos++) {
    if (i == n) {
      if (f_flag) {
        pos--;
      }
      return pos;
    }
    f_flag = 0;
    if (buf[pos] == '\n' || j == MAX_CHAR_A_LINE) {
      i++;
      j = 0;
      if (j == MAX_CHAR_A_LINE) {
        f_flag = 1;
      }
    } else {
      j++;
    }
  }
  return pos; // Added return for control path that reaches end of non-void function
}
/* parse类是重要《核心》之一 */
/*
  Editor利用他来获取当前的一个渲染布局，才能控制光标啊什么的走向以及index
  利用先获取布局中字符的index，再进行选择index的值（Parse类的用处），会使程序变得简单、稳定、bug少
*/
class parse {
public:
  parse(Camera *c) { // 构造函数，初始化了Line缓冲区和各个变量
    this->l = (Line *)malloc(sizeof(Line) * MAX_LINE);
    for (int i = 0; i < MAX_LINE; i++) {
      this->l[i].line = (Char *)malloc(sizeof(Char) * MAX_CHAR_A_LINE);
    }
    camera = c;
    clean();
    /* 这三个都只有一个用处---减少遍历缓冲区的次数，降低时间复杂度
      ny就是指上一次Set的y值（摄像机高度），我们通过get_index_of_nth_xxx_line函数来定位当前摄像机高度下，
      我们布局中，第一个字符的位置是在哪里，并且给到nidx，而是否启用，和从哪里开始，就要用到ny这个媒介来存储
      nidx刚刚提到，nidx是now
      index的缩写，editor中是用于记录布局中第一个字符的索引，通过记录这样一个索引，
      我们就不用每次都重新遍历一遍，大大增加程序的效率
      cf的全称是change_flag =>
      在用户输入、删除缓冲区中的内容时，cf会置为1,所以cf是用来记录这个缓冲区有没有改变，
      以此用来判断是否需要Set，没有改变就不需要Set，减少遍历次数
    */
    ny = 0;
    nidx = 0;
    cf = 1; // 调用构造函数时，布局为空，必须要设置为1才能正常加载
  }
  void SetUse() { cf = 1; } // 这里就是用来告诉parse类，缓冲区修改过了
  // 核心函数：设置布局
  void Set() {
    // 首先，要判断到底需不需要重新设置布局
    // 当满足下列条件时，则不用设置布局
    /*
      1. ny和当前摄像机高度相同
      2. 缓冲区没有被修改过
     */
    if (ny == static_cast<unsigned int>(camera->y) && cf == 0) { // Cast camera->y to unsigned
      return;
    }
    cf = 0;
    // 根据camera的y值来设置布局
    clean(); // 清空布局
    /* 这n个变量有的有用有的没有用，我一一来解释一下 */
    /*
      l -----> 没用，历史残留，不做解释，有用到也是废话工程
      sc -----> 记录当前行的字符数量
      f  ----> 没用
      nl ----> 行，用于定位现在解析到第几行
      len ----> 当前行的长度
      sl ----> 没用
      i ----> 索引
    */
    int l = 0;
    int sc = 0;
    int f = 0; (void)f; // Mark f as unused
    int nl = 0;
    int len = 0;
    int sl = 0; (void)sl; // Mark sl as unused
    int i;
    // 首先，重新设置nidx，控制nidx到当前摄像机高度下的第一个字符
    if (ny == static_cast<unsigned int>(camera->y)) { // Cast camera->y to unsigned
      i = nidx;
      l = ny;
    } else if (ny > static_cast<unsigned int>(camera->y)) { // Cast camera->y to unsigned
      nidx = get_index_of_nth_last_line(ny - camera->y, camera->buffer, nidx,
                                        camera->len);
      i = nidx;
      l = camera->y;
      ny = l;
    } else { // ny < 摄像机高度， 说明用户向下移动了几行
      i = get_index_of_nth_next_line(camera->y - ny, camera->buffer, nidx,
                                     camera->len);
      nidx = i;
      ny = camera->y;
    }
    for (; i < camera->len && nl < MAX_LINE; i++) { // 开始解析
      if (sc == 0) {                                // sl == 0 处于行开头
        this->l[nl].start_index = i;                // 设置行开始的index
      }
      if (camera->buffer[i] == '\n' ||
          sc == MAX_CHAR_A_LINE) { // 是否需要进行换行？
        this->l[nl].line_flag =
            1; // 此行不是空白行（即有东西记录，如字符、换行符等）
        this->l[nl].len = len; // 设置长度
        // reset(全部归零，nl自增，换下一行)
        len = 0;
        sl = 0;
        nl++;
        f = sc == MAX_CHAR_A_LINE ? 1 : 0;
        sc = 0;
      } else {
        // 设置字符
        this->l[nl].line[sc++].ch = camera->buffer[i];
        this->l[nl].line[sc - 1].index = i; // 设置索引
        len++;
        // 如果满MAX_CHAR_A_LINE个字符，就在这里换行
        if (sc == MAX_CHAR_A_LINE) {
          if (i + 1 < camera->len &&
              camera->buffer[i + 1] ==
                  '\n') { // 后面还有东西，说明不是一个超过屏幕长度的行
            i++;
          }
          this->l[nl].len = MAX_CHAR_A_LINE; // 设置长度
          // reset
          nl++;
          f = sc == MAX_CHAR_A_LINE ? 1 : 0;
          sc = 0;
          len = 0;
        }
      }
    }
    // 最后一行会遗漏，这里补上
    f = sc == MAX_CHAR_A_LINE ? 1 : 0;
    sc = 0;
    if (sc == 0 && nl < MAX_LINE) { // 不让他越界访问
      this->l[nl].line_flag = 1;
      this->l[nl].len = len;
      this->l[nl].start_index = i;
      len = 0;
      sl = 0;
    }
  }
  Line *getBuf() { return l; }

private:
  Camera *camera;
  Line *l;
  unsigned int wtf; // 编译器抽风了，这里不加一个变量，ny就会自动无限置1
  // 下面的变量在上面解释过
  unsigned int ny;
  int nidx;
  int cf;
  void clean() { // 重置
    for (int i = 0; i < MAX_LINE; i++) {
      l[i].line_flag = 0;
      l[i].len = 0;
      l[i].start_index = 0;
      for (int j = 0; j < MAX_CHAR_A_LINE; j++) {
        l[i].line[j].index = 0;
        l[i].line[j].ch = 0;
      }
    }
  }
};
// render 类：将Parse类处理好的信息显示到屏幕上，我称之为渲染
class render {
  /*
    buf ---- 文件缓冲区
    c ---- 基结构体
    p ---- parse类，用于处理布局
    filename --- 文件名称
    buffer_screen ---- 用于记录当前屏幕已经被显示的字符
    buf_x/y ---- not used
  */
  char *buf;
  parse *p;
  Camera *camera;
  char *filename;
  char **buffer_screen;
  int buf_x, buf_y;

public:
  render(char *buffer, Camera *c, parse *_p, char *fm) {
    // 用来处理下面的状态栏显示的文字
    this->buf0 = (char *)malloc(MAX_CHAR_A_LINE);
    this->buf1 = (char *)malloc(MAX_CHAR_A_LINE + 1);
    // 分配MAX_LINE x MAX_CHAR_A_LINE的二维数组
    buffer_screen = (char **)malloc(MAX_LINE * sizeof(char **));
    for (int i = 0; i < MAX_LINE; i++) {
      buffer_screen[i] = (char *)malloc(MAX_CHAR_A_LINE);
    }
    r_clean(); // 清空
    r_check(); // check一下，确保都到位了（有时候会出现写了但没写上去的情况，这里检查一下，实际上不检查也可以）
    buf = buffer;
    p = _p;
    camera = c;
    filename = fm;
    buf_x = 0;
    buf_y = 0;
  }
  /* 高效的打印字符，空间换时间 */
  void r_putch(int ch) {
    // 获取当前光标的xy,那么我们就不用再去记录光标的位置了
    short bx, by;
    // int r = get_xy(); // get_xy stub returns 0, not useful here.
    // Ncurses getyx can be used if needed, but r_putch logic might change.
    // For now, assume direct put.
    getyx(stdscr, by, bx); // Get current cursor position from ncurses

    if (buffer_screen[by][bx] != ch) { // 如果说不一样？
      buffer_screen[by][bx] = ch; // 那就重新设置一下 然后覆盖输出
      editor_putch(ch); // Changed to editor_putch
    } else { // 一样的话还管啥啊
      // goto_xy(bx + 1, by); // This logic might need rethink with ncurses direct addch
      // For now, if char is same, just advance cursor
      // Or, rely on addch to advance cursor. Let's try relying on addch.
      // If we need to explicitly move, it would be move(by, bx + 1)
      // but addch itself advances the cursor.
      // So, if char is same, we effectively do nothing to screen, cursor position is key.
      // This part of "efficient printing" might behave differently.
      // Let's ensure cursor is where it should be for next potential char.
      move(by, bx + 1);
    }
  }
  /* 清空缓冲区 */
  void r_clean() {
    for (int i = 0; i < MAX_LINE; i++) {
      for (int j = 0; j < MAX_CHAR_A_LINE; j++) {
        buffer_screen[i][j] = ' ';
      }
    }
  }
  /* 清空之后检查一下清空完成没有（或许没必要） */
  void r_check() {
    for (int i = 0; i < MAX_LINE; i++) {
      for (int j = 0; j < MAX_CHAR_A_LINE; j++) {
        if (buffer_screen[i][j] != ' ') {
          printf("error\n");
          for (;;)
            ;
        }
      }
    }
  }
  ///////////////显示////////////////////
  void showAll() {
    Line *l = p->getBuf(); // 获取布局信息
    char buf[90]; (void)buf; // Mark buf as unused
    tty_stop_cur_moving(); // 暂停光标移动
    goto_xy(0, 0);         // 将下一个字符的显示位置移动到原点
    for (int i = 0; i < MAX_LINE; i++) {
      goto_xy(0, i);
#if 0
      int fg = 0;
      int fg1 = 0;
      int ll = 0;
      int b = 0;
      char *end;
#endif
      for (int j = 0, l1 = 0; j < MAX_CHAR_A_LINE; j++) {
        (void)l1; // Mark l1 as unused
        r_putch(l[i].line[j].ch == '\0' ? ' ' : l[i].line[j].ch);
#if 0
        if (mst_obj == nullptr) {
          continue;
        }
        if (fg1) {
          T_DrawBox(j, i, 1, 1, 0x0c);
          if (b >= ll) {
            buf[l1++] = l[i].line[j].ch;
            buf[l1] = 0;
            if (kmp(buf, end) != -1) {
              fg1 = 0;
              buf[l1] = 0;
              l1 = 0;
            }
          } else {
            b++;
          }
          continue;
        }
        if (fg) {
          T_DrawBox(
              j, i, 1, 1,
              MST_space_get_integer(MST_get_var("color", comment_config)));
          continue;
        }
        if (number) {
          if ((l[i].line[j].ch - 0x30 <= 9) && (l[i].line[j].ch - 0x30 >= 0))
            T_DrawBox(j, i, 1, 1, number_color);
        }

        if (l[i].line[j].ch == ' ' || l[i].line[j].ch == '\0') {
          buf[l1] = 0;
          l1 = 0;
          if (l[i].len != 0) {
            for (int h = 0; h < key_map.size(); h++) {
              int pos = 0;
              int pos1 = 0;
              char *buf1 = buf;
              while (1) {
                pos = kmp(buf1, key_map[h].key);
                if (pos == -1) {
                  break;
                }
                pos1 += pos;
                T_DrawBox(j - ((strlen(buf) - pos1)), i, strlen(key_map[h].key),
                          1, key_map[h].color);
                pos1 += strlen(key_map[h].key);
                buf1 += pos + strlen(key_map[h].key);
                // printf("%d\n",pos);
              }
            }
          }

        } else {
          buf[l1++] = l[i].line[j].ch;
          buf[l1] = 0;
          if (kmp(buf, MST_space_get_str(MST_get_var("key", comment_config))) !=
              -1) {
            j -= strlen(MST_space_get_str(MST_get_var("key", comment_config)));
            fg = 1;
            goto_xy(j + 1, i);
          } else {
            for (int q = 0; q < string_vector.size(); q++) {
              if (kmp(buf, string_vector[q].start) != -1) {
                j -= strlen(string_vector[q].start);
                end = string_vector[q].end;
                fg1 = 1;
                buf[l1] = 0;
                l1 = 0;
                ll = strlen(string_vector[q].start);
                b = 0;
                goto_xy(j + 1, i);
                for (int h = 0; h < key_map.size(); h++) {
                  int pos = 0;
                  int pos1 = 0;
                  char *buf1 = buf;
                  while (1) {
                    pos = kmp(buf1, key_map[h].key);
                    if (pos == -1) {
                      break;
                    }
                    pos1 += pos;
                    T_DrawBox(j - ((strlen(buf) - pos1)), i,
                              strlen(key_map[h].key), 1, key_map[h].color);
                    pos1 += strlen(key_map[h].key);
                    buf1 += pos + strlen(key_map[h].key);
                    // printf("%d\n",pos);
                  }
                }

                break;
              }
            }
          }
        }
#endif
      }
      buf[0] = 0;
    }
    /* 通过一个MAX_CHAR_A_LINE个空格的字符串，往里面写，让格式不会乱套 */
    memset(buf1, 0, MAX_CHAR_A_LINE + 1);
    memset(buf1, ' ', MAX_CHAR_A_LINE);
    char buf2[5];
    memset(buf0, 0, MAX_CHAR_A_LINE);
#if VIEW_LINE

    sprintf(buf0, "COL %d ROW %d/%d      %s | Text", camera->index,
            camera->y + camera->curser_pos_y + 1, camera->ml + 1, filename);
#else
    sprintf(buf0, "COL %d LINE %d      %s | Text", camera->index,
            camera->y + camera->curser_pos_y + 1, filename);
#endif
    if (camera->len != 0) { // 分母不是0
      int d = (int)(((float)camera->index / (float)camera->len) *
                    100); // 算出百分比
      const char *s; // Changed to const char*
      // 计算之后需要的间隔
      if (d == 100) {
        s = "";
      } else if (d < 100 && d != 0) {
        s = " ";
      } else {
        s = "  ";
      }
      sprintf(buf2, "%s%d%%", s, d);
    } else {
      sprintf(buf2, " --%%");
    }
    for (size_t i = 0; i < strlen(buf0); i++) { // Changed int to size_t
      buf1[i] = buf0[i];
    }
    for (size_t i = 0; i < strlen(buf2); i++) { // Changed int to size_t
      buf1[MAX_CHAR_A_LINE - 5 + i] = buf2[i];
    }
    // 设置状态栏
    setState(buf1);
    goto_xy(camera->curser_pos_x, camera->curser_pos_y);

    // 重新开始移动光标
    tty_start_cur_moving();
    refresh(); // Added ncurses refresh
  }

private:
  char *buf0, *buf1;
};
// 暂时没用，这里不写说明
bool Need_Sroll(Line *l) {
  if (l[MAX_LINE - 1].line_flag == 1 || l[MAX_LINE - 1].line[0].ch != '\0') {
    return true;
  }
  return false;
}
// 最大可显示行数
int Show_Line_Max(Line *l) {
  int i;
  for (i = 0; i < MAX_LINE; i++) {
    if (l[i].line[0].ch == '\0' && l[i].line_flag != 1) {
      return i;
    }
  }
  return i;
}
class Note {
  Camera *camera;
  parse *p;

public:
  int maxLine() {
    int l = 0;
    int sc = 0;
    for (int i = 0; i < camera->len; i++) {
      if (camera->buffer[i] == '\n' || sc == MAX_CHAR_A_LINE) {
        l++;
        sc = 0;
      } else {
        sc++;
      }
    }
    return l;
  }

  Note(Camera *c, parse *_p) {
    camera = c;
    p = _p;
  }
  void Insert(char ch) {
#if VIEW_LINE
    if (ch == '\n') {
      camera->ml++;
    } else {
      // FIXME: 行数计算错误
      p->Set();
      Line *l = p->getBuf();
      char *s = (char *)(camera->index + (uint32_t)camera->buffer);
      for (; s > camera->buffer && *s != '\n'; s--)
        ;
      if (*s == '\n') {
        s++;
      }
      int a = 0;
      for (; a < (camera->len + (s - camera->buffer)) && s[a] != '\n'; a++)
        ;
      if (a != 0 && a % MAX_CHAR_A_LINE == 0) {
        camera->ml++;
      }
    }
#endif
    insert_char(camera->buffer, camera->index, ch, camera); // 直接插入
    p->SetUse(); // 标记修改过了
  }
  void Delete() {
    /* 判断3“0”情况 */
#if VIEW_LINE
    if (camera->buffer[camera->index] == '\n') {
      camera->ml--;
    } else {
      p->Set();
      Line *l = p->getBuf();

      char *s = (char *)(camera->index + (uint32_t)camera->buffer);
      for (; s > camera->buffer && *s != '\n'; s--)
        ;
      if (s != camera->buffer) {
        s++;
      }
      int a = 0;
      for (; a < camera->len && s[a] != '\n'; a++)
        ;
      if (a != 0 && (a - 1) != 0 && (a - 1) % MAX_CHAR_A_LINE == 0) {
        camera->ml--;
      }
    }
#endif
    delete_char(camera->buffer, camera->index, camera); // 直接删除
    p->SetUse();                                        // 标记一下
  }
  void To(int line) {
    if (line <= 0) {
      setState("Cannot To");
      getch();
      return;
    }
    p->Set();
    Line *l = p->getBuf();
    if (line <= (camera->y) + MAX_LINE && (line > camera->y)) {
      if (l[line - camera->y - 1].line_flag == 0) {
        setState("Cannot To");
        getch();
        return;
      }
      camera->curser_pos_y = line - camera->y - 1;
      if (l[line - camera->y - 1].len == 0) {
        camera->index = l[line - camera->y - 1].start_index;
      } else {
        camera->index = l[line - camera->y - 1]
                            .line[l[line - camera->y - 1].len - 1]
                            .index +
                        1;
      }
      if (camera->buffer[l[line - camera->y - 1]
                             .line[l[line - camera->y - 1].len - 1]
                             .index +
                         1] != '\n' &&
          l[line - camera->y - 1].len == MAX_CHAR_A_LINE) {
        camera->curser_pos_x = l[line - camera->y - 1].len - 1;
        camera->index--;
      } else {
        camera->curser_pos_x = l[line - camera->y - 1].len;
      }
    } else {
      if (line > maxLine() + 1) {
        setState("Cannot To");
        getch();
      } else {
        camera->y = line - 1;
        p->Set();
        l = p->getBuf();
        camera->curser_pos_y = line - camera->y - 1;
        if (l[line - camera->y - 1].len == 0) {
          camera->index = l[line - camera->y - 1].start_index;
        } else {
          camera->index = l[line - camera->y - 1]
                              .line[l[line - camera->y - 1].len - 1]
                              .index +
                          1;
        }
        if (camera->buffer[l[line - camera->y - 1]
                               .line[l[line - camera->y - 1].len - 1]
                               .index +
                           1] != '\n' &&
            l[line - camera->y - 1].len == MAX_CHAR_A_LINE) {
          camera->curser_pos_x = l[line - camera->y - 1].len - 1;
          camera->index--;
        } else {
          camera->curser_pos_x = l[line - camera->y - 1].len;
        }
      }
    }
  }
  void Click(int x, int y) {
    p->Set();
    Line *l = p->getBuf(); // 获取当前行布局
    if (y >= MAX_LINE) {
      return;
    }
    if (l[y].line[0].ch == '\0' && l[y].line_flag == 0) {
      return;
    }
    if (l[y].len < x) {
      x = l[y].len - 1;
      if (l[y].len == 0) {
        camera->index = l[y].start_index;
        camera->curser_pos_x = 0;
      } else {
        camera->index = l[y].line[x].index + 1;
        camera->curser_pos_x = x + 1;
      }
      camera->curser_pos_y = y;
    } else {
      if (l[y].len == 0) {
        camera->index = l[y].start_index;
      } else {
        camera->index = l[y].line[x].index;
      }
      camera->curser_pos_x = x;
      camera->curser_pos_y = y;
    }
  }
  /* 上下左右操作 */

  // 向上移动
  void up() {
    if (camera->y == 0 && camera->curser_pos_y == 0) {
      // 移不了了，就什么都不干
      return;
    }
    if (camera->curser_pos_y == 0) {
      camera->y--;
    }
    p->Set();
    Line *l = p->getBuf(); // 获取当前行布局
    if (camera->curser_pos_y == 0) {
      if (l[0].len == 0) {
        camera->index = l[0].start_index;
      } else {
        camera->index = l[0].line[l[0].len - 1].index + 1;
      }
      if (camera->buffer[l[camera->curser_pos_y]
                             .line[l[camera->curser_pos_y].len - 1]
                             .index +
                         1] != '\n' &&
          l[camera->curser_pos_y].len == MAX_CHAR_A_LINE) {
        camera->curser_pos_x = l[camera->curser_pos_y].len - 1;
        camera->index--;
      } else {
        camera->curser_pos_x = l[camera->curser_pos_y].len;
      }
      camera->curser_pos_y = 0;
    } else {
      camera->curser_pos_y--;
      if (l[camera->curser_pos_y].len == 0) {
        camera->index = l[camera->curser_pos_y].start_index;
      } else {
        camera->index = l[camera->curser_pos_y]
                            .line[l[camera->curser_pos_y].len - 1]
                            .index +
                        1;
      }
      if (camera->buffer[l[camera->curser_pos_y]
                             .line[l[camera->curser_pos_y].len - 1]
                             .index +
                         1] != '\n' &&
          l[camera->curser_pos_y].len == MAX_CHAR_A_LINE) {
        camera->curser_pos_x = l[camera->curser_pos_y].len - 1;
        camera->index--;
      } else {
        camera->curser_pos_x = l[camera->curser_pos_y].len;
      }
    }
  }
  // 向下移动
  int down() {
    Line *l;
    p->Set();
    l = p->getBuf();
    int ml = maxLine();
    if (camera->curser_pos_y != MAX_LINE - 1) {
      if (l[camera->curser_pos_y + 1].line[0].ch == '\0' &&
          l[camera->curser_pos_y + 1].line_flag == 0) {
        return 0; // failed
      }
    } else {
      if (ml < (camera->y + camera->curser_pos_y) + 1) {
        return 0; // failed
      }
    }
    if (camera->curser_pos_y == MAX_LINE - 1) {
      camera->y++;
    }
    p->Set();
    l = p->getBuf(); // 获取当前行布局
    if (camera->curser_pos_y == MAX_LINE - 1) {
      if (l[MAX_LINE - 1].len == 0) {
        camera->index = l[MAX_LINE - 1].start_index;
      } else {
        camera->index = l[MAX_LINE - 1].line[l[MAX_LINE - 1].len - 1].index + 1;
      }
      if (camera->buffer[l[camera->curser_pos_y]
                             .line[l[camera->curser_pos_y].len - 1]
                             .index +
                         1] != '\n' &&
          l[camera->curser_pos_y].len == MAX_CHAR_A_LINE) {
        camera->curser_pos_x = l[camera->curser_pos_y].len - 1;
        camera->index--;
      } else {
        camera->curser_pos_x = l[camera->curser_pos_y].len;
      }
      camera->curser_pos_y = MAX_LINE - 1;

    } else {
      camera->curser_pos_y++;
      if (l[camera->curser_pos_y].len == 0) {
        camera->index = l[camera->curser_pos_y].start_index;
      } else {
        camera->index = l[camera->curser_pos_y]
                            .line[l[camera->curser_pos_y].len - 1]
                            .index +
                        1;
      }
      if (camera->buffer[l[camera->curser_pos_y]
                             .line[l[camera->curser_pos_y].len - 1]
                             .index +
                         1] != '\n' &&
          l[camera->curser_pos_y].len == MAX_CHAR_A_LINE) {
        camera->curser_pos_x = l[camera->curser_pos_y].len - 1;
        camera->index--;
      } else {
        camera->curser_pos_x = l[camera->curser_pos_y].len;
      }
    }
    return 1; // success
  }
  // 左移动
  void left() {
    if (camera->curser_pos_x == 0) { // 已经完全无法左移了，所以上移
      up();
    } else {
      // 就是正常左移，索引减去一
      camera->curser_pos_x--;
      camera->index--;
    }
  }
  // 右移动
  void right(int b) {
    /*
      b指代模式
      0 代表输入的向右
      1 代表用户更改索引位置时的向右
     */
    // 获取行布局
    p->Set();
    Line *l = p->getBuf();
    if ((camera->curser_pos_x == l[camera->curser_pos_y].len && b)) {
      int ret = down(); // 看一下能不能下去
      if (ret) {        // 成功了！
        if (b) {        // 用户的向右
          camera->curser_pos_x = 0;
          if (l[camera->curser_pos_y].len != 0) {
            camera->index = l[camera->curser_pos_y].line[0].index;
          }
        } else {
          // 正常输入的
          camera->curser_pos_x = 1;
          camera->index = l[camera->curser_pos_y].start_index + 1;
        }
      }
    } else {
      if (camera->curser_pos_x == MAX_CHAR_A_LINE) {
        camera->curser_pos_x = 1;
        if (camera->curser_pos_y == MAX_LINE - 1) {
          camera->y++;
        } else {
          camera->curser_pos_y++;
        }
      } else {
        camera->curser_pos_x++;
      }
      camera->index++;
    }
  }
};
////////////////处理鼠标的进程函数声明///////////////
void m_thread(void *s);

class Editor {
public:
  parse *prse;
  Note *n;
  Camera *c;
  render *r;
  void Click(int x, int y) {
    n->Click(x, y);
    r->showAll();
  }
  void Up() {
    if (c->y == 0) {
      return;
    }
    c->curser_pos_x = 0;
    c->curser_pos_y = 0;

    n->up();
    r->showAll();
  }
  void Down() {
    int temp_x = c->curser_pos_x;
    int temp_y = c->curser_pos_y;
    c->curser_pos_x = 0;
    c->curser_pos_y = MAX_LINE - 1;
    if (n->down() == 0) {
      c->curser_pos_x = temp_x;
      c->curser_pos_y = temp_y;
    }
    r->showAll();
  }
  char *Main(char *filename) {
    MEVENT event; // For mouse events
    // system("cls"); // This will not work as expected with ncurses. Ncurses handles screen clearing.
    // Note: The original `system("cls")` was here. It's removed as ncurses manages the screen.
    c = (Camera *)malloc(sizeof(Camera));
    c->buffer = (char *)malloc(filesize(filename) + 1000);
    char *bf2 = (char *)malloc(filesize(filename) + 1000);
    c->array_len = filesize(filename) + 1000;
    c->len = 0;
#if VIEW_LINE
    c->ml = 0;
#endif
    c->y = 0;
    c->curser_pos_x = 0;
    c->curser_pos_y = 0;
    c->index = 0;

    if (filesize(filename) != -1) {
      api_ReadFile(filename, bf2);
      int fsz = filesize(filename);
      for (int i = 0, j = 0; i < fsz + 1000; i++) {
        if (bf2[i] != '\r') {
          c->buffer[j++] = bf2[i];
        }
      }
    } else {
      mkfile(filename);
    }
    free(bf2);
    c->len = strlen(c->buffer);
    prse = new parse(c);
    prse->Set();
    n = new Note(c, prse);
    Line *l = prse->getBuf(); // This is the existing declaration of 'l' (line 1457)
    r = new render(c->buffer, c, prse, filename);
#if VIEW_LINE
    c->ml = n->maxLine();
#endif
    char *stack = (char *)malloc(16 * 1024);
    stack += 16 * 1024 - 4;
    unsigned int *s = (unsigned int *)(stack);
    *s = (unsigned int)(uintptr_t)this; // Fixed cast
    // Removed call to AddThread and mouse_support()
    // if (mouse_support())
    //   AddThread("mouse", (void*)&m_thread, (void*)(stack - 4));
    r->showAll();
    int times = 0; (void)times; // Mark times as unused
    int tap = 0; // For tab/auto-indent logic
    int flag = 0; // For 'stap' command (toggle auto-indent behavior)
    // Line *l = prse->getBuf(); // REMOVED REDECLARATION - 'l' is already in scope from line 1457
    
    for (;;) {
        int ch = getch(); // Ncurses getch()

        // if (ch == 0) { // This check is likely not needed with ncurses getch()
        //     continue;
        // }

        if (ch == KEY_MOUSE) {
            if (getmouse(&event) == OK) {
                if (event.bstate & BUTTON1_CLICKED) {
                    this->Click(event.x, event.y);
                } else if (event.bstate & BUTTON4_PRESSED) { // Scroll wheel up
                    this->Up();
                } else if (event.bstate & BUTTON5_PRESSED) { // Scroll wheel down
                    this->Down();
                }
            }
        } else {
            // Keyboard handling logic
            if (ch == '\n' || ch == KEY_ENTER) { // Handle KEY_ENTER as well
                l = prse->getBuf(); // Refresh line buffer before potential use
                if (!flag) { 
                    tap = 0; 
                    if (c->curser_pos_y < MAX_LINE && l[c->curser_pos_y].line_flag) {
                        for (tap = 0; tap < l[c->curser_pos_y].len; tap++) {
                            if (l[c->curser_pos_y].line[tap].ch != ' ')
                                break;
                        }
                    }
                }
                n->Insert('\n');
                n->down(); 
                
                l = prse->getBuf(); // Re-fetch lines after potential modification by n->down()

                if (c->curser_pos_x != 0) { 
                    c->curser_pos_x = 0;
                    if (c->curser_pos_y < MAX_LINE && l[c->curser_pos_y].line_flag) {
                         if (l[c->curser_pos_y].len > 0) c->index = l[c->curser_pos_y].line[0].index;
                         else c->index = l[c->curser_pos_y].start_index;
                    } else if (c->curser_pos_y < MAX_LINE) { 
                         c->index = l[c->curser_pos_y].start_index;
                    }
                }

                if (!flag) { 
                    for (int i = 0; i < tap; i++) {
                        n->Insert(' ');
                        n->right(0); 
                    }
                }
            } else if (ch == '\b' || ch == KEY_BACKSPACE) { 
                if (c->y + c->curser_pos_x + c->curser_pos_y != 0) {
                    n->left();
                    n->Delete();
                }
            } else if (ch == '\t') { // TAB - original code used this to SAVE and EXIT editor
                return c->buffer; 
            } else if (ch == 0x01) { // CTRL+A for command input
                mvaddstr(LINES - 1, 0, ""); 
                clrtoeol(); 
                setState("");                        
                char buf_cmd[100]; 
                unsigned short temp_color = get_cons_color(); 
                set_cons_color(0x70); 
                
                move(LINES - 1, 0);
                echo(); 
                getnstr(buf_cmd, 99); 
                noecho(); 
                
                set_cons_color(temp_color); 

                if (strncmp("to ", buf_cmd, 3) == 0) {
                    n->To(strtol(buf_cmd + 3, nullptr, 10));
                } else if (strcmp("stap", buf_cmd) == 0) {
                    flag = !flag;
                    setState(flag ? "Auto-indent ON " : "Auto-indent OFF"); 
                    getch(); 
                } else {
                    setState("Bad Command!   "); 
                    getch(); 
                }
                move(LINES -1, 0); clrtoeol();

            } else if (ch == KEY_UP) { 
                n->up();
            } else if (ch == KEY_DOWN) { 
                n->down();
            } else if (ch == KEY_LEFT) { 
                n->left();
            } else if (ch == KEY_RIGHT) { 
                n->right(1); 
            } else {
                if (isprint(ch)) { // Use isprint from <cctype> for printable characters
                    n->Insert(ch);
                    n->right(0); 
                }
            }
        }
        prse->Set();  
        r->showAll(); 
    }
  }
};

// Old mouse thread (Removed)
// void m_thread(void *s) {
//   Editor *b = (Editor *)s;
//   for (;;) {
//     int mouse = get_mouse();
//     if (GetMouse_btn(mouse) == CLICK_LEFT) {
//       b->Click(GetMouse_x(mouse), GetMouse_y(mouse));
//     } else if (GetMouse_btn(mouse) == 4) {
//       b->Up();
//     } else if (GetMouse_btn(mouse) == 5) {
//       b->Down();
//     }
//   }
// }
int mLine(char *buffer, int len) {
  int l = 0;
  for (int i = 0; i < len; i++) {
    if (buffer[i] == '\n') {
      l++;
    }
  }
  return l;
}
int main(int argc, char **argv) {
  if (argc == 1) {
    printf("Usage: %s <FileName>\n",argv[0]);
    return 0;
  }
  initscr();      // Start ncurses mode
  start_color();  // Start color functionality

  // Define Color Pairs
  if (has_colors()) {
      init_pair(1, COLOR_WHITE, COLOR_BLACK); // Normal text (DOS 0x07)
      init_pair(2, COLOR_BLACK, COLOR_WHITE); // Status bar (DOS 0x70)
      // Add more pairs as needed, e.g., for syntax highlighting later
      init_pair(3, COLOR_RED, COLOR_BLACK);
      init_pair(4, COLOR_GREEN, COLOR_BLACK);
      init_pair(5, COLOR_BLUE, COLOR_BLACK);
      init_pair(6, COLOR_CYAN, COLOR_BLACK);
      init_pair(7, COLOR_MAGENTA, COLOR_BLACK);
      init_pair(8, COLOR_YELLOW, COLOR_BLACK);
  }

  raw();          // Line buffering disabled, pass on evertything
  noecho();       // Don't echo() while we do getch
  keypad(stdscr, TRUE); // Enable Funtion keys, arrow keys etc.
  mousemask(ALL_MOUSE_EVENTS, NULL); // Listen for all mouse events
  curs_set(1);    // Set cursor to normal visibility initially


  max_line = LINES - 1; // Global variable
  max_char_a_line = COLS;   // Global variable

  // mst_obj == nullptr; // Commented out, it's a check not an assignment and mst_obj is already null
  char ext_str[100];
  size_t q; // Changed int to size_t
  for (q = 0; q < strlen(argv[1]); q++) {
    if (argv[1][q] == '.') {
      break;
    }
  }
  strcpy(ext_str, argv[1] + q);
  strtoupper(ext_str);
  if (filesize("/editor.mst") == -1) {
    printf("Warning: Couldn't find `editor.mst`.\n");
  } else {
  }
  printf("Powerint DOS Editor v0.2c\n");
  printf("We can help you write note(code)s in Powerint DOS\n");
  printf("Copyright (C) 2023 min0911_\n");
  printf("Build in %s %s\n", __DATE__, __TIME__);
  Editor *e = new Editor();
  char *c_buf = e->Main(argv[1]); // Renamed c to c_buf
  int len_c_buf = strlen(c_buf);  // Renamed l to len_c_buf
  editor_clear(); // Renamed from clear()
  char *bb = (char *)malloc(len_c_buf + 1 + mLine(c_buf, len_c_buf));
  for (int i = 0, j = 0; i < len_c_buf; i++) {
    if (c_buf[i] == '\n') {
      bb[j++] = '\r';
      bb[j++] = '\n';
    } else {
      bb[j++] = c_buf[i];
    }
  }

  editor_clear(); // Renamed from clear()
  Edit_File(argv[1], bb, len_c_buf + mLine(c_buf, len_c_buf), 0);
  endwin();       // End ncurses mode
  return 0;
}
