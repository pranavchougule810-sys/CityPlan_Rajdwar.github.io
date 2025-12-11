// petrol_pump.cpp
// Standalone Petrol Pump Management module (CSV-backed)
// Extracted from integrated_management.cpp. Source: user's integrated file. :contentReference[oaicite:1]{index=1}

#include <bits/stdc++.h>
using namespace std;

// ----------------- SHARED HELPERS (CSV / Time / MergeSort) -----------------
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

// Merge sort utilities (kept for consistency — taken from integrated file)
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

// ----------------- PETROL PUMP MODULE -----------------
const string P_FUEL_FILE = "pump_fuel.csv";
const string P_SALES_FILE = "pump_sales.csv";
const string P_ATTENDANT_FILE = "pump_attendants.csv";
const string P_EXPENSE_FILE = "pump_expenses.csv";
const string P_TANK_FILE = "pump_tanks.csv";

/* ---------------------------------------------------------
                 FUEL INVENTORY FUNCTIONS
--------------------------------------------------------- */

void pump_addFuelType() {
    string id, name, price, stock, octane, notes;
    cout << "Fuel ID: "; getline(cin, id);
    cout << "Fuel Name (Petrol/Diesel/Power/etc): "; getline(cin, name);
    cout << "Price per Litre: "; getline(cin, price);
    cout << "Available Stock (Litres): "; getline(cin, stock);
    cout << "Octane Rating: "; getline(cin, octane);
    cout << "Notes: "; getline(cin, notes);

    appendCSV(P_FUEL_FILE, {id,name,price,stock,octane,notes});
    cout << "Fuel type added.\n";
}

void pump_viewFuelTypes() {
    auto rows = readCSV(P_FUEL_FILE);
    if (rows.size() <= 1) {
        cout << "No fuel types available.\n";
        return;
    }

    cout << "\n=== FUEL TYPES ===\n";
    for (size_t i = 1; i < rows.size(); i++) {
        cout << rows[i][0] << " | " << rows[i][1]
             << " | Price:" << rows[i][2]
             << " | Stock:" << rows[i][3]
             << " | Octane:" << rows[i][4] << "\n";
    }
}

void pump_updateFuelPrice() {
    auto rows = readCSV(P_FUEL_FILE);
    cout << "Enter Fuel ID to update price: ";
    string id; getline(cin, id);

    cout << "Enter new price: ";
    string newPrice; getline(cin, newPrice);

    bool found = false;
    for (size_t i = 1; i < rows.size(); i++) {
        if (rows[i][0] == id) {
            rows[i][2] = newPrice;
            found = true;
            break;
        }
    }

    if (found) {
        overwriteCSV(P_FUEL_FILE, rows);
        cout << "Fuel price updated.\n";
    } else cout << "Fuel ID not found.\n";
}

void pump_updateStock() {
    auto rows = readCSV(P_FUEL_FILE);

    cout << "Fuel ID to update stock: ";
    string id; getline(cin, id);

    cout << "Add stock (L): ";
    string qty; getline(cin, qty);
    double q = atof(qty.c_str());

    bool ok=false;
    for (size_t i = 1; i < rows.size(); i++) {
        if (rows[i][0] == id) {
            double prev = atof(rows[i][3].c_str());
            rows[i][3] = to_string(prev + q);
            ok = true;
            break;
        }
    }

    if (ok) {
        overwriteCSV(P_FUEL_FILE, rows);
        cout << "Stock updated successfully.\n";
    } else cout << "Fuel not found.\n";
}

/* ---------------------------------------------------------
                   FUEL SALE & BILLING
--------------------------------------------------------- */

void pump_newSale() {
    auto fuel = readCSV(P_FUEL_FILE);

    cout << "Sale ID: ";
    string sid; getline(cin, sid);

    cout << "Fuel ID: ";
    string fid; getline(cin, fid);

    bool found = false;
    double price = 0;
    double stock = 0;

    for (size_t i = 1; i < fuel.size(); i++) {
        if (fuel[i][0] == fid) {
            found = true;
            price = atof(fuel[i][2].c_str());
            stock = atof(fuel[i][3].c_str());
            break;
        }
    }

    if (!found) {
        cout << "Fuel ID not found.\n";
        return;
    }

    cout << "Litres sold: ";
    string q; getline(cin, q);
    double qty = atof(q.c_str());

    if (qty > stock) {
        cout << "Not enough stock.\n";
        return;
    }

    double amount = qty * price;

    appendCSV(P_SALES_FILE,
              {sid, fid, q, to_string(amount),
               nowDate(), nowTimeHHMM(), "completed"});

    // reduce stock:
    for (size_t i = 1; i < fuel.size(); i++) {
        if (fuel[i][0] == fid) {
            double st = atof(fuel[i][3].c_str());
            fuel[i][3] = to_string(st - qty);
        }
    }
    overwriteCSV(P_FUEL_FILE, fuel);

    cout << "Sale recorded. Amount = " << amount << "\n";
}

void pump_viewSales() {
    auto rows = readCSV(P_SALES_FILE);
    cout << "\n=== SALES RECORD ===\n";

    for (size_t i = 1; i < rows.size(); i++) {
        cout << rows[i][0] << " | Fuel:" << rows[i][1]
             << " | Qty:" << rows[i][2]
             << " | Amount:" << rows[i][3]
             << " | " << rows[i][4] << " " << rows[i][5] << "\n";
    }
}

void pump_totalSalesOfDay() {
    cout << "Enter date (YYYY-MM-DD): ";
    string d; getline(cin, d);

    auto rows = readCSV(P_SALES_FILE);
    double sum = 0;

    for (size_t i = 1; i < rows.size(); i++) {
        if (rows[i][4] == d) {
            sum += atof(rows[i][3].c_str());
        }
    }

    cout << "Total sales for " << d << " = " << sum << "\n";
}

/* ---------------------------------------------------------
                  ATTENDANT MANAGEMENT
--------------------------------------------------------- */

void pump_addAttendant() {
    string id,name,shift,phone;
    cout << "Attendant ID: "; getline(cin,id);
    cout << "Name: "; getline(cin,name);
    cout << "Shift (Day/Night): "; getline(cin,shift);
    cout << "Phone: "; getline(cin,phone);

    appendCSV(P_ATTENDANT_FILE, {id,name,shift,phone});
    cout << "Attendant added.\n";
}

void pump_viewAttendants() {
    auto rows = readCSV(P_ATTENDANT_FILE);

    cout << "\n=== PUMP ATTENDANTS ===\n";
    for (size_t i = 1; i < rows.size(); i++) {
        cout << rows[i][0] << " | " << rows[i][1]
             << " | Shift:" << rows[i][2]
             << " | Phone:" << rows[i][3] << "\n";
    }
}

void pump_findAttendant() {
    cout << "Enter name to search: ";
    string term; getline(cin, term);

    auto rows = readCSV(P_ATTENDANT_FILE);
    string low = term;
    transform(low.begin(), low.end(), low.begin(), ::tolower);

    bool found = false;

    for (size_t i = 1; i < rows.size(); i++) {
        string tmp = rows[i][1];
        transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);

        if (tmp.find(low) != string::npos) {
            cout << "Found: " << rows[i][0] << " | " << rows[i][1] << "\n";
            found = true;
        }
    }

    if (!found) cout << "No match.\n";
}

/* ---------------------------------------------------------
                    PUMP EXPENSE TRACKING
--------------------------------------------------------- */

void pump_addExpense() {
    string id, type, amount, date, notes;

    cout << "Expense ID: "; getline(cin, id);
    cout << "Type (Electricity/Maintenance/etc): "; getline(cin, type);
    cout << "Amount: "; getline(cin, amount);
    cout << "Date (YYYY-MM-DD): "; getline(cin, date);
    cout << "Notes: "; getline(cin, notes);

    appendCSV(P_EXPENSE_FILE, {id,type,amount,date,notes});
    cout << "Expense added.\n";
}

void pump_viewExpenses() {
    auto rows = readCSV(P_EXPENSE_FILE);

    cout << "\n=== EXPENSE LIST ===\n";
    for (size_t i = 1; i < rows.size(); i++) {
        cout << rows[i][0] << " | " << rows[i][1]
             << " | Amt:" << rows[i][2]
             << " | " << rows[i][3] << "\n";
    }
}

void pump_totalExpenses() {
    auto rows = readCSV(P_EXPENSE_FILE);
    double sum = 0;

    for (size_t i = 1; i < rows.size(); i++) {
        sum += atof(rows[i][2].c_str());
    }
    cout << "Total expenses = " << sum << "\n";
}

/* ---------------------------------------------------------
                    FUEL TANK MONITORING
--------------------------------------------------------- */

void pump_addTank() {
    string id,fuelId,capacity,level,notes;
    cout << "Tank ID: "; getline(cin,id);
    cout << "Fuel ID: "; getline(cin,fuelId);
    cout << "Capacity (L): "; getline(cin,capacity);
    cout << "Current Level (L): "; getline(cin,level);
    cout << "Notes: "; getline(cin,notes);

    appendCSV(P_TANK_FILE, {id,fuelId,capacity,level,notes});
    cout << "Tank added.\n";
}

void pump_viewTanks() {
    auto rows = readCSV(P_TANK_FILE);

    cout << "\n=== STORAGE TANKS ===\n";
    for (size_t i = 1; i < rows.size(); i++) {
        cout << rows[i][0] << " | Fuel:" << rows[i][1]
             << " | Capacity:" << rows[i][2]
             << " | Level:" << rows[i][3] << "\n";
    }
}

void pump_checkLowTanks() {
    auto rows = readCSV(P_TANK_FILE);

    cout << "\nTANKS BELOW 20%:\n";
    for (size_t i = 1; i < rows.size(); i++) {
        double level = atof(rows[i][3].c_str());
        double cap = atof(rows[i][2].c_str());

        if (level < 0.2 * cap) {
            cout << rows[i][0] << " | Level:" << rows[i][3]
                 << " / " << rows[i][2] << "\n";
        }
    }
}

/* ---------------------------------------------------------
                  SAMPLE DATA LOADER (20 entries)
--------------------------------------------------------- */

void pump_loadSampleData() {

    vector<vector<string>> fuel = {
        {"id","name","price","stock","octane","notes"},
        {"F01","Petrol","106","8000","91","Regular"},
        {"F02","Diesel","94","9000","0","Diesel"},
        {"F03","Power Petrol","118","5000","97","High octane"},
        {"F04","Extra Mile","120","4500","98","Premium"},
        {"F05","AutoGas","55","6000","0","CNG"},
        {"F06","XP95","115","4800","95","Premium fuel"},
        {"F07","XP100","135","3200","100","High performance"},
        {"F08","BioDiesel","88","7000","0","Eco fuel"},
        {"F09","RaceFuel","150","2500","110","Sports"},
        {"F10","Aviation Fuel","170","1000","115","Jet grade"},
        {"F11","Petrol","106","2000","91","Backup Tank"},
        {"F12","Diesel","94","3000","0","Reserve"},
        {"F13","CNG","60","7000","0","Gas stock"},
        {"F14","Petrol","110","4000","93","Seasonal"},
        {"F15","Diesel","99","3500","0","High efficiency"},
        {"F16","Petrol","108","2500","92","Premium 92"},
        {"F17","Power Petrol","119","2800","98","SuperClean"},
        {"F18","CNG","58","6500","0","Type2"},
        {"F19","RaceFuel","155","1600","112","Track"},
        {"F20","XP95","116","3400","95","Blend B"}
    };
    overwriteCSV(P_FUEL_FILE, fuel);

    vector<vector<string>> sales = {
        {"sale_id","fuel_id","qty","amount","date","time","status"},
        {"S01","F01","5","530","2025-01-01","09:05","completed"},
        {"S02","F02","10","940","2025-01-01","10:15","completed"},
        {"S03","F03","4","472","2025-01-02","11:20","completed"},
        {"S04","F08","7","616","2025-01-02","12:10","completed"},
        {"S05","F06","6","690","2025-01-03","13:00","completed"},
        {"S06","F01","8","848","2025-01-03","13:45","completed"},
        {"S07","F07","3","405","2025-01-04","14:30","completed"},
        {"S08","F02","12","1128","2025-01-04","15:20","completed"},
        {"S09","F01","9","954","2025-01-05","16:00","completed"},
        {"S10","F05","15","825","2025-01-05","17:10","completed"}
    };
    overwriteCSV(P_SALES_FILE, sales);

    vector<vector<string>> attendants = {
        {"id","name","shift","phone"},
        {"A01","Ramesh","Day","9876512340"},
        {"A02","Suresh","Night","9123456780"},
        {"A03","Arun","Day","9988776655"},
        {"A04","Vijay","Night","9012345678"},
        {"A05","Kiran","Day","9090909090"},
        {"A06","Ravi","Night","9888877771"},
        {"A07","Mahesh","Day","9445566772"},
        {"A08","Rohit","Night","9556677889"},
        {"A09","Dinesh","Day","9778855443"},
        {"A10","Sanju","Night","9332211445"}
    };
    overwriteCSV(P_ATTENDANT_FILE, attendants);

    vector<vector<string>> expenses = {
        {"id","type","amount","date","notes"},
        {"E01","Electricity","4500","2025-01-01","Monthly bill"},
        {"E02","Maintenance","2500","2025-01-02","Pump repair"},
        {"E03","Cleaning","800","2025-01-03","Daily cleaning"},
        {"E04","Security","1200","2025-01-03","Guard salary"},
        {"E05","Water","600","2025-01-04","Utility payment"}
    };
    overwriteCSV(P_EXPENSE_FILE, expenses);

    vector<vector<string>> tanks = {
        {"id","fuel_id","capacity","level","notes"},
        {"T01","F01","10000","8000","Main Petrol"},
        {"T02","F02","12000","9000","Main Diesel"},
        {"T03","F06","6000","4800","Premium Tank"},
        {"T04","F13","8000","6500","CNG Storage"},
        {"T05","F03","5000","4200","Power Petrol"}
    };
    overwriteCSV(P_TANK_FILE, tanks);

    cout << "Petrol Pump sample data loaded (20 entries).\n";
}

/* ---------------------------------------------------------
                   PUMP MAIN MENU
--------------------------------------------------------- */

void pump_mainMenu() {
    while (true) {
        cout << "\n=== PETROL PUMP MANAGEMENT ===\n";
        cout << "1. Fuel Types\n";
        cout << "2. New Sale\n";
        cout << "3. View Sales\n";
        cout << "4. Total Sales of Day\n";
        cout << "5. Attendants\n";
        cout << "6. Expenses\n";
        cout << "7. Tanks\n";
        cout << "8. Load Sample Data\n";
        cout << "9. Back / Exit\n";
        cout << "Choice: ";

        string c; getline(cin, c);

        if (c == "1") {
            while (true) {
                cout << "\nFuel Menu\n1.Add Fuel\n2.View\n3.Update Price\n4.Update Stock\n5.Back\nChoice: ";
                string s; getline(cin, s);
                if (s == "1") pump_addFuelType();
                else if (s == "2") pump_viewFuelTypes();
                else if (s == "3") pump_updateFuelPrice();
                else if (s == "4") pump_updateStock();
                else break;
            }
        }
        else if (c == "2") pump_newSale();
        else if (c == "3") pump_viewSales();
        else if (c == "4") pump_totalSalesOfDay();
        else if (c == "5") {
            while (true) {
                cout << "\nAttendant Menu\n1.Add\n2.View\n3.Search\n4.Back\nChoice: ";
                string s; getline(cin, s);
                if (s == "1") pump_addAttendant();
                else if (s == "2") pump_viewAttendants();
                else if (s == "3") pump_findAttendant();
                else break;
            }
        }
        else if (c == "6") {
            while (true) {
                cout << "\nExpense Menu\n1.Add\n2.View\n3.Total\n4.Back\nChoice: ";
                string s; getline(cin, s);
                if (s == "1") pump_addExpense();
                else if (s == "2") pump_viewExpenses();
                else if (s == "3") pump_totalExpenses();
                else break;
            }
        }
        else if (c == "7") {
            while (true) {
                cout << "\nTank Menu\n1.Add Tank\n2.View Tanks\n3.Low Level Check\n4.Back\nChoice: ";
                string s; getline(cin, s);
                if (s == "1") pump_addTank();
                else if (s == "2") pump_viewTanks();
                else if (s == "3") pump_checkLowTanks();
                else break;
            }
        }
        else if (c == "8") pump_loadSampleData();
        else if (c == "9") break;
        else cout << "Invalid choice.\n";
    }
}

// Standalone main for this module
int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    pump_mainMenu();
    cout << "Exiting Petrol Pump module. Goodbye!\n";
    return 0;
}
