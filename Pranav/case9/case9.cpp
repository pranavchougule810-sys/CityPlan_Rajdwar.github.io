#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <string>
using namespace std;

struct Student {
    string studentID;
    string name;
    int skillLevel;
    string course;
    vector<string> prerequisites;
};

class SkillCenter {
private:
    unordered_map<string, Student> students;
    unordered_map<string, vector<string>> courseGraph; // course -> prerequisites
    unordered_map<string, int> enrollmentCount;
    
public:
    SkillCenter() {
        initializeCourseGraph();
    }
    
    void initializeCourseGraph() {
        // Build course dependency graph
        courseGraph["Programming_Basics"] = {};
        courseGraph["Data_Structures"] = {"Programming_Basics"};
        courseGraph["Algorithms"] = {"Data_Structures"};
        courseGraph["Web_Development"] = {"Programming_Basics"};
        courseGraph["Machine_Learning"] = {"Algorithms", "Data_Structures"};
        courseGraph["Advanced_ML"] = {"Machine_Learning"};
    }
    
    string recommendCourse(int skillLevel) {
        if(skillLevel < 3) return "Programming_Basics";
        else if(skillLevel < 5) return "Data_Structures";
        else if(skillLevel < 7) return "Algorithms";
        else if(skillLevel < 9) return "Web_Development";
        else return "Machine_Learning";
    }
    
    void enrollStudent(string id, string name, int skillLevel) {
        Student student;
        student.studentID = id;
        student.name = name;
        student.skillLevel = skillLevel;
        student.course = recommendCourse(skillLevel);
        student.prerequisites = courseGraph[student.course];
        
        students[id] = student;
        enrollmentCount[student.course]++;
    }
    
    void BFS_CoursePath(string startCourse) {
        cout << "\n========== BFS Course Learning Path from " << startCourse << " ==========\n";
        
        unordered_map<string, bool> visited;
        queue<string> q;
        
        q.push(startCourse);
        visited[startCourse] = true;
        int level = 1;
        
        while(!q.empty()) {
            int size = q.size();
            cout << "Level " << level << ": ";
            
            for(int i = 0; i < size; i++) {
                string course = q.front();
                q.pop();
                cout << course << " ";
                
                // Find courses that depend on current course
                for(auto& pair : courseGraph) {
                    for(auto& prereq : pair.second) {
                        if(prereq == course && !visited[pair.first]) {
                            visited[pair.first] = true;
                            q.push(pair.first);
                        }
                    }
                }
            }
            cout << endl;
            level++;
        }
    }
    
    void displayCourseEnrollments() {
        cout << "\n========== Course Enrollments ==========\n";
        for(auto& pair : enrollmentCount) {
            cout << pair.first << ": " << pair.second << " students" << endl;
        }
    }
    
    void displayStatistics() {
        cout << "\n========== Skill Center Statistics ==========\n";
        cout << "Total Students: " << students.size() << endl;
        cout << "Total Courses: " << courseGraph.size() << endl;
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
            string id, name;
            int skillLevel;
            
            getline(ss, id, ',');
            getline(ss, name, ',');
            ss >> skillLevel;
            
            enrollStudent(id, name, skillLevel);
        }
        
        file.close();
        cout << "Data loaded successfully from " << filename << endl;
    }
};

int main() {
    SkillCenter center;
    
    cout << "========== Skill Development Center System ==========\n";
    cout << "Loading data from case9.csv...\n";
    
    center.loadFromCSV("case9.csv");
    center.displayStatistics();
    center.displayCourseEnrollments();
    center.BFS_CoursePath("Programming_Basics");
    
    return 0;
}
