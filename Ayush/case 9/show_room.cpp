// vehicle_showroom.cpp
// Standalone Vehicle Showroom Management module (CSV-backed)
// Extracted-style standalone module (helpers included so it runs independently)

#include <bits/stdc++.h>
using namespace std;

/* -------------------------- SHARED HELPERS -------------------------- */

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

string nowDate() {
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm local = *localtime(&tt);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &local);
    return string(buf);
}

/* -------------------------- FILE NAMES -------------------------- */

const string VS_VEH_FILE   = "showroom_vehicles.csv";
const string VS_SALES_FILE = "showroom_sales.csv";
const string VS_TESTDRV    = "showroom_testdrives.csv";
// ---- Graph for Vehicle Similarity (Adjacency List) ----
map<string, vector<string>> vs_vehicleGraph;

/* -------------------------- UTILITIES -------------------------- */

string col(const vector<string> &r, size_t idx) {
    if (idx < r.size()) return r[idx];
    return "";
}

/* -------------------------- VEHICLE INVENTORY -------------------------- */

void vs_addVehicle() {
    string id, make, model, year, price, stock, color, notes;
    cout << "Vehicle ID: "; getline(cin, id);
    cout << "Make (e.g. Toyota): "; getline(cin, make);
    cout << "Model: "; getline(cin, model);
    cout << "Year: "; getline(cin, year);
    cout << "Price: "; getline(cin, price);
    cout << "Stock (units): "; getline(cin, stock);
    cout << "Color: "; getline(cin, color);
    cout << "Notes: "; getline(cin, notes);

    appendCSV(VS_VEH_FILE, {id, make, model, year, price, stock, color, notes});
    cout << "Vehicle added to inventory.\n";
}

void vs_viewVehicles() {
    auto rows = readCSV(VS_VEH_FILE);
    if (rows.size() <= 1) {
        cout << "No vehicles in showroom.\n";
        return;
    }

    // -------- BUILD GRAPH (vehicles with same make) --------
    vs_vehicleGraph.clear();

    for (size_t i = 1; i < rows.size(); ++i) {
        for (size_t j = 1; j < rows.size(); ++j) {
            if (i != j && col(rows[i],1) == col(rows[j],1)) {
                vs_vehicleGraph[col(rows[i],0)].push_back(col(rows[j],0));
            }
        }
    }

    // -------- BFS TRAVERSAL --------
    queue<string> q;
    set<string> visited;

    string start = col(rows[1],0);   // first vehicle
    q.push(start);
    visited.insert(start);

    cout << "\n=== VEHICLE INVENTORY (BFS ORDER) ===\n";

    while (!q.empty()) {
        string u = q.front(); q.pop();

        // print vehicle details
        for (size_t i = 1; i < rows.size(); ++i) {
            if (col(rows[i],0) == u) {
                cout << col(rows[i],0) << " | "
                     << col(rows[i],1) << " " << col(rows[i],2)
                     << " | Year:" << col(rows[i],3)
                     << " | Price:" << col(rows[i],4)
                     << " | Stock:" << col(rows[i],5)
                     << " | Color:" << col(rows[i],6) << "\n";
                break;
            }
        }

        for (auto &v : vs_vehicleGraph[u]) {
            if (!visited.count(v)) {
                visited.insert(v);
                q.push(v);
            }
        }
    }
}


void vs_updatePrice() {
    auto rows = readCSV(VS_VEH_FILE);
    cout << "Enter Vehicle ID to update price: ";
    string id; getline(cin, id);
    cout << "Enter new price: "; string np; getline(cin, np);
    bool found = false;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (col(rows[i],0) == id) {
            rows[i][4] = np;
            found = true; break;
        }
    }
    if (found) { overwriteCSV(VS_VEH_FILE, rows); cout << "Price updated.\n"; }
    else cout << "Vehicle ID not found.\n";
}

void vs_updateStock() {
    auto rows = readCSV(VS_VEH_FILE);
    cout << "Enter Vehicle ID to update stock: ";
    string id; getline(cin, id);
    cout << "Change in stock (e.g. 2 or -1): "; string ds; getline(cin, ds);
    int delta = atoi(ds.c_str());
    bool found = false;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (col(rows[i],0) == id) {
            int st = atoi(col(rows[i],5).c_str());
            rows[i][5] = to_string(st + delta);
            found = true; break;
        }
    }
    if (found) { overwriteCSV(VS_VEH_FILE, rows); cout << "Stock updated.\n"; }
    else cout << "Vehicle not found.\n";
}

/* -------------------------- SALES & FINANCE -------------------------- */

void vs_recordSale() {
    auto veh = readCSV(VS_VEH_FILE);
    cout << "Sale ID: "; string sid; getline(cin, sid);
    cout << "Vehicle ID: "; string vid; getline(cin, vid);

    bool found = false;
    double price = 0;
    int stock = 0;
    for (size_t i = 1; i < veh.size(); ++i) {
        if (col(veh[i],0) == vid) {
            price = atof(col(veh[i],4).c_str());
            stock = atoi(col(veh[i],5).c_str());
            found = true; break;
        }
    }
    if (!found) { cout << "Vehicle not found.\n"; return; }

    cout << "Buyer Name: "; string buyer; getline(cin, buyer);
    cout << "Quantity (units): "; string qs; getline(cin, qs);
    int q = atoi(qs.c_str());
    if (q > stock) { cout << "Not enough stock.\n"; return; }

    double total = price * q;
    cout << "Downpayment (enter 0 if full payment): "; string dp_s; getline(cin, dp_s);
    double dp = atof(dp_s.c_str());
    string paymentStatus = (dp >= total ? "Paid" : "Pending");

    appendCSV(VS_SALES_FILE, {sid, vid, buyer, to_string(q), to_string(total), to_string(dp), paymentStatus, nowDate()});

    // reduce stock
    for (size_t i = 1; i < veh.size(); ++i) {
        if (col(veh[i],0) == vid) {
            int st = atoi(col(veh[i],5).c_str());
            veh[i][5] = to_string(st - q);
            break;
        }
    }
    overwriteCSV(VS_VEH_FILE, veh);

    cout << "Sale recorded. Total = " << total << " | Status: " << paymentStatus << "\n";
}

void vs_viewSales() {
    auto rows = readCSV(VS_SALES_FILE);
    cout << "\n=== SALES HISTORY ===\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << col(rows[i],0) << " | Vehicle:" << col(rows[i],1)
             << " | Buyer:" << col(rows[i],2) << " | Qty:" << col(rows[i],3)
             << " | Total:" << col(rows[i],4) << " | Paid:" << col(rows[i],5)
             << " | " << col(rows[i],6) << " | " << col(rows[i],7) << "\n";
    }
}

void vs_totalSalesOfDay() {
    cout << "Enter date (YYYY-MM-DD): "; string d; getline(cin, d);
    auto rows = readCSV(VS_SALES_FILE);
    double sum = 0;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (col(rows[i],7) == d) sum += atof(col(rows[i],4).c_str());
    }
    cout << "Total sales on " << d << " = " << sum << "\n";
}

/* -------------------------- TEST DRIVES -------------------------- */

void vs_scheduleTestDrive() {
    cout << "TestDrive ID: "; string tid; getline(cin, tid);
    cout << "Vehicle ID: "; string vid; getline(cin, vid);
    cout << "Customer Name: "; string cname; getline(cin, cname);
    cout << "Date (YYYY-MM-DD): "; string date; getline(cin, date);
    cout << "Time (HH:MM): "; string time; getline(cin, time);
    cout << "Contact: "; string phone; getline(cin, phone);

    appendCSV(VS_TESTDRV, {tid, vid, cname, date, time, phone, "Scheduled"});
    cout << "Test drive scheduled.\n";
}

void vs_viewTestDrives() {
    auto rows = readCSV(VS_TESTDRV);
    cout << "\n=== TEST DRIVES ===\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << col(rows[i],0) << " | Vehicle:" << col(rows[i],1)
             << " | " << col(rows[i],2) << " | " << col(rows[i],3) << " " << col(rows[i],4)
             << " | " << col(rows[i],6) << "\n";
    }
}

void vs_updateTestDriveStatus() {
    auto rows = readCSV(VS_TESTDRV);
    cout << "Enter TestDrive ID: "; string tid; getline(cin, tid);
    cout << "Enter new status (Completed/Cancelled): "; string ns; getline(cin, ns);
    bool found = false;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (col(rows[i],0) == tid) {
            rows[i][6] = ns;
            found = true; break;
        }
    }
    if (found) { overwriteCSV(VS_TESTDRV, rows); cout << "Status updated.\n"; }
    else cout << "TestDrive ID not found.\n";
}

/* -------------------------- SAMPLE DATA -------------------------- */

void vs_loadSampleData() {
    vector<vector<string>> vehicles = {
        {"id","make","model","year","price","stock","color","notes"},
        {"V01","Maruti","Swift","2024","650000","5","Red","Hatchback"},
        {"V02","Hyundai","Creta","2024","1200000","3","White","SUV"},
        {"V03","Tata","Nexon","2023","900000","4","Blue","Electric option"},
        {"V04","Kia","Seltos","2024","1350000","2","Black","Premium"},
        {"V05","Toyota","Urban Cruiser","2024","1100000","1","Silver","Compact SUV"}
    };
    overwriteCSV(VS_VEH_FILE, vehicles);

    vector<vector<string>> sales = {
        {"sale_id","vehicle_id","buyer","qty","total","downpayment","status","date"},
        {"SA01","V01","Ramesh","1","650000","650000","Paid","2025-01-05"},
        {"SA02","V03","Sita","1","900000","300000","Pending","2025-01-07"}
    };
    overwriteCSV(VS_SALES_FILE, sales);

    vector<vector<string>> tds = {
        {"td_id","vehicle_id","customer","date","time","phone","status"},
        {"TD01","V02","Amit","2025-01-10","10:00","9876543210","Scheduled"},
        {"TD02","V04","Neha","2025-01-11","11:30","9123456780","Scheduled"}
    };
    overwriteCSV(VS_TESTDRV, tds);

    cout << "Vehicle Showroom sample data loaded.\n";
}

/* -------------------------- MAIN MENU -------------------------- */

void vs_mainMenu() {
    while (true) {
        cout << "\n=== VEHICLE SHOWROOM MANAGEMENT ===\n";
        cout << "1. Inventory\n2. Sales\n3. Test Drives\n4. Load Sample Data\n5. Back / Exit\n";
        cout << "Choice: ";
        string c; getline(cin, c);

        if (c == "1") {
            while (true) {
                cout << "\nInventory Menu\n1.Add Vehicle\n2.View Vehicles\n3.Update Price\n4.Update Stock\n5.Back\nChoice: ";
                string s; getline(cin, s);
                if (s == "1") vs_addVehicle();
                else if (s == "2") vs_viewVehicles();
                else if (s == "3") vs_updatePrice();
                else if (s == "4") vs_updateStock();
                else break;
            }
        } else if (c == "2") {
            while (true) {
                cout << "\nSales Menu\n1.Record Sale\n2.View Sales\n3.Total Sales of Day\n4.Back\nChoice: ";
                string s; getline(cin, s);
                if (s == "1") vs_recordSale();
                else if (s == "2") vs_viewSales();
                else if (s == "3") vs_totalSalesOfDay();
                else break;
            }
        } else if (c == "3") {
            while (true) {
                cout << "\nTest Drives Menu\n1.Schedule\n2.View\n3.Update Status\n4.Back\nChoice: ";
                string s; getline(cin, s);
                if (s == "1") vs_scheduleTestDrive();
                else if (s == "2") vs_viewTestDrives();
                else if (s == "3") vs_updateTestDriveStatus();
                else break;
            }
        } else if (c == "4") vs_loadSampleData();
        else if (c == "5") break;
        else cout << "Invalid choice.\n";
    }
}

/* -------------------------- STANDALONE MAIN -------------------------- */

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    vs_mainMenu();
    cout << "Exiting Vehicle Showroom module. Goodbye!\n";
    return 0;
}
