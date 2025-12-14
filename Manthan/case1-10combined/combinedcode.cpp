#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
using namespace std;

// =====================================================
// MALL LIMITS
// =====================================================

#define MALL_MAX_SHOPS 2000
#define MALL_MAX_ITEMS 5000
#define MALL_MAX_STAFF 2000
#define MALL_MAX_CLEAN 3000
#define MALL_PARKING_SIZE 200

// =====================================================
// UTILITY
// =====================================================

int randint(int a, int b) { return a + rand() % (b - a + 1); }

int timeToMin(string t) {
    int hh = stoi(t.substr(0,2));
    int mm = stoi(t.substr(3,2));
    return hh*60 + mm;
}

string minToTime(int m) {
    int hh = m/60;
    int mm = m%60;
    char buf[16];
    sprintf(buf, "%02d:%02d", hh, mm);
    return string(buf);
}

// =====================================================
// SHOP STRUCTURES
// =====================================================

struct ItemNode {
    string name;
    int price;
    int stock;
    ItemNode *left;
    ItemNode *right;
};

struct Shop {
    int id;
    string name;
    int open_time;
    int close_time;
    int revenue;
    bool isOpen;
    ItemNode *root;
};

Shop mallShops[MALL_MAX_SHOPS];
int shopCount = 0;

// =====================================================
// STAFF SYSTEM
// =====================================================

struct Staff {
    int id;
    string name;
    string role;
    int salary;
};

Staff staffList[MALL_MAX_STAFF];
int staffCount = 0;

// =====================================================
// CLEANING LOG
// =====================================================

struct CleanEvent {
    int time;
    string note;
};

CleanEvent cleanLog[MALL_MAX_CLEAN];
int cleanCount = 0;

// =====================================================
// PARKING GRID (BFS)
// =====================================================

int parkingGrid[MALL_PARKING_SIZE][MALL_PARKING_SIZE];

// =====================================================
// BST SYSTEM
// =====================================================

class ItemBST {
public:
    ItemNode* insertNode(ItemNode* root, string name, int price, int stock) {
        ItemNode* newnode = new ItemNode;
        newnode->name = name;
        newnode->price = price;
        newnode->stock = stock;
        newnode->left = newnode->right = NULL;

        if (!root) return newnode;

        ItemNode* parent = NULL;
        ItemNode* curr = root;
        while (curr) {
            parent = curr;
            if (name < curr->name) curr = curr->left;
            else curr = curr->right;
        }

        if (name < parent->name) parent->left = newnode;
        else parent->right = newnode;

        return root;
    }

    void inorder(ItemNode* root) {
        if (!root) return;
        inorder(root->left);
        cout << "   " << root->name
             << " | Price: " << root->price
             << " | Stock: " << root->stock << "\n";
        inorder(root->right);
    }
};

ItemBST itemManager;

// =====================================================
// QUICKSORT
// =====================================================

int partitionQuick(int arr[], int l, int r) {
    int pivot = mallShops[arr[l]].revenue;
    int i = l, j = r + 1;

    while (true) {
        do { i++; } while (i <= r && mallShops[arr[i]].revenue > pivot);
        do { j--; } while (mallShops[arr[j]].revenue < pivot);
        if (i >= j) break;
        swap(arr[i], arr[j]);
    }
    swap(arr[l], arr[j]);
    return j;
}

void quickSortShops(int arr[], int l, int r) {
    if (l < r) {
        int s = partitionQuick(arr, l, r);
        quickSortShops(arr, l, s - 1);
        quickSortShops(arr, s + 1, r);
    }
}

// =====================================================
// BFS PARKING
// =====================================================

int BFS_nearestParking(int grid[MALL_PARKING_SIZE][MALL_PARKING_SIZE], int sr, int sc) {
    int qr[MALL_PARKING_SIZE * MALL_PARKING_SIZE];
    int qc[MALL_PARKING_SIZE * MALL_PARKING_SIZE];
    int visited[MALL_PARKING_SIZE][MALL_PARKING_SIZE] = {0};

    int front = 0, rear = 0;
    qr[0] = sr; qc[0] = sc;
    visited[sr][sc] = 1;

    int dr[4] = {-1,1,0,0};
    int dc[4] = {0,0,-1,1};

    while (front <= rear) {
        int r = qr[front];
        int c = qc[front++];
        if (grid[r][c] == 0)
            return r * MALL_PARKING_SIZE + c;

        for (int i = 0; i < 4; i++) {
            int nr = r + dr[i], nc = c + dc[i];
            if (nr >= 0 && nr < MALL_PARKING_SIZE &&
                nc >= 0 && nc < MALL_PARKING_SIZE &&
                !visited[nr][nc] && grid[nr][nc] != 2) {
                visited[nr][nc] = 1;
                qr[++rear] = nr;
                qc[rear] = nc;
            }
        }
    }
    return -1;
}

// =====================================================
// CORE SHOP FUNCTIONS
// =====================================================

int createShopID() {
    static int sid = 100;
    return ++sid;
}

void addShop() {
    if (shopCount >= MALL_MAX_SHOPS) {
        cout << "Shop limit reached.\n";
        return;
    }

    cin.ignore();
    string nm, op, cl;

    cout << "Enter shop name: ";
    getline(cin, nm);
    cout << "Open time (HH:MM): ";
    cin >> op;
    cout << "Close time (HH:MM): ";
    cin >> cl;

    mallShops[shopCount].id = createShopID();
    mallShops[shopCount].name = nm;
    mallShops[shopCount].open_time = timeToMin(op);
    mallShops[shopCount].close_time = timeToMin(cl);
    mallShops[shopCount].revenue = 0;
    mallShops[shopCount].root = NULL;

    cout << "Shop added (ID = " << mallShops[shopCount].id << ")\n";
    shopCount++;
}

void addItemToShop() {
    int id;
    cout << "Enter shop ID: ";
    cin >> id;

    int idx = -1;
    for (int i = 0; i < shopCount; i++)
        if (mallShops[i].id == id) idx = i;

    if (idx == -1) { cout << "Shop not found.\n"; return; }

    cin.ignore();
    string name;
    int price, stock;

    cout << "Item name: ";
    getline(cin, name);
    cout << "Price: ";
    cin >> price;
    cout << "Stock: ";
    cin >> stock;

    mallShops[idx].root = itemManager.insertNode(
        mallShops[idx].root, name, price, stock
    );

    cout << "Item added!\n";
}

void showAllShops() {
    if (shopCount == 0) { cout << "No shops.\n"; return; }

    for (int i = 0; i < shopCount; i++) {
        cout << "\n----------- SHOP " << mallShops[i].id << " -----------\n";
        cout << "Name: " << mallShops[i].name << "\n";
        cout << "Time: " << minToTime(mallShops[i].open_time)
             << " - " << minToTime(mallShops[i].close_time) << "\n";
        cout << "Revenue: " << mallShops[i].revenue << "\n";
        cout << "Items:\n";
        if (!mallShops[i].root) cout << "  (empty)\n";
        else itemManager.inorder(mallShops[i].root);
    }
}

void simulateCustomer() {
    int id;
    cout << "Enter shop ID: ";
    cin >> id;

    int idx = -1;
    for (int i = 0; i < shopCount; i++)
        if (mallShops[i].id == id) idx = i;

    if (idx == -1 || !mallShops[idx].root) {
        cout << "Invalid shop or no items.\n";
        return;
    }

    ItemNode *cur = mallShops[idx].root;
    while (cur->left) cur = cur->left;

    if (cur->stock <= 0) { cout << "Out of stock.\n"; return; }

    int qty = randint(1,3);
    qty = min(qty, cur->stock);

    cur->stock -= qty;
    mallShops[idx].revenue += qty * cur->price;

    cout << "Sold " << qty << " of " << cur->name << "\n";
}

void sortShopsByRevenue() {
    if (shopCount == 0) { cout << "No shops.\n"; return; }

    int arr[MALL_MAX_SHOPS];
    for (int i = 0; i < shopCount; i++) arr[i] = i;

    quickSortShops(arr, 0, shopCount - 1);

    cout << "\n--- Shops sorted (high → low revenue) ---\n";
    for (int i = 0; i < shopCount; i++) {
        cout << mallShops[arr[i]].name
             << " | Rs " << mallShops[arr[i]].revenue << "\n";
    }
}

// =====================================================
// STAFF + CLEANER
// =====================================================

void addStaff() {
    if (staffCount >= MALL_MAX_STAFF) {
        cout << "Staff limit reached.\n";
        return;
    }

    cin.ignore();
    cout << "Staff name: ";
    getline(cin, staffList[staffCount].name);
    cout << "Role: ";
    getline(cin, staffList[staffCount].role);
    cout << "Salary: ";
    cin >> staffList[staffCount].salary;

    staffList[staffCount].id = staffCount + 1;
    staffCount++;
    cout << "Staff added.\n";
}

void showStaff() {
    for (int i = 0; i < staffCount; i++) {
        cout << staffList[i].id << ") "
             << staffList[i].name << " - "
             << staffList[i].role << " - Rs "
             << staffList[i].salary << "\n";
    }
}

void deleteStaff() {
    cout << "Enter staff ID to delete: ";
    int id; cin >> id;

    int idx = -1;
    for (int i = 0; i < staffCount; i++)
        if (staffList[i].id == id) idx = i;

    if (idx == -1) { cout << "Staff not found.\n"; return; }

    for (int i = idx; i < staffCount - 1; i++)
        staffList[i] = staffList[i + 1];

    staffCount--;
    cout << "Staff removed.\n";
}

void scheduleCleaner() {
    if (cleanCount >= MALL_MAX_CLEAN) {
        cout << "Cleaner log full.\n";
        return;
    }

    cin.ignore();
    string t, note;
    cout << "Time (HH:MM): ";
    cin >> t;
    cin.ignore();
    cout << "Note: ";
    getline(cin, note);

    cleanLog[cleanCount].time = timeToMin(t);
    cleanLog[cleanCount].note = note;
    cleanCount++;
}

void showCleanLog() {
    for (int i = 0; i < cleanCount; i++)
        cout << minToTime(cleanLog[i].time)
             << " - " << cleanLog[i].note << "\n";
}

// =====================================================
// CSV HELPERS
// =====================================================

int toInt(const string &s) {
    try { return stoi(s); }
    catch (...) { return 0; }
}

int splitCSV(const string &line, string out[], int maxCols) {
    int col = 0; string cur;
    for (char c : line) {
        if (c == ',' && col < maxCols - 1) {
            out[col++] = cur;
            cur.clear();
        } else cur.push_back(c);
    }
    out[col++] = cur;
    return col;
}

int loadShopsCSV(const string &f) {
    ifstream in(f);
    if (!in.is_open()) { cout << "ERROR loading " << f << "\n"; return 0; }

    string line; getline(in, line);
    int loaded = 0;

    while (getline(in, line)) {
        if (shopCount >= MALL_MAX_SHOPS) break;
        string c[5];
        if (splitCSV(line, c, 5) < 4) continue;

        mallShops[shopCount].id = createShopID();
        mallShops[shopCount].name = c[0];
        mallShops[shopCount].open_time = timeToMin(c[1]);
        mallShops[shopCount].close_time = timeToMin(c[2]);
        mallShops[shopCount].revenue = toInt(c[3]);
        mallShops[shopCount].root = NULL;

        shopCount++; loaded++;
    }
    cout << "Loaded " << loaded << " shops.\n";
    return loaded;
}

int loadItemsCSV(const string &f) {
    ifstream in(f);
    if (!in.is_open()) { cout << "ERROR loading " << f << "\n"; return 0; }

    string line; getline(in, line);
    int loaded = 0;

    while (getline(in, line)) {
        string c[5];
        if (splitCSV(line, c, 5) < 4) continue;
        int id = toInt(c[0]);

        for (int i = 0; i < shopCount; i++)
            if (mallShops[i].id == id)
                mallShops[i].root =
                    itemManager.insertNode(
                        mallShops[i].root,
                        c[1], toInt(c[2]), toInt(c[3])
                    );
        loaded++;
    }
    cout << "Loaded " << loaded << " items.\n";
    return loaded;
}

int loadStaffCSV(const string &f) {
    ifstream in(f);
    if (!in.is_open()) { cout << "ERROR loading " << f << "\n"; return 0; }

    string line; getline(in, line);
    int loaded = 0;

    while (getline(in, line)) {
        if (staffCount >= MALL_MAX_STAFF) break;
        string c[5];
        if (splitCSV(line, c, 5) < 3) continue;

        staffList[staffCount++] = {
            staffCount + 1, c[0], c[1], toInt(c[2])
        };
        loaded++;
    }
    cout << "Loaded " << loaded << " staff.\n";
    return loaded;
}

// =====================================================
// PARKING MENU
// =====================================================

void parkingMenu() {
    int r, c;
    cout << "Enter starting row (0-" << MALL_PARKING_SIZE-1 << "): ";
    cin >> r;
    cout << "Enter starting col (0-" << MALL_PARKING_SIZE-1 << "): ";
    cin >> c;

    if (r < 0 || r >= MALL_PARKING_SIZE ||
        c < 0 || c >= MALL_PARKING_SIZE) {
        cout << "Invalid position.\n";
        return;
    }

    int res = BFS_nearestParking(parkingGrid, r, c);
    if (res == -1) cout << "No parking slot available.\n";
    else cout << "Nearest empty slot at ("
              << res / MALL_PARKING_SIZE
              << ", " << res % MALL_PARKING_SIZE << ")\n";
}
// =====================================================
// KMP SEARCH (SEARCH ONLY — NO ROUTING)
// =====================================================

vector<int> kmpPrefix(const string &pat) {
    int m = pat.size();
    vector<int> lps(m, 0);
    int j = 0;

    for (int i = 1; i < m; i++) {
        while (j > 0 && pat[i] != pat[j])
            j = lps[j - 1];
        if (pat[i] == pat[j])
            j++;
        lps[i] = j;
    }
    return lps;
}

bool kmpSearch(const string &txt, const string &pat) {
    if (pat.empty()) return true;
    if (txt.size() < pat.size()) return false;

    vector<int> lps = kmpPrefix(pat);
    int i = 0, j = 0;

    while (i < (int)txt.size()) {
        if (txt[i] == pat[j]) {
            i++; j++;
        } else if (j > 0) {
            j = lps[j - 1];
        } else {
            i++;
        }

        if (j == (int)pat.size())
            return true;
    }
    return false;
}

struct SearchResult {
    int shopID;
    string shopName;
    string itemName;
    int price;
    int stock;
};

vector<SearchResult> searchItemInMall(const string &pattern) {
    vector<SearchResult> results;

    string patt = pattern;
    transform(patt.begin(), patt.end(), patt.begin(), ::tolower);

    for (int s = 0; s < shopCount; s++) {
        function<void(ItemNode*)> dfs = [&](ItemNode *node) {
            if (!node) return;

            dfs(node->left);

            string item = node->name;
            transform(item.begin(), item.end(), item.begin(), ::tolower);

            if (kmpSearch(item, patt)) {
                results.push_back({
                    mallShops[s].id,
                    mallShops[s].name,
                    node->name,
                    node->price,
                    node->stock
                });
            }

            dfs(node->right);
        };

        dfs(mallShops[s].root);
    }

    return results;
}

void searchItemsOnly() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string pat;

    cout << "Enter item name to search: ";
    getline(cin, pat);

    if (pat.empty()) {
        cout << "Empty search.\n";
        return;
    }

    auto results = searchItemInMall(pat);

    if (results.empty()) {
        cout << "No matching items found.\n";
        return;
    }

    cout << "\n=== Item Availability ===\n";
    for (int i = 0; i < results.size(); i++) {
        cout << i+1 << ") Shop: " << results[i].shopName
             << " (ID " << results[i].shopID << ")"
             << " | Item: " << results[i].itemName
             << " | Price: " << results[i].price
             << " | Stock: " << results[i].stock << "\n";
    }
}



// =====================================================
// MENU + SYSTEM
// =====================================================

void showMenu() {
    cout << "\n====================================\n";
    cout << "            MALL MENU\n";
    cout << "====================================\n";
    cout << " 1. Show all shops\n";
    cout << " 2. Add new shop\n";
    cout << " 3. Add item to shop\n";
    cout << " 4. Simulate customer purchase\n";
    cout << " 5. Sort shops by revenue (QuickSort)\n";
    cout << " 6. Parking Simulation (BFS)\n";
    cout << " 7. Fire Staff\n";
    cout << " 8. Add staff\n";
    cout << " 9. Show staff list\n";
    cout << "10. Schedule cleaner\n";
    cout << "11. Show cleaning log\n";
    cout << "12. Load shops from CSV (shops.csv)\n";
    cout << "13. Load items from CSV (items.csv)\n";
    cout << "14. Load staff from CSV (staff.csv)\n";
    cout << "15. SEARCH + route to shop (KMP)\n";
    cout << " 0. Exit\n";
    cout << "====================================\n";
    cout << "Enter choice: ";
}

void mallSystem() {
    srand(time(NULL));
    int choice;
    while (true) {
        showMenu();
        cin >> choice;

        switch (choice) {
            case 1: showAllShops(); break;
            case 2: addShop(); break;
            case 3: addItemToShop(); break;
            case 4: simulateCustomer(); break;
            case 5: sortShopsByRevenue(); break;
            case 6: parkingMenu(); break;
            case 7: deleteStaff(); break;
            case 8: addStaff(); break;
            case 9: showStaff(); break;
            case 10: scheduleCleaner(); break;
            case 11: showCleanLog(); break;
            case 12: loadShopsCSV("shops.csv"); break;
            case 13: loadItemsCSV("items.csv"); break;
            case 14: loadStaffCSV("staff.csv"); break;
            case 15: searchItemsOnly(); break;
            case 0:
                cout << "Exiting Mall Simulator...\n";
                return;
            default:
                cout << "Invalid choice!\n";
        }
    }
}

#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>
#include <cstring> // for strncpy
#include <limits>

using namespace std;

// -------------------- CONFIG (tweak if needed) --------------------
#define THEATRE_MAX_MOVIES 2000
#define THEATRE_MAX_AUDITORIUMS 50
#define THEATRE_MAX_SEAT_ROWS 50
#define THEATRE_MAX_SEAT_COLS 50
#define THEATRE_MAX_SHOWS 5000
#define THEATRE_MAX_BOOKINGS 20000
#define THEATRE_MAX_SNACKS 1000
#define THEATRE_MAX_SNACK_ORDERS 20000
#define THEATRE_MAX_STAFF 2000
#define THEATRE_MAX_MAINT_LOGS 5000
#define THEATRE_HASH_SIZE 32749
#define THEATRE_INF 999999

// -------------------- SIMPLE CSV & UTIL HELPERS --------------------
int theatreToInt(const string &s) {
    try { return stoi(s); } catch(...) { return 0; }
}

int theatreSplitCSV(const string &line, string out[], int maxCols) {
    int col = 0;
    string cur = "";
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') { inQuotes = !inQuotes; continue; }
        if (c == ',' && !inQuotes && col < maxCols - 1) {
            out[col++] = cur;
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    out[col++] = cur;
    return col;
}

// -------------------- DATA STRUCTS --------------------
// Movie BST (manual pointers)
struct TheatreMovie
{
    int movie_id;
    string title;
    string genre;
    int duration_minutes;
    double rating;
    string language;
    string release_date;
    TheatreMovie *left;
    TheatreMovie *right;
};

// Auditorium (array-based seats)
struct TheatreAuditorium
{
    int aud_id;
    string name;
    int rows;
    int cols;
    int total_seats;
    char seats[THEATRE_MAX_SEAT_ROWS][THEATRE_MAX_SEAT_COLS]; // 'E','B','X'
    string seat_type[THEATRE_MAX_SEAT_ROWS][THEATRE_MAX_SEAT_COLS];
};

// Show (array)
struct TheatreShow
{
    int show_id;
    int movie_id;
    int aud_id;
    string start_datetime; // "YYYY-MM-DD HH:MM"
    string end_datetime;
    int base_price;
    int tickets_sold;
    int revenue;
};

// Booking record (small POD)
struct TheatreBooking
{
    int booking_id;
    int show_id;
    char seat_label[8];
    char customer_name[100];
    char customer_phone[20];
    int price_paid;
    int status; // 0 cancelled, 1 active
    char booking_datetime[32];
};

// booking hash entry
struct TheatreBookingEntry
{
    int key;
    TheatreBooking val;
    bool used;
    bool deleted;
};

// Snack
struct TheatreSnack
{
    int snack_id;
    string name;
    string category;
    int price;
    int prep_time;
};

// Snack order queue item
struct TheatreSnackOrder
{
    int order_id;
    int booking_id;
    char seat_label[8];
    int item_ids[10];
    int item_count;
    int total_price;
    int status; // 0 pending,1 inprogress,2 done
    char order_datetime[32];
};

// staff & maintenance
struct TheatreStaff
{
    int id;
    string name;
    string role;
    int salary;
};
struct TheatreMaint
{
    int id;
    int aud_id;
    string date;
    string task;
    int staff_id;
    int status;
};

// -------------------- MODULE GLOBALS --------------------
static TheatreMovie *theatreMovieRoot = NULL;
static TheatreAuditorium auditoriums[THEATRE_MAX_AUDITORIUMS];
static int theatreAudCount = 0;

static TheatreShow shows[THEATRE_MAX_SHOWS];
static int theatreShowCount = 0;

static TheatreBookingEntry bookingHash[THEATRE_HASH_SIZE];
static int theatreBookingCount = 0;
static int theatreNextBookingId = 50000;

static TheatreSnack snacks[THEATRE_MAX_SNACKS];
static int theatreSnackCount = 0;

static TheatreSnackOrder snackQueueArr[THEATRE_MAX_SNACK_ORDERS];
static int snackQueueFront = 0, snackQueueRear = -1, snackQueueCount = 0;

static TheatreStaff theatreStaff[THEATRE_MAX_STAFF];
static int theatreStaffCount = 0;
static TheatreMaint theatreMaint[THEATRE_MAX_MAINT_LOGS];
static int theatreMaintCount = 0;

// auditorium graph for Dijkstra (matrix)


// -------------------- SMALL HELPERS --------------------
static string theatre_now_datetime()
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char buf[64];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min);
    return string(buf);
}

// -------------------- MOVIE BST FUNCTIONS --------------------
TheatreMovie *theatreCreateMovieNode(int id, const string &title, const string &genre,
                                     int duration, double rating, const string &lang, const string &rdate)
{
    TheatreMovie *m = new TheatreMovie();
    m->movie_id = id;
    m->title = title;
    m->genre = genre;
    m->duration_minutes = duration;
    m->rating = rating;
    m->language = lang;
    m->release_date = rdate;
    m->left = m->right = NULL;
    return m;
}

TheatreMovie *theatreInsertMovieNode(TheatreMovie *root, TheatreMovie *node)
{
    if (!root)
        return node;
    if (node->title < root->title)
        root->left = theatreInsertMovieNode(root->left, node);
    else
        root->right = theatreInsertMovieNode(root->right, node);
    return root;
}

TheatreMovie *theatreFindMovieByTitle(TheatreMovie *root, const string &title)
{
    if (!root)
        return NULL;
    if (title == root->title)
        return root;
    if (title < root->title)
        return theatreFindMovieByTitle(root->left, title);
    return theatreFindMovieByTitle(root->right, title);
}

int theatre_count_movies_recursive(TheatreMovie *r)
{
    if (!r)
        return 0;
    return 1 + theatre_count_movies_recursive(r->left) + theatre_count_movies_recursive(r->right);
}

bool theatreMoviesAtCapacity()
{
    int c = theatre_count_movies_recursive(theatreMovieRoot);
    return c >= THEATRE_MAX_MOVIES;
}

void theatreInorderList(TheatreMovie *root)
{
    if (!root)
        return;
    theatreInorderList(root->left);
    cout << "ID:" << root->movie_id << " | " << root->title << " | " << root->genre
         << " | " << root->duration_minutes << "min | Rating:" << root->rating
         << " | " << root->language << " | " << root->release_date << "\n";
    theatreInorderList(root->right);
}

TheatreMovie *theatreDeleteMovieNode(TheatreMovie *root, const string &title, bool &deleted)
{
    if (!root)
        return NULL;
    if (title < root->title)
        root->left = theatreDeleteMovieNode(root->left, title, deleted);
    else if (title > root->title)
        root->right = theatreDeleteMovieNode(root->right, title, deleted);
    else
    {
        deleted = true;
        if (!root->left)
        {
            TheatreMovie *t = root->right;
            delete root;
            return t;
        }
        else if (!root->right)
        {
            TheatreMovie *t = root->left;
            delete root;
            return t;
        }
        else
        {
            TheatreMovie *succ = root->right;
            while (succ->left)
                succ = succ->left;
            root->title = succ->title;
            root->movie_id = succ->movie_id;
            root->genre = succ->genre;
            root->duration_minutes = succ->duration_minutes;
            root->rating = succ->rating;
            root->language = succ->language;
            root->release_date = succ->release_date;
            root->right = theatreDeleteMovieNode(root->right, succ->title, deleted);
        }
    }
    return root;
}

// -------------------- QUICK SORT for indexes (exact style) --------------------
// We'll sort an index array referencing shows[] by comparator that returns negative/0/positive
typedef int (*IndexCmpFunc)(int Aidx, int Bidx);

int theatre_partition_index(int arr[], int l, int r, IndexCmpFunc cmp)
{
    // pivot = arr[l]
    int pivotIndex = arr[l];
    int i = l;
    int j = r + 1;
    while (true)
    {
        // i++
        do
        {
            i++;
        } while (i <= r && cmp(arr[i], pivotIndex) < 0);
        // j--
        do
        {
            j--;
        } while (j >= l && cmp(arr[j], pivotIndex) > 0);
        if (i >= j)
            break;
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
    // swap arr[l] and arr[j]
    int tmp = arr[l];
    arr[l] = arr[j];
    arr[j] = tmp;
    return j;
}

void theatre_quickSort_indices(int arr[], int l, int r, IndexCmpFunc cmp)
{
    if (l < r)
    {
        int s = theatre_partition_index(arr, l, r, cmp);
        theatre_quickSort_indices(arr, l, s - 1, cmp);
        theatre_quickSort_indices(arr, s + 1, r, cmp);
    }
}

// comparator for shows by start_datetime (lexicographic compares works for YYYY-MM-DD HH:MM)
int theatre_cmp_show_start(int Aidx, int Bidx)
{
    string A = shows[Aidx].start_datetime;
    string B = shows[Bidx].start_datetime;
    if (A < B)
        return -1;
    if (A > B)
        return 1;
    return 0;
}

// comparator for shows by revenue (desc)
int theatre_cmp_show_revenue(int Aidx, int Bidx)
{
    if (shows[Aidx].revenue > shows[Bidx].revenue)
        return -1;
    if (shows[Aidx].revenue < shows[Bidx].revenue)
        return 1;
    return 0;
}

// -------------------- BOYER–MOORE (bad char) --------------------
void theatre_bm_build_badchar(const string &pat, int badchar[256])
{
    int m = (int)pat.size();
    for (int i = 0; i < 256; ++i)
        badchar[i] = -1;
    for (int i = 0; i < m; ++i)
        badchar[(unsigned char)pat[i]] = i;
}
bool theatre_boyer_moore_search(const string &txt, const string &pat)
{
    if (pat.empty())
        return true;
    int n = (int)txt.size(), m = (int)pat.size();
    if (n < m)
        return false;
    int badchar[256];
    theatre_bm_build_badchar(pat, badchar);
    int s = 0;
    while (s <= n - m)
    {
        int j = m - 1;
        while (j >= 0 && pat[j] == txt[s + j])
            j--;
        if (j < 0)
            return true;
        else
        {
            int bc = badchar[(unsigned char)txt[s + j]];
            int shift = j - bc;
            if (shift < 1)
                shift = 1;
            s += shift;
        }
    }
    return false;
}

// -------------------- KMP (reuse style) --------------------
int *theatre_kmp_prefix_alloc(const string &pat)
{
    int m = (int)pat.size();
    int *lps = new int[m];
    if (m == 0)
        return lps;
    lps[0] = 0;
    int j = 0;
    for (int i = 1; i < m; ++i)
    {
        while (j > 0 && pat[i] != pat[j])
            j = lps[j - 1];
        if (pat[i] == pat[j])
            j++;
        lps[i] = j;
    }
    return lps;
}
bool theatre_kmp_search(const string &text, const string &pat)
{
    if (pat.empty())
        return true;
    if (text.size() < pat.size())
        return false;
    int *lps = theatre_kmp_prefix_alloc(pat);
    int i = 0, j = 0;
    int n = (int)text.size(), m = (int)pat.size();
    while (i < n)
    {
        if (text[i] == pat[j])
        {
            i++;
            j++;
        }
        else
        {
            if (j > 0)
                j = lps[j - 1];
            else
                i++;
        }
        if (j == m)
        {
            delete[] lps;
            return true;
        }
    }
    delete[] lps;
    return false;
}

// -------------------- BOOKING HASH (open addressing linear probing) --------------------
unsigned int theatre_hash_key(int key)
{
    return (unsigned int)(key * 2654435761u) % THEATRE_HASH_SIZE;
}
void theatre_init_booking_hash()
{
    for (int i = 0; i < THEATRE_HASH_SIZE; ++i)
    {
        bookingHash[i].used = false;
        bookingHash[i].deleted = false;
        bookingHash[i].key = -1;
    }
}
bool theatre_booking_insert(const TheatreBooking &b)
{
    if (theatreBookingCount >= THEATRE_MAX_BOOKINGS)
    {
        cout << "Overflow: booking capacity reached!\n";
        return false;
    }
    unsigned int idx = theatre_hash_key(b.booking_id);
    unsigned int start = idx;
    while (bookingHash[idx].used && !bookingHash[idx].deleted && bookingHash[idx].key != b.booking_id)
    {
        idx = (idx + 1) % THEATRE_HASH_SIZE;
        if (idx == start)
        {
            cout << "Hash table full!\n";
            return false;
        }
    }
    bookingHash[idx].used = true;
    bookingHash[idx].deleted = false;
    bookingHash[idx].key = b.booking_id;
    bookingHash[idx].val = b;
    theatreBookingCount++;
    return true;
}
bool theatre_booking_get(int key, TheatreBooking &out)
{
    unsigned int idx = theatre_hash_key(key);
    unsigned int start = idx;
    while (bookingHash[idx].used)
    {
        if (!bookingHash[idx].deleted && bookingHash[idx].key == key)
        {
            out = bookingHash[idx].val;
            return true;
        }
        idx = (idx + 1) % THEATRE_HASH_SIZE;
        if (idx == start)
            break;
    }
    return false;
}
bool theatre_booking_remove(int key)
{
    unsigned int idx = theatre_hash_key(key);
    unsigned int start = idx;
    while (bookingHash[idx].used)
    {
        if (!bookingHash[idx].deleted && bookingHash[idx].key == key)
        {
            bookingHash[idx].deleted = true;
            theatreBookingCount--;
            return true;
        }
        idx = (idx + 1) % THEATRE_HASH_SIZE;
        if (idx == start)
            break;
    }
    return false;
}

// -------------------- SNACK QUEUE --------------------
void theatre_init_snack_queue()
{
    snackQueueFront = 0;
    snackQueueRear = -1;
    snackQueueCount = 0;
}
bool theatre_enqueue_snack(const TheatreSnackOrder &ord)
{
    if (snackQueueCount >= THEATRE_MAX_SNACK_ORDERS)
    {
        cout << "Overflow: snack queue full!\n";
        return false;
    }
    snackQueueRear = (snackQueueRear + 1) % THEATRE_MAX_SNACK_ORDERS;
    snackQueueArr[snackQueueRear] = ord;
    snackQueueCount++;
    return true;
}
TheatreSnackOrder theatre_dequeue_snack()
{
    TheatreSnackOrder empty;
    empty.order_id = -1;
    if (snackQueueCount == 0)
        return empty;
    TheatreSnackOrder res = snackQueueArr[snackQueueFront];
    snackQueueFront = (snackQueueFront + 1) % THEATRE_MAX_SNACK_ORDERS;
    snackQueueCount--;
    return res;
}

// -------------------- CSV LOADERS & GENERATORS --------------------
// movies.csv: movie_id,title,genre,duration_minutes,rating,language,release_date
void theatreLoadMoviesCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open())
    {
        cout << "Cannot open " << fn << "\n";
        return;
    }
    string line;
    getline(in, line); // header
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.size() < 2)
            continue;
        string cols[8];
        int n = theatreSplitCSV(line, cols, 8);
        if (n < 6)
            continue;
        if (theatreMoviesAtCapacity())
        {
            cout << "Overflow: movies capacity reached!\n";
            break;
        }
        int id = theatreToInt(cols[0]);
        string title = cols[1];
        string genre = cols[2];
        int dur = theatreToInt(cols[3]);
        double rating = atof(cols[4].c_str());
        string lang = cols[5];
        string date = (n >= 7 ? cols[6] : "1970-01-01");
        TheatreMovie *m = theatreCreateMovieNode(id, title, genre, dur, rating, lang, date);
        theatreMovieRoot = theatreInsertMovieNode(theatreMovieRoot, m);
        loaded++;
    }
    cout << "Loaded " << loaded << " movies from " << fn << "\n";
}

// auditoriums.csv: aud_id,name,rows,cols,type,total_seats
void theatreLoadAuditoriumsCSV(const string &fn)
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
        if (line.size() < 2)
            continue;
        string cols[8];
        int n = theatreSplitCSV(line, cols, 8);
        if (n < 4)
            continue;
        if (theatreAudCount >= THEATRE_MAX_AUDITORIUMS)
        {
            cout << "Overflow: auditoriums max reached!\n";
            break;
        }
        int id = theatreToInt(cols[0]);
        auditoriums[theatreAudCount].aud_id = id;
        auditoriums[theatreAudCount].name = cols[1];
        int r = min(theatreToInt(cols[2]), THEATRE_MAX_SEAT_ROWS);
        int c = min(theatreToInt(cols[3]), THEATRE_MAX_SEAT_COLS);
        auditoriums[theatreAudCount].rows = r;
        auditoriums[theatreAudCount].cols = c;
        auditoriums[theatreAudCount].total_seats = r * c;
        for (int i = 0; i < r; ++i)
            for (int j = 0; j < c; ++j)
            {
                auditoriums[theatreAudCount].seats[i][j] = 'E';
                auditoriums[theatreAudCount].seat_type[i][j] = "standard";
            }
        theatreAudCount++;
        loaded++;
    }
    
    cout << "Loaded " << loaded << " auditoriums from " << fn << "\n";
}

// shows.csv: show_id,movie_id,aud_id,start_datetime,end_datetime,base_price
void theatreLoadShowsCSV(const string &fn)
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
        if (line.size() < 2)
            continue;
        string cols[8];
        int n = theatreSplitCSV(line, cols, 8);
        if (n < 6)
            continue;
        if (theatreShowCount >= THEATRE_MAX_SHOWS)
        {
            cout << "Overflow: shows max reached!\n";
            break;
        }
        shows[theatreShowCount].show_id = theatreToInt(cols[0]);
        shows[theatreShowCount].movie_id = theatreToInt(cols[1]);
        shows[theatreShowCount].aud_id = theatreToInt(cols[2]);
        shows[theatreShowCount].start_datetime = cols[3];
        shows[theatreShowCount].end_datetime = cols[4];
        shows[theatreShowCount].base_price = theatreToInt(cols[5]);
        shows[theatreShowCount].tickets_sold = 0;
        shows[theatreShowCount].revenue = 0;
        theatreShowCount++;
        loaded++;
    }
    cout << "Loaded " << loaded << " shows from " << fn << "\n";
}

// bookings.csv: booking_id,show_id,seat_label,customer_name,customer_phone,price_paid,status,booking_datetime
void theatreLoadBookingsCSV(const string &fn)
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
        if (line.size() < 2)
            continue;
        string cols[12];
        int n = theatreSplitCSV(line, cols, 12);
        if (n < 7)
            continue;
        TheatreBooking b;
        b.booking_id = theatreToInt(cols[0]);
        b.show_id = theatreToInt(cols[1]);
        strncpy(b.seat_label, cols[2].c_str(), sizeof(b.seat_label) - 1);
        b.seat_label[sizeof(b.seat_label) - 1] = 0;
        strncpy(b.customer_name, cols[3].c_str(), sizeof(b.customer_name) - 1);
        b.customer_name[sizeof(b.customer_name) - 1] = 0;
        strncpy(b.customer_phone, cols[4].c_str(), sizeof(b.customer_phone) - 1);
        b.customer_phone[sizeof(b.customer_phone) - 1] = 0;
        b.price_paid = theatreToInt(cols[5]);
        b.status = theatreToInt(cols[6]);
        if (n >= 8)
            strncpy(b.booking_datetime, cols[7].c_str(), sizeof(b.booking_datetime) - 1);
        else
            strncpy(b.booking_datetime, theatre_now_datetime().c_str(), sizeof(b.booking_datetime) - 1);
        theatre_booking_insert(b);
        loaded++;
        // mark seat as booked in auditorium if possible
        int sidx = -1;
        for (int i = 0; i < theatreShowCount; ++i) if (shows[i].show_id == b.show_id) { sidx = i; break; }
        if (sidx != -1) {
            int aud_idx = -1;
            for (int i = 0; i < theatreAudCount; ++i) if (auditoriums[i].aud_id == shows[sidx].aud_id) { aud_idx = i; break; }
            if (aud_idx != -1) {
                int rr, cc;
                string seatl = string(b.seat_label);
                auto theatre_parse_seat_label_local = [&](TheatreAuditorium &a, const string &label, int &r, int &c)->bool {
                    if (label.size() < 2) return false;
                    char rc = label[0];
                    int rowIdx = rc - 'A';
                    int colIdx = atoi(label.substr(1).c_str()) - 1;
                    if (rowIdx < 0 || rowIdx >= a.rows) return false;
                    if (colIdx < 0 || colIdx >= a.cols) return false;
                    r = rowIdx; c = colIdx; return true;
                };
                if (theatre_parse_seat_label_local(auditoriums[aud_idx], seatl, rr, cc)) {
                    auditoriums[aud_idx].seats[rr][cc] = (b.status==1 ? 'B' : 'E');
                }
            }
        }
    }
    cout << "Loaded " << loaded << " bookings from " << fn << "\n";
}

// staff CSV loader: id,name,role,salary
void theatreLoadStaffCSV(const string &fn)
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
        if (line.size() < 2)
            continue;
        string cols[6];
        int n = theatreSplitCSV(line, cols, 6);
        if (n < 4)
            continue;
        if (theatreStaffCount >= THEATRE_MAX_STAFF)
        {
            cout << "Overflow: theatre staff max reached!\n";
            break;
        }
        theatreStaff[theatreStaffCount].id = theatreToInt(cols[0]);
        theatreStaff[theatreStaffCount].name = cols[1];
        theatreStaff[theatreStaffCount].role = cols[2];
        theatreStaff[theatreStaffCount].salary = theatreToInt(cols[3]);
        theatreStaffCount++;
        loaded++;
    }
    cout << "Loaded " << loaded << " staff from " << fn << "\n";
}

// -------------------- INTERACTIVE / MENU FUNCTIONS --------------------
int theatre_createMovieID()
{
    static int mid = 3000;
    return ++mid;
}
int theatre_createShowID()
{
    static int sid = 11000;
    return ++sid;
}

void theatreAddMovie()
{
    if (theatreMoviesAtCapacity())
    {
        cout << "Overflow: movies capacity reached!\n";
        return;
    }
    int id = theatre_createMovieID();
    string title, genre, durS, ratingS, lang, date;
    cout << "Enter title: ";
    getline(cin, title);
    cout << "Enter genre: ";
    getline(cin, genre);
    cout << "Enter duration (minutes): ";
    getline(cin, durS);
    cout << "Enter rating (numeric): ";
    getline(cin, ratingS);
    cout << "Enter language: ";
    getline(cin, lang);
    cout << "Enter release date (YYYY-MM-DD): ";
    getline(cin, date);
    TheatreMovie *m = theatreCreateMovieNode(id, title, genre, theatreToInt(durS), atof(ratingS.c_str()), lang, date);
    theatreMovieRoot = theatreInsertMovieNode(theatreMovieRoot, m);
    cout << "Added movie id " << id << "\n";
}

void theatreDeleteMovie()
{
    cout << "Enter exact title to delete: ";
    string t;
    getline(cin, t);
    bool deleted = false;
    theatreMovieRoot = theatreDeleteMovieNode(theatreMovieRoot, t, deleted);
    if (deleted)
        cout << "Deleted.\n";
    else
        cout << "Not found.\n";
}

void theatreListMovies()
{
    if (!theatreMovieRoot)
    {
        cout << "No movies.\n";
        return;
    }
    theatreInorderList(theatreMovieRoot);
}

void theatreSearchMovie()
{
    cout << "Enter search pattern: ";
    string pat;
    getline(cin, pat);
    if (pat.empty())
    {
        cout << "Empty pattern.\n";
        return;
    }
    // traverse inorder and use boyer-moore on lowercase forms
    function<void(TheatreMovie *)> inorder = [&](TheatreMovie *node)
    {
        if (!node)
            return;
        inorder(node->left);
        string s = node->title;
        string ss = s, pp = pat;
        for (size_t i = 0; i < ss.size(); ++i)
            ss[i] = tolower(ss[i]);
        for (size_t i = 0; i < pp.size(); ++i)
            pp[i] = tolower(pp[i]);
        if (theatre_boyer_moore_search(ss, pp))
        {
            cout << "Found: " << node->title << " (ID " << node->movie_id << ")\n";
        }
        inorder(node->right);
    };
    inorder(theatreMovieRoot);
}

// Add auditorium interactively
void theatreAddAuditorium()
{
    if (theatreAudCount >= THEATRE_MAX_AUDITORIUMS)
    {
        cout << "Overflow: auditoriums limit\n";
        return;
    }
    int id = 600 + theatreAudCount;
    string name, rS, cS;
    cout << "Enter auditorium name: ";
    getline(cin, name);
    cout << "Enter rows: ";
    getline(cin, rS);
    cout << "Enter cols: ";
    getline(cin, cS);
    int r = min(theatreToInt(rS), THEATRE_MAX_SEAT_ROWS);
    int c = min(theatreToInt(cS), THEATRE_MAX_SEAT_COLS);
    auditoriums[theatreAudCount].aud_id = id;
    auditoriums[theatreAudCount].name = name;
    auditoriums[theatreAudCount].rows = r;
    auditoriums[theatreAudCount].cols = c;
    auditoriums[theatreAudCount].total_seats = r * c;
    for (int i = 0; i < r; ++i)
        for (int j = 0; j < c; ++j)
        {
            auditoriums[theatreAudCount].seats[i][j] = 'E';
            auditoriums[theatreAudCount].seat_type[i][j] = "standard";
        }
    theatreAudCount++;
    cout << "Added auditorium id " << id << "\n";
}

void theatreViewSeatMap()
{
    cout << "Enter auditorium id: ";
    int id;
    cin >> id;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    int idx = -1;
    for (int i = 0; i < theatreAudCount; ++i)
        if (auditoriums[i].aud_id == id)
        {
            idx = i;
            break;
        }
    if (idx == -1)
    {
        cout << "Not found.\n";
        return;
    }
    TheatreAuditorium &a = auditoriums[idx];
    cout << "Seat map for " << a.name << " (" << a.rows << "x" << a.cols << ")\n";
    for (int r = 0; r < a.rows; ++r)
    {
        for (int c = 0; c < a.cols; ++c)
            cout << a.seats[r][c];
        cout << "\n";
    }
}

bool theatre_parse_seat_label(TheatreAuditorium &a, const string &label, int &r, int &c)
{
    if (label.size() < 2)
        return false;
    char rc = label[0];
    int rowIdx = rc - 'A';
    int colIdx = atoi(label.substr(1).c_str()) - 1;
    if (rowIdx < 0 || rowIdx >= a.rows)
        return false;
    if (colIdx < 0 || colIdx >= a.cols)
        return false;
    r = rowIdx;
    c = colIdx;
    return true;
}

void theatreBookSeat()
{
    if (theatreBookingCount >= THEATRE_MAX_BOOKINGS)
    {
        cout << "Overflow: booking limit\n";
        return;
    }
    cout << "Enter show id: ";
    int sid;
    cin >> sid;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    int sidx = -1;
    for (int i = 0; i < theatreShowCount; ++i)
        if (shows[i].show_id == sid)
        {
            sidx = i;
            break;
        }
    if (sidx == -1)
    {
        cout << "Show not found.\n";
        return;
    }
    int aud_idx = -1;
    for (int i = 0; i < theatreAudCount; ++i)
        if (auditoriums[i].aud_id == shows[sidx].aud_id)
        {
            aud_idx = i;
            break;
        }
    if (aud_idx == -1)
    {
        cout << "Auditorium not found for this show.\n";
        return;
    }
    cout << "Enter seat label (e.g., A1): ";
    string seat;
    getline(cin, seat);
    int r, c;
    if (!theatre_parse_seat_label(auditoriums[aud_idx], seat, r, c))
    {
        cout << "Invalid seat label\n";
        return;
    }
    if (auditoriums[aud_idx].seats[r][c] != 'E')
    {
        cout << "Seat unavailable\n";
        return;
    }
    cout << "Enter customer name: ";
    string cname;
    getline(cin, cname);
    cout << "Enter customer phone: ";
    string phone;
    getline(cin, phone);
    TheatreBooking b;
    b.booking_id = theatreNextBookingId++;
    b.show_id = sid;
    strncpy(b.seat_label, seat.c_str(), sizeof(b.seat_label) - 1);
    b.seat_label[sizeof(b.seat_label) - 1] = 0;
    strncpy(b.customer_name, cname.c_str(), sizeof(b.customer_name) - 1);
    b.customer_name[sizeof(b.customer_name) - 1] = 0;
    strncpy(b.customer_phone, phone.c_str(), sizeof(b.customer_phone) - 1);
    b.customer_phone[sizeof(b.customer_phone) - 1] = 0;
    b.price_paid = shows[sidx].base_price;
    b.status = 1;
    strncpy(b.booking_datetime, theatre_now_datetime().c_str(), sizeof(b.booking_datetime) - 1);
    if (!theatre_booking_insert(b))
    {
        cout << "Failed to insert booking.\n";
        return;
    }
    auditoriums[aud_idx].seats[r][c] = 'B';
    shows[sidx].tickets_sold++;
    shows[sidx].revenue += b.price_paid;
    cout << "Booking done. ID: " << b.booking_id << "\n";
}

void theatreCancelBooking()
{
    cout << "Enter booking id: ";
    int bid;
    cin >> bid;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    TheatreBooking b;
    if (!theatre_booking_get(bid, b))
    {
        cout << "Not found.\n";
        return;
    }
    if (b.status == 0)
    {
        cout << "Already cancelled.\n";
        return;
    }
    // find show and free seat
    int sidx = -1;
    for (int i = 0; i < theatreShowCount; ++i)
        if (shows[i].show_id == b.show_id)
        {
            sidx = i;
            break;
        }
    if (sidx != -1)
    {
        int aud_idx = -1;
        for (int i = 0; i < theatreAudCount; ++i)
            if (auditoriums[i].aud_id == shows[sidx].aud_id)
            {
                aud_idx = i;
                break;
            }
        if (aud_idx != -1)
        {
            int rr, cc;
            if (theatre_parse_seat_label(auditoriums[aud_idx], string(b.seat_label), rr, cc))
            {
                auditoriums[aud_idx].seats[rr][cc] = 'E';
                shows[sidx].tickets_sold = max(0, shows[sidx].tickets_sold - 1);
                shows[sidx].revenue = max(0, shows[sidx].revenue - b.price_paid);
            }
        }
    }
    theatre_booking_remove(bid);
    cout << "Cancelled booking " << bid << "\n";
}

void theatreListShows()
{
    if (theatreShowCount == 0)
    {
        cout << "No shows.\n";
        return;
    }
    int idxArr[THEATRE_MAX_SHOWS];
    for (int i = 0; i < theatreShowCount; ++i)
        idxArr[i] = i;
    theatre_quickSort_indices(idxArr, 0, theatreShowCount - 1, theatre_cmp_show_start);
    cout << "Shows sorted by start time:\n";
    for (int i = 0; i < theatreShowCount; ++i)
    {
        TheatreShow &s = shows[idxArr[i]];
        cout << s.show_id << " | Movie:" << s.movie_id << " | Aud:" << s.aud_id << " | " << s.start_datetime << " - " << s.end_datetime << " | Price:" << s.base_price << " | Tickets:" << s.tickets_sold << "\n";
    }
}

bool theatre_check_show_conflict(const string &s1, const string &e1, const string &s2, const string &e2)
{
    // assuming format YYYY-MM-DD HH:MM lexicographical comparisons work
    if (s1 >= e2)
        return false;
    if (s2 >= e1)
        return false;
    return true;
}

void theatreAddShow()
{
    if (theatreShowCount >= THEATRE_MAX_SHOWS)
    {
        cout << "Overflow: shows limit\n";
        return;
    }
    TheatreShow sh;
    sh.show_id = theatre_createShowID();
    string mS, aS;
    cout << "Enter movie id: ";
    getline(cin, mS);
    cout << "Enter auditorium id: ";
    getline(cin, aS);
    cout << "Enter start datetime (YYYY-MM-DD HH:MM): ";
    getline(cin, sh.start_datetime);
    cout << "Enter end datetime (YYYY-MM-DD HH:MM): ";
    getline(cin, sh.end_datetime);
    cout << "Enter base price: ";
    string pS;
    getline(cin, pS);
    sh.movie_id = theatreToInt(mS);
    sh.aud_id = theatreToInt(aS);
    sh.base_price = theatreToInt(pS);
    sh.tickets_sold = 0;
    sh.revenue = 0;
    // conflict check
    for (int i = 0; i < theatreShowCount; ++i)
        if (shows[i].aud_id == sh.aud_id)
        {
            if (theatre_check_show_conflict(sh.start_datetime, sh.end_datetime, shows[i].start_datetime, shows[i].end_datetime))
            {
                cout << "Conflict with show " << shows[i].show_id << " : " << shows[i].start_datetime << " - " << shows[i].end_datetime << "\n";
                cout << "Add anyway? (y/n): ";
                char c;
                cin >> c;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (c != 'y' && c != 'Y')
                    return;
                break;
            }
        }
    shows[theatreShowCount++] = sh;
    cout << "Show added id " << sh.show_id << "\n";
}

// Snack functions
void theatreAddSnack()
{
    if (theatreSnackCount >= THEATRE_MAX_SNACKS)
    {
        cout << "Overflow: snacks limit\n";
        return;
    }
    TheatreSnack &s = snacks[theatreSnackCount];
    s.snack_id = 8000 + theatreSnackCount + 1;
    cout << "Enter snack name: ";
    getline(cin, s.name);
    cout << "Enter category: ";
    getline(cin, s.category);
    cout << "Enter price: ";
    string p;
    getline(cin, p);
    s.price = theatreToInt(p);
    cout << "Enter prep time (minutes): ";
    string t;
    getline(cin, t);
    s.prep_time = theatreToInt(t);
    theatreSnackCount++;
    cout << "Snack added id " << s.snack_id << "\n";
}
void theatreListSnacks()
{
    if (theatreSnackCount == 0)
    {
        cout << "No snacks.\n";
        return;
    }
    for (int i = 0; i < theatreSnackCount; ++i)
        cout << snacks[i].snack_id << " | " << snacks[i].name << " | " << snacks[i].category << " | Rs " << snacks[i].price << "\n";
}
void theatreOrderSnack()
{
    if (snackQueueCount >= THEATRE_MAX_SNACK_ORDERS)
    {
        cout << "Overflow: snack orders full\n";
        return;
    }
    TheatreSnackOrder ord;
    ord.order_id = rand() % 100000 + 10000;
    cout << "Enter booking id (0 if none): ";
    int bid;
    cin >> bid;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    ord.booking_id = bid;
    cout << "Enter seat label (e.g., A1): ";
    string seat;
    getline(cin, seat);
    strncpy(ord.seat_label, seat.c_str(), sizeof(ord.seat_label) - 1);
    ord.item_count = 0;
    ord.total_price = 0;
    cout << "Enter snack ids separated by space, terminated by 0:\n";
    while (true)
    {
        int sid;
        cin >> sid;
        if (sid == 0)
            break;
        if (ord.item_count < 10)
            ord.item_ids[ord.item_count++] = sid;
        for (int i = 0; i < theatreSnackCount; ++i)
            if (snacks[i].snack_id == sid)
                ord.total_price += snacks[i].price;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    ord.status = 0;
    strncpy(ord.order_datetime, theatre_now_datetime().c_str(), sizeof(ord.order_datetime) - 1);
    if (theatre_enqueue_snack(ord))
        cout << "Order queued id " << ord.order_id << " total Rs " << ord.total_price << "\n";
}

void theatreProcessSnack()
{
    TheatreSnackOrder ord = theatre_dequeue_snack();
    if (ord.order_id == -1)
    {
        cout << "No snack orders.\n";
        return;
    }
    cout << "Processing order " << ord.order_id << " total Rs " << ord.total_price << "\n";
    cout << "Mark done? (y/n): ";
    char c;
    cin >> c;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (c == 'y' || c == 'Y')
        cout << "Done.\n";
    else
    {
        ord.status = 0;
        theatre_enqueue_snack(ord);
        cout << "Requeued.\n";
    }
}

// Staff & maintenance
void theatreAddStaff()
{
    if (theatreStaffCount >= THEATRE_MAX_STAFF)
    {
        cout << "Overflow: staff limit\n";
        return;
    }
    TheatreStaff &s = theatreStaff[theatreStaffCount];
    s.id = theatreStaffCount + 1;
    cout << "Enter name: ";
    getline(cin, s.name);
    cout << "Enter role: ";
    getline(cin, s.role);
    cout << "Enter salary: ";
    string tmp;
    getline(cin, tmp);
    s.salary = theatreToInt(tmp);
    theatreStaffCount++;
    cout << "Staff added id " << s.id << "\n";
}
void theatreListStaff()
{
    if (theatreStaffCount == 0)
    {
        cout << "No staff.\n";
        return;
    }
    for (int i = 0; i < theatreStaffCount; ++i)
        cout << theatreStaff[i].id << " | " << theatreStaff[i].name << " | " << theatreStaff[i].role << " | Rs " << theatreStaff[i].salary << "\n";
}
void theatreAddMaint()
{
    if (theatreMaintCount >= THEATRE_MAX_MAINT_LOGS)
    {
        cout << "Overflow: maint logs full\n";
        return;
    }
    TheatreMaint &m = theatreMaint[theatreMaintCount];
    m.id = theatreMaintCount + 1;
    cout << "Enter auditorium id: ";
    string t;
    getline(cin, t);
    m.aud_id = theatreToInt(t);
    cout << "Enter date (YYYY-MM-DD): ";
    getline(cin, m.date);
    cout << "Enter task: ";
    getline(cin, m.task);
    cout << "Enter staff id: ";
    getline(cin, t);
    m.staff_id = theatreToInt(t);
    m.status = 0;
    theatreMaintCount++;
    cout << "Maintenance logged id " << m.id << "\n";
}
void theatreListMaint()
{
    if (theatreMaintCount == 0)
    {
        cout << "No maintenance logs.\n";
        return;
    }
    for (int i = 0; i < theatreMaintCount; ++i)
        cout << theatreMaint[i].id << " | Aud:" << theatreMaint[i].aud_id << " | " << theatreMaint[i].date << " | " << theatreMaint[i].task << " | Staff:" << theatreMaint[i].staff_id << "\n";
}

// Revenue report per show
void theatreShowRevenue()
{
    cout << "Enter show id: ";
    int sid;
    cin >> sid;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    for (int i = 0; i < theatreShowCount; ++i)
        if (shows[i].show_id == sid)
        {
            cout << "Tickets sold: " << shows[i].tickets_sold << " Revenue: Rs " << shows[i].revenue << "\n";
            return;
        }
    cout << "Show not found.\n";
}

// Evacuation route (Dijkstra)


// -------------------- MODULE MENU --------------------
void theatreShowMenu()
{
    cout << "\n====================================\n";
    cout << "         THEATRE MANAGEMENT\n";
    cout << "====================================\n";
    cout << " 1. Show all movies\n";
    cout << " 2. Add new movie\n";
    cout << " 3. Delete movie\n";
    cout << " 4. Search movie (BM)\n";
    cout << " 5. Sort shows by start time (QuickSort)\n";
    cout << " 6. Add auditorium\n";
    cout << " 7. View seat map\n";
    cout << " 8. Load auditoriums from CSV (auditoriums.csv)\n";
    cout << " 9. Add show (schedule)\n";
    cout << "10. List shows\n";
    cout << "11. Load shows from CSV (shows.csv)\n";
    cout << "12. Book seat\n";
    cout << "13. Cancel booking\n";
    cout << "14. Load bookings CSV (bookings.csv)\n";
    cout << "15. Find booking by phone (BM search)\n";
    cout << "16. Add snack\n";
    cout << "17. List snacks\n";
    cout << "18. Order snack to seat\n";
    cout << "19. Process next snack order\n";
    cout << "20. Add staff\n";
    cout << "21. Show staff\n";
    cout << "22. Load staff CSV (theatre_staff.csv)\n";
    cout << "23. Add maintenance log\n";
    cout << "24. Show maintenance logs\n";
    cout << "25. Show revenue for show\n";
    cout << "26. Load movies from CSV (movies.csv)\n";
    cout << "27. Load bookings from CSV (bookings.csv)\n";
    cout << "28. List auditorium\n";
    cout << "29.Load All Data\n";
    cout << " 0. Return to MAIN MENU\n";
    cout << "====================================\n";
    cout << "Enter choice: ";
}

void theatreInitModule()
{
    srand((unsigned int)time(NULL));
    theatre_init_booking_hash();
    theatre_init_snack_queue();
    
}

// find bookings by phone substring using BM
void theatreFindBookingByPhone()
{
    cout << "Enter phone substring: ";
    string pat;
    getline(cin, pat);
    if (pat.empty())
    {
        cout << "Empty.\n";
        return;
    }
    bool found = false;
    for (int i = 0; i < THEATRE_HASH_SIZE; ++i)
    {
        if (bookingHash[i].used && !bookingHash[i].deleted)
        {
            TheatreBooking &b = bookingHash[i].val;
            string phone = string(b.customer_phone);
            string phoneL = phone;
            string patL = pat;
            for (size_t k = 0; k < phoneL.size(); ++k)
                phoneL[k] = tolower(phoneL[k]);
            for (size_t k = 0; k < patL.size(); ++k)
                patL[k] = tolower(patL[k]);
            if (theatre_boyer_moore_search(phoneL, patL))
            {
                cout << "Booking ID: " << b.booking_id << " Show: " << b.show_id << " Seat: " << b.seat_label << " Cust: " << b.customer_name << " Phone: " << b.customer_phone << "\n";
                found = true;
            }
        }
    }
    if (!found)
        cout << "No matching bookings.\n";
}
void theatreListAuditoriums()
{
    if (theatreAudCount == 0)
    {
        cout << "No auditoriums available.\n";
        return;
    }

    cout << "\n--- AUDITORIUM LIST ---\n";
    for (int i = 0; i < theatreAudCount; i++)
    {
        TheatreAuditorium &a = auditoriums[i];
        cout << "ID: " << a.aud_id
             << " | Name: " << a.name
             << " | Rows: " << a.rows
             << " | Cols: " << a.cols
             << " | Total Seats: " << a.total_seats
             << "\n";
    }
}
void theatreLoadAllData()
{
    cout << "\n=== Loading ALL Theatre Data ===\n";

    theatreLoadMoviesCSV("movies.csv");
    theatreLoadAuditoriumsCSV("auditoriums.csv");
    theatreLoadShowsCSV("shows.csv");
    theatreLoadBookingsCSV("bookings.csv");
    theatreLoadStaffCSV("theatre_staff.csv");

    cout << "=== Finished loading all CSV files ===\n";
}



// -------------------- THEATRE SYSTEM MAIN (entry) --------------------
void theatreSystem()
{
    theatreInitModule();
    int choice;
    while (true)
    {
        theatreShowMenu();
        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        switch (choice)
        {
        case 1:
            theatreListMovies();
            break;
        case 2:
            theatreAddMovie();
            break;
        case 3:
            theatreDeleteMovie();
            break;
        case 4:
            theatreSearchMovie();
            break;
        case 5:
            theatreListShows();
            break;
        case 6:
            theatreAddAuditorium();
            break;
        case 7:
            theatreViewSeatMap();
            break;
        case 8:
            theatreLoadAuditoriumsCSV("auditoriums.csv");
            break;
        case 9:
            theatreAddShow();
            break;
        case 10:
            theatreListShows();
            break;
        case 11:
            theatreLoadShowsCSV("shows.csv");
            break;
        case 12:
            theatreBookSeat();
            break;
        case 13:
            theatreCancelBooking();
            break;
        case 14:
            theatreLoadBookingsCSV("bookings.csv");
            break;
        case 15:
            theatreFindBookingByPhone();
            break;
        case 16:
            theatreAddSnack();
            break;
        case 17:
            theatreListSnacks();
            break;
        case 18:
            theatreOrderSnack();
            break;
        case 19:
            theatreProcessSnack();
            break;
        case 20:
            theatreAddStaff();
            break;
        case 21:
            theatreListStaff();
            break;
        case 22:
            theatreLoadStaffCSV("theatre_staff.csv");
            break;
        case 23:
            theatreAddMaint();
            break;
        case 24:
            theatreListMaint();
            break;
        case 25:
            theatreShowRevenue();
            break;
        case 26:
            theatreLoadMoviesCSV("movies.csv");
            break;
        case 27:
            theatreLoadBookingsCSV("bookings.csv");
            break;
        case 28:
            theatreListAuditoriums();
            break;
        case 29:
             theatreLoadAllData();
             break;
        case 0:
            cout << "Returning to main menu...\n";
            return;
        default:
            cout << "Invalid choice.\n";
        }
    }
}

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
struct HotelsStaff {
    int id;
    string name;
    string role;
    int salary;
};
HotelsStaff hotelStaff[HOTEL_MAX_STAFF];
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
// hospital_module_reduced.cpp
// Reduced Hospital Management module — appointments-only approach.
// Removed: ER processing, separate surgeries, ambulance, Dijkstra, interactive lab result input.
// Lab results are only loaded via CSV (hospitalLoadLabsCSV).
// Minimal helper functions included: toInt, splitCSV.

#include <bits/stdc++.h>
using namespace std;

#define HOSPITAL_MAX_PATIENTS 2000
#define HOSPITAL_MAX_STAFF 500
#define HOSPITAL_MAX_ROOMS 500
#define HOSPITAL_MAX_BEDS 2000
#define HOSPITAL_MAX_APPOINTS 2000
#define HOSPITAL_HASH_SIZE 32749
#define HOSPITAL_INF 999999

// ---------- Helper utilities (simple) ----------
int hospitalToInt(const string &s)
{
    if (s.empty()) return 0;
    try { return stoi(s); } catch (...) { return 0; }
}

// Very simple CSV splitter
// Handles quoted fields with commas (basic)
int hospitalSplitCSV(const string &line, string out[], int maxCols)
{
    int col = 0;
    string cur;
    bool inq = false;
    for (size_t i = 0; i < line.size(); ++i)
    {
        char c = line[i];
        if (c == '"' )
        {
            inq = !inq;
            continue;
        }
        if (c == ',' && !inq)
        {
            if (col < maxCols) out[col++] = cur;
            cur.clear();
        }
        else
        {
            cur.push_back(c);
        }
    }
    if (col < maxCols) out[col++] = cur;
    return col;
}

// ---------- Entities ----------
struct HospitalPatient
{
    int patient_id;
    char name[128];
    int age;
    char gender[16];
    char contact[32];
    char address[256];
    int status; // 0 registered, 1 admitted, 3 discharged
    char notes[512];
    // minimal: more fields can be added
};

struct HospitalStaff
{
    int id;
    char name[128];
    char role[64];
    char department[64];
    int shift;
    int salary;
    char contact[32];
    char specialty[64];
};

struct HospitalRoom
{
    int roomID;
    char type[32];
    int capacity;
    int occupied;
    int bedStartIdx;
};

struct HospitalBed
{
    int bedID;
    int roomID;
    bool occupied;
    int patientID; // -1 if empty
};

// Appointment is now unified: general / surgery / lab test
struct HospitalAppointment
{
    int apptID;
    int type; // 1 = general, 2 = surgery, 3 = lab test
    int patientID;
    int doctorID;
    char date[16]; // YYYY-MM-DD
    char time[6];  // HH:MM
    int duration;  // minutes
    int status;    // 0 booked,1 done,2 cancelled

    // surgery-specific (optional, set when type==2)
    int OTroomID;
    int durationMins; // surgery duration
    char anesthesiaType[32];

    // lab-specific (optional, set when type==3)
    char testType[64];
    char resultDate[16];
    char resultSummary[256];

    char remarks[256];
};

// ---------- Module globals ----------
static HospitalPatient hospitalPatients[HOSPITAL_MAX_PATIENTS];
static int hospitalPatientCount = 0;
static int hospitalNextPatientID = 9000;

static HospitalStaff hospitalStaff[HOSPITAL_MAX_STAFF];
static int hospitalStaffCount = 0;
static int hospitalNextStaffID = 7000;

static HospitalRoom hospitalRooms[HOSPITAL_MAX_ROOMS];
static int hospitalRoomCount = 0;
static HospitalBed hospitalBeds[HOSPITAL_MAX_BEDS];
static int hospitalBedCount = 0;

static HospitalAppointment hospitalAppts[HOSPITAL_MAX_APPOINTS];
static int hospitalApptCount = 0;
static int hospitalNextApptID = 21000;

// ---------- Hash table for patients (by patient_id) ----------
struct HospitalPatientHashEntry
{
    int key;
    HospitalPatient val;
    bool used;
    bool deleted;
};
static HospitalPatientHashEntry hospitalPatientHash[HOSPITAL_HASH_SIZE];
static int hospitalPatientHashCount = 0;

unsigned int hospital_hash_key_int(int key)
{
    return (unsigned int)(key * 2654435761u) % HOSPITAL_HASH_SIZE;
}
void hospital_init_patient_hash()
{
    for (int i = 0; i < HOSPITAL_HASH_SIZE; ++i)
    {
        hospitalPatientHash[i].used = false;
        hospitalPatientHash[i].deleted = false;
        hospitalPatientHash[i].key = -1;
    }
    hospitalPatientHashCount = 0;
}
bool hospital_patient_hash_insert(const HospitalPatient &p)
{
    if (hospitalPatientHashCount >= HOSPITAL_MAX_PATIENTS) return false;
    unsigned int idx = hospital_hash_key_int(p.patient_id);
    unsigned int start = idx;
    while (hospitalPatientHash[idx].used && !hospitalPatientHash[idx].deleted && hospitalPatientHash[idx].key != p.patient_id)
    {
        idx = (idx + 1) % HOSPITAL_HASH_SIZE;
        if (idx == start) return false;
    }
    hospitalPatientHash[idx].used = true;
    hospitalPatientHash[idx].deleted = false;
    hospitalPatientHash[idx].key = p.patient_id;
    hospitalPatientHash[idx].val = p;
    hospitalPatientHashCount++;
    return true;
}
bool hospital_patient_hash_get(int key, HospitalPatient &out)
{
    unsigned int idx = hospital_hash_key_int(key);
    unsigned int start = idx;
    while (hospitalPatientHash[idx].used)
    {
        if (!hospitalPatientHash[idx].deleted && hospitalPatientHash[idx].key == key)
        {
            out = hospitalPatientHash[idx].val;
            return true;
        }
        idx = (idx + 1) % HOSPITAL_HASH_SIZE;
        if (idx == start) break;
    }
    return false;
}
bool hospital_patient_hash_remove(int key)
{
    unsigned int idx = hospital_hash_key_int(key);
    unsigned int start = idx;
    while (hospitalPatientHash[idx].used)
    {
        if (!hospitalPatientHash[idx].deleted && hospitalPatientHash[idx].key == key)
        {
            hospitalPatientHash[idx].deleted = true;
            hospitalPatientHashCount--;
            return true;
        }
        idx = (idx + 1) % HOSPITAL_HASH_SIZE;
        if (idx == start) break;
    }
    return false;
}

// ---------- Date/time helpers ----------
static string hospital_now_date()
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char buf[32];
    sprintf(buf, "%04d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    return string(buf);
}
static string hospital_now_time()
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char buf[16];
    sprintf(buf, "%02d:%02d", tm->tm_hour, tm->tm_min);
    return string(buf);
}

// ---------- CSV loaders & generators ----------
void hospitalLoadPatientsCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open()) { cout << "Cannot open " << fn << "\n"; return; }
    string line;
    getline(in, line); // header
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.size() < 2) continue;
        string cols[8];
        int n =hospitalSplitCSV(line, cols, 8);
        if (n < 2) continue;
        if (hospitalPatientCount >= HOSPITAL_MAX_PATIENTS) break;
        HospitalPatient p;
        p.patient_id = hospitalToInt(cols[0]);
        if (p.patient_id == 0) p.patient_id = ++hospitalNextPatientID;
        strncpy(p.name, cols[1].c_str(), sizeof(p.name) - 1); p.name[sizeof(p.name)-1]=0;
        p.age = (n>=3? hospitalToInt(cols[2]):0);
        strncpy(p.gender,(n>=4?cols[3].c_str():""),sizeof(p.gender)-1);
        strncpy(p.contact,(n>=5?cols[4].c_str():""),sizeof(p.contact)-1);
        strncpy(p.address,(n>=6?cols[5].c_str():""),sizeof(p.address)-1);
        p.status = (n>=7? hospitalToInt(cols[6]):0);
        strncpy(p.notes,(n>=8?cols[7].c_str():""),sizeof(p.notes)-1);
        hospitalPatients[hospitalPatientCount] = p;
        hospitalPatientCount++;
        hospital_patient_hash_insert(p);
        loaded++;
    }
    cout << "Loaded " << loaded << " patients from " << fn << "\n";
}

void hospitalLoadStaffCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open()) { cout << "Cannot open " << fn << "\n"; return; }
    string line;
    getline(in, line);
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.size() < 2) continue;
        string cols[8];
        int n = hospitalSplitCSV(line, cols, 8);
        if (n < 2) continue;
        if (hospitalStaffCount >= HOSPITAL_MAX_STAFF) break;
        HospitalStaff s;
        s.id =  hospitalToInt(cols[0]); if (s.id == 0) s.id = ++hospitalNextStaffID;
        strncpy(s.name, cols[1].c_str(), sizeof(s.name)-1); s.name[sizeof(s.name)-1]=0;
        strncpy(s.role, (n>=3?cols[2].c_str():""), sizeof(s.role)-1);
        strncpy(s.department, (n>=4?cols[3].c_str():""), sizeof(s.department)-1);
        s.shift = (n>=5? hospitalToInt(cols[4]):0);
        s.salary = (n>=6? hospitalToInt(cols[5]):0);
        strncpy(s.contact, (n>=7?cols[6].c_str():""), sizeof(s.contact)-1);
        strncpy(s.specialty, (n>=8?cols[7].c_str():""), sizeof(s.specialty)-1);
        hospitalStaff[hospitalStaffCount++] = s;
        loaded++;
    }
    cout << "Loaded " << loaded << " staff from " << fn << "\n";
}

void hospitalLoadRoomsCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open()) { cout << "Cannot open " << fn << "\n"; return; }
    string line;
    getline(in, line);
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.size() < 2) continue;
        string cols[6];
        int n = hospitalSplitCSV(line, cols, 6);
        if (n < 3) continue;
        if (hospitalRoomCount >= HOSPITAL_MAX_ROOMS) break;
        HospitalRoom r;
        r.roomID =  hospitalToInt(cols[0]);
        strncpy(r.type, cols[1].c_str(), sizeof(r.type)-1); r.type[sizeof(r.type)-1]=0;
        r.capacity =  hospitalToInt(cols[2]);
        r.occupied = 0;
        r.bedStartIdx = hospitalBedCount;
        for (int b = 0; b < r.capacity && hospitalBedCount < HOSPITAL_MAX_BEDS; ++b)
        {
            HospitalBed bd;
            bd.bedID = hospitalBedCount + 10000;
            bd.roomID = r.roomID;
            bd.occupied = false;
            bd.patientID = -1;
            hospitalBeds[hospitalBedCount++] = bd;
        }
        hospitalRooms[hospitalRoomCount++] = r;
        loaded++;
    }
    cout << "Loaded " << loaded << " rooms from " << fn << "\n";
}

// Appointments CSV loader: columns suggested:
// apptID, type(1=gen,2=surg,3=lab), patientID, doctorID, date, time, duration, status, remarks, (for surgery: OTroomID,durationMins,anesthesia),(for lab: testType,resultDate,resultSummary)
void hospitalLoadAppointmentsCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open()) { cout << "Cannot open " << fn << "\n"; return; }
    string line;
    getline(in, line);
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.size() < 2) continue;
        string cols[20];
        int n = hospitalSplitCSV(line, cols, 20);
        if (n < 6) continue;
        if (hospitalApptCount >= HOSPITAL_MAX_APPOINTS) break;
        HospitalAppointment a;
        a.apptID = hospitalToInt(cols[0]); if (a.apptID == 0) a.apptID = ++hospitalNextApptID;
        a.type =  hospitalToInt(cols[1]);
        a.patientID =  hospitalToInt(cols[2]);
        a.doctorID =  hospitalToInt(cols[3]);
        strncpy(a.date, cols[4].c_str(), sizeof(a.date)-1); a.date[sizeof(a.date)-1]=0;
        strncpy(a.time, cols[5].c_str(), sizeof(a.time)-1); a.time[sizeof(a.time)-1]=0;
        a.duration = (n>=7? hospitalToInt(cols[6]):15);
        a.status = (n>=8? hospitalToInt(cols[7]):0);
        if (n >= 9) strncpy(a.remarks, cols[8].c_str(), sizeof(a.remarks)-1); else strncpy(a.remarks,"",sizeof(a.remarks)-1);
        // surgery fields
        a.OTroomID = (n>=10? hospitalToInt(cols[9]):-1);
        a.durationMins = (n>=11? hospitalToInt(cols[10]):0);
        if (n >= 12) strncpy(a.anesthesiaType, cols[11].c_str(), sizeof(a.anesthesiaType)-1); else strncpy(a.anesthesiaType,"",sizeof(a.anesthesiaType)-1);
        // lab fields
        if (n >= 13) strncpy(a.testType, cols[12].c_str(), sizeof(a.testType)-1); else strncpy(a.testType,"",sizeof(a.testType)-1);
        if (n >= 14) strncpy(a.resultDate, cols[13].c_str(), sizeof(a.resultDate)-1); else strncpy(a.resultDate,"",sizeof(a.resultDate)-1);
        if (n >= 15) strncpy(a.resultSummary, cols[14].c_str(), sizeof(a.resultSummary)-1); else strncpy(a.resultSummary,"",sizeof(a.resultSummary)-1);
        hospitalAppts[hospitalApptCount++] = a;
        loaded++;
    }
    cout << "Loaded " << loaded << " appointments from " << fn << "\n";
}

// Lab CSV loader ONLY — used to update lab results (no interactive updating).
// Expected columns (recommended): apptID, patientID, testType, resultDate, resultSummary
void hospitalLoadLabsCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open()) { cout << "Cannot open " << fn << "\n"; return; }
    string line;
    getline(in, line);
    int updated = 0;
    while (getline(in, line))
    {
        if (line.size() < 2) continue;
        string cols[8];
        int n = hospitalSplitCSV(line, cols, 8);
        if (n < 3) continue;
        int apptID =  hospitalToInt(cols[0]);
        int pid =  hospitalToInt(cols[1]);
        string testType = (n>=3?cols[2]:"");
        string resDate = (n>=4?cols[3]:"");
        string resSummary = (n>=5?cols[4]:"");
        // Prefer matching by apptID if provided (>0), else match by patientID + testType (first not-done)
        bool found = false;
        if (apptID > 0)
        {
            for (int i = 0; i < hospitalApptCount; ++i)
            {
                if (hospitalAppts[i].apptID == apptID && hospitalAppts[i].type == 3)
                {
                    if (!testType.empty()) strncpy(hospitalAppts[i].testType, testType.c_str(), sizeof(hospitalAppts[i].testType)-1);
                    if (!resDate.empty()) strncpy(hospitalAppts[i].resultDate, resDate.c_str(), sizeof(hospitalAppts[i].resultDate)-1);
                    if (!resSummary.empty()) strncpy(hospitalAppts[i].resultSummary, resSummary.c_str(), sizeof(hospitalAppts[i].resultSummary)-1);
                    updated++; found = true; break;
                }
            }
        }
        if (!found && pid > 0)
        {
            for (int i = 0; i < hospitalApptCount; ++i)
            {
                if (hospitalAppts[i].patientID == pid && hospitalAppts[i].type == 3)
                {
                    // Use additional matching: testType if specified
                    if (!testType.empty() && strlen(hospitalAppts[i].testType) > 0 && string(hospitalAppts[i].testType) != testType) continue;
                    if (!testType.empty()) strncpy(hospitalAppts[i].testType, testType.c_str(), sizeof(hospitalAppts[i].testType)-1);
                    if (!resDate.empty()) strncpy(hospitalAppts[i].resultDate, resDate.c_str(), sizeof(hospitalAppts[i].resultDate)-1);
                    if (!resSummary.empty()) strncpy(hospitalAppts[i].resultSummary, resSummary.c_str(), sizeof(hospitalAppts[i].resultSummary)-1);
                    updated++; found = true; break;
                }
            }
        }
    }
    cout << "Updated lab results from " << fn << ".\n";
}

// ---------- Interactive functions (menus) ----------
int hospital_createPatientID() { return ++hospitalNextPatientID; }
int hospital_createStaffID() { return ++hospitalNextStaffID; }
int hospital_createApptID() { return ++hospitalNextApptID; }

// Add a new patient (ER or OPD)
void hospitalAddPatient()
{
    if (hospitalPatientCount >= HOSPITAL_MAX_PATIENTS)
    {
        cout << "Overflow: patients capacity reached!\n";
        return;
    }
    HospitalPatient p;
    p.patient_id = hospital_createPatientID();
    cout << "Enter name: ";
    string tmp;
    getline(cin, tmp);
    strncpy(p.name, tmp.c_str(), sizeof(p.name) - 1); p.name[sizeof(p.name)-1]=0;
    cout << "Enter age: ";
    getline(cin, tmp);
    p.age =  hospitalToInt(tmp);
    cout << "Enter gender: ";
    getline(cin, tmp);
    strncpy(p.gender, tmp.c_str(), sizeof(p.gender) - 1); p.gender[sizeof(p.gender)-1]=0;
    cout << "Enter contact: ";
    getline(cin, tmp);
    strncpy(p.contact, tmp.c_str(), sizeof(p.contact) - 1); p.contact[sizeof(p.contact)-1]=0;
    cout << "Enter address: ";
    getline(cin, tmp);
    strncpy(p.address, tmp.c_str(), sizeof(p.address) - 1); p.address[sizeof(p.address)-1]=0;
    p.status = 0;
    strncpy(p.notes, "", sizeof(p.notes)-1);
    hospitalPatients[hospitalPatientCount] = p;
    hospitalPatientCount++;
    hospital_patient_hash_insert(p);
    cout << "Patient registered. ID: " << p.patient_id << "\n";
}

// Admit patient by ID to specified room/bed
void hospitalAdmitPatient()
{
    cout << "Enter patient ID: ";
    int pid;
    if (!(cin >> pid)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout<<"Invalid.\n"; return; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    HospitalPatient p;
    if (!hospital_patient_hash_get(pid, p)) { cout << "Patient not found.\n"; return; }
    cout << "Enter roomID to admit into: ";
    int rid;
    if (!(cin >> rid)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout<<"Invalid.\n"; return; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    int bedIdx = -1;
    for (int i = 0; i < hospitalBedCount; i++)
        if (hospitalBeds[i].roomID == rid && !hospitalBeds[i].occupied)
        {
            bedIdx = i; break;
        }
    if (bedIdx == -1) { cout << "No free bed in room " << rid << "\n"; return; }
    hospitalBeds[bedIdx].occupied = true;
    hospitalBeds[bedIdx].patientID = pid;
    for (int i = 0; i < hospitalPatientCount; i++)
        if (hospitalPatients[i].patient_id == pid)
        {
            hospitalPatients[i].status = 1; // admitted
            hospital_patient_hash_remove(pid);
            hospital_patient_hash_insert(hospitalPatients[i]);
            break;
        }
    cout << "Admitted patient " << pid << " to bed " << hospitalBeds[bedIdx].bedID << "\n";
}

// Discharge patient
void hospitalDischargePatient()
{
    cout << "Enter patient ID to discharge: ";
    int pid;
    if (!(cin >> pid)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout<<"Invalid.\n"; return; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    for (int i = 0; i < hospitalPatientCount; i++)
    {
        if (hospitalPatients[i].patient_id == pid)
        {
            for (int b = 0; b < hospitalBedCount; b++)
            {
                if (hospitalBeds[b].patientID == pid)
                {
                    hospitalBeds[b].patientID = -1;
                    hospitalBeds[b].occupied = false;
                    break;
                }
            }
            hospitalPatients[i].status = 3;
            hospital_patient_hash_remove(pid);
            hospital_patient_hash_insert(hospitalPatients[i]);
            cout << "Patient " << pid << " discharged.\n";
            return;
        }
    }
    cout << "Patient not found.\n";
}

// Add staff
void hospitalAddStaffInteractive()
{
    if (hospitalStaffCount >= HOSPITAL_MAX_STAFF) { cout << "Overflow: staff capacity reached!\n"; return; }
    HospitalStaff s;
    s.id = hospital_createStaffID();
    cout << "Enter name: ";
    string tmp; getline(cin, tmp);
    strncpy(s.name, tmp.c_str(), sizeof(s.name)-1); s.name[sizeof(s.name)-1]=0;
    cout << "Enter role: "; getline(cin, tmp);
    strncpy(s.role, tmp.c_str(), sizeof(s.role)-1); s.role[sizeof(s.role)-1]=0;
    cout << "Enter department: "; getline(cin, tmp);
    strncpy(s.department, tmp.c_str(), sizeof(s.department)-1); s.department[sizeof(s.department)-1]=0;
    cout << "Enter shift (0 none,1 morning,2 evening,3 night): "; getline(cin, tmp);
    s.shift =  hospitalToInt(tmp);
    cout << "Enter salary: "; getline(cin, tmp); s.salary =  hospitalToInt(tmp);
    cout << "Enter contact: "; getline(cin, tmp); strncpy(s.contact, tmp.c_str(), sizeof(s.contact)-1);
    cout << "Enter specialty (if doctor): "; getline(cin, tmp); strncpy(s.specialty, tmp.c_str(), sizeof(s.specialty)-1);
    hospitalStaff[hospitalStaffCount++] = s;
    cout << "Staff added ID " << s.id << "\n";
}

void hospitalListStaff()
{
    if (hospitalStaffCount == 0) { cout << "No staff.\n"; return; }
    for (int i = 0; i < hospitalStaffCount; i++)
    {
        cout << hospitalStaff[i].id << " | " << hospitalStaff[i].name << " | " << hospitalStaff[i].role << " | Dept:" << hospitalStaff[i].department << " | Sal:" << hospitalStaff[i].salary << "\n";
    }
}

// Search patient by name substring (simple case-insensitive)
void hospitalSearchPatientByName()
{
    cout << "Enter name pattern: ";
    string pat;
    getline(cin, pat);
    if (pat.empty()) { cout << "Empty.\n"; return; }
    string patL = pat; for (size_t i = 0; i < patL.size(); ++i) patL[i] = tolower((unsigned char)patL[i]);
    bool found = false;
    for (int i = 0; i < hospitalPatientCount; i++)
    {
        string name = hospitalPatients[i].name;
        string nameL = name; for (size_t k = 0; k < nameL.size(); ++k) nameL[k] = tolower((unsigned char)nameL[k]);
        if (nameL.find(patL) != string::npos)
        {
            cout << "Found ID:" << hospitalPatients[i].patient_id << " | " << hospitalPatients[i].name << " | Age:" << hospitalPatients[i].age << " | Status:" << hospitalPatients[i].status << "\n";
            found = true;
        }
    }
    if (!found) cout << "No patients found.\n";
}

// Search by ID (hash)
void hospitalSearchPatientByIDInteractive()
{
    cout << "Enter patient ID: ";
    int pid;
    if (!(cin >> pid)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout<<"Invalid.\n"; return; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    HospitalPatient p;
    if (!hospital_patient_hash_get(pid, p)) { cout << "Patient not found.\n"; return; }
    cout << "Patient: " << p.patient_id << " | " << p.name << " | Age:" << p.age << " | Gender:" << p.gender << " | Contact:" << p.contact << " | Status:" << p.status << "\n";
}

// ---------- Merged Booking: Appointment / Surgery / Lab ----------
void hospitalBookAppointmentInteractive()
{
    if (hospitalApptCount >= HOSPITAL_MAX_APPOINTS) { cout << "Overflow: appointments limit\n"; return; }
    HospitalAppointment a;
    a.apptID = hospital_createApptID();
    cout << "Select booking type: (1) General Checkup  (2) Surgery  (3) Lab Test\n";
    int t;
    string tmp;
    getline(cin, tmp);
    t =  hospitalToInt(tmp);
    if (t < 1 || t > 3) { cout << "Invalid type.\n"; return; }
    a.type = t;
    cout << "Enter patient ID: "; getline(cin, tmp); a.patientID =  hospitalToInt(tmp);
    cout << "Enter doctor ID: "; getline(cin, tmp); a.doctorID =  hospitalToInt(tmp);
    cout << "Enter date (YYYY-MM-DD): "; getline(cin, tmp); strncpy(a.date, tmp.c_str(), sizeof(a.date)-1); a.date[sizeof(a.date)-1]=0;
    cout << "Enter time (HH:MM): "; getline(cin, tmp); strncpy(a.time, tmp.c_str(), sizeof(a.time)-1); a.time[sizeof(a.time)-1]=0;
    cout << "Enter duration minutes (typical): "; getline(cin, tmp); a.duration =  hospitalToInt(tmp);
    a.status = 0;
    strncpy(a.remarks, "", sizeof(a.remarks)-1);
    // If surgery, collect optional surgery details
    if (a.type == 2)
    {
        cout << "Enter OT room ID (optional, -1 if none): "; getline(cin, tmp); a.OTroomID =  hospitalToInt(tmp);
        cout << "Enter expected surgery duration (minutes): "; getline(cin, tmp); a.durationMins =  hospitalToInt(tmp);
        cout << "Enter anesthesia type: "; getline(cin, tmp); strncpy(a.anesthesiaType, tmp.c_str(), sizeof(a.anesthesiaType)-1); a.anesthesiaType[sizeof(a.anesthesiaType)-1]=0;
    }
    else
    {
        a.OTroomID = -1; a.durationMins = 0; strncpy(a.anesthesiaType,"",sizeof(a.anesthesiaType)-1);
    }
    // If lab, collect test type (results are loaded from CSV only later)
    if (a.type == 3)
    {
        cout << "Enter test type: "; getline(cin, tmp); strncpy(a.testType, tmp.c_str(), sizeof(a.testType)-1); a.testType[sizeof(a.testType)-1]=0;
        strncpy(a.resultDate, "", sizeof(a.resultDate)-1);
        strncpy(a.resultSummary, "", sizeof(a.resultSummary)-1);
    }
    else
    {
        strncpy(a.testType,"",sizeof(a.testType)-1);
        strncpy(a.resultDate,"",sizeof(a.resultDate)-1);
        strncpy(a.resultSummary,"",sizeof(a.resultSummary)-1);
    }
    hospitalAppts[hospitalApptCount++] = a;
    cout << "Appointment booked ID " << a.apptID << " (type " << a.type << ")\n";
}

// List appointments
void hospitalListAppointments()
{
    if (hospitalApptCount == 0) { cout << "No appointments.\n"; return; }
    cout << "Appointments:\n";
    for (int i = 0; i < hospitalApptCount; ++i)
    {
        HospitalAppointment &a = hospitalAppts[i];
        string tstr = (a.type==1?"General":(a.type==2?"Surgery":"Lab"));
        cout << a.apptID << " | " << tstr << " | Patient:" << a.patientID << " | Doctor:" << a.doctorID << " | " << a.date << " " << a.time << " | Dur:" << a.duration << " | Status:" << a.status;
        if (a.type == 2)
            cout << " | OT:" << a.OTroomID << " | SurgDur:" << a.durationMins << " | Anesth:" << a.anesthesiaType;
        if (a.type == 3)
            cout << " | Test:" << a.testType << " | ResultDate:" << a.resultDate << " | ResultSummary:" << a.resultSummary;
        if (strlen(a.remarks) > 0) cout << " | Remarks:" << a.remarks;
        cout << "\n";
    }
}
void hospitalShowAllPatients()
{
    if (hospitalPatientCount == 0)
    {
        cout << "No patients found.\n";
        return;
    }

    cout << "\n=========== ALL PATIENTS ==========\n";
    for (int i = 0; i < hospitalPatientCount; i++)
    {
        HospitalPatient &p = hospitalPatients[i];
        cout << "ID: " << p.patient_id
             << " | Name: " << p.name
             << " | Age: " << p.age
             << " | Gender: " << p.gender
             << " | Contact: " << p.contact
             << " | Status: " << p.status
             << "\nAddress: " << p.address
             << "\nNotes: " << p.notes
             << "\n------------------------------------\n";
    }
}

// Load all hospital data from CSVs at once
void hospitalLoadAllData()
{
    cout << "\n=== LOADING ALL HOSPITAL DATA ===\n";

    hospitalLoadPatientsCSV("patients.csv");
    hospitalLoadStaffCSV("hospital_staff.csv");
    hospitalLoadRoomsCSV("hospital_rooms.csv");
    hospitalLoadAppointmentsCSV("appointments.csv");
    hospitalLoadLabsCSV("labs.csv"); // updates lab results

    cout << "=== ALL DATA LOADED SUCCESSFULLY ===\n";
}


// ---------- Module menu ----------
void hospitalShowMenu()
{
    cout << "\n====================================\n";
    cout << "           HOSPITAL MANAGEMENT\n";
    cout << "====================================\n";
    cout << " 1. Add Patient (ER/OPD)\n";
    cout << " 2. Search Patient by ID\n";
    cout << " 3. Search Patient by Name\n";
    cout << " 4. Admit Patient (assign bed)\n";
    cout << " 5. Discharge Patient\n";
    cout << " 6. Add Staff\n";
    cout << " 7. List Staff\n";
    cout << " 8. Load patients CSV (patients.csv)\n";
    cout << " 9. Load staff CSV (hospital_staff.csv)\n";
    cout << "10. Load rooms CSV (hospital_rooms.csv)\n";
    cout << "11. Book Appointment / Surgery / Lab Test\n";
    cout << "12. List Appointments\n";
    cout << "13. Load appointments CSV (appointments.csv)\n";
    cout << "14. Load labs CSV (labs.csv)  (updates lab results only)\n";
    cout << "15. Load All CSV\n";
    cout << "16. Show All Patients\n";
    cout << " 0. Return to MAIN MENU\n";
    cout << "====================================\n";
    cout << "Enter choice: ";
}

void hospitalInitModule()
{
    hospital_init_patient_hash();
    // other arrays are default-initialized by static storage
}

// hospitalSystem (entry)
void hospitalSystem()
{
    hospitalInitModule();
    int choice;
    while (true)
    {
        hospitalShowMenu();
        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        switch (choice)
        {
        case 1: hospitalAddPatient(); break;
        case 2: hospitalSearchPatientByIDInteractive(); break;
        case 3: hospitalSearchPatientByName(); break;
        case 4: hospitalAdmitPatient(); break;
        case 5: hospitalDischargePatient(); break;
        case 6: hospitalAddStaffInteractive(); break;
        case 7: hospitalListStaff(); break;
        case 8: hospitalLoadPatientsCSV("patients.csv"); break;
        case 9: hospitalLoadStaffCSV("hospital_staff.csv"); break;
        case 10: hospitalLoadRoomsCSV("hospital_rooms.csv"); break;
        case 11: hospitalBookAppointmentInteractive(); break;
        case 12: hospitalListAppointments(); break;
        case 13: hospitalLoadAppointmentsCSV("appointments.csv"); break;
        case 14: hospitalLoadLabsCSV("labs.csv"); break;
        case 15: hospitalLoadAllData();break;
        case 16: hospitalShowAllPatients();break;

        case 0: cout << "Returning to main menu...\n"; return;
        default: cout << "Invalid choice.\n";
        }
    }
}
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
using namespace std;

/* ===================== CONFIG ===================== */
#define MAX_MEMBERS      2000
#define MAX_STAFF        1000
#define MAX_FACILITIES   500
#define MAX_EQUIPMENT    500
#define MAX_EVENTS       1500
#define MAX_BOOKINGS     5000
#define MAX_TXN          5000

/* ===================== HELPERS ===================== */
static int toInteger(const string &s){ try{return stoi(s);}catch(...){return 0;} }
static double toDouble(const string &s){ try{return stod(s);}catch(...){return 0.0;} }

static int communitysplitCSV(const string &line, string out[], int maxCols){
    int col = 0;
    string cur = "";
    bool inQ = false;
    for(char c : line){
        if(c == '"'){ inQ = !inQ; continue; }
        if(c == ',' && !inQ){
            if(col < maxCols) out[col++] = cur;
            cur.clear();
        } else cur.push_back(c);
    }
    if(col < maxCols) out[col++] = cur;
    return col;
}

static bool timeOverlap(const string &d1,const string &s1,const string &e1,
                        const string &d2,const string &s2,const string &e2){
    if(d1 != d2) return false;
    string a=d1+" "+s1, b=d1+" "+e1, c=d2+" "+s2, d=d2+" "+e2;
    return (a < d && c < b);
}

/* ===================== STRUCTS ===================== */
struct communityMember{ int id,age,active; string name,phone,email,mtype,join_date,address; };
struct communityStaff{ int id,active; double salary; string name,role,phone,email,join_date; };
struct communityFacility{ int id,capacity,active; double price; string name,type,location,from,to; };
struct communityEquipment{ int id,qty_total,qty_avail; string name,cond,last_maint; };
struct communityEvent{ int id,org_member,facility,expected; double revenue; string title,date,start,end,status; };
struct communityBooking{ int id,event_id,member_id,facility_id; double total; string date,start,end,status; };
struct communityRevenue{ int id,src_id; double amount; string src,date,desc; };
struct communityExpense{ int id,related; double amount; string date,vendor,desc,type; };

/* ===================== GLOBAL ARRAYS ===================== */
static communityMember members[MAX_MEMBERS]; int memberCount=0;
static communityStaff staffs[MAX_STAFF]; int staffCounts=0;
static communityFacility facilities[MAX_FACILITIES]; int facilityCount=0;
static communityEquipment equipmentArr[MAX_EQUIPMENT]; int equipmentCount=0;
static communityEvent eventsArr[MAX_EVENTS]; int eventCount=0;
static communityBooking bookings[MAX_BOOKINGS]; int bookingCount=0;
static communityRevenue revenues[MAX_TXN]; int revenueCount=0;
static communityExpense expenses[MAX_TXN]; int expenseCount=0;

/* ===================== FORWARD DEFS ===================== */
bool communityCheckBookingOverlap(int fac,const string &d,const string &st,const string &en);
void communityBulkFindOverlaps();
void communityMainMenu();

/* ============================================================
   ===================== TABLE FORMATTER =======================
   ============================================================ */

static void printHeader(const string cols[], const int w[], int n){
    for(int i=0;i<n;i++){
        cout<<left<<setw(w[i])<<cols[i];
        if(i<n-1) cout<<" | ";
    }
    cout<<"\n";
    for(int i=0;i<n;i++){
        cout<<string(w[i],'-');
        if(i<n-1) cout<<"-+-";
    }
    cout<<"\n";
}

/* ============================================================
   ======================= LOADERS =============================
   (Each loader resets its own count)
   ============================================================ */

void communityLoadMembersCSV(const string &fn){
    memberCount = 0;
    ifstream in(fn);
    if(!in){ cout<<"Cannot open "<<fn<<"\n"; return; }
    string line; getline(in,line);
    while(getline(in,line)){
        string c[9]; int n = communitysplitCSV(line,c,9);
        if(n<2) continue;
        communityMember &m = members[memberCount];
        m.id=toInteger(c[0]); m.name=c[1]; m.age=(n>2?toInteger(c[2]):0);
        m.phone=c[3]; m.email=c[4]; m.mtype=c[5];
        m.join_date=c[6]; m.address=c[7]; m.active=toInteger(c[8]);
        if(m.id!=0) memberCount++;
    }
    cout<<"Loaded "<<memberCount<<" members\n";
}

void communityLoadStaffCSV(const string &fn){
    staffCounts = 0;
    ifstream in(fn);
    if(!in){ cout<<"Cannot open "<<fn<<"\n"; return; }
    string line; getline(in,line);
    while(getline(in,line)){
        string c[8]; int n=communitysplitCSV(line,c,8);
        if(n<2) continue;
        communityStaff &s = staffs[staffCounts];
        s.id=toInteger(c[0]); s.name=c[1]; s.role=c[2];
        s.phone=c[3]; s.email=c[4];
        s.salary=(n>5?toDouble(c[5]):0); s.join_date=c[6];
        s.active=(n>7?toInteger(c[7]):1);
        if(s.id!=0) staffCounts++;
    }
    cout<<"Loaded "<<staffCounts<<" staff\n";
}

void communityLoadFacilitiesCSV(const string &fn){
    facilityCount = 0;
    ifstream in(fn);
    if(!in){ cout<<"Cannot open "<<fn<<"\n"; return; }
    string line; getline(in,line);
    while(getline(in,line)){
        string c[9]; int n=communitysplitCSV(line,c,9);
        communityFacility &f=facilities[facilityCount];
        f.id=toInteger(c[0]); f.name=c[1]; f.type=c[2];
        f.capacity=toInteger(c[3]); f.price=toDouble(c[4]);
        f.location=c[5]; f.from=c[6]; f.to=c[7];
        f.active=(n>8?toInteger(c[8]):1);
        if(f.id!=0) facilityCount++;
    }
    cout<<"Loaded "<<facilityCount<<" facilities\n";
}

void communityLoadEquipmentCSV(const string &fn){
    equipmentCount = 0;
    ifstream in(fn);
    if(!in){ cout<<"Cannot open "<<fn<<"\n"; return; }
    string line; getline(in,line);
    while(getline(in,line)){
        string c[6]; int n=communitysplitCSV(line,c,6);
        communityEquipment &e = equipmentArr[equipmentCount];
        e.id=toInteger(c[0]); e.name=c[1];
        e.qty_total=toInteger(c[2]);
        e.qty_avail=toInteger(c[3]);
        e.cond=c[4]; e.last_maint=c[5];
        if(e.id!=0) equipmentCount++;
    }
    cout<<"Loaded "<<equipmentCount<<" equipment\n";
}

void communityLoadEventsCSV(const string &fn){
    eventCount=0;
    ifstream in(fn);
    if(!in){ cout<<"Cannot open "<<fn<<"\n"; return; }
    string l; getline(in,l);
    while(getline(in,l)){
        string c[10]; communitysplitCSV(l,c,10);
        communityEvent &e = eventsArr[eventCount];
        e.id=toInteger(c[0]); e.title=c[1];
        e.org_member=toInteger(c[2]); e.facility=toInteger(c[3]);
        e.date=c[4]; e.start=c[5]; e.end=c[6];
        e.expected=toInteger(c[7]); e.revenue=toDouble(c[8]);
        e.status=c[9];
        if(e.id!=0) eventCount++;
    }
    cout<<"Loaded "<<eventCount<<" events\n";
}

bool communityCheckBookingOverlap(int fac,const string &d,const string &st,const string &en){
    for(int i=0;i<bookingCount;i++){
        communityBooking &b=bookings[i];
        if(b.facility_id==fac && b.date==d){
            if(timeOverlap(d,st,en,b.date,b.start,b.end))
                return true;
        }
    }
    return false;
}

void communityLoadBookingsCSV(const string &fn){
    bookingCount=0;
    ifstream in(fn);
    if(!in){ cout<<"Cannot open "<<fn<<"\n"; return; }
    string l; getline(in,l);
    while(getline(in,l)){
        string c[9]; communitysplitCSV(l,c,9);
        communityBooking b;
        b.id=toInteger(c[0]); b.event_id=toInteger(c[1]);
        b.member_id=toInteger(c[2]); b.facility_id=toInteger(c[3]);
        b.date=c[4]; b.start=c[5]; b.end=c[6];
        b.total=toDouble(c[7]); b.status=c[8];
        if(b.id==0) continue;

        if(communityCheckBookingOverlap(b.facility_id,b.date,b.start,b.end)){
            cout<<"Skip overlap booking "<<b.id<<"\n"; 
            continue;
        }
        bookings[bookingCount++] = b;
    }
    cout<<"Loaded "<<bookingCount<<" bookings\n";
}

void communityLoadRevenueCSV(const string &fn){
    revenueCount=0;
    ifstream in(fn);
    if(!in){ cout<<"Cannot open "<<fn<<"\n"; return; }
    string l; getline(in,l);
    while(getline(in,l)){
        string c[6]; communitysplitCSV(l,c,6);
        communityRevenue &r = revenues[revenueCount];
        r.id=toInteger(c[0]); r.src=c[1]; r.src_id=toInteger(c[2]);
        r.date=c[3]; r.amount=toDouble(c[4]); r.desc=c[5];
        if(r.id!=0) revenueCount++;
    }
    cout<<"Loaded "<<revenueCount<<" revenue rows\n";
}

void communityLoadExpensesCSV(const string &fn){
    expenseCount=0;
    ifstream in(fn);
    if(!in){ cout<<"Cannot open "<<fn<<"\n"; return; }
    string l; getline(in,l);
    while(getline(in,l)){
        string c[7]; communitysplitCSV(l,c,7);
        communityExpense &e=expenses[expenseCount];
        e.id=toInteger(c[0]); e.related=toInteger(c[1]);
        e.date=c[2]; e.amount=toDouble(c[3]);
        e.vendor=c[4]; e.desc=c[5]; e.type=c[6];
        if(e.id!=0) expenseCount++;
    }
    cout<<"Loaded "<<expenseCount<<" expenses\n";
}

/* Load ALL */
void communityLoadAllCSVsFromFolder(const string &p){
    string x=p;
    if(!x.empty() && x.back()!='/' && x.back()!='\\') x.push_back('/');
    memberCount=staffCounts=facilityCount=equipmentCount=eventCount=bookingCount=revenueCount=expenseCount=0;
    communityLoadMembersCSV(x+"communityMembers.csv");
    communityLoadStaffCSV(x+"communityStaff.csv");
    communityLoadFacilitiesCSV(x+"communityFacilities.csv");
    communityLoadEquipmentCSV(x+"communityEquipment.csv");
    communityLoadEventsCSV(x+"communityEvents.csv");
    communityLoadBookingsCSV(x+"communityBookings.csv");
    communityLoadRevenueCSV(x+"communityRevenue.csv");
    communityLoadExpensesCSV(x+"communityExpenses.csv");
}

/* ============================================================
   ====================== SORTING =============================
   ============================================================ */

static void swapB(communityBooking &a,communityBooking &b){ communityBooking t=a; a=b; b=t; }
static string bk(const communityBooking &b){ return b.date+" "+b.start; }

int partB(int l,int r){
    string p=bk(bookings[l]);
    int i=l,j=r+1;
    while(true){
        do{i++;} while(i<=r && bk(bookings[i])<p);
        do{j--;} while(j>=l && bk(bookings[j])>p);
        if(i>=j) break;
        swapB(bookings[i],bookings[j]);
    }
    swapB(bookings[l],bookings[j]);
    return j;
}
void qsortB(int l,int r){ if(l<r){ int s=partB(l,r); qsortB(l,s-1); qsortB(s+1,r);} }

void communitySortBookings(){
    if(bookingCount>1) qsortB(0,bookingCount-1);
    cout<<"Bookings sorted\n";
}

/* Facilities sorting */
static void swapF(communityFacility &a,communityFacility &b){ communityFacility t=a;a=b;b=t; }

int partFP(int l,int r){
    double p=facilities[l].price;
    int i=l,j=r+1;
    while(true){
        do{i++;} while(i<=r && facilities[i].price<p);
        do{j--;} while(j>=l && facilities[j].price>p);
        if(i>=j) break;
        swapF(facilities[i],facilities[j]);
    }
    swapF(facilities[l],facilities[j]);
    return j;
}
void qsortFP(int l,int r){ if(l<r){ int s=partFP(l,r); qsortFP(l,s-1); qsortFP(s+1,r);} }
void communitySortFacilitiesByPrice(){ if(facilityCount>1) qsortFP(0,facilityCount-1); }

int partFC(int l,int r){
    int p=facilities[l].capacity;
    int i=l,j=r+1;
    while(true){
        do{i++;} while(i<=r && facilities[i].capacity<p);
        do{j--;} while(j>=l && facilities[j].capacity>p);
        if(i>=j) break;
        swapF(facilities[i],facilities[j]);
    }
    swapF(facilities[l],facilities[j]);
    return j;
}
void qsortFC(int l,int r){ if(l<r){ int s=partFC(l,r); qsortFC(l,s-1); qsortFC(s+1,r);} }
void communitySortFacilitiesByCapacity(){ if(facilityCount>1) qsortFC(0,facilityCount-1); }

void communityBulkFindOverlaps(){
    communitySortBookings();
    for(int i=1;i<bookingCount;i++){
        communityBooking &a=bookings[i-1], &b=bookings[i];
        if(a.facility_id==b.facility_id && a.date==b.date){
            if(timeOverlap(a.date,a.start,a.end,b.date,b.start,b.end))
                cout<<"Overlap: "<<a.id<<" <-> "<<b.id<<" ("<<a.date<<")\n";
        }
    }
}

/* ============================================================
   ====================== MANUAL ADD ==========================
   ============================================================ */

void addMember(){
    if(memberCount>=MAX_MEMBERS){ cout<<"Full\n"; return; }
    communityMember &m=members[memberCount];
    cout<<"ID: "; cin>>m.id; cin.ignore();
    cout<<"Name: "; getline(cin,m.name);
    cout<<"Age: "; cin>>m.age; cin.ignore();
    cout<<"Phone: "; getline(cin,m.phone);
    cout<<"Email: "; getline(cin,m.email);
    cout<<"Type: "; getline(cin,m.mtype);
    cout<<"Join Date: "; getline(cin,m.join_date);
    cout<<"Address: "; getline(cin,m.address);
    m.active=1; memberCount++;
}

void addStaffs(){
    if(staffCounts>=MAX_STAFF){ cout<<"Full\n"; return; }
    communityStaff &s=staffs[staffCounts];
    cout<<"ID: "; cin>>s.id; cin.ignore();
    cout<<"Name: "; getline(cin,s.name);
    cout<<"Role: "; getline(cin,s.role);
    cout<<"Phone: "; getline(cin,s.phone);
    cout<<"Email: "; getline(cin,s.email);
    cout<<"Salary: "; cin>>s.salary; cin.ignore();
    cout<<"Join Date: "; getline(cin,s.join_date);
    s.active=1; staffCounts++;
}

void addFacility(){
    if(facilityCount>=MAX_FACILITIES){ cout<<"Full\n"; return; }
    communityFacility &f=facilities[facilityCount];
    cout<<"ID: "; cin>>f.id; cin.ignore();
    cout<<"Name: "; getline(cin,f.name);
    cout<<"Type: "; getline(cin,f.type);
    cout<<"Capacity: "; cin>>f.capacity; cin.ignore();
    cout<<"Price: "; cin>>f.price; cin.ignore();
    cout<<"Location: "; getline(cin,f.location);
    cout<<"From: "; getline(cin,f.from);
    cout<<"To: "; getline(cin,f.to);
    f.active=1; facilityCount++;
}

void addEquipment(){
    if(equipmentCount>=MAX_EQUIPMENT){ cout<<"Full\n"; return; }
    communityEquipment &e=equipmentArr[equipmentCount];
    cout<<"ID: "; cin>>e.id; cin.ignore();
    cout<<"Name: "; getline(cin,e.name);
    cout<<"Total Qty: "; cin>>e.qty_total; cin.ignore();
    cout<<"Available Qty: "; cin>>e.qty_avail; cin.ignore();
    cout<<"Cond: "; getline(cin,e.cond);
    cout<<"Last Maint: "; getline(cin,e.last_maint);
    equipmentCount++;
}

void addEvent(){
    if(eventCount>=MAX_EVENTS){ cout<<"Full\n"; return; }
    communityEvent &ev=eventsArr[eventCount];
    cout<<"ID: "; cin>>ev.id; cin.ignore();
    cout<<"Title: "; getline(cin,ev.title);
    cout<<"Org Member: "; cin>>ev.org_member;
    cout<<"Facility: "; cin>>ev.facility; cin.ignore();
    cout<<"Date: "; getline(cin,ev.date);
    cout<<"Start: "; getline(cin,ev.start);
    cout<<"End: "; getline(cin,ev.end);
    cout<<"Expected: "; cin>>ev.expected;
    cout<<"Revenue Est: "; cin>>ev.revenue; cin.ignore();
    ev.status="scheduled";
    eventCount++;
}

void addBooking(){
    if(bookingCount>=MAX_BOOKINGS){ cout<<"Full\n"; return; }
    communityBooking b;
    cout<<"ID: "; cin>>b.id;
    cout<<"Event: "; cin>>b.event_id;
    cout<<"Member: "; cin>>b.member_id;
    cout<<"Facility: "; cin>>b.facility_id; cin.ignore();
    cout<<"Date: "; getline(cin,b.date);
    cout<<"Start: "; getline(cin,b.start);
    cout<<"End: "; getline(cin,b.end);

    if(communityCheckBookingOverlap(b.facility_id,b.date,b.start,b.end)){
        cout<<"Overlap rejected\n"; return;
    }

    cout<<"Total: "; cin>>b.total; cin.ignore();
    cout<<"Status: "; getline(cin,b.status);

    bookings[bookingCount++] = b;

if(b.status == "paid" && revenueCount < MAX_TXN){
    revenues[revenueCount].id = revenueCount;
    revenues[revenueCount].src = "booking";
    revenues[revenueCount].src_id = b.id;
    revenues[revenueCount].date = b.date;
    revenues[revenueCount].amount = b.total;
    revenues[revenueCount].desc = "auto";
    revenueCount++;
}

}

void addRevenue(){
    if(revenueCount>=MAX_TXN){ cout<<"Full\n"; return; }
    communityRevenue &r=revenues[revenueCount];
    cout<<"ID: "; cin>>r.id; cin.ignore();
    cout<<"Source: "; getline(cin,r.src);
    cout<<"Source ID: "; cin>>r.src_id; cin.ignore();
    cout<<"Date: "; getline(cin,r.date);
    cout<<"Amount: "; cin>>r.amount; cin.ignore();
    cout<<"Desc: "; getline(cin,r.desc);
    revenueCount++;
}

void addExpense(){
    if(expenseCount>=MAX_TXN){ cout<<"Full\n"; return; }
    communityExpense &e=expenses[expenseCount];
    cout<<"ID: "; cin>>e.id;
    cout<<"Related Event: "; cin>>e.related; cin.ignore();
    cout<<"Date: "; getline(cin,e.date);
    cout<<"Amount: "; cin>>e.amount; cin.ignore();
    cout<<"Vendor: "; getline(cin,e.vendor);
    cout<<"Desc: "; getline(cin,e.desc);
    cout<<"Type: "; getline(cin,e.type);
    expenseCount++;
}

/* ============================================================
   ======================== LISTING ============================
   ============================================================ */

void listMembers(){
    const string c[]={"ID","Name","Phone","Email","Type","Join"};
    int w[]={5,16,12,20,10,12};
    printHeader(c,w,6);
    for(int i=0;i<memberCount;i++){
        communityMember &m=members[i];
        cout<<setw(w[0])<<m.id<<" | "<<setw(w[1])<<m.name<<" | "<<setw(w[2])<<m.phone<<" | "<<setw(w[3])<<m.email<<" | "<<setw(w[4])<<m.mtype<<" | "<<setw(w[5])<<m.join_date<<"\n";
    }
}

void listStaff(){
    const string c[]={"ID","Name","Role","Phone","Salary","Join"};
    int w[]={5,16,12,12,10,12};
    printHeader(c,w,6);
    for(int i=0;i<staffCounts;i++){
        communityStaff &s=staffs[i];
        cout<<setw(w[0])<<s.id<<" | "<<setw(w[1])<<s.name<<" | "<<setw(w[2])<<s.role<<" | "<<setw(w[3])<<s.phone<<" | "<<setw(w[4])<<s.salary<<" | "<<setw(w[5])<<s.join_date<<"\n";
    }
}

void listFacilities(){
    const string c[]={"ID","Name","Type","Cap","Price","Location"};
    int w[]={5,16,10,6,10,14};
    printHeader(c,w,6);
    for(int i=0;i<facilityCount;i++){
        communityFacility &f=facilities[i];
        cout<<setw(w[0])<<f.id<<" | "<<setw(w[1])<<f.name<<" | "<<setw(w[2])<<f.type<<" | "<<setw(w[3])<<f.capacity<<" | "<<setw(w[4])<<f.price<<" | "<<setw(w[5])<<f.location<<"\n";
    }
}

void listEquipment(){
    const string c[]={"ID","Name","Total","Avail","Cond","LastMaint"};
    int w[]={5,16,7,7,10,12};
    printHeader(c,w,6);
    for(int i=0;i<equipmentCount;i++){
        communityEquipment &e=equipmentArr[i];
        cout<<setw(w[0])<<e.id<<" | "<<setw(w[1])<<e.name<<" | "<<setw(w[2])<<e.qty_total<<" | "<<setw(w[3])<<e.qty_avail<<" | "<<setw(w[4])<<e.cond<<" | "<<setw(w[5])<<e.last_maint<<"\n";
    }
}

void listEvents(){
    const string c[]={"ID","Title","Fac","Date","Start","End","Exp","Rev","Status"};
    int w[]={5,16,5,12,7,7,6,10,10};
    printHeader(c,w,9);
    for(int i=0;i<eventCount;i++){
        communityEvent &e=eventsArr[i];
        cout<<setw(w[0])<<e.id<<" | "<<setw(w[1])<<e.title<<" | "<<setw(w[2])<<e.facility<<" | "<<setw(w[3])<<e.date<<" | "
            <<setw(w[4])<<e.start<<" | "<<setw(w[5])<<e.end<<" | "<<setw(w[6])<<e.expected<<" | "<<setw(w[7])<<e.revenue<<" | "<<setw(w[8])<<e.status<<"\n";
    }
}

void listBookings(){
    const string c[]={"ID","Event","Member","Facility","Date","Start","End","Amt","Status"};
    int w[]={5,7,7,8,12,7,7,10,10};
    printHeader(c,w,9);
    for(int i=0;i<bookingCount;i++){
        communityBooking &b=bookings[i];
        cout<<setw(w[0])<<b.id<<" | "<<setw(w[1])<<b.event_id<<" | "<<setw(w[2])<<b.member_id<<" | "<<setw(w[3])<<b.facility_id<<" | "
            <<setw(w[4])<<b.date<<" | "<<setw(w[5])<<b.start<<" | "<<setw(w[6])<<b.end<<" | "<<setw(w[7])<<b.total<<" | "<<setw(w[8])<<b.status<<"\n";
    }
}

/* ============================================================
   ======================== FINANCE ============================
   ============================================================ */

void eventPnL(int eid){
    double R=0,E=0;
    for(int i=0;i<revenueCount;i++){
        communityRevenue &r=revenues[i];
        if(r.src=="booking"){
            for(int j=0;j<bookingCount;j++)
                if(bookings[j].id==r.src_id && bookings[j].event_id==eid)
                    R+=r.amount;
        }
        else if(r.src=="event" && r.src_id==eid)
            R+=r.amount;
    }
    for(int i=0;i<expenseCount;i++)
        if(expenses[i].related==eid) E+=expenses[i].amount;
    cout<<"P&L for event "<<eid<<" = "<<(R-E)<<" (Rev: "<<R<<"  Exp: "<<E<<")\n";
}

void monthlyRevenue(const string &m){
    double t=0; 
    for(int i=0;i<revenueCount;i++)
        if(revenues[i].date.rfind(m,0)==0) t+=revenues[i].amount;
    cout<<"Revenue for "<<m<<" = "<<t<<"\n";
}

void bookingsRange(const string &f,const string &t){
    const string c[]={"ID","Event","Mem","Fac","Date","Start","End","Amt"};
    int w[]={5,6,6,6,12,7,7,10};
    printHeader(c,w,8);
    for(int i=0;i<bookingCount;i++){
        communityBooking &b=bookings[i];
        if(b.date>=f && b.date<=t)
            cout<<setw(w[0])<<b.id<<" | "<<setw(w[1])<<b.event_id<<" | "<<setw(w[2])<<b.member_id<<" | "
                <<setw(w[3])<<b.facility_id<<" | "<<setw(w[4])<<b.date<<" | "<<setw(w[5])<<b.start<<" | "
                <<setw(w[6])<<b.end<<" | "<<setw(w[7])<<b.total<<"\n";
    }
}

/* ============================================================
   ========================= MAIN MENU =========================
   ============================================================ */

void communitySystem(){
    while(true){
        cout<<"\n==== COMMUNITY CENTRE ====\n"
            <<"1. Members\n"
            <<"2. Staff\n"
            <<"3. Facilities\n"
            <<"4. Equipment\n"
            <<"5. Events & Bookings\n"
            <<"6. Finance\n"
            <<"7. Reports\n"
            <<"8. Load ALL CSVs\n"
            <<"0. Return\nChoice: ";

        int c; cin>>c; cin.ignore();

        switch(c){

        case 1:{
            cout<<"1=Load 2=Add 3=List\nChoice: ";
            int s; cin>>s; cin.ignore();
            if(s==1) communityLoadMembersCSV("communityMembers.csv");
            else if(s==2) addMember();
            else if(s==3) listMembers();
            break;
        }

        case 2:{
            cout<<"1=Load 2=Add 3=List 4=Payroll\nChoice: ";
            int s; cin>>s; cin.ignore();
            if(s==1) communityLoadStaffCSV("communityStaff.csv");
            else if(s==2) addStaffs();
            else if(s==3) listStaff();
            else if(s==4){
                double total=0;
                for(int i=0;i<staffCounts;i++) total+=staffs[i].salary;
                cout<<"Total payroll = "<<total<<"\n";
            }
            break;
        }

        case 3:{
            cout<<"1=Load 2=Add 3=List 4=SortPrice 5=SortCap\nChoice: ";
            int s; cin>>s; cin.ignore();
            if(s==1) communityLoadFacilitiesCSV("communityFacilities.csv");
            else if(s==2) addFacility();
            else if(s==3) listFacilities();
            else if(s==4) communitySortFacilitiesByPrice();
            else if(s==5) communitySortFacilitiesByCapacity();
            break;
        }

        case 4:{
            cout<<"1=Load 2=Add 3=List\nChoice: ";
            int s; cin>>s; cin.ignore();
            if(s==1) communityLoadEquipmentCSV("communityEquipment.csv");
            else if(s==2) addEquipment();
            else if(s==3) listEquipment();
            break;
        }

        case 5:{
            cout<<"1=LoadEvts 2=LoadBkng 3=AddEvt 4=AddBkng 5=ListEvts 6=ListBkng 7=Sort 8=Overlap\nChoice: ";
            int s; cin>>s; cin.ignore();
            if(s==1) communityLoadEventsCSV("communityEvents.csv");
            else if(s==2) communityLoadBookingsCSV("communityBookings.csv");
            else if(s==3) addEvent();
            else if(s==4) addBooking();
            else if(s==5) listEvents();
            else if(s==6) listBookings();
            else if(s==7) communitySortBookings();
            else if(s==8) communityBulkFindOverlaps();
            break;
        }

        case 6:{
            cout<<"1=AddRevenue 2=AddExpense 3=EventPnL\nChoice: ";
            int s; cin>>s; cin.ignore();
            if(s==1) addRevenue();
            else if(s==2) addExpense();
            else if(s==3){ int id; cout<<"Event id: "; cin>>id; eventPnL(id); }
            break;
        }

        case 7:{
            cout<<"1=Range 2=Monthly\nChoice: ";
            int s; cin>>s; cin.ignore();
            if(s==1){ string f,t; cout<<"From: "; getline(cin,f); cout<<"To: "; getline(cin,t); bookingsRange(f,t); }
            else if(s==2){ string m; cout<<"YYYY-MM: "; getline(cin,m); monthlyRevenue(m); }
            break;
        }

        case 8:{
            cout<<"Loading ALL...\n";
            communityLoadAllCSVsFromFolder("");
            break;
        }

        case 0: return;
        default: cout<<"Invalid\n";
        }
    }
}


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>
using namespace std;

#define ATM_MAX_ACCOUNTS 2000
#define ATM_MAX_TRANSACTIONS 20000
#define ATM_MAX_CASH 10
#define ATM_MAX_RATES 50
#define ATM_PIN_ATTEMPT_LIMIT 3
#define ATM_MAX_WITHDRAW_PER_TXN 20000
#define ATM_MAX_DEPOSIT_PER_TXN 200000
#define ATM_DAILY_LIMIT 50000
#define ATM_INTL_FEE 0.02

struct ATMAccount {
    string number, name, type, currency, cardCountry;
    int pin=0, locked=0, wrongPin=0;
    double balance=0, dayWithdraw=0, limit=0;
};

struct ATMTransaction {
    int id=0;
    string acc, type, currency, atmID, time, remark;
    double amt=0, balAfter=0;
};

struct ATMCash {
    string atmID, location;
    int n2000=0, n500=0, n200=0, n100=0, n50=0;
};

struct ATMRate {
    string code;
    double toINR=1.0;
};

ATMAccount ACC[ATM_MAX_ACCOUNTS];
ATMTransaction TX[ATM_MAX_TRANSACTIONS];
ATMCash CASH[ATM_MAX_CASH];
ATMRate RATE[ATM_MAX_RATES];

int accCnt = 0, txCnt = 0, cashCnt = 0, rateCnt = 0;
bool dataLoaded = false;

int findAcc(const string &a){
    for(int i=0;i<accCnt;i++) if(ACC[i].number==a) return i;
    return -1;
}
double getRate(const string &c){
    for(int i=0;i<rateCnt;i++) if(RATE[i].code==c) return RATE[i].toINR;
    return 1.0;
}
double toINR(double x,const string &c){ return x * getRate(c); }
double fromINR(double x,const string &c){ double r = getRate(c); return (r==0? x : x / r); }
double feeINR(double x){ return x * ATM_INTL_FEE; }

// Safe stoi/stod that return false on invalid input
bool safe_stoi(const string &s, int &out){
    if(s.empty()) return false;
    try { size_t pos; out = stoi(s,&pos); return pos==s.size(); }
    catch(...) { return false; }
}
bool safe_stod(const string &s, double &out){
    if(s.empty()) return false;
    try { size_t pos; out = stod(s,&pos); return pos==s.size(); }
    catch(...) { return false; }
}

// trim helper (remove leading/trailing spaces)
static inline string trim(const string &s){
    size_t i=0,j=s.size();
    while(i<j && isspace((unsigned char)s[i])) i++;
    while(j>i && isspace((unsigned char)s[j-1])) j--;
    return s.substr(i,j-i);
}

void loadAccounts(){
    ifstream f("atm_accounts.csv");
    if(!f){ cout<<"Warning: atm_accounts.csv not found -> continuing with empty accounts.\n"; return; }
    string line;
    accCnt = 0;
    while(getline(f,line)){
        if(line.empty()) continue;
        stringstream ss(line);
        ATMAccount a;
        string tmp;

        // read all fields defensively
        getline(ss,tmp,','); a.number = trim(tmp);
        getline(ss,tmp,','); a.name = trim(tmp);
        getline(ss,tmp,','); if(!safe_stoi(trim(tmp), a.pin)) { continue; }
        getline(ss,tmp,','); a.type = trim(tmp);
        getline(ss,tmp,','); a.currency = trim(tmp);
        getline(ss,tmp,','); if(!safe_stod(trim(tmp), a.balance)) { continue; }
        getline(ss,tmp,','); safe_stoi(trim(tmp), a.locked); // optional
        getline(ss,tmp,','); safe_stoi(trim(tmp), a.wrongPin);
        getline(ss,tmp,','); safe_stod(trim(tmp), a.dayWithdraw);
        getline(ss,tmp,','); safe_stod(trim(tmp), a.limit);
        getline(ss,tmp,','); a.cardCountry = trim(tmp);

        if(a.number.empty()) continue;
        ACC[accCnt++] = a;
        if(accCnt >= ATM_MAX_ACCOUNTS) break;
    }
    f.close();
}

void loadCash(){
    ifstream f("atm_cash.csv");
    if(!f){ cout<<"Warning: atm_cash.csv not found -> continuing with empty cash.\n"; return; }
    string line;
    cashCnt = 0;
    while(getline(f,line)){
        if(line.empty()) continue;
        stringstream ss(line);
        ATMCash c; string tmp;
        getline(ss,tmp,','); c.atmID = trim(tmp);
        getline(ss,tmp,','); c.location = trim(tmp);
        if(!getline(ss,tmp,',')) continue; if(!safe_stoi(trim(tmp), c.n2000)) c.n2000 = 0;
        if(!getline(ss,tmp,',')) continue; if(!safe_stoi(trim(tmp), c.n500)) c.n500 = 0;
        if(!getline(ss,tmp,',')) continue; if(!safe_stoi(trim(tmp), c.n200)) c.n200 = 0;
        if(!getline(ss,tmp,',')) continue; if(!safe_stoi(trim(tmp), c.n100)) c.n100 = 0;
        if(!getline(ss,tmp,',')) continue; if(!safe_stoi(trim(tmp), c.n50)) c.n50 = 0;
        if(c.atmID.empty()) c.atmID = "ATM001";
        CASH[cashCnt++] = c;
        if(cashCnt >= ATM_MAX_CASH) break;
    }
    f.close();
}

void loadRates(){
    ifstream f("atm_rates.csv");
    if(!f){ cout<<"Warning: atm_rates.csv not found -> continuing with default rates.\n"; return; }
    string line;
    rateCnt = 0;
    while(getline(f,line)){
        if(line.empty()) continue;
        stringstream ss(line);
        ATMRate r; string tmp;
        getline(ss,tmp,','); r.code = trim(tmp);
        getline(ss,tmp,',');
        if(!safe_stod(trim(tmp), r.toINR)) continue;
        if(r.code.empty()) continue;
        RATE[rateCnt++] = r;
        if(rateCnt >= ATM_MAX_RATES) break;
    }
    f.close();
}

void loadTx(){
    ifstream f("atm_transactions.csv");
    if(!f){ cout<<"Warning: atm_transactions.csv not found -> starting with empty transactions.\n"; return; }
    string line;
    txCnt = 0;
    while(getline(f,line)){
        if(line.empty()) continue;
        stringstream ss(line);
        ATMTransaction t; string tmp;

        // try to parse id; if fail, skip row (handles header or bad rows)
        if(!getline(ss,tmp,',')) continue;
        if(!safe_stoi(trim(tmp), t.id)) continue;
        getline(ss,t.acc,','); t.acc = trim(t.acc);
        getline(ss,t.type,','); t.type = trim(t.type);
        if(!getline(ss,tmp,',')) continue;
        if(!safe_stod(trim(tmp), t.amt)) continue;
        getline(ss,t.currency,','); t.currency = trim(t.currency);
        getline(ss,t.atmID,','); t.atmID = trim(t.atmID);
        getline(ss,t.time,','); t.time = trim(t.time);
        if(!getline(ss,tmp,',')) continue;
        if(!safe_stod(trim(tmp), t.balAfter)) continue;
        getline(ss,t.remark,','); t.remark = trim(t.remark);

        TX[txCnt++] = t;
        if(txCnt >= ATM_MAX_TRANSACTIONS) break;
    }
    f.close();
}

void loadAll(){
    loadAccounts();
    loadCash();
    loadRates();
    loadTx();
    dataLoaded = true;
    cout<<"CSV LOAD COMPLETE. (manual load done)\n";
}

void logTx(const string &acc,const string &type,double amt,const string &remark){
    if(txCnt >= ATM_MAX_TRANSACTIONS) return;
    ATMTransaction t;
    t.id = txCnt + 1;
    t.acc = acc;
    t.type = type;
    t.amt = amt;
    t.currency = "INR";
    t.atmID = (cashCnt>0 ? CASH[0].atmID : string("ATM001"));
    t.time = "TODAY";
    int idx = findAcc(acc);
    t.balAfter = (idx==-1? 0.0 : ACC[idx].balance);
    t.remark = remark;
    TX[txCnt++] = t;
}

bool dispense(int amt, ATMCash &c){
    long long total = (long long)c.n2000*2000 + (long long)c.n500*500 + (long long)c.n200*200 + (long long)c.n100*100 + (long long)c.n50*50;
    if(total < amt) return false;
    int r = amt;
    int a = min(c.n2000, r/2000); r -= a*2000;
    int b = min(c.n500, r/500); r -= b*500;
    int d = min(c.n200, r/200); r -= d*200;
    int e = min(c.n100, r/100); r -= e*100;
    int f = min(c.n50, r/50); r -= f*50;
    if(r != 0) return false;
    c.n2000 -= a; c.n500 -= b; c.n200 -= d; c.n100 -= e; c.n50 -= f;
    return true;
}

int login(){
    string a; int pin;
    cout<<"Enter Account Number: ";
    if(!(cin>>a)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); return -1; }
    int i = findAcc(a);
    if(i == -1){ cout<<"Not found.\n"; return -1; }
    if(ACC[i].locked){ cout<<"Locked.\n"; return -1; }
    cout<<"PIN: ";
    if(!(cin>>pin)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); return -1; }
    if(pin != ACC[i].pin){
        ACC[i].wrongPin++;
        if(ACC[i].wrongPin >= ATM_PIN_ATTEMPT_LIMIT) ACC[i].locked = 1;
        cout<<"Wrong PIN.\n";
        return -1;
    }
    ACC[i].wrongPin = 0;
    return i;
}

void checkBal(int i){
    cout<<"Balance: "<<ACC[i].balance<<" "<<ACC[i].currency<<"\n";
    logTx(ACC[i].number,"BAL_CHECK",0,"Balance Inquiry");
}

void deposit(int i){
    double amt;
    cout<<"Enter deposit amount: ";
    if(!(cin>>amt)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout<<"Invalid input.\n"; return; }
    if(amt <= 0 || amt > ATM_MAX_DEPOSIT_PER_TXN){ cout<<"Invalid deposit amount.\n"; return; }
    ACC[i].balance += amt;
    logTx(ACC[i].number,"DEPOSIT",amt,"Cash Deposit");
    cout<<"Deposit successful. New balance: "<<ACC[i].balance<<"\n";
}

void withdraw(int i){
    double amt;
    cout<<"Enter withdrawal amount: ";
    if(!(cin>>amt)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout<<"Invalid input.\n"; return; }
    if(amt <= 0 || amt > ATM_MAX_WITHDRAW_PER_TXN){ cout<<"Invalid withdrawal amount.\n"; return; }
    if(ACC[i].dayWithdraw + amt > ATM_DAILY_LIMIT){ cout<<"Daily withdrawal limit exceeded.\n"; return; }
    double required = amt;
    if(ACC[i].currency != "INR"){
        double conv = fromINR(amt, ACC[i].currency);
        double fee = feeINR(amt);
        double feeConv = fromINR(fee, ACC[i].currency);
        required = conv + feeConv;
        cout<<"International conversion: "<<conv<<" "<<ACC[i].currency<<", fee: "<<feeConv<<" "<<ACC[i].currency<<"\n";
    }
    if(required > ACC[i].balance){ cout<<"Insufficient funds after conversion.\n"; return; }
    if(cashCnt == 0){ cout<<"ATM cash not loaded.\n"; return; }
    if(!dispense((int)amt, CASH[0])){ cout<<"ATM cannot dispense this amount exactly.\n"; return; }
    ACC[i].balance -= required;
    ACC[i].dayWithdraw += amt;
    logTx(ACC[i].number,"WITHDRAW",amt,"Cash Withdrawal");
    cout<<"Withdrawal successful. New balance: "<<ACC[i].balance<<"\n";
}

bool adminLogin(){
    cout<<"Enter ADMIN PIN: ";
    int p; if(!(cin>>p)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); return false; }
    return p == 9999;
}

void adminAddAcc(){
    if(accCnt >= ATM_MAX_ACCOUNTS){ cout<<"Account storage full.\n"; return; }
    ATMAccount a;
    cout<<"Enter Account Number: "; cin>>a.number;
    if(findAcc(a.number) != -1){ cout<<"Account already exists.\n"; return; }
    cout<<"Customer Name: "; cin.ignore(); getline(cin, a.name);
    cout<<"PIN (numeric): "; cin>>a.pin;
    cout<<"Account Type (SAVINGS/CURRENT): "; cin>>a.type;
    cout<<"Currency (INR/USD/EUR): "; cin>>a.currency;
    cout<<"Initial Balance: "; cin>>a.balance;
    if(a.balance < 0){ cout<<"Balance cannot be negative.\n"; return; }
    cout<<"Withdrawal Limit: "; cin>>a.limit;
    cout<<"Card Country Code: "; cin>>a.cardCountry;
    a.locked = 0; a.wrongPin = 0; a.dayWithdraw = 0;
    ACC[accCnt++] = a;
    cout<<"Account created successfully.\n";
}

void adminEditAcc(){
    string an; cout<<"Enter Account Number to edit: "; cin>>an;
    int i = findAcc(an);
    if(i == -1){ cout<<"Account not found.\n"; return; }
    int ch = -1;
    do{
        cout<<"1.Change Name 2.Change PIN 3.Change Type 4.Change Balance 5.Unlock 6.Change Limit 7.Change Currency 0.Done\nChoice: ";
        if(!(cin>>ch)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout<<"Invalid input.\n"; continue; }
        switch(ch){
            case 1: cin.ignore(); cout<<"New Name: "; getline(cin, ACC[i].name); break;
            case 2: cout<<"New PIN: "; cin>>ACC[i].pin; break;
            case 3: cout<<"New Type: "; cin>>ACC[i].type; break;
            case 4: { double nb; cout<<"New Balance: "; cin>>nb; if(nb < 0) cout<<"Cannot set negative balance.\n"; else ACC[i].balance = nb; break; }
            case 5: ACC[i].locked = 0; ACC[i].wrongPin = 0; cout<<"Account unlocked.\n"; break;
            case 6: cout<<"New Withdrawal Limit: "; cin>>ACC[i].limit; break;
            case 7: cout<<"New Currency: "; cin>>ACC[i].currency; break;
            case 0: break;
            default: cout<<"Invalid option.\n";
        }
    } while(ch != 0);
}

void adminCash(){
    if(cashCnt == 0){
        CASH[0].atmID = "ATM001";
        CASH[0].location = "MAIN";
        CASH[0].n2000 = CASH[0].n500 = CASH[0].n200 = CASH[0].n100 = CASH[0].n50 = 0;
        cashCnt = 1;
    }
    int a,b,d,e,f;
    cout<<"Enter counts to ADD for 2000 500 200 100 50 (space separated): ";
    if(!(cin>>a>>b>>d>>e>>f)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout<<"Invalid input.\n"; return; }
    if(a<0||b<0||d<0||e<0||f<0){ cout<<"Cannot add negative notes.\n"; return; }
    CASH[0].n2000 += a; CASH[0].n500 += b; CASH[0].n200 += d; CASH[0].n100 += e; CASH[0].n50 += f;
    cout<<"ATM cash updated.\n";
}

void adminAddRate(){
    ATMRate r;
    cout<<"Enter currency code (e.g. USD): "; cin>>r.code;
    cout<<"Enter rate to INR (e.g. 83): "; cin>>r.toINR;
    if(r.toINR <= 0){ cout<<"Invalid rate.\n"; return; }
    RATE[rateCnt++] = r;
    cout<<"Rate added.\n";
}

void userMenu(int idx){
    while(true){
        int c;
        cout<<"1.Check Balance 2.Withdraw 3.Deposit 0.Logout\nChoice: ";
        if(!(cin>>c)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout<<"Invalid input.\n"; continue; }
        if(c==0) { cout<<"Logging out.\n"; return; }
        if(c==1) checkBal(idx);
        else if(c==2) withdraw(idx);
        else if(c==3) deposit(idx);
        else cout<<"Invalid option.\n";
    }
}

void adminMenu(){
    while(true){
        int c;
        cout<<"1.Add Account 2.Edit Account 3.Add/Refill Cash 4.Add Rate 5.List Accounts 0.Exit\nChoice: ";
        if(!(cin>>c)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout<<"Invalid input.\n"; continue; }
        if(c==0){ cout<<"Exiting admin menu.\n"; return; }
        if(c==1) adminAddAcc();
        else if(c==2) adminEditAcc();
        else if(c==3) adminCash();
        else if(c==4) adminAddRate();
        else if(c==5){
            for(int i=0;i<accCnt;i++){
                cout<<ACC[i].number<<" | "<<ACC[i].name<<" | "<<ACC[i].balance<<" "<<ACC[i].currency<<" | "<<(ACC[i].locked?"LOCKED":"OK")<<"\n";
            }
        } else cout<<"Invalid option.\n";
    }
}

void atmSystem(){
    while(true){
        int c;
        cout<<"\nATM SYSTEM - Choose an option\n1.User Login 2.Admin Login 3.Load CSV (Manual) 0.Exit\nChoice: ";
        if(!(cin>>c)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout<<"Invalid input.\n"; continue; }
        if(c==0){ cout<<"Exiting ATM.\n"; return; }
        if(c==3){ loadAll(); continue; }
        if(!dataLoaded){ cout<<"⚠ CSV FILES NOT LOADED. Please press 3 to load CSV files (manual load).\n"; continue; }
        if(c==1){
            int idx = login();
            if(idx != -1) userMenu(idx);
        } else if(c==2){
            if(adminLogin()) adminMenu();
        } else cout<<"Invalid choice.\n";
    }
}
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
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <ctime>
using namespace std;

// ---------- LIMITS ----------
#define PHARMACY_MAX_MEDICINES 2000
#define PHARMACY_MAX_SUPPLIERS 500
#define PHARMACY_MAX_SALES 5000

// ---------- STRUCTS ----------
struct PharmacyMedicine
{
    int id;
    char name[50];
    char category[30];
    int stock;
    double price;
    char expiry[15];
};

struct PharmacySupplier
{
    int id;
    char name[50];
    char contact[30];
};

struct PharmacySale
{
    int saleID;
    int medID;
    int qty;
    double total;
    char date[15];
};

// ---------- GLOBAL STORAGE ----------
PharmacyMedicine pharmacyMeds[PHARMACY_MAX_MEDICINES];
PharmacySupplier pharmacySuppliers[PHARMACY_MAX_SUPPLIERS];
PharmacySale pharmacySales[PHARMACY_MAX_SALES];

int pharmacyMedCount = 0;
int pharmacySupplierCount = 0;
int pharmacySaleCount = 0;

// ---------- CSV SPLIT ----------
int pharmacySplitCSV(const string &line, string out[], int maxCols)
{
    string temp;
    int count = 0;
    stringstream ss(line);
    while (getline(ss, temp, ',') && count < maxCols)
        out[count++] = temp;
    return count;
}

// ---------- LOAD CSV ----------
void pharmacyLoadMedicinesCSV(const string &file)
{
    ifstream fin(file);
    if (!fin)
    {
        cout << "Cannot open " << file << "\n";
        return;
    }

    string line, col[6];
    getline(fin, line);
    while (getline(fin, line))
    {
        if (pharmacyMedCount >= PHARMACY_MAX_MEDICINES)
            break;
        int n = pharmacySplitCSV(line, col, 6);
        if (n < 6)
            continue;

        PharmacyMedicine &m = pharmacyMeds[pharmacyMedCount++];
        m.id = stoi(col[0]);
        strncpy(m.name, col[1].c_str(), 49);
        strncpy(m.category, col[2].c_str(), 29);
        m.stock = stoi(col[3]);
        m.price = stod(col[4]);
        strncpy(m.expiry, col[5].c_str(), 14);
    }
    fin.close();
    cout << "Loaded Medicines: " << pharmacyMedCount << "\n";
}

// ---------- UTIL ----------
int pharmacyFindMedicine(int id)
{
    for (int i = 0; i < pharmacyMedCount; i++)
        if (pharmacyMeds[i].id == id)
            return i;
    return -1;
}

// ---------- MANUAL ADD MEDICINE ----------
void pharmacyAddMedicineManual()
{
    if (pharmacyMedCount >= PHARMACY_MAX_MEDICINES)
    {
        cout << "Medicine storage full\n";
        return;
    }

    PharmacyMedicine &m = pharmacyMeds[pharmacyMedCount++];

    cout << "Medicine ID: ";
    cin >> m.id;

    cin.ignore();
    cout << "Medicine Name: ";
    cin.getline(m.name, 50);

    cout << "Category: ";
    cin.getline(m.category, 30);

    cout << "Stock Quantity: ";
    cin >> m.stock;

    cout << "Price per unit: ";
    cin >> m.price;

    cout << "Expiry (MM/YYYY): ";
    cin >> m.expiry;

    cout << "Medicine added successfully\n";
}

// ---------- LIST ----------
void pharmacyListMedicines()
{
    cout << "\nID   Name            Stock   Price\n";
    for (int i = 0; i < pharmacyMedCount; i++)
    {
        PharmacyMedicine &m = pharmacyMeds[i];
        cout << m.id << "  " << m.name
             << "  " << m.stock
             << "  " << m.price << "\n";
    }
}

// ---------- MANUAL SALE ----------
void pharmacyManualSale()
{
    int id, qty;
    cout << "Medicine ID: ";
    cin >> id;

    int idx = pharmacyFindMedicine(id);
    if (idx == -1)
    {
        cout << "Medicine not found\n";
        return;
    }

    cout << "Quantity to sell: ";
    cin >> qty;

    PharmacyMedicine &m = pharmacyMeds[idx];
    if (qty <= 0 || qty > m.stock)
    {
        cout << "Invalid quantity / insufficient stock\n";
        return;
    }

    m.stock -= qty;

    PharmacySale &s = pharmacySales[pharmacySaleCount++];
    s.saleID = pharmacySaleCount;
    s.medID = id;
    s.qty = qty;
    s.total = qty * m.price;
    strcpy(s.date, "2025");

    cout << "Sale completed | Bill: " << s.total << "\n";
}

// ---------- AUTO SIMULATE SALES ----------
void pharmacyAutoSimulateSales() {
    if (pharmacyMedCount == 0) {
        cout << "No medicines available\n";
        return;
    }

    srand(time(NULL));

    // Random number of customers: 5 to 25
    int customers = (rand() % 21) + 5;

    cout << "\n--- AUTO PURCHASE SIMULATION ---\n";
    cout << "Customers today: " << customers << "\n\n";

    int completed = 0;

    for (int i = 0; i < customers && pharmacySaleCount < PHARMACY_MAX_SALES; i++) {

        // Try limited attempts to find in-stock medicine
        int attempts = 0;
        int idx = -1;

        while (attempts < pharmacyMedCount) {
            int r = rand() % pharmacyMedCount;
            if (pharmacyMeds[r].stock > 0) {
                idx = r;
                break;
            }
            attempts++;
        }

        if (idx == -1) {
            cout << "All medicines out of stock\n";
            break;
        }

        PharmacyMedicine &m = pharmacyMeds[idx];

        int qty = (rand() % 5) + 1;  // 1–5 units
        if (qty > m.stock) qty = m.stock;

        m.stock -= qty;

        PharmacySale &s = pharmacySales[pharmacySaleCount++];
        s.saleID = pharmacySaleCount;
        s.medID = m.id;
        s.qty = qty;
        s.total = qty * m.price;
        strcpy(s.date, "2025");

        cout << "Customer " << (i + 1)
             << " bought " << qty
             << " x " << m.name
             << " | Unit: " << m.price
             << " | Total: " << s.total << "\n";

        completed++;
    }

    cout << "\nSimulation complete: "
         << completed << " purchases recorded\n";
}



// ---------- REPORT ----------
void pharmacyLowStockReport()
{
    cout << "\n--- LOW STOCK (<10) ---\n";
    for (int i = 0; i < pharmacyMedCount; i++)
        if (pharmacyMeds[i].stock < 10)
            cout << pharmacyMeds[i].name
                 << " | Stock: " << pharmacyMeds[i].stock << "\n";
}
void pharmacyLoadSuppliersCSV(const string &file)
{
    ifstream fin(file);
    if (!fin)
    {
        cout << "Cannot open " << file << "\n";
        return;
    }

    string line, col[3];
    getline(fin, line); // header

    while (getline(fin, line))
    {
        if (pharmacySupplierCount >= PHARMACY_MAX_SUPPLIERS)
            break;
        int n = pharmacySplitCSV(line, col, 3);
        if (n < 3)
            continue;

        PharmacySupplier &s = pharmacySuppliers[pharmacySupplierCount++];
        s.id = stoi(col[0]);
        strncpy(s.name, col[1].c_str(), 49);
        strncpy(s.contact, col[2].c_str(), 29);
    }
    fin.close();

    cout << "Loaded Suppliers: " << pharmacySupplierCount << "\n";
}
void pharmacyListSuppliers()
{
    cout << "\nID   Supplier Name        Contact\n";
    for (int i = 0; i < pharmacySupplierCount; i++)
    {
        cout << pharmacySuppliers[i].id << "   "
             << pharmacySuppliers[i].name << "   "
             << pharmacySuppliers[i].contact << "\n";
    }
}

// ---------- LOAD ALL ----------
void pharmacyLoadAll()
{
    pharmacyLoadMedicinesCSV("pharmacy_medicines.csv");
    pharmacyLoadSuppliersCSV("pharmacy_suppliers.csv");
}

// ---------- MENU ----------
void pharmacySystem()
{
    int ch, n;
    while (1)
    {
        cout << "\n==== PHARMACY SYSTEM ====\n";
        cout << "1. Load ALL CSVs\n";
        cout << "2. Add Medicine (Manual)\n";
        cout << "3. List Medicines\n";
        cout << "4. List Suppliers\n";
        cout << "5. Manual Sale\n";
        cout << "6. Auto Simulate Sales\n";
        cout << "7. Low Stock Report\n";
        cout << "0. Return\n";
        cout << "Choice: ";

        cin >> ch;          // ✅ THIS WAS MISSING

        switch (ch)
        {
        case 1:
            pharmacyLoadAll();
            break;
        case 2:
            pharmacyAddMedicineManual();
            break;
        case 3:
            pharmacyListMedicines();
            break;
        case 4:
            pharmacyListSuppliers();
            break;
        case 5:
            pharmacyManualSale();
            break;
        case 6:
            pharmacyAutoSimulateSales();
            break;
        case 7:
            pharmacyLowStockReport();
            break;
        case 0:
            cout << "Returning...\n";
            return;
        default:
            cout << "Invalid choice! Try again.\n";
        }
    }
}
#define FP_MAX_VENDORS     1000
#define FP_MAX_ITEMS       5000
#define FP_MAX_ORDERS      5000
#define FP_MAX_ORDERITEMS  20000
#define FP_MAX_NODES       50
#define FP_INF             999999

// ---------------- STRUCTS ----------------
struct FPVendor {
    int id;
    char name[50];
    char type[10];
};

struct FPItem {
    int id;
    int vendorId;
    char name[50];
    double price;
};

struct FPOrder {
    int id;
    char customer[50];
    char address[100];
    int locationNode;
    int distance;
    char status[20]; // PLACED / COMPLETED
};

struct FPOrderItem {
    int orderId;
    int itemId;
    int qty;
};

// ---------------- GLOBAL STORAGE ----------------
FPVendor vendors[FP_MAX_VENDORS]; int vendorCount = 0;
FPItem items[FP_MAX_ITEMS]; int itemCount = 0;
FPOrder orders[FP_MAX_ORDERS]; int orderCount = 0;
FPOrderItem orderItems[FP_MAX_ORDERITEMS]; int orderItemCount = 0;
int foodPharmaDijkstra(int src, int dest);


// ---------------- GRAPH ----------------
int nodeCount = 0;
int graph[FP_MAX_NODES][FP_MAX_NODES];
int dist[FP_MAX_NODES], visited[FP_MAX_NODES];

// ---------------- CSV HELPER ----------------
int foodPharmaSplitCSV(const string &line, string out[], int max) {
    stringstream ss(line); string cell; int i = 0;
    while (getline(ss, cell, ',') && i < max) out[i++] = cell;
    return i;
}

// ---------------- GRAPH LOADER ----------------
bool foodPharmaLoadGraph(const string &file) {
    ifstream fin(file);
    if (!fin) return false;

    fin >> nodeCount;
    for (int i = 0; i < nodeCount; i++)
        for (int j = 0; j < nodeCount; j++)
            graph[i][j] = (i == j ? 0 : FP_INF);

    string line, c[4];
    getline(fin, line);
    while (getline(fin, line)) {
        foodPharmaSplitCSV(line, c, 4);
        int u = stoi(c[0]), v = stoi(c[1]), w = stoi(c[2]);
        graph[u][v] = graph[v][u] = w;
    }
    return true;
}
bool foodPharmaLoadOrders(const string &file) {
    ifstream fin(file);
    if (!fin) return false;

    string line, c[6];
    orderCount = 0;
    getline(fin, line);

    while (getline(fin, line)) {
        foodPharmaSplitCSV(line, c, 6);
        orders[orderCount].id = stoi(c[0]);
        strncpy(orders[orderCount].customer, c[1].c_str(), 49);
        strncpy(orders[orderCount].address, c[2].c_str(), 99);
        orders[orderCount].locationNode = stoi(c[3]);
        strncpy(orders[orderCount].status, c[4].c_str(), 19);
        orders[orderCount].distance =
            foodPharmaDijkstra(0, orders[orderCount].locationNode);
        orderCount++;
    }
    return true;
}
bool foodPharmaLoadOrderItems(const string &file) {
    ifstream fin(file);
    if (!fin) return false;

    string line, c[4];
    orderItemCount = 0;
    getline(fin, line);

    while (getline(fin, line)) {
        foodPharmaSplitCSV(line, c, 4);
        orderItems[orderItemCount].orderId = stoi(c[0]);
        orderItems[orderItemCount].itemId = stoi(c[1]);
        orderItems[orderItemCount].qty = stoi(c[2]);
        orderItemCount++;
    }
    return true;
}


// ---------------- DIJKSTRA ----------------
int foodPharmaDijkstra(int src, int dest) {
    for (int i = 0; i < nodeCount; i++) {
        dist[i] = FP_INF; visited[i] = 0;
    }
    dist[src] = 0;

    for (int i = 0; i < nodeCount - 1; i++) {
        int u = -1, mn = FP_INF;
        for (int j = 0; j < nodeCount; j++)
            if (!visited[j] && dist[j] < mn)
                mn = dist[j], u = j;

        if (u == -1) break;
        visited[u] = 1;

        for (int v = 0; v < nodeCount; v++)
            if (!visited[v] && graph[u][v] != FP_INF &&
                dist[u] + graph[u][v] < dist[v])
                dist[v] = dist[u] + graph[u][v];
    }
    return dist[dest];
}

// ---------------- LOADERS ----------------
bool foodPharmaLoadVendors(const string &file) {
    ifstream fin(file);
    if (!fin) return false;

    string line, c[5];
    vendorCount = 0;
    getline(fin, line);

    while (getline(fin, line)) {
        foodPharmaSplitCSV(line, c, 5);
        vendors[vendorCount].id = stoi(c[0]);
        strncpy(vendors[vendorCount].name, c[1].c_str(), 49);
        strncpy(vendors[vendorCount].type, c[2].c_str(), 9);
        vendorCount++;
    }
    return true;
}


bool foodPharmaLoadItems(const string &file) {
    ifstream fin(file);
    if (!fin) return false;

    string line, c[6];
    itemCount = 0;
    getline(fin, line);

    while (getline(fin, line)) {
        foodPharmaSplitCSV(line, c, 6);
        items[itemCount].id = stoi(c[0]);
        items[itemCount].vendorId = stoi(c[1]);
        strncpy(items[itemCount].name, c[2].c_str(), 49);
        items[itemCount].price = stod(c[3]);
        itemCount++;
    }
    return true;
}


void foodPharmaLoadALL() {
    cout << "\n--- LOADING FOODPHARMA CSV DATA ---\n";

    if (foodPharmaLoadVendors("foodpharma_vendors.csv"))
        cout << " Vendors loaded: " << vendorCount << "\n";
    else
        cout << " Vendors CSV missing\n";

    if (foodPharmaLoadItems("foodpharma_items.csv"))
        cout << " Items loaded: " << itemCount << "\n";
    else
        cout << " Items CSV missing\n";

    if (foodPharmaLoadGraph("foodpharma_graph.csv"))
        cout << " Graph loaded: " << nodeCount << " nodes\n";
    else
        cout << " Graph CSV missing\n";

    if (foodPharmaLoadOrders("foodpharma_orders.csv"))
        cout << " Orders loaded: " << orderCount << "\n";
    else
        cout << " Orders CSV not found (optional)\n";

    if (foodPharmaLoadOrderItems("foodpharma_orderitems.csv"))
        cout << " Order items loaded: " << orderItemCount << "\n";
    else
        cout << " Order items CSV not found (optional)\n";

    cout << "--- LOAD COMPLETE ---\n";
}


// ---------------- SHOW FUNCTIONS ----------------
void foodPharmaShowVendors() {
    cout << "\n--- VENDORS ---\n";
    for (int i = 0; i < vendorCount; i++)
        cout << vendors[i].id << " | " << vendors[i].name
             << " | " << vendors[i].type << "\n";
}

void foodPharmaShowItems() {
    cout << "\n--- ITEMS ---\n";
    for (int i = 0; i < itemCount; i++)
        cout << items[i].id << " | Vendor " << items[i].vendorId
             << " | " << items[i].name
             << " | Rs" << items[i].price << "\n";
}

// ---------------- MANUAL ADD ----------------
void foodPharmaAddVendor() {
    FPVendor &v = vendors[vendorCount];
    cout << "Vendor ID: "; cin >> v.id;
    cin.ignore();
    cout << "Name: "; cin.getline(v.name, 50);
    cout << "Type (FOOD/PHARMA): "; cin.getline(v.type, 10);
    vendorCount++;
}

void foodPharmaAddItem() {
    FPItem &i = items[itemCount];
    cout << "Item ID: "; cin >> i.id;
    cout << "Vendor ID: "; cin >> i.vendorId;
    cin.ignore();
    cout << "Name: "; cin.getline(i.name, 50);
    cout << "Price: "; cin >> i.price;
    itemCount++;
}

// ---------------- ORDER FUNCTIONS ----------------
void foodPharmaCreateOrder() {
    FPOrder &o = orders[orderCount];
    o.id = orderCount + 1;

    cin.ignore();
    cout << "Customer Name: "; cin.getline(o.customer, 50);
    cout << "Address: "; cin.getline(o.address, 100);
    cout << "Delivery Location Node: "; cin >> o.locationNode;

    o.distance = foodPharmaDijkstra(0, o.locationNode);
    strcpy(o.status, "PLACED");

    int more = 1;
    while (more) {
        FPOrderItem &oi = orderItems[orderItemCount];
        oi.orderId = o.id;
        cout << "Item ID: "; cin >> oi.itemId;
        cout << "Qty: "; cin >> oi.qty;
        orderItemCount++;
        cout << "Add more items? (1/0): "; cin >> more;
    }
    orderCount++;
}

void foodPharmaCompleteOrder() {
    int oid;
    cout << "Enter Order ID to complete: ";
    cin >> oid;

    for (int i = 0; i < orderCount; i++) {
        if (orders[i].id == oid) {
            strcpy(orders[i].status, "COMPLETED");
            cout << "Order marked COMPLETED\n";
            return;
        }
    }
    cout << "Order not found\n";
}

void foodPharmaRemoveCompletedOrders() {
    for (int i = 0; i < orderCount; i++) {
        if (strcmp(orders[i].status, "COMPLETED") == 0) {
            // remove order items
            for (int j = 0; j < orderItemCount; j++) {
                if (orderItems[j].orderId == orders[i].id) {
                    orderItems[j] = orderItems[--orderItemCount];
                    j--;
                }
            }
            orders[i] = orders[--orderCount];
            i--;
        }
    }
    cout << "Completed orders removed\n";
}

// ---------------- DISPLAY ORDERS ----------------
void foodPharmaShowOrders() {
    for (int i = 0; i < orderCount; i++)
        cout << orders[i].id << " | " << orders[i].customer
             << " | " << orders[i].status
             << " | Dist: " << orders[i].distance << "\n";
}

// ---------------- MENU ----------------
void foodpharmaSystem() {
    int ch; string path;
    do {
        cout << "\n--- FOOD & PHARMACY DELIVERY ---\n";
        cout << "1. Load ALL CSVs\n";
        cout << "2. Show Vendors\n";
        cout << "3. Show Items\n";
        cout << "4. Add Vendor\n";
        cout << "5. Add Item\n";
        cout << "6. Create Order\n";
        cout << "7. Complete Order\n";
        cout << "8. Remove Completed Orders\n";
        cout << "9. Show Orders\n";
        cout << "0. Return\nChoice: ";
        cin >> ch;

        switch (ch) {
            case 1: foodPharmaLoadALL(); break;
            case 2: foodPharmaShowVendors(); break;
            case 3: foodPharmaShowItems(); break;
            case 4: foodPharmaAddVendor(); break;
            case 5: foodPharmaAddItem(); break;
            case 6: foodPharmaCreateOrder(); break;
            case 7: foodPharmaCompleteOrder(); break;
            case 8: foodPharmaRemoveCompletedOrders(); break;
            case 9: foodPharmaShowOrders(); break;
        }
    } while (ch != 0);
}
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

// ---------------- CONFIG ----------------
#define SPA_MAX_CUSTOMERS     2000
#define SPA_MAX_STAFF         500
#define SPA_MAX_SERVICES      500
#define SPA_MAX_APPOINTMENTS  3000
#define SPA_MAX_BILLS         3000

// ---------------- STRUCTS ----------------
struct SpaCustomer {
    int id;
    string name;
    string phone;
};

struct SpaStaff {
    int id;
    string name;
    string role;       // Therapist / Receptionist / Manager
    string specialty;  // For therapists
    int available;     // 1 = free, 0 = busy
};

struct SpaService {
    int id;
    string name;
    int duration;
    double price;
};

struct SpaAppointment {
    int id;
    int customerId;
    int serviceId;
    int staffId;
    string timeSlot;
    string status; // Booked / InProgress / Completed
};

struct SpaBill {
    int id;
    int appointmentId;
    double amount;
    string status; // Pending / Paid
};

// ---------------- STORAGE ----------------
SpaCustomer customers[SPA_MAX_CUSTOMERS];
SpaStaff staff[SPA_MAX_STAFF];
SpaService services[SPA_MAX_SERVICES];
SpaAppointment appointments[SPA_MAX_APPOINTMENTS];
SpaBill bills[SPA_MAX_BILLS];

int customerCount = 0;
int spastaffCount = 0;
int spaserviceCount = 0;
int appointmentCount = 0;
int billCount = 0;

// ---------------- CSV HELPER ----------------
int spaSplitCSV(const string &line, string out[], int maxCols) {
    stringstream ss(line);
    string item;
    int i = 0;
    while (getline(ss, item, ',') && i < maxCols)
        out[i++] = item;
    return i;
}

// ---------------- LOAD CSVs ----------------
void spaLoadCustomers() {
    ifstream f("spa_customers.csv");
    if (!f) return;
    string line, c[4];
    getline(f, line);
    while (getline(f, line) && customerCount < SPA_MAX_CUSTOMERS) {
        spaSplitCSV(line, c, 4);
        customers[customerCount++] = {stoi(c[0]), c[1], c[2]};
    }
}

void spaLoadStaff() {
    ifstream f("spa_staff.csv");
    if (!f) return;
    string line, c[6];
    getline(f, line);
    while (getline(f, line) && spastaffCount < SPA_MAX_STAFF) {
        spaSplitCSV(line, c, 6);
        staff[spastaffCount++] = {
            stoi(c[0]), c[1], c[2], c[3], stoi(c[4])
        };
    }
}

void spaLoadServices() {
    ifstream f("spa_services.csv");
    if (!f) return;
    string line, c[5];
    getline(f, line);
    while (getline(f, line) &&  spaserviceCount < SPA_MAX_SERVICES) {
        spaSplitCSV(line, c, 5);
        services[ spaserviceCount++] = {
            stoi(c[0]), c[1], stoi(c[2]), stod(c[3])
        };
    }
}

void spaLoadAppointments() {
    ifstream f("spa_appointments.csv");
    if (!f) return;
    string line, c[6];
    getline(f, line);
    while (getline(f, line) && appointmentCount < SPA_MAX_APPOINTMENTS) {
        spaSplitCSV(line, c, 6);
        appointments[appointmentCount++] = {
            stoi(c[0]), stoi(c[1]), stoi(c[2]),
            stoi(c[3]), c[4], c[5]
        };
    }
}
string spaGetCustomerName(int id) {
    for (int i = 0; i < customerCount; i++)
        if (customers[i].id == id)
            return customers[i].name;
    return "Unknown";
}

string spaGetStaffName(int id) {
    for (int i = 0; i < spastaffCount; i++)
        if (staff[i].id == id)
            return staff[i].name;
    return "Unassigned";
}

string spaGetServiceName(int id) {
    for (int i = 0; i <  spaserviceCount; i++)
        if (services[i].id == id)
            return services[i].name;
    return "Unknown";
}


void spaLoadAll() {
    spaLoadCustomers();
    spaLoadStaff();
    spaLoadServices();
    spaLoadAppointments();

    cout << "\nLoaded:\n";
    cout << customerCount << " customers\n";
    cout << spastaffCount << " staff\n";
    cout <<  spaserviceCount << " services\n";
    cout << appointmentCount << " appointments\n";
}

// ---------------- STAFF MANAGEMENT ----------------
void spaListStaff() {
    cout << "\n--- STAFF LIST ---\n";
    for (int i = 0; i < spastaffCount; i++) {
        cout << "ID:" << staff[i].id
             << " | " << staff[i].name
             << " | Role:" << staff[i].role
             << " | Spec:" << staff[i].specialty
             << " | " << (staff[i].available ? "Available" : "Busy") << "\n";
    }
}

void spaAddStaff() {
    if (spastaffCount >= SPA_MAX_STAFF) return;

    SpaStaff &s = staff[spastaffCount];
    s.id = spastaffCount + 1;

    cout << "Name: ";
    cin.ignore();
    getline(cin, s.name);
    cout << "Role: ";
    getline(cin, s.role);
    cout << "Specialty: ";
    getline(cin, s.specialty);

    s.available = 1;
    spastaffCount++;

    cout << "Staff added\n";
}

void spaRemoveStaff() {
    int id;
    cout << "Staff ID to remove: ";
    cin >> id;

    for (int i = 0; i < spastaffCount; i++) {
        if (staff[i].id == id) {
            staff[i] = staff[spastaffCount- 1];
            spastaffCount--;
            cout << "Staff removed\n";
            return;
        }
    }
    cout << "Staff not found\n";
}

// ---------------- APPOINTMENTS ----------------
void spaBookAppointment() {
    SpaAppointment &a = appointments[appointmentCount];
    a.id = appointmentCount + 1;

    cout << "Customer ID: ";
    cin >> a.customerId;
    cout << "Service ID: ";
    cin >> a.serviceId;
    cout << "Time Slot: ";
    cin >> a.timeSlot;

    a.staffId = -1;
    a.status = "Booked";
    appointmentCount++;

    cout << "Appointment booked\n";
}

void spaAssignStaff() {
    int aid;
    cout << "Appointment ID: ";
    cin >> aid;

    for (int i = 0; i < appointmentCount; i++) {
        if (appointments[i].id == aid &&
            appointments[i].status == "Booked") {

            for (int s = 0; s < spastaffCount; s++) {
                if (staff[s].available &&
                    staff[s].role == "Therapist") {

                    appointments[i].staffId = staff[s].id;
                    appointments[i].status = "InProgress";
                    staff[s].available = 0;

                    cout << "Assigning "
                         << spaGetStaffName(staff[s].id)
                         << " to "
                         << spaGetCustomerName(appointments[i].customerId)
                         << "\n";

                    return;
                }
            }
        }
    }
    cout << "No therapist available\n";
}


void spaCompleteAppointment() {
    int aid;
    cout << "Appointment ID: ";
    cin >> aid;

    for (int i = 0; i < appointmentCount; i++) {
        if (appointments[i].id == aid &&
            appointments[i].status == "InProgress") {

            appointments[i].status = "Completed";

            for (int s = 0; s < spastaffCount; s++)
                if (staff[s].id == appointments[i].staffId)
                    staff[s].available = 1;

            SpaBill &b = bills[billCount];
            b.id = billCount + 1;
            b.appointmentId = aid;
            b.status = "Pending";

            for (int j = 0; j <  spaserviceCount; j++)
                if (services[j].id == appointments[i].serviceId)
                    b.amount = services[j].price;

            billCount++;
            cout << "Completed & billed\n";
            return;
        }
    }
    cout << "Invalid appointment\n";
}

// ---------------- LIST APPOINTMENTS ----------------
void spaListAppointments() {
    cout << "\n--- APPOINTMENTS ---\n";
    for (int i = 0; i < appointmentCount; i++) {
        cout << "ApptID:" << appointments[i].id
             << " | Customer: " << spaGetCustomerName(appointments[i].customerId)
             << " | Service: " << spaGetServiceName(appointments[i].serviceId)
             << " | Staff: " << spaGetStaffName(appointments[i].staffId)
             << " | [" << appointments[i].status << "]\n";
    }
}


// ---------------- MENU ----------------
void spaMenu() {
    int ch;
    do {
        cout << "\n==== SPA MANAGEMENT ====\n";
        cout << "1. Load ALL CSVs\n";
        cout << "2. List Staff\n";
        cout << "3. Add Staff\n";
        cout << "4. Remove Staff\n";
        cout << "5. Book Appointment\n";
        cout << "6. Assign Therapist\n";
        cout << "7. Complete Appointment & Bill\n";
        cout << "8. List Appointments\n";
        cout << "0. Exit\n";
        cout << "Choice: ";
        cin >> ch;

        switch (ch) {
            case 1: spaLoadAll(); break;
            case 2: spaListStaff(); break;
            case 3: spaAddStaff(); break;
            case 4: spaRemoveStaff(); break;
            case 5: spaBookAppointment(); break;
            case 6: spaAssignStaff(); break;
            case 7: spaCompleteAppointment(); break;
            case 8: spaListAppointments(); break;
        }
    } while (ch != 0);
}
void mallSystem();
void hotelSystem();
void theatreSystem();
void hospitalSystem();
void communitySystem();
void grocerySystem();
void atmSystem();
void pharmacySystem();
void foodpharmaSystem();
void spaMenu();

/* ========= MAIN MENU ========= */
int main() {
    int choice;

    do {
        cout << "\n=========================================\n";
        cout << "        CAMPUS / MALL MANAGEMENT SYSTEM\n";
        cout << "=========================================\n";
        cout << "1. Mall Management System\n";
        cout << "2. Hotel Management System\n";
        cout << "3. Theatre Management System\n";
        cout << "4. Hospital Management System\n";
        cout << "5. Community Centre Management\n";
        cout << "6. Grocery Store Management\n";
        cout << "7. ATM Management System\n";
        cout << "8. Pharmacy Delivery Management\n";
        cout << "9. Food & Pharmacy Delivery System\n";
        cout << "10. Spa Management System\n";
        cout << "0. Exit\n";
        cout << "=========================================\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1: mallSystem(); break;
            case 2: hotelSystem(); break;
            case 3: theatreSystem(); break;
            case 4: hospitalSystem(); break;
            case 5: communitySystem(); break;
            case 6: grocerySystem(); break;
            case 7: atmSystem(); break;
            case 8: pharmacySystem(); break;
            case 9: foodpharmaSystem(); break;
            case 10: spaMenu(); break;
            case 0:
                cout << "Exiting system. Goodbye!\n";
                break;
            default:
                cout << "Invalid choice. Try again.\n";
        }

    } while (choice != 0);

    return 0;
}
