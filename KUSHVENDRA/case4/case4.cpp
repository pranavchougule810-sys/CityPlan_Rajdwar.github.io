#include <bits/stdc++.h>
using namespace std;

// ---------- Helper ----------
int safeStoi(const string &s) {
    try { return s.empty() ? 0 : stoi(s); }
    catch(...) { return 0; }
}

// ---------- Structures ----------
struct MetroStation {
    string id, name, location, status;
    int capacity, daily_ridership;
};

struct BusRoute {
    string route_id, source, destination;
    int distance_km;
};

struct Vehicle {
    string vehicle_id, type, driver_name;
    int capacity;
};

// ---------- Trie for Station Search ----------
struct TrieNode {
    unordered_map<char, TrieNode*> children;
    vector<int> stationIndices; // indices in stations vector
};

TrieNode* root = new TrieNode();

// ---------- Data ----------
vector<MetroStation> stations;
vector<BusRoute> routes;
vector<Vehicle> vehicles;

// ---------- File Names ----------
const string STATION_FILE = "metro_stations.csv";
const string ROUTE_FILE   = "metro_routes.csv";
const string VEHICLE_FILE = "metro_vehicles.csv";

// ---------- CSV Helpers ----------
void loadStations() {
    stations.clear();
    ifstream in(STATION_FILE);
    if (!in.is_open()) return;
    string line; getline(in, line); // skip header
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line); string cell;
        MetroStation s;
        if (!getline(ss, s.id, ',')) continue;
        if (!getline(ss, s.name, ',')) continue;
        if (!getline(ss, s.location, ',')) continue;
        if (!getline(ss, cell, ',')) continue; s.capacity = safeStoi(cell);
        if (!getline(ss, cell, ',')) continue; s.daily_ridership = safeStoi(cell);
        if (!getline(ss, s.status, ',')) s.status = "active";
        if (s.id.empty() || s.name.empty()) continue;
        stations.push_back(s);
    }
    in.close();

    // Build Trie
    root = new TrieNode();
    for (int i = 0; i < (int)stations.size(); ++i) {
        string key = stations[i].id + " " + stations[i].name;
        TrieNode* node = root;
        for (char c : key) {
            if (!node->children.count(c)) node->children[c] = new TrieNode();
            node = node->children[c];
            node->stationIndices.push_back(i);
        }
    }
}

void saveStations() {
    ofstream out(STATION_FILE);
    out << "id,name,location,capacity,daily_ridership,status\n";
    for (auto &s : stations)
        out << s.id << "," << s.name << "," << s.location << ","
            << s.capacity << "," << s.daily_ridership << "," << s.status << "\n";
    out.close();
}

void loadRoutes() {
    routes.clear();
    ifstream in(ROUTE_FILE);
    if (!in.is_open()) return;
    string line; getline(in, line);
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line); string cell;
        BusRoute r;
        if (!getline(ss, r.route_id, ',')) continue;
        if (!getline(ss, r.source, ',')) continue;
        if (!getline(ss, r.destination, ',')) continue;
        if (!getline(ss, cell, ',')) continue; r.distance_km = safeStoi(cell);
        routes.push_back(r);
    }
    in.close();
}

void saveRoutes() {
    ofstream out(ROUTE_FILE);
    out << "route_id,source,destination,distance_km\n";
    for (auto &r : routes)
        out << r.route_id << "," << r.source << "," << r.destination << "," << r.distance_km << "\n";
    out.close();
}

void loadVehicles() {
    vehicles.clear();
    ifstream in(VEHICLE_FILE);
    if (!in.is_open()) return;
    string line; getline(in, line);
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line); string cell;
        Vehicle v;
        if (!getline(ss, v.vehicle_id, ',')) continue;
        if (!getline(ss, v.type, ',')) continue;
        if (!getline(ss, v.driver_name, ',')) continue;
        if (!getline(ss, cell, ',')) continue; v.capacity = safeStoi(cell);
        vehicles.push_back(v);
    }
    in.close();
}

void saveVehicles() {
    ofstream out(VEHICLE_FILE);
    out << "vehicle_id,type,driver_name,capacity\n";
    for (auto &v : vehicles)
        out << v.vehicle_id << "," << v.type << "," << v.driver_name << "," << v.capacity << "\n";
    out.close();
}

// ---------- Features ----------

// Add & View
void addStation() {
    MetroStation s;
    cout << "Enter Station ID: "; cin >> s.id;
    cout << "Enter Name: "; cin >> s.name;
    cout << "Enter Location: "; cin >> s.location;
    cout << "Enter Capacity: "; cin >> s.capacity;
    s.daily_ridership = 0; s.status = "active";
    stations.push_back(s);
    saveStations();
    cout << "Station added.\n";

    // Update Trie
    string key = s.id + " " + s.name;
    TrieNode* node = root;
    for (char c : key) {
        if (!node->children.count(c)) node->children[c] = new TrieNode();
        node = node->children[c];
        node->stationIndices.push_back((int)stations.size() - 1);
    }
}

void viewStations() {
    if(stations.empty()){ cout << "No stations.\n"; return; }
    cout << left << setw(5) << "ID" << setw(12) << "Name" << setw(12) << "Location"
         << setw(10) << "Capacity" << setw(15) << "Daily Ridership" << setw(10) << "Status\n";
    cout << string(64,'-') << "\n";
    for(auto &s : stations)
        cout << left << setw(5) << s.id << setw(12) << s.name << setw(12) << s.location
             << setw(10) << s.capacity << setw(15) << s.daily_ridership << setw(10) << s.status << "\n";
}

void addRoute() {
    BusRoute r;
    cout << "Route ID: "; cin >> r.route_id;
    cout << "Source Station: "; cin >> r.source;
    cout << "Destination Station: "; cin >> r.destination;
    cout << "Distance (km): "; cin >> r.distance_km;
    routes.push_back(r); saveRoutes();
    cout << "Route added.\n";
}

void viewRoutes() {
    if(routes.empty()){ cout << "No routes.\n"; return; }
    cout << left << setw(8) << "RouteID" << setw(12) << "Source" << setw(12) << "Destination" << setw(10) << "Distance\n";
    cout << string(42,'-') << "\n";
    for(auto &r : routes)
        cout << left << setw(8) << r.route_id << setw(12) << r.source << setw(12) << r.destination
             << setw(10) << r.distance_km << "\n";
}

void addVehicle() {
    Vehicle v;
    cout << "Vehicle ID: "; cin >> v.vehicle_id;
    cout << "Type: "; cin >> v.type;
    cout << "Driver Name: "; cin >> v.driver_name;
    cout << "Capacity: "; cin >> v.capacity;
    vehicles.push_back(v); saveVehicles();
    cout << "Vehicle added.\n";
}

void viewVehicles() {
    if(vehicles.empty()){ cout << "No vehicles.\n"; return; }
    cout << left << setw(10) << "VehicleID" << setw(12) << "Type" << setw(15) << "Driver" << setw(10) << "Capacity\n";
    cout << string(47,'-') << "\n";
    for(auto &v : vehicles)
        cout << left << setw(10) << v.vehicle_id << setw(12) << v.type << setw(15) << v.driver_name
             << setw(10) << v.capacity << "\n";
}

// ---------- New Features ----------

// Trie-based Search
void searchStation() {
    if(stations.empty()){ cout << "No stations.\n"; return; }
    string key; cout << "Enter Station ID or Name: "; cin >> key;
    TrieNode* node = root;
    for(char c : key) {
        if(!node->children.count(c)) {
            cout << "Station not found.\n";
            return;
        }
        node = node->children[c];
    }
    if(node->stationIndices.empty()) { cout << "Station not found.\n"; return; }
    cout << "Stations found:\n";
    for(int idx : node->stationIndices) {
        auto &s = stations[idx];
        cout << s.id << " | " << s.name << " | " << s.location
             << " | Capacity: " << s.capacity
             << " | Daily Ridership: " << s.daily_ridership
             << " | " << s.status << "\n";
    }
}

// Ridership Summary
void ridershipSummary() {
    int total = 0;
    for(auto &s : stations) total += s.daily_ridership;
    cout << "Total Daily Ridership: " << total << "\n";
}

// Delete & Update
void deleteStation() {
    if(stations.empty()){ cout << "No stations.\n"; return; }
    string id; cout << "Enter Station ID to delete: "; cin >> id;
    auto it = remove_if(stations.begin(), stations.end(), [&](MetroStation &s){ return s.id == id; });
    if(it != stations.end()){
        stations.erase(it, stations.end());
        saveStations(); cout << "Station deleted.\n";
    } else cout << "Station not found.\n";
}

void deleteRoute() {
    if(routes.empty()){ cout << "No routes.\n"; return; }
    string id; cout << "Enter Route ID to delete: "; cin >> id;
    auto it = remove_if(routes.begin(), routes.end(), [&](BusRoute &r){ return r.route_id == id; });
    if(it != routes.end()){
        routes.erase(it, routes.end());
        saveRoutes(); cout << "Route deleted.\n";
    } else cout << "Route not found.\n";
}

void deleteVehicle() {
    if(vehicles.empty()){ cout << "No vehicles.\n"; return; }
    string id; cout << "Enter Vehicle ID to delete: "; cin >> id;
    auto it = remove_if(vehicles.begin(), vehicles.end(), [&](Vehicle &v){ return v.vehicle_id == id; });
    if(it != vehicles.end()){
        vehicles.erase(it, vehicles.end());
        saveVehicles(); cout << "Vehicle deleted.\n";
    } else cout << "Vehicle not found.\n";
}

void updateStation() {
    if(stations.empty()){ cout << "No stations.\n"; return; }
    string id; cout << "Enter Station ID to update: "; cin >> id;
    for(auto &s : stations){
        if(s.id == id){
            cout << "New Name: "; cin >> s.name;
            cout << "New Location: "; cin >> s.location;
            cout << "New Capacity: "; cin >> s.capacity;
            saveStations(); cout << "Station updated.\n"; return;
        }
    }
    cout << "Station not found.\n";
}

// ---------- Sample Data Loader (10+ rows each) ----------
void loadSampleData() {
    // Sample stations
    ofstream stOut(STATION_FILE, ios::trunc);
    stOut << "id,name,location,capacity,daily_ridership,status\n";
    stOut << "S001,Central,CityCenter,50000,42000,active\n";
    stOut << "S002,NorthGate,NorthSide,30000,8000,underutilized\n";
    stOut << "S003,SouthEnd,SouthSide,28000,26000,active\n";
    stOut << "S004,EastPark,EastSide,25000,6000,underutilized\n";
    stOut << "S005,WestSquare,WestSide,27000,29000,overloaded\n";
    stOut << "S006,Airport,Outskirts,40000,35000,active\n";
    stOut << "S007,TechPark,ITZone,22000,15000,active\n";
    stOut << "S008,Stadium,SportsArea,26000,24000,active\n";
    stOut << "S009,Riverside,RiverBank,18000,3000,underutilized\n";
    stOut << "S010,OldTown,Heritage,20000,12000,active\n";
    stOut.close();

    // Sample routes
    ofstream rtOut(ROUTE_FILE, ios::trunc);
    rtOut << "route_id,source,destination,distance_km\n";
    rtOut << "R001,Central,NorthGate,12\n";
    rtOut << "R002,Central,SouthEnd,10\n";
    rtOut << "R003,Central,EastPark,9\n";
    rtOut << "R004,Central,WestSquare,11\n";
    rtOut << "R005,Airport,Central,18\n";
    rtOut << "R006,TechPark,Central,14\n";
    rtOut << "R007,Stadium,Central,8\n";
    rtOut << "R008,OldTown,Central,7\n";
    rtOut << "R009,Riverside,Central,13\n";
    rtOut << "R010,NorthGate,Airport,20\n";
    rtOut.close();

    // Sample vehicles
    ofstream vhOut(VEHICLE_FILE, ios::trunc);
    vhOut << "vehicle_id,type,driver_name,capacity\n";
    vhOut << "V001,MetroTrain,Rahul,1000\n";
    vhOut << "V002,MetroTrain,Anita,900\n";
    vhOut << "V003,Bus,Sanjay,60\n";
    vhOut << "V004,Bus,Deepa,50\n";
    vhOut << "V005,Bus,Imran,55\n";
    vhOut << "V006,Shuttle,Vikram,30\n";
    vhOut << "V007,Shuttle,Sneha,28\n";
    vhOut << "V008,Tram,Karan,120\n";
    vhOut << "V009,Tram,Pooja,110\n";
    vhOut << "V010,ServiceVan,Manoj,15\n";
    vhOut.close();

    // Reload all into memory and rebuild Trie
    loadStations();
    loadRoutes();
    loadVehicles();
    cout << "Sample data loaded for stations, routes, and vehicles.\n";
}

// ---------- Menu ----------
void menu() {
    // Load 10+ sample data at startup (overwrites existing CSVs)
    loadSampleData();

    while(true){
        cout << "\n========== METRO & PUBLIC TRANSPORT MANAGEMENT SYSTEM ==========\n";
        cout << "1.Add Station\n2.View Stations\n3.Add Route\n4.View Routes\n";
        cout << "5.Add Vehicle\n6.View Vehicles\n7.Search Station\n8.Ridership Summary\n";
        cout << "9.Delete Station/Route/Vehicle\n10.Update Station Details\n11.Exit\nChoice: ";
        int choice; cin >> choice;
        switch(choice){
            case 1: addStation(); break;
            case 2: viewStations(); break;
            case 3: addRoute(); break;
            case 4: viewRoutes(); break;
            case 5: addVehicle(); break;
            case 6: viewVehicles(); break;
            case 7: searchStation(); break;
            case 8: ridershipSummary(); break;
            case 9: {
                cout << "Delete: \n1.Station\n2.Route\n3.Vehicle\nChoice: "; int c; cin >> c;
                if(c==1) deleteStation();
                else if(c==2) deleteRoute();
                else if(c==3) deleteVehicle();
                else cout<<"Invalid choice\n";
            } break;
            case 10: updateStation(); break;
            case 11: return;
            default: cout << "Invalid choice\n";
        }
    }
}

int main() {
    menu();
    return 0;
}
