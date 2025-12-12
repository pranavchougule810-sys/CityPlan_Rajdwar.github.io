
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <limits>
#include <functional>
using namespace std;

#define HOTEL_MAX_NODES 1200
#define HOTEL_MAX_ROOMS 1000
#define HOTEL_MAX_GUESTS 2000
#define HOTEL_MAX_MENU 1000
#define HOTEL_MAX_STAFF 500
#define HOTEL_MAX_FLOORS 200
#define HOTEL_MAX_REQUESTS 2000

// Node Types
enum hotelNodeType { HOTEL_ROOM, HOTEL_FLOOR, HOTEL_OTHER };

// --------- Structures ----------
struct HotelNode {
    int id;
    string name;
    hotelNodeType type;
};
HotelNode hotelNodes[HOTEL_MAX_NODES];
int hotelNodeCount = 0;

// Room BST
struct Room {
    int roomNo;
    string type;
    string floorName;
    int price;
    int capacity;
    int nodeIndex;
    Room *left, *right;
};
Room *hotelRoomRoot = nullptr;
int hotelRoomCount = 0;

// Guest BST
struct Guest {
    int id;
    string name;
    string phone;
    int roomNo;
    int nights;
    double balance;
    Guest *left, *right;
};
Guest *hotelGuestRoot = nullptr;
int hotelGuestCount = 0;
int hotelNextGuestId = 1;

// Menu BST
struct MenuItem {
    string name;
    double price;
    MenuItem *left, *right;
};
MenuItem *hotelMenuRoot = nullptr;
int hotelMenuCount = 0;

// Staff array
struct Staff {
    int id;
    string name;
    string role;
    int salary;
};
Staff hotelStaff[HOTEL_MAX_STAFF];
int hotelStaffCount = 0;

// Floors
string hotelFloors[HOTEL_MAX_FLOORS];
int hotelFloorsCount = 0;

// Service Requests
enum RequestType { REQ_EXTRA_BED, REQ_SHEETS, REQ_ORDER, REQ_HELP, REQ_EMERGENCY };
struct ServiceRequest {
    int id;
    int guestId;
    int roomNo;
    RequestType type;
    string details;
    bool completed;
};
ServiceRequest serviceQueue[HOTEL_MAX_REQUESTS];
int serviceFront = 0, serviceRear = -1, serviceCount = 0;
int hotelNextRequestId = 1;

// ------------- Utilities --------------
int hotel_addNode(const string &name, hotelNodeType t) {
    for (int i = 0; i < hotelNodeCount; i++)
        if (hotelNodes[i].name == name)
            return i;
    hotelNodes[hotelNodeCount] = { hotelNodeCount, name, t };
    return hotelNodeCount++;
}

void hotel_safeIgnore() { cin.ignore(numeric_limits<streamsize>::max(), '\n'); }

// ROOM BST FUNCTIONS
Room* hotel_createRoomNode(int rno, const string &rtype, const string &floor, int price, int cap) {
    Room* r = new Room();
    r->roomNo = rno;
    r->type = rtype;
    r->floorName = floor;
    r->price = price;
    r->capacity = cap;

    string nodeName = "Room" + to_string(rno);
    r->nodeIndex = hotel_addNode(nodeName, HOTEL_ROOM);

    r->left = r->right = nullptr;
    hotelRoomCount++;
    return r;
}

Room* hotel_insertRoom(Room* root, int rno, const string &rtype, const string &floor, int price, int cap) {
    if (!root) return hotel_createRoomNode(rno, rtype, floor, price, cap);
    if (rno < root->roomNo) root->left = hotel_insertRoom(root->left, rno, rtype, floor, price, cap);
    else if (rno > root->roomNo) root->right = hotel_insertRoom(root->right, rno, rtype, floor, price, cap);
    return root;
}

Room* hotel_findRoomByNo(Room* root, int rno) {
    if (!root) return nullptr;
    if (rno == root->roomNo) return root;
    if (rno < root->roomNo) return hotel_findRoomByNo(root->left, rno);
    return hotel_findRoomByNo(root->right, rno);
}

void hotel_inorderRooms(Room* root) {
    if (!root) return;
    hotel_inorderRooms(root->left);
    cout << "RoomNo: " << root->roomNo << " | Type: " << root->type
         << " | Floor: " << root->floorName << " | Price: " << root->price
         << " | Cap: " << root->capacity << "\n";
    hotel_inorderRooms(root->right);
}

// GUEST BST FUNCTIONS
Guest* hotel_createGuest(const string &name, const string &phone, int roomNo, int nights, double balance) {
    Guest* g = new Guest();
    g->id = hotelNextGuestId++;
    g->name = name;
    g->phone = phone;
    g->roomNo = roomNo;
    g->nights = nights;
    g->balance = balance;
    g->left = g->right = nullptr;
    hotelGuestCount++;
    return g;
}

Guest* hotel_insertGuest(Guest* root, const string &name, const string &phone, int roomNo, int nights, double balance) {
    if (!root) return hotel_createGuest(name, phone, roomNo, nights, balance);
    if (name < root->name) root->left = hotel_insertGuest(root->left, name, phone, roomNo, nights, balance);
    else if (name > root->name) root->right = hotel_insertGuest(root->right, name, phone, roomNo, nights, balance);
    return root;
}

Guest* hotel_findGuestByName(Guest* root, const string &name) {
    if (!root) return nullptr;
    if (name == root->name) return root;
    if (name < root->name) return hotel_findGuestByName(root->left, name);
    return hotel_findGuestByName(root->right, name);
}

Guest* hotel_removeGuestByName(Guest* root, const string &name) {
    if (!root) return nullptr;
    if (name < root->name) root->left = hotel_removeGuestByName(root->left, name);
    else if (name > root->name) root->right = hotel_removeGuestByName(root->right, name);
    else {
        if (!root->left) { Guest* t = root->right; delete root; hotelGuestCount--; return t; }
        else if (!root->right) { Guest* t = root->left; delete root; hotelGuestCount--; return t; }
        else {
            Guest* succ = root->right;
            while (succ->left) succ = succ->left;
            *root = *succ;
            root->right = hotel_removeGuestByName(root->right, succ->name);
        }
    }
    return root;
}

void hotel_inorderGuests(Guest* root) {
    if (!root) return;
    hotel_inorderGuests(root->left);
    cout << "GuestID: " << root->id << " | " << root->name << " | Phone: " << root->phone
         << " | Room: " << root->roomNo << " | Nights: " << root->nights
         << " | Balance: " << root->balance << "\n";
    hotel_inorderGuests(root->right);
}

// MENU BST
MenuItem* hotel_createMenuItem(const string &name, double price) {
    MenuItem* m = new MenuItem();
    m->name = name;
    m->price = price;
    m->left = m->right = nullptr;
    hotelMenuCount++;
    return m;
}

MenuItem* hotel_insertMenuItem(MenuItem* root, const string &name, double price) {
    if (!root) return hotel_createMenuItem(name, price);
    if (name < root->name) root->left = hotel_insertMenuItem(root->left, name, price);
    else if (name > root->name) root->right = hotel_insertMenuItem(root->right, name, price);
    return root;
}

MenuItem* hotel_findMenuItem(MenuItem* root, const string &name) {
    if (!root) return nullptr;
    if (name == root->name) return root;
    if (name < root->name) return hotel_findMenuItem(root->left, name);
    return hotel_findMenuItem(root->right, name);
}

void hotel_inorderMenu(MenuItem* root) {
    if (!root) return;
    hotel_inorderMenu(root->left);
    cout << "  " << root->name << " - Rs " << root->price << "\n";
    hotel_inorderMenu(root->right);
}

// SERVICE QUEUE
void hotel_enqueueService(const ServiceRequest &rq) {
    if (serviceCount >= HOTEL_MAX_REQUESTS) { cout << "Queue Full!\n"; return; }
    serviceRear = (serviceRear + 1) % HOTEL_MAX_REQUESTS;
    serviceQueue[serviceRear] = rq;
    serviceCount++;
}

ServiceRequest hotel_dequeueService() {
    ServiceRequest empty; empty.id = -1;
    if (serviceCount == 0) return empty;
    ServiceRequest r = serviceQueue[serviceFront];
    serviceFront = (serviceFront + 1) % HOTEL_MAX_REQUESTS;
    serviceCount--;
    return r;
}

void hotel_showServiceQueue() {
    if (serviceCount == 0) { cout << "No requests.\n"; return; }
    cout << "--- Pending Requests ---\n";
    int idx = serviceFront;
    for (int i = 0; i < serviceCount; i++) {
        auto &r = serviceQueue[idx];
        cout << "ID:" << r.id << " Guest:" << r.guestId << " Room:" << r.roomNo
             << " Details:" << r.details << "\n";
        idx = (idx + 1) % HOTEL_MAX_REQUESTS;
    }
}

// CSV HELPER
int hotel_toInt(const string &s) { try { return stoi(s); } catch (...) { return 0; } }

int hotel_splitCSV(const string &line, string out[], int maxCols) {
    int c = 0; string cur = "";
    for (char ch : line) {
        if (ch == ',' && c < maxCols - 1) {
            out[c++] = cur; cur = "";
        } else cur.push_back(ch);
    }
    out[c++] = cur;
    return c;
}

// CSV LOADERS
void hotel_loadFloorsCSV(const string &fn) {
    ifstream in(fn);
    if (!in.is_open()) { cout << "Missing " << fn << "\n"; return; }

    string line;
    getline(in, line);
    int loaded = 0;

    while (getline(in, line)) {
        if (line.empty()) continue;
        hotelFloors[hotelFloorsCount++] = line;
        hotel_addNode(line, HOTEL_FLOOR);
        loaded++;
    }
    cout << "Loaded " << loaded << " floors.\n";
}

void hotel_loadRoomsCSV(const string &fn) {
    ifstream in(fn);
    if (!in.is_open()) { cout << "Missing " << fn << "\n"; return; }

    string line;
    getline(in, line);
    int loaded = 0;

    while (getline(in, line)) {
        string c[6];
        int n = hotel_splitCSV(line, c, 6);
        if (n < 4) continue;

        int rno = hotel_toInt(c[0]);
        string type = c[1];
        string floor = c[2];
        int price = hotel_toInt(c[3]);
        int cap = (n > 4 ? hotel_toInt(c[4]) : 1);

        hotel_addNode(floor, HOTEL_FLOOR);
        hotelRoomRoot = hotel_insertRoom(hotelRoomRoot, rno, type, floor, price, cap);
        loaded++;
    }
    cout << "Loaded " << loaded << " rooms.\n";
}

void hotel_loadMenuCSV(const string &fn) {
    ifstream in(fn);
    if (!in.is_open()) { cout << "Missing " << fn << "\n"; return; }

    string line;
    getline(in, line);
    int loaded = 0;

    while (getline(in, line)) {
        string c[3];
        int n = hotel_splitCSV(line, c, 3);
        if (n < 2) continue;

        hotelMenuRoot = hotel_insertMenuItem(hotelMenuRoot, c[0], atof(c[1].c_str()));
        loaded++;
    }
    cout << "Loaded " << loaded << " menu items.\n";
}

void hotel_loadStaffCSV(const string &fn) {
    ifstream in(fn);
    if (!in.is_open()) { cout << "Missing " << fn << "\n"; return; }

    string line;
    getline(in, line);
    int loaded = 0;

    while (getline(in, line)) {
        string c[4];
        int n = hotel_splitCSV(line, c, 4);
        if (n < 3) continue;

        hotelStaff[hotelStaffCount] = {
            hotelStaffCount + 1, c[0], c[1], hotel_toInt(c[2])
        };
        hotelStaffCount++;
        loaded++;
    }
    cout << "Loaded " << loaded << " staff.\n";
}

void hotel_loadGuestsCSV(const string &fn) {
    ifstream in(fn);
    if (!in.is_open()) { cout << "Missing " << fn << "\n"; return; }

    string line;
    getline(in, line);
    int loaded = 0;

    while (getline(in, line)) {
        string c[6];
        int n = hotel_splitCSV(line, c, 6);
        if (n < 4) continue;

        string name = c[0];
        string phone = c[1];
        int roomNo = hotel_toInt(c[2]);
        int nights = hotel_toInt(c[3]);
        double bal = (n > 4 ? atof(c[4].c_str()) : 0);

        hotelGuestRoot = hotel_insertGuest(hotelGuestRoot, name, phone, roomNo, nights, bal);
        loaded++;
    }
    cout << "Loaded " << loaded << " guests.\n";
}

// ⭐ NEW FUNCTION — LOAD EVERYTHING AT ONCE
void hotel_loadAllCSVs() {
    cout << "\n--- Loading All Hotel Data ---\n";
    hotel_loadFloorsCSV("floors.csv");
    hotel_loadRoomsCSV("rooms.csv");
    hotel_loadMenuCSV("menu.csv");
    hotel_loadStaffCSV("hotel_staff.csv");
    hotel_loadGuestsCSV("hotel_guests.csv");
    cout << "--------------------------------\n";
    cout << "All hotel data loaded successfully!\n";
}

// ---------- INTERACTIVE SECTION ----------
void hotel_showRooms() { hotel_inorderRooms(hotelRoomRoot); }
void hotel_showGuests() { hotel_inorderGuests(hotelGuestRoot); }
void hotel_showMenuItems() { hotel_inorderMenu(hotelMenuRoot); }

void hotel_showNodes() {
    cout << "\n--- HOTEL NODES (" << hotelNodeCount << ") ---\n";
    for (int i = 0; i < hotelNodeCount; i++)
        cout << i << ": " << hotelNodes[i].name << " | Type=" << hotelNodes[i].type << "\n";
}

// Staff
void hotel_addStaffInteractive() {
    hotel_safeIgnore();
    cout << "Staff name: "; string nm; getline(cin, nm);
    cout << "Role: "; string role; getline(cin, role);
    cout << "Salary: "; int sal; cin >> sal;
    hotel_safeIgnore();

    hotelStaff[hotelStaffCount] = { hotelStaffCount + 1, nm, role, sal };
    hotelStaffCount++;
    cout << "Staff added.\n";
}

// Add Floor
void hotel_addFloorInteractive() {
    hotel_safeIgnore();
    cout << "Enter floor name: "; string f; getline(cin, f);
    hotelFloors[hotelFloorsCount++] = f;
    hotel_addNode(f, HOTEL_FLOOR);
    cout << "Floor added.\n";
}

// Add Room
void hotel_addRoomInteractive() {
    cout << "Room number: "; int rno; cin >> rno;
    hotel_safeIgnore();
    cout << "Room type: "; string t; getline(cin, t);
    cout << "Floor name: "; string f; getline(cin, f);
    cout << "Price: "; int price; cin >> price;
    cout << "Capacity: "; int cap; cin >> cap;

    hotel_addNode(f, HOTEL_FLOOR);
    hotelRoomRoot = hotel_insertRoom(hotelRoomRoot, rno, t, f, price, cap);
    cout << "Room added.\n";
}

// Check-in
void hotel_checkInInteractive() {
    hotel_safeIgnore();
    cout << "Guest name: "; string name; getline(cin, name);
    cout << "Phone: "; string phone; getline(cin, phone);
    cout << "Room no: "; int rno; cin >> rno;
    cout << "Nights: "; int nights; cin >> nights;

    Room* rr = hotel_findRoomByNo(hotelRoomRoot, rno);
    if (!rr) { cout << "Room not found.\n"; return; }

    double bal = rr->price * nights;
    hotelGuestRoot = hotel_insertGuest(hotelGuestRoot, name, phone, rno, nights, bal);

    cout << "Checked in " << name << " | Bill: Rs " << bal << "\n";
}

// Check-out
void hotel_checkOutInteractive() {
    hotel_safeIgnore();
    cout << "Guest name: "; string name; getline(cin, name);
    Guest* g = hotel_findGuestByName(hotelGuestRoot, name);
    if (!g) { cout << "Not found.\n"; return; }

    cout << "Final bill: Rs " << g->balance << "\n";
    hotelGuestRoot = hotel_removeGuestByName(hotelGuestRoot, name);
    cout << "Checked out.\n";
}

// Add Menu Item
void hotel_addMenuItemInteractive() {
    hotel_safeIgnore();
    cout << "Menu item name: "; string name; getline(cin, name);
    cout << "Price: "; double p; cin >> p;

    hotelMenuRoot = hotel_insertMenuItem(hotelMenuRoot, name, p);
    cout << "Menu item added.\n";
}

// In-room dining
void hotel_inRoomDiningInteractive() {
    hotel_safeIgnore();
    cout << "Guest name: "; string name; getline(cin, name);

    Guest* g = hotel_findGuestByName(hotelGuestRoot, name);
    if (!g) { cout << "Guest not found.\n"; return; }

    hotel_showMenuItems();

    cout << "Item (DONE to stop): ";
    string item;
    while (true) {
        getline(cin, item);
        if (item == "DONE") break;

        MenuItem* mi = hotel_findMenuItem(hotelMenuRoot, item);
        if (!mi) { cout << "Not found.\n"; continue; }

        g->balance += mi->price;
        cout << "Added. New balance: Rs " << g->balance << "\n";
    }
}

// Service Request
void hotel_requestServiceInteractive() {
    hotel_safeIgnore();
    cout << "Guest name: "; string name; getline(cin, name);

    Guest* g = hotel_findGuestByName(hotelGuestRoot, name);
    if (!g) { cout << "Not found.\n"; return; }

    cout << "Request type:\n1-Bed\n2-Sheets\n3-Order\n4-Help\n5-Emergency\n";
    int c; cin >> c; hotel_safeIgnore();

    ServiceRequest rq;
    rq.id = hotelNextRequestId++;
    rq.guestId = g->id;
    rq.roomNo = g->roomNo;

    if (c == 1) rq.type = REQ_EXTRA_BED, rq.details = "Extra bed";
    else if (c == 2) rq.type = REQ_SHEETS, rq.details = "Sheets";
    else if (c == 3) rq.type = REQ_ORDER, rq.details = "Food order";
    else if (c == 4) rq.type = REQ_HELP, rq.details = "Help needed";
    else rq.type = REQ_EMERGENCY, rq.details = "Emergency";

    hotel_enqueueService(rq);
    cout << "Request queued.\n";
}

// Process Request
void hotel_processNextServiceInteractive() {
    auto rq = hotel_dequeueService();
    if (rq.id == -1) { cout << "No requests.\n"; return; }

    cout << "Processing request " << rq.id << ": " << rq.details << "\nDone.\n";
}

// Guest Bill Viewer
void hotel_viewGuestBillInteractive() {
    hotel_safeIgnore();
    cout << "Guest name: "; string name; getline(cin, name);
    Guest* g = hotel_findGuestByName(hotelGuestRoot, name);
    if (!g) { cout << "Not found.\n"; return; }

    cout << "Balance: Rs " << g->balance << "\n";
}

// Sorting Guests
void hotel_sortGuestsByBalanceInteractive() {
    if (hotelGuestCount == 0) { cout << "No guests.\n"; return; }

    Guest* arr[HOTEL_MAX_GUESTS];
    int idx = 0;

    function<void(Guest*)> collect = [&](Guest* r){ if (!r) return; collect(r->left); arr[idx++] = r; collect(r->right); };
    collect(hotelGuestRoot);

    function<void(int,int)> qs = [&](int L, int R){
        if (L >= R) return;
        double pivot = arr[L]->balance;
        int i = L, j = R + 1;
        while (true) {
            while (++i <= R && arr[i]->balance > pivot);
            while (--j >= L && arr[j]->balance < pivot);
            if (i >= j) break;
            swap(arr[i], arr[j]);
        }
        swap(arr[L], arr[j]);
        qs(L, j - 1);
        qs(j + 1, R);
    };

    qs(0, idx - 1);

    cout << "Guests sorted by balance:\n";
    for (int i = 0; i < idx; i++)
        cout << arr[i]->name << " | Rs " << arr[i]->balance << "\n";
}

// ===============================
// HOTEL MENU
// ===============================
void hotel_showMenu() {
    cout << "\n==============================\n";
    cout << "      HOTEL MANAGEMENT\n";
    cout << "==============================\n";
    cout << " 1. Add Floor\n";
    cout << " 2. Add Room\n";
    cout << " 3. Show Hotel Nodes\n";
    cout << " 4. Show Rooms\n";
    cout << " 5. Show Guests\n";
    cout << " 6. Check-in Guest\n";
    cout << " 7. Check-out Guest\n";
    cout << " 8. Add Menu Item\n";
    cout << " 9. Show Menu\n";
    cout << "10. In-room Dining\n";
    cout << "11. Request Service\n";
    cout << "12. Process Next Service\n";
    cout << "13. Show Service Queue\n";
    cout << "14. View Guest Bill\n";
    cout << "15. Sort Guests by Balance\n";
    cout << "16. Load Floors CSV\n";
    cout << "17. Load Rooms CSV\n";
    cout << "18. Load Menu CSV\n";
    cout << "19. Load Staff CSV\n";
    cout << "20. Load Guests CSV\n";
    cout << "21. LOAD ALL CSV FILES\n";
    cout << " 0. Return to Main Menu\n";
    cout << "==============================\n";
    cout << "Enter choice: ";
}

// MAIN HOTEL CONTROLLER
void hotelSystem() {
    int choice;
    while (true) {
        hotel_showMenu();
        cin >> choice;

        switch (choice) {
            case 1: hotel_addFloorInteractive(); break;
            case 2: hotel_addRoomInteractive(); break;
            case 3: hotel_showNodes(); break;
            case 4: hotel_showRooms(); break;
            case 5: hotel_showGuests(); break;
            case 6: hotel_checkInInteractive(); break;
            case 7: hotel_checkOutInteractive(); break;
            case 8: hotel_addMenuItemInteractive(); break;
            case 9: hotel_showMenuItems(); break;
            case 10: hotel_inRoomDiningInteractive(); break;
            case 11: hotel_requestServiceInteractive(); break;
            case 12: hotel_processNextServiceInteractive(); break;
            case 13: hotel_showServiceQueue(); break;
            case 14: hotel_viewGuestBillInteractive(); break;
            case 15: hotel_sortGuestsByBalanceInteractive(); break;
            case 16: hotel_loadFloorsCSV("floors.csv"); break;
            case 17: hotel_loadRoomsCSV("rooms.csv"); break;
            case 18: hotel_loadMenuCSV("menu.csv"); break;
            case 19: hotel_loadStaffCSV("hotel_staff.csv"); break;
            case 20: hotel_loadGuestsCSV("hotel_guests.csv"); break;
            case 21: hotel_loadAllCSVs(); break;  // ⭐ NEW
            case 0: return;
            default: cout << "Invalid choice.\n";
        }
    }
}
int main()
{
    hotelSystem();
}

