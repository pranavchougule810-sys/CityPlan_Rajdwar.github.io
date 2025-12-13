#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
using namespace std;

// ---------------- CONFIG ----------------
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

int main()
{
    foodpharmaSystem();
}
