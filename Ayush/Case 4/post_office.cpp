// post_office.cpp
// Standalone Post Office Management (helpers + post module + main)
// Extracted from integrated_management.cpp. See source. :contentReference[oaicite:2]{index=2}

#include <bits/stdc++.h>
using namespace std;

// ---------- CSV helpers (shared) ----------
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

// ---------- Time helpers (shared) ----------
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

// ---------- Merge sort for CSV rows by column (shared) ----------
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

// -----------------------------
// POST OFFICE MANAGEMENT
// -----------------------------
const string POST_PARCEL_FILE = "post_parcels.csv";
const string POST_CUSTOMER_FILE = "post_customers.csv";
const string POST_DISPATCH_FILE = "post_dispatch.csv";
const string POST_TRACK_FILE = "post_tracking.csv";

// ------------------- Register Parcel -------------------
void post_registerParcel() {
    string pid, sender, receiver, weight, type, notes;
    cout << "Parcel ID: "; getline(cin, pid);
    cout << "Sender Name: "; getline(cin, sender);
    cout << "Receiver Name: "; getline(cin, receiver);
    cout << "Weight (kg): "; getline(cin, weight);
    cout << "Parcel Type (Speed/Standard): "; getline(cin, type);
    cout << "Notes: "; getline(cin, notes);

    appendCSV(POST_PARCEL_FILE,
              {pid, sender, receiver, weight, type, "registered", nowDate(), notes});

    cout << "Parcel registered.\n";
}

// ------------------- Track Parcel -------------------
void post_trackParcel() {
    cout << "Enter Parcel ID: ";
    string pid; getline(cin, pid);

    auto rows = readCSV(POST_TRACK_FILE);
    bool found = false;

    cout << "\nTRACKING DETAILS:\n";
    for (size_t i = 1; i < rows.size(); i++) {
        if (rows[i][0] == pid) {
            cout << rows[i][1] << " | Status: " << rows[i][2]
                 << " | Location: " << rows[i][3]
                 << " | Time: " << rows[i][4] << "\n";
            found = true;
        }
    }

    if (!found) cout << "No tracking updates found.\n";
}

// ------------------- Update Parcel Status -------------------
void post_updateParcelStatus() {
    string pid, status, location;
    cout << "Parcel ID: "; getline(cin, pid);
    cout << "New Status (In-Transit/Delivered/Delayed): ";
    getline(cin, status);
    cout << "Location: ";
    getline(cin, location);

    appendCSV(POST_TRACK_FILE,
              {pid, nowDate(), status, location, nowTimeHHMM()});

    cout << "Tracking updated.\n";
}

// ------------------- Customer Records -------------------
void post_addCustomer() {
    string cid, name, phone, addr, notes;
    cout << "Customer ID: "; getline(cin, cid);
    cout << "Name: "; getline(cin, name);
    cout << "Phone: "; getline(cin, phone);
    cout << "Address: "; getline(cin, addr);
    cout << "Notes: "; getline(cin, notes);

    appendCSV(POST_CUSTOMER_FILE, {cid, name, phone, addr, notes});
    cout << "Customer added.\n";
}

void post_viewCustomers() {
    auto rows = readCSV(POST_CUSTOMER_FILE);
    if (rows.size() <= 1) {
        cout << "No customers.\n";
        return;
    }
    cout << "CUSTOMER LIST:\n";
    for (size_t i = 1; i < rows.size(); i++) {
        cout << rows[i][0] << " | " << rows[i][1]
             << " | " << rows[i][2]
             << " | " << rows[i][3] << "\n";
    }
}

// ------------------- Dispatch Parcel -------------------
void post_dispatchParcel() {
    string pid, vehicle, driver, notes;
    cout << "Parcel ID: "; getline(cin, pid);
    cout << "Vehicle No: "; getline(cin, vehicle);
    cout << "Driver Name: "; getline(cin, driver);
    cout << "Notes: "; getline(cin, notes);

    appendCSV(POST_DISPATCH_FILE,
              {pid, vehicle, driver, nowDate(), nowTimeHHMM(), notes});

    cout << "Parcel dispatched.\n";
}

void post_viewDispatchReport() {
    auto rows = readCSV(POST_DISPATCH_FILE);
    if (rows.size() <= 1) {
        cout << "No dispatch entries.\n";
        return;
    }
    cout << "DISPATCH REPORT:\n";
    for (size_t i = 1; i < rows.size(); i++) {
        cout << rows[i][0] << " | Vehicle:" << rows[i][1]
             << " | Driver:" << rows[i][2]
             << " | Date:" << rows[i][3]
             << " | Time:" << rows[i][4] << "\n";
    }
}

// ------------------- Load Sample Data -------------------
void post_loadSampleData() {

    // parcel header + sample 5 rows
    vector<vector<string>> parcels = {
        {"parcel_id","sender","receiver","weight","type","status","date","notes"},
        {"P001","Amit","Rohan","1.5","Speed","registered","2025-01-10","Fragile"},
        {"P002","Sara","Mehul","2.0","Standard","registered","2025-01-12","None"},
        {"P003","Pooja","Kiran","0.5","Speed","registered","2025-01-11","Urgent"},
        {"P004","Ravi","Nitin","3.2","Standard","registered","2025-01-14","Heavy"},
        {"P005","John","Arun","1.0","Speed","registered","2025-01-15","None"}
    };
    overwriteCSV(POST_PARCEL_FILE, parcels);

    // customer header + sample rows
    vector<vector<string>> cust = {
        {"cust_id","name","phone","address","notes"},
        {"C001","Amit","9123456780","Delhi","Frequent sender"},
        {"C002","Sara","9988776655","Mumbai","VIP"},
        {"C003","Pooja","9876501234","Pune","None"}
    };
    overwriteCSV(POST_CUSTOMER_FILE, cust);

    // dispatch header
    vector<vector<string>> disp = {
        {"parcel_id","vehicle","driver","date","time","notes"},
        {"P001","MH12AB1234","Ramesh","2025-01-11","10:30","On route"},
        {"P002","MH14CD5678","Suresh","2025-01-13","09:00","Left facility"}
    };
    overwriteCSV(POST_DISPATCH_FILE, disp);

    // tracking header
    vector<vector<string>> track = {
        {"parcel_id","date","status","location","time"},
        {"P001","2025-01-11","In-Transit","Sorting Center","10:45"},
        {"P001","2025-01-12","In-Transit","Delhi Office","11:50"},
        {"P002","2025-01-13","In-Transit","Mumbai Hub","09:15"}
    };
    overwriteCSV(POST_TRACK_FILE, track);

    cout << "Post Office sample data loaded.\n";
}

// ------------------- POST OFFICE MAIN MENU -------------------
void post_mainMenu() {
    while (true) {
        cout << "\n=== POST OFFICE MANAGEMENT ===\n";
        cout << "1. Register Parcel\n";
        cout << "2. Track Parcel\n";
        cout << "3. Update Parcel Status\n";
        cout << "4. Customer Records\n";
        cout << "5. Dispatch Parcel\n";
        cout << "6. View Dispatch Report\n";
        cout << "7. Load Sample Data\n";
        cout << "8. Back to MAIN MENU\n";
        cout << "Choice: ";

        string c;
        getline(cin, c);

        if (c == "1") post_registerParcel();
        else if (c == "2") post_trackParcel();
        else if (c == "3") post_updateParcelStatus();
        else if (c == "4") {
            cout << "\n1. Add Customer\n2. View Customers\nChoice: ";
            string cc; getline(cin, cc);
            if (cc == "1") post_addCustomer();
            else if (cc == "2") post_viewCustomers();
        }
        else if (c == "5") post_dispatchParcel();
        else if (c == "6") post_viewDispatchReport();
        else if (c == "7") post_loadSampleData();
        else if (c == "8") break;
        else cout << "Invalid choice.\n";
    }
}

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    post_mainMenu();
    return 0;
}
