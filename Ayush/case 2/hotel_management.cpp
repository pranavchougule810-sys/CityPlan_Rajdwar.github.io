// hotel_management.cpp
// Single-file Hotel Management System (CSV backed, menu-driven)
// Option B (extended features) - designed to integrate with your IT Office program.
// Compile: g++ -std=gnu++17 hotel_management.cpp -o hotel_management

#include <bits/stdc++.h>
using namespace std;

// ---------- CSV helpers (same style as IT program) ----------
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

// ---------- Time helpers ----------
string nowDate() {
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm local = *localtime(&tt);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &local);
    return string(buf);
}
string nowTimeHHMM() {
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm local = *localtime(&tt);
    char buf[10];
    strftime(buf, sizeof(buf), "%H:%M", &local);
    return string(buf);
}
int timeToMinutes(const string &hhmm) {
    int h = 0, m = 0;
    sscanf(hhmm.c_str(), "%d:%d", &h, &m);
    return h * 60 + m;
}
string minutesToHHMM(int mins) {
    mins = (mins % (24*60) + 24*60) % (24*60);
    int h = mins / 60;
    int m = mins % 60;
    char buf[6];
    sprintf(buf, "%02d:%02d", h, m);
    return string(buf);
}

// ---------- simple merge sort for CSV rows by column ----------
void mergeByCol(vector<vector<string>> &a, int l, int m, int r, int col) {
    int n1 = m - l + 1, n2 = r - m;
    vector<vector<string>> L(n1), R(n2);
    for (int i = 0; i < n1; ++i) L[i] = a[l + i];
    for (int j = 0; j < n2; ++j) R[j] = a[m + 1 + j];
    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        string lv = (col < (int)L[i].size() ? L[i][col] : "");
        string rv = (col < (int)R[j].size() ? R[j][col] : "");
        bool ln = !lv.empty() && all_of(lv.begin(), lv.end(), [](char c){ return isdigit(c) || c=='-' || c=='.'; });
        bool rn = !rv.empty() && all_of(rv.begin(), rv.end(), [](char c){ return isdigit(c) || c=='-' || c=='.'; });
        bool takeLeft;
        if (ln && rn) takeLeft = stod(lv) <= stod(rv);
        else takeLeft = lv <= rv;
        if (takeLeft) a[k++] = L[i++]; else a[k++] = R[j++];
    }
    while (i < n1) a[k++] = L[i++];
    while (j < n2) a[k++] = R[j++];
}
void mergeSortByCol(vector<vector<string>> &a, int l, int r, int col) {
    if (l >= r) return;
    int m = (l + r) / 2;
    mergeSortByCol(a, l, m, col);
    mergeSortByCol(a, m+1, r, col);
    mergeByCol(a, l, m, r, col);
}

// ---------- File names (hotel-specific, separate to avoid collisions) ----------
const string HOTEL_ROOMS_FILE = "hotel_rooms.csv";
const string HOTEL_CUSTOMERS_FILE = "hotel_customers.csv";
const string HOTEL_BOOKINGS_FILE = "hotel_bookings.csv";
const string HOTEL_RESTAURANT_FILE = "hotel_restaurant.csv";
const string HOTEL_STAFF_SCHED_FILE = "hotel_staff_sched.csv";
const string HOTEL_BILL_FILE = "hotel_bills.csv";
const string HOTEL_INV_FILE = "hotel_inventory.csv";
const string HOTEL_HOUSE_FILE = "hotel_housekeeping.csv";
const string HOTEL_FLOORS_FILE = "hotel_floors.csv";
const string HOTEL_EVENTS_FILE = "hotel_events.csv";

// ---------- Utility date-range helpers ----------
bool validDateFormat(const string &d) {
    if (d.size() != 10) return false;
    if (d[4] != '-' || d[7] != '-') return false;
    // basic numeric check
    for (size_t i = 0; i < d.size(); ++i) if (i!=4 && i!=7 && !isdigit(d[i])) return false;
    return true;
}
int dateToDays(const string &d) { // naive days count Y*360 + M*30 + D for comparison (sufficient for ordering)
    int y=0,m=0,dd=0;
    sscanf(d.c_str(), "%d-%d-%d", &y, &m, &dd);
    return y*360 + m*30 + dd;
}
bool rangesOverlap(const string &s1, const string &e1, const string &s2, const string &e2) {
    int a1 = dateToDays(s1), b1 = dateToDays(e1);
    int a2 = dateToDays(s2), b2 = dateToDays(e2);
    return max(a1,a2) <= min(b1,b2);
}

// ---------- Rooms module ----------
void hotel_addRoom() {
    string id,type,floor,rate,notes;
    cout << "Room ID: "; getline(cin, id);
    cout << "Type (Single/Double/Deluxe/Suite): "; getline(cin, type);
    cout << "Floor: "; getline(cin, floor);
    cout << "Rate per night: "; getline(cin, rate);
    cout << "Notes: "; getline(cin, notes);
    appendCSV(HOTEL_ROOMS_FILE, {id,type,floor,rate,"available",notes});
    cout << "Room added.\n";
}
void hotel_viewRooms() {
    auto rows = readCSV(HOTEL_ROOMS_FILE);
    if (rows.size() <= 1) { cout << "No rooms.\n"; return; }
    cout << "Rooms:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        auto &r = rows[i];
        cout << r[0] << " | " << (r.size()>1?r[1]:"") << " | Floor:" << (r.size()>2?r[2]:"") << " | Rate:" << (r.size()>3?r[3]:"") << " | Status:" << (r.size()>4?r[4]:"") << " | " << (r.size()>5?r[5]:"") << "\n";
    }
}
void hotel_searchRoom() {
    auto rows = readCSV(HOTEL_ROOMS_FILE);
    cout << "Enter search term (id/type/floor): ";
    string term; getline(cin, term);
    string low = term; transform(low.begin(), low.end(), low.begin(), ::tolower);
    bool found = false;
    for (size_t i = 1; i < rows.size(); ++i) {
        for (auto &cell : rows[i]) {
            string tmp = cell; transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
            if (tmp.find(low) != string::npos) {
                cout << "Found: ";
                for (auto &c : rows[i]) cout << c << " | ";
                cout << "\n";
                found = true; break;
            }
        }
    }
    if (!found) cout << "No match found.\n";
}
void hotel_sortRooms() {
    auto rows = readCSV(HOTEL_ROOMS_FILE);
    if (rows.size() <= 1) { cout << "Nothing to sort.\n"; return; }
    cout << "Sort by column: 0:id 1:type 2:floor 3:rate 4:status\nChoice: ";
    string s; getline(cin, s);
    int col = 0; try { col = stoi(s); } catch(...) { col = 0; }
    vector<vector<string>> body(rows.begin()+1, rows.end());
    mergeSortByCol(body, 0, (int)body.size()-1, col);
    vector<vector<string>> out; out.push_back(rows[0]); out.insert(out.end(), body.begin(), body.end());
    overwriteCSV(HOTEL_ROOMS_FILE, out);
    cout << "Sorted saved.\n";
}

// ---------- Customers ----------
void hotel_addCustomer() {
    string cid,name,phone,email,notes;
    cout << "Customer ID: "; getline(cin, cid);
    cout << "Name: "; getline(cin, name);
    cout << "Phone: "; getline(cin, phone);
    cout << "Email: "; getline(cin, email);
    cout << "Notes: "; getline(cin, notes);
    appendCSV(HOTEL_CUSTOMERS_FILE, {cid,name,phone,email,notes});
    cout << "Customer added.\n";
}
void hotel_viewCustomers() {
    auto rows = readCSV(HOTEL_CUSTOMERS_FILE);
    cout << "Customers:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << rows[i][0] << " | " << rows[i][1] << " | " << (rows[i].size()>2?rows[i][2]:"") << " | " << (rows[i].size()>3?rows[i][3]:"") << "\n";
    }
}
void hotel_searchCustomer() {
    auto rows = readCSV(HOTEL_CUSTOMERS_FILE);
    cout << "Enter search term (id/name/phone): ";
    string term; getline(cin, term);
    string low = term; transform(low.begin(), low.end(), low.begin(), ::tolower);
    bool found = false;
    for (size_t i = 1; i < rows.size(); ++i) {
        for (auto &cell : rows[i]) {
            string tmp = cell; transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
            if (tmp.find(low) != string::npos) {
                cout << "Found: ";
                for (auto &c : rows[i]) cout << c << " | ";
                cout << "\n";
                found = true; break;
            }
        }
    }
    if (!found) cout << "No match found.\n";
}

// ---------- Bookings ----------
bool roomAvailableForRange(const string &roomId, const string &startDate, const string &endDate) {
    auto rows = readCSV(HOTEL_BOOKINGS_FILE);
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i].size() >= 6 && rows[i][2] == roomId) {
            string s = rows[i][3], e = rows[i][4];
            if (rangesOverlap(s,e,startDate,endDate)) return false;
        }
    }
    return true;
}
void hotel_bookRoom() {
    string bid, custId, roomId, startDate, endDate, occupants, notes;
    cout << "Booking ID: "; getline(cin, bid);
    cout << "Customer ID: "; getline(cin, custId);
    cout << "Room ID: "; getline(cin, roomId);
    cout << "Start Date (YYYY-MM-DD): "; getline(cin, startDate);
    cout << "End Date (YYYY-MM-DD): "; getline(cin, endDate);
    if (!validDateFormat(startDate) || !validDateFormat(endDate) || dateToDays(endDate) < dateToDays(startDate)) {
        cout << "Invalid dates.\n"; return;
    }
    cout << "Occupants count: "; getline(cin, occupants);
    cout << "Notes: "; getline(cin, notes);
    // check room exists and status
    auto rooms = readCSV(HOTEL_ROOMS_FILE);
    bool foundRoom = false;
    for (size_t i = 1; i < rooms.size(); ++i) {
        if (rooms[i][0] == roomId) { foundRoom = true; break; }
    }
    if (!foundRoom) { cout << "Room not found.\n"; return; }
    if (!roomAvailableForRange(roomId, startDate, endDate)) { cout << "Room not available for requested dates.\n"; return; }
    appendCSV(HOTEL_BOOKINGS_FILE, {bid, custId, roomId, startDate, endDate, occupants, "booked", notes});
    cout << "Booked.\n";
}
void hotel_viewBookings() {
    auto rows = readCSV(HOTEL_BOOKINGS_FILE);
    cout << "Bookings:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << rows[i][0] << " | Cust:" << rows[i][1] << " | Room:" << rows[i][2] << " | " << rows[i][3] << " to " << rows[i][4] << " | Occ:" << (rows[i].size()>5?rows[i][5]:"") << " | Status:" << (rows[i].size()>6?rows[i][6]:"") << "\n";
    }
}
void hotel_checkIn() {
    auto rows = readCSV(HOTEL_BOOKINGS_FILE);
    cout << "Enter Booking ID to check-in: "; string bid; getline(cin, bid);
    bool found = false;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i][0] == bid) {
            found = true;
            if (rows[i].size() < 7) rows[i].resize(7);
            if (rows[i][6] == "checked-in") { cout << "Already checked-in.\n"; break; }
            rows[i][6] = "checked-in";
            // update room status to occupied for the range (for simplicity we won't change room CSV per-day)
            overwriteCSV(HOTEL_BOOKINGS_FILE, rows);
            cout << "Checked-in.\n";
            break;
        }
    }
    if (!found) cout << "Booking not found.\n";
}
void hotel_checkOut() {
    auto rows = readCSV(HOTEL_BOOKINGS_FILE);
    cout << "Enter Booking ID to check-out: "; string bid; getline(cin, bid);
    bool found = false;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i][0] == bid) {
            found = true;
            if (rows[i].size() < 7) rows[i].resize(7);
            if (rows[i][6] != "checked-in") { cout << "Not currently checked-in (status=" << rows[i][6] << ").\n"; break; }
            rows[i][6] = "completed";
            overwriteCSV(HOTEL_BOOKINGS_FILE, rows);
            cout << "Checked-out.\n";
            break;
        }
    }
    if (!found) cout << "Booking not found.\n";
}

// ---------- Billing ----------
double parseDoubleSafe(const string &s) {
    try { return stod(s); } catch(...) { return 0.0; }
}
void hotel_generateBill() {
    cout << "Enter Booking ID to generate bill: "; string bid; getline(cin, bid);
    auto bookings = readCSV(HOTEL_BOOKINGS_FILE);
    auto rooms = readCSV(HOTEL_ROOMS_FILE);
    bool found = false;
    for (size_t i = 1; i < bookings.size(); ++i) {
        if (bookings[i][0] == bid) {
            found = true;
            string roomId = bookings[i][2];
            string sdate = bookings[i][3], edate = bookings[i][4];
            int nights = max(1, dateToDays(edate) - dateToDays(sdate) + 1);
            double rate = 0.0;
            for (size_t r = 1; r < rooms.size(); ++r) if (rooms[r][0] == roomId) rate = parseDoubleSafe(rooms[r][3]);
            double roomTotal = nights * rate;
            // add restaurant charges
            double restTotal = 0.0;
            auto rest = readCSV(HOTEL_RESTAURANT_FILE);
            for (size_t j = 1; j < rest.size(); ++j) {
                if (rest[j].size() >= 4 && rest[j][1] == bid) {
                    restTotal += parseDoubleSafe(rest[j][3]);
                }
            }
            double subtotal = roomTotal + restTotal;
            double taxes = subtotal * 0.12; // 12% tax
            double service = subtotal * 0.05; // 5% service
            double total = subtotal + taxes + service;
            // save bill
            string billId = string("BIL") + bid;
            appendCSV(HOTEL_BILL_FILE, {billId,bid,to_string(nights),to_string(roomTotal),to_string(restTotal),to_string(taxes),to_string(service),to_string(total), nowDate()});
            cout << "Bill generated: " << billId << "\n";
            cout << "Nights: " << nights << " | Room:" << roomTotal << " | Food:" << restTotal << " | Taxes:" << taxes << " | Service:" << service << " | Total:" << total << "\n";
            break;
        }
    }
    if (!found) cout << "Booking not found.\n";
}
void hotel_viewBills() {
    auto rows = readCSV(HOTEL_BILL_FILE);
    cout << "Bills:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << rows[i][0] << " | Booking:" << rows[i][1] << " | Nights:" << rows[i][2] << " | Room:" << rows[i][3] << " | Food:" << rows[i][4] << " | Total:" << rows[i][7] << " | Date:" << (rows[i].size()>8?rows[i][8]:"") << "\n";
    }
}

// ---------- Restaurant Orders ----------
void hotel_addRestaurantOrder() {
    string oid, bookingId, itemList, amount, notes;
    cout << "Order ID: "; getline(cin, oid);
    cout << "Booking ID (or Customer ID): "; getline(cin, bookingId);
    cout << "Items (comma separated): "; getline(cin, itemList);
    cout << "Amount: "; getline(cin, amount);
    cout << "Notes: "; getline(cin, notes);
    appendCSV(HOTEL_RESTAURANT_FILE, {oid,bookingId,itemList,amount,nowDate(),nowTimeHHMM(),notes});
    cout << "Order recorded.\n";
}
void hotel_viewOrders() {
    auto rows = readCSV(HOTEL_RESTAURANT_FILE);
    cout << "Restaurant Orders:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << rows[i][0] << " | Booking:" << rows[i][1] << " | Items:" << rows[i][2] << " | Amount:" << rows[i][3] << " | " << rows[i][4] << " " << rows[i][5] << "\n";
    }
}

// ---------- Staff Scheduling ----------
void hotel_addStaffSchedule() {
    string sid, staffId, date, shiftStart, shiftEnd, role;
    cout << "Schedule ID: "; getline(cin, sid);
    cout << "Staff ID: "; getline(cin, staffId);
    cout << "Date (YYYY-MM-DD): "; getline(cin, date);
    cout << "Shift Start (HH:MM): "; getline(cin, shiftStart);
    cout << "Shift End (HH:MM): "; getline(cin, shiftEnd);
    cout << "Role: "; getline(cin, role);
    appendCSV(HOTEL_STAFF_SCHED_FILE, {sid,staffId,date,shiftStart,shiftEnd,role});
    cout << "Schedule added.\n";
}
bool staffConflict(const string &staffId, const string &date, const string &start, const string &end) {
    auto rows = readCSV(HOTEL_STAFF_SCHED_FILE);
    int s = timeToMinutes(start), e = timeToMinutes(end);
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i].size() >= 5 && rows[i][1] == staffId && rows[i][2] == date) {
            int ss = timeToMinutes(rows[i][3]), ee = timeToMinutes(rows[i][4]);
            if (max(ss, s) < min(ee, e)) return true;
        }
    }
    return false;
}
void hotel_viewStaffSchedule() {
    auto rows = readCSV(HOTEL_STAFF_SCHED_FILE);
    cout << "Staff Schedules:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << rows[i][0] << " | Staff:" << rows[i][1] << " | " << rows[i][2] << " | " << rows[i][3] << "-" << rows[i][4] << " | Role:" << (rows[i].size()>5?rows[i][5]:"") << "\n";
    }
}

// ---------- Inventory (food, supplies) ----------
void hotel_addInvItem() {
    string id,name,qty,unit,notes;
    cout << "Item ID: "; getline(cin, id);
    cout << "Name: "; getline(cin, name);
    cout << "Quantity: "; getline(cin, qty);
    cout << "Unit (kg/pcs/L): "; getline(cin, unit);
    cout << "Notes: "; getline(cin, notes);
    appendCSV(HOTEL_INV_FILE, {id,name,qty,unit,notes});
    cout << "Inventory item added.\n";
}
void hotel_updateInvQty() {
    auto rows = readCSV(HOTEL_INV_FILE);
    cout << "Enter Item ID to update qty: "; string id; getline(cin, id);
    cout << "Enter new quantity: "; string q; getline(cin, q);
    bool ok=false;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i][0] == id) { if (rows[i].size() < 3) rows[i].resize(3); rows[i][2] = q; ok=true; break; }
    }
    if (ok) { overwriteCSV(HOTEL_INV_FILE, rows); cout << "Updated.\n"; } else cout << "Item not found.\n";
}
void hotel_viewInventory() {
    auto rows = readCSV(HOTEL_INV_FILE);
    cout << "Inventory:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << rows[i][0] << " | " << (rows[i].size()>1?rows[i][1]:"") << " | Qty:" << (rows[i].size()>2?rows[i][2]:"") << " " << (rows[i].size()>3?rows[i][3]:"") << "\n";
    }
}

// ---------- Housekeeping ----------
void hotel_addHouseTask() {
    string hid, roomId, date, staffId, status, notes;
    cout << "Task ID: "; getline(cin, hid);
    cout << "Room ID: "; getline(cin, roomId);
    cout << "Date (YYYY-MM-DD): "; getline(cin, date);
    cout << "Assigned Staff ID: "; getline(cin, staffId);
    cout << "Notes: "; getline(cin, notes);
    status = "pending";
    appendCSV(HOTEL_HOUSE_FILE, {hid,roomId,date,staffId,status,notes});
    cout << "Housekeeping task created.\n";
}
void hotel_viewHouseTasks() {
    auto rows = readCSV(HOTEL_HOUSE_FILE);
    cout << "Housekeeping Tasks:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << rows[i][0] << " | Room:" << rows[i][1] << " | " << rows[i][2] << " | Staff:" << rows[i][3] << " | Status:" << rows[i][4] << "\n";
    }
}
void hotel_updateHouseStatus() {
    auto rows = readCSV(HOTEL_HOUSE_FILE);
    cout << "Enter Task ID to update status: "; string hid; getline(cin, hid);
    cout << "Enter status (pending/in-progress/done): "; string st; getline(cin, st);
    bool ok = false;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i][0] == hid) { if (rows[i].size() < 5) rows[i].resize(5); rows[i][4] = st; ok=true; break; }
    }
    if (ok) { overwriteCSV(HOTEL_HOUSE_FILE, rows); cout << "Updated.\n"; } else cout << "Task not found.\n";
}

// ---------- Floor & Room Allocation ----------
void hotel_addFloor() {
    string fid, number, notes;
    cout << "Floor ID: "; getline(cin, fid);
    cout << "Floor Number: "; getline(cin, number);
    cout << "Notes: "; getline(cin, notes);
    appendCSV(HOTEL_FLOORS_FILE, {fid,number,notes});
    cout << "Floor added.\n";
}
void hotel_viewFloors() {
    auto rows = readCSV(HOTEL_FLOORS_FILE);
    cout << "Floors:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << rows[i][0] << " | Number:" << rows[i][1] << " | " << (rows[i].size()>2?rows[i][2]:"") << "\n";
    }
}

// ---------- Events / Conference Booking ----------
bool eventConflictExists(const string &date, const string &st, const string &et, const string &hall) {
    auto rows = readCSV(HOTEL_EVENTS_FILE);
    int s = timeToMinutes(st), e = timeToMinutes(et);
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i].size() >= 6 && rows[i][1] == date && rows[i][4] == hall) {
            int ss = timeToMinutes(rows[i][2]), ee = timeToMinutes(rows[i][3]);
            if (max(ss, s) < min(ee, e)) return true;
        }
    }
    return false;
}
void hotel_scheduleEvent() {
    string eid,date,st,et,hall,title,organizer,notes;
    cout << "Event ID: "; getline(cin, eid);
    cout << "Date (YYYY-MM-DD): "; getline(cin, date);
    cout << "Start (HH:MM): "; getline(cin, st);
    cout << "End (HH:MM): "; getline(cin, et);
    cout << "Hall: "; getline(cin, hall);
    cout << "Title: "; getline(cin, title);
    cout << "Organizer: "; getline(cin, organizer);
    cout << "Notes: "; getline(cin, notes);
    int s = timeToMinutes(st), e = timeToMinutes(et);
    if (e <= s) { cout << "Invalid time range.\n"; return; }
    if (eventConflictExists(date, st, et, hall)) { cout << "Conflict: hall not free.\n"; return; }
    appendCSV(HOTEL_EVENTS_FILE, {eid,date,st,et,hall,title,organizer,notes});
    cout << "Event scheduled.\n";
}
void hotel_viewEvents() {
    auto rows = readCSV(HOTEL_EVENTS_FILE);
    cout << "Events:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
       string extra = (rows[i].size() > 5 ? rows[i][5] : "");
cout << rows[i][0] << " | " << rows[i][1] << " | "
     << rows[i][2] << "-" << rows[i][3]
     << " | Hall:" << rows[i][4]
     << " | " << extra << "\n";
    }
}

// ---------- Sample data loader (smaller sample to keep file size moderate) ----------
void hotel_loadSampleData() {
    cout << "This will OVERWRITE hotel CSV files with sample data.\nAre you sure? (yes/no): ";
    string ans; getline(cin, ans);
    string low = ans; transform(low.begin(), low.end(), low.begin(), ::tolower);
    if (low != "yes") { cout << "Aborted. No changes made.\n"; return; }

    // rooms
    vector<vector<string>> rooms = {
        {"room_id","type","floor","rate","status","notes"},
        {"R101","Single","1","1500","available","City view"},
        {"R102","Double","1","2000","available","Standard double"},
        {"R103","Deluxe","1","3500","available","Balcony"},
        {"R201","Single","2","1600","available","Near stairs"},
        {"R202","Double","2","2100","available","Accessible"},
        {"R203","Suite","2","6000","available","With living area"},
        {"R301","Deluxe","3","3700","available","Executive floor"},
        {"R302","Double","3","2200","available","Quiet"},
        {"R303","Suite","3","6500","available","Presidential"},
        {"R304","Single","3","1550","available","Economy"}
    };
    overwriteCSV(HOTEL_ROOMS_FILE, rooms);

    // customers
    vector<vector<string>> cust = {
        {"cust_id","name","phone","email","notes"},
        {"C001","Aman","9876543210","aman@example.com","VIP"},
        {"C002","Reema","9123456780","reema@example.com","Allergic to nuts"},
        {"C003","Jon","9988776655","jon@example.com","Prefers early breakfast"},
        {"C004","Priya","9012345678","priya@example.com","Requires baby crib"}
    };
    overwriteCSV(HOTEL_CUSTOMERS_FILE, cust);

    // bookings
    vector<vector<string>> book = {
        {"booking_id","cust_id","room_id","start_date","end_date","occupants","status","notes"},
        {"BK001","C001","R103","2025-12-20","2025-12-22","2","booked","Anniversary"},
        {"BK002","C002","R202","2025-12-25","2025-12-27","3","booked","Family"},
        {"BK003","C003","R301","2025-12-24","2025-12-25","1","booked","Business"}
    };
    overwriteCSV(HOTEL_BOOKINGS_FILE, book);

    // restaurant orders
    vector<vector<string>> rest = {
        {"order_id","booking_id","items","amount","date","time","notes"},
        {"OR001","BK001","Breakfast Platter","450","2025-12-21","08:15","Room service"},
        {"OR002","BK002","Veg Thali","250","2025-12-26","13:00","Dine-in"}
    };
    overwriteCSV(HOTEL_RESTAURANT_FILE, rest);

    // staff schedule
    vector<vector<string>> sched = {
        {"sched_id","staff_id","date","start","end","role"},
        {"S001","ST01","2025-12-20","09:00","17:00","FrontDesk"},
        {"S002","ST02","2025-12-20","07:00","15:00","Housekeeping"}
    };
    overwriteCSV(HOTEL_STAFF_SCHED_FILE, sched);

    // bills
    vector<vector<string>> bills = {
        {"bill_id","booking_id","nights","room_total","food_total","taxes","service","total","date"},
    };
    overwriteCSV(HOTEL_BILL_FILE, bills);

    // inventory
    vector<vector<string>> inv = {
        {"item_id","name","qty","unit","notes"},
        {"F001","Rice","20","kg","Basmati"},
        {"F002","Tomatoes","30","kg","Fresh"},
        {"S001","Soap","100","pcs","Guest soaps"}
    };
    overwriteCSV(HOTEL_INV_FILE, inv);

    // housekeeping
    vector<vector<string>> house = {
        {"task_id","room_id","date","staff_id","status","notes"},
    };
    overwriteCSV(HOTEL_HOUSE_FILE, house);

    // floors
    vector<vector<string>> floors = {
        {"floor_id","number","notes"},
        {"F1","1","Standard"},
        {"F2","2","Executive"},
        {"F3","3","Presidential"}
    };
    overwriteCSV(HOTEL_FLOORS_FILE, floors);

    // events
    vector<vector<string>> events = {
        {"event_id","date","start","end","hall","title","organizer","notes"},
        {"EV001","2025-12-31","19:00","23:00","GrandHall","New Year Gala","HotelMgmt","Ticketed"}
    };
    overwriteCSV(HOTEL_EVENTS_FILE, events);

    cout << "Hotel sample data loaded successfully!\n";
}

// ---------- Menus ----------
void hotel_roomMenu() {
    while (true) {
        cout << "\nRoom Menu\n1.Add Room\n2.View Rooms\n3.Search Room\n4.Sort Rooms\n5.Back\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") hotel_addRoom();
        else if (c == "2") hotel_viewRooms();
        else if (c == "3") hotel_searchRoom();
        else if (c == "4") hotel_sortRooms();
        else return;
    }
}
void hotel_customerMenu() {
    while (true) {
        cout << "\nCustomer Menu\n1.Add Customer\n2.View Customers\n3.Search Customer\n4.Back\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") hotel_addCustomer();
        else if (c == "2") hotel_viewCustomers();
        else if (c == "3") hotel_searchCustomer();
        else return;
    }
}
void hotel_bookingMenu() {
    while (true) {
        cout << "\nBooking Menu\n1.Book Room\n2.View Bookings\n3.Check-in\n4.Check-out\n5.Generate Bill\n6.View Bills\n7.Back\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") hotel_bookRoom();
        else if (c == "2") hotel_viewBookings();
        else if (c == "3") hotel_checkIn();
        else if (c == "4") hotel_checkOut();
        else if (c == "5") hotel_generateBill();
        else if (c == "6") hotel_viewBills();
        else return;
    }
}
void hotel_restMenu() {
    while (true) {
        cout << "\nRestaurant Menu\n1.Add Order\n2.View Orders\n3.Back\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") hotel_addRestaurantOrder();
        else if (c == "2") hotel_viewOrders();
        else return;
    }
}
void hotel_staffMenu() {
    while (true) {
        cout << "\nStaff Scheduling Menu\n1.Add Schedule\n2.View Schedules\n3.Back\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") hotel_addStaffSchedule();
        else if (c == "2") hotel_viewStaffSchedule();
        else return;
    }
}
void hotel_inventoryMenu() {
    while (true) {
        cout << "\nInventory Menu\n1.Add Item\n2.Update Qty\n3.View Inventory\n4.Back\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") hotel_addInvItem();
        else if (c == "2") hotel_updateInvQty();
        else if (c == "3") hotel_viewInventory();
        else return;
    }
}
void hotel_houseMenu() {
    while (true) {
        cout << "\nHousekeeping Menu\n1.Add Task\n2.View Tasks\n3.Update Status\n4.Back\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") hotel_addHouseTask();
        else if (c == "2") hotel_viewHouseTasks();
        else if (c == "3") hotel_updateHouseStatus();
        else return;
    }
}
void hotel_floorMenu() {
    while (true) {
        cout << "\nFloor Menu\n1.Add Floor\n2.View Floors\n3.Back\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") hotel_addFloor();
        else if (c == "2") hotel_viewFloors();
        else return;
    }
}
void hotel_eventMenu() {
    while (true) {
        cout << "\nEvent Menu\n1.Schedule Event\n2.View Events\n3.Back\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") hotel_scheduleEvent();
        else if (c == "2") hotel_viewEvents();
        else return;
    }
}

// ---------- main ----------
int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    while (true) {
        cout << "\n=== HOTEL MANAGEMENT ===\n";
        cout << "1.Rooms\n";
        cout << "2.Customers\n";
        cout << "3.Bookings (room)\n";
        cout << "4.Restaurant Orders\n";
        cout << "5.Staff Scheduling\n";
        cout << "6.Payment & Billing\n";
        cout << "7.Inventory (food/supplies)\n";
        cout << "8.Housekeeping\n";
        cout << "9.Floor & Room Allocation\n";
        cout << "10.Event/Conference Booking\n";
        cout << "11.Load Sample Data\n";
        cout << "12.Exit\n";
        cout << "Choice: ";
        cout.flush();

        string ch; getline(cin, ch);
        if (ch == "1") hotel_roomMenu();
        else if (ch == "2") hotel_customerMenu();
        else if (ch == "3") hotel_bookingMenu();
        else if (ch == "4") hotel_restMenu();
        else if (ch == "5") hotel_staffMenu();
        else if (ch == "6") { // billing quick menu
            while (true) {
                cout << "\nBilling Menu\n1.Generate Bill (from booking)\n2.View Bills\n3.Back\nChoice: ";
                string c; getline(cin, c);
                if (c == "1") hotel_generateBill();
                else if (c == "2") hotel_viewBills();
                else break;
            }
        }
        else if (ch == "7") hotel_inventoryMenu();
        else if (ch == "8") hotel_houseMenu();
        else if (ch == "9") hotel_floorMenu();
        else if (ch == "10") hotel_eventMenu();
        else if (ch == "11") hotel_loadSampleData();
        else if (ch == "12") { cout << "Exiting...\n"; break; }
        else cout << "Invalid choice.\n";
    }
    return 0;
}
