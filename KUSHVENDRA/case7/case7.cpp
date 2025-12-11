// retail_supermarket.cpp
// Compile: g++ -std=gnu++17 retail_supermarket.cpp -o retail_supermarket

#include <bits/stdc++.h>
using namespace std;

// ---------- CSV Helpers ----------
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

void overwriteCSV(const string &fname, const vector<vector<string>> &rows) {
    ofstream out(fname, ios::trunc);
    for (auto &r : rows) {
        for (size_t i = 0; i < r.size(); ++i) {
            if (r[i].find(',') != string::npos) out << '"' << r[i] << '"';
            else out << r[i];
            if (i+1 < r.size()) out << ',';
        }
        out << '\n';
    }
    out.close();
}

void appendCSV(const string &fname, const vector<string> &row) {
    ofstream out(fname, ios::app);
    for (size_t i = 0; i < row.size(); ++i) {
        if (row[i].find(',') != string::npos) out << '"' << row[i] << '"';
        else out << row[i];
        if (i+1 < row.size()) out << ',';
    }
    out << '\n';
    out.close();
}

void ensureHeader(const string &fname, const vector<string> &hdr) {
    ifstream in(fname);
    if (!in.is_open()) {
        ofstream out(fname);
        for (size_t i = 0; i < hdr.size(); ++i) {
            out << hdr[i];
            if (i+1 < hdr.size()) out << ',';
        }
        out << '\n';
        out.close();
    } else in.close();
}

// ---------- File names ----------
const string STORE_FILE    = "stores.csv";
const string PRODUCT_FILE  = "products.csv";
const string INVENTORY_FILE= "inventory.csv";

// ---------- Store Module ----------
struct Store {
    string id, name, location;
};

void initStore() {
    ensureHeader(STORE_FILE, {"store_id","name","location"});
}

void addStore() {
    initStore();
    string id, name, loc;
    cout << "Store ID: "; cin >> id; cin.ignore();
    cout << "Name: "; getline(cin, name);
    cout << "Location: "; getline(cin, loc);
    appendCSV(STORE_FILE, {id,name,loc});
    cout << "Store added.\n";
}

void viewStores() {
    initStore();
    auto rows = readCSV(STORE_FILE);
    cout << "Stores:\n";
    for (size_t i=1;i<rows.size();++i)
        cout << rows[i][0] << " | " << rows[i][1] << " | " << rows[i][2] << "\n";
}

// ---------- Product Module (Trie for search) ----------
struct TrieNode {
    unordered_map<char, TrieNode*> children;
    vector<string> product_ids; // store IDs of products with this prefix
};

TrieNode* root = new TrieNode();

struct Product {
    string id, name, category;
};

void initProduct() {
    ensureHeader(PRODUCT_FILE, {"product_id","name","category"});
}

void addProduct() {
    initProduct();
    string id, name, cat;
    cout << "Product ID: "; cin >> id; cin.ignore();
    cout << "Name: "; getline(cin, name);
    cout << "Category: "; getline(cin, cat);
    appendCSV(PRODUCT_FILE, {id,name,cat});

    // Insert into Trie
    TrieNode* node = root;
    for (char c : name) {
        if (!node->children[c]) node->children[c] = new TrieNode();
        node = node->children[c];
        node->product_ids.push_back(id);
    }
    cout << "Product added.\n";
}

void searchProduct() {
    string prefix;
    cout << "Enter product name prefix to search: "; cin.ignore(); getline(cin, prefix);
    TrieNode* node = root;
    for (char c : prefix) {
        if (!node->children[c]) { cout << "No products found.\n"; return; }
        node = node->children[c];
    }
    if (node->product_ids.empty()) { cout << "No products found.\n"; return; }
    cout << "Products matching prefix:\n";
    for (string pid : node->product_ids) cout << pid << "\n";
}

// ---------- Inventory Module (Segment Tree for total stock query) ----------
struct Inventory {
    string store_id, product_id;
    int quantity;
};

vector<Inventory> inventory;
vector<int> segTree;

void initInventory() {
    ensureHeader(INVENTORY_FILE, {"store_id","product_id","quantity"});
    inventory = {};
    auto rows = readCSV(INVENTORY_FILE);
    for (size_t i=1;i<rows.size();++i)
        inventory.push_back({rows[i][0],rows[i][1],stoi(rows[i][2])});
}

void saveInventory() {
    vector<vector<string>> rows = {{"store_id","product_id","quantity"}};
    for (auto &inv : inventory)
        rows.push_back({inv.store_id,inv.product_id,to_string(inv.quantity)});
    overwriteCSV(INVENTORY_FILE, rows);
}

// Segment Tree for range sum query (total inventory per store)
void buildSegTree(int n) {
    int size = 1; while(size<n) size*=2;
    segTree.assign(2*size,0);
    for (int i=0;i<n;i++) segTree[size+i] = inventory[i].quantity;
    for (int i=size-1;i>0;i--) segTree[i] = segTree[2*i]+segTree[2*i+1];
}

void updateSegTree(int idx, int val, int n) {
    int size = segTree.size()/2;
    idx += size;
    segTree[idx] = val;
    while(idx>1){
        idx/=2;
        segTree[idx] = segTree[2*idx]+segTree[2*idx+1];
    }
}

int querySegTree(int l,int r,int n){
    int size = segTree.size()/2;
    l+=size; r+=size;
    int res=0;
    while(l<=r){
        if(l%2==1) res+=segTree[l++];
        if(r%2==0) res+=segTree[r--];
        l/=2; r/=2;
    }
    return res;
}

void addInventory() {
    string sid,pid; int qty;
    cout << "Store ID: "; cin >> sid;
    cout << "Product ID: "; cin >> pid;
    cout << "Quantity: "; cin >> qty;
    inventory.push_back({sid,pid,qty});
    saveInventory();
    buildSegTree((int)inventory.size());
    cout << "Inventory added.\n";
}

void viewInventory() {
    if(inventory.empty()){ cout << "No inventory.\n"; return; }
    cout << "StoreID | ProductID | Quantity\n";
    for(auto &inv:inventory)
        cout << inv.store_id << " | " << inv.product_id << " | " << inv.quantity << "\n";
}

void totalInventoryQuery() {
    if(inventory.empty()){ cout << "No inventory.\n"; return; }
    string sid;
    cout << "Enter Store ID for total inventory query: "; cin >> sid;
    int total=0;
    for(int i=0;i<(int)inventory.size();i++)
        if(inventory[i].store_id==sid) total+=inventory[i].quantity;
    cout << "Total inventory for store " << sid << ": " << total << "\n";
}

// ---------- Sample Data Loader (10+ rows each) ----------
void loadSampleData() {
    // 10 sample stores
    vector<vector<string>> sRows;
    sRows.push_back({"store_id","name","location"});
    sRows.push_back({"S001","Central Mart","City Center"});
    sRows.push_back({"S002","North Mart","North Zone"});
    sRows.push_back({"S003","South Mart","South Zone"});
    sRows.push_back({"S004","East Mart","East Zone"});
    sRows.push_back({"S005","West Mart","West Zone"});
    sRows.push_back({"S006","Airport Mart","Airport"});
    sRows.push_back({"S007","Tech Mart","IT Park"});
    sRows.push_back({"S008","Campus Mart","University"});
    sRows.push_back({"S009","Highway Mart","Highway"});
    sRows.push_back({"S010","Mall Mart","Shopping Mall"});
    overwriteCSV(STORE_FILE, sRows);

    // 10 sample products
    vector<vector<string>> pRows;
    pRows.push_back({"product_id","name","category"});
    pRows.push_back({"P001","Rice 5kg","Grocery"});
    pRows.push_back({"P002","Wheat Flour","Grocery"});
    pRows.push_back({"P003","Sugar 1kg","Grocery"});
    pRows.push_back({"P004","Milk 1L","Dairy"});
    pRows.push_back({"P005","Curd 500g","Dairy"});
    pRows.push_back({"P006","Shampoo","Personal Care"});
    pRows.push_back({"P007","Soap","Personal Care"});
    pRows.push_back({"P008","Biscuits","Snacks"});
    pRows.push_back({"P009","Chips","Snacks"});
    pRows.push_back({"P010","Soft Drink","Beverages"});
    overwriteCSV(PRODUCT_FILE, pRows);

    // Rebuild Trie from these products
    // Reset Trie
    root = new TrieNode();
    for(size_t i=1;i<pRows.size();++i){
        string id = pRows[i][0];
        string name = pRows[i][1];
        TrieNode* node = root;
        for(char c : name){
            if(!node->children[c]) node->children[c] = new TrieNode();
            node = node->children[c];
            node->product_ids.push_back(id);
        }
    }

    // 10 sample inventory records
    vector<vector<string>> iRows;
    iRows.push_back({"store_id","product_id","quantity"});
    iRows.push_back({"S001","P001","100"});
    iRows.push_back({"S001","P004","80"});
    iRows.push_back({"S002","P002","60"});
    iRows.push_back({"S002","P005","40"});
    iRows.push_back({"S003","P003","50"});
    iRows.push_back({"S004","P006","30"});
    iRows.push_back({"S005","P007","70"});
    iRows.push_back({"S006","P008","90"});
    iRows.push_back({"S007","P009","110"});
    iRows.push_back({"S008","P010","55"});
    overwriteCSV(INVENTORY_FILE, iRows);

    // Load inventory into memory and build segment tree
    initInventory();
    buildSegTree((int)inventory.size());

    cout << "Sample data loaded for stores, products, and inventory.\n";
}

// ---------- Menu ----------
void menu() {
    // Load sample data once at start (overwrites CSVs)
    loadSampleData();

    while(true) {
        cout << "\n===== RETAIL & SUPERMARKET MANAGEMENT SYSTEM =====\n";
        cout << "1.Add Store\n2.View Stores\n3.Add Product\n4.Search Product\n5.Add Inventory\n6.View Inventory\n7.Total Inventory Query\n8.Exit\nChoice: ";
        int c; cin >> c;
        switch(c) {
            case 1: addStore(); break;
            case 2: viewStores(); break;
            case 3: addProduct(); break;
            case 4: searchProduct(); break;
            case 5: addInventory(); break;
            case 6: viewInventory(); break;
            case 7: totalInventoryQuery(); break;
            case 8: return;
            default: cout << "Invalid choice\n";
        }
    }
}

int main() {
    menu();
    return 0;
}
