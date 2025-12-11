#include <bits/stdc++.h>
using namespace std;


// ---------- Structure ----------
struct Lake {
    string id, name, location, status;
    double area;          // in sq. km
    double waterQuality;  // 0 to 100 scale
    bool recreational;    // true if recreational facilities exist
};


// ---------- Data ----------
vector<Lake> lakes;
const string LAKE_FILE = "lakes.csv";


// ---------- CSV Helpers ----------
void loadLakes() {
    lakes.clear();
    ifstream in(LAKE_FILE);
    if (!in.is_open()) return;
    string line; getline(in, line); // skip header
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line); string cell;
        Lake l;
        if (!getline(ss, l.id, ',')) continue;
        if (!getline(ss, l.name, ',')) continue;
        if (!getline(ss, l.location, ',')) continue;
        if (!getline(ss, cell, ',')) continue; l.area = stod(cell);
        if (!getline(ss, cell, ',')) continue; l.waterQuality = stod(cell);
        if (!getline(ss, cell, ',')) continue; l.recreational = (cell=="1");
        if (!getline(ss, l.status, ',')) l.status="active";
        lakes.push_back(l);
    }
    in.close();
}

void saveLakes() {
    ofstream out(LAKE_FILE);
    out << "id,name,location,area,waterQuality,recreational,status\n";
    for(auto &l: lakes)
        out << l.id << "," << l.name << "," << l.location << "," << l.area << ","
            << l.waterQuality << "," << (l.recreational?"1":"0") << "," << l.status << "\n";
    out.close();
}


// ---------- Features ----------
void addLake() {
    Lake l;
    cout << "Lake ID: "; cin >> l.id;
    cout << "Name: "; cin >> l.name;
    cout << "Location: "; cin >> l.location;
    cout << "Area (sq.km): "; cin >> l.area;
    cout << "Water Quality (0-100): "; cin >> l.waterQuality;
    cout << "Has recreational facilities? (1=Yes,0=No): "; cin >> l.recreational;
    l.status = "active";
    lakes.push_back(l);
    saveLakes();
    cout << "Lake added successfully.\n";
}

void viewLakes() {
    if(lakes.empty()){ cout << "No lakes.\n"; return; }
    cout << left << setw(5) << "ID" << setw(15) << "Name" << setw(15) << "Location"
         << setw(8) << "Area" << setw(15) << "Water Quality" << setw(12) << "Recreational"
         << setw(10) << "Status\n";
    cout << string(80,'-') << "\n";
    for(auto &l: lakes)
        cout << left << setw(5) << l.id << setw(15) << l.name << setw(15) << l.location
             << setw(8) << l.area << setw(15) << l.waterQuality << setw(12) 
             << (l.recreational?"Yes":"No") << setw(10) << l.status << "\n";
}

// 1️⃣ Search Lake
void searchLake() {
    string key; cout << "Enter Lake ID or Name: "; cin >> key;
    for(auto &l: lakes) {
        if(l.id == key || l.name == key){
            cout << "Found: " << l.id << " | " << l.name << " | " << l.location
                 << " | Area: " << l.area << " sq.km"
                 << " | Water Quality: " << l.waterQuality
                 << " | Recreational: " << (l.recreational?"Yes":"No")
                 << " | Status: " << l.status << "\n";
            return;
        }
    }
    cout << "Lake not found.\n";
}

// 2️⃣ Daily Water Quality Summary
void dailySummary() {
    if(lakes.empty()){ cout << "No lakes.\n"; return; }
    double totalQuality=0; int count=0, good=0, poor=0;
    for(auto &l: lakes){
        totalQuality += l.waterQuality;
        count++;
        if(l.waterQuality >= 75) good++;
        else if(l.waterQuality < 40) poor++;
    }
    double avgQuality = (count>0) ? totalQuality/count : 0;
    cout << "Total Lakes: " << count << "\n";
    cout << "Average Water Quality: " << fixed << setprecision(2) << avgQuality << "\n";
    cout << "Good Quality Lakes (>=75): " << good << "\n";
    cout << "Poor Quality Lakes (<40): " << poor << "\n";
}


// ---------- Sample Data Loader (10 lakes) ----------
void loadSampleData() {
    ofstream out(LAKE_FILE, ios::trunc);
    out << "id,name,location,area,waterQuality,recreational,status\n";
    out << "L001,BlueLake,NorthSide,3.5,82.5,1,active\n";
    out << "L002,GreenLake,EastPark,1.8,76.0,1,active\n";
    out << "L003,SilverLake,CityOutskirts,5.2,65.0,0,active\n";
    out << "L004,CrystalLake,WestHill,2.1,90.0,1,active\n";
    out << "L005,LotusLake,SouthernValley,4.0,38.5,0,polluted\n";
    out << "L006,SunsetLake,ResortArea,2.7,88.0,1,active\n";
    out << "L007,MirrorLake,ForestEdge,1.2,55.0,0,active\n";
    out << "L008,RainbowLake,AdventurePark,3.0,79.5,1,active\n";
    out << "L009,OldLake,OldTown,6.5,35.0,0,under_cleanup\n";
    out << "L010,HeritageLake,CityCenter,2.3,72.0,1,active\n";
    out.close();

    loadLakes();
    cout << "Sample data loaded for lakes.\n";
}


// ---------- Menu ----------
void menu() {
    // Load sample data at startup (overwrites existing CSV)
    loadSampleData();

    while(true){
        cout << "\n===== WATER BODY / LAKE MANAGEMENT =====\n";
        cout << "1. Add Lake\n2. View Lakes\n3. Search Lake\n4. Daily Water Quality Summary\n5. Exit\nChoice: ";
        int c; cin >> c;
        switch(c){
            case 1: addLake(); break;
            case 2: viewLakes(); break;
            case 3: searchLake(); break;
            case 4: dailySummary(); break;
            case 5: return;
            default: cout << "Invalid choice.\n";
        }
    }
}


// ---------- Main ----------
int main(){
    menu();
    return 0;
}
