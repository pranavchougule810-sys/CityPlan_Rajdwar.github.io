#include <bits/stdc++.h>
using namespace std;


// ---------- Structures ----------
struct ParkingSlot {
    string slot_id;
    string area;      // Parking area/zone
    int distance;     // Distance from entrance (meters)
    bool occupied;
};

struct Vehicle {
    string vehicle_id;
    string owner_name;
    string parked_slot; // slot_id if parked, "" otherwise
};


// ---------- Data ----------
vector<ParkingSlot> slots;
vector<Vehicle> vehicles;

const string SLOT_FILE = "parking_slots.csv";
const string VEH_FILE  = "vehicles.csv";


// ---------- CSV Helpers ----------
void loadSlots() {
    slots.clear();
    ifstream in(SLOT_FILE);
    if (!in.is_open()) return;
    string line; getline(in, line); // skip header
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line); string cell;
        ParkingSlot s;
        if (!getline(ss, s.slot_id, ',')) continue;
        if (!getline(ss, s.area, ',')) continue;
        if (!getline(ss, cell, ',')) continue; s.distance = stoi(cell);
        if (!getline(ss, cell, ',')) continue; s.occupied = (cell == "1");
        slots.push_back(s);
    }
    in.close();
}

void saveSlots() {
    ofstream out(SLOT_FILE);
    out << "slot_id,area,distance,occupied\n";
    for (auto &s : slots)
        out << s.slot_id << "," << s.area << "," << s.distance << "," << (s.occupied ? 1 : 0) << "\n";
    out.close();
}

void loadVehicles() {
    vehicles.clear();
    ifstream in(VEH_FILE);
    if (!in.is_open()) return;
    string line; getline(in, line); // skip header
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line); string cell;
        Vehicle v;
        if (!getline(ss, v.vehicle_id, ',')) continue;
        if (!getline(ss, v.owner_name, ',')) continue;
        if (!getline(ss, v.parked_slot, ',')) v.parked_slot = "";
        vehicles.push_back(v);
    }
    in.close();
}

void saveVehicles() {
    ofstream out(VEH_FILE);
    out << "vehicle_id,owner_name,parked_slot\n";
    for (auto &v : vehicles)
        out << v.vehicle_id << "," << v.owner_name << "," << v.parked_slot << "\n";
    out.close();
}


// ---------- Features ----------

// Add parking slot
void addSlot() {
    ParkingSlot s;
    cout << "Slot ID: "; cin >> s.slot_id;
    cout << "Area: "; cin >> s.area;
    cout << "Distance from entrance (m): "; cin >> s.distance;
    s.occupied = false;
    slots.push_back(s);
    saveSlots();
    cout << "Parking slot added.\n";
}

// View all slots (sorted by distance)
void viewSlots() {
    if(slots.empty()){ cout << "No slots.\n"; return; }
    // Sort by distance (Heap Sort style using standard heap ops)
    vector<ParkingSlot> sorted_slots = slots;
    make_heap(sorted_slots.begin(), sorted_slots.end(),
              [](ParkingSlot &a, ParkingSlot &b){ return a.distance > b.distance; });
    sort_heap(sorted_slots.begin(), sorted_slots.end(),
              [](ParkingSlot &a, ParkingSlot &b){ return a.distance > b.distance; });

    cout << left << setw(10) << "SlotID" << setw(10) << "Area" << setw(10) << "Distance" << setw(10) << "Occupied\n";
    cout << string(40,'-') << "\n";
    for(auto &s : sorted_slots)
        cout << left << setw(10) << s.slot_id << setw(10) << s.area << setw(10) << s.distance
             << setw(10) << (s.occupied ? "Yes" : "No") << "\n";
}

// Add vehicle
void addVehicle() {
    Vehicle v;
    cout << "Vehicle ID: "; cin >> v.vehicle_id;
    cout << "Owner Name: "; cin >> v.owner_name;
    v.parked_slot = "";
    vehicles.push_back(v);
    saveVehicles();
    cout << "Vehicle added.\n";
}

// Park a vehicle
void parkVehicle() {
    string vid; cout << "Enter Vehicle ID: "; cin >> vid;
    auto it = find_if(vehicles.begin(), vehicles.end(), [&](Vehicle &v){ return v.vehicle_id == vid; });
    if(it == vehicles.end()){ cout << "Vehicle not found.\n"; return; }

    // Find nearest available slot using Heap
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq; // distance, index
    for(int i=0;i<(int)slots.size();i++){
        if(!slots[i].occupied) pq.push({slots[i].distance,i});
    }

    if(pq.empty()){ cout << "No available slots.\n"; return; }
    int idx = pq.top().second;
    slots[idx].occupied = true;
    it->parked_slot = slots[idx].slot_id;
    saveSlots(); saveVehicles();
    cout << "Vehicle parked at slot " << slots[idx].slot_id << " in area " << slots[idx].area << "\n";
}

// Remove vehicle
void removeVehicle() {
    string vid; cout << "Enter Vehicle ID: "; cin >> vid;
    auto it = find_if(vehicles.begin(), vehicles.end(), [&](Vehicle &v){ return v.vehicle_id == vid; });
    if(it == vehicles.end()){ cout << "Vehicle not found.\n"; return; }
    if(it->parked_slot.empty()){ cout << "Vehicle is not parked.\n"; return; }

    auto sit = find_if(slots.begin(), slots.end(), [&](ParkingSlot &s){ return s.slot_id == it->parked_slot; });
    if(sit != slots.end()) sit->occupied = false;
    it->parked_slot = "";
    saveSlots(); saveVehicles();
    cout << "Vehicle removed from parking.\n";
}

// View all vehicles
void viewVehicles() {
    if(vehicles.empty()){ cout << "No vehicles.\n"; return; }
    cout << left << setw(12) << "VehicleID" << setw(15) << "Owner" << setw(10) << "Slot\n";
    cout << string(37,'-') << "\n";
    for(auto &v : vehicles)
        cout << left << setw(12) << v.vehicle_id << setw(15) << v.owner_name
             << setw(10) << (v.parked_slot.empty()?"-":v.parked_slot) << "\n";
}

// ---------- Sample Data Loader (10+ slots & vehicles) ----------
void loadSampleData() {
    // 10 sample parking slots
    ofstream sOut(SLOT_FILE, ios::trunc);
    sOut << "slot_id,area,distance,occupied\n";
    sOut << "S001,A1,10,0\n";
    sOut << "S002,A1,15,1\n";
    sOut << "S003,A2,20,0\n";
    sOut << "S004,A2,25,0\n";
    sOut << "S005,B1,30,1\n";
    sOut << "S006,B1,35,0\n";
    sOut << "S007,B2,40,0\n";
    sOut << "S008,B2,45,1\n";
    sOut << "S009,C1,50,0\n";
    sOut << "S010,C1,55,0\n";
    sOut.close();

    // 10 sample vehicles (a few already parked)
    ofstream vOut(VEH_FILE, ios::trunc);
    vOut << "vehicle_id,owner_name,parked_slot\n";
    vOut << "V001,Rahul,S002\n";
    vOut << "V002,Anita,S005\n";
    vOut << "V003,Sanjay,S008\n";
    vOut << "V004,Deepa,\n";
    vOut << "V005,Imran,\n";
    vOut << "V006,Vikram,\n";
    vOut << "V007,Sneha,\n";
    vOut << "V008,Karan,\n";
    vOut << "V009,Pooja,\n";
    vOut << "V010,Manoj,\n";
    vOut.close();

    // Reload into memory
    loadSlots();
    loadVehicles();
    cout << "Sample data loaded for parking slots and vehicles.\n";
}

// ---------- Menu ----------
void menu() {
    // Load sample dataset at startup (overwrites existing CSVs)
    loadSampleData();

    while(true){
        cout << "\n===== PARKING MANAGEMENT SYSTEM =====\n";
        cout << "1.Add Parking Slot\n2.View Parking Slots\n3.Add Vehicle\n4.View Vehicles\n";
        cout << "5.Park Vehicle\n6.Remove Vehicle\n7.Exit\nChoice: ";
        int choice; cin >> choice;
        switch(choice){
            case 1: addSlot(); break;
            case 2: viewSlots(); break;
            case 3: addVehicle(); break;
            case 4: viewVehicles(); break;
            case 5: parkVehicle(); break;
            case 6: removeVehicle(); break;
            case 7: return;
            default: cout << "Invalid choice\n";
        }
    }
}

int main() {
    menu();
    return 0;
}
