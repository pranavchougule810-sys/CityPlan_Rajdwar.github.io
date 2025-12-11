// factory_csv_interactive_optimized.cpp
// Adds: Trie for material prefix search, Min-Heap for orders scheduling,
// std::map (red-black tree) for sorted worker listing,
// and loadSampleData() to generate 10+ sample rows in each CSV at startup.

#include <bits/stdc++.h>
using namespace std;

// -------------------- File names / headers --------------------
const string WORKER_FILE     = "workers.csv";
const string MACHINE_FILE    = "machines.csv";
const string MATERIAL_FILE   = "materials.csv";
const string PRODUCTION_FILE = "production.csv";
const string ORDERS_FILE     = "orders.csv";

const string WORKER_HEADER     = "worker_id,name,role,salary";
const string MACHINE_HEADER    = "machine_id,name,status";
const string MATERIAL_HEADER   = "material_name,qty";
const string PRODUCTION_HEADER = "date,item,qty";
const string ORDERS_HEADER     = "order_id,item,qty,deadline_days";

// -------------------- Helpers --------------------
string trim(const string &s) {
    size_t a = 0;
    while (a < s.size() && isspace((unsigned char)s[a])) ++a;
    size_t b = s.size();
    while (b > a && isspace((unsigned char)s[b-1])) --b;
    return s.substr(a, b-a);
}

// Robust CSV splitter: handles quoted fields (no embedded newlines assumed)
vector<string> splitCSVLine(const string &line) {
    vector<string> out;
    string cur;
    bool inquote = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') {
            inquote = !inquote;
            continue;
        }
        if (c == ',' && !inquote) {
            out.push_back(trim(cur));
            cur.clear();
        } else cur.push_back(c);
    }
    out.push_back(trim(cur));
    return out;
}

void ensureCSV(const string &fname, const string &header) {
    ifstream in(fname);
    if (!in.is_open()) {
        ofstream out(fname);
        out << header << "\n";
        out.close();
    } else {
        string line;
        if (!getline(in, line) || trim(line).empty()) {
            in.close();
            ofstream out(fname);
            out << header << "\n";
            out.close();
        } else in.close();
    }
}

// Convert string to int safely
int safeStoi(const string &s) {
    try { return s.empty() ? 0 : stoi(s); } catch(...) { return 0; }
}

// -------------------- Types --------------------
struct Worker {
    string id;
    string name;
    string role;
    string salary;
};
struct Machine {
    string id;
    string name;
    string status; // Working / Broken
};
struct Material {
    string name;
    int qty;
};
struct Production {
    string date; // DD/MM or similar
    string item;
    int qty;
};
struct Order {
    string id;
    string item;
    int qty;
    int deadline_days; // EDF uses this
};

// -------------------- In-memory storage + indices --------------------
vector<Worker> workers;
unordered_map<string,int> workerIndex;     // id -> index (fast lookup)
map<string,int> workerSortedIndex;         // id -> index (sorted, std::map -> RBT)

vector<Machine> machines;
unordered_map<string,int> machineIndex;

vector<Material> materials;
unordered_map<string,int> materialIndex;

// Trie will be built from materials for prefix search (see below)

// productions - log only
vector<Production> productions;

vector<Order> orders;
unordered_map<string,int> orderIndex;

// Min-heap for orders (deadline,min)
priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> orderMinHeap;
// note: pair<deadline_days, index>

// -------------------- Trie for materials --------------------
struct TrieNode {
    unordered_map<char, TrieNode*> next;
    bool isEnd = false;
    // we store pointers to material indices for exact retrieval of qty
    vector<int> materialIndices;
};

struct Trie {
    TrieNode *root = nullptr;
    Trie() { root = new TrieNode(); }
    void clearNode(TrieNode *n) {
        if(!n) return;
        for(auto &p : n->next) clearNode(p.second);
        delete n;
    }
    ~Trie() { clearNode(root); }

    void insert(const string &s, int matIdx) {
        TrieNode *cur = root;
        for(char ch : s) {
            char c = ch;
            if(cur->next.find(c) == cur->next.end()) cur->next[c] = new TrieNode();
            cur = cur->next[c];
        }
        cur->isEnd = true;
        cur->materialIndices.push_back(matIdx);
    }

    // collect up to limit material indices under the node
    void collect(TrieNode *node, vector<int> &out, int limit) {
        if(!node) return;
        if(node->isEnd) {
            for(int idx : node->materialIndices) {
                out.push_back(idx);
                if((int)out.size() >= limit) return;
            }
        }
        for(auto &p : node->next) {
            collect(p.second, out, limit);
            if((int)out.size() >= limit) return;
        }
    }

    // returns indices of materials matching prefix (up to limit)
    vector<int> startsWith(const string &prefix, int limit=50) {
        vector<int> res;
        TrieNode *cur = root;
        for(char ch : prefix) {
            char c = ch;
            if(cur->next.find(c) == cur->next.end()) return res;
            cur = cur->next[c];
        }
        collect(cur, res, limit);
        return res;
    }
} materialTrie;

// helper: rebuild trie from materials vector
void rebuildMaterialTrie() {
    // destroy and create new trie
    materialTrie.~Trie();
    new(&materialTrie) Trie();
    for(int i=0;i<(int)materials.size();++i) {
        string s = materials[i].name;
        // normalize a bit: lowercase for matching
        for(char &c : s) c = (char)tolower((unsigned char)c);
        materialTrie.insert(s, i);
    }
}

// -------------------- Load / Save functions (unchanged CSV format) --------------------
void loadWorkers() {
    workers.clear(); workerIndex.clear(); workerSortedIndex.clear();
    ensureCSV(WORKER_FILE, WORKER_HEADER);
    ifstream in(WORKER_FILE);
    string line;
    getline(in, line); // header
    while(getline(in, line)) {
        if(trim(line).empty()) continue;
        auto cols = splitCSVLine(line);
        if(cols.size() < 1) continue;
        Worker w;
        w.id     = cols.size()>0 ? cols[0] : "";
        w.name   = cols.size()>1 ? cols[1] : "";
        w.role   = cols.size()>2 ? cols[2] : "";
        w.salary = cols.size()>3 ? cols[3] : "";
        if(w.id.empty()) continue;
        workerIndex[w.id]       = (int)workers.size();
        workerSortedIndex[w.id] = (int)workers.size(); // sorted map
        workers.push_back(w);
    }
    in.close();
}
void saveWorkers() {
    ofstream out(WORKER_FILE);
    out << WORKER_HEADER << "\n";
    for(auto &w: workers) {
        out << w.id << "," << w.name << "," << w.role << "," << w.salary << "\n";
    }
    out.close();
}

void loadMachines() {
    machines.clear(); machineIndex.clear();
    ensureCSV(MACHINE_FILE, MACHINE_HEADER);
    ifstream in(MACHINE_FILE);
    string line; getline(in,line); // header
    while(getline(in,line)) {
        if(trim(line).empty()) continue;
        auto c = splitCSVLine(line);
        if(c.size() < 1) continue;
        Machine m;
        m.id     = c.size()>0 ? c[0] : "";
        m.name   = c.size()>1 ? c[1] : "";
        m.status = c.size()>2 ? c[2] : "Working";
        if(m.id.empty()) continue;
        machineIndex[m.id] = (int)machines.size();
        machines.push_back(m);
    }
    in.close();
}
void saveMachines() {
    ofstream out(MACHINE_FILE);
    out << MACHINE_HEADER << "\n";
    for(auto &m: machines) out << m.id << "," << m.name << "," << m.status << "\n";
    out.close();
}

void loadMaterials() {
    materials.clear(); materialIndex.clear();
    ensureCSV(MATERIAL_FILE, MATERIAL_HEADER);
    ifstream in(MATERIAL_FILE);
    string line; getline(in,line);
    while(getline(in,line)) {
        if(trim(line).empty()) continue;
        auto c = splitCSVLine(line);
        if(c.size() < 1) continue;
        Material mm;
        mm.name = c.size()>0 ? c[0] : "";
        mm.qty  = c.size()>1 ? safeStoi(c[1]) : 0;
        if(mm.name.empty()) continue;
        materialIndex[mm.name] = (int)materials.size();
        materials.push_back(mm);
    }
    in.close();
    rebuildMaterialTrie(); // rebuild trie whenever materials loaded
}
void saveMaterials() {
    ofstream out(MATERIAL_FILE);
    out << MATERIAL_HEADER << "\n";
    for(auto &m: materials) out << m.name << "," << m.qty << "\n";
    out.close();
}

void loadProductions() {
    productions.clear();
    ensureCSV(PRODUCTION_FILE, PRODUCTION_HEADER);
    ifstream in(PRODUCTION_FILE);
    string line; getline(in,line);
    while(getline(in,line)) {
        if(trim(line).empty()) continue;
        auto c = splitCSVLine(line);
        Production p;
        p.date = c.size()>0 ? c[0] : "";
        p.item = c.size()>1 ? c[1] : "";
        p.qty  = c.size()>2 ? safeStoi(c[2]) : 0;
        productions.push_back(p);
    }
    in.close();
}
void saveProductions() {
    ofstream out(PRODUCTION_FILE);
    out << PRODUCTION_HEADER << "\n";
    for(auto &p: productions) out << p.date << "," << p.item << "," << p.qty << "\n";
    out.close();
}

void loadOrders() {
    orders.clear(); orderIndex.clear();
    ensureCSV(ORDERS_FILE, ORDERS_HEADER);
    ifstream in(ORDERS_FILE);
    string line; getline(in,line);
    while(getline(in,line)) {
        if(trim(line).empty()) continue;
        auto c = splitCSVLine(line);
        if(c.size() < 1) continue;
        Order o;
        o.id            = c.size()>0 ? c[0] : "";
        o.item          = c.size()>1 ? c[1] : "";
        o.qty           = c.size()>2 ? safeStoi(c[2]) : 0;
        o.deadline_days = c.size()>3 ? safeStoi(c[3]) : INT_MAX;
        if(o.id.empty()) continue;
        orderIndex[o.id] = (int)orders.size();
        orders.push_back(o);
    }
    in.close();
    // rebuild heap after loading
    while(!orderMinHeap.empty()) orderMinHeap.pop();
    for(int i=0;i<(int)orders.size();++i) orderMinHeap.push({orders[i].deadline_days, i});
}
void saveOrders() {
    ofstream out(ORDERS_FILE);
    out << ORDERS_HEADER << "\n";
    for(auto &o: orders) out << o.id << "," << o.item << "," << o.qty << "," << o.deadline_days << "\n";
    out.close();
}

// -------------------- CRUD + Menu Operations --------------------
void addWorkerInteractive() {
    Worker w;
    cout << "Enter Worker ID: "; cin >> w.id;
    if(workerIndex.find(w.id) != workerIndex.end()) { cout << "Worker ID already exists.\n"; return; }
    cout << "Enter Name: "; cin >> ws; getline(cin, w.name);
    cout << "Enter Role: "; cin >> ws; getline(cin, w.role);
    cout << "Enter Salary: "; cin >> w.salary;
    workerIndex[w.id]       = (int)workers.size();
    workerSortedIndex[w.id] = (int)workers.size();
    workers.push_back(w);
    saveWorkers();
    cout << "Worker added and saved to CSV.\n";
}

void viewWorkers() {
    cout << "\n--- WORKERS LIST ---\n";
    if(workers.empty()) { cout << "No workers.\n"; return; }
    for(size_t i=0;i<workers.size();++i) {
        auto &w=workers[i];
        cout << "ID: " << w.id << " | Name: " << w.name << " | Role: " << w.role << " | Salary: " << w.salary << "\n";
    }
}

void viewWorkersSorted() {
    cout << "\n--- WORKERS LIST (SORTED BY ID) ---\n";
    if(workerSortedIndex.empty()) { cout << "No workers.\n"; return; }
    for(auto &p : workerSortedIndex) {
        int idx = p.second;
        if(idx >= 0 && idx < (int)workers.size()) {
            auto &w = workers[idx];
            cout << "ID: " << w.id << " | Name: " << w.name << " | Role: " << w.role << " | Salary: " << w.salary << "\n";
        }
    }
}

void searchWorker() {
    string id; cout << "Enter Worker ID to search: "; cin >> id;
    auto it = workerIndex.find(id);
    if(it==workerIndex.end()) { cout << "Worker not found.\n"; return; }
    auto &w = workers[it->second];
    cout << "FOUND → ID: " << w.id << " | Name: " << w.name << " | Role: " << w.role << " | Salary: " << w.salary << "\n";
}

void deleteWorker() {
    string id; cout << "Enter Worker ID to delete: "; cin >> id;
    auto it = workerIndex.find(id);
    if(it==workerIndex.end()) { cout << "Worker not found.\n"; return; }
    int idx = it->second;
    workers.erase(workers.begin() + idx);
    // rebuild index maps
    workerIndex.clear();
    workerSortedIndex.clear();
    for(size_t i=0;i<workers.size();++i) {
        workerIndex[workers[i].id]       = (int)i;
        workerSortedIndex[workers[i].id] = (int)i;
    }
    saveWorkers();
    cout << "Worker deleted and CSV updated.\n";
}

// Machines
void addMachineInteractive() {
    Machine m;
    cout << "Machine ID: "; cin >> m.id;
    if(machineIndex.find(m.id) != machineIndex.end()) { cout << "Machine ID exists.\n"; return; }
    cout << "Machine Name: "; cin >> ws; getline(cin, m.name);
    m.status = "Working";
    machineIndex[m.id] = (int)machines.size();
    machines.push_back(m);
    saveMachines();
    cout << "Machine added.\n";
}
void viewMachines() {
    cout << "\n--- MACHINES ---\n";
    for(auto &m: machines) cout << "ID: " << m.id << " | Name: " << m.name << " | Status: " << m.status << "\n";
}
void reportBreakdownInteractive() {
    string id; cout << "Enter Machine ID: "; cin >> id;
    auto it = machineIndex.find(id);
    if(it==machineIndex.end()) { cout << "Machine not found.\n"; return; }
    machines[it->second].status = "Broken";
    saveMachines();
    cout << "Breakdown reported and saved.\n";
}
void assignMechanicInteractive() {
    string id, mech; cout << "Enter Machine ID: "; cin >> id;
    cout << "Assign Mechanic Name: "; cin >> ws; getline(cin, mech);
    if(machineIndex.find(id)==machineIndex.end()) { cout << "Machine not found.\n"; return; }
    cout << "Mechanic " << mech << " assigned to Machine " << id << " (Greedy assignment).\n";
}

// Materials
void addMaterialInteractive() {
    string name; int qty;
    cout << "Material Name: "; cin >> ws; getline(cin, name);
    cout << "Quantity: "; cin >> qty;
    auto it = materialIndex.find(name);
    if(it==materialIndex.end()) {
        materialIndex[name] = (int)materials.size();
        materials.push_back({name, qty});
    } else {
        materials[it->second].qty += qty;
    }
    saveMaterials();
    rebuildMaterialTrie();
    cout << "Material added/updated.\n";
}
void viewMaterial() {
    cout << "\n--- MATERIAL STOCK ---\n";
    for(auto &m: materials) cout << m.name << " : " << m.qty << "\n";
}

// New: prefix search (Trie) for materials
void materialPrefixSearchInteractive() {
    cout << "Enter prefix to search materials: ";
    string pref; cin >> ws; getline(cin, pref);
    string q = pref;
    for(char &c : q) c = (char)tolower((unsigned char)c);
    auto indices = materialTrie.startsWith(q, 50);
    if(indices.empty()) {
        cout << "No materials found with prefix \"" << pref << "\"\n";
        return;
    }
    cout << "Matches:\n";
    for(int idx : indices) {
        if(idx >= 0 && idx < (int)materials.size()) {
            cout << materials[idx].name << " : " << materials[idx].qty << "\n";
        }
    }
}

// Production
void addProductionInteractive() {
    Production p;
    cout << "Enter Date (DD/MM): "; cin >> p.date;
    cout << "Item Produced: "; cin >> ws; getline(cin, p.item);
    cout << "Quantity: "; cin >> p.qty;
    productions.push_back(p);
    saveProductions();
    cout << "Production record saved.\n";
}
void viewProduction() {
    cout << "\n--- PRODUCTION RECORDS ---\n";
    for(auto &p: productions) cout << "Date: " << p.date << " | Item: " << p.item << " | Qty: " << p.qty << "\n";
}

// Orders
void addOrderInteractive() {
    Order o; cout << "Order ID: "; cin >> o.id;
    if(orderIndex.find(o.id) != orderIndex.end()) { cout << "Order ID exists.\n"; return; }
    cout << "Item: "; cin >> ws; getline(cin, o.item);
    cout << "Quantity: "; cin >> o.qty;
    cout << "Deadline (in days): "; cin >> o.deadline_days;
    orderIndex[o.id] = (int)orders.size();
    orders.push_back(o);
    saveOrders();
    // push into heap
    orderMinHeap.push({o.deadline_days, (int)orders.size()-1});
    cout << "Order added.\n";
}

// Old: EDF by sorting (keeps available)
void scheduleOrdersEDF() {
    auto copyOrders = orders;
    sort(copyOrders.begin(), copyOrders.end(), [](const Order &a, const Order &b){
        return a.deadline_days < b.deadline_days;
    });
    cout << "\n--- ORDER SCHEDULE (EDF - sort) ---\n";
    for(auto &o: copyOrders) cout << "Order " << o.id << " | Item: " << o.item << " | Deadline: " << o.deadline_days << " days\n";
}

// New: schedule using Min-Heap (efficient for dynamic streaming workloads)
// We'll pop from a local copy of heap to avoid destroying global heap
void scheduleOrdersHeap() {
    if(orders.empty()) { cout << "No orders.\n"; return; }
    auto heapCopy = orderMinHeap;
    cout << "\n--- ORDER SCHEDULE (Min-Heap) ---\n";
    // We must verify lazily that top entries are still valid (index in range and matching deadline).
    while(!heapCopy.empty()) {
        auto p = heapCopy.top(); heapCopy.pop();
        int dl = p.first; int idx = p.second;
        if(idx < 0 || idx >= (int)orders.size()) continue;
        // double-check consistency: orderIndex[orders[idx].id] might have changed after deletions;
        if(orderIndex.find(orders[idx].id) == orderIndex.end()) continue;
        // Also ensure deadlines match; if the order was updated, ensure consistency
        if(orders[idx].deadline_days != dl) continue;
        auto &o = orders[idx];
        cout << "Order " << o.id << " | Item: " << o.item << " | Deadline: " << o.deadline_days << " days\n";
    }
}

// delete order
void deleteOrderInteractive() {
    string id; cout << "Order ID to delete: "; cin >> id;
    auto it = orderIndex.find(id);
    if(it==orderIndex.end()) { cout << "Order not found.\n"; return; }
    int idx = it->second;
    orders.erase(orders.begin() + idx);
    // rebuild index and heap
    orderIndex.clear();
    while(!orderMinHeap.empty()) orderMinHeap.pop();
    for(size_t i=0;i<orders.size();++i) {
        orderIndex[orders[i].id] = (int)i;
        orderMinHeap.push({orders[i].deadline_days, (int)i});
    }
    saveOrders();
    cout << "Order deleted and CSV updated.\n";
}

// reload all CSV data
void reloadAll() {
    loadWorkers(); loadMachines(); loadMaterials(); loadProductions(); loadOrders();
    cout << "Reloaded CSVs and rebuilt internal structures.\n";
}

// -------------------- Sample data loader (10+ rows each) --------------------
void loadSampleData() {
    // Workers (10 sample workers)
    {
        ofstream out(WORKER_FILE);
        out << WORKER_HEADER << "\n";
        out << "W001,Alice,Operator,30000\n";
        out << "W002,Bob,Supervisor,45000\n";
        out << "W003,Charlie,Manager,60000\n";
        out << "W004,David,Operator,32000\n";
        out << "W005,Eva,Accountant,40000\n";
        out << "W006,Frank,Technician,35000\n";
        out << "W007,Grace,HR,38000\n";
        out << "W008,Henry,Operator,31000\n";
        out << "W009,Irene,Quality Engineer,42000\n";
        out << "W010,Jack,Storekeeper,28000\n";
        out.close();
    }

    // Machines (10 sample machines)
    {
        ofstream out(MACHINE_FILE);
        out << MACHINE_HEADER << "\n";
        out << "M001,Lathe,Working\n";
        out << "M002,Drill,Broken\n";
        out << "M003,Conveyor,Working\n";
        out << "M004,Milling,Working\n";
        out << "M005,Press,Working\n";
        out << "M006,CNC,Broken\n";
        out << "M007,Welder,Working\n";
        out << "M008,Grinder,Working\n";
        out << "M009,Paint Booth,Working\n";
        out << "M010,Packing Line,Broken\n";
        out.close();
    }

    // Materials (10 sample materials)
    {
        ofstream out(MATERIAL_FILE);
        out << MATERIAL_HEADER << "\n";
        out << "Steel Rod,120\n";
        out << "Copper Wire,200\n";
        out << "Plastic Sheet,500\n";
        out << "Aluminum Plate,150\n";
        out << "Rubber Belt,80\n";
        out << "Glass Panel,60\n";
        out << "Screw Pack,1000\n";
        out << "Bolt Pack,800\n";
        out << "Paint Bucket,40\n";
        out << "Lubricant Oil,90\n";
        out.close();
    }

    // Production (10 sample production records)
    {
        ofstream out(PRODUCTION_FILE);
        out << PRODUCTION_HEADER << "\n";
        out << "01/12,WidgetA,100\n";
        out << "02/12,WidgetB,150\n";
        out << "03/12,WidgetA,80\n";
        out << "04/12,Gear,60\n";
        out << "05/12,Shaft,70\n";
        out << "06/12,WidgetC,90\n";
        out << "07/12,Bracket,110\n";
        out << "08/12,WidgetA,95\n";
        out << "09/12,WidgetB,130\n";
        out << "10/12,Casing,75\n";
        out.close();
    }

    // Orders (10 sample orders)
    {
        ofstream out(ORDERS_FILE);
        out << ORDERS_HEADER << "\n";
        out << "O001,WidgetA,50,3\n";
        out << "O002,WidgetB,70,1\n";
        out << "O003,WidgetA,30,5\n";
        out << "O004,Gear,40,2\n";
        out << "O005,Shaft,25,4\n";
        out << "O006,WidgetC,60,6\n";
        out << "O007,Bracket,35,2\n";
        out << "O008,WidgetB,45,7\n";
        out << "O009,Casing,20,5\n";
        out << "O010,WidgetA,55,1\n";
        out.close();
    }

    // Reload everything into memory (also rebuilds Trie and heap)
    reloadAll();
    cout << "Sample data (10 rows per CSV) loaded into all CSV files.\n";
}

// -------------------- Main menu --------------------
void printMenu() {
    cout << "\n========== FACTORY MANAGEMENT SYSTEM (OPTIMIZED) ==========\n";
    cout << "0. Reload CSVs from disk\n";
    cout << "1. Add Worker\n2. View Workers\n3. View Workers (Sorted by ID - RBT)\n4. Search Worker\n5. Delete Worker\n";
    cout << "6. Add Machine\n7. View Machines\n8. Report Breakdown\n9. Assign Mechanic\n";
    cout << "10. Add Material\n11. View Material\n12. Search Materials by Prefix (Trie)\n";
    cout << "13. Add Production\n14. View Production\n";
    cout << "15. Add Order\n16. Delete Order\n17. Schedule Orders (Min-Heap)\n18. Schedule Orders (Sort - EDF)\n";
    cout << "19. Exit\n";
    cout << "Enter choice: ";
}

int main() {
    // Ensure CSVs exist (headers)
    ensureCSV(WORKER_FILE, WORKER_HEADER);
    ensureCSV(MACHINE_FILE, MACHINE_HEADER);
    ensureCSV(MATERIAL_FILE, MATERIAL_HEADER);
    ensureCSV(PRODUCTION_FILE, PRODUCTION_HEADER);
    ensureCSV(ORDERS_FILE, ORDERS_HEADER);

    // Fill with sample data (overwrites any existing data) and load into memory
    loadSampleData();

    int ch;
    while (true) {
        printMenu();
        if(!(cin >> ch)) { cout << "Invalid input. Exiting.\n"; break; }
        switch(ch) {
            case 0: reloadAll(); break;
            case 1: addWorkerInteractive(); break;
            case 2: viewWorkers(); break;
            case 3: viewWorkersSorted(); break;
            case 4: searchWorker(); break;
            case 5: deleteWorker(); break;
            case 6: addMachineInteractive(); break;
            case 7: viewMachines(); break;
            case 8: reportBreakdownInteractive(); break;
            case 9: assignMechanicInteractive(); break;
            case 10: addMaterialInteractive(); break;
            case 11: viewMaterial(); break;
            case 12: materialPrefixSearchInteractive(); break;
            case 13: addProductionInteractive(); break;
            case 14: viewProduction(); break;
            case 15: addOrderInteractive(); break;
            case 16: deleteOrderInteractive(); break;
            case 17: scheduleOrdersHeap(); break;
            case 18: scheduleOrdersEDF(); break;
            case 19: cout << "Exiting. Bye.\n"; return 0;
            default: cout << "Invalid Choice\n";
        }
    }
    return 0;
}
