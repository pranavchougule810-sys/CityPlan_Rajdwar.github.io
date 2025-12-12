#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <string>
using namespace std;

// Node for Linked List
struct Member {
    string memberID;
    string name;
    string membershipType;
    string enrollmentDate;
    int sessionsAttended;
    Member* next;
    
    Member(string id, string n, string type, string date) 
        : memberID(id), name(n), membershipType(type), 
          enrollmentDate(date), sessionsAttended(0), next(nullptr) {}
};

// Class booking structure for queue
struct ClassBooking {
    string memberID;
    string className;
    string timeSlot;
    string date;
};

class WellnessCenter {
private:
    Member* head; // Linked list head
    queue<ClassBooking> waitingQueue;
    int totalMembers;
    
public:
    WellnessCenter() : head(nullptr), totalMembers(0) {}
    
    // Add member to linked list
    void addMember(string id, string name, string type, string date) {
        Member* newMember = new Member(id, name, type, date);
        
        if(head == nullptr) {
            head = newMember;
        } else {
            Member* temp = head;
            while(temp->next != nullptr) {
                temp = temp->next;
            }
            temp->next = newMember;
        }
        totalMembers++;
    }
    
    // Add class booking to queue
    void addToWaitingQueue(string memberID, string className, 
                           string timeSlot, string date) {
        ClassBooking booking = {memberID, className, timeSlot, date};
        waitingQueue.push(booking);
    }
    
    // Process waiting queue
    void processWaitingQueue(int count) {
        cout << "\n========== Processing Waiting Queue ==========\n";
        int processed = 0;
        
        while(!waitingQueue.empty() && processed < count) {
            ClassBooking booking = waitingQueue.front();
            waitingQueue.pop();
            
            cout << "Processed: Member " << booking.memberID 
                 << " | Class: " << booking.className
                 << " | Time: " << booking.timeSlot << endl;
            processed++;
        }
        cout << "Total processed: " << processed << endl;
    }
    
    // Search member in linked list
    void searchMember(string id) {
        Member* temp = head;
        while(temp != nullptr) {
            if(temp->memberID == id) {
                cout << "\n========== Member Found ==========\n";
                cout << "Member ID: " << temp->memberID << endl;
                cout << "Name: " << temp->name << endl;
                cout << "Type: " << temp->membershipType << endl;
                cout << "Enrolled: " << temp->enrollmentDate << endl;
                cout << "Sessions: " << temp->sessionsAttended << endl;
                return;
            }
            temp = temp->next;
        }
        cout << "Member not found!" << endl;
    }
    
    // Display all members
    void displayAllMembers() {
        cout << "\n========== All Members ==========\n";
        Member* temp = head;
        int count = 0;
        
        while(temp != nullptr && count < 20) {
            cout << temp->memberID << " | " << temp->name 
                 << " | " << temp->membershipType << endl;
            temp = temp->next;
            count++;
        }
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
            string id, name, type, date, className, timeSlot;
            
            getline(ss, id, ',');
            getline(ss, name, ',');
            getline(ss, type, ',');
            getline(ss, date, ',');
            getline(ss, className, ',');
            getline(ss, timeSlot, ',');
            
            addMember(id, name, type, date);
            addToWaitingQueue(id, className, timeSlot, date);
        }
        
        file.close();
        cout << "Data loaded successfully from " << filename << endl;
    }
    
    void displayStatistics() {
        cout << "\n========== Center Statistics ==========\n";
        cout << "Total Members: " << totalMembers << endl;
        cout << "Waiting Queue Size: " << waitingQueue.size() << endl;
    }
};

int main() {
    WellnessCenter center;
    
    cout << "========== Health & Wellness Center System ==========\n";
    cout << "Loading data from case2.csv...\n";
    
    center.loadFromCSV("case2.csv");
    center.displayStatistics();
    center.displayAllMembers();
    center.processWaitingQueue(15);
    
    // Search example
    cout << "\nSearching for member M001:" << endl;
    center.searchMember("M001");
    
    return 0;
}
