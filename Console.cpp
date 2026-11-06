#include "Console.h"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif

namespace Console {

const std::string RESET   = "\033[0m";
const std::string BOLD    = "\033[1m";
const std::string DIM     = "\033[2m";
const std::string BORDER  = "\033[90m"; // muted gray — box lines shouldn't shout
const std::string WHITE   = "\033[97m";
const std::string GRAY    = "\033[37m";
const std::string GREEN   = "\033[32m"; // normal intensity, not neon-bright
const std::string YELLOW  = "\033[33m";
const std::string RED     = "\033[31m";

void init() {
#ifdef _WIN32
    // Two separate Windows console quirks to work around, not one:
    //
    // 1) conhost (classic PowerShell/cmd) doesn't interpret ANSI escape
    //    codes unless virtual-terminal processing is explicitly switched
    //    on for the process — without this, color codes are sent but
    //    ignored and the UI looks like plain text.
    //
    // 2) The box-drawing characters (─│┌┐ etc.) and the peso sign are
    //    UTF-8 multi-byte sequences in this source file, but conhost's
    //    *default* input/output code page is an old 8-bit one (437 or
    //    850), so it decodes those UTF-8 bytes one byte at a time and
    //    prints garbage ("ΓòÉ" instead of "─"). Switching the console
    //    to the UTF-8 code page fixes that independently of VT mode.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
#endif
    // No setup needed on Linux/macOS — real terminals there already
    // interpret ANSI codes without being asked.
}

int width() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        return info.srWindow.Right - info.srWindow.Left + 1;
    }
    return 100;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        return w.ws_col;
    }
    return 100; // sane fallback when stdout isn't a real tty (piped, redirected)
#endif
}

void clear() {
    // ANSI clear + cursor-home. Cheaper and more portable across
    // terminal emulators than shelling out to system("clear").
    std::cout << "\033[2J\033[H";
    std::cout.flush();
}

// Counts visible columns only: skips ANSI "\033[...m" escape sequences
// entirely, and treats each UTF-8 code point (not each byte) as one
// column. Without this, every colored or box-drawing line would be
// measured too long and centering would drift to one side.
size_t visibleLength(const std::string& s) {
    size_t len = 0;
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[i]);

        if (c == 0x1B) { // ESC — start of an ANSI escape sequence
            size_t j = i + 1;
            while (j < s.size() && s[j] != 'm') j++;
            i = j + 1;
            continue;
        }

        if ((c & 0x80) == 0)      i += 1; // 1-byte ASCII
        else if ((c & 0xE0) == 0xC0) i += 2; // 2-byte UTF-8
        else if ((c & 0xF0) == 0xE0) i += 3; // 3-byte UTF-8 (box-drawing, peso sign)
        else if ((c & 0xF8) == 0xF0) i += 4; // 4-byte UTF-8
        else i += 1;

        len++;
    }
    return len;
}

static void printPadded(const std::string& text) {
    int w = width();
    int pad = (w - static_cast<int>(visibleLength(text))) / 2;
    if (pad > 0) std::cout << std::string(pad, ' ');
    std::cout << text;
}

void line(const std::string& text) {
    printPadded(text);
    std::cout << "\n";
}

void prompt(const std::string& text) {
    printPadded(text);
    std::cout.flush(); // no newline — input is typed right after the prompt
}

void pause(const std::string& message) {
    line(DIM + message + RESET);
    std::string discard;
    std::getline(std::cin, discard);
}

std::string money(double amount) {
    std::ostringstream ss;
    ss << "₱" << std::fixed << std::setprecision(2) << amount; // peso sign
    return ss.str();
}

std::string title(const std::string& text) {
    // A simple centered label; the double-ruled banner box itself is
    // built in main.cpp with Console::line calls so it can sit right
    // under the program's ASCII title without this function owning
    // the whole multi-line layout.
    return BOLD + WHITE + text + RESET;
}

// ---- Table --------------------------------------------------------------

void Table::setHeaders(const std::vector<std::string>& h) {
    headers = h;
}

void Table::addRow(const std::vector<std::string>& cells, const std::vector<std::string>& cellColors) {
    rows.push_back(cells);
    colors.push_back(cellColors);
}

namespace {
    std::string buildBorder(const std::vector<size_t>& widths,
                             const std::string& left, const std::string& mid,
                             const std::string& right, const std::string& fill) {
        std::string s = left;
        for (size_t i = 0; i < widths.size(); i++) {
            for (size_t k = 0; k < widths[i]; k++) s += fill;
            s += (i + 1 < widths.size()) ? mid : right;
        }
        return s;
    }

    // Pads `text` out to innerWidth visible columns, then wraps it in
    // `color` (if any) with a RESET right after — so coloring one cell
    // never bleeds into the next.
    std::string buildCell(const std::string& text, size_t innerWidth, const std::string& color) {
        size_t visLen = Console::visibleLength(text);
        size_t padding = (visLen < innerWidth) ? (innerWidth - visLen) : 0;
        std::string padded = text + std::string(padding, ' ');
        if (color.empty()) return padded;
        return color + padded + Console::RESET;
    }

    std::string buildRow(const std::vector<std::string>& cells,
                          const std::vector<std::string>& cellColors,
                          const std::vector<size_t>& widths) {
        std::string s = Console::BORDER + "│" + Console::RESET; // │
        for (size_t i = 0; i < widths.size(); i++) {
            std::string text = (i < cells.size()) ? cells[i] : "";
            std::string color = (i < cellColors.size()) ? cellColors[i] : "";
            size_t innerWidth = widths[i] - 2; // minus the 1-space margin on each side
            s += " " + buildCell(text, innerWidth, color) + " ";
            s += Console::BORDER + "│" + Console::RESET; // │
        }
        return s;
    }
}

void Table::print() const {
    if (headers.empty()) return;

    size_t cols = headers.size();
    std::vector<size_t> widths(cols, 0);

    for (size_t i = 0; i < cols; i++) {
        widths[i] = Console::visibleLength(headers[i]);
    }
    for (const auto& row : rows) {
        for (size_t i = 0; i < cols && i < row.size(); i++) {
            widths[i] = std::max(widths[i], Console::visibleLength(row[i]));
        }
    }
    for (auto& w : widths) w += 2; // 1-space margin on each side of the cell text

    std::string top = buildBorder(widths, "┌", "┬", "┐", "─"); // ┌┬┐─
    std::string sep = buildBorder(widths, "├", "┼", "┤", "─"); // ├┼┤─
    std::string bot = buildBorder(widths, "└", "┴", "┘", "─"); // └┴┘─

    Console::line(Console::BORDER + top + Console::RESET);

    std::vector<std::string> headerColors(cols, Console::BOLD + Console::WHITE);
    Console::line(buildRow(headers, headerColors, widths));

    Console::line(Console::BORDER + sep + Console::RESET);

    if (rows.empty()) {
        // A merged, centered "(no data)" row spanning the full table width,
        // rather than trying to fit it into the first column only.
        size_t innerSpan = cols - 1; // one column of internal border width per gap, replaced by plain space
        for (auto w : widths) innerSpan += w;
        size_t textWidth = innerSpan - 2; // minus the 1-space margin on each side

        std::string msg = "(no data)";
        size_t msgLen = Console::visibleLength(msg);
        size_t pad = (msgLen < textWidth) ? (textWidth - msgLen) : 0;
        size_t leftPad = pad / 2;
        size_t rightPad = pad - leftPad;

        std::string content = std::string(leftPad, ' ') + Console::DIM + msg + Console::RESET + std::string(rightPad, ' ');
        Console::line(Console::BORDER + "│" + Console::RESET + " " + content + " " + Console::BORDER + "│" + Console::RESET);
    }

    for (size_t r = 0; r < rows.size(); r++) {
        Console::line(buildRow(rows[r], colors[r], widths));
    }

    Console::line(Console::BORDER + bot + Console::RESET);
}

} // namespace Console
