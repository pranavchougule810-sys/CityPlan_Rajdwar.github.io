// community_module_final.cpp
// Full Community Module - No graph, No SaveCSV (Version A final)
// Paste-ready. Uses fixed CSV names like communityMembers.csv
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
using namespace std;

/* ---------------------- CONFIG / LIMITS ---------------------- */
#define COMMUNITY_MAX_MEMBERS      2000
#define COMMUNITY_MAX_STAFF        500
#define COMMUNITY_MAX_FACILITIES   300
#define COMMUNITY_MAX_EQUIPMENT    1000
#define COMMUNITY_MAX_EVENTS       2000
#define COMMUNITY_MAX_BOOKINGS     5000
#define COMMUNITY_MAX_TRANSACTIONS 20000

/* ---------------------- STATIC HELPERS (internal linkage) ---------------------- */
static int toInt_static(const string &s) {
    try { return stoi(s); } catch(...) { return 0; }
}
static double toDouble_static(const string &s) {
    try { return stod(s); } catch(...) { return 0.0; }
}

static int splitCSV_static(const string &line, string out[], int maxCols) {
    int col = 0;
    string cur = "";
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') { inQuotes = !inQuotes; continue; }
        if (c == ',' && !inQuotes && col < maxCols - 1) {
            out[col++] = cur;
            cur.clear();
        } else cur.push_back(c);
    }
    out[col++] = cur;
    return col;
}

/* Provide aliases inside this translation unit */
#define toInt toInt_static
#define toDouble toDouble_static
#define splitCSV splitCSV_static

/* ---------------------- STRUCTS ---------------------- */

struct communityMember {
    int member_id;
    string name;
    int age;
    string phone;
    string email;
    string membership_type;
    string join_date;
    string address;
    int active;
};

struct communityStaff {
    int staff_id;
    string name;
    string role;
    string phone;
    string email;
    double salary_per_month;
    string join_date;
    int is_active;
};

struct communityFacility {
    int facility_id;
    string name;
    string type;
    int capacity;
    double price_per_hour;
    string location;
    string available_from;
    string available_to;
    int active;
};

struct communityEquipment {
    int equipment_id;
    string name;
    int quantity_total;
    int quantity_available;
    string condition_desc;
    string last_maintenance_date;
};

struct communityEvent {
    int event_id;
    string title;
    int organizer_member_id;
    int facility_id;
    string date;
    string start_time;
    string end_time;
    int expected_attendance;
    double revenue_expected;
    string status;
};

struct communityBooking {
    int booking_id;
    int event_id;
    int member_id;
    int facility_id;
    string date;
    string start_time;
    string end_time;
    double total_amount;
    string payment_status;
};

struct communityRevenue {
    int revenue_id;
    string source_type;
    int source_id;
    string date;
    double amount;
    string description;
};

struct communityExpense {
    int expense_id;
    int related_event_id;
    string date;
    double amount;
    string vendor;
    string description;
    string expense_type;
};

/* ---------------------- GLOBAL ARRAYS & COUNTERS ---------------------- */

static communityMember communityMembers[COMMUNITY_MAX_MEMBERS];
static int communityMemberCount = 0;

static communityStaff communityStaffs[COMMUNITY_MAX_STAFF];
static int communityStaffCount = 0;

static communityFacility communityFacilities[COMMUNITY_MAX_FACILITIES];
static int communityFacilityCount = 0;

static communityEquipment communityEquipments[COMMUNITY_MAX_EQUIPMENT];
static int communityEquipmentCount = 0;

static communityEvent communityEvents[COMMUNITY_MAX_EVENTS];
static int communityEventCount = 0;

static communityBooking communityBookings[COMMUNITY_MAX_BOOKINGS];
static int communityBookingCount = 0;

static communityRevenue communityRevenues[COMMUNITY_MAX_TRANSACTIONS];
static int communityRevenueCount = 0;

static communityExpense communityExpenses[COMMUNITY_MAX_TRANSACTIONS];
static int communityExpenseCount = 0;

/* ---------------------- SMALL HELPERS ---------------------- */

static string dtKey(const string &date, const string &time) {
    return date + " " + time;
}

/* overlap check */
static bool timeOverlap(const string &date1, const string &s1, const string &e1,
                        const string &date2, const string &s2, const string &e2) {
    if (date1 != date2) return false;
    string a = dtKey(date1, s1);
    string b = dtKey(date1, e1);
    string c = dtKey(date2, s2);
    string d = dtKey(date2, e2);
    return (a < d && c < b);
}

/* booking overlap search */
static bool communityCheckBookingOverlap(int facility_id, const string &date, const string &start_time, const string &end_time) {
    for (int i=0;i<communityBookingCount;i++){
        communityBooking &b = communityBookings[i];
        if (b.facility_id == facility_id && b.date == date) {
            if (timeOverlap(date, start_time, end_time, b.date, b.start_time, b.end_time)) return true;
        }
    }
    return false;
}

/* ---------------------- CSV LOADERS (NO SAVE FUNCTIONS) ---------------------- */

/* Members */
void communityLoadMembersCSV(const string &fn) {
    ifstream in(fn);
    if (!in.is_open()) { cout<<"Cannot open "<<fn<<"\n"; return; }
    string line; getline(in,line); int loaded=0;
    while (getline(in,line)) {
        if (line.size()<1) continue;
        string cols[9]; int n = splitCSV(line, cols, 9);
        if (n < 2) continue;
        if (communityMemberCount >= COMMUNITY_MAX_MEMBERS) { cout<<"Reached members capacity\n"; break; }
        communityMember &m = communityMembers[communityMemberCount];
        m.member_id = toInt(cols[0]);
        m.name = cols[1];
        m.age = (n>2)? toInt(cols[2]) : 0;
        m.phone = (n>3)? cols[3] : "";
        m.email = (n>4)? cols[4] : "";
        m.membership_type = (n>5)? cols[5] : "";
        m.join_date = (n>6)? cols[6] : "";
        m.address = (n>7)? cols[7] : "";
        m.active = (n>8)? toInt(cols[8]) : 1;
        if (m.member_id==0 || m.name.empty()) continue;
        communityMemberCount++; loaded++;
    }
    cout<<"Loaded "<<loaded<<" members from "<<fn<<"\n";
    in.close();
}

/* Staff */
void communityLoadStaffCSV(const string &fn) {
    ifstream in(fn);
    if (!in.is_open()) { cout<<"Cannot open "<<fn<<"\n"; return; }
    string line; getline(in,line); int loaded=0;
    while (getline(in,line)) {
        if (line.size()<1) continue;
        string cols[8]; int n = splitCSV(line, cols, 8);
        if (n < 2) continue;
        if (communityStaffCount >= COMMUNITY_MAX_STAFF) { cout<<"Reached staff capacity\n"; break; }
        communityStaff &s = communityStaffs[communityStaffCount];
        s.staff_id = toInt(cols[0]);
        s.name = cols[1];
        s.role = (n>2)? cols[2] : "";
        s.phone = (n>3)? cols[3] : "";
        s.email = (n>4)? cols[4] : "";
        s.salary_per_month = (n>5)? toDouble(cols[5]) : 0.0;
        s.join_date = (n>6)? cols[6] : "";
        s.is_active = (n>7)? toInt(cols[7]) : 1;
        if (s.staff_id==0 || s.name.empty()) continue;
        communityStaffCount++; loaded++;
    }
    cout<<"Loaded "<<loaded<<" staff from "<<fn<<"\n";
    in.close();
}

/* Facilities */
void communityLoadFacilitiesCSV(const string &fn) {
    ifstream in(fn);
    if (!in.is_open()) { cout<<"Cannot open "<<fn<<"\n"; return; }
    string line; getline(in,line); int loaded=0;
    while (getline(in,line)) {
        if (line.size()<1) continue;
        string cols[9]; int n = splitCSV(line, cols, 9);
        if (n < 2) continue;
        if (communityFacilityCount >= COMMUNITY_MAX_FACILITIES) { cout<<"Reached facilities capacity\n"; break; }
        communityFacility &f = communityFacilities[communityFacilityCount];
        f.facility_id = toInt(cols[0]);
        f.name = cols[1];
        f.type = (n>2)? cols[2] : "";
        f.capacity = (n>3)? toInt(cols[3]) : 0;
        f.price_per_hour = (n>4)? toDouble(cols[4]) : 0.0;
        f.location = (n>5)? cols[5] : "";
        f.available_from = (n>6)? cols[6] : "";
        f.available_to = (n>7)? cols[7] : "";
        f.active = (n>8)? toInt(cols[8]) : 1;
        if (f.facility_id==0 || f.name.empty()) continue;
        communityFacilityCount++; loaded++;
    }
    cout<<"Loaded "<<loaded<<" facilities from "<<fn<<"\n";
    in.close();
}

/* Equipment */
void communityLoadEquipmentCSV(const string &fn) {
    ifstream in(fn);
    if (!in.is_open()) { cout<<"Cannot open "<<fn<<"\n"; return; }
    string line; getline(in,line); int loaded=0;
    while (getline(in,line)) {
        if (line.size()<1) continue;
        string cols[6]; int n = splitCSV(line, cols, 6);
        if (n < 2) continue;
        if (communityEquipmentCount >= COMMUNITY_MAX_EQUIPMENT) { cout<<"Reached equipment capacity\n"; break; }
        communityEquipment &e = communityEquipments[communityEquipmentCount];
        e.equipment_id = toInt(cols[0]);
        e.name = cols[1];
        e.quantity_total = (n>2)? toInt(cols[2]) : 0;
        e.quantity_available = (n>3)? toInt(cols[3]) : e.quantity_total;
        e.condition_desc = (n>4)? cols[4] : "";
        e.last_maintenance_date = (n>5)? cols[5] : "";
        if (e.equipment_id==0 || e.name.empty()) continue;
        communityEquipmentCount++; loaded++;
    }
    cout<<"Loaded "<<loaded<<" equipment from "<<fn<<"\n";
    in.close();
}

/* Events */
void communityLoadEventsCSV(const string &fn) {
    ifstream in(fn);
    if (!in.is_open()) { cout<<"Cannot open "<<fn<<"\n"; return; }
    string line; getline(in,line); int loaded=0;
    while (getline(in,line)) {
        if (line.size()<1) continue;
        string cols[10]; int n = splitCSV(line, cols, 10);
        if (n < 2) continue;
        if (communityEventCount >= COMMUNITY_MAX_EVENTS) { cout<<"Reached events capacity\n"; break; }
        communityEvent &ev = communityEvents[communityEventCount];
        ev.event_id = toInt(cols[0]);
        ev.title = cols[1];
        ev.organizer_member_id = (n>2)? toInt(cols[2]) : -1;
        ev.facility_id = (n>3)? toInt(cols[3]) : -1;
        ev.date = (n>4)? cols[4] : "";
        ev.start_time = (n>5)? cols[5] : "";
        ev.end_time = (n>6)? cols[6] : "";
        ev.expected_attendance = (n>7)? toInt(cols[7]) : 0;
        ev.revenue_expected = (n>8)? toDouble(cols[8]) : 0.0;
        ev.status = (n>9)? cols[9] : "";
        if (ev.event_id==0 || ev.title.empty()) continue;
        communityEventCount++; loaded++;
    }
    cout<<"Loaded "<<loaded<<" events from "<<fn<<"\n";
    in.close();
}

/* Bookings */
void communityLoadBookingsCSV(const string &fn) {
    ifstream in(fn);
    if (!in.is_open()) { cout<<"Cannot open "<<fn<<"\n"; return; }
    string line; getline(in,line); int loaded=0;
    while (getline(in,line)) {
        if (line.size()<1) continue;
        string cols[9]; int n = splitCSV(line, cols, 9);
        if (n < 2) continue;
        communityBooking b;
        b.booking_id = toInt(cols[0]);
        b.event_id = (n>1)? toInt(cols[1]) : -1;
        b.member_id = (n>2)? toInt(cols[2]) : -1;
        b.facility_id = (n>3)? toInt(cols[3]) : -1;
        b.date = (n>4)? cols[4] : "";
        b.start_time = (n>5)? cols[5] : "";
        b.end_time = (n>6)? cols[6] : "";
        b.total_amount = (n>7)? toDouble(cols[7]) : 0.0;
        b.payment_status = (n>8)? cols[8] : "";
        if (b.booking_id==0) continue;
        if (communityBookingCount >= COMMUNITY_MAX_BOOKINGS) { cout<<"Reached bookings capacity\n"; break; }
        if (communityCheckBookingOverlap(b.facility_id, b.date, b.start_time, b.end_time)) {
            cout<<"Skipping overlapping booking id "<<b.booking_id<<"\n";
            continue;
        }
        communityBookings[communityBookingCount++] = b;
        loaded++;
    }
    cout<<"Loaded "<<loaded<<" bookings from "<<fn<<"\n";
    in.close();
}

/* Revenue & Expenses */
void communityLoadRevenueCSV(const string &fn) {
    ifstream in(fn);
    if (!in.is_open()) { cout<<"Cannot open "<<fn<<"\n"; return; }
    string line; getline(in,line); int loaded=0;
    while (getline(in,line)) {
        if (line.size()<1) continue;
        string cols[6]; int n = splitCSV(line, cols, 6);
        if (n < 2) continue;
        if (communityRevenueCount >= COMMUNITY_MAX_TRANSACTIONS) { cout<<"Reached revenue capacity\n"; break; }
        communityRevenue &r = communityRevenues[communityRevenueCount];
        r.revenue_id = toInt(cols[0]);
        r.source_type = cols[1];
        r.source_id = (n>2)? toInt(cols[2]) : -1;
        r.date = (n>3)? cols[3] : "";
        r.amount = (n>4)? toDouble(cols[4]) : 0.0;
        r.description = (n>5)? cols[5] : "";
        if (r.revenue_id==0) continue;
        communityRevenueCount++; loaded++;
    }
    cout<<"Loaded "<<loaded<<" revenue rows from "<<fn<<"\n";
    in.close();
}

void communityLoadExpensesCSV(const string &fn) {
    ifstream in(fn);
    if (!in.is_open()) { cout<<"Cannot open "<<fn<<"\n"; return; }
    string line; getline(in,line); int loaded=0;
    while (getline(in,line)) {
        if (line.size()<1) continue;
        string cols[7]; int n = splitCSV(line, cols, 7);
        if (n < 2) continue;
        if (communityExpenseCount >= COMMUNITY_MAX_TRANSACTIONS) { cout<<"Reached expenses capacity\n"; break; }
        communityExpense &e = communityExpenses[communityExpenseCount];
        e.expense_id = toInt(cols[0]);
        e.related_event_id = (n>1)? toInt(cols[1]) : -1;
        e.date = (n>2)? cols[2] : "";
        e.amount = (n>3)? toDouble(cols[3]) : 0.0;
        e.vendor = (n>4)? cols[4] : "";
        e.description = (n>5)? cols[5] : "";
        e.expense_type = (n>6)? cols[6] : "";
        if (e.expense_id == 0) continue;
        communityExpenseCount++; loaded++;
    }
    cout<<"Loaded "<<loaded<<" expenses from "<<fn<<"\n";
    in.close();
}

/* ---------------------- LOAD ALL UTILITY ---------------------- */
void communityLoadAllCSVsFromFolder(const string &pathPrefix) {
    string p = pathPrefix;
    if (!p.empty() && p.back() != '/' && p.back() != '\\') p.push_back('/');
    communityLoadMembersCSV(p + "communityMembers.csv");
    communityLoadStaffCSV(p + "communityStaff.csv");
    communityLoadFacilitiesCSV(p + "communityFacilities.csv");
    communityLoadEquipmentCSV(p + "communityEquipment.csv");
    communityLoadEventsCSV(p + "communityEvents.csv");
    communityLoadBookingsCSV(p + "communityBookings.csv");
    communityLoadRevenueCSV(p + "communityRevenue.csv");
    communityLoadExpensesCSV(p + "communityExpenses.csv");
}

/* ---------------------- SORTING (QuickSort) ---------------------- */

/* Booking key: date + start_time */
static string bookingKey(const communityBooking &b) {
    return b.date + " " + b.start_time;
}
static void swapBookings(communityBooking &a, communityBooking &b) {
    communityBooking tmp = a; a = b; b = tmp;
}
static int partitionBookings(communityBooking A[], int l, int r) {
    string p = bookingKey(A[l]);
    int i = l; int j = r + 1;
    while (1) {
        do { i = i + 1; } while (i <= r && bookingKey(A[i]) < p);
        do { j = j - 1; } while (j >= l && bookingKey(A[j]) > p);
        if (i >= j) break;
        swapBookings(A[i], A[j]);
    }
    swapBookings(A[l], A[j]);
    return j;
}
void quickSortBookings(communityBooking A[], int l, int r) {
    if (l < r) {
        int s = partitionBookings(A,l,r);
        quickSortBookings(A,l,s-1);
        quickSortBookings(A,s+1,r);
    }
}
void communitySortBookingsByStartTime() {
    if (communityBookingCount <= 1) return;
    quickSortBookings(communityBookings, 0, communityBookingCount - 1);
    cout<<"Bookings sorted by date+start_time\n";
}

/* Facilities sort by price / capacity */
static string facilityPriceKey(const communityFacility &f) {
    // use textual representation to keep it simple though numeric compare used in partition decisions
    char buf[64];
    sprintf(buf, "%015.2f", f.price_per_hour);
    return string(buf) + "-" + f.name;
}
static string facilityCapacityKey(const communityFacility &f) {
    char buf[32];
    sprintf(buf, "%09d", f.capacity);
    return string(buf) + "-" + f.name;
}
static void swapFacilities(communityFacility &a, communityFacility &b) {
    communityFacility tmp = a; a = b; b = tmp;
}

/* Partition and quicksort for price */
static int partitionFacilitiesPrice(communityFacility A[], int l, int r) {
    double pivot = A[l].price_per_hour;
    int i = l;
    int j = r + 1;
    while (1) {
        do { i = i + 1; } while (i <= r && A[i].price_per_hour < pivot);
        do { j = j - 1; } while (j >= l && A[j].price_per_hour > pivot);
        if (i >= j) break;
        swapFacilities(A[i], A[j]);
    }
    swapFacilities(A[l], A[j]);
    return j;
}
void quickSortFacilitiesPrice(communityFacility A[], int l, int r) {
    if (l < r) {
        int s = partitionFacilitiesPrice(A,l,r);
        quickSortFacilitiesPrice(A,l,s-1);
        quickSortFacilitiesPrice(A,s+1,r);
    }
}
void communitySortFacilitiesByPrice() {
    if (communityFacilityCount <= 1) return;
    quickSortFacilitiesPrice(communityFacilities, 0, communityFacilityCount - 1);
    cout<<"Facilities sorted by price per hour\n";
}

/* Partition and quicksort for capacity */
static int partitionFacilitiesCapacity(communityFacility A[], int l, int r) {
    int pivot = A[l].capacity;
    int i = l;
    int j = r + 1;
    while (1) {
        do { i = i + 1; } while (i <= r && A[i].capacity < pivot);
        do { j = j - 1; } while (j >= l && A[j].capacity > pivot);
        if (i >= j) break;
        swapFacilities(A[i], A[j]);
    }
    swapFacilities(A[l], A[j]);
    return j;
}
void quickSortFacilitiesCapacity(communityFacility A[], int l, int r) {
    if (l < r) {
        int s = partitionFacilitiesCapacity(A,l,r);
        quickSortFacilitiesCapacity(A,l,s-1);
        quickSortFacilitiesCapacity(A,s+1,r);
    }
}
void communitySortFacilitiesByCapacity() {
    if (communityFacilityCount <= 1) return;
    quickSortFacilitiesCapacity(communityFacilities, 0, communityFacilityCount - 1);
    cout<<"Facilities sorted by capacity\n";
}

/* ---------------------- INTERACTIVE MANUAL ADD FUNCTIONS ---------------------- */

void communityAddMemberManual() {
    if (communityMemberCount >= COMMUNITY_MAX_MEMBERS) { cout<<"Overflow\n"; return; }
    communityMember &m = communityMembers[communityMemberCount];
    cout<<"Enter member_id: "; cin>>m.member_id; cin.ignore();
    cout<<"Enter name: "; getline(cin, m.name);
    cout<<"Enter age: "; cin>>m.age; cin.ignore();
    cout<<"Enter phone: "; getline(cin, m.phone);
    cout<<"Enter email: "; getline(cin, m.email);
    cout<<"Enter membership_type: "; getline(cin, m.membership_type);
    cout<<"Enter join_date (YYYY-MM-DD): "; getline(cin, m.join_date);
    cout<<"Enter address: "; getline(cin, m.address);
    m.active = 1;
    communityMemberCount++;
    cout<<"Member added successfully!\n";
}

void communityAddStaffManual() {
    if (communityStaffCount >= COMMUNITY_MAX_STAFF) { cout<<"Overflow\n"; return; }
    communityStaff &s = communityStaffs[communityStaffCount];
    cout<<"Enter staff_id: "; cin>>s.staff_id; cin.ignore();
    cout<<"Enter name: "; getline(cin, s.name);
    cout<<"Enter role: "; getline(cin, s.role);
    cout<<"Enter phone: "; getline(cin, s.phone);
    cout<<"Enter email: "; getline(cin, s.email);
    cout<<"Enter salary_per_month: "; cin>>s.salary_per_month; cin.ignore();
    cout<<"Enter join_date (YYYY-MM-DD): "; getline(cin, s.join_date);
    s.is_active = 1;
    communityStaffCount++;
    cout<<"Staff added successfully!\n";
}

void communityAddFacilityManual() {
    if (communityFacilityCount >= COMMUNITY_MAX_FACILITIES) { cout<<"Overflow\n"; return; }
    communityFacility &f = communityFacilities[communityFacilityCount];
    cout<<"Enter facility_id: "; cin>>f.facility_id; cin.ignore();
    cout<<"Enter name: "; getline(cin, f.name);
    cout<<"Enter type: "; getline(cin, f.type);
    cout<<"Enter capacity: "; cin>>f.capacity; cin.ignore();
    cout<<"Enter price_per_hour: "; cin>>f.price_per_hour; cin.ignore();
    cout<<"Enter location: "; getline(cin, f.location);
    cout<<"Enter available_from (HH:MM): "; getline(cin, f.available_from);
    cout<<"Enter available_to (HH:MM): "; getline(cin, f.available_to);
    f.active = 1;
    communityFacilityCount++;
    cout<<"Facility added!\n";
}

void communityAddEquipmentManual() {
    if (communityEquipmentCount >= COMMUNITY_MAX_EQUIPMENT) { cout<<"Overflow\n"; return; }
    communityEquipment &e = communityEquipments[communityEquipmentCount];
    cout<<"Enter equipment_id: "; cin>>e.equipment_id; cin.ignore();
    cout<<"Enter name: "; getline(cin, e.name);
    cout<<"Enter quantity_total: "; cin>>e.quantity_total; cin.ignore();
    cout<<"Enter quantity_available: "; cin>>e.quantity_available; cin.ignore();
    cout<<"Enter condition: "; getline(cin, e.condition_desc);
    cout<<"Enter last_maintenance_date: "; getline(cin, e.last_maintenance_date);
    communityEquipmentCount++;
    cout<<"Equipment added!\n";
}

void communityAddEventManual() {
    if (communityEventCount >= COMMUNITY_MAX_EVENTS) { cout<<"Overflow\n"; return; }
    communityEvent &ev = communityEvents[communityEventCount];
    cout<<"Enter event_id: "; cin>>ev.event_id; cin.ignore();
    cout<<"Enter title: "; getline(cin, ev.title);
    cout<<"Enter organizer_member_id: "; cin>>ev.organizer_member_id;
    cout<<"Enter facility_id: "; cin>>ev.facility_id; cin.ignore();
    cout<<"Enter date (YYYY-MM-DD): "; getline(cin, ev.date);
    cout<<"Enter start_time (HH:MM): "; getline(cin, ev.start_time);
    cout<<"Enter end_time (HH:MM): "; getline(cin, ev.end_time);
    cout<<"Enter expected attendance: "; cin>>ev.expected_attendance;
    cout<<"Enter expected revenue: "; cin>>ev.revenue_expected; cin.ignore();
    ev.status = "scheduled";
    communityEventCount++;
    cout<<"Event added successfully!\n";
}

void communityAddBookingManual() {
    if (communityBookingCount >= COMMUNITY_MAX_BOOKINGS) { cout<<"Overflow\n"; return; }
    communityBooking b;
    cout<<"Enter booking_id: "; cin>>b.booking_id;
    cout<<"Enter event_id: "; cin>>b.event_id;
    cout<<"Enter member_id: "; cin>>b.member_id;
    cout<<"Enter facility_id: "; cin>>b.facility_id;
    cin.ignore();
    cout<<"Enter date (YYYY-MM-DD): "; getline(cin, b.date);
    cout<<"Enter start_time (HH:MM): "; getline(cin, b.start_time);
    cout<<"Enter end_time (HH:MM): "; getline(cin, b.end_time);
    if (communityCheckBookingOverlap(b.facility_id, b.date, b.start_time, b.end_time)) {
        cout<<"Booking overlaps existing booking. Rejecting.\n";
        return;
    }
    cout<<"Enter total_amount: "; cin>>b.total_amount; cin.ignore();
    cout<<"Enter payment_status: "; getline(cin, b.payment_status);
    communityBookings[communityBookingCount] = b;
    communityBookingCount++;
    cout<<"Booking created.\n";
    if (b.payment_status == "paid") {
        if (communityRevenueCount < COMMUNITY_MAX_TRANSACTIONS) {
            communityRevenue &r = communityRevenues[communityRevenueCount++];
            r.revenue_id = communityRevenueCount; r.source_type = "booking"; r.source_id = b.booking_id; r.date = b.date; r.amount = b.total_amount; r.description = "Booking payment";
        }
    }
}

void communityAddRevenueManual() {
    if (communityRevenueCount >= COMMUNITY_MAX_TRANSACTIONS) { cout<<"Overflow\n"; return; }
    communityRevenue &r = communityRevenues[communityRevenueCount];
    cout<<"Enter revenue_id: "; cin>>r.revenue_id; cin.ignore();
    cout<<"Enter source_type: "; getline(cin, r.source_type);
    cout<<"Enter source_id: "; cin>>r.source_id; cin.ignore();
    cout<<"Enter date (YYYY-MM-DD): "; getline(cin, r.date);
    cout<<"Enter amount: "; cin>>r.amount; cin.ignore();
    cout<<"Enter description: "; getline(cin, r.description);
    communityRevenueCount++;
    cout<<"Revenue added!\n";
}

void communityAddExpenseManual() {
    if (communityExpenseCount >= COMMUNITY_MAX_TRANSACTIONS) { cout<<"Overflow\n"; return; }
    communityExpense &e = communityExpenses[communityExpenseCount];
    cout<<"Enter expense_id: "; cin>>e.expense_id;
    cout<<"Enter related_event_id: "; cin>>e.related_event_id; cin.ignore();
    cout<<"Enter date (YYYY-MM-DD): "; getline(cin, e.date);
    cout<<"Enter amount: "; cin>>e.amount; cin.ignore();
    cout<<"Enter vendor: "; getline(cin, e.vendor);
    cout<<"Enter description: "; getline(cin, e.description);
    cout<<"Enter expense_type: "; getline(cin, e.expense_type);
    communityExpenseCount++;
    cout<<"Expense added!\n";
}

/* ---------------------- REPORTS ---------------------- */

void communityBookingsReportRange(const string &from, const string &to) {
    cout<<"Bookings from "<<from<<" to "<<to<<"\n";
    for (int i=0;i<communityBookingCount;i++){
        communityBooking &b = communityBookings[i];
        if (b.date >= from && b.date <= to) {
            cout<<b.booking_id<<","<<b.event_id<<","<<b.member_id<<","<<b.facility_id<<","<<b.date<<","<<b.start_time<<","<<b.end_time<<","<<b.total_amount<<","<<b.payment_status<<"\n";
        }
    }
}

void communityMonthlyRevenueReport(const string &monthPrefix) {
    double total = 0.0;
    for (int i=0;i<communityRevenueCount;i++){
        communityRevenue &r = communityRevenues[i];
        if (r.date.rfind(monthPrefix,0) == 0) total += r.amount;
    }
    cout<<"Monthly revenue ("<<monthPrefix<<") = "<<total<<"\n";
}

void communityEventPnL(int event_id) {
    double rev = 0.0, exp = 0.0;
    for (int i=0;i<communityRevenueCount;i++){
        communityRevenue &r = communityRevenues[i];
        if (r.source_type == "booking") {
            int bid = r.source_id;
            for (int j=0;j<communityBookingCount;j++){
                if (communityBookings[j].booking_id == bid && communityBookings[j].event_id == event_id) rev += r.amount;
            }
        } else if (r.source_type == "event") {
            if (r.source_id == event_id) rev += r.amount;
        }
    }
    for (int i=0;i<communityExpenseCount;i++){
        communityExpense &e = communityExpenses[i];
        if (e.related_event_id == event_id) exp += e.amount;
    }
    cout<<"Event P&L: event "<<event_id<<" revenue="<<rev<<" expense="<<exp<<" profit="<<(rev-exp)<<"\n";
}
void communityBulkFindOverlaps() {
    // Ensure bookings sorted to make it meaningful
    communitySortBookingsByStartTime();
    for (int i=1;i<communityBookingCount;i++){
        communityBooking &prev = communityBookings[i-1];
        communityBooking &cur = communityBookings[i];
        if (prev.facility_id == cur.facility_id && prev.date == cur.date) {
            if (timeOverlap(cur.date, cur.start_time, cur.end_time, prev.date, prev.start_time, prev.end_time)) {
                cout<<"Overlap: booking "<<prev.booking_id<<" and "<<cur.booking_id<<" facility "<<cur.facility_id<<" date "<<cur.date<<"\n";
            }
        }
    }
}

/* ---------------------- MAIN MENU (switch-case) ---------------------- */

/* Forward declaration (if your mega main already has it, ensure no duplicate) */
void communitySystem();

void communitySystem() {

    while (true) {
        cout << "\n--- Community Centre Menu ---\n";
        cout << "1. Members\n";
        cout << "2. Staff\n";
        cout << "3. Facilities\n";
        cout << "4. Equipment\n";
        cout << "5. Events & Bookings\n";
        cout << "6. Finance\n";
        cout << "7. Reports\n";
        cout << "8. Load ALL Community CSVs\n";
        cout << "0. Return\n";
        cout << "Choice: ";

        int c;
        cin >> c;
        cin.ignore();

        switch (c) {

        /* ---------------- MEMBERS ---------------- */
        case 1: {
            cout << "1:LoadCSV  2:AddManual  3:List\nChoice: ";
            int s; cin >> s; cin.ignore();
            switch (s) {
                case 1: communityLoadMembersCSV("communityMembers.csv"); break;
                case 2: communityAddMemberManual(); break;
                case 3: {
                    cout << "Members (" << communityMemberCount << ")\n";
                    for (int i=0;i<communityMemberCount;i++){
                        communityMember &m = communityMembers[i];
                        cout<<m.member_id<<","<<m.name<<","<<m.phone<<","<<m.email<<","<<m.membership_type<<"\n";
                    }
                    break;
                }
                default: cout<<"Invalid choice.\n";
            }
            break;
        }

        /* ---------------- STAFF ---------------- */
        case 2: {
            cout << "1:LoadCSV  2:AddManual  3:SalaryReport\nChoice: ";
            int s; cin >> s; cin.ignore();
            switch (s) {
                case 1: communityLoadStaffCSV("communityStaff.csv"); break;
                case 2: communityAddStaffManual(); break;
                case 3: {
                    double tot=0;
                    cout<<"Staff List:\n";
                    for (int i=0;i<communityStaffCount;i++){
                        communityStaff &st = communityStaffs[i];
                        cout<<st.staff_id<<","<<st.name<<","<<st.role<<","<<st.salary_per_month<<"\n";
                        tot += st.salary_per_month;
                    }
                    cout<<"Total Payroll = "<<tot<<"\n";
                    break;
                }
                default: cout<<"Invalid choice.\n";
            }
            break;
        }

        /* ---------------- FACILITIES ---------------- */
        case 3: {
            cout << "1:LoadCSV  2:AddManual  3:List  4:SortByPrice  5:SortByCapacity\nChoice: ";
            int s; cin >> s; cin.ignore();
            switch (s) {
                case 1: communityLoadFacilitiesCSV("communityFacilities.csv"); break;
                case 2: communityAddFacilityManual(); break;
                case 3: {
                    cout<<"Facilities ("<<communityFacilityCount<<")\n";
                    for (int i=0;i<communityFacilityCount;i++){
                        communityFacility &f = communityFacilities[i];
                        cout<<f.facility_id<<","<<f.name<<","<<f.type<<","<<f.capacity<<","<<f.price_per_hour<<","<<f.location<<"\n";
                    }
                    break;
                }
                case 4: communitySortFacilitiesByPrice(); break;
                case 5: communitySortFacilitiesByCapacity(); break;
                default: cout<<"Invalid choice.\n";
            }
            break;
        }

        /* ---------------- EQUIPMENT ---------------- */
        case 4: {
            cout << "1:LoadCSV  2:AddManual  3:List\nChoice: ";
            int s; cin >> s; cin.ignore();
            switch (s) {
                case 1: communityLoadEquipmentCSV("communityEquipment.csv"); break;
                case 2: communityAddEquipmentManual(); break;
                case 3: {
                    cout<<"Equipment ("<<communityEquipmentCount<<")\n";
                    for (int i=0;i<communityEquipmentCount;i++){
                        communityEquipment &e = communityEquipments[i];
                        cout<<e.equipment_id<<","<<e.name<<","<<e.quantity_total<<","<<e.quantity_available<<","<<e.condition_desc<<"\n";
                    }
                    break;
                }
                default: cout<<"Invalid choice.\n";
            }
            break;
        }

        /* ---------------- EVENTS & BOOKINGS ---------------- */
        case 5: {
            cout << "1:LoadEvents  2:LoadBookings  3:AddEvent  4:AddBooking  5:SortBookings  6:CheckOverlap\nChoice: ";
            int s; cin >> s; cin.ignore();
            switch (s) {
                case 1: communityLoadEventsCSV("communityEvents.csv"); break;
                case 2: communityLoadBookingsCSV("communityBookings.csv"); break;
                case 3: communityAddEventManual(); break;
                case 4: communityAddBookingManual(); break;
                case 5: communitySortBookingsByStartTime(); break;
                case 6: {
                    communityBulkFindOverlaps();
                    break;
                }
                default: cout<<"Invalid choice.\n";
            }
            break;
        }

        /* ---------------- FINANCE ---------------- */
        case 6: {
            cout << "1:AddRevenue  2:AddExpense  3:EventPnL\nChoice: ";
            int s; cin >> s; cin.ignore();
            switch (s) {
                case 1: communityAddRevenueManual(); break;
                case 2: communityAddExpenseManual(); break;
                case 3: {
                    int eid; cout<<"Event id: "; cin>>eid; cin.ignore();
                    communityEventPnL(eid);
                    break;
                }
                default: cout<<"Invalid choice.\n";
            }
            break;
        }

        /* ---------------- REPORTS ---------------- */
        case 7: {
            cout << "1:BookingsRange  2:MonthlyRevenue\nChoice: ";
            int s; cin >> s; cin.ignore();
            switch (s) {
                case 1: {
                    string f,t;
                    cout<<"From (YYYY-MM-DD): "; getline(cin,f);
                    cout<<"To (YYYY-MM-DD): "; getline(cin,t);
                    communityBookingsReportRange(f,t);
                    break;
                }
                case 2: {
                    string mp; cout<<"Month (YYYY-MM): "; getline(cin, mp);
                    communityMonthlyRevenueReport(mp);
                    break;
                }
                default: cout<<"Invalid choice.\n";
            }
            break;
        }

        /* ---------------- LOAD EVERYTHING AT ONCE ---------------- */
        case 8: {
            cout << "\nLoading ALL community CSVs from current folder...\n";
            communityLoadAllCSVsFromFolder("");
            cout << "Load complete.\n";
            break;
        }

        case 0:
            return;

        default:
            cout << "Invalid choice! Try again.\n";
        }
    }
}
int main()
{
    communitySystem();
}

/* ---------------------- OPTIONAL: Bulk overlap printer ---------------------- */



