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
                cols.push_back(cur); cur.clear();
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


// ---------- Files ----------
const string OFFICE_FILE = "post_offices.csv";
const string PARCEL_FILE = "parcels.csv";
const string LOG_FILE    = "delivery_log.csv";


// ---------- Structures ----------
struct PostOffice {
    string office_id, name, location;
    int capacity;
};

struct Parcel {
    string parcel_id, sender, receiver, origin, destination, status;
    int priority; // higher priority = urgent
};


// ---------- Trie for Parcel Search ----------
struct TrieNode {
    unordered_map<char, TrieNode*> children;
    vector<Parcel*> parcels; // pointers to parcels with this prefix
};

class ParcelTrie {
public:
    ParcelTrie() { root = new TrieNode(); }
    
    void insert(const string &key, Parcel *p) {
        TrieNode *cur = root;
        for (char c : key) {
            if (!cur->children[c]) cur->children[c] = new TrieNode();
            cur = cur->children[c];
            cur->parcels.push_back(p);
        }
    }
    
    vector<Parcel*> search(const string &key) {
        TrieNode *cur = root;
        for (char c : key) {
            if (!cur->children[c]) return {};
            cur = cur->children[c];
        }
        return cur->parcels;
    }
    
private:
    TrieNode *root;
};


// ---------- Data ----------
vector<PostOffice> offices;
vector<Parcel> parcels;
ParcelTrie parcelTrie;


// ---------- Initialization ----------
void loadOffices() {
    offices.clear();
    auto rows = readCSV(OFFICE_FILE);
    if (rows.empty()) return;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i].size() < 4) continue;
        PostOffice po;
        po.office_id = rows[i][0];
        po.name      = rows[i][1];
        po.location  = rows[i][2];
        po.capacity  = stoi(rows[i][3]);
        offices.push_back(po);
    }
}

void loadParcels() {
    parcels.clear();
    parcelTrie = ParcelTrie(); // reset trie
    auto rows = readCSV(PARCEL_FILE);
    if (rows.empty()) return;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i].size() < 6) continue;
        Parcel p;
        p.parcel_id   = rows[i][0];
        p.sender      = rows[i][1];
        p.receiver    = rows[i][2];
        p.origin      = rows[i][3];
        p.destination = rows[i][4];
        p.status      = rows[i][5];
        p.priority    = (p.status == "urgent") ? 2 : 1;
        parcels.push_back(p);
        parcelTrie.insert(p.sender,   &parcels.back());
        parcelTrie.insert(p.receiver, &parcels.back());
    }
}


// ---------- Features ----------
void addOffice() {
    PostOffice po;
    cout << "Office ID: "; cin >> po.office_id;
    cout << "Name: "; cin >> po.name;
    cout << "Location: "; cin >> po.location;
    cout << "Capacity: "; cin >> po.capacity;
    offices.push_back(po);
    appendCSV(OFFICE_FILE, {po.office_id, po.name, po.location, to_string(po.capacity)});
    cout << "Post office added.\n";
}

void viewOffices() {
    cout << "Post Offices:\n";
    for (auto &o : offices)
        cout << o.office_id << " | " << o.name << " | " << o.location << " | Capacity:" << o.capacity << "\n";
}

void addParcel() {
    Parcel p;
    cout << "Parcel ID: "; cin >> p.parcel_id;
    cout << "Sender: "; cin >> p.sender;
    cout << "Receiver: "; cin >> p.receiver;
    cout << "Origin Office ID: "; cin >> p.origin;
    cout << "Destination Office ID: "; cin >> p.destination;
    cout << "Status (normal/urgent): "; cin >> p.status;
    p.priority = (p.status == "urgent") ? 2 : 1;
    parcels.push_back(p);
    appendCSV(PARCEL_FILE, {p.parcel_id,p.sender,p.receiver,p.origin,p.destination,p.status});
    parcelTrie.insert(p.sender,   &parcels.back());
    parcelTrie.insert(p.receiver, &parcels.back());
    cout << "Parcel added.\n";
}

void viewParcels() {
    cout << "Parcels:\n";
    for (auto &p : parcels)
        cout << p.parcel_id << " | " << p.sender << " -> " << p.receiver
             << " | " << p.origin << " -> " << p.destination << " | " << p.status << "\n";
}

void searchParcel() {
    string key; cout << "Enter Sender or Receiver name to search: "; cin >> key;
    auto results = parcelTrie.search(key);
    if (results.empty()) { cout << "No parcels found.\n"; return; }
    for (auto p : results)
        cout << p->parcel_id << " | " << p->sender << " -> " << p->receiver
             << " | " << p->origin << " -> " << p->destination << " | " << p->status << "\n";
}

void dispatchParcels() {
    // Priority queue: urgent first
    auto cmp = [](Parcel *a, Parcel *b){ return a->priority < b->priority; };
    priority_queue<Parcel*, vector<Parcel*>, decltype(cmp)> pq(cmp);
    for (auto &p : parcels) if(p.status != "delivered") pq.push(&p);
    cout << "Dispatching parcels (urgent first):\n";
    while(!pq.empty()) {
        Parcel *p = pq.top(); pq.pop();
        cout << p->parcel_id << " | " << p->sender << " -> " << p->receiver
             << " | " << p->origin << " -> " << p->destination
             << " | " << p->status << "\n";
        // Mark as delivered for demo
        p->status = "delivered";
    }
    // Save updates
    vector<vector<string>> rows = {{"parcel_id","sender","receiver","origin","destination","status"}};
    for (auto &p : parcels)
        rows.push_back({p.parcel_id,p.sender,p.receiver,p.origin,p.destination,p.status});
    overwriteCSV(PARCEL_FILE, rows);
}


// ---------- Sample Data Loader (10 offices + 10 parcels) ----------
void loadSampleData() {
    // Offices
    vector<vector<string>> oRows;
    oRows.push_back({"office_id","name","location","capacity"});
    oRows.push_back({"O001","Central PO","City Center","500"});
    oRows.push_back({"O002","North PO","North Zone","300"});
    oRows.push_back({"O003","South PO","South Zone","300"});
    oRows.push_back({"O004","East PO","East Zone","250"});
    oRows.push_back({"O005","West PO","West Zone","250"});
    oRows.push_back({"O006","Airport PO","Airport","400"});
    oRows.push_back({"O007","Campus PO","University","200"});
    oRows.push_back({"O008","Industrial PO","Industrial Area","350"});
    oRows.push_back({"O009","Old Town PO","Old Town","180"});
    oRows.push_back({"O010","Highway PO","Highway","220"});
    overwriteCSV(OFFICE_FILE, oRows);

    // Parcels
    vector<vector<string>> pRows;
    pRows.push_back({"parcel_id","sender","receiver","origin","destination","status"});
    pRows.push_back({"P001","Rahul","Anita","O001","O002","urgent"});
    pRows.push_back({"P002","Anita","Sanjay","O002","O003","normal"});
    pRows.push_back({"P003","Sanjay","Deepa","O003","O004","urgent"});
    pRows.push_back({"P004","Deepa","Imran","O004","O005","normal"});
    pRows.push_back({"P005","Imran","Vikram","O005","O006","urgent"});
    pRows.push_back({"P006","Vikram","Sneha","O006","O007","normal"});
    pRows.push_back({"P007","Sneha","Karan","O007","O008","urgent"});
    pRows.push_back({"P008","Karan","Pooja","O008","O009","normal"});
    pRows.push_back({"P009","Pooja","Manoj","O009","O010","urgent"});
    pRows.push_back({"P010","Manoj","Rahul","O010","O001","normal"});
    overwriteCSV(PARCEL_FILE, pRows);

    // Reload into memory and rebuild trie
    loadOffices();
    loadParcels();
    cout << "Sample data loaded for post offices and parcels.\n";
}


// ---------- Menu ----------
void menu() {
    // Load sample data at startup (overwrites existing CSVs)
    loadSampleData();

    while(true) {
        cout << "\n===== POST OFFICE & DELIVERY MANAGEMENT =====\n";
        cout << "1. Add Post Office\n2. View Post Offices\n3. Add Parcel\n4. View Parcels\n";
        cout << "5. Search Parcel by Name\n6. Dispatch Parcels\n7. Exit\nChoice: ";
        int choice; cin >> choice;
        switch(choice) {
            case 1: addOffice(); break;
            case 2: viewOffices(); break;
            case 3: addParcel(); break;
            case 4: viewParcels(); break;
            case 5: searchParcel(); break;
            case 6: dispatchParcels(); break;
            case 7: return;
            default: cout << "Invalid choice.\n";
        }
    }
}


// ---------- Main ----------
int main() {
    menu();
    return 0;
}
