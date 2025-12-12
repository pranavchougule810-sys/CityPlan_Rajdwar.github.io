#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>
using namespace std;

struct Medicine {
    string medicineID;
    string name;
    double price;
    int quantity;
    string expiryDate;
    string category;
};

class Pharmacy {
private:
    unordered_map<string, Medicine> medicineMap; // ID -> Medicine
    vector<Medicine> sortedMedicines; // For binary search
    double totalRevenue;
    int totalSales;
    
public:
    Pharmacy() : totalRevenue(0.0), totalSales(0) {}
    
    void addMedicine(string id, string name, double price, 
                     int quantity, string expiry, string category) {
        Medicine med;
        med.medicineID = id;
        med.name = name;
        med.price = price;
        med.quantity = quantity;
        med.expiryDate = expiry;
        med.category = category;
        
        medicineMap[id] = med;
        sortedMedicines.push_back(med);
    }
    
    void sortMedicinesByPrice() {
        sort(sortedMedicines.begin(), sortedMedicines.end(), 
             [](const Medicine& a, const Medicine& b) {
                 return a.price < b.price;
             });
        cout << "Medicines sorted by price!" << endl;
    }
    
    Medicine* binarySearchByPrice(double targetPrice) {
        int left = 0, right = sortedMedicines.size() - 1;
        Medicine* closest = nullptr;
        double minDiff = 1e9;
        
        while(left <= right) {
            int mid = left + (right - left) / 2;
            double diff = abs(sortedMedicines[mid].price - targetPrice);
            
            if(diff < minDiff) {
                minDiff = diff;
                closest = &sortedMedicines[mid];
            }
            
            if(sortedMedicines[mid].price < targetPrice) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        
        return closest;
    }
    
    bool dispenseMedicine(string id, int quantity) {
        if(medicineMap.find(id) != medicineMap.end()) {
            Medicine& med = medicineMap[id];
            if(med.quantity >= quantity) {
                med.quantity -= quantity;
                double bill = med.price * quantity;
                totalRevenue += bill;
                totalSales++;
                
                cout << "Dispensed: " << med.name << " x" << quantity 
                     << " | Bill: Rs." << fixed << setprecision(2) << bill << endl;
                
                if(med.quantity < 10) {
                    cout << "*** LOW STOCK ALERT: " << med.name << " ***" << endl;
                }
                return true;
            }
        }
        return false;
    }
    
    void displayLowStock() {
        cout << "\n========== Low Stock Medicines ==========\n";
        for(auto& pair : medicineMap) {
            if(pair.second.quantity < 10) {
                cout << pair.second.name << ": " << pair.second.quantity 
                     << " units left" << endl;
            }
        }
    }
    
    void displayStatistics() {
        cout << "\n========== Pharmacy Statistics ==========\n";
        cout << "Total Medicines: " << medicineMap.size() << endl;
        cout << "Total Sales: " << totalSales << endl;
        cout << "Total Revenue: Rs." << fixed << setprecision(2) << totalRevenue << endl;
    }
    
    void loadFromCSV(string filename) {
        ifstream file(filename);
        if(!file.is_open()) {
            cout << "Error: Could not open " << filename << endl;
            return;
        }
        
        string line;
        getline(file, line); // Skip header
        
        while(getline(file, line)) {
            stringstream ss(line);
            string id, name, expiry, category;
            double price;
            int quantity;
            
            getline(ss, id, ',');
            getline(ss, name, ',');
            ss >> price;
            ss.ignore();
            ss >> quantity;
            ss.ignore();
            getline(ss, expiry, ',');
            getline(ss, category, ',');
            
            addMedicine(id, name, price, quantity, expiry, category);
        }
        
        file.close();
        cout << "Data loaded successfully from " << filename << endl;
    }
};

int main() {
    Pharmacy pharmacy;
    
    cout << "========== 24/7 Pharmacy System ==========\n";
    cout << "Loading data from case6.csv...\n";
    
    pharmacy.loadFromCSV("case6.csv");
    pharmacy.displayStatistics();
    pharmacy.sortMedicinesByPrice();
    
    // Binary search example
    cout << "\nSearching for medicine around Rs.50:" << endl;
    Medicine* found = pharmacy.binarySearchByPrice(50.0);
    if(found) {
        cout << "Found: " << found->name << " at Rs." << found->price << endl;
    }
    
    pharmacy.displayLowStock();
    
    return 0;
}
