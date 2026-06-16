#include "AUILib.h"
#include <random>
#include <iomanip>
#include <sstream>

using namespace aui;

// Helper: generate random string
std::string random_string(size_t len) {
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);
    std::string str;
    for (size_t i = 0; i < len; ++i)
        str += alphanum[dis(gen)];
    return str;
}

int main() {
    AUI *au = AUI::Create("Table Demo");
    AWindow *w = au->MainWnd();
    w->EnableResize();
    w->Resize(900, 700);
    w->SetBGColor(0xFFF0F0F0);
    // Create a container box with a border for visual separation
    ABox *box = ABox::AttachTo(w);
    box->Move(20, 20);
    box->Resize(860, 660);
    box->SetBGColor(0xFFFFFFFF);
    box->SetBorderThickness(2);
    box->SetBorderColor(0xFFAAAAAA);
    // Create the table
    ATable *table = ATable::AttachTo(box);
    table->Move(10, 10);
    table->Resize(840, 640);
    table->SetBGColor(0xFFFFFFFF);
    table->SetGridColor(0xFFDDDDDD);
    table->SetHeaderBgColor(0xFFE8E8E8);
    table->SetHeaderTextColor(0xFF000000);
    table->SetSelectionColor(0xCCE5FF);   // light blue selection
    table->SetCursorBorderColor(0xFF3399FF);
    table->SetFontSize(12);
    table->SetAutoWiden(true);
    table->SetRowSelectMode(true);        // select entire rows
    // Define columns: 8 columns with custom labels and widths
    const int numCols = 8;
    std::vector<std::string> colLabels = {
        "ID", "Product", "Category", "Price ($)", "Stock", "Rating", "Last Sale", "Country"
    };
    for (int32_t i = 0; i < numCols; ++i) {
        table->AddColumn();
        table->SetColumnLabel(i, colLabels[(size_t)i]);
        // Set custom widths for some columns
        if (i == 1) table->SetColumnWidth(i, 150);  // Product wider
        else if (i == 2) table->SetColumnWidth(i, 100);
        else if (i == 3) table->SetColumnWidth(i, 80);
        else if (i == 6) table->SetColumnWidth(i, 100);
        else table->SetColumnWidth(i, 70);
    }
    // Add 30 rows with fancy data
    const int numRows = 100;
    std::vector<std::string> categories = {"Electronics", "Clothing", "Books", "Toys", "Food"};
    std::vector<std::string> countries = {"USA", "Germany", "Japan", "France", "UK", "China"};
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> priceDist(10, 999);
    std::uniform_int_distribution<> stockDist(0, 500);
    std::uniform_real_distribution<> ratingDist(1.0, 5.0);
    std::uniform_int_distribution<> catDist(0, static_cast<int>(categories.size() - 1));
    std::uniform_int_distribution<> countryDist(0, static_cast<int>(countries.size() - 1));
    for (int row = 0; row < numRows; ++row) {
        table->AddRow();
        // Row label (optional)
        table->SetRowLabel(row, "R" + std::to_string(row + 1));
        // Generate random product name
        std::string product = random_string(8);
        // Fill cells
        table->SetCellData(row, 0, std::to_string(row + 1));
        table->SetCellData(row, 1, product);
        table->SetCellData(row, 2, categories[(size_t)catDist(gen)]);
        double price = priceDist(gen) / 100.0;
        std::stringstream price_ss;
        price_ss << std::fixed << std::setprecision(2) << price;
        table->SetCellData(row, 3, price_ss.str());
        table->SetCellData(row, 4, std::to_string(stockDist(gen)));
        double rating = ratingDist(gen);
        std::stringstream rating_ss;
        rating_ss << std::fixed << std::setprecision(1) << rating;
        table->SetCellData(row, 5, rating_ss.str());
        // Last sale date – random day in 2024
        int day = 1 + (rand() % 365);
        table->SetCellData(row, 6, "2024-" + std::to_string(day / 30 + 1) + "-" + std::to_string(day % 28 + 1));
        table->SetCellData(row, 7, countries[(size_t)countryDist(gen)]);
    }
    // Apply some conditional formatting: color cells based on stock level (low = red background)
    // This would require custom cell rendering, but we can at least set selection to show something.
    // For a "fancy" touch, we'll add a click callback that shows product details in a label.
    AList *logList = AList::AttachTo(box);
    logList->Move(10, 480);
    logList->Resize(810, 150);
    logList->SetBGColor(0xFFF9F9F9);
    logList->SetBorderThickness(1);
    logList->SetBorderColor(0xFFCCCCCC);
    logList->AddItem("Click on a row to see details here...");
    // Set up a click handler on the table (using the base widget callback)
    // Note: ATable's OnMouseClick already handles selection; we can add a callback via SetClickCallback
    table->SetClickCallback([](AWindow*, AWidget* widget, void* userData, int32_t, int32_t, bool pressed) {
        if (!pressed) return;
        ATable *tbl = static_cast<ATable*>(widget);
        AList *log = static_cast<AList*>(userData);
        int64_t row = tbl->GetSelectedRow();
        if (row >= 0) {
            std::string product = tbl->GetCellData(row, 1);
            std::string price = tbl->GetCellData(row, 3);
            std::string stock = tbl->GetCellData(row, 4);
            std::string msg = "Selected: " + product + " | Price: $" + price + " | Stock: " + stock;
            log->AddItem(msg);
            // Keep only last 10 messages to avoid clutter
            while (log->GetItemCount() > 10) log->RemoveItem(0);
        }
    }, logList);
    // Enable scrollbars (they will appear automatically when content exceeds view)
    table->SetScrollbarsEnabled(true);
    // Auto-fit columns (already set AutoWiden, but we can call manually if needed)
    for (int i = 0; i < numCols; ++i) {
        table->AutoWidenColumn(i);
    }
    // Scroll to a specific cell (e.g., row 5, col 2) just for demo
    table->ScrollToCell(5, 2);
    // Process events and keep window open
    au->ProcessMessages();

    delete au;
    return 0;
}


