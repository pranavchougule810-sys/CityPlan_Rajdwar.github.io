// residential.cpp
#include <bits/stdc++.h>
using namespace std;

// ---------------------- Helpers ----------------------
int safeStoi(const string &s) {
    try { return s.empty() ? 0 : stoi(s); } catch(...) { return 0; }
}
string trim(string s) {
    while(!s.empty() && isspace((unsigned char)s.back())) s.pop_back();
    size_t i=0; while(i<s.size() && isspace((unsigned char)s[i])) ++i;
    return s.substr(i);
}
vector<string> splitCSVLine(const string &line) {
    // Simple CSV splitter (doesn't support embedded commas in quotes)
    vector<string> out;
    string cur;
    bool inquote = false;
    for(char c : line) {
        if(c == '"') { inquote = !inquote; continue; }
        if(c == ',' && !inquote) { out.push_back(trim(cur)); cur.clear(); }
        else cur.push_back(c);
    }
    out.push_back(trim(cur));
    return out;
}

// ---------------------- Types ----------------------
struct Building {
    string id;
    string name;
    string location;
    int total_units = 0;
    int occupied_units = 0;
    string status = "active";
};

struct Resident {
    string id;
    string name;
    int age = 0;
    string building_id;
    string unit_no;
};

// ---------------------- Global Data ----------------------
vector<Building> buildings;                 // indexable array
unordered_map<string,int> buildingIndex;    // building_id -> index in buildings
vector<Resident> residents;
unordered_map<string,int> residentIndex;    // resident_id -> index in residents

const string BUILDING_FILE = "buildings.csv";
const string RESIDENT_FILE = "residents.csv";

// ---------------------- Segment Tree (occupied_units) ----------------------
struct SegmentTree {
    int n;
    vector<int> tree; // sums of occupied_units
    void init(int _n=0) {
        n = _n;
        tree.assign(4*max(1,n), 0);
    }
    void build(int node, int l, int r) {
        if(l==r) {
            tree[node] = buildings[l].occupied_units;
            return;
        }
        int mid=(l+r)/2;
        build(2*node,l,mid);
        build(2*node+1,mid+1,r);
        tree[node] = tree[2*node] + tree[2*node+1];
    }
    void rebuild() {
        if(n == 0) return;
        init((int)buildings.size());
        build(1,0,n-1);
    }
    void update(int node, int l, int r, int idx, int newVal) {
        if(l==r) {
            tree[node] = newVal;
            return;
        }
        int mid=(l+r)/2;
        if(idx<=mid) update(2*node,l,mid,idx,newVal);
        else update(2*node+1,mid+1,r,idx,newVal);
        tree[node] = tree[2*node] + tree[2*node+1];
    }
    int rangeSum(int node, int l, int r, int ql, int qr) {
        if(qr<l || ql>r) return 0;
        if(ql<=l && r<=qr) return tree[node];
        int mid=(l+r)/2;
        return rangeSum(2*node,l,mid,ql,qr) + rangeSum(2*node+1,mid+1,r,ql,qr);
    }
} seg;

// ---------------------- Heaps (lazy) ----------------------
// maxHeap returns building with max occupied_units
priority_queue<pair<int,int>> maxHeap; // (occupied_units, index)
// minHeap returns building with min occupied_units
priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> minHeap;

// push current building state to heaps
void pushHeapForIndex(int idx) {
    if(idx < 0 || idx >= (int)buildings.size()) return;
    maxHeap.push({buildings[idx].occupied_units, idx});
    minHeap.push({buildings[idx].occupied_units, idx});
}

// clean top of maxHeap until matches current state
int getMaxOccupiedIndex() {
    while(!maxHeap.empty()) {
        auto p = maxHeap.top();
        int occ = p.first, idx = p.second;
        if(idx >=0 && idx < (int)buildings.size() && buildings[idx].occupied_units == occ)
            return idx;
        maxHeap.pop();
    }
    return -1;
}
int getMinOccupiedIndex() {
    while(!minHeap.empty()) {
        auto p = minHeap.top();
        int occ = p.first, idx = p.second;
        if(idx >=0 && idx < (int)buildings.size() && buildings[idx].occupied_units == occ)
            return idx;
        minHeap.pop();
    }
    return -1;
}

// ---------------------- CSV Load / Save ----------------------
void loadBuildings() {
    buildings.clear();
    buildingIndex.clear();
    ifstream in(BUILDING_FILE);
    if(!in.is_open()) {
        return;
    }
    string line;
    getline(in,line); // header
    while(getline(in,line)) {
        if(line.empty()) continue;
        auto cols = splitCSVLine(line);
        Building b;
        if(cols.size() >= 1) b.id = cols[0];
        if(cols.size() >= 2) b.name = cols[1];
        if(cols.size() >= 3) b.location = cols[2];
        if(cols.size() >= 4) b.total_units = safeStoi(cols[3]);
        if(cols.size() >= 5) b.occupied_units = safeStoi(cols[4]);
        if(cols.size() >= 6) b.status = cols[5];
        if(b.id.empty()) continue;
        buildingIndex[b.id] = (int)buildings.size();
        buildings.push_back(b);
    }
    in.close();
}

void saveBuildings() {
    ofstream out(BUILDING_FILE);
    out << "id,name,location,total_units,occupied_units,status\n";
    for(auto &b : buildings) {
        out << b.id << "," << b.name << "," << b.location << "," << b.total_units << "," << b.occupied_units << "," << b.status << "\n";
    }
    out.close();
}

void loadResidents() {
    residents.clear();
    residentIndex.clear();
    ifstream in(RESIDENT_FILE);
    if(!in.is_open()) return;
    string line;
    getline(in,line); // header
    while(getline(in,line)) {
        if(line.empty()) continue;
        auto cols = splitCSVLine(line);
        Resident r;
        if(cols.size() >= 1) r.id = cols[0];
        if(cols.size() >= 2) r.name = cols[1];
        if(cols.size() >= 3) r.age = safeStoi(cols[2]);
        if(cols.size() >= 4) r.building_id = cols[3];
        if(cols.size() >= 5) r.unit_no = cols[4];
        if(r.id.empty()) continue;
        residentIndex[r.id] = (int)residents.size();
        residents.push_back(r);
    }
    in.close();
}

void saveResidents() {
    ofstream out(RESIDENT_FILE);
    out << "id,name,age,building_id,unit_no\n";
    for(auto &r : residents) {
        out << r.id << "," << r.name << "," << r.age << "," << r.building_id << "," << r.unit_no << "\n";
    }
    out.close();
}

// ---------------------- Utilities ----------------------
int findBuildingIndex(const string &bid) {
    auto it = buildingIndex.find(bid);
    if(it == buildingIndex.end()) return -1;
    return it->second;
}
int findResidentIndex(const string &rid) {
    auto it = residentIndex.find(rid);
    if(it == residentIndex.end()) return -1;
    return it->second;
}

// ---------------------- Feature Implementations ----------------------
void rebuildAllStructures() {
    // rebuild index map
    buildingIndex.clear();
    for(int i=0;i<(int)buildings.size();++i) buildingIndex[buildings[i].id] = i;
    // rebuild segtree
    seg.n = (int)buildings.size();
    seg.rebuild();
    // rebuild heaps
    while(!maxHeap.empty()) maxHeap.pop();
    while(!minHeap.empty()) minHeap.pop();
    for(int i=0;i<(int)buildings.size();++i) pushHeapForIndex(i);
}

void viewBuildings() {
    if(buildings.empty()) { cout << "No buildings loaded.\n"; return; }
    cout << left << setw(6) << "Idx" << setw(8) << "ID" << setw(18) << "Name" << setw(12) << "Location"
         << setw(10) << "Total" << setw(10) << "Occupied" << setw(10) << "Status\n";
    cout << string(80,'-') << "\n";
    for(int i=0;i<(int)buildings.size();++i) {
        auto &b = buildings[i];
        cout << left << setw(6) << i << setw(8) << b.id << setw(18) << b.name << setw(12) << b.location
             << setw(10) << b.total_units << setw(10) << b.occupied_units << setw(10) << b.status << "\n";
    }
}

void viewResidents() {
    if(residents.empty()) { cout << "No residents loaded.\n"; return; }
    cout << left << setw(6) << "Idx" << setw(8) << "ID" << setw(18) << "Name" << setw(6) << "Age"
         << setw(8) << "BldgID" << setw(8) << "Unit\n";
    cout << string(70,'-') << "\n";
    for(int i=0;i<(int)residents.size();++i) {
        auto &r = residents[i];
        cout << left << setw(6) << i << setw(8) << r.id << setw(18) << r.name << setw(6) << r.age
             << setw(8) << r.building_id << setw(8) << r.unit_no << "\n";
    }
}

void addResidentInteractive() {
    // interactive add (also updates CSV and structures)
    Resident r;
    cout << "Enter Resident ID: "; cin >> r.id;
    if(findResidentIndex(r.id) != -1) { cout << "Resident ID already exists.\n"; return; }
    cout << "Enter Name: "; cin >> ws; getline(cin, r.name);
    cout << "Enter Age: "; cin >> r.age;
    cout << "Enter Building ID: "; cin >> r.building_id;
    int bidx = findBuildingIndex(r.building_id);
    if(bidx == -1) { cout << "Building not found.\n"; return; }
    if(buildings[bidx].occupied_units >= buildings[bidx].total_units) {
        cout << "No vacant units in this building.\n"; return;
    }
    cout << "Enter Unit Number (string): "; cin >> r.unit_no;
    // add resident
    residentIndex[r.id] = (int)residents.size();
    residents.push_back(r);
    // update building
    buildings[bidx].occupied_units += 1;
    // update segment tree & heaps
    if((int)buildings.size() > 0) seg.update(1,0,seg.n-1,bidx,buildings[bidx].occupied_units);
    pushHeapForIndex(bidx);
    saveResidents(); saveBuildings();
    cout << "Resident added and CSV updated.\n";
}

void removeResidentInteractive() {
    cout << "Enter Resident ID to remove: ";
    string rid; cin >> rid;
    int ridx = findResidentIndex(rid);
    if(ridx == -1) { cout << "Resident not found.\n"; return; }
    Resident r = residents[ridx];
    int bidx = findBuildingIndex(r.building_id);
    // remove resident (erase and update index map)
    residents.erase(residents.begin()+ridx);
    residentIndex.clear();
    for(int i=0;i<(int)residents.size();++i) residentIndex[residents[i].id] = i;
    // update building occupancy
    if(bidx != -1) {
        buildings[bidx].occupied_units = max(0, buildings[bidx].occupied_units - 1);
        if((int)buildings.size() > 0) seg.update(1,0,seg.n-1,bidx,buildings[bidx].occupied_units);
        pushHeapForIndex(bidx);
    }
    saveResidents(); saveBuildings();
    cout << "Resident removed and CSVs updated.\n";
}

void highestOccupancy() {
    int idx = getMaxOccupiedIndex();
    if(idx == -1) { cout << "No buildings or heap empty.\n"; return; }
    auto &b = buildings[idx];
    cout << "Highest occupied building: ID=" << b.id << " Name=" << b.name
         << " Occupied=" << b.occupied_units << " / " << b.total_units << "\n";
}

void lowestOccupancy() {
    int idx = getMinOccupiedIndex();
    if(idx == -1) { cout << "No buildings or heap empty.\n"; return; }
    auto &b = buildings[idx];
    cout << "Lowest occupied building: ID=" << b.id << " Name=" << b.name
         << " Occupied=" << b.occupied_units << " / " << b.total_units << "\n";
}

void rangeOccupancyQueryInteractive() {
    if(buildings.empty()) { cout << "No buildings loaded.\n"; return; }
    cout << "You can query by building index range [L..R] (indices shown in View Buildings).\n";
    cout << "Enter L index (0-based): "; int L; cin >> L;
    cout << "Enter R index (0-based): "; int R; cin >> R;
    if(L < 0) L = 0;
    if(R >= (int)buildings.size()) R = (int)buildings.size()-1;
    if(L > R) { cout << "Invalid range.\n"; return; }
    int sum = seg.rangeSum(1,0,seg.n-1,L,R);
    cout << "Total occupied units in index range [" << L << "," << R << "] is: " << sum << "\n";
    int totalCap = 0;
    for(int i=L;i<=R;++i) totalCap += buildings[i].total_units;
    cout << "Total capacity in range = " << totalCap << " => occupancy% = "
         << (totalCap ? (100.0 * sum / totalCap) : 0.0) << "%\n";
}

void reloadCSVandRebuild() {
    loadBuildings();
    loadResidents();
    seg.init((int)buildings.size());
    if(buildings.size() > 0) seg.rebuild();
    // rebuild residentIndex
    residentIndex.clear();
    for(int i=0;i<(int)residents.size();++i) residentIndex[residents[i].id] = i;
    // rebuild buildingIndex + heaps
    rebuildAllStructures();
    cout << "Reloaded CSVs and rebuilt internal structures.\n";
}

// ---------------------- Sample Data Loader (10+ each) ----------------------
void loadSampleData() {
    // 10 sample buildings
    ofstream bout(BUILDING_FILE, ios::trunc);
    bout << "id,name,location,total_units,occupied_units,status\n";
    bout << "B001,Sunrise Towers,Downtown,50,45,active\n";
    bout << "B002,RiverView Apartments,Riverside,40,10,active\n";
    bout << "B003,Green Meadows,Suburb,60,55,active\n";
    bout << "B004,LakeSide Residency,Lakeside,30,5,under_maintenance\n";
    bout << "B005,City Heights,CityCenter,80,70,active\n";
    bout << "B006,HillTop Homes,HillArea,25,8,active\n";
    bout << "B007,Metro Residency,MetroZone,100,90,active\n";
    bout << "B008,Park View,ParkSide,35,12,active\n";
    bout << "B009,Old Town Blocks,OldTown,20,18,active\n";
    bout << "B010,Tech Park Flats,ITHub,45,30,active\n";
    bout.close();

    // 10 sample residents mapped to some buildings
    ofstream rout(RESIDENT_FILE, ios::trunc);
    rout << "id,name,age,building_id,unit_no\n";
    rout << "R001,Alice,28,B001,101\n";
    rout << "R002,Bob,32,B001,102\n";
    rout << "R003,Charlie,25,B002,201\n";
    rout << "R004,David,40,B003,305\n";
    rout << "R005,Eva,22,B003,306\n";
    rout << "R006,Frank,35,B005,402\n";
    rout << "R007,Grace,30,B007,701\n";
    rout << "R008,Henry,29,B007,702\n";
    rout << "R009,Irene,27,B010,1001\n";
    rout << "R010,Jack,33,B010,1002\n";
    rout.close();

    // Reload and rebuild all structures
    loadBuildings();
    loadResidents();
    seg.init((int)buildings.size());
    if(buildings.size() > 0) seg.rebuild();
    residentIndex.clear();
    for(int i=0;i<(int)residents.size();++i) residentIndex[residents[i].id] = i;
    rebuildAllStructures();
    cout << "Sample data loaded for buildings and residents.\n";
}

// ---------------------- Menu ----------------------
void printMenu() {
    cout << "\n====== RESIDENTIAL HOUSING (CSV + SegmentTree + Heaps) ======\n";
    cout << "0. Reload CSVs from disk\n";
    cout << "1. View Buildings\n2. View Residents\n3. Add Resident (interactive)  [updates CSV]\n4. Remove Resident (interactive) [updates CSV]\n";
    cout << "5. Highest Occupancy Building\n6. Lowest Occupancy Building\n7. Range Occupancy Query (by index)\n8. Save & Exit\nChoice: ";
}

int main() {
    // Load 10-sample data at startup (overwrites existing CSVs)
    loadSampleData();

    // Menu loop
    while(true) {
        printMenu();
        int c; if(!(cin >> c)) break;
        switch(c) {
            case 0: reloadCSVandRebuild(); break;
            case 1: viewBuildings(); break;
            case 2: viewResidents(); break;
            case 3: addResidentInteractive(); break;
            case 4: removeResidentInteractive(); break;
            case 5: highestOccupancy(); break;
            case 6: lowestOccupancy(); break;
            case 7: rangeOccupancyQueryInteractive(); break;
            case 8: {
                saveResidents();
                saveBuildings();
                cout << "Saved CSVs and exiting.\n";
                return 0;
            }
            default: cout << "Invalid choice.\n";
        }
    }

    return 0;
}
