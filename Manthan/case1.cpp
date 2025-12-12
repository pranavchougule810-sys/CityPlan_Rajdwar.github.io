
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
using namespace std;
int randint(int a, int b) {
    return a + rand() % (b - a + 1);
}

// Convert HH:MM to minutes
int timeToMin(string t) {
    int hh = stoi(t.substr(0,2));
    int mm = stoi(t.substr(3,2));
    return hh*60 + mm;
}

// Convert minutes back to HH:MM
string minToTime(int m) {
    int hh = m/60;
    int mm = m%60;
    char buf[16];
    sprintf(buf, "%02d:%02d", hh, mm);
    return string(buf);
}

// =====================================================
// SHOP STRUCTURES (arrays only)
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
    ItemNode *root;   // BST root
};

// Max shops
Shop mallShops[50];
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

Staff staffList[50];
int staffCount = 0;

// =====================================================
// CLEANING EVENTS
// =====================================================

struct CleanEvent {
    int time;
    string note;
};

CleanEvent cleanLog[100];
int cleanCount = 0;

// =====================================================
// PARKING (10x10 grid)
// =====================================================

int parkingGrid[10][10];   // 0 empty, 1 obstacle, 2 occupied

// =====================================================
// DIJKSTRA GRAPH (user requested array-based version)
// =====================================================

int costGraph[20][20]; // adjacency matrix
int graphNodes = 0;
/*******************************************************
   PART 2 — BST SYSTEM (BASED ON YOUR COLLEGE CODE)
********************************************************/

class ItemBST {
public:
    ItemNode* insertNode(ItemNode* root, string name, int price, int stock) {
        ItemNode* newnode = new ItemNode;
        newnode->name = name;
        newnode->price = price;
        newnode->stock = stock;
        newnode->left = NULL;
        newnode->right = NULL;

        if (root == NULL) {
            return newnode;
        }

        ItemNode* parent = NULL;
        ItemNode* curr = root;

        while (curr != NULL) {
            parent = curr;
            if (name < curr->name)
                curr = curr->left;
            else
                curr = curr->right;
        }

        if (name < parent->name)
            parent->left = newnode;
        else
            parent->right = newnode;

        return root;
    }

    ItemNode* deleteNode(ItemNode* root, string key) {
        if (!root) return NULL;

        if (key < root->name)
            root->left = deleteNode(root->left, key);
        else if (key > root->name)
            root->right = deleteNode(root->right, key);
        else {
            if (!root->left) {
                ItemNode* temp = root->right;
                delete root;
                return temp;
            }
            else if (!root->right) {
                ItemNode* temp = root->left;
                delete root;
                return temp;
            }
            else {
                ItemNode* succ = root->right;
                while (succ->left) succ = succ->left;

                root->name = succ->name;
                root->price = succ->price;
                root->stock = succ->stock;

                root->right = deleteNode(root->right, succ->name);
            }
        }
        return root;
    }

    void inorder(ItemNode* root) {
        if (!root) return;
        inorder(root->left);
        cout << "   " << root->name
             << "  Price: " << root->price
             << "  Stock: " << root->stock << "\n";
        inorder(root->right);
    }
};

ItemBST itemManager; // global BST handler
/*******************************************************
   PART 3 — QUICKSORT (FOR SHOP REVENUE) + BFS PARKING
********************************************************/

// ----------------------------------------------------
// QUICKSORT (DESCENDING BY REVENUE)
// ----------------------------------------------------

int partitionQuick(int arr[], int l, int r) {
    int pivot = mallShops[arr[l]].revenue;
    int i = l, j = r + 1;

    while (true) {
        do { i++; } while (i <= r && mallShops[arr[i]].revenue > pivot);
        do { j--; } while (j >= l && mallShops[arr[j]].revenue < pivot);
        if (i >= j) break;

        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }

    int temp = arr[l];
    arr[l] = arr[j];
    arr[j] = temp;

    return j;
}

void quickSortShops(int arr[], int l, int r) {
    if (l < r) {
        int s = partitionQuick(arr, l, r);
        quickSortShops(arr, l, s - 1);
        quickSortShops(arr, s + 1, r);
    }
}

// ----------------------------------------------------
// BFS PARKING (Your BFS Code Integrated)
// ----------------------------------------------------

int BFS_nearestParking(int grid[10][10], int startRow, int startCol) {
    int queueR[200], queueC[200];
    int front = 0, rear = 0;

    int visited[10][10] = {0};

    queueR[rear] = startRow;
    queueC[rear] = startCol;
    visited[startRow][startCol] = 1;

    int dr[4] = {-1, 1, 0, 0};
    int dc[4] = {0, 0, -1, 1};

    while (front <= rear) {
        int r = queueR[front];
        int c = queueC[front];
        front++;

        if (grid[r][c] == 0) {
            return r * 10 + c; // encoded position
        }

        for (int i = 0; i < 4; i++) {
            int nr = r + dr[i];
            int nc = c + dc[i];

            if (nr >= 0 && nr < 10 && nc >= 0 && nc < 10) {
                if (!visited[nr][nc] && grid[nr][nc] != 2) {
                    visited[nr][nc] = 1;
                    rear++;
                    queueR[rear] = nr;
                    queueC[rear] = nc;
                }
            }
        }
    }

    return -1; // no available slot
}
/********************************************************
   PART 4 — DIJKSTRA SHORTEST PATH (USER-PROVIDED STYLE)
*********************************************************/

int dijkstra_dist[50];
int dijkstra_path[50];
int dijkstra_visited[50];

/*
    n  = number of nodes
    costGraph[][] = adjacency matrix (we built earlier)
    src = starting node index
*/
void runDijkstra(int n, int src) {
    // Initialization
    for (int i = 0; i < n; i++) {
        dijkstra_dist[i] = costGraph[src][i];
        dijkstra_path[i] = src;
        dijkstra_visited[i] = 0;
    }
    dijkstra_visited[src] = 1;

    // n-1 iterations
    for (int iter = 0; iter < n - 1; iter++) {
        int u = -1;
        int min = 999999;

        // Find smallest unvisited node
        for (int i = 0; i < n; i++) {
            if (!dijkstra_visited[i] && dijkstra_dist[i] < min) {
                min = dijkstra_dist[i];
                u = i;
            }
        }

        if (u == -1)
            return;

        dijkstra_visited[u] = 1;

        // Relax edges
        for (int v = 0; v < n; v++) {
            if (!dijkstra_visited[v] && costGraph[u][v] < 999999) {
                if (dijkstra_dist[u] + costGraph[u][v] < dijkstra_dist[v]) {
                    dijkstra_dist[v] = dijkstra_dist[u] + costGraph[u][v];
                    dijkstra_path[v] = u;
                }
            }
        }
    }
}

// Reconstruct path by walking backwards through dijkstra_path[]
void printDijkstraPath(int dest) {
    int pathArr[50];
    int cnt = 0;

    int cur = dest;
    while (cur != dijkstra_path[cur]) {
        pathArr[cnt++] = cur;
        cur = dijkstra_path[cur];
    }
    pathArr[cnt++] = cur;

    cout << "Shortest evacuation path: ";
    for (int i = cnt - 1; i >= 0; i--) {
        cout << pathArr[i];
        if (i != 0) cout << " -> ";
    }
    cout << "\n";
}
/********************************************************
        PART 5 — MALL CORE FUNCTIONALITY
*********************************************************/

// ------------------------------------------------------
// CREATE A NEW SHOP
// ------------------------------------------------------

int createShopID() {
    static int sid = 100;
    return ++sid;
}

void addShop() {
    cin.ignore();
    string nm, openS, closeS;

    cout << "Enter shop name: ";
    getline(cin, nm);

    cout << "Enter opening time (HH:MM): ";
    cin >> openS;

    cout << "Enter closing time (HH:MM): ";
    cin >> closeS;

    mallShops[shopCount].id = createShopID();
    mallShops[shopCount].name = nm;
    mallShops[shopCount].open_time = timeToMin(openS);
    mallShops[shopCount].close_time = timeToMin(closeS);
    mallShops[shopCount].revenue = 0;
    mallShops[shopCount].isOpen = false;
    mallShops[shopCount].root = NULL;

    cout << "Shop added with ID: " << mallShops[shopCount].id << "\n";

    shopCount++;
}

// ------------------------------------------------------
// ADD ITEM TO A SHOP (BST INSERT)
// ------------------------------------------------------

void addItemToShop() {
    int id;
    cout << "Enter shop ID: ";
    cin >> id;

    int index = -1;
    for (int i = 0; i < shopCount; i++) {
        if (mallShops[i].id == id) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        cout << "Shop not found!\n";
        return;
    }

    cin.ignore();
    string nm;
    int price, stock;

    cout << "Enter item name: ";
    getline(cin, nm);

    cout << "Enter price: ";
    cin >> price;

    cout << "Enter stock: ";
    cin >> stock;

    mallShops[index].root =
        itemManager.insertNode(mallShops[index].root, nm, price, stock);

    cout << "Item added to inventory.\n";
}

// ------------------------------------------------------
// DISPLAY SHOPS + INVENTORY
// ------------------------------------------------------

void showAllShops() {
    if (shopCount == 0) {
        cout << "No shops in the mall.\n";
        return;
    }

    for (int i = 0; i < shopCount; i++) {
        cout << "\n====================================\n";
        cout << "Shop ID: " << mallShops[i].id << "\n";
        cout << "Name: " << mallShops[i].name << "\n";
        cout << "Timing: " << minToTime(mallShops[i].open_time)
             << " - " << minToTime(mallShops[i].close_time) << "\n";
        cout << "Revenue: Rs " << mallShops[i].revenue << "\n";

        cout << "Items:\n";
        if (mallShops[i].root == NULL)
            cout << "   (No Items)\n";
        else
            itemManager.inorder(mallShops[i].root);
    }
}

// ------------------------------------------------------
// SIMULATE CUSTOMER PURCHASE
// ------------------------------------------------------

void simulateCustomer() {
    int id;
    cout << "Enter shop ID to simulate customer: ";
    cin >> id;

    int index = -1;
    for (int i = 0; i < shopCount; i++)
        if (mallShops[i].id == id) index = i;

    if (index == -1) {
        cout << "Shop not found.\n";
        return;
    }

    if (mallShops[index].root == NULL) {
        cout << "Shop has no items!\n";
        return;
    }

    // Pick a random item using inorder-leftmost logic
    ItemNode* curr = mallShops[index].root;
    while (curr->left) curr = curr->left;

    if (curr->stock <= 0) {
        cout << "Item out of stock!\n";
        return;
    }

    int qty = randint(1, 3);
    if (qty > curr->stock) qty = curr->stock;

    curr->stock -= qty;
    int sale = qty * curr->price;
    mallShops[index].revenue += sale;

    cout << "Customer bought " << qty << " unit(s) of "
         << curr->name << " for Rs " << sale << "\n";
}

// ------------------------------------------------------
// SORT SHOPS BY REVENUE (QUICKSORT)
// ------------------------------------------------------

void sortShopsByRevenue() {
    if (shopCount == 0) {
        cout << "No shops to sort.\n";
        return;
    }

    int idx[50];
    for (int i = 0; i < shopCount; i++)
        idx[i] = i;

    quickSortShops(idx, 0, shopCount - 1);

    cout << "\nShops sorted by revenue (high → low):\n";
    for (int i = 0; i < shopCount; i++) {
        cout << mallShops[idx[i]].name
             << "  Rs " << mallShops[idx[i]].revenue << "\n";
    }
}

// ------------------------------------------------------
// PARKING LOT (10x10 BFS)
// ------------------------------------------------------

void parkingMenu() {
    cout << "Enter target row (0-9): ";
    int r; cin >> r;

    cout << "Enter target col (0-9): ";
    int c; cin >> c;

    int result = BFS_nearestParking(parkingGrid, r, c);

    if (result == -1)
        cout << "No parking slot available.\n";
    else {
        int rr = result / 10;
        int cc = result % 10;
        cout << "Nearest empty slot: ("<<rr<<","<<cc<<")\n";
    }
}

// ------------------------------------------------------
// BUILD EMERGENCY GRAPH FOR DIJKSTRA
// ------------------------------------------------------

/*
    For simplicity:
    Node 0..shopCount-1 = shops
    Node shopCount = EXIT 1
    Node shopCount+1 = EXIT 2
*/

void buildEmergencyGraph() {
    graphNodes = shopCount + 2;

    for (int i = 0; i < graphNodes; i++)
        for (int j = 0; j < graphNodes; j++)
            costGraph[i][j] = 999999;

    // connect every shop to both exits
    for (int i = 0; i < shopCount; i++) {
        costGraph[i][shopCount] = randint(3, 10);
        costGraph[shopCount][i] = costGraph[i][shopCount];

        costGraph[i][shopCount+1] = randint(5, 15);
        costGraph[shopCount+1][i] = costGraph[i][shopCount+1];
    }
}

// ------------------------------------------------------
// RUN DIJKSTRA FOR EMERGENCY ROUTE
// ------------------------------------------------------

void emergencyRoute() {
    int id;
    cout << "Enter shop ID: ";
    cin >> id;

    int index = -1;
    for (int i = 0; i < shopCount; i++)
        if (mallShops[i].id == id) index = i;

    if (index == -1) {
        cout << "Shop not found!\n";
        return;
    }

    buildEmergencyGraph();

    runDijkstra(graphNodes, index);

    cout << "\nPaths from shop " << mallShops[index].name << ":\n";

    for (int ex = shopCount; ex < shopCount+2; ex++) {
        cout << "To exit " << (ex-shopCount+1)
             << " distance = " << dijkstra_dist[ex] << "\n";
        printDijkstraPath(ex);
    }
}

// ------------------------------------------------------
// STAFF MANAGEMENT
// ------------------------------------------------------

void addStaff() {
    cin.ignore();

    cout << "Enter staff name: ";
    string nm; getline(cin, nm);

    cout << "Enter role: ";
    string role; getline(cin, role);

    cout << "Enter salary: ";
    int sal; cin >> sal;

    staffList[staffCount].id = staffCount + 1;
    staffList[staffCount].name = nm;
    staffList[staffCount].role = role;
    staffList[staffCount].salary = sal;

    staffCount++;

    cout << "Staff added.\n";
}

void showStaff() {
    cout << "\n--- Staff List ---\n";
    for (int i = 0; i < staffCount; i++) {
        cout << staffList[i].id << ") "
             << staffList[i].name << " - "
             << staffList[i].role << " - Rs "
             << staffList[i].salary << "\n";
    }
}

// ------------------------------------------------------
// CLEANING SCHEDULE
// ------------------------------------------------------

void scheduleCleaner() {
    cin.ignore();
    string t, note;

    cout << "Enter time (HH:MM): ";
    cin >> t;

    cin.ignore();
    cout << "Note: ";
    getline(cin, note);

    cleanLog[cleanCount].time = timeToMin(t);
    cleanLog[cleanCount].note = note;
    cleanCount++;

    cout << "Cleaner scheduled.\n";
}

void showCleanLog() {
    cout << "\n--- Cleaning Log ---\n";
    for (int i = 0; i < cleanCount; i++) {
        cout << minToTime(cleanLog[i].time)
             << " - " << cleanLog[i].note << "\n";
    }
}


/*******************************************************
        PART A — CSV READER + SAFE INTEGER PARSER
********************************************************/

#include <fstream>

// SAFE stoi — NEVER crashes
int toInt(const string &s) {
    try {
        return stoi(s);
    } catch (...) {
        return 0;   // fallback value
    }
}

// Simple CSV splitter
int splitCSV(const string &line, string out[], int maxCols) {
    int col = 0;
    string cur = "";
    for (char c : line) {
        if (c == ',' && col < maxCols - 1) {
            out[col++] = cur;
            cur = "";
        } else {
            cur.push_back(c);
        }
    }
    out[col++] = cur;
    return col;
}


/*******************************************************
            FIXED — LOAD SHOPS CSV
********************************************************/
int loadShopsCSV(const string &filename) {
    ifstream in(filename);
    if (!in.is_open()) {
        cout << "ERROR: Cannot open " << filename << "\n";
        return 0;
    }

    string line;
    int loaded = 0;

    getline(in, line);  // skip header

    while (getline(in, line)) {
        if (line.size() < 3) continue;

        string cols[5];
        int n = splitCSV(line, cols, 5);
        if (n < 4) continue;

        mallShops[shopCount].id = createShopID();
        mallShops[shopCount].name = cols[0];
        mallShops[shopCount].open_time = timeToMin(cols[1]);
        mallShops[shopCount].close_time = timeToMin(cols[2]);
        mallShops[shopCount].revenue = toInt(cols[3]);
        mallShops[shopCount].root = NULL;

        shopCount++;
        loaded++;
    }

    cout << "Loaded " << loaded << " shops from " << filename << "\n";
    return loaded;
}


/*******************************************************
            FIXED — LOAD ITEMS CSV
********************************************************/
int loadItemsCSV(const string &filename) {
    ifstream in(filename);
    if (!in.is_open()) {
        cout << "ERROR: Cannot open " << filename << "\n";
        return 0;
    }

    string line;
    int loaded = 0;

    getline(in, line);  // skip header

    while (getline(in, line)) {
        if (line.size() < 3) continue;

        string cols[5];
        int n = splitCSV(line, cols, 5);
        if (n < 4) continue;

        int shopID = toInt(cols[0]);

        // find shop
        int idx = -1;
        for (int i = 0; i < shopCount; i++)
            if (mallShops[i].id == shopID)
                idx = i;

        if (idx == -1) continue;

        string itemName = cols[1];
        int price = toInt(cols[2]);
        int stock = toInt(cols[3]);

        mallShops[idx].root =
            itemManager.insertNode(mallShops[idx].root, itemName, price, stock);

        loaded++;
    }

    cout << "Loaded " << loaded << " items from " << filename << "\n";
    return loaded;
}


/*******************************************************
            FIXED — LOAD STAFF CSV
********************************************************/
int loadStaffCSV(const string &filename) {
    ifstream in(filename);
    if (!in.is_open()) {
        cout << "ERROR: Cannot open " << filename << "\n";
        return 0;
    }

    string line;
    int loaded = 0;

    getline(in, line);  // skip header

    while (getline(in, line)) {
        if (line.size() < 3) continue;

        string cols[5];
        int n = splitCSV(line, cols, 5);
        if (n < 3) continue;

        staffList[staffCount].id = staffCount + 1;
        staffList[staffCount].name = cols[0];
        staffList[staffCount].role = cols[1];
        staffList[staffCount].salary = toInt(cols[2]);

        staffCount++;
        loaded++;
    }

    cout << "Loaded " << loaded << " staff from " << filename << "\n";
    return loaded;
}
/********************************************************
        PART 2A — STRING SEARCH (KMP ONLY)
*********************************************************/

// Build KMP prefix (LPS) table
vector<int> kmpPrefix(const string &pat) {
    int m = pat.size();
    vector<int> lps(m);
    lps[0] = 0;

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

// KMP search function — returns true if pattern exists inside text
bool kmpSearch(const string &text, const string &pat) {
    if (pat.empty()) return true;
    if (text.size() < pat.size()) return false;

    vector<int> lps = kmpPrefix(pat);
    int i = 0, j = 0;

    while (i < (int)text.size()) {
        if (text[i] == pat[j]) {
            i++; j++;
        } else {
            if (j > 0)
                j = lps[j - 1];
            else
                i++;
        }

        if (j == (int)pat.size())
            return true;  // pattern found
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

// Search for items inside ALL SHOPS using KMP
vector<SearchResult> searchItemInMall(const string &pattern) {
    vector<SearchResult> results;

    string pattLower = pattern;
    transform(pattLower.begin(), pattLower.end(), pattLower.begin(), ::tolower);

    // Loop through all shops
    for (int s = 0; s < shopCount; s++) {

        // Collect all items from the shop's BST inventory
        vector<ItemNode*> items;

        function<void(ItemNode*)> collect = [&](ItemNode* r) {
            if (!r) return;
            collect(r->left);
            items.push_back(r);
            collect(r->right);
        };

        collect(mallShops[s].root);

        // Check each item using KMP
        for (ItemNode* node : items) {
            string itemLower = node->name;
            transform(itemLower.begin(), itemLower.end(), itemLower.begin(), ::tolower);

            if (kmpSearch(itemLower, pattLower)) {
                SearchResult r;
                r.shopID = mallShops[s].id;
                r.shopName = mallShops[s].name;
                r.itemName = node->name;
                r.price = node->price;
                r.stock = node->stock;
                results.push_back(r);
            }
        }
    }

    return results;
}
void showFastestRouteToShop_byIndex(int shopIndex) {
    if (shopIndex < 0 || shopIndex >= shopCount) {
        cout << "Invalid shop index\n";
        return;
    }

    // ensure graph built
    buildEmergencyGraph(); // your function builds costGraph and sets graphNodes

    // run your matrix-based Dijkstra starting from this shop index
    runDijkstra(graphNodes, shopIndex);

    cout << "\n=== Fastest Routes from shop: " << mallShops[shopIndex].name << " ===\n";

    // exits are at indices shopCount and shopCount+1 per your builder
    for (int ex = shopCount; ex < shopCount + 2; ++ex) {
        // dijkstra_dist is global; if unreachable it will still be large number
        cout << "To Exit " << (ex - shopCount + 1)
             << " (node " << ex << ") distance = " << dijkstra_dist[ex] << "\n";
        // print path using your routine which uses dijkstra_path[]
        printDijkstraPath(ex);
    }
    cout << "====================================\n";
}

// menu helper: search items (uses your searchItemInMall function) and then route
void searchAndRoute() {
    string pat;
    cout << "Enter item name to search: ";
    // read a line so multiword names work
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    getline(cin, pat);
    if (pat.empty()) {
        cout << "Empty pattern — aborting.\n";
        return;
    }

    auto results = searchItemInMall(pat); // must exist from Part 2B

    if (results.empty()) {
        cout << "No shops found selling '" << pat << "'.\n";
        return;
    }

    cout << "\nFound " << results.size() << " match(es) for '" << pat << "':\n";
    // show list with shop index (we must convert shopID -> index)
    // build a vector of indices to pick from
    vector<int> indices;
    for (size_t i = 0; i < results.size(); ++i) {
        // find index in mallShops[] for results[i].shopID
        int idx = -1;
        for (int j = 0; j < shopCount; ++j) {
            if (mallShops[j].id == results[i].shopID) { idx = j; break; }
        }
        indices.push_back(idx);
        cout << i+1 << ") Shop: " << results[i].shopName
             << " (Shop ID: " << results[i].shopID << ", index " << idx << ")"
             << " | Item: " << results[i].itemName
             << " | Price: " << results[i].price
             << " | Stock: " << results[i].stock << "\n";
    }

    cout << "\nSelect a result number to route to (1-" << results.size() << "), or 0 to cancel: ";
    int choice = 0;
    if (!(cin >> choice)) {
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cout << "Invalid input. Aborting.\n";
        return;
    }
    if (choice <= 0) {
        cout << "Cancelled.\n";
        return;
    }
    if (choice > (int)indices.size()) {
        cout << "Invalid choice number.\n";
        return;
    }

    int chosenIndex = indices[choice - 1];
    if (chosenIndex == -1) {
        cout << "Selected shop index not found in mall data. Aborting.\n";
        return;
    }

    // now show fastest routes from that shop
    showFastestRouteToShop_byIndex(chosenIndex);
}
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
    cout << " 7. Emergency Route (Dijkstra)\n";
    cout << " 8. Add staff\n";
    cout << " 9. Show staff list\n";
    cout << "10. Schedule cleaner\n";
    cout << "11. Show cleaning log\n";
    cout << "12. Load shops from CSV (shops.csv)\n";
    cout << "13. Load items from CSV (items.csv)\n";
    cout << "14. Load staff from CSV (staff.csv)\n";
    cout << "15. SEARCH + route to shop (KMP + Dijkstra)\n";
    cout << " 0. Exit\n";
    cout << "====================================\n";
    cout << "Enter choice: ";
}

int main() {
    srand(time(NULL));

    cout << "========== MEGA MALL SIMULATOR ==========\n";
    cout << "Algorithms Included:\n";
    cout << "• BST Inventory\n";
    cout << "• QuickSort (Revenue)\n";
    cout << "• BFS Parking\n";
    cout << "• Dijkstra Emergency Routes\n";
    cout << "• KMP Item Search (FAST)\n";
    cout << "• CSV Integration\n";
    cout << "==========================================\n";

    int choice;

    while (true) {
        showMenu();
        cin >> choice;

        switch (choice) {

        case 1:
            showAllShops();
            break;

        case 2:
            addShop();
            break;

        case 3:
            addItemToShop();
            break;

        case 4:
            simulateCustomer();
            break;

        case 5:
            sortShopsByRevenue();
            break;

        case 6:
            parkingMenu();
            break;

        case 7:
            emergencyRoute();
            break;

        case 8:
            addStaff();
            break;

        case 9:
            showStaff();
            break;

        case 10:
            scheduleCleaner();
            break;

        case 11:
            showCleanLog();
            break;

        case 12:
            loadShopsCSV("shops.csv");
            break;

        case 13:
            loadItemsCSV("items.csv");
            break;

        case 14:
            loadStaffCSV("staff.csv");
            break;

        case 15:
            searchAndRoute();   // ← NEW (KMP + Dijkstra)
            break;

        case 0:
            cout << "Exiting Mega Mall Simulator...\n";
            return 0;

        default:
            cout << "Invalid choice! Try again.\n";
        }
    }

    return 0;
}

