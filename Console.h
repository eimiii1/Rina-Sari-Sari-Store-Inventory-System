#ifndef CONSOLE_H
#define CONSOLE_H

#include <string>
#include <vector>

// Everything the UI layer needs: ANSI colors, terminal-width detection,
// centering, screen clearing, and a reusable bordered Table renderer.
// main.cpp is the only place that calls into this — it has zero
// knowledge of the inventory/sales logic (that stays in InventoryBST
// and CSVHandler), so swapping the look of the program never risks
// touching the data structures.
namespace Console {

// ---- colors --------------------------------------------------------
// Minimalist palette: structure (borders, headers, labels) stays
// monochrome gray/white so the UI doesn't look like a rainbow; color
// is spent only where it means something — a status, a warning, an
// error, a confirmed success.
extern const std::string RESET;
extern const std::string BOLD;
extern const std::string DIM;
extern const std::string BORDER;   // table/box lines — muted gray, not a loud accent
extern const std::string WHITE;    // primary text, headers, labels
extern const std::string GRAY;     // secondary/quiet text (codes, timestamps, units)
extern const std::string GREEN;    // success, profit, "OK" status
extern const std::string YELLOW;   // warnings, pending/attention state
extern const std::string RED;      // errors, low stock, destructive actions

// ---- terminal geometry ----------------------------------------------
void init();                      // call once at program start (enables ANSI colors on Windows consoles)
int width();                      // current terminal column count (falls back to 100)
void clear();                     // clears screen + moves cursor home
size_t visibleLength(const std::string& s); // length ignoring ANSI codes, UTF-8 aware

// ---- centered output -------------------------------------------------
void line(const std::string& text = "");     // one centered line, newline-terminated
void prompt(const std::string& text);        // centered, NO newline (input continues on same line)
void pause(const std::string& message = "Press Enter to continue...");

// ---- formatting helpers ------------------------------------------------
std::string money(double amount);             // "P1,234.50" with peso sign, 2 decimals
std::string title(const std::string& text);   // a bold double-ruled banner, pre-centered as a block

// ---- table ------------------------------------------------------------
// A column's cells can each carry their own color code (e.g. red for a
// low-stock quantity) without affecting the rest of the row.
class Table {
public:
    void setHeaders(const std::vector<std::string>& headers);

    // cellColors is optional; pass {} for no special coloring on this row.
    // When provided it must be the same size as cells (empty string = default).
    void addRow(const std::vector<std::string>& cells,
                const std::vector<std::string>& cellColors = {});

    void print() const;
    bool empty() const { return rows.empty(); }

private:
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
    std::vector<std::vector<std::string>> colors;
};

} // namespace Console

#endif
