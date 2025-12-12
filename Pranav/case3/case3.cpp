#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <string>
#include <iomanip>
using namespace std;

struct Order {
    string orderID;
    string customerName;
    string itemName;
    int quantity;
    double price;
    string timestamp;
};

class EcoFoodCourt {
private:
    stack<Order> orderHistory;  // Stack for order tracking
    unordered_map<string, int> inventory;  // Hash map for inventory
    unordered_map<string, double> itemPrices;
    double totalRevenue;
    double totalWaste;
    
public:
    EcoFoodCourt() : totalRevenue(0.0), totalWaste(0.0) {
        initializePrices();
    }
    
    void initializePrices() {
        itemPrices["Salad"] = 150.0;
        itemPrices["Juice"] = 80.0;
        itemPrices["Sandwich"] = 120.0;
        itemPrices["Smoothie"] = 100.0;
        itemPrices["Protein_Bowl"] = 200.0;
    }
    
    void addToInventory(string item, int quantity) {
        inventory[item] += quantity;
    }
    
    bool placeOrder(string orderID, string customerName, string item, 
                    int quantity, string timestamp) {
        if(inventory[item] >= quantity) {
            Order order;
            order.orderID = orderID;
            order.customerName = customerName;
            order.itemName = item;
            order.quantity = quantity;
            order.price = itemPrices[item] * quantity;
            order.timestamp = timestamp;
            
            orderHistory.push(order);
            inventory[item] -= quantity;
            totalRevenue += order.price;
            return true;
        }
        return false;
    }
    
    void viewRecentOrders(int count) {
        cout << "\n========== Recent Orders ==========\n";
        stack<Order> temp = orderHistory;
        int shown = 0;
        
        while(!temp.empty() && shown < count) {
            Order o = temp.top();
            temp.pop();
            cout << "Order: " << o.orderID << " | Customer: " << o.customerName
                 << " | Item: " << o.itemName << " | Qty: " << o.quantity
                 << " | Price: Rs." << fixed << setprecision(2) << o.price << endl;
            shown++;
        }
    }
    
    void undoLastOrder() {
        if(!orderHistory.empty()) {
            Order lastOrder = orderHistory.top();
            orderHistory.pop();
            inventory[lastOrder.itemName] += lastOrder.quantity;
            totalRevenue -= lastOrder.price;
            cout << "Order " << lastOrder.orderID << " cancelled successfully!" << endl;
        } else {
            cout << "No orders to undo!" << endl;
        }
    }
    
    void checkInventory() {
        cout << "\n========== Current Inventory ==========\n";
        for(auto& item : inventory) {
            cout << item.first << ": " << item.second << " units";
            if(item.second < 10) cout << " [LOW STOCK ALERT!]";
            cout << endl;
        }
    }
    
    void trackWaste(string item, double wasteKg) {
        totalWaste += wasteKg;
        cout << "Waste tracked: " << wasteKg << " kg of " << item << endl;
    }
    
    void displayStatistics() {
        cout << "\n========== Food Court Statistics ==========\n";
        cout << "Total Orders: " << orderHistory.size() << endl;
        cout << "Total Revenue: Rs." << fixed << setprecision(2) << totalRevenue << endl;
        cout << "Total Waste: " << totalWaste << " kg" << endl;
        cout << "Avg Order Value: Rs." << (orderHistory.size() > 0 ? 
              totalRevenue / orderHistory.size() : 0) << endl;
    }
    
    void loadFromCSV(string filename) {
        ifstream file(filename);
        if(!file.is_open()) {
            cout << "Error: Could not open " << filename << endl;
            return;
        }
        
        string line;
        getline(file, line); // Skip header
        
        // Initialize inventory
        addToInventory("Salad", 500);
        addToInventory("Juice", 500);
        addToInventory("Sandwich", 500);
        addToInventory("Smoothie", 500);
        addToInventory("Protein_Bowl", 500);
        
        while(getline(file, line)) {
            stringstream ss(line);
            string orderID, customerName, item, timestamp;
            int quantity;
            
            getline(ss, orderID, ',');
            getline(ss, customerName, ',');
            getline(ss, item, ',');
            ss >> quantity;
            ss.ignore();
            getline(ss, timestamp, ',');
            
            placeOrder(orderID, customerName, item, quantity, timestamp);
        }
        
        file.close();
        cout << "Data loaded successfully from " << filename << endl;
    }
};

int main() {
    EcoFoodCourt foodCourt;
    
    cout << "========== Eco-Friendly Food Court System ==========\n";
    cout << "Loading data from case3.csv...\n";
    
    foodCourt.loadFromCSV("case3.csv");
    foodCourt.displayStatistics();
    foodCourt.checkInventory();
    foodCourt.viewRecentOrders(10);
    
    // Track some waste
    foodCourt.trackWaste("Salad", 2.5);
    foodCourt.trackWaste("Juice", 1.2);
    
    return 0;
}
