#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <vector>
#include <string>
#include <iomanip>
using namespace std;

struct Booking {
    string bookingID;
    string guestName;
    string roomType;
    string checkIn;
    string checkOut;
    int nights;
    double totalCost;
    int priority; // 1=VIP, 2=Regular, 3=Budget
    
    bool operator<(const Booking& other) const {
        // Greedy: VIP first, then by cost (higher paying first)
        if(priority != other.priority) return priority > other.priority;
        return totalCost < other.totalCost;
    }
};

class EcoGuestHouse {
private:
    priority_queue<Booking> bookingQueue;
    vector<Booking> confirmedBookings;
    int totalRooms;
    int availableRooms;
    double solarEnergyUsage; // percentage
    double totalRevenue;
    
public:
    EcoGuestHouse(int rooms) : totalRooms(rooms), availableRooms(rooms), 
                                solarEnergyUsage(75.0), totalRevenue(0.0) {}
    
    double calculateCost(string roomType, int nights) {
        double basePrice = 1000.0;
        if(roomType == "Deluxe") basePrice = 1500.0;
        else if(roomType == "Suite") basePrice = 2000.0;
        
        double total = basePrice * nights;
        return total * 0.9; // 10% eco discount
    }
    
    void addBooking(string id, string guest, string type, string checkIn, 
                    string checkOut, int nights, int priority) {
        Booking booking;
        booking.bookingID = id;
        booking.guestName = guest;
        booking.roomType = type;
        booking.checkIn = checkIn;
        booking.checkOut = checkOut;
        booking.nights = nights;
        booking.totalCost = calculateCost(type, nights);
        booking.priority = priority;
        
        bookingQueue.push(booking);
    }
    
    void processBookings() {
        cout << "\n========== Processing Bookings (Greedy - VIP First) ==========\n";
        int processed = 0;
        
        while(!bookingQueue.empty() && availableRooms > 0 && processed < 20) {
            Booking b = bookingQueue.top();
            bookingQueue.pop();
            
            confirmedBookings.push_back(b);
            availableRooms--;
            totalRevenue += b.totalCost;
            
            cout << "Confirmed: " << b.bookingID << " | Guest: " << b.guestName
                 << " | Room: " << b.roomType << " | Nights: " << b.nights
                 << " | Cost: Rs." << fixed << setprecision(2) << b.totalCost
                 << " | Priority: " << b.priority << endl;
            processed++;
        }
        
        availableRooms = totalRooms; // Reset
    }
    
    void displayEcoMetrics() {
        cout << "\n========== Eco-Friendly Metrics ==========\n";
        cout << "Solar Energy Usage: " << solarEnergyUsage << "%" << endl;
        cout << "Carbon Footprint Reduction: " << (solarEnergyUsage * 0.5) << " kg CO2/day" << endl;
        cout << "Water Conservation: " << (confirmedBookings.size() * 50) << " liters saved" << endl;
    }
    
    void displayStatistics() {
        cout << "\n========== Guest House Statistics ==========\n";
        cout << "Total Rooms: " << totalRooms << endl;
        cout << "Confirmed Bookings: " << confirmedBookings.size() << endl;
        cout << "Pending Requests: " << bookingQueue.size() << endl;
        cout << "Total Revenue: Rs." << fixed << setprecision(2) << totalRevenue << endl;
        cout << "Occupancy Rate: " << (confirmedBookings.size() * 100.0 / totalRooms) << "%" << endl;
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
            string id, guest, type, checkIn, checkOut;
            int nights, priority;
            
            getline(ss, id, ',');
            getline(ss, guest, ',');
            getline(ss, type, ',');
            getline(ss, checkIn, ',');
            getline(ss, checkOut, ',');
            ss >> nights;
            ss.ignore();
            ss >> priority;
            
            addBooking(id, guest, type, checkIn, checkOut, nights, priority);
        }
        
        file.close();
        cout << "Data loaded successfully from " << filename << endl;
    }
};

int main() {
    EcoGuestHouse guestHouse(50);
    
    cout << "========== Eco-Friendly Guest House System ==========\n";
    cout << "Loading data from case7.csv...\n";
    
    guestHouse.loadFromCSV("case7.csv");
    guestHouse.displayStatistics();
    guestHouse.processBookings();
    guestHouse.displayEcoMetrics();
    
    return 0;
}
