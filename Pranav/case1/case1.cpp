#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <string>
#include <iomanip>
using namespace std;

// Structure for workspace booking
struct Booking {
    string bookingID;
    string userName;
    string userType;
    string date;
    int duration;
    double price;
    int priority; // 1=urgent, 2=normal, 3=flexible
    
    bool operator<(const Booking& other) const {
        return priority > other.priority; // Min heap based on priority
    }
};

class CoWorkingHub {
private:
    unordered_map<string, Booking> bookingMap; // bookingID -> Booking
    priority_queue<Booking> bookingQueue;
    int totalSpaces;
    int availableSpaces;
    
public:
    CoWorkingHub(int spaces) : totalSpaces(spaces), availableSpaces(spaces) {}
    
    double calculatePrice(string userType, int duration) {
        double basePrice = 100.0; // per hour
        if(userType == "student") return basePrice * duration * 0.5;
        else if(userType == "freelancer") return basePrice * duration * 0.7;
        else if(userType == "startup") return basePrice * duration * 0.8;
        return basePrice * duration;
    }
    
    bool addBooking(string id, string name, string type, string date, 
                    int duration, int priority) {
        if(availableSpaces > 0) {
            Booking b;
            b.bookingID = id;
            b.userName = name;
            b.userType = type;
            b.date = date;
            b.duration = duration;
            b.price = calculatePrice(type, duration);
            b.priority = priority;
            
            bookingMap[id] = b;
            bookingQueue.push(b);
            availableSpaces--;
            return true;
        }
        return false;
    }
    
    void processHighPriorityBookings() {
        cout << "\n========== High Priority Bookings ==========\n";
        int count = 0;
        priority_queue<Booking> tempQueue = bookingQueue;
        
        while(!tempQueue.empty() && count < 10) {
            Booking b = tempQueue.top();
            tempQueue.pop();
            cout << "ID: " << b.bookingID << " | User: " << b.userName 
                 << " | Type: " << b.userType << " | Priority: " << b.priority
                 << " | Price: Rs." << fixed << setprecision(2) << b.price << endl;
            count++;
        }
    }
    
    void searchBooking(string id) {
        if(bookingMap.find(id) != bookingMap.end()) {
            Booking b = bookingMap[id];
            cout << "\n========== Booking Found ==========\n";
            cout << "Booking ID: " << b.bookingID << endl;
            cout << "User Name: " << b.userName << endl;
            cout << "User Type: " << b.userType << endl;
            cout << "Date: " << b.date << endl;
            cout << "Duration: " << b.duration << " hours" << endl;
            cout << "Price: Rs." << fixed << setprecision(2) << b.price << endl;
            cout << "Priority: " << b.priority << endl;
        } else {
            cout << "Booking not found!" << endl;
        }
    }
    
    void displayStatistics() {
        cout << "\n========== Hub Statistics ==========\n";
        cout << "Total Spaces: " << totalSpaces << endl;
        cout << "Available Spaces: " << availableSpaces << endl;
        cout << "Booked Spaces: " << (totalSpaces - availableSpaces) << endl;
        cout << "Total Bookings: " << bookingMap.size() << endl;
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
            string id, name, type, date;
            int duration, priority;
            
            getline(ss, id, ',');
            getline(ss, name, ',');
            getline(ss, type, ',');
            getline(ss, date, ',');
            ss >> duration;
            ss.ignore();
            ss >> priority;
            
            addBooking(id, name, type, date, duration, priority);
        }
        
        file.close();
        cout << "Data loaded successfully from " << filename << endl;
    }
};

int main() {
    CoWorkingHub hub(500);
    
    cout << "========== Co-Working Hub Management System ==========\n";
    cout << "Loading data from case1.csv...\n";
    
    hub.loadFromCSV("case1.csv");
    hub.displayStatistics();
    hub.processHighPriorityBookings();
    
    // Search example
    cout << "\nSearching for booking B001:" << endl;
    hub.searchBooking("B001");
    
    return 0;
}
