// grocery_module.cpp — compact, array-based, load-only, limits preserved, no STL
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
using namespace std;

/* EXTERN HELPERS (must exist in mega-project) */
static int grocery_splitCSV(const string &line, string out[], int maxcols);
static int grocery_toInt(const string &s);
static double grocery_toDouble(const string &s);

/* LIMITS */
#define GROCERY_MAX_ITEMS 2000
#define GROCERY_MAX_TRANSACTIONS 2000
#define GROCERY_MAX_STAFF 2000
#define GROCERY_MAX_ATTENDANCE 2000
#define GROCERY_MAX_QUEUE 2000
#define GROCERY_HASH_SIZE 4096
int grocery_splitCSV(const string &line, string out[], int maxcols)
{
    int cnt = 0;
    string cur = "";
    for (char c : line)
    {
        if (c == ',' && cnt < maxcols - 1)
        {
            out[cnt++] = cur;
            cur = "";
        }
        else
        {
            cur += c;
        }
    }
    out[cnt++] = cur;
    return cnt;
}

int grocery_toInt(const string &s)
{
    return atoi(s.c_str());
}

double grocery_toDouble(const string &s)
{
    return atof(s.c_str());
}

/* STRUCTS */
struct groceryItem
{
    int item_id;
    string name;
    string category;
    double price;
    int stock_qty;
    int reorder_level;
    string supplier_name;
    int perishable;
    string expiry_date;
};

struct groceryStaff
{
    int staff_id;
    string name;
    string role;
    double salary;
    int is_active;
};

struct groceryTransaction
{
    int txn_id;
    string datetime;
    int item_id;
    int qty;
    double unit_price;
    double line_total;
    string cashier_name;
};

struct groceryAttendance
{
    int attendance_id;
    int staff_id;
    string date;
    string clock_in;
    string clock_out;
    double hours_worked;
};

/* GLOBALS */
groceryItem groceryItems[GROCERY_MAX_ITEMS];
int groceryItemCount = 0;

groceryStaff groceryStaffs[GROCERY_MAX_STAFF];
int groceryStaffCount = 0;

groceryTransaction groceryTransactions[GROCERY_MAX_TRANSACTIONS];
int groceryTransactionCount = 0;

groceryAttendance groceryAttendances[GROCERY_MAX_ATTENDANCE];
int groceryAttendanceCount = 0;

/* HASH TABLE (open addressing, linear probing) */
int groceryHashKeys[GROCERY_HASH_SIZE]; // 0 = empty (assumes no item has id 0)
int groceryHashIdx[GROCERY_HASH_SIZE];  // index in groceryItems or -1

void groceryInitHash()
{
    for (int i = 0; i < GROCERY_HASH_SIZE; ++i)
    {
        groceryHashKeys[i] = 0;
        groceryHashIdx[i] = -1;
    }
}

static inline int groceryHashFunc(int key)
{
    unsigned int k = (unsigned int)key;
    k ^= (k >> 16);
    k *= 0x7feb352dU;
    k ^= (k >> 15);
    return (int)(k & (GROCERY_HASH_SIZE - 1));
}

void groceryHashInsert(int key, int idx)
{
    int h = groceryHashFunc(key);
    for (int i = 0; i < GROCERY_HASH_SIZE; ++i)
    {
        int pos = (h + i) & (GROCERY_HASH_SIZE - 1);
        if (groceryHashIdx[pos] == -1 || groceryHashKeys[pos] == key)
        {
            groceryHashKeys[pos] = key;
            groceryHashIdx[pos] = idx;
            return;
        }
    }
}

int groceryHashFind(int key)
{
    int h = groceryHashFunc(key);
    for (int i = 0; i < GROCERY_HASH_SIZE; ++i)
    {
        int pos = (h + i) & (GROCERY_HASH_SIZE - 1);
        if (groceryHashIdx[pos] == -1)
            return -1;
        if (groceryHashKeys[pos] == key)
            return groceryHashIdx[pos];
    }
    return -1;
}

/* QUICK SORT (by stock_qty) - first-element pivot */
void swapItem(groceryItem &a, groceryItem &b)
{
    groceryItem t = a;
    a = b;
    b = t;
}

int partitionByStock(groceryItem A[], int l, int r)
{
    int p = A[l].stock_qty;
    int i = l, j = r + 1;
    while (1)
    {
        while (++i <= r && A[i].stock_qty < p)
        {
        }
        while (--j >= l && A[j].stock_qty > p)
        {
        }
        if (i >= j)
            break;
        swapItem(A[i], A[j]);
    }
    swapItem(A[l], A[j]);
    return j;
}

void quickSortByStock(groceryItem A[], int l, int r)
{
    if (l < r)
    {
        int s = partitionByStock(A, l, r);
        quickSortByStock(A, l, s - 1);
        quickSortByStock(A, s + 1, r);
    }
}

/* NAME SEARCH: use simple substring (string::find) to keep code small and reliable */
int groceryFindItemByName(const string &pattern)
{
    if (pattern.empty())
        return -1;
    for (int i = 0; i < groceryItemCount; ++i)
    {
        if (groceryItems[i].name.find(pattern) != string::npos)
            return i;
    }
    return -1;
}

/* CSV LOADERS (LOAD ONLY) */
/* Items CSV columns:
   item_id,name,category,price,stock_qty,reorder_level,supplier_name,perishable,expiry_date
*/
void groceryLoadItemsCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open())
    {
        cout << "Cannot open " << fn << "\n";
        return;
    }
    string line;
    getline(in, line); // skip header if present
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.empty())
            continue;
        string cols[9];
        int n = grocery_splitCSV(line, cols, 9);
        if (n < 2)
            continue;
        if (groceryItemCount >= GROCERY_MAX_ITEMS)
        {
            cout << "Items capacity reached\n";
            break;
        }
        groceryItem &it = groceryItems[groceryItemCount];
        it.item_id = grocery_toInt(cols[0]);
        it.name = cols[1];
        it.category = (n > 2) ? cols[2] : "";
        it.price = (n > 3) ? grocery_toDouble(cols[3]) : 0.0;
        it.stock_qty = (n > 4) ? grocery_toInt(cols[4]) : 0;
        it.reorder_level = (n > 5) ? grocery_toInt(cols[5]) : 0;
        it.supplier_name = (n > 6) ? cols[6] : "";
        it.perishable = (n > 7) ? grocery_toInt(cols[7]) : 0;
        it.expiry_date = (n > 8) ? cols[8] : "";
        if (it.item_id == 0 || it.name.empty())
            continue; // skip invalid
        groceryHashInsert(it.item_id, groceryItemCount);
        groceryItemCount++;
        loaded++;
    }
    cout << "Loaded " << loaded << " items from " << fn << "\n";
    in.close();
}

/* Staff CSV: name,role,salary  (we set staff_id sequentially) */
void groceryLoadStaffCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open())
    {
        cout << "Cannot open " << fn << "\n";
        return;
    }
    string line;
    getline(in, line);
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.empty())
            continue;
        string cols[3];
        int n = grocery_splitCSV(line, cols, 3);
        if (n < 1)
            continue;
        if (groceryStaffCount >= GROCERY_MAX_STAFF)
        {
            cout << "Staff capacity reached\n";
            break;
        }
        groceryStaff &s = groceryStaffs[groceryStaffCount];
        s.staff_id = groceryStaffCount + 1;
        s.name = cols[0];
        s.role = (n > 1) ? cols[1] : "";
        s.salary = (n > 2) ? grocery_toDouble(cols[2]) : 0.0;
        s.is_active = 1;
        if (s.name.empty())
            continue;
        groceryStaffCount++;
        loaded++;
    }
    cout << "Loaded " << loaded << " staff from " << fn << "\n";
    in.close();
}

/* Transactions CSV:
   txn_id,datetime,item_id,qty,unit_price,line_total,cashier_name
*/
void groceryLoadTransactionsCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open())
    {
        cout << "Cannot open " << fn << "\n";
        return;
    }
    string line;
    getline(in, line);
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.empty())
            continue;
        string cols[7];
        int n = grocery_splitCSV(line, cols, 7);
        if (n < 2)
            continue;
        if (groceryTransactionCount >= GROCERY_MAX_TRANSACTIONS)
        {
            cout << "Transactions capacity reached\n";
            break;
        }
        groceryTransaction &t = groceryTransactions[groceryTransactionCount];
        t.txn_id = grocery_toInt(cols[0]);
        t.datetime = (n > 1) ? cols[1] : "";
        t.item_id = (n > 2) ? grocery_toInt(cols[2]) : 0;
        t.qty = (n > 3) ? grocery_toInt(cols[3]) : 0;
        t.unit_price = (n > 4) ? grocery_toDouble(cols[4]) : 0.0;
        t.line_total = (n > 5) ? grocery_toDouble(cols[5]) : (t.qty * t.unit_price);
        t.cashier_name = (n > 6) ? cols[6] : "";
        if (t.txn_id == 0)
            continue;
        groceryTransactionCount++;
        loaded++;
        // deduct stock if item exists
        int idx = groceryHashFind(t.item_id);
        if (idx != -1)
        {
            groceryItems[idx].stock_qty -= t.qty;
            if (groceryItems[idx].stock_qty < 0)
                groceryItems[idx].stock_qty = 0;
        }
    }
    cout << "Loaded " << loaded << " transactions from " << fn << "\n";
    in.close();
}

/* Attendance CSV:
   attendance_id,staff_id,date,clock_in,clock_out,hours_worked
*/
void groceryLoadAttendanceCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open())
    {
        cout << "Cannot open " << fn << "\n";
        return;
    }
    string line;
    getline(in, line);
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.empty())
            continue;
        string cols[6];
        int n = grocery_splitCSV(line, cols, 6);
        if (n < 2)
            continue;
        if (groceryAttendanceCount >= GROCERY_MAX_ATTENDANCE)
        {
            cout << "Attendance capacity reached\n";
            break;
        }
        groceryAttendance &a = groceryAttendances[groceryAttendanceCount];
        a.attendance_id = grocery_toInt(cols[0]);
        a.staff_id = (n > 1) ? grocery_toInt(cols[1]) : 0;
        a.date = (n > 2) ? cols[2] : "";
        a.clock_in = (n > 3) ? cols[3] : "";
        a.clock_out = (n > 4) ? cols[4] : "";
        a.hours_worked = (n > 5) ? grocery_toDouble(cols[5]) : 0.0;
        if (a.attendance_id == 0)
            continue;
        groceryAttendanceCount++;
        loaded++;
    }
    cout << "Loaded " << loaded << " attendance rows from " << fn << "\n";
    in.close();
}

/* MANUAL OPERATIONS */
void groceryAddItemManual()
{
    if (groceryItemCount >= GROCERY_MAX_ITEMS)
    {
        cout << "Overflow\n";
        return;
    }
    groceryItem &it = groceryItems[groceryItemCount];
    cout << "Enter item_id: ";
    cin >> it.item_id;
    cin.ignore();
    cout << "Enter name: ";
    getline(cin, it.name);
    cout << "Enter category: ";
    getline(cin, it.category);
    cout << "Enter price: ";
    cin >> it.price;
    cin.ignore();
    cout << "Enter stock_qty: ";
    cin >> it.stock_qty;
    cin.ignore();
    cout << "Enter reorder_level: ";
    cin >> it.reorder_level;
    cin.ignore();
    cout << "Enter supplier_name: ";
    getline(cin, it.supplier_name);
    cout << "Perishable? (1/0): ";
    cin >> it.perishable;
    cin.ignore();
    cout << "Expiry date (YYYY-MM-DD or empty): ";
    getline(cin, it.expiry_date);
    if (it.item_id == 0 || it.name.empty())
    {
        cout << "Invalid item, not added.\n";
        return;
    }
    groceryHashInsert(it.item_id, groceryItemCount);
    groceryItemCount++;
    cout << "Item added.\n";
}

void groceryRestockItem()
{
    cout << "Enter item id OR name: ";
    string key;
    getline(cin, key);
    int idx = grocery_toInt(key) ? groceryHashFind(grocery_toInt(key)) : groceryFindItemByName(key);
    if (idx == -1)
    {
        cout << "Not found.\n";
        return;
    }
    int add;
    cout << "Qty to add: ";
    cin >> add;
    cin.ignore();
    if (add > 0)
        groceryItems[idx].stock_qty += add;
    cout << "New stock: " << groceryItems[idx].stock_qty << "\n";
}

void groceryRemoveItem()
{
    cout << "Enter item id to remove: ";
    int id;
    cin >> id;
    cin.ignore();
    int idx = groceryHashFind(id);
    if (idx == -1)
    {
        cout << "Not found.\n";
        return;
    }
    for (int i = idx + 1; i < groceryItemCount; ++i)
        groceryItems[i - 1] = groceryItems[i];
    groceryItemCount--;
    groceryInitHash();
    for (int i = 0; i < groceryItemCount; ++i)
        groceryHashInsert(groceryItems[i].item_id, i);
    cout << "Removed.\n";
}

void groceryListItems()
{
    cout << "Items (" << groceryItemCount << "):\n";
    for (int i = 0; i < groceryItemCount; ++i)
    {
        groceryItem &it = groceryItems[i];
        cout << it.item_id << "," << it.name << "," << it.category << "," << it.price << "," << it.stock_qty << "\n";
    }
}

void grocerySearchItemInteractive()
{
    cout << "Enter substring: ";
    string pat;
    getline(cin, pat);
    cout << "Matches:\n";
    for (int i = 0; i < groceryItemCount; ++i)
        if (groceryItems[i].name.find(pat) != string::npos)
            cout << groceryItems[i].item_id << "," << groceryItems[i].name << "," << groceryItems[i].stock_qty << "," << groceryItems[i].price << "\n";
}

void groceryLowStockReport()
{
    cout << "Low stock (<= reorder):\n";
    for (int i = 0; i < groceryItemCount; ++i)
    {
        groceryItem &it = groceryItems[i];
        if (it.stock_qty <= it.reorder_level)
            cout << it.item_id << "," << it.name << "," << it.stock_qty << "," << it.reorder_level << "\n";
    }
}

/* STAFF */
void groceryAddStaffManual()
{
    if (groceryStaffCount >= GROCERY_MAX_STAFF)
    {
        cout << "Overflow\n";
        return;
    }
    groceryStaff &s = groceryStaffs[groceryStaffCount];
    s.staff_id = groceryStaffCount + 1;
    cout << "Enter name: ";
    getline(cin, s.name);
    cout << "Enter role: ";
    getline(cin, s.role);
    cout << "Enter salary: ";
    cin >> s.salary;
    cin.ignore();
    s.is_active = 1;
    if (s.name.empty())
    {
        cout << "Invalid.\n";
        return;
    }
    groceryStaffCount++;
    cout << "Staff added id=" << s.staff_id << "\n";
}

void groceryFireStaff()
{
    cout << "Enter staff_id: ";
    int id;
    cin >> id;
    cin.ignore();
    for (int i = 0; i < groceryStaffCount; ++i)
        if (groceryStaffs[i].staff_id == id)
        {
            groceryStaffs[i].is_active = 0;
            cout << "Marked inactive\n";
            return;
        }
    cout << "Not found\n";
}

void groceryListStaff()
{
    cout << "Staff (" << groceryStaffCount << "):\n";
    for (int i = 0; i < groceryStaffCount; ++i)
    {
        groceryStaff &s = groceryStaffs[i];
        cout << s.staff_id << "," << s.name << "," << s.role << "," << s.salary << "," << s.is_active << "\n";
    }
}

void grocerySalaryReport()
{
    double tot = 0.0;
    for (int i = 0; i < groceryStaffCount; ++i)
        tot += groceryStaffs[i].salary;
    cout << "Total payroll = " << tot << "\n";
}

/* ATTENDANCE */
void groceryAddAttendanceManual()
{
    if (groceryAttendanceCount >= GROCERY_MAX_ATTENDANCE)
    {
        cout << "Overflow\n";
        return;
    }
    groceryAttendance &a = groceryAttendances[groceryAttendanceCount];
    cout << "Enter attendance_id: ";
    cin >> a.attendance_id;
    cout << "Enter staff_id: ";
    cin >> a.staff_id;
    cin.ignore();
    cout << "Date (YYYY-MM-DD): ";
    getline(cin, a.date);
    cout << "Clock in (HH:MM): ";
    getline(cin, a.clock_in);
    cout << "Clock out (HH:MM or empty): ";
    getline(cin, a.clock_out);
    if (a.clock_out.empty())
        a.hours_worked = 0.0;
    else
    {
        int h1 = 0, m1 = 0, h2 = 0, m2 = 0;
        sscanf(a.clock_in.c_str(), "%d:%d", &h1, &m1);
        sscanf(a.clock_out.c_str(), "%d:%d", &h2, &m2);
        double hrs = (h2 + m2 / 60.0) - (h1 + m1 / 60.0);
        a.hours_worked = hrs < 0 ? 0.0 : hrs;
    }
    groceryAttendanceCount++;
    cout << "Attendance added.\n";
}

void groceryClockIn()
{
    if (groceryAttendanceCount >= GROCERY_MAX_ATTENDANCE)
    {
        cout << "Overflow\n";
        return;
    }
    groceryAttendance &a = groceryAttendances[groceryAttendanceCount++];
    a.attendance_id = groceryAttendanceCount;
    cout << "Staff_id: ";
    cin >> a.staff_id;
    cin.ignore();
    cout << "Date: ";
    getline(cin, a.date);
    cout << "Clock in: ";
    getline(cin, a.clock_in);
    a.clock_out = "";
    a.hours_worked = 0.0;
    cout << "Clock-in recorded.\n";
}

void groceryClockOut()
{
    cout << "Staff_id: ";
    int sid;
    cin >> sid;
    cin.ignore();
    cout << "Date: ";
    string date;
    getline(cin, date);
    cout << "Clock out (HH:MM): ";
    string co;
    getline(cin, co);
    for (int i = groceryAttendanceCount - 1; i >= 0; --i)
    {
        groceryAttendance &a = groceryAttendances[i];
        if (a.staff_id == sid && a.date == date && a.clock_out.empty())
        {
            a.clock_out = co;
            int h1 = 0, m1 = 0, h2 = 0, m2 = 0;
            sscanf(a.clock_in.c_str(), "%d:%d", &h1, &m1);
            sscanf(co.c_str(), "%d:%d", &h2, &m2);
            double hrs = (h2 + m2 / 60.0) - (h1 + m1 / 60.0);
            a.hours_worked = hrs < 0 ? 0.0 : hrs;
            cout << "Clock-out recorded. Hours = " << a.hours_worked << "\n";
            return;
        }
    }
    cout << "No matching clock-in found.\n";
}

void groceryViewAttendance()
{
    cout << "Attendance (" << groceryAttendanceCount << "):\n";
    for (int i = 0; i < groceryAttendanceCount; ++i)
    {
        groceryAttendance &a = groceryAttendances[i];
        cout << a.attendance_id << "," << a.staff_id << "," << a.date << "," << a.clock_in << "," << a.clock_out << "," << a.hours_worked << "\n";
    }
}

/* TRANSACTIONS / POS (cart-mode) */
void groceryAddTransactionManual()
{
    if (groceryTransactionCount >= GROCERY_MAX_TRANSACTIONS)
    {
        cout << "Overflow\n";
        return;
    }
    groceryTransaction &t = groceryTransactions[groceryTransactionCount];
    cout << "txn_id: ";
    cin >> t.txn_id;
    cin.ignore();
    cout << "datetime: ";
    getline(cin, t.datetime);
    cout << "item_id: ";
    cin >> t.item_id;
    cin.ignore();
    cout << "qty: ";
    cin >> t.qty;
    cin.ignore();
    cout << "unit_price: ";
    cin >> t.unit_price;
    cin.ignore();
    t.line_total = t.qty * t.unit_price;
    cout << "cashier_name: ";
    getline(cin, t.cashier_name);
    groceryTransactionCount++;
    int idx = groceryHashFind(t.item_id);
    if (idx != -1)
    {
        groceryItems[idx].stock_qty -= t.qty;
        if (groceryItems[idx].stock_qty < 0)
            groceryItems[idx].stock_qty = 0;
    }
    cout << "Transaction added.\n";
}

void groceryStartSale()
{
    cout << "Enter txn_id: ";
    int txn_base;
    cin >> txn_base;
    cin.ignore();
    cout << "datetime: ";
    string datetime;
    getline(cin, datetime);
    cout << "cashier: ";
    string cashier;
    getline(cin, cashier);
    cout << "distinct items in cart: ";
    int n;
    cin >> n;
    cin.ignore();
    if (n <= 0)
    {
        cout << "No items.\n";
        return;
    }
    double sale_total = 0.0;
    for (int i = 0; i < n; ++i)
    {
        if (groceryTransactionCount >= GROCERY_MAX_TRANSACTIONS)
        {
            cout << "Transactions full\n";
            break;
        }
        cout << "Item " << (i + 1) << " id OR name: ";
        string ident;
        getline(cin, ident);
        int idx = grocery_toInt(ident) ? groceryHashFind(grocery_toInt(ident)) : groceryFindItemByName(ident);
        if (idx == -1)
        {
            cout << "Not found. Skip.\n";
            continue;
        }
        groceryItem &it = groceryItems[idx];
        cout << "Found: " << it.item_id << "," << it.name << "," << it.price << ", stock=" << it.stock_qty << "\n";
        cout << "qty: ";
        int q;
        cin >> q;
        cin.ignore();
        if (q <= 0)
        {
            cout << "Skip.\n";
            continue;
        }
        if (it.stock_qty < q)
        {
            cout << "Insufficient stock avail=" << it.stock_qty << ". Proceed? (1=yes): ";
            int p;
            cin >> p;
            cin.ignore();
            if (!p)
                continue;
        }
        groceryTransaction gt;
        gt.txn_id = txn_base;
        gt.datetime = datetime;
        gt.item_id = it.item_id;
        gt.qty = q;
        gt.unit_price = it.price;
        gt.line_total = q * it.price;
        gt.cashier_name = cashier;
        if (groceryTransactionCount < GROCERY_MAX_TRANSACTIONS)
            groceryTransactions[groceryTransactionCount++] = gt;
        it.stock_qty -= q;
        if (it.stock_qty < 0)
            it.stock_qty = 0;
        sale_total += gt.line_total;
        cout << "Added: " << gt.item_id << "," << gt.qty << "," << gt.line_total << "\n";
    }
    cout << "Sale complete. Total=" << sale_total << "\n";
}

void grocerySimulateQueue()
{
    static int q[GROCERY_MAX_QUEUE], front = 0, back = 0;
    while (1)
    {
        cout << "Queue:1=enq 2=deq 3=show 0=exit: ";
        int c;
        cin >> c;
        cin.ignore();
        if (c == 0)
            break;
        if (c == 1)
        {
            int tid;
            cout << "txn_id: ";
            cin >> tid;
            cin.ignore();
            if ((back - front) >= GROCERY_MAX_QUEUE)
            {
                cout << "Overflow\n";
            }
            else
            {
                q[back++ % GROCERY_MAX_QUEUE] = tid;
                cout << "Enqueued\n";
            }
        }
        else if (c == 2)
        {
            if (front == back)
                cout << "Empty\n";
            else
            {
                int tid = q[front++ % GROCERY_MAX_QUEUE];
                cout << "Processing " << tid << "\n";
                for (int i = 0; i < groceryTransactionCount; ++i)
                    if (groceryTransactions[i].txn_id == tid)
                        cout << "Line: " << groceryTransactions[i].item_id << "," << groceryTransactions[i].qty << "," << groceryTransactions[i].line_total << "\n";
            }
        }
        else if (c == 3)
        {
            cout << "Queue:";
            for (int i = front; i < back; ++i)
                cout << q[i % GROCERY_MAX_QUEUE] << " ";
            cout << "\n";
        }
        else
            cout << "Invalid\n";
    }
}

void groceryViewTransactions()
{
    cout << "Transactions (" << groceryTransactionCount << "):\n";
    for (int i = 0; i < groceryTransactionCount; ++i)
    {
        groceryTransaction &t = groceryTransactions[i];
        cout << t.txn_id << "," << t.datetime << "," << t.item_id << "," << t.qty << "," << t.unit_price << "," << t.line_total << "," << t.cashier_name << "\n";
    }
}

void groceryItemSalesReport()
{
    cout << "Item sales (id,name,total_qty,total_revenue):\n";
    for (int i = 0; i < groceryItemCount; ++i)
    {
        int id = groceryItems[i].item_id;
        int totq = 0;
        double totr = 0.0;
        for (int j = 0; j < groceryTransactionCount; ++j)
            if (groceryTransactions[j].item_id == id)
            {
                totq += groceryTransactions[j].qty;
                totr += groceryTransactions[j].line_total;
            }
        if (totq > 0)
            cout << id << "," << groceryItems[i].name << "," << totq << "," << totr << "\n";
    }
}

void groceryDailySales(const string &datePrefix)
{
    double total = 0.0;
    for (int i = 0; i < groceryTransactionCount; ++i)
        if (groceryTransactions[i].datetime.rfind(datePrefix, 0) == 0)
            total += groceryTransactions[i].line_total;
    cout << "Sales " << datePrefix << " = " << total << "\n";
}

void groceryMonthlySales(const string &monthPrefix)
{
    double total = 0.0;
    for (int i = 0; i < groceryTransactionCount; ++i)
        if (groceryTransactions[i].datetime.rfind(monthPrefix, 0) == 0)
            total += groceryTransactions[i].line_total;
    cout << "Monthly " << monthPrefix << " = " << total << "\n";
}

void groceryProfitReport(const string &datePrefix)
{
    double revenue = 0.0, cogs = 0.0;
    for (int i = 0; i < groceryTransactionCount; ++i)
    {
        groceryTransaction &t = groceryTransactions[i];
        if (datePrefix.empty() || t.datetime.rfind(datePrefix, 0) == 0)
        {
            revenue += t.line_total;
            cogs += t.line_total * 0.7;
        }
    }
    cout << "Profit (" << (datePrefix.empty() ? "ALL" : datePrefix) << ") rev=" << revenue << " cogs=" << cogs << " profit=" << revenue - cogs << "\n";
}

/* LOAD ALL FROM FOLDER */
void groceryLoadAllCSVsFromFolder()
{
    groceryLoadItemsCSV("groceryitems.csv");
    groceryLoadStaffCSV("grocerystaff.csv");
    groceryLoadTransactionsCSV("grocerytransactions.csv");
    groceryLoadAttendanceCSV("groceryattendance.csv");
}

/* MAIN MENU */
void grocerySystem()
{
    groceryInitHash();

    while (true)
    {
        cout << "\n--- Grocery Store Menu ---\n";
        cout << "1. Inventory\n";
        cout << "2. Sales / POS\n";
        cout << "3. Staff\n";
        cout << "4. Scheduling & Attendance\n";
        cout << "5. Reports\n";
        cout << "6. CSV Operations\n";
        cout << "0. Return\n";
        cout << "Choice: ";

        int c;
        cin >> c;
        cin.ignore();

        switch (c)
        {

        /* ---------------- INVENTORY ---------------- */
        case 1:
        {
            cout << "1:LoadCSV 2:AddItemManual 3:Restock 4:Remove "
                    "5:ListItems 6:SearchItem 7:LowStockReport 8:SortByStock\nChoice: ";
            int s;
            cin >> s;
            cin.ignore();

            switch (s)
            {
            case 1:
            {
                groceryLoadItemsCSV("groceryitems.csv");
                break;
            }
            case 2:
                groceryAddItemManual();
                break;
            case 3:
                groceryRestockItem();
                break;
            case 4:
                groceryRemoveItem();
                break;
            case 5:
                groceryListItems();
                break;
            case 6:
                grocerySearchItemInteractive();
                break;
            case 7:
                groceryLowStockReport();
                break;
            case 8:
                if (groceryItemCount > 0)
                    quickSortByStock(groceryItems, 0, groceryItemCount - 1);
                cout << "Sorted by stock.\n";
                break;
            default:
                cout << "Invalid\n";
            }
            break;
        }

        /* ---------------- SALES / POS ---------------- */
        case 2:
        {
            cout << "1:LoadTransactionsCSV 2:AddTransactionManual 3:StartSale "
                    "4:SimulateQueue 5:ViewTransactions 6:ItemSalesReport\nChoice: ";
            int s;
            cin >> s;
            cin.ignore();

            switch (s)
            {
            case 1:
            {
                groceryLoadTransactionsCSV("grocerytransactions.csv");
                break;
            }
            case 2:
                groceryAddTransactionManual();
                break;
            case 3:
                groceryStartSale();
                break;
            case 4:
                grocerySimulateQueue();
                break;
            case 5:
                groceryViewTransactions();
                break;
            case 6:
                groceryItemSalesReport();
                break;
            default:
                cout << "Invalid\n";
            }
            break;
        }

        /* ---------------- STAFF ---------------- */
        case 3:
        {
            cout << "1:LoadStaffCSV 2:AddStaffManual 3:FireStaff "
                    "4:ListStaff 5:SalaryReport\nChoice: ";
            int s;
            cin >> s;
            cin.ignore();

            switch (s)
            {
            case 1:
            {
                groceryLoadStaffCSV("grocerystaff.csv");
                break;
            }
            case 2:
                groceryAddStaffManual();
                break;
            case 3:
                groceryFireStaff();
                break;
            case 4:
                groceryListStaff();
                break;
            case 5:
                grocerySalaryReport();
                break;
            default:
                cout << "Invalid\n";
            }
            break;
        }

        /* ---------------- ATTENDANCE ---------------- */
        case 4:
        {
            cout << "1:LoadAttendanceCSV 2:AddAttendanceManual 3:ClockIn "
                    "4:ClockOut 5:ViewAttendance 6:GenerateSchedule\nChoice: ";
            int s;
            cin >> s;
            cin.ignore();

            switch (s)
            {
            case 1:
            {
                groceryLoadAttendanceCSV("groceryattendance.csv");
                break;
            }
            case 2:
                groceryAddAttendanceManual();
                break;
            case 3:
                groceryClockIn();
                break;
            case 4:
                groceryClockOut();
                break;
            case 5:
                groceryViewAttendance();
                break;
            case 6:
                cout << "Greedy schedule not implemented yet.\n";
                break;
            default:
                cout << "Invalid\n";
            }

            break;
        }

        /* ---------------- REPORTS ---------------- */
        case 5:
        {
            cout << "1:DailySales 2:MonthlySales 3:ProfitReport 4:LowStockAnalysis\nChoice: ";
            int s;
            cin >> s;
            cin.ignore();

            switch (s)
            {
            case 1:
            {
                string d;
                cout << "Date (YYYY-MM-DD): ";
                getline(cin, d);
                groceryDailySales(d);
                break;
            }
            case 2:
            {
                string m;
                cout << "Month (YYYY-MM): ";
                getline(cin, m);
                groceryMonthlySales(m);
                break;
            }
            case 3:
            {
                string p;
                cout << "Date prefix (or empty): ";
                getline(cin, p);
                groceryProfitReport(p);
                break;
            }
            case 4:
                groceryLowStockReport();
                break;
            default:
                cout << "Invalid\n";
            }
            break;
        }

        /* ---------------- CSV OPERATIONS ---------------- */
        case 6:
        {
            cout << "1:LoadAllCSVs\nChoice: ";
            int s;
            cin >> s;
            cin.ignore();
            switch (s)
            {
            case 1:
                groceryLoadAllCSVsFromFolder();
                break;
            default:
                cout << "Invalid\n";
            }
            break;
        }

        /* EXIT */
        case 0:
            return;

        /* INVALID OPTION */
        default:
            cout << "Invalid choice.\n";
        }
    }
} // end groceryMainMenu
int main()
{
    grocerySystem();
    return 0;
}
