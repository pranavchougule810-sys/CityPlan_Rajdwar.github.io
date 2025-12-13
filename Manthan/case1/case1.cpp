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

int main() {
    mallSystem();
    return 0;
}
