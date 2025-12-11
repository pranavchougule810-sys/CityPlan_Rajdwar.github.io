// power_stations_avl.cpp
// Full AVL-indexed Power Station Management System (Option A)
// Compile: g++ -std=gnu++17 power_stations_avl.cpp -O2 -o power_stations_avl

#include <bits/stdc++.h>
using namespace std;

// ---------- Station Structure ----------
struct Station {
    string id;
    string name;
    string location;
    string status;
    int capacity = 0;
    int load = 0;
};

// ---------- Filenames ----------
const string FNAME = "stations.csv";
const string LOGFNAME = "station_logs.csv";

// ---------- Utility helpers ----------
string trim(const string &s) {
    size_t a = 0;
    while (a < s.size() && isspace((unsigned char)s[a])) ++a;
    size_t b = s.size();
    while (b > a && isspace((unsigned char)s[b-1])) --b;
    return s.substr(a, b-a);
}

// Simple CSV line splitter (handles quoted cells simply)
vector<string> splitCSVLine(const string &line) {
    vector<string> out;
    string cur;
    bool inquote = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') { inquote = !inquote; continue; }
        if (c == ',' && !inquote) {
            out.push_back(cur);
            cur.clear();
        } else cur.push_back(c);
    }
    out.push_back(cur);
    for (auto &x : out) x = trim(x);
    return out;
}

void ensureCSVHeader() {
    // ensure stations.csv exists and has header
    ifstream in(FNAME);
    if (!in.is_open()) {
        ofstream out(FNAME);
        out << "id,name,location,capacity,load,status\n";
        out.close();
    } else {
        string line;
        if(!getline(in,line) || trim(line).empty()) {
            in.close();
            ofstream out(FNAME);
            out << "id,name,location,capacity,load,status\n";
            out.close();
        } else in.close();
    }
    // ensure log file exists with header
    ifstream lg(LOGFNAME);
    if (!lg.is_open()) {
        ofstream out(LOGFNAME);
        out << "datetime,id,name,load\n";
        out.close();
    } else {
        string line;
        if(!getline(lg,line) || trim(line).empty()) {
            lg.close();
            ofstream out(LOGFNAME);
            out << "datetime,id,name,load\n";
            out.close();
        } else lg.close();
    }
}

// ---------- AVL Node for ID (stores Station) ----------
struct IDAVLNode {
    string key; // station id
    Station st;
    IDAVLNode *left = nullptr, *right = nullptr;
    int height = 1;
    IDAVLNode(const string &k, const Station &s) : key(k), st(s) {}
};

int heightOf(IDAVLNode* n) { return n ? n->height : 0; }
void updateHeight(IDAVLNode* n) { if(n) n->height = 1 + max(heightOf(n->left), heightOf(n->right)); }
int balanceOf(IDAVLNode* n) { return n ? heightOf(n->left) - heightOf(n->right) : 0; }
IDAVLNode* rotateRight(IDAVLNode* y) {
    IDAVLNode* x = y->left;
    IDAVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    updateHeight(y); updateHeight(x);
    return x;
}
IDAVLNode* rotateLeft(IDAVLNode* x) {
    IDAVLNode* y = x->right;
    IDAVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    updateHeight(x); updateHeight(y);
    return y;
}

IDAVLNode* idInsert(IDAVLNode* node, const string &key, const Station &s) {
    if(!node) return new IDAVLNode(key,s);
    if(key < node->key) node->left = idInsert(node->left, key, s);
    else if(key > node->key) node->right = idInsert(node->right, key, s);
    else {
        // duplicate id — replace station
        node->st = s;
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

IDAVLNode* idMinValueNode(IDAVLNode* node) {
    IDAVLNode* cur = node;
    while(cur && cur->left) cur = cur->left;
    return cur;
}

IDAVLNode* idDelete(IDAVLNode* root, const string &key, bool &removed) {
    if(!root) return root;
    if(key < root->key) root->left = idDelete(root->left, key, removed);
    else if(key > root->key) root->right = idDelete(root->right, key, removed);
    else {
        removed = true;
        if(!root->left || !root->right) {
            IDAVLNode* temp = root->left ? root->left : root->right;
            if(!temp) { temp = root; root = nullptr; delete temp; }
            else { *root = *temp; delete temp; }
        } else {
            IDAVLNode* temp = idMinValueNode(root->right);
            root->key = temp->key;
            root->st = temp->st;
            bool dummy=false;
            root->right = idDelete(root->right, temp->key, dummy);
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

IDAVLNode* idFind(IDAVLNode* root, const string &key) {
    IDAVLNode* cur = root;
    while(cur) {
        if(key == cur->key) return cur;
        if(key < cur->key) cur = cur->left;
        else cur = cur->right;
    }
    return nullptr;
}

void idInorderCollect(IDAVLNode* root, vector<Station> &out) {
    if(!root) return;
    idInorderCollect(root->left, out);
    out.push_back(root->st);
    idInorderCollect(root->right, out);
}

void idDeleteAll(IDAVLNode* root) {
    if(!root) return;
    idDeleteAll(root->left);
    idDeleteAll(root->right);
    delete root;
}

// ---------- AVL Node for Name (maps name -> list of station ids) ----------
struct NameAVLNode {
    string key; // station name
    vector<string> ids; // station ids with this name
    NameAVLNode *left = nullptr, *right = nullptr;
    int height = 1;
    NameAVLNode(const string &k, const string &id) : key(k) { ids.push_back(id); }
};

int heightOf(NameAVLNode* n) { return n ? n->height : 0; }
void updateHeight(NameAVLNode* n) { if(n) n->height = 1 + max(heightOf(n->left), heightOf(n->right)); }
int balanceOf(NameAVLNode* n) { return n ? heightOf(n->left) - heightOf(n->right) : 0; }
NameAVLNode* rotateRight(NameAVLNode* y) {
    NameAVLNode* x = y->left;
    NameAVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    updateHeight(y); updateHeight(x);
    return x;
}
NameAVLNode* rotateLeft(NameAVLNode* x) {
    NameAVLNode* y = x->right;
    NameAVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    updateHeight(x); updateHeight(y);
    return y;
}

NameAVLNode* nameInsert(NameAVLNode* node, const string &key, const string &id) {
    if(!node) return new NameAVLNode(key,id);
    if(key < node->key) node->left = nameInsert(node->left, key, id);
    else if(key > node->key) node->right = nameInsert(node->right, key, id);
    else {
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

NameAVLNode* nameMinNode(NameAVLNode* n) {
    NameAVLNode* cur = n;
    while(cur && cur->left) cur = cur->left;
    return cur;
}

NameAVLNode* nameDelete(NameAVLNode* root, const string &key, const string &id, bool &removedKeyNode) {
    if(!root) return root;
    if(key < root->key) root->left = nameDelete(root->left, key, id, removedKeyNode);
    else if(key > root->key) root->right = nameDelete(root->right, key, id, removedKeyNode);
    else {
        // remove id from ids vector
        auto it = find(root->ids.begin(), root->ids.end(), id);
        if(it != root->ids.end()) root->ids.erase(it);
        if(!root->ids.empty()) return root;
        // else delete node entirely
        removedKeyNode = true;
        if(!root->left || !root->right) {
            NameAVLNode* temp = root->left ? root->left : root->right;
            if(!temp) { temp = root; root = nullptr; delete temp; }
            else { *root = *temp; delete temp; }
        } else {
            NameAVLNode* temp = nameMinNode(root->right);
            root->key = temp->key;
            root->ids = temp->ids;
            bool dummy=false;
            root->right = nameDelete(root->right, temp->key, temp->ids.front(), dummy);
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

NameAVLNode* nameFind(NameAVLNode* root, const string &key) {
    NameAVLNode* cur = root;
    while(cur) {
        if(key == cur->key) return cur;
        if(key < cur->key) cur = cur->left;
        else cur = cur->right;
    }
    return nullptr;
}

void nameInorder(NameAVLNode* root, function<void(NameAVLNode*)> f) {
    if(!root) return;
    nameInorder(root->left, f);
    f(root);
    nameInorder(root->right, f);
}

void nameDeleteAll(NameAVLNode* root) {
    if(!root) return;
    nameDeleteAll(root->left);
    nameDeleteAll(root->right);
    delete root;
}

// ---------- Global AVL roots ----------
IDAVLNode* idRoot = nullptr;
NameAVLNode* nameRoot = nullptr;

// ---------- CSV load/save helpers ----------
void loadStationsFromCSV() {
    ensureCSVHeader();
    // clear existing AVLs
    idDeleteAll(idRoot); idRoot = nullptr;
    nameDeleteAll(nameRoot); nameRoot = nullptr;

    ifstream in(FNAME);
    if(!in.is_open()) return;
    string line;
    getline(in, line); // header
    while(getline(in, line)) {
        if(trim(line).empty()) continue;
        auto cols = splitCSVLine(line);
        if(cols.size() < 6) continue;
        Station s;
        s.id = cols[0];
        s.name = cols[1];
        s.location = cols[2];
        try { s.capacity = stoi(cols[3]); } catch(...) { s.capacity = 0; }
        try { s.load = stoi(cols[4]); } catch(...) { s.load = 0; }
        s.status = cols[5].empty() ? "active" : cols[5];
        idRoot = idInsert(idRoot, s.id, s);
        nameRoot = nameInsert(nameRoot, s.name, s.id);
    }
    in.close();
}

void saveStationsToCSV() {
    // collect in-order by id (sorted by id)
    vector<Station> rows;
    idInorderCollect(idRoot, rows);
    ofstream out(FNAME, ios::trunc);
    out << "id,name,location,capacity,load,status\n";
    for(auto &s : rows) {
        out << s.id << "," << s.name << "," << s.location << "," << s.capacity << "," << s.load << "," << s.status << "\n";
    }
    out.close();
}

// ---------- Logging ----------
void logLoad(const Station &s) {
    ofstream out(LOGFNAME, ios::app);
    if(!out.is_open()) return;
    time_t now = time(0);
    string dt = ctime(&now);
    if(!dt.empty() && dt.back() == '\n') dt.pop_back();
    out << dt << "," << s.id << "," << s.name << "," << s.load << "\n";
    out.close();
}

// ---------- Business logic (uses AVL ops) ----------
void updateStatus(Station &s) {
    if (s.load > s.capacity) s.status = "overloaded";
    else if (s.capacity > 0 && s.load < s.capacity/4) s.status = "underutilized";
    else s.status = "active";
}

// Add station (insert into both AVLs and save)
void addStation() {
    Station s;
    cout << "Enter Station ID: "; cin >> s.id; cin.ignore();
    cout << "Enter Name: "; getline(cin, s.name);
    cout << "Enter Location: "; getline(cin, s.location);
    cout << "Enter Capacity (MW): "; string tmp; getline(cin,tmp); try{s.capacity = stoi(tmp);}catch(...){s.capacity=0;}
    s.load = 0; s.status = "active";

    // check duplicate id
    if(idFind(idRoot, s.id)) {
        cout << "Station ID already exists — updating record instead.\n";
        idRoot = idInsert(idRoot, s.id, s);
        // name AVL fully rebuilt below
    } else {
        idRoot = idInsert(idRoot, s.id, s);
    }
    // rebuild name AVL fully for correctness
    nameDeleteAll(nameRoot); nameRoot = nullptr;
    vector<Station> all;
    idInorderCollect(idRoot, all);
    for(auto &st : all) nameRoot = nameInsert(nameRoot, st.name, st.id);

    // persist to CSV
    saveStationsToCSV();
    cout << "Station added.\n";
}

// View all stations (in-order by id)
void viewStations() {
    if(!idRoot) { cout << "No stations.\n"; return; }
    cout << left << setw(5) << "ID" 
         << setw(12) << "Name" 
         << setw(12) << "Location" 
         << setw(12) << "Capacity(MW)" 
         << setw(12) << "Load(MW)" 
         << setw(15) << "Status" << "\n";
    cout << string(68,'-') << "\n";
    vector<Station> all;
    idInorderCollect(idRoot, all);
    for(auto &s : all) {
        cout << left << setw(5) << s.id 
             << setw(12) << s.name 
             << setw(12) << s.location 
             << setw(12) << s.capacity 
             << setw(12) << s.load 
             << setw(15) << s.status << "\n";
    }
}

// Search by ID or name
void searchStation() {
    cout << "Enter Station ID or Name: ";
    string term; getline(cin, term);
    if(term.empty()) { cout << "Empty input.\n"; return; }
    // try ID
    IDAVLNode* nid = idFind(idRoot, term);
    if(nid) {
        Station &s = nid->st;
        cout << s.id << " | " << s.name << " | " << s.location 
             << " | Capacity:" << s.capacity << " | Load:" << s.load << " | Status:" << s.status << "\n";
        return;
    }
    // try name
    NameAVLNode* nnode = nameFind(nameRoot, term);
    if(nnode) {
        cout << "Found " << nnode->ids.size() << " matches for name '" << term << "':\n";
        for(auto &sid : nnode->ids) {
            IDAVLNode* sn = idFind(idRoot, sid);
            if(sn) {
                Station &s = sn->st;
                cout << s.id << " | " << s.name << " | " << s.location 
                     << " | Capacity:" << s.capacity << " | Load:" << s.load << " | Status:" << s.status << "\n";
            }
        }
        return;
    }
    cout << "Station not found.\n";
}

// Delete station (id)
void deleteStation() {
    cout << "Enter Station ID to delete: ";
    string id; getline(cin, id);
    if(id.empty()) { cout << "Empty id.\n"; return; }
    bool removed = false;
    // before deleting, find station to know its name
    IDAVLNode* node = idFind(idRoot, id);
    if(!node) { cout << "ID not found.\n"; return; }
    string name = node->st.name;
    idRoot = idDelete(idRoot, id, removed);
    // update name AVL: remove id from that name node
    bool removedKey=false;
    nameRoot = nameDelete(nameRoot, name, id, removedKey);
    // if deletion succeeded, persist
    if(removed) saveStationsToCSV();
    cout << (removed ? "Deleted.\n" : "ID not found.\n");
}

// Update load (and log)
void updateLoad() {
    cout << "Station ID: ";
    string id; getline(cin, id);
    if(id.empty()) { cout << "Empty id.\n"; return; }
    IDAVLNode* node = idFind(idRoot, id);
    if(!node) { cout << "Station not found.\n"; return; }
    Station s = node->st;
    cout << "Enter current load: ";
    string tmp; getline(cin,tmp);
    int newLoad=0;
    try { newLoad = stoi(tmp); } catch(...) { newLoad = s.load; }
    s.load = newLoad;
    if (s.load > s.capacity) cout << "WARNING: Load exceeds capacity!\n";
    updateStatus(s);
    // update node
    node->st = s;
    // log
    logLoad(s);
    // suggestion
    if(s.capacity > 0 && s.load > s.capacity*0.8) cout << "NOTE: Consider upgrading capacity.\n";
    // persist
    saveStationsToCSV();
    cout << "Load updated and logged.\n";
}

// Daily summary
void dailySummary() {
    long long totalCap = 0, totalLoad = 0;
    int low = 0, high = 0;
    vector<Station> all;
    idInorderCollect(idRoot, all);
    for(auto &s : all) {
        totalCap += s.capacity;
        totalLoad += s.load;
        if (s.capacity > 0 && s.load < s.capacity/4) low++;
        if (s.capacity > 0 && s.load > s.capacity*3/4) high++;
    }
    double util = totalCap ? (double)totalLoad*100.0/totalCap : 0.0;
    cout << "Total Stations: " << (int)all.size() << "\n";
    cout << "Total Capacity: " << totalCap << "\n";
    cout << "Total Load: " << totalLoad << "\n";
    cout << "Utilization: " << fixed << setprecision(2) << util << "%\n";
    cout << "Low Load Stations: " << low << "\n";
    cout << "High Load Stations: " << high << "\n";
}

// ---------- Sample data loader (10+ stations) ----------
void loadSampleData() {
    // Overwrite stations.csv with 10 sample stations
    ofstream out(FNAME, ios::trunc);
    out << "id,name,location,capacity,load,status\n";
    out << "S001,North Grid,North City,500,300,active\n";
    out << "S002,South Grid,South City,600,100,underutilized\n";
    out << "S003,East Hydro,East Valley,400,420,overloaded\n";
    out << "S004,West Thermal,West Town,800,700,active\n";
    out << "S005,Central Nuclear,Metro Center,1000,850,active\n";
    out << "S006,River Hydro,Riverbank,350,60,underutilized\n";
    out << "S007,Wind Farm Alpha,Hill Top,200,150,active\n";
    out << "S008,Wind Farm Beta,Coastal Line,220,50,underutilized\n";
    out << "S009,Solar Park One,Desert Zone,300,260,active\n";
    out << "S010,Solar Park Two,Desert Zone,320,40,underutilized\n";
    out.close();

    // Reset log file to just header (fresh logging for sample run)
    ofstream lg(LOGFNAME, ios::trunc);
    lg << "datetime,id,name,load\n";
    lg.close();

    // Reload AVLs from this sample CSV
    loadStationsFromCSV();
    cout << "Sample data loaded into stations.csv (10 stations).\n";
}

// ---------- Menu ----------
void menu() {
    ensureCSVHeader();

    // Load 10-sample dataset at startup (overwrites existing stations.csv)
    loadSampleData();

    // Now read that into AVLs (already done inside loadSampleData, but safe to call again)
    loadStationsFromCSV();

    while (true) {
        cout << "\n=========== POWER STATION MANAGEMENT SYSTEM (AVL Optimized) ==========\n";
        cout << "1. Add Station\n2. View Stations\n3. Search Station\n4. Delete Station\n5. Update Load\n6. Daily Summary\n7. Exit\nChoice: ";
        string choice;
        if(!getline(cin, choice)) return;
        if(choice.empty()) continue;
        int ch = 0;
        try { ch = stoi(choice); } catch(...) { ch = 0; }
        switch(ch) {
            case 1: addStation(); break;
            case 2: viewStations(); break;
            case 3: searchStation(); break;
            case 4: deleteStation(); break;
            case 5: updateLoad(); break;
            case 6: dailySummary(); break;
            case 7: return;
            default: cout << "Invalid choice\n";
        }
    }
}

int main() {
    menu();
    // cleanup
    idDeleteAll(idRoot); idRoot = nullptr;
    nameDeleteAll(nameRoot); nameRoot = nullptr;
    return 0;
}
