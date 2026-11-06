#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
using namespace std;

const int MAX = 500;

struct Product {
    string id, name, brand, category, packaging, expiry;
    double price;
    int stock, minimum;
};

struct Sale {
    string date, productId, name, category;
    int quantity;
    double total;
};

struct Withdrawal {
    string date, person, reason;
    double amount;
};

Product products[MAX];
Sale sales[MAX];
Withdrawal withdrawals[MAX];

int productCount = 0;
int saleCount = 0;
int withdrawalCount = 0;


// ---------- INPUT HELPERS ----------

string readText(string message) {
    string text;

    do {
        cout << message;
        getline(cin >> ws, text);
    } while (text.empty());

    return text;
}

int readInt(string message, int minimum = 0) {
    int value;

    while (true) {
        cout << message;

        if (cin >> value && value >= minimum) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }

        cout << "Invalid input. Try again.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

double readMoney(string message, double minimum = 0) {
    double value;

    while (true) {
        cout << message;

        if (cin >> value && value >= minimum) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }

        cout << "Invalid amount. Try again.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}


// ---------- SEARCH ALGORITHMS ----------

int linearSearch(string id) {
    for (int i = 0; i < productCount; i++) {
        if (products[i].id == id)
            return i;
    }

    return -1;
}


// ---------- SORTING ALGORITHMS ----------

// Bubble Sort: product name
void bubbleSort() {
    for (int i = 0; i < productCount - 1; i++) {
        for (int j = 0; j < productCount - i - 1; j++) {
            if (products[j].name > products[j + 1].name) {
                Product temp = products[j];
                products[j] = products[j + 1];
                products[j + 1] = temp;
            }
        }
    }
}

// Merge Sort: product ID
void merge(int left, int middle, int right) {
    Product temp[MAX];

    int i = left;
    int j = middle + 1;
    int k = left;

    while (i <= middle && j <= right) {
        if (products[i].id <= products[j].id)
            temp[k++] = products[i++];
        else
            temp[k++] = products[j++];
    }

    while (i <= middle)
        temp[k++] = products[i++];

    while (j <= right)
        temp[k++] = products[j++];

    for (int index = left; index <= right; index++)
        products[index] = temp[index];
}

void mergeSort(int left, int right) {
    if (left >= right)
        return;

    int middle = left + (right - left) / 2;

    mergeSort(left, middle);
    mergeSort(middle + 1, right);
    merge(left, middle, right);
}

int binarySearch(string id) {
    // Binary Search requires records sorted by ID.
    mergeSort(0, productCount - 1);

    int left = 0;
    int right = productCount - 1;

    while (left <= right) {
        int middle = left + (right - left) / 2;

        if (products[middle].id == id)
            return middle;

        if (products[middle].id < id)
            left = middle + 1;
        else
            right = middle - 1;
    }

    return -1;
}


// ---------- DISPLAY ----------

void showProduct(int index) {
    Product p = products[index];

    cout << "\nID: " << p.id
         << "\nName: " << p.name
         << "\nBrand: " << p.brand
         << "\nCategory: " << p.category
         << "\nPackaging: " << p.packaging
         << "\nPrice: PHP " << p.price
         << "\nStock: " << p.stock
         << "\nMinimum stock: " << p.minimum
         << "\nExpiration: " << p.expiry << "\n";
}

void viewProducts() {
    if (productCount == 0) {
        cout << "No products yet.\n";
        return;
    }

    cout << "\n"
         << left << setw(12) << "ID"
         << setw(25) << "Product"
         << right << setw(12) << "Price"
         << setw(10) << "Stock" << "\n";

    cout << string(59, '-') << "\n";

    for (int i = 0; i < productCount; i++) {
        cout << left << setw(12) << products[i].id
             << setw(25) << products[i].name.substr(0, 24)
             << right << setw(12) << products[i].price
             << setw(10) << products[i].stock << "\n";
    }
}


// ---------- PRODUCT MANAGEMENT ----------

void addProduct() {
    if (productCount == MAX) {
        cout << "Product storage is full.\n";
        return;
    }

    Product p;

    // Automatically generate the product ID.
    p.id = "P" + to_string(productCount + 1);

    cout << "Generated Product ID: " << p.id << "\n";

    p.name = readText("Product name: ");
    p.brand = readText("Brand: ");
    p.category = readText("Category: ");
    p.packaging = readText("Packaging / size: ");
    p.expiry = readText("Expiry (YYYY-MM-DD or N/A): ");
    p.price = readMoney("Selling price: ", 0.01);
    p.stock = readInt("Stock quantity: ");
    p.minimum = readInt("Minimum stock: ");

    products[productCount++] = p;

    cout << "Product added successfully.\n";
}

void updateProduct() {
    int index = linearSearch(readText("Product ID: "));

    if (index == -1) {
        cout << "Product not found.\n";
        return;
    }

    showProduct(index);

    cout << "\n1. Name\n2. Price\n3. Category"
         << "\n4. Minimum stock\n5. Expiry\n";

    int choice = readInt("Choose: ", 1);

    switch (choice) {
        case 1:
            products[index].name = readText("New name: ");
            break;
        case 2:
            products[index].price = readMoney("New price: ", 0.01);
            break;
        case 3:
            products[index].category = readText("New category: ");
            break;
        case 4:
            products[index].minimum = readInt("New minimum stock: ");
            break;
        case 5:
            products[index].expiry = readText("New expiry: ");
            break;
        default:
            cout << "Invalid choice.\n";
            return;
    }

    cout << "Product updated.\n";
}


// ---------- SALES MANAGEMENT ----------

void recordSale() {
    if (saleCount == MAX) {
        cout << "Sales storage is full.\n";
        return;
    }

    int index = linearSearch(readText("Product ID: "));

    if (index == -1) {
        cout << "Product not found.\n";
        return;
    }

    cout << "Product: " << products[index].name
         << "\nPrice: PHP " << products[index].price
         << "\nAvailable stock: " << products[index].stock << "\n";

    int quantity = readInt("Quantity: ", 1);

    if (quantity > products[index].stock) {
        cout << "Not enough stock.\n";
        return;
    }

    string date = readText("Date (YYYY-MM-DD): ");
    double total = products[index].price * quantity;

    cout << "Total: PHP " << total << "\n";

    string confirm = readText("Confirm sale? (Y/N): ");

    if (confirm != "Y" && confirm != "y") {
        cout << "Sale cancelled.\n";
        return;
    }

    double payment = readMoney("Cash received: ", total);

    Sale s;
    s.date = date;
    s.productId = products[index].id;
    s.name = products[index].name;
    s.category = products[index].category;
    s.quantity = quantity;
    s.total = total;

    sales[saleCount++] = s;
    products[index].stock -= quantity;

    cout << "Change: PHP " << payment - total
         << "\nSale recorded.\n";
}


// ---------- INVENTORY ----------

void restockProduct() {
    int index = linearSearch(readText("Product ID: "));

    if (index == -1) {
        cout << "Product not found.\n";
        return;
    }

    int quantity = readInt("Quantity received: ", 1);
    products[index].stock += quantity;

    cout << "New stock: " << products[index].stock << "\n";
}

void lowStock() {
    bool found = false;

    cout << "\n--- LOW-STOCK PRODUCTS ---\n";

    for (int i = 0; i < productCount; i++) {
        if (products[i].stock <= products[i].minimum) {
            cout << products[i].id << " - "
                 << products[i].name << " | Stock: "
                 << products[i].stock << "\n";
            found = true;
        }
    }

    if (!found)
        cout << "No low-stock products.\n";
}

void removeDamagedStock() {
    int index = linearSearch(readText("Product ID: "));

    if (index == -1) {
        cout << "Product not found.\n";
        return;
    }

    int quantity = readInt("Damaged / expired quantity: ", 1);

    if (quantity > products[index].stock) {
        cout << "Quantity exceeds available stock.\n";
        return;
    }

    products[index].stock -= quantity;

    cout << "Stock adjusted. Remaining: "
         << products[index].stock << "\n";
}


// ---------- CASH WITHDRAWALS ----------

void recordWithdrawal() {
    if (withdrawalCount == MAX) {
        cout << "Withdrawal storage is full.\n";
        return;
    }

    Withdrawal w;

    w.date = readText("Date (YYYY-MM-DD): ");
    w.person = readText("Person responsible: ");
    w.reason = readText("Reason: ");
    w.amount = readMoney("Amount: ", 0.01);

    withdrawals[withdrawalCount++] = w;

    cout << "Withdrawal recorded.\n";
}


// ---------- REPORTS ----------

void salesReport() {
    cout << "\nUse YYYY-MM-DD for a daily report."
         << "\nUse YYYY-MM for a monthly report.\n";

    string period = readText("Report date / month: ");

    if (period.length() != 7 && period.length() != 10) {
        cout << "Use YYYY-MM or YYYY-MM-DD.\n";
        return;
    }

    double totalSales = 0;
    double totalWithdrawals = 0;
    int transactions = 0;
    int itemsSold = 0;

    cout << "\n--- SALES ---\n";

    for (int i = 0; i < saleCount; i++) {
        if (sales[i].date.substr(0, period.length()) == period) {
            cout << sales[i].date << " | "
                 << sales[i].name << " | Qty: "
                 << sales[i].quantity << " | PHP "
                 << sales[i].total << "\n";

            totalSales += sales[i].total;
            itemsSold += sales[i].quantity;
            transactions++;
        }
    }

    cout << "\n--- WITHDRAWALS ---\n";

    for (int i = 0; i < withdrawalCount; i++) {
        if (withdrawals[i].date.substr(0, period.length()) == period) {
            cout << withdrawals[i].date << " | "
                 << withdrawals[i].person << " | PHP "
                 << withdrawals[i].amount << " | "
                 << withdrawals[i].reason << "\n";

            totalWithdrawals += withdrawals[i].amount;
        }
    }

    cout << "\nTransactions: " << transactions
         << "\nItems sold: " << itemsSold
         << "\nTotal sales: PHP " << totalSales
         << "\nTotal withdrawals: PHP " << totalWithdrawals
         << "\nSales minus withdrawals: PHP "
         << totalSales - totalWithdrawals << "\n";

    cout << "This is not profit or the actual cash drawer balance.\n";
}


// ---------- SEARCH AND SORT MENUS ----------

void searchProduct() {
    cout << "\n1. Linear Search\n2. Binary Search\n";

    int choice = readInt("Choose: ", 1);

    if (choice != 1 && choice != 2) {
        cout << "Invalid choice.\n";
        return;
    }

    string id = readText("Product ID: ");
    int index;

    if (choice == 1)
        index = linearSearch(id);
    else
        index = binarySearch(id);

    if (index == -1)
        cout << "Product not found.\n";
    else
        showProduct(index);
}

void sortProducts() {
    cout << "\n1. Bubble Sort by name"
         << "\n2. Merge Sort by product ID\n";

    int choice = readInt("Choose: ", 1);

    if (choice == 1)
        bubbleSort();
    else if (choice == 2)
        mergeSort(0, productCount - 1);
    else {
        cout << "Invalid choice.\n";
        return;
    }

    viewProducts();
}


// ---------- MAIN MENU ----------

int main() {
    cout << fixed << setprecision(2);

    int choice;

    do {
        cout << "\n=================================="
             << "\n     RANJO'S SARI-SARI STORE"
             << "\n   SALES AND INVENTORY SYSTEM"
             << "\n=================================="
             << "\n1.  Add product"
             << "\n2.  View inventory"
             << "\n3.  Update product"
             << "\n4.  Record sale"
             << "\n5.  Restock product"
             << "\n6.  View low-stock products"
             << "\n7.  Remove damaged / expired stock"
             << "\n8.  Record cash withdrawal"
             << "\n9.  Sales and withdrawal report"
             << "\n10. Search product"
             << "\n11. Sort products"
             << "\n0.  Exit\n";

        choice = readInt("Choose: ");

        switch (choice) {
            case 1: addProduct(); break;
            case 2: viewProducts(); break;
            case 3: updateProduct(); break;
            case 4: recordSale(); break;
            case 5: restockProduct(); break;
            case 6: lowStock(); break;
            case 7: removeDamagedStock(); break;
            case 8: recordWithdrawal(); break;
            case 9: salesReport(); break;
            case 10: searchProduct(); break;
            case 11: sortProducts(); break;
            case 0:
                cout << "Goodbye! Records will not be saved.\n";
                break;
            default:
                cout << "Invalid choice.\n";
        }

    } while (choice != 0);

    return 0;
}