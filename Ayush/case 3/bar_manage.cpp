// bar_management.cpp
// Standalone Bar Management (helpers + bar module + main)
// Extracted from integrated_management.cpp. See source. :contentReference[oaicite:1]{index=1}

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
// BAR MANAGEMENT MODULE
// -----------------------------
const string BAR_MENU_FILE = "bar_menu.csv";
const string BAR_ORDERS_FILE = "bar_orders.csv";
const string BAR_SALES_FILE = "bar_sales.csv";

// ------------------- Add Drink Item -------------------
void bar_addDrink() {
    string id, name, price, alcohol, notes;
    cout << "Drink ID: "; getline(cin, id);
    cout << "Name: "; getline(cin, name);
    cout << "Price: "; getline(cin, price);
    cout << "Alcohol %: "; getline(cin, alcohol);
    cout << "Notes: "; getline(cin, notes);

    appendCSV(BAR_MENU_FILE, {id, name, price, alcohol, notes});
    cout << "Drink added.\n";
}

// ------------------- View Drink Menu -------------------
void bar_viewMenu() {
    auto rows = readCSV(BAR_MENU_FILE);
    if (rows.size() <= 1) {
        cout << "No drinks in menu.\n";
        return;
    }
    cout << "BAR MENU:\n";
    for (size_t i = 1; i < rows.size(); i++) {
        cout << rows[i][0] << " | " << rows[i][1]
             << " | Price:" << rows[i][2]
             << " | Alc:" << rows[i][3] << "%\n";
    }
}

// ------------------- Add Bar Order -------------------
void bar_addOrder() {
    string oid, drinkId, qty, custName, notes;
    cout << "Order ID: "; getline(cin, oid);
    cout << "Drink ID: "; getline(cin, drinkId);
    cout << "Qty: "; getline(cin, qty);
    cout << "Customer Name: "; getline(cin, custName);
    cout << "Notes: "; getline(cin, notes);

    auto menu = readCSV(BAR_MENU_FILE);
    double price = 0;
    bool found = false;

    for (size_t i = 1; i < menu.size(); i++) {
        if (menu[i][0] == drinkId) {
            price = atof(menu[i][2].c_str());
            found = true;
            break;
        }
    }

    if (!found) {
        cout << "Drink not found.\n";
        return;
    }

    int q = stoi(qty);
    double total = q * price;

    appendCSV(BAR_ORDERS_FILE,
              {oid, drinkId, qty, custName,
               to_string(total), nowDate(), nowTimeHHMM(), notes});

    cout << "Order added. Total = " << total << "\n";
}

// ------------------- View Orders -------------------
void bar_viewOrders() {
    auto rows = readCSV(BAR_ORDERS_FILE);
    if (rows.size() <= 1) {
        cout << "No bar orders.\n";
        return;
    }

    cout << "ORDERS:\n";
    for (size_t i = 1; i < rows.size(); i++) {
        cout << rows[i][0] << " | Drink:" << rows[i][1]
             << " | Qty:" << rows[i][2]
             << " | Cust:" << rows[i][3]
             << " | Total:" << rows[i][4]
             << " | " << rows[i][5] << " " << rows[i][6] << "\n";
    }
}

// ------------------- Generate Bill -------------------
void bar_generateBill() {
    cout << "Enter Order ID: ";
    string oid; getline(cin, oid);

    auto rows = readCSV(BAR_ORDERS_FILE);
    bool found = false;

    for (size_t i = 1; i < rows.size(); i++) {
        if (rows[i][0] == oid) {
            found = true;

            double total = atof(rows[i][4].c_str());
            double gst = total * 0.18;
            double finalBill = total + gst;

            appendCSV(BAR_SALES_FILE,
                      {oid, rows[i][3], rows[i][4],
                       to_string(gst), to_string(finalBill), nowDate()});

            cout << "\nBAR BILL\n";
            cout << "Order ID: " << oid << "\n";
            cout << "Customer: " << rows[i][3] << "\n";
            cout << "Amount: " << total << "\n";
            cout << "GST(18%): " << gst << "\n";
            cout << "Final Bill: " << finalBill << "\n";
            break;
        }
    }

    if (!found) cout << "Order not found.\n";
}

// ------------------- Sample Data Loader -------------------
void bar_loadSampleData() {

    vector<vector<string>> menu = {
        {"drink_id","name","price","alcohol","notes"},
        {"D001","Whiskey","150","40","Premium"},
        {"D002","Rum","120","42","Dark"},
        {"D003","Beer","90","5","Chilled"},
        {"D004","Vodka","140","38","Imported"},
        {"D005","Wine","180","12","Red Wine"}
    };
    overwriteCSV(BAR_MENU_FILE, menu);

    vector<vector<string>> orders = {
        {"order_id","drink_id","qty","customer","total","date","time","notes"},
        {"O001","D003","2","Amit","180","2025-01-20","19:20",""},
        {"O002","D001","1","Ravi","150","2025-01-21","20:10",""},
        {"O003","D005","1","Neha","180","2025-01-21","21:00",""}
    };
    overwriteCSV(BAR_ORDERS_FILE, orders);

    vector<vector<string>> sales = {
        {"order_id","customer","amount","gst","final","date"},
        {"O001","Amit","180","32.4","212.4","2025-01-20"},
        {"O002","Ravi","150","27","177","2025-01-21"}
    };
    overwriteCSV(BAR_SALES_FILE, sales);

    cout << "Bar sample data loaded.\n";
}

// ------------------- MAIN MENU -------------------
void bar_mainMenu() {
    while (true) {
        cout << "\n=== BAR MANAGEMENT ===\n";
        cout << "1. Add Drink Item\n";
        cout << "2. View Drink Menu\n";
        cout << "3. Add Bar Order\n";
        cout << "4. View Orders\n";
        cout << "5. Generate Bill\n";
        cout << "6. Load Sample Data\n";
        cout << "7. Back to MAIN MENU\n";
        cout << "Choice: ";

        string c; getline(cin, c);

        if (c == "1") bar_addDrink();
        else if (c == "2") bar_viewMenu();
        else if (c == "3") bar_addOrder();
        else if (c == "4") bar_viewOrders();
        else if (c == "5") bar_generateBill();
        else if (c == "6") bar_loadSampleData();
        else if (c == "7") break;
        else cout << "Invalid choice.\n";
    }
}

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);
    bar_mainMenu();
    return 0;
}
