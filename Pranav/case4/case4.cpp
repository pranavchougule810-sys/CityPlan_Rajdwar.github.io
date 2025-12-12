#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>
using namespace std;

struct ChargingRequest {
    string vehicleNo;
    string ownerName;
    int batteryCapacity;
    int currentCharge;
    string arrivalTime;
    int priority; // 1=emergency, 2=normal, 3=flexible
    int unitsNeeded;
    double estimatedCost;
    
    bool operator<(const ChargingRequest& other) const {
        // Greedy: Prioritize by emergency, then by units needed (smallest first)
        if(priority != other.priority) return priority > other.priority;
        return unitsNeeded > other.unitsNeeded;
    }
};

class EVChargingStation {
private:
    priority_queue<ChargingRequest> chargingQueue;
    vector<ChargingRequest> completedCharges;
    int totalPorts;
    int availablePorts;
    double costPerKWh;
    double totalRevenue;
    double carbonSaved; // kg CO2
    
public:
    EVChargingStation(int ports, double cost) 
        : totalPorts(ports), availablePorts(ports), 
          costPerKWh(cost), totalRevenue(0.0), carbonSaved(0.0) {}
    
    void addChargingRequest(string vehicleNo, string owner, int capacity, 
                            int current, string time, int priority) {
        ChargingRequest req;
        req.vehicleNo = vehicleNo;
        req.ownerName = owner;
        req.batteryCapacity = capacity;
        req.currentCharge = current;
        req.arrivalTime = time;
        req.priority = priority;
        req.unitsNeeded = capacity - current;
        req.estimatedCost = req.unitsNeeded * costPerKWh;
        
        chargingQueue.push(req);
    }
    
    void processChargingQueue() {
        cout << "\n========== Processing Charging Requests (Greedy) ==========\n";
        int processed = 0;
        
        while(!chargingQueue.empty() && availablePorts > 0 && processed < 20) {
            ChargingRequest req = chargingQueue.top();
            chargingQueue.pop();
            
            cout << "Charging: " << req.vehicleNo << " | Owner: " << req.ownerName
                 << " | Units: " << req.unitsNeeded << " kWh"
                 << " | Priority: " << req.priority
                 << " | Cost: Rs." << fixed << setprecision(2) << req.estimatedCost << endl;
            
            completedCharges.push_back(req);
            totalRevenue += req.estimatedCost;
            carbonSaved += req.unitsNeeded * 0.92; // 0.92 kg CO2 per kWh saved
            availablePorts--;
            processed++;
        }
        
        availablePorts = totalPorts; // Reset for next batch
    }
    
    void displayStatistics() {
        cout << "\n========== Charging Station Statistics ==========\n";
        cout << "Total Ports: " << totalPorts << endl;
        cout << "Vehicles in Queue: " << chargingQueue.size() << endl;
        cout << "Completed Charges: " << completedCharges.size() << endl;
        cout << "Total Revenue: Rs." << fixed << setprecision(2) << totalRevenue << endl;
        cout << "Carbon Saved: " << carbonSaved << " kg CO2" << endl;
        cout << "Avg Charge Cost: Rs." << (completedCharges.size() > 0 ? 
              totalRevenue / completedCharges.size() : 0) << endl;
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
            string vehicleNo, owner, time;
            int capacity, current, priority;
            
            getline(ss, vehicleNo, ',');
            getline(ss, owner, ',');
            ss >> capacity;
            ss.ignore();
            ss >> current;
            ss.ignore();
            getline(ss, time, ',');
            ss >> priority;
            
            addChargingRequest(vehicleNo, owner, capacity, current, time, priority);
        }
        
        file.close();
        cout << "Data loaded successfully from " << filename << endl;
    }
};

int main() {
    EVChargingStation station(10, 8.0);
    
    cout << "========== EV Charging Station System ==========\n";
    cout << "Loading data from case4.csv...\n";
    
    station.loadFromCSV("case4.csv");
    station.processChargingQueue();
    station.displayStatistics();
    
    return 0;
}
