// market_bazaar_management.cpp
// Standalone MARKET / BAZAAR MANAGEMENT MODULE
// DS USED: vector, set, unordered_map, priority_queue
// CSV-backed, menu-driven, independent execution
// Compile: g++ -std=gnu++17 market_bazaar_management.cpp -o market

#include <bits/stdc++.h>
using namespace std;

/* ================= CSV HELPERS ================= */

vector<vector<string>> readCSV(const string &fname) {
    vector<vector<string>> rows;
    ifstream in(fname);
    if (!in.is_open()) return rows;
    string line;
    while (getline(in, line)) {
        vector<string> cols;
        string cur;
        bool inquote = false;
        for (char c : line) {
            if (c == '"') inquote = !inquote;
            else if (c == ',' && !inquote) {
                cols.push_back(cur);
                cur.clear();
            } else cur.push_back(c);
        }
        cols.push_back(cur);
        rows.push_back(cols);
    }
    in.close();
    return rows;
}

void appendCSV(const string &fname, const vector<string> &row) {
    ofstream out(fname, ios::app);
    for (size_t i = 0; i < row.size(); i++) {
        if (row[i].find(',') != string::npos)
            out << '"' << row[i] << '"';
        else out << row[i];
        if (i + 1 < row.size()) out << ',';
    }
    out << '\n';
}

void overwriteCSV(const string &fname, const vector<vector<string>> &rows) {
    ofstream out(fname, ios::trunc);
    for (auto &r : rows) {
        for (size_t i = 0; i < r.size(); i++) {
            if (r[i].find(',') != string::npos)
                out << '"' << r[i] << '"';
            else out << r[i];
            if (i + 1 < r.size()) out << ',';
        }
        out << '\n';
    }
}

/* ================= TIME HELPERS ================= */

string nowDate() {
    time_t t = time(nullptr);
    tm *lt = localtime(&t);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d", lt);
    return string(buf);
}

string nowTimeHHMM() {
    time_t t = time(nullptr);
    tm *lt = localtime(&t);
    char buf[10];
    strftime(buf, sizeof(buf), "%H:%M", lt);
    return string(buf);
}

double parseDoubleSafe(const string &s) {
    try { return stod(s); }
    catch (...) { return 0.0; }
}

/* ================= FILE NAMES ================= */

const string BAZ_PRODUCTS_FILE = "baz_products.csv";
const string BAZ_VENDOR_FILE   = "baz_vendors.csv";
const string BAZ_SALES_FILE    = "baz_sales.csv";
const string BAZ_RESTOCK_FILE  = "baz_restock.csv";

/* ================= DATA STRUCTURES ================= */

set<string> baz_productIds;
unordered_map<string,int> baz_productIndex;
set<string> baz_vendorIds;
priority_queue<pair<int,string>, vector<pair<int,string>>, greater<pair<int,string>>> baz_lowStockPQ;

/* ================= BUILD HELPERS ================= */

void baz_buildProductIndex() {
    baz_productIds.clear();
    baz_productIndex.clear();
    while (!baz_lowStockPQ.empty()) baz_lowStockPQ.pop();

    auto rows = readCSV(BAZ_PRODUCTS_FILE);
    for (size_t i = 1; i < rows.size(); i++) {
        baz_productIds.insert(rows[i][0]);
        baz_productIndex[rows[i][0]] = i;
        int qty = stoi(rows[i][5]);
        baz_lowStockPQ.push({qty, rows[i][0]});
    }
}

void baz_buildVendorSet() {
    baz_vendorIds.clear();
    auto rows = readCSV(BAZ_VENDOR_FILE);
    for (size_t i = 1; i < rows.size(); i++)
        baz_vendorIds.insert(rows[i][0]);
}

/* ================= MODULE FUNCTIONS ================= */

void baz_addProduct() {
    string id,name,cat,buy,sell,qty,unit,notes;
    cout << "Product ID: "; getline(cin,id);
    baz_buildProductIndex();
    if (baz_productIds.count(id)) {
        cout << "Product ID already exists.\n";
        return;
    }
    cout << "Name: "; getline(cin,name);
    cout << "Category: "; getline(cin,cat);
    cout << "Buy Price: "; getline(cin,buy);
    cout << "Sell Price: "; getline(cin,sell);
    cout << "Quantity: "; getline(cin,qty);
    cout << "Unit (kg/pcs): "; getline(cin,unit);
    cout << "Notes: "; getline(cin,notes);
    appendCSV(BAZ_PRODUCTS_FILE,{id,name,cat,buy,sell,qty,unit,notes});
    cout << "Product added.\n";
}

void baz_viewProducts() {
    auto rows = readCSV(BAZ_PRODUCTS_FILE);
    if (rows.size() <= 1) { cout << "No products.\n"; return; }
    cout << "\n=== MARKET PRODUCTS ===\n";
    for (size_t i=1;i<rows.size();i++)
        cout << rows[i][0] << " | " << rows[i][1] << " | " << rows[i][2]
             << " | Buy:" << rows[i][3] << " | Sell:" << rows[i][4]
             << " | Qty:" << rows[i][5] << " " << rows[i][6] << '\n';
}

void baz_sellProduct() {
    string sid,pid,qtyStr;
    cout << "Sale ID: "; getline(cin,sid);
    cout << "Product ID: "; getline(cin,pid);
    cout << "Quantity: "; getline(cin,qtyStr);
    int qty = stoi(qtyStr);
    auto rows = readCSV(BAZ_PRODUCTS_FILE);
    baz_buildProductIndex();
    if (!baz_productIndex.count(pid)) {
        cout << "Product not found.\n";
        return;
    }
    int i = baz_productIndex[pid];
    int stock = stoi(rows[i][5]);
    if (stock < qty) { cout << "Not enough stock.\n"; return; }
    double sellPrice = parseDoubleSafe(rows[i][4]);
    rows[i][5] = to_string(stock - qty);
    overwriteCSV(BAZ_PRODUCTS_FILE, rows);
    double total = sellPrice * qty;
    appendCSV(BAZ_SALES_FILE,{sid,pid,to_string(qty),to_string(sellPrice),to_string(total),nowDate(),nowTimeHHMM()});
    cout << "Sale recorded. Total: " << total << '\n';
}

void baz_viewSales() {
    auto rows = readCSV(BAZ_SALES_FILE);
    cout << "\n=== MARKET SALES ===\n";
    for (size_t i=1;i<rows.size();i++)
        cout << rows[i][0] << " | Prod:" << rows[i][1]
             << " | Qty:" << rows[i][2]
             << " | Each:" << rows[i][3]
             << " | Total:" << rows[i][4]
             << " | " << rows[i][5] << " " << rows[i][6] << '\n';
}

void baz_addVendor() {
    string vid,name,phone,items,notes;
    cout << "Vendor ID: "; getline(cin,vid);
    baz_buildVendorSet();
    if (baz_vendorIds.count(vid)) { cout << "Vendor already exists.\n"; return; }
    cout << "Name: "; getline(cin,name);
    cout << "Phone: "; getline(cin,phone);
    cout << "Supply Items: "; getline(cin,items);
    cout << "Notes: "; getline(cin,notes);
    appendCSV(BAZ_VENDOR_FILE,{vid,name,phone,items,notes});
    cout << "Vendor added.\n";
}

void baz_viewVendors() {
    auto rows = readCSV(BAZ_VENDOR_FILE);
    cout << "\n=== VENDORS LIST ===\n";
    for (size_t i=1;i<rows.size();i++)
        cout << rows[i][0] << " | " << rows[i][1]
             << " | " << rows[i][2]
             << " | Supplies: " << rows[i][3] << '\n';
}

void baz_restockProduct() {
    string rid,pid,qtyStr,vid;
    cout << "Restock ID: "; getline(cin,rid);
    cout << "Product ID: "; getline(cin,pid);
    cout << "Quantity Added: "; getline(cin,qtyStr);
    cout << "Vendor ID: "; getline(cin,vid);
    int qty = stoi(qtyStr);
    auto rows = readCSV(BAZ_PRODUCTS_FILE);
    baz_buildProductIndex();
    if (!baz_productIndex.count(pid)) { cout << "Product not found.\n"; return; }
    int i = baz_productIndex[pid];
    rows[i][5] = to_string(stoi(rows[i][5]) + qty);
    overwriteCSV(BAZ_PRODUCTS_FILE, rows);
    appendCSV(BAZ_RESTOCK_FILE,{rid,pid,to_string(qty),vid,nowDate()});
    cout << "Restock recorded.\n";
}

void baz_loadSampleData() {
    cout << "Overwrite Market CSV data? (yes/no): ";
    string ans; getline(cin,ans);
    transform(ans.begin(),ans.end(),ans.begin(),::tolower);
    if (ans != "yes") { cout << "Aborted.\n"; return; }

    vector<vector<string>> prod = {
        {"pid","name","category","buy","sell","qty","unit","notes"},
        {"P001","Tomatoes","Vegetable","20","30","50","kg","Fresh"},
        {"P002","Potatoes","Vegetable","18","25","80","kg","Organic"},
        {"P003","Onions","Vegetable","22","30","70","kg","Red onions"},
        {"P004","Apples","Fruit","120","150","40","kg","Kashmiri"},
        {"P005","Bananas","Fruit","25","35","60","kg","Yellow"},
        {"P006","Milk 1L","Dairy","45","55","100","pcs","Toned"},
        {"P007","Curd 500g","Dairy","25","35","80","pcs","Fresh"},
        {"P008","Rice 1kg","Grains","50","70","90","kg","Basmati"},
        {"P009","Wheat 1kg","Grains","35","50","110","kg","MP wheat"},
        {"P010","Sugar 1kg","Grocery","36","45","100","kg","Refined"},
        {"P011","Tea Powder 250g","Grocery","90","120","40","pcs","Premium"},
        {"P012","Sunflower Oil 1L","Grocery","90","110","55","pcs","Refined"},
        {"P013","Soap Bar","Daily Use","18","25","120","pcs","Herbal"},
        {"P014","Shampoo Sachet","Daily Use","2","5","200","pcs","Popular"},
        {"P015","Toothpaste 100g","Daily Use","35","50","90","pcs","Mint"},
        {"P016","Detergent 1kg","Cleaning","60","80","70","pcs","Bucket wash"},
        {"P017","Floor Cleaner 1L","Cleaning","70","90","40","pcs","Floral"},
        {"P018","Chocolates","Snacks","10","15","200","pcs","Mini bars"},
        {"P019","Biscuits","Snacks","15","25","160","pcs","Tea biscuits"},
        {"P020","Chips","Snacks","10","20","180","pcs","Salted"},
        {"P021","Cola 1L","Beverage","28","40","70","pcs","Soft drink"},
        {"P022","Juice 1L","Beverage","55","75","60","pcs","Mixed fruit"},
        {"P023","Eggs (dozen)","Poultry","50","65","30","pcs","Farm fresh"},
        {"P024","Chicken 1kg","Poultry","160","200","25","kg","Grade A"},
        {"P025","Fish 1kg","Seafood","200","260","20","kg","Fresh catch"}
    };
    overwriteCSV(BAZ_PRODUCTS_FILE, prod);

    vector<vector<string>> vendor = {
        {"vid","name","phone","items","notes"},
        {"V001","Fresh Farm Suppliers","9876543211","Vegetables,Fruits","Daily supply"},
        {"V002","Dairy Corp","9876543212","Milk,Curd","Morning delivery"},
        {"V003","Grocery Hub","9876543213","Rice,Wheat,Sugar","Bulk supply"},
        {"V004","CleanCo","9876543214","Detergents,Cleaners","Weekly"},
        {"V005","SnackPoint","9876543215","Chips,Biscuits","Alternate days"}
    };
    overwriteCSV(BAZ_VENDOR_FILE, vendor);

    vector<vector<string>> sales = {
        {"sid","pid","qty","each","total","date","time"},
        {"S001","P003","5","30","150","2025-02-01","10:15"},
        {"S002","P008","2","70","140","2025-02-01","11:20"}
    };
    overwriteCSV(BAZ_SALES_FILE, sales);

    vector<vector<string>> restock = {
        {"rid","pid","qty","vendor","date"},
        {"R001","P001","20","V001","2025-01-28"},
        {"R002","P006","40","V002","2025-01-29"}
    };
    overwriteCSV(BAZ_RESTOCK_FILE, restock);

    cout << "Market sample data loaded (25 products).\n";
}

/* ================= MAIN MENU ================= */

void baz_mainMenu() {
    while (true) {
        cout << "\n=== MARKET / BAZAAR MANAGEMENT ===\n";
        cout << "1. Add Product\n2. View Products\n3. Sell Product\n4. View Sales\n5. Add Vendor\n6. View Vendors\n7. Restock Product\n8. Load Sample Data\n9. Exit\nChoice: ";
        string c; getline(cin,c);
        if (c=="1") baz_addProduct();
        else if (c=="2") baz_viewProducts();
        else if (c=="3") baz_sellProduct();
        else if (c=="4") baz_viewSales();
        else if (c=="5") baz_addVendor();
        else if (c=="6") baz_viewVendors();
        else if (c=="7") baz_restockProduct();
        else if (c=="8") baz_loadSampleData();
        else if (c=="9") break;
        else cout << "Invalid choice.\n";
    }
}

int main() {
    baz_mainMenu();
    return 0;
}
