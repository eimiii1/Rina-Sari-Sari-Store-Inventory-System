#include "Product.h"
#include "InventoryBST.h"
#include "CSVHandler.h"
#include "Console.h"
#include "RestockQueue.h"
#include "SaleHistoryStack.h"
#include "Algorithms.h"

#include <iostream>
#include <limits>
#include <ctime>
#include <sstream>
#include <cctype>

const std::string INVENTORY_FILE = "inventory.csv";
const std::string SALES_FILE = "sales.csv";

// ---- small input helpers --------------------------------------------
// Console programs crash easily on bad input (letters where a number
// is expected). These helpers keep asking until they get something
// usable instead of letting cin fail silently. Prompts are printed
// through Console::prompt() so they stay centered with everything else.

// If the input stream itself has closed (piped input ran out, terminal
// closed, Ctrl+D), cin.fail() is permanently true and clear()+retry would
// spin forever re-reading nothing. Exit cleanly instead of hanging/flooding
// output. (Data is safe either way — every menu action saves to CSV right
// after it runs, not just on exit.)
void exitOnClosedInput() {
    std::cout << "\n";
    Console::line(Console::YELLOW + "Input stream closed — exiting." + Console::RESET);
    std::exit(0);
}

int readInt(const std::string& promptText) {
    int value;
    while (true) {
        Console::prompt(promptText);
        std::cin >> value;
        if (std::cin.eof()) exitOnClosedInput();
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            Console::line(Console::RED + "Please enter a whole number." + Console::RESET);
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return value;
    }
}

double readDouble(const std::string& promptText) {
    double value;
    while (true) {
        Console::prompt(promptText);
        std::cin >> value;
        if (std::cin.eof()) exitOnClosedInput();
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            Console::line(Console::RED + "Please enter a number." + Console::RESET);
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return value;
    }
}

std::string readLine(const std::string& promptText) {
    std::string value;
    Console::prompt(promptText);
    std::getline(std::cin, value);
    return value;
}

std::string currentTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm* local = std::localtime(&now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", local);
    return std::string(buf);
}

// ---- shared chrome ----------------------------------------------------

void printHeader(const std::string& subtitle = "") {
    const std::string storeName = "Rina Sari-Sari Store";
    const std::string systemName = "Inventory & Sales System";

    std::string plainContent = "   " + storeName + "  " + systemName + "   ";
    std::string horizontal;
    for (size_t i = 0; i < plainContent.size(); i++) horizontal += "═";

    Console::line("");
    Console::line(Console::BOLD + Console::BORDER + "╔" + horizontal + "╗" + Console::RESET);
    Console::line(Console::BOLD + Console::BORDER + "║" + Console::RESET + "   " +
                   Console::BOLD + Console::WHITE + storeName + Console::RESET + "  " +
                   Console::WHITE + systemName + Console::RESET + "   " +
                   Console::BOLD + Console::BORDER + "║" + Console::RESET);
    Console::line(Console::BOLD + Console::BORDER + "╚" + horizontal + "╝" + Console::RESET);
    if (!subtitle.empty()) {
        Console::line(Console::DIM + Console::GRAY + subtitle + Console::RESET);
    }
    Console::line("");
}

void waitThenReturn() {
    Console::line("");
    Console::pause("Press Enter to return to the menu..."); // pause() already dims the message itself
}

// ---- rendering helpers ---------------------------------------------------

std::vector<std::string> productCells(const Product& p) {
    return {
        p.code,
        p.name,
        p.category,
        p.unit,
        std::to_string(p.quantity),
        Console::money(p.sellingPrice),
        p.isLowStock() ? "LOW STOCK" : "OK"
    };
}

std::vector<std::string> productColors(const Product& p) {
    // Minimalist rule: normal rows stay monochrome; color only shows up
    // to flag a problem (low stock), not to decorate every value.
    std::string qtyColor = p.isLowStock() ? Console::RED : Console::WHITE;
    std::string statusColor = p.isLowStock() ? (Console::BOLD + Console::RED) : (Console::DIM + Console::GRAY);
    return {
        Console::DIM + Console::GRAY,   // code
        Console::BOLD + Console::WHITE, // name
        Console::GRAY,                   // category
        Console::GRAY,                   // unit
        qtyColor,                         // quantity
        Console::WHITE,                   // price
        statusColor                       // status
    };
}

void printProductTable(const std::vector<Product>& products) {
    Console::Table table;
    table.setHeaders({"CODE", "NAME", "CATEGORY", "UNIT", "STOCK", "PRICE", "STATUS"});
    for (const auto& p : products) {
        table.addRow(productCells(p), productColors(p));
    }
    table.print();
}

// ---- menu actions ------------------------------------------------------

// Looks at every existing code of the form "P" + digits (e.g. P001, P002),
// finds the highest numeric suffix in use, and returns the next one with the
// same zero-padded width — so the catalog keeps growing P009, P010, P011...
// without the user ever having to type or track a code by hand.
std::string nextProductCode(InventoryBST& tree) {
    int highest = 0;
    size_t width = 3; // default padding width if the catalog is empty or has no "Pxxx" codes yet
    for (const auto& p : tree.getAllSorted()) {
        const std::string& code = p.code;
        if (code.size() < 2 || (code[0] != 'P' && code[0] != 'p')) continue;
        std::string digits = code.substr(1);
        bool allDigits = !digits.empty();
        for (char c : digits) if (!std::isdigit(static_cast<unsigned char>(c))) { allDigits = false; break; }
        if (!allDigits) continue;

        int value = std::stoi(digits);
        if (value > highest) {
            highest = value;
            width = digits.size();
        }
    }

    int next = highest + 1;
    std::string digits = std::to_string(next);
    if (digits.size() < width) digits = std::string(width - digits.size(), '0') + digits;
    return "P" + digits;
}

void addProduct(InventoryBST& tree, RestockQueue& queue) {
    printHeader("Add a new product to the catalog");

    Product p;
    p.code = nextProductCode(tree);
    Console::line(Console::GRAY + "Assigned product code: " + Console::RESET + Console::WHITE + p.code + Console::RESET);
    Console::line("");
    p.name = readLine("Name: ");

    if (tree.search(p.name)) {
        Console::line("");
        Console::line(Console::RED + "A product named \"" + p.name + "\" already exists. Use Edit instead." + Console::RESET);
        return;
    }

    p.category = readLine("Category: ");
    p.unit = readLine("Unit (pc, pack, bottle...): ");
    p.costPrice = readDouble("Cost price: ");
    p.sellingPrice = readDouble("Selling price: ");
    p.quantity = readInt("Starting quantity: ");
    p.reorderLevel = readInt("Reorder level (low-stock warning threshold): ");

    tree.insert(p);

    if (p.isLowStock() && !queue.contains(p.name)) {
        queue.enqueue({p.name, currentTimestamp()});
    }

    Console::line("");
    Console::line(Console::GREEN + "Added \"" + p.name + "\" to inventory." + Console::RESET);
}

void searchProduct(InventoryBST& tree) {
    printHeader("Search the catalog by product name");

    std::string name = readLine("Product name to search: ");
    Product* p = tree.search(name);
    Console::line("");

    if (!p) {
        Console::line(Console::RED + "No product named \"" + name + "\" was found." + Console::RESET);
        return;
    }

    printProductTable({*p});
    Console::line("");
    Console::line(Console::GRAY + "Cost price: " + Console::RESET + Console::WHITE + Console::money(p->costPrice) + Console::RESET +
                   "    " + Console::GRAY + "Margin per unit: " + Console::RESET + Console::GREEN + Console::money(p->margin()) + Console::RESET);
}

void listAllProducts(InventoryBST& tree) {
    printHeader("Full catalog, alphabetical (in-order BST traversal)");

    std::vector<Product> all = tree.getAllSorted();
    printProductTable(all);

    Console::line("");
    Console::line(Console::DIM + Console::GRAY + std::to_string(all.size()) + " product(s) total" + Console::RESET);
}

void lowStockReport(InventoryBST& tree) {
    printHeader("At or below reorder level, most urgent first (Bubble Sort)");

    std::vector<Product> low = tree.getLowStock();
    Algorithms::bubbleSortByQuantity(low); // ascending by quantity -> lowest stock shows first
    printProductTable(low);

    if (!low.empty()) {
        Console::line("");
        Console::line(Console::YELLOW + "Consider restocking these " + std::to_string(low.size()) + " item(s) soon." + Console::RESET);
    }
}

void updateStock(InventoryBST& tree, RestockQueue& queue) {
    printHeader("Restock or deduct stock for a product");

    std::string name = readLine("Product name: ");
    Product* p = tree.search(name);
    Console::line("");

    if (!p) {
        Console::line(Console::RED + "Not found." + Console::RESET);
        return;
    }

    Console::line(Console::GRAY + "Current stock: " + Console::RESET + Console::WHITE + std::to_string(p->quantity) + Console::RESET);
    int delta = readInt("Enter change (+10 to restock, -3 to deduct): ");

    if (!tree.updateStock(name, delta)) {
        Console::line("");
        Console::line(Console::RED + "That would take stock below zero. No change made." + Console::RESET);
        return;
    }

    // Keep the restock queue in sync: newly low -> queue it; restocked
    // back above the reorder level -> any pending request is resolved.
    if (p->isLowStock()) {
        if (!queue.contains(p->name)) queue.enqueue({p->name, currentTimestamp()});
    } else {
        queue.removeByName(p->name);
    }

    Console::line("");
    Console::line(Console::GREEN + "Updated. New stock: " + Console::RESET + Console::BOLD + Console::WHITE + std::to_string(p->quantity) + Console::RESET);
}

void editProduct(InventoryBST& tree, RestockQueue& queue) {
    printHeader("Edit pricing and reorder level for a product");

    std::string name = readLine("Product name to edit: ");
    Product* p = tree.search(name);
    Console::line("");

    if (!p) {
        Console::line(Console::RED + "Not found." + Console::RESET);
        return;
    }

    Console::line(Console::GRAY + "Current cost price: " + Console::RESET + Console::WHITE + Console::money(p->costPrice) + Console::RESET);
    double newCost = readDouble("New cost price: ");
    Console::line(Console::GRAY + "Current selling price: " + Console::RESET + Console::WHITE + Console::money(p->sellingPrice) + Console::RESET);
    double newSelling = readDouble("New selling price: ");
    Console::line(Console::GRAY + "Current reorder level: " + Console::RESET + std::to_string(p->reorderLevel));
    int newReorder = readInt("New reorder level: ");

    tree.updatePricing(name, newCost, newSelling, newReorder);

    // A changed reorder level can flip low-stock status without the
    // quantity itself moving at all, so re-check it here too.
    if (p->isLowStock()) {
        if (!queue.contains(p->name)) queue.enqueue({p->name, currentTimestamp()});
    } else {
        queue.removeByName(p->name);
    }

    Console::line("");
    Console::line(Console::GREEN + "Updated \"" + name + "\"." + Console::RESET);
}

void deleteProduct(InventoryBST& tree, RestockQueue& queue) {
    printHeader("Remove a product from the catalog");

    std::string name = readLine("Product name to delete: ");
    std::string confirm = readLine("Type the name again to confirm deletion: ");
    Console::line("");

    if (confirm != name) {
        Console::line(Console::YELLOW + "Names didn't match — cancelled." + Console::RESET);
        return;
    }
    if (tree.remove(name)) {
        queue.removeByName(name); // no point keeping a restock request for a product that no longer exists
        Console::line(Console::GREEN + "Deleted \"" + name + "\"." + Console::RESET);
    } else {
        Console::line(Console::RED + "Not found." + Console::RESET);
    }
}

void recordSale(InventoryBST& tree, RestockQueue& queue, SaleHistoryStack& history) {
    printHeader("Record a sale at the counter");

    std::string name = readLine("Product name: ");
    Product* p = tree.search(name);
    Console::line("");

    if (!p) {
        Console::line(Console::RED + "Not found." + Console::RESET);
        return;
    }

    Console::line(Console::GRAY + "In stock: " + Console::RESET + Console::WHITE + std::to_string(p->quantity) + Console::RESET +
                   Console::GRAY + "   Unit price: " + Console::RESET + Console::WHITE + Console::money(p->sellingPrice) + Console::RESET);
    int qty = readInt("Quantity sold: ");
    Console::line("");

    if (qty <= 0) { Console::line(Console::RED + "Quantity must be positive." + Console::RESET); return; }
    if (qty > p->quantity) {
        Console::line(Console::RED + "Not enough stock (have " + std::to_string(p->quantity) + ")." + Console::RESET);
        return;
    }

    double total = qty * p->sellingPrice;
    p->quantity -= qty;

    std::string timestamp = currentTimestamp(); // one timestamp, shared by the log line and the undo-stack entry
    CSVHandler::logSale(SALES_FILE, timestamp, p->name, qty, p->costPrice, p->sellingPrice, total);

    CSVHandler::SaleRecord record;
    record.timestamp = timestamp;
    record.productName = p->name;
    record.quantity = qty;
    record.unitCost = p->costPrice;
    record.unitPrice = p->sellingPrice;
    record.total = total;
    history.push(record);

    if (p->isLowStock() && !queue.contains(p->name)) {
        queue.enqueue({p->name, timestamp});
    }

    Console::Table receipt;
    receipt.setHeaders({"PRODUCT", "QTY", "UNIT PRICE", "TOTAL"});
    receipt.addRow(
        {p->name, std::to_string(qty), Console::money(p->sellingPrice), Console::money(total)},
        {Console::BOLD + Console::WHITE, Console::WHITE, Console::WHITE, Console::BOLD + Console::GREEN}
    );
    receipt.print();

    Console::line("");
    Console::line(Console::GREEN + "Sale recorded. " + Console::RESET + Console::GRAY + "Remaining stock: " + Console::RESET +
                   Console::WHITE + std::to_string(p->quantity) + Console::RESET);
}

void salesReport() {
    printHeader("Revenue and profit across all recorded sales");

    std::vector<CSVHandler::SaleRecord> sales = CSVHandler::loadSales(SALES_FILE);

    Console::Table table;
    table.setHeaders({"TIMESTAMP", "PRODUCT", "QTY", "TOTAL"});

    double totalRevenue = 0, totalProfit = 0;
    for (const auto& s : sales) {
        totalRevenue += s.total;
        totalProfit += (s.unitPrice - s.unitCost) * s.quantity;
        table.addRow(
            {s.timestamp, s.productName, std::to_string(s.quantity), Console::money(s.total)},
            {Console::GRAY, Console::WHITE, Console::WHITE, Console::WHITE}
        );
    }
    table.print();

    if (!sales.empty()) {
        Console::line("");
        Console::line(Console::GRAY + "Transactions: " + Console::RESET + Console::WHITE + std::to_string(sales.size()) + Console::RESET);
        Console::line(Console::GRAY + "Total revenue: " + Console::RESET + Console::WHITE + Console::money(totalRevenue) + Console::RESET);
        Console::line(Console::GRAY + "Total profit:  " + Console::RESET + Console::BOLD + Console::GREEN + Console::money(totalProfit) + Console::RESET);
    }
}

void viewRestockQueue(const RestockQueue& queue) {
    printHeader("Pending restock requests, oldest first (Queue — FIFO)");

    std::vector<RestockRequest> items = queue.listAll();

    Console::Table table;
    table.setHeaders({"PRODUCT", "REQUESTED AT"});
    for (const auto& r : items) {
        table.addRow({r.productName, r.requestedAt}, {Console::BOLD + Console::WHITE, Console::GRAY});
    }
    table.print();

    if (!items.empty()) {
        Console::line("");
        Console::line(Console::YELLOW + std::to_string(items.size()) + " item(s) waiting to be restocked." + Console::RESET);
    }
}

void processRestockRequest(InventoryBST& tree, RestockQueue& queue) {
    printHeader("Process the oldest pending restock request");

    RestockRequest request;
    if (!queue.dequeue(request)) {
        Console::line(Console::GREEN + "Nothing pending — every product is above its reorder level." + Console::RESET);
        return;
    }

    Console::line(Console::GRAY + "Next request: " + Console::RESET + Console::BOLD + Console::WHITE + request.productName + Console::RESET +
                   Console::GRAY + "  (requested " + request.requestedAt + ")" + Console::RESET);

    Product* p = tree.search(request.productName);
    if (!p) {
        Console::line("");
        Console::line(Console::YELLOW + "That product no longer exists in the catalog — request discarded." + Console::RESET);
        return;
    }

    Console::line(Console::GRAY + "Current stock: " + Console::RESET + Console::WHITE + std::to_string(p->quantity) + Console::RESET);
    int addQty = readInt("Quantity to restock: ");
    Console::line("");

    if (addQty <= 0) {
        Console::line(Console::YELLOW + "No stock added. The request has been cleared from the queue." + Console::RESET);
        return;
    }

    tree.updateStock(request.productName, addQty);
    Console::line(Console::GREEN + "Restocked. New stock: " + Console::RESET + Console::BOLD + Console::WHITE + std::to_string(p->quantity) + Console::RESET);

    // Might still be at/below reorder level even after this restock —
    // if so, put it back at the end of the line rather than losing track of it.
    if (p->isLowStock() && !queue.contains(p->name)) {
        queue.enqueue({p->name, currentTimestamp()});
        Console::line(Console::YELLOW + "Still at or below reorder level — re-added to the queue." + Console::RESET);
    }
}

void undoLastSale(InventoryBST& tree, RestockQueue& queue, SaleHistoryStack& history) {
    printHeader("Undo the most recent sale (Stack — LIFO)");

    CSVHandler::SaleRecord record;
    if (!history.pop(record)) {
        Console::line(Console::YELLOW + "No sales recorded yet to undo." + Console::RESET);
        return;
    }

    Product* p = tree.search(record.productName);
    if (p) {
        tree.updateStock(record.productName, record.quantity); // give the stock back
        if (!p->isLowStock()) queue.removeByName(p->name);
    }

    CSVHandler::removeLastSale(SALES_FILE);

    Console::Table table;
    table.setHeaders({"PRODUCT", "QTY", "TOTAL"});
    table.addRow(
        {record.productName, std::to_string(record.quantity), Console::money(record.total)},
        {Console::BOLD + Console::WHITE, Console::WHITE, Console::WHITE}
    );
    table.print();

    Console::line("");
    if (p) {
        Console::line(Console::GREEN + "Sale undone. Stock restored and removed from the sales log." + Console::RESET);
    } else {
        Console::line(Console::YELLOW + "Removed from the sales log, but \"" + record.productName +
                       "\" no longer exists, so stock could not be restored." + Console::RESET);
    }
}

void searchSalesHistory() {
    printHeader("Search past sales by product name (Linear Search)");

    std::string name = readLine("Product name: ");
    Console::line("");

    std::vector<CSVHandler::SaleRecord> all = CSVHandler::loadSales(SALES_FILE);
    std::vector<CSVHandler::SaleRecord> matches = Algorithms::linearSearchSales(all, name);

    Console::Table table;
    table.setHeaders({"TIMESTAMP", "PRODUCT", "QTY", "TOTAL"});
    for (const auto& s : matches) {
        table.addRow(
            {s.timestamp, s.productName, std::to_string(s.quantity), Console::money(s.total)},
            {Console::GRAY, Console::WHITE, Console::WHITE, Console::WHITE}
        );
    }
    table.print();

    Console::line("");
    if (matches.empty()) {
        Console::line(Console::YELLOW + "No past sales found for \"" + name + "\"." + Console::RESET);
        return;
    }

    int totalQty = 0;
    double totalAmount = 0;
    for (const auto& s : matches) { totalQty += s.quantity; totalAmount += s.total; }

    Console::line(Console::GRAY + "Matches: " + Console::RESET + Console::WHITE + std::to_string(matches.size()) + Console::RESET +
                   Console::GRAY + "   Total sold: " + Console::RESET + Console::WHITE + std::to_string(totalQty) + Console::RESET +
                   Console::GRAY + "   Total value: " + Console::RESET + Console::WHITE + Console::money(totalAmount) + Console::RESET);
}

void quickLookup(InventoryBST& tree) {
    printHeader("Quick lookup on the sorted catalog (Binary Search)");

    std::string name = readLine("Product name: ");
    Console::line("");

    // The BST's own search() would normally answer this; this path
    // takes its already-sorted output and re-finds the same product
    // with a textbook binary search instead, to demonstrate the
    // algorithm explicitly rather than relying on the tree's traversal.
    std::vector<Product> sorted = tree.getAllSorted();
    const Product* found = Algorithms::binarySearchProduct(sorted, name);

    if (!found) {
        Console::line(Console::RED + "No product named \"" + name + "\" was found." + Console::RESET);
        return;
    }
    printProductTable({*found});
}

void topSalesReport() {
    printHeader("All sales ranked by amount, highest first (Merge Sort)");

    std::vector<CSVHandler::SaleRecord> sales = CSVHandler::loadSales(SALES_FILE);
    Algorithms::mergeSortByTotal(sales);

    Console::Table table;
    table.setHeaders({"RANK", "PRODUCT", "QTY", "TOTAL"});
    for (size_t i = 0; i < sales.size(); i++) {
        table.addRow(
            {std::to_string(i + 1), sales[i].productName, std::to_string(sales[i].quantity), Console::money(sales[i].total)},
            {Console::GRAY, Console::BOLD + Console::WHITE, Console::WHITE, Console::WHITE}
        );
    }
    table.print();

    if (sales.empty()) {
        Console::line("");
        Console::line(Console::YELLOW + "No sales recorded yet." + Console::RESET);
    }
}

// ---- menu ---------------------------------------------------------------

struct MenuItem { std::string key; std::string label; };

const std::vector<MenuItem> MENU_ITEMS = {
    {"1", "Add new product"},
    {"2", "Search product"},
    {"3", "List all products (sorted)"},
    {"4", "Low stock report"},
    {"5", "Update stock (restock/deduct)"},
    {"6", "Edit product pricing"},
    {"7", "Delete product"},
    {"8", "Record a sale"},
    {"9", "Sales report"},
    {"10", "View restock queue"},
    {"11", "Process next restock request"},
    {"12", "Undo last sale"},
    {"13", "Search sales history (Linear Search)"},
    {"14", "Quick product lookup (Binary Search)"},
    {"15", "Top sales by amount (Merge Sort)"},
    {"0", "Save and exit"},
};

void printMenu(int productCount) {
    printHeader(std::to_string(productCount) + " product(s) currently tracked");

    Console::Table menu;
    menu.setHeaders({"#", "ACTION"});
    for (const auto& item : MENU_ITEMS) {
        std::string color = (item.key == "0") ? Console::RED : Console::WHITE;
        menu.addRow({item.key, item.label}, {Console::BOLD + Console::WHITE, color});
    }
    menu.print();
    Console::line("");
}

int main() {
    Console::init(); // must run before any colored output, or Windows consoles print raw escape codes

    InventoryBST tree;
    RestockQueue restockQueue;

    std::vector<Product> loaded = CSVHandler::loadInventory(INVENTORY_FILE);
    for (const auto& p : loaded) tree.insert(p);

    // Neither the queue nor the undo stack are persisted to disk — they're
    // working memory for the current session — so rebuild them from what's
    // already on record each time the program starts, rather than coming
    // up empty even though some products are already low or sales already
    // happened in a previous run.
    for (const auto& p : tree.getLowStock()) {
        restockQueue.enqueue({p.name, "at startup"});
    }

    std::vector<CSVHandler::SaleRecord> existingSales = CSVHandler::loadSales(SALES_FILE);
    SaleHistoryStack saleHistory(static_cast<int>(existingSales.size()) + 8);
    for (const auto& s : existingSales) saleHistory.push(s); // chronological order in the file -> most recent ends up on top

    bool running = true;
    while (running) {
        Console::clear();
        printMenu(tree.size());
        int choice = readInt("Choose an option: ");

        Console::clear();

        switch (choice) {
            case 1: addProduct(tree, restockQueue); break;
            case 2: searchProduct(tree); break;
            case 3: listAllProducts(tree); break;
            case 4: lowStockReport(tree); break;
            case 5: updateStock(tree, restockQueue); break;
            case 6: editProduct(tree, restockQueue); break;
            case 7: deleteProduct(tree, restockQueue); break;
            case 8: recordSale(tree, restockQueue, saleHistory); break;
            case 9: salesReport(); break;
            case 10: viewRestockQueue(restockQueue); break;
            case 11: processRestockRequest(tree, restockQueue); break;
            case 12: undoLastSale(tree, restockQueue, saleHistory); break;
            case 13: searchSalesHistory(); break;
            case 14: quickLookup(tree); break;
            case 15: topSalesReport(); break;
            case 0: {
                CSVHandler::saveInventory(INVENTORY_FILE, tree.getAllSorted());
                Console::clear();
                printHeader();
                Console::line(Console::GREEN + "Saved " + std::to_string(tree.size()) + " product(s) to " + INVENTORY_FILE + "." + Console::RESET);
                Console::line(Console::BOLD + Console::WHITE + "Goodbye!" + Console::RESET);
                Console::line("");
                running = false;
                break;
            }
            default:
                printHeader();
                Console::line(Console::RED + "Invalid option. Choose a number from the menu." + Console::RESET);
        }

        // Save after every action that can change inventory numbers, not
        // just on exit — so a crash or closed terminal never loses what
        // was just entered. Options 9, 10 are read-only reports/views.
        bool mutatesInventory = (choice >= 1 && choice <= 8) || choice == 11 || choice == 12;
        if (mutatesInventory) {
            CSVHandler::saveInventory(INVENTORY_FILE, tree.getAllSorted());
        }

        if (running) {
            waitThenReturn();
        }
    }

    return 0;
}
