#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>
using namespace std;

struct MenuItem {
    string itemID;
    string name;
    int calories;
    int protein;
    int carbs;
    double price;
    int orderCount;
};

struct Order {
    string orderID;
    string customerName;
    string item;
    string fitnessGoal;
    double totalPrice;
};

class FitnessCafe {
private:
    unordered_map<string, MenuItem> menu;
    vector<Order> orders;
    double totalRevenue;
    
public:
    FitnessCafe() : totalRevenue(0.0) {}
    
    void addMenuItem(string id, string name, int cal, int prot, int carb, double price) {
        MenuItem item;
        item.itemID = id;
        item.name = name;
        item.calories = cal;
        item.protein = prot;
        item.carbs = carb;
        item.price = price;
        item.orderCount = 0;
        menu[id] = item;
    }
    
    void placeOrder(string orderID, string customer, string itemID, string goal) {
        if(menu.find(itemID) != menu.end()) {
            MenuItem& item = menu[itemID];
            item.orderCount++;
            
            Order order;
            order.orderID = orderID;
            order.customerName = customer;
            order.item = item.name;
            order.fitnessGoal = goal;
            order.totalPrice = item.price;
            
            orders.push_back(order);
            totalRevenue += item.price;
            
            // Provide nutrition advice
            if(goal == "weight_loss" && item.calories > 300) {
                cout << "💡 Tip: High calories for weight loss goal!" << endl;
            } else if(goal == "muscle_gain" && item.protein < 20) {
                cout << "💡 Tip: Consider adding extra protein!" << endl;
            }
        }
    }
    
    void sortMenuByPopularity() {
        vector<MenuItem> sortedMenu;
        for(auto& pair : menu) {
            sortedMenu.push_back(pair.second);
        }
        
        sort(sortedMenu.begin(), sortedMenu.end(), 
             [](const MenuItem& a, const MenuItem& b) {
                 return a.orderCount > b.orderCount;
             });
        
        cout << "\n========== Top 10 Popular Items ==========\n";
        for(int i = 0; i < min(10, (int)sortedMenu.size()); i++) {
            cout << (i+1) << ". " << sortedMenu[i].name 
                 << " | Orders: " << sortedMenu[i].orderCount
                 << " | Calories: " << sortedMenu[i].calories
                 << " | Protein: " << sortedMenu[i].protein << "g" << endl;
        }
    }
    
    void sortMenuByCalories() {
        vector<MenuItem> sortedMenu;
        for(auto& pair : menu) {
            sortedMenu.push_back(pair.second);
        }
        
        sort(sortedMenu.begin(), sortedMenu.end(), 
             [](const MenuItem& a, const MenuItem& b) {
                 return a.calories < b.calories;
             });
        
        cout << "\n========== Low Calorie Options ==========\n";
        for(int i = 0; i < min(10, (int)sortedMenu.size()); i++) {
            cout << sortedMenu[i].name << " | " << sortedMenu[i].calories 
                 << " cal | Rs." << sortedMenu[i].price << endl;
        }
    }
    
    void displayStatistics() {
        cout << "\n========== Fitness Café Statistics ==========\n";
        cout << "Menu Items: " << menu.size() << endl;
        cout << "Total Orders: " << orders.size() << endl;
        cout << "Total Revenue: Rs." << fixed << setprecision(2) << totalRevenue << endl;
        cout << "Avg Order Value: Rs." << (orders.size() > 0 ? 
              totalRevenue / orders.size() : 0) << endl;
    }
    
    void loadFromCSV(string filename) {
        ifstream file(filename);
        if(!file.is_open()) {
            cout << "Error: Could not open " << filename << endl;
            return;
        }
        
        string line;
        getline(file, line); // Skip header
        
        // Initialize menu
        addMenuItem("I001", "Protein_Shake", 250, 30, 20, 150.0);
        addMenuItem("I002", "Green_Salad", 120, 5, 15, 100.0);
        addMenuItem("I003", "Smoothie_Bowl", 200, 8, 35, 130.0);
        addMenuItem("I004", "Chicken_Wrap", 400, 35, 40, 200.0);
        addMenuItem("I005", "Fruit_Juice", 150, 2, 35, 80.0);
        
        while(getline(file, line)) {
            stringstream ss(line);
            string orderID, customer, itemID, goal;
            
            getline(ss, orderID, ',');
            getline(ss, customer, ',');
            getline(ss, itemID, ',');
            getline(ss, goal, ',');
            
            placeOrder(orderID, customer, itemID, goal);
        }
        
        file.close();
        cout << "Data loaded successfully from " << filename << endl;
    }
};

int main() {
    FitnessCafe cafe;
    
    cout << "========== Fitness Café System ==========\n";
    cout << "Loading data from case8.csv...\n";
    
    cafe.loadFromCSV("case8.csv");
    cafe.displayStatistics();
    cafe.sortMenuByPopularity();
    cafe.sortMenuByCalories();
    
    return 0;
}
