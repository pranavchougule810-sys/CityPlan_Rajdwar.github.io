// ===========================
// PHARMACY MANAGEMENT MODULE
// Prefix: pharmacyXXX
// ===========================

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

int main()
{
    pharmacySystem();
}
