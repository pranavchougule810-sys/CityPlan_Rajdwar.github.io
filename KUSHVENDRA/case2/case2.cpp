// warehouses_full_avl.cpp
// AVL-indexed Warehouse Management System (CSV backed)
// Compile: g++ -std=gnu++17 warehouses_full_avl.cpp -O2 -o warehouses_full_avl

#include <bits/stdc++.h>
using namespace std;

// ---------- CSV helpers ----------
vector<vector<string>> readCSV(const string &fname) {
    vector<vector<string>> rows;
    ifstream in(fname);
    if (!in.is_open()) return rows;
    string line;
    while (getline(in, line)) {
        vector<string> cols;
        string cur;
        bool inquote = false;
        for (size_t i = 0; i < line.size(); ++i) {
            char c = line[i];
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
    if (!out.is_open()) out.open(fname);
    for (size_t i = 0; i < row.size(); ++i) {
        string cell = row[i];
        if (cell.find(',') != string::npos) out << '"' << cell << '"';
        else out << cell;
        if (i + 1 < row.size()) out << ',';
    }
    out << '\n';
    out.close();
}

void overwriteCSV(const string &fname, const vector<vector<string>> &rows) {
    ofstream out(fname, ios::trunc);
    for (auto &r : rows) {
        for (size_t i = 0; i < r.size(); ++i) {
            if (r[i].find(',') != string::npos) out << '"' << r[i] << '"';
            else out << r[i];
            if (i + 1 < r.size()) out << ',';
        }
        out << '\n';
    }
    out.close();
}

void ensureHeader(const string &fname, const vector<string> &hdr) {
    ifstream in(fname);
    if (!in.is_open()) {
        ofstream out(fname);
        for (size_t i = 0; i < hdr.size(); ++i) {
            out << hdr[i];
            if (i + 1 < hdr.size()) out << ',';
        }
        out << '\n';
        out.close();
    } else in.close();
}

string trim(const string &s) {
    size_t a = 0;
    while (a < s.size() && isspace((unsigned char)s[a])) ++a;
    size_t b = s.size();
    while (b > a && isspace((unsigned char)s[b-1])) --b;
    return s.substr(a, b-a);
}

// ---------- File names ----------
const string WH_FILE = "warehouses.csv";
const string VEH_FILE = "vehicles.csv";

// ---------- AVL (string key -> vector of warehouse_ids) ----------
struct AVLNode {
    string key;                 // the AVL key (warehouse_id or name)
    vector<string> ids;         // one or more warehouse_id(s) associated with key
    AVLNode *left = nullptr;
    AVLNode *right = nullptr;
    int height = 1;
    AVLNode(const string &k, const string &id) : key(k) { ids.push_back(id); }
};

int heightOf(AVLNode *n) { return n ? n->height : 0; }
void updateHeight(AVLNode *n) { if(n) n->height = 1 + max(heightOf(n->left), heightOf(n->right)); }
int balanceOf(AVLNode *n) { return n ? heightOf(n->left) - heightOf(n->right) : 0; }

AVLNode* rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    updateHeight(y); updateHeight(x);
    return x;
}
AVLNode* rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    updateHeight(x); updateHeight(y);
    return y;
}

AVLNode* insertAVLNode(AVLNode* node, const string &key, const string &id) {
    if(!node) return new AVLNode(key,id);
    if(key < node->key) node->left = insertAVLNode(node->left, key, id);
    else if(key > node->key) node->right = insertAVLNode(node->right, key, id);
    else {
        // key equal -> add id if not present
        if(find(node->ids.begin(), node->ids.end(), id) == node->ids.end())
            node->ids.push_back(id);
        return node;
    }
    updateHeight(node);
    int balance = balanceOf(node);
    if(balance > 1 && key < node->left->key) return rotateRight(node);
    if(balance < -1 && key > node->right->key) return rotateLeft(node);
    if(balance > 1 && key > node->left->key) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if(balance < -1 && key < node->right->key) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}

// find min node
AVLNode* minValueNode(AVLNode* node) {
    AVLNode* cur = node;
    while(cur && cur->left) cur = cur->left;
    return cur;
}

// remove id from node's ids vector; if resulting ids empty, remove node entirely
AVLNode* deleteAVLNode(AVLNode* root, const string &key, const string &id, bool &removedKeyNode) {
    if(!root) return root;
    if(key < root->key) root->left = deleteAVLNode(root->left, key, id, removedKeyNode);
    else if(key > root->key) root->right = deleteAVLNode(root->right, key, id, removedKeyNode);
    else {
        // key match: remove id from ids vector
        auto it = find(root->ids.begin(), root->ids.end(), id);
        if(it != root->ids.end()) root->ids.erase(it);
        // if ids remain, keep node
        if(!root->ids.empty()) return root;
        // else delete this node
        removedKeyNode = true;
        if(!root->left || !root->right) {
            AVLNode* temp = root->left ? root->left : root->right;
            if(!temp) { temp = root; root = nullptr; delete temp; }
            else { *root = *temp; delete temp; }
        } else {
            AVLNode* temp = minValueNode(root->right);
            root->key = temp->key;
            root->ids = temp->ids;
            bool dummy=false;
            root->right = deleteAVLNode(root->right, temp->key, temp->ids.front(), dummy);
        }
    }
    if(!root) return root;
    updateHeight(root);
    int balance = balanceOf(root);
    if(balance > 1 && balanceOf(root->left) >= 0) return rotateRight(root);
    if(balance > 1 && balanceOf(root->left) < 0) {
        root->left = rotateLeft(root->left);
        return rotateRight(root);
    }
    if(balance < -1 && balanceOf(root->right) <= 0) return rotateLeft(root);
    if(balance < -1 && balanceOf(root->right) > 0) {
        root->right = rotateRight(root->right);
        return rotateLeft(root);
    }
    return root;
}

AVLNode* findAVLNode(AVLNode* root, const string &key) {
    AVLNode* cur = root;
    while(cur) {
        if(key == cur->key) return cur;
        if(key < cur->key) cur = cur->left;
        else cur = cur->right;
    }
    return nullptr;
}

void inorderAVL(AVLNode* root, function<void(AVLNode*)> f) {
    if(!root) return;
    inorderAVL(root->left, f);
    f(root);
    inorderAVL(root->right, f);
}

void deleteAllAVL(AVLNode* root) {
    if(!root) return;
    deleteAllAVL(root->left);
    deleteAllAVL(root->right);
    delete root;
}

// ---------- Global in-memory state ----------
vector<vector<string>> rows; // CSV rows for warehouses (header at rows[0])
unordered_map<string,int> idToIndex; // warehouse_id -> row index in rows
AVLNode* idAVL = nullptr;    // AVL keyed by warehouse_id, ids vector contains the same warehouse_id
AVLNode* nameAVL = nullptr;  // AVL keyed by warehouse_name, ids vector contains warehouse_id(s)

// Vehicle CSV is left unchanged; we will use simple append/read for vehicles
void initWarehouse() {
    ensureHeader(WH_FILE, {"warehouse_id","name","capacity","current_stock","location"});
}

// Rebuild in-memory rows and indices (call after major changes or at startup)
void rebuildFromCSV() {
    rows = readCSV(WH_FILE);
    idToIndex.clear();
    deleteAllAVL(idAVL); idAVL = nullptr;
    deleteAllAVL(nameAVL); nameAVL = nullptr;
    for(size_t i = 1; i < rows.size(); ++i) {
        string id = rows[i].size() > 0 ? trim(rows[i][0]) : "";
        string name = rows[i].size() > 1 ? trim(rows[i][1]) : "";
        if(id.empty()) continue;
        idToIndex[id] = (int)i;
        idAVL = insertAVLNode(idAVL, id, id);
        // normalize name as-is (case-sensitive); you may choose to lowercase if you want case-insensitive
        nameAVL = insertAVLNode(nameAVL, name, id);
    }
}

// Persist current rows vector to CSV file
void persistRowsToCSV() {
    overwriteCSV(WH_FILE, rows);
}

// ---------- CRUD & operations (using AVL indices) ----------
void addWarehouse() {
    initWarehouse();
    string id, name, cap, stock, loc;
    cout << "Warehouse ID: "; getline(cin, id);
    cout << "Name: "; getline(cin, name);
    cout << "Capacity: "; getline(cin, cap);
    cout << "Current Stock: "; getline(cin, stock);
    cout << "Location: "; getline(cin, loc);

    // append row in-memory then persist
    if(rows.empty()) {
        // header missing: add header
        rows.push_back({"warehouse_id","name","capacity","current_stock","location"});
    }
    rows.push_back({id,name,cap,stock,loc});
    persistRowsToCSV();

    // update indices
    int idx = (int)rows.size()-1;
    idToIndex[id] = idx;
    idAVL = insertAVLNode(idAVL, id, id);
    nameAVL = insertAVLNode(nameAVL, name, id);

    cout << "Warehouse added.\n";
}

void viewWarehouse() {
    initWarehouse();
    rebuildFromCSV();
    cout << "Warehouses (unsorted):\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        auto &r = rows[i];
        if(r.size() < 5) continue;
        cout << r[0] << " | " << r[1] << " | Cap:" << r[2] << " | Stock:" << r[3] << " | " << r[4] << "\n";
    }
}

void viewWarehouseSortedById() {
    initWarehouse();
    rebuildFromCSV();
    cout << "Warehouses (sorted by ID):\n";
    inorderAVL(idAVL, [&](AVLNode* node){
        for(auto &wid : node->ids) {
            auto it = idToIndex.find(wid);
            if(it != idToIndex.end()) {
                auto &r = rows[it->second];
                if(r.size() < 5) continue;
                cout << r[0] << " | " << r[1] << " | Cap:" << r[2] << " | Stock:" << r[3] << " | " << r[4] << "\n";
            }
        }
    });
}

void viewWarehouseSortedByName() {
    initWarehouse();
    rebuildFromCSV();
    cout << "Warehouses (sorted by NAME):\n";
    inorderAVL(nameAVL, [&](AVLNode* node){
        for(auto &wid : node->ids) {
            auto it = idToIndex.find(wid);
            if(it != idToIndex.end()) {
                auto &r = rows[it->second];
                if(r.size() < 5) continue;
                cout << r[0] << " | " << r[1] << " | Cap:" << r[2] << " | Stock:" << r[3] << " | " << r[4] << "\n";
            }
        }
    });
}

void searchWarehouse() {
    initWarehouse();
    rebuildFromCSV();
    cout << "Enter warehouse ID or exact name: ";
    string term; getline(cin, term);
    // try ID first
    auto it = idToIndex.find(term);
    if(it != idToIndex.end()) {
        auto &r = rows[it->second];
        cout << "Found by ID: " << r[0] << " | " << r[1] << " | Cap:" << r[2] << " | Stock:" << r[3] << " | " << r[4] << "\n";
        return;
    }
    // try name AVL
    AVLNode* node = findAVLNode(nameAVL, term);
    if(node) {
        cout << "Found by name (matches " << node->ids.size() << " warehouses):\n";
        for(auto &wid : node->ids) {
            auto it2 = idToIndex.find(wid);
            if(it2 != idToIndex.end()) {
                auto &r = rows[it2->second];
                cout << r[0] << " | " << r[1] << " | Cap:" << r[2] << " | Stock:" << r[3] << " | " << r[4] << "\n";
            }
        }
        return;
    }
    cout << "Not found.\n";
}

void deleteWarehouse() {
    initWarehouse();
    rebuildFromCSV();
    cout << "Enter warehouse ID to delete: ";
    string id; getline(cin, id);
    auto it = idToIndex.find(id);
    if(it == idToIndex.end()) { cout << "ID not found.\n"; return; }
    int idx = it->second;
    string name = rows[idx].size()>1 ? rows[idx][1] : "";
    // remove row from rows
    vector<vector<string>> newRows;
    newRows.push_back(rows[0]); // header
    for(size_t i = 1; i < rows.size(); ++i) if((int)i != idx) newRows.push_back(rows[i]);
    rows.swap(newRows);
    // persist
    persistRowsToCSV();
    // rebuild indices
    rebuildFromCSV();
    cout << "Deleted.\n";
}

// Auto-stock alert: shows low (<20%) or over-capacity (>100%) warehouses
void autoStockAlert() {
    initWarehouse();
    rebuildFromCSV();
    cout << "Auto-stock alerts:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        int cap = 0, stock = 0;
        try { cap = stoi(rows[i][2]); } catch(...) { cap = 0; }
        try { stock = stoi(rows[i][3]); } catch(...) { stock = 0; }
        if (stock > cap) cout << "Over capacity: " << rows[i][1] << " (ID: " << rows[i][0] << ")\n";
        else if (cap>0 && stock < cap*0.2) cout << "Low stock: " << rows[i][1] << " (ID: " << rows[i][0] << ")\n";
    }
}

// Storage utilization calculation
void storageUtilization() {
    initWarehouse();
    rebuildFromCSV();
    cout << "Warehouse utilization:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        int cap = 0, stock = 0;
        try { cap = stoi(rows[i][2]); } catch(...) { cap = 0; }
        try { stock = stoi(rows[i][3]); } catch(...) { stock = 0; }
        double util = (cap==0?0.0:(double)stock*100.0/cap);
        cout << rows[i][1] << ": " << fixed << setprecision(2) << util << "% (ID: " << rows[i][0] << ")\n";
    }
}

// Goods-In / Goods-Out using idToIndex
void goodsIn() {
    initWarehouse();
    rebuildFromCSV();
    cout << "Enter Warehouse ID: "; string id; getline(cin, id);
    auto it = idToIndex.find(id);
    if(it == idToIndex.end()) { cout << "Warehouse not found.\n"; return; }
    int idx = it->second;
    int stock=0, cap=0, qty=0;
    try { stock = stoi(rows[idx][3]); } catch(...) { stock = 0; }
    try { cap = stoi(rows[idx][2]); } catch(...) { cap = 0; }
    cout << "Quantity coming in: "; string s; getline(cin,s); try{qty=stoi(s);}catch(...){qty=0;}
    stock += qty;
    rows[idx][3] = to_string(stock);
    persistRowsToCSV();
    rebuildFromCSV();
    if (cap>0 && stock > cap) cout << "Alert: Stock exceeds capacity!\n";
    cout << "Goods-In updated.\n";
}

void goodsOut() {
    initWarehouse();
    rebuildFromCSV();
    cout << "Enter Warehouse ID: "; string id; getline(cin, id);
    auto it = idToIndex.find(id);
    if(it == idToIndex.end()) { cout << "Warehouse not found.\n"; return; }
    int idx = it->second;
    int stock=0, qty=0;
    try { stock = stoi(rows[idx][3]); } catch(...) { stock = 0; }
    cout << "Quantity leaving: "; string s; getline(cin,s); try{qty=stoi(s);}catch(...){qty=0;}
    if (qty > stock) cout << "Cannot remove more than current stock!\n";
    else {
        stock -= qty;
        rows[idx][3] = to_string(stock);
        persistRowsToCSV();
        rebuildFromCSV();
        cout << "Goods-Out updated.\n";
    }
}

// ---------- Vehicle functions ----------
void initVehicles() {
    ensureHeader(VEH_FILE, {"vehicle_id","warehouse_id","driver_name","vehicle_type","capacity_tons"});
}

void assignVehicle() {
    initVehicles();
    string vid, wid, driver, type, cap;
    cout << "Vehicle ID: "; getline(cin, vid);
    cout << "Warehouse ID: "; getline(cin, wid);
    cout << "Driver Name: "; getline(cin, driver);
    cout << "Vehicle Type (truck/mini-truck/tempo/van): "; getline(cin, type);
    cout << "Capacity (tons): "; getline(cin, cap);
    appendCSV(VEH_FILE, {vid,wid,driver,type,cap});
    cout << "Vehicle assigned.\n";
}

// ---------- Daily Summary ----------
void dailySummary() {
    initWarehouse();
    rebuildFromCSV();
    int totalWarehouses = (int)max((size_t)0, rows.size() > 0 ? rows.size()-1 : 0);
    long long totalCapacity = 0, totalStock = 0;
    int lowStock = 0, highStock = 0;
    for (size_t i = 1; i < rows.size(); ++i) {
        int cap = 0, stock = 0;
        try { cap = stoi(rows[i][2]); } catch(...) { cap = 0; }
        try { stock = stoi(rows[i][3]); } catch(...) { stock = 0; }
        totalCapacity += cap;
        totalStock += stock;
        if (cap>0 && stock < cap*0.2) lowStock++;
        if (cap>0 && stock > cap*0.8) highStock++;
    }
    double overallUtil = (totalCapacity==0?0.0:(double)totalStock*100.0/totalCapacity);
    cout << "Daily Storage Summary:\n";
    cout << "Total warehouses: " << totalWarehouses << "\n";
    cout << "Total capacity: " << totalCapacity << "\n";
    cout << "Total current stock: " << totalStock << "\n";
    cout << "Overall utilization: " << fixed << setprecision(2) << overallUtil << "%\n";
    cout << "Low-stock warehouses: " << lowStock << "\n";
    cout << "High-stock warehouses: " << highStock << "\n";
}

// ---------- Sample data loader (10+ rows each for warehouses & vehicles) ----------
void loadSampleData() {
    // Sample warehouses: overwrite warehouses.csv completely
    vector<vector<string>> whRows;
    whRows.push_back({"warehouse_id","name","capacity","current_stock","location"});
    whRows.push_back({"W001","Central Warehouse","10000","7000","City Center"});
    whRows.push_back({"W002","North Hub","8000","1200","North Zone"});
    whRows.push_back({"W003","South Hub","9000","8500","South Zone"});
    whRows.push_back({"W004","East Depot","6000","1000","East Zone"});
    whRows.push_back({"W005","West Depot","5000","4500","West Zone"});
    whRows.push_back({"W006","Spare Parts Store","4000","600","Industrial Area"});
    whRows.push_back({"W007","Finished Goods Store","11000","9500","Export Yard"});
    whRows.push_back({"W008","Raw Material Yard","15000","3000","Raw Material Park"});
    whRows.push_back({"W009","Transit Warehouse","7000","6800","Highway Junction"});
    whRows.push_back({"W010","Overflow Warehouse","12000","500","Outer Ring Road"});
    overwriteCSV(WH_FILE, whRows);

    // Sample vehicles: overwrite vehicles.csv completely
    vector<vector<string>> vehRows;
    vehRows.push_back({"vehicle_id","warehouse_id","driver_name","vehicle_type","capacity_tons"});
    vehRows.push_back({"V001","W001","Ramesh","truck","10"});
    vehRows.push_back({"V002","W002","Suresh","mini-truck","4"});
    vehRows.push_back({"V003","W003","Mahesh","truck","12"});
    vehRows.push_back({"V004","W004","Lokesh","tempo","3"});
    vehRows.push_back({"V005","W005","Naresh","van","2"});
    vehRows.push_back({"V006","W006","Ganesh","truck","8"});
    vehRows.push_back({"V007","W007","Dinesh","truck","15"});
    vehRows.push_back({"V008","W008","Harish","mini-truck","5"});
    vehRows.push_back({"V009","W009","Yogesh","tempo","3"});
    vehRows.push_back({"V010","W010","Rajesh","truck","10"});
    overwriteCSV(VEH_FILE, vehRows);

    // Rebuild in-memory structures from the fresh warehouse CSV
    rebuildFromCSV();
    cout << "Sample data loaded: 10 warehouses and 10 vehicles.\n";
}

// ---------- Menus ----------
void warehouseMenu() {
    while (true) {
        cout << "\n========== WAREHOUSE MANAGEMENT SYSTEM (AVL Indexed) ==========\n";
        cout << "1.Add Warehouse\n2.View Warehouse (unsorted)\n3.View Warehouse (sorted by ID)\n4.View Warehouse (sorted by NAME)\n5.Search Warehouse\n6.Delete Warehouse\n";
        cout << "7.Auto-stock Alert\n8.Storage Utilization\n";
        cout << "9.Goods-In Entry\n10.Goods-Out Entry\n11.Assign Transport Vehicle\n12.Daily Storage Summary\n13.Exit\nChoice: ";
        string choice;
        if(!getline(cin, choice)) return;
        if(choice.empty()) continue;
        int c = stoi(choice);
        if (c == 1) addWarehouse();
        else if (c == 2) viewWarehouse();
        else if (c == 3) viewWarehouseSortedById();
        else if (c == 4) viewWarehouseSortedByName();
        else if (c == 5) searchWarehouse();
        else if (c == 6) deleteWarehouse();
        else if (c == 7) autoStockAlert();
        else if (c == 8) storageUtilization();
        else if (c == 9) goodsIn();
        else if (c == 10) goodsOut();
        else if (c == 11) assignVehicle();
        else if (c == 12) dailySummary();
        else if (c == 13) return;
        else cout << "Invalid choice.\n";
    }
}

int main() {
    initWarehouse();
    initVehicles();

    // Load sample data once at startup (overwrites existing CSV contents)
    loadSampleData();

    warehouseMenu();
    cout << "Exiting Warehouse Management System...\n";
    // cleanup AVLs
    deleteAllAVL(idAVL); idAVL = nullptr;
    deleteAllAVL(nameAVL); nameAVL = nullptr;
    return 0;
}
