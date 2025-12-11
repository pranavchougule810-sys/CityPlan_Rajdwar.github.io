// bazaar_market.cpp
// Standalone Market / Bazaar Management module (CSV-backed)
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

const string BZ_STALL_FILE   = "bazaar_stalls.csv";      // stall_id, vendor_id, category, rent, status
const string BZ_VENDOR_FILE  = "bazaar_vendors.csv";     // vendor_id, name, phone, goods
const string BZ_RENT_FILE    = "bazaar_rents.csv";       // rent_id, stall_id, vendor_id, amount, due_date, status
const string BZ_EVENT_FILE   = "bazaar_events.csv";      // event_id, name, date, description, status
const string BZ_SALES_FILE   = "bazaar_sales.csv";       // sale_id, vendor_id, amount, date

/* -------------------------- UTIL -------------------------- */

string col(const vector<string> &r, size_t idx) {
    if (idx < r.size()) return r[idx];
    return "";
}

/* -------------------------- STALL MANAGEMENT -------------------------- */

void baz_addStall() {
    string id, vendorId, category, rent, status;
    cout << "Stall ID: "; getline(cin, id);
    cout << "Vendor ID (leave blank if unassigned): "; getline(cin, vendorId);
    cout << "Category (Food/Clothes/Electronics/General): "; getline(cin, category);
    cout << "Monthly Rent: "; getline(cin, rent);
    cout << "Status (Available/Occupied): "; getline(cin, status);
    appendCSV(BZ_STALL_FILE, {id, vendorId, category, rent, status});
    cout << "Stall added.\n";
}

void baz_viewStalls() {
    auto rows = readCSV(BZ_STALL_FILE);
    cout << "\n=== STALLS ===\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << col(rows[i],0) << " | Vendor:" << col(rows[i],1)
             << " | " << col(rows[i],2) << " | Rent:" << col(rows[i],3)
             << " | " << col(rows[i],4) << "\n";
    }
}

void baz_assignStall() {
    auto stalls = readCSV(BZ_STALL_FILE);
    cout << "Enter Stall ID: "; string sid; getline(cin, sid);
    cout << "Enter Vendor ID to assign: "; string vid; getline(cin, vid);
    bool ok=false;
    for (size_t i = 1; i < stalls.size(); ++i) {
        if (col(stalls[i],0) == sid) {
            stalls[i][1] = vid;
            stalls[i][4] = "Occupied";
            ok=true; break;
        }
    }
    if (ok) { overwriteCSV(BZ_STALL_FILE, stalls); cout << "Stall assigned.\n"; }
    else cout << "Stall not found.\n";
}

/* -------------------------- VENDOR MANAGEMENT -------------------------- */

void baz_addVendor() {
    string id,name,phone,goods;
    cout << "Vendor ID: "; getline(cin, id);
    cout << "Name: "; getline(cin, name);
    cout << "Phone: "; getline(cin, phone);
    cout << "Goods / Category: "; getline(cin, goods);
    appendCSV(BZ_VENDOR_FILE, {id,name,phone,goods});
    cout << "Vendor added.\n";
}

void baz_viewVendors() {
    auto rows = readCSV(BZ_VENDOR_FILE);
    cout << "\n=== VENDORS ===\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << col(rows[i],0) << " | " << col(rows[i],1)
             << " | " << col(rows[i],2) << " | Goods:" << col(rows[i],3) << "\n";
    }
}

void baz_findVendor() {
    cout << "Enter vendor name or part: "; string q; getline(cin, q);
    string low=q; transform(low.begin(), low.end(), low.begin(), ::tolower);
    auto rows = readCSV(BZ_VENDOR_FILE);
    bool found=false;
    for (size_t i = 1; i < rows.size(); ++i) {
        string nm = col(rows[i],1); string tmp=nm; transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
        if (tmp.find(low) != string::npos) {
            cout << col(rows[i],0) << " | " << nm << " | " << col(rows[i],3) << "\n";
            found=true;
        }
    }
    if(!found) cout<<"No vendor found.\n";
}

/* -------------------------- RENT & FINANCE -------------------------- */

void baz_addRentRecord() {
    string rid, stallId, vendorId, amount, due, status;
    cout << "Rent Record ID: "; getline(cin, rid);
    cout << "Stall ID: "; getline(cin, stallId);
    cout << "Vendor ID: "; getline(cin, vendorId);
    cout << "Amount: "; getline(cin, amount);
    cout << "Due Date (YYYY-MM-DD): "; getline(cin, due);
    cout << "Status (Paid/Unpaid): "; getline(cin, status);
    appendCSV(BZ_RENT_FILE, {rid, stallId, vendorId, amount, due, status});
    cout << "Rent record added.\n";
}

void baz_viewRents() {
    auto rows = readCSV(BZ_RENT_FILE);
    cout << "\n=== RENT RECORDS ===\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << col(rows[i],0) << " | Stall:" << col(rows[i],1) << " | Vendor:" << col(rows[i],2)
             << " | Amt:" << col(rows[i],3) << " | Due:" << col(rows[i],4) << " | " << col(rows[i],5) << "\n";
    }
}

void baz_collectDueRentsForDate() {
    cout << "Enter date (YYYY-MM-DD) to check dues: "; string d; getline(cin, d);
    auto rows = readCSV(BZ_RENT_FILE);
    cout << "\nDue rents on " << d << ":\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        if (col(rows[i],4) == d && (col(rows[i],5) == "Unpaid" || col(rows[i],5) == "unpaid")) {
            cout << col(rows[i],0) << " | Stall:" << col(rows[i],1) << " | Amt:" << col(rows[i],3) << "\n";
        }
    }
}

/* -------------------------- EVENTS & PROMOTIONS -------------------------- */

void baz_addEvent() {
    string id,name,date,desc,status;
    cout << "Event ID: "; getline(cin, id);
    cout << "Event Name: "; getline(cin, name);
    cout << "Date (YYYY-MM-DD): "; getline(cin, date);
    cout << "Description: "; getline(cin, desc);
    cout << "Status (Planned/Completed/Cancelled): "; getline(cin, status);
    appendCSV(BZ_EVENT_FILE, {id,name,date,desc,status});
    cout << "Event added.\n";
}

void baz_viewEvents() {
    auto rows = readCSV(BZ_EVENT_FILE);
    cout << "\n=== EVENTS ===\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << col(rows[i],0) << " | " << col(rows[i],1) << " | " << col(rows[i],2)
             << " | " << col(rows[i],4) << "\n";
    }
}

/* -------------------------- VENDOR SALES -------------------------- */

void baz_recordSale() {
    string sid, vendorId, amount;
    cout << "Sale ID: "; getline(cin, sid);
    cout << "Vendor ID: "; getline(cin, vendorId);
    cout << "Amount: "; getline(cin, amount);
    appendCSV(BZ_SALES_FILE, {sid, vendorId, amount, nowDate()});
    cout << "Sale recorded.\n";
}

void baz_viewSales() {
    auto rows = readCSV(BZ_SALES_FILE);
    cout << "\n=== SALES ===\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << col(rows[i],0) << " | Vendor:" << col(rows[i],1)
             << " | Amt:" << col(rows[i],2) << " | " << col(rows[i],3) << "\n";
    }
}

void baz_totalSalesForVendor() {
    cout << "Enter Vendor ID: "; string vid; getline(cin, vid);
    auto rows = readCSV(BZ_SALES_FILE);
    double sum = 0;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (col(rows[i],1) == vid) sum += atof(col(rows[i],2).c_str());
    }
    cout << "Total sales for " << vid << " = " << sum << "\n";
}

/* -------------------------- SAMPLE DATA LOADER -------------------------- */

void baz_loadSampleData() {
    vector<vector<string>> stalls = {
        {"stall_id","vendor_id","category","rent","status"},
        {"S01","V01","Food","3000","Occupied"},
        {"S02","","Clothes","2500","Available"},
        {"S03","V02","Electronics","5000","Occupied"},
        {"S04","","General","2000","Available"},
        {"S05","V03","Food","3200","Occupied"}
    };
    overwriteCSV(BZ_STALL_FILE, stalls);

    vector<vector<string>> vendors = {
        {"vendor_id","name","phone","goods"},
        {"V01","Rakesh","9876501234","Snacks"},
        {"V02","Sunita","9123405678","Mobile Accessories"},
        {"V03","Aman","9988776655","Street Food"},
        {"V04","Leela","9001122334","Clothing"}
    };
    overwriteCSV(BZ_VENDOR_FILE, vendors);

    vector<vector<string>> rents = {
        {"rent_id","stall_id","vendor_id","amount","due_date","status"},
        {"R01","S01","V01","3000","2025-12-01","Paid"},
        {"R02","S03","V02","5000","2025-12-05","Unpaid"}
    };
    overwriteCSV(BZ_RENT_FILE, rents);

    vector<vector<string>> events = {
        {"event_id","name","date","description","status"},
        {"E01","Winter Sale","2025-12-15","Discounts across stalls","Planned"},
        {"E02","Food Fest","2025-11-20","Street food special","Completed"}
    };
    overwriteCSV(BZ_EVENT_FILE, events);

    vector<vector<string>> sales = {
        {"sale_id","vendor_id","amount","date"},
        {"SA01","V01","1200","2025-01-01"},
        {"SA02","V03","2400","2025-01-02"}
    };
    overwriteCSV(BZ_SALES_FILE, sales);

    cout << "Bazaar sample data loaded.\n";
}

/* -------------------------- MAIN MENU -------------------------- */

void baz_mainMenu() {
    while (true) {
        cout << "\n=== BAZAAR / MARKET MANAGEMENT ===\n";
        cout << "1. Stalls\n2. Vendors\n3. Rents\n4. Events\n5. Sales\n6. Load Sample Data\n7. Back / Exit\n";
        cout << "Choice: ";
        string c; getline(cin, c);

        if (c == "1") {
            while (true) {
                cout << "\nStalls Menu\n1.Add Stall\n2.View Stalls\n3.Assign Stall\n4.Back\nChoice: ";
                string s; getline(cin, s);
                if (s == "1") baz_addStall();
                else if (s == "2") baz_viewStalls();
                else if (s == "3") baz_assignStall();
                else break;
            }
        } else if (c == "2") {
            while (true) {
                cout << "\nVendors Menu\n1.Add Vendor\n2.View Vendors\n3.Find Vendor\n4.Back\nChoice: ";
                string s; getline(cin, s);
                if (s == "1") baz_addVendor();
                else if (s == "2") baz_viewVendors();
                else if (s == "3") baz_findVendor();
                else break;
            }
        } else if (c == "3") {
            while (true) {
                cout << "\nRents Menu\n1.Add Rent Record\n2.View Rents\n3.Check Dues by Date\n4.Back\nChoice: ";
                string s; getline(cin, s);
                if (s == "1") baz_addRentRecord();
                else if (s == "2") baz_viewRents();
                else if (s == "3") baz_collectDueRentsForDate();
                else break;
            }
        } else if (c == "4") {
            while (true) {
                cout << "\nEvents Menu\n1.Add Event\n2.View Events\n3.Back\nChoice: ";
                string s; getline(cin, s);
                if (s == "1") baz_addEvent();
                else if (s == "2") baz_viewEvents();
                else break;
            }
        } else if (c == "5") {
            while (true) {
                cout << "\nSales Menu\n1.Record Sale\n2.View Sales\n3.Total Sales for Vendor\n4.Back\nChoice: ";
                string s; getline(cin, s);
                if (s == "1") baz_recordSale();
                else if (s == "2") baz_viewSales();
                else if (s == "3") baz_totalSalesForVendor();
                else break;
            }
        } else if (c == "6") baz_loadSampleData();
        else if (c == "7") break;
        else cout << "Invalid choice.\n";
    }
}

/* -------------------------- STANDALONE MAIN -------------------------- */

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    baz_mainMenu();
    cout << "Exiting Bazaar module. Goodbye!\n";
    return 0;
}
