#include <bits/stdc++.h>
using namespace std;


// ---------- Structures ----------
struct Node {
    string id, name;
};

struct Edge {
    string from, to;
    int distance; // in km
};


// ---------- Data ----------
vector<Node> nodes;
vector<Edge> edges;
const string NODE_FILE = "industrial_nodes.csv";
const string EDGE_FILE = "industrial_edges.csv";


// ---------- CSV Helpers ----------
void loadNodes() {
    nodes.clear();
    ifstream in(NODE_FILE);
    if (!in.is_open()) return;
    string line;
    getline(in, line); // skip header
    while(getline(in, line)) {
        if(line.empty()) continue;
        stringstream ss(line);
        Node n;
        if(!getline(ss, n.id, ',')) continue;
        if(!getline(ss, n.name, ',')) continue;
        nodes.push_back(n);
    }
    in.close();
}

void saveNodes() {
    ofstream out(NODE_FILE);
    out << "id,name\n";
    for(auto &n : nodes)
        out << n.id << "," << n.name << "\n";
    out.close();
}

void loadEdges() {
    edges.clear();
    ifstream in(EDGE_FILE);
    if(!in.is_open()) return;
    string line;
    getline(in, line); // skip header
    while(getline(in, line)) {
        if(line.empty()) continue;
        stringstream ss(line);
        Edge e; string dist;
        if(!getline(ss, e.from, ',')) continue;
        if(!getline(ss, e.to, ',')) continue;
        if(!getline(ss, dist, ',')) continue;
        e.distance = stoi(dist);
        edges.push_back(e);
    }
    in.close();
}

void saveEdges() {
    ofstream out(EDGE_FILE);
    out << "from,to,distance\n";
    for(auto &e : edges)
        out << e.from << "," << e.to << "," << e.distance << "\n";
    out.close();
}


// ---------- Features ----------
void addNode() {
    Node n;
    cout << "Node ID: "; cin >> n.id;
    cout << "Name: "; cin >> n.name;
    nodes.push_back(n);
    saveNodes();
    cout << "Node added.\n";
}

void addEdge() {
    Edge e;
    cout << "From Node ID: "; cin >> e.from;
    cout << "To Node ID: "; cin >> e.to;
    cout << "Distance (km): "; cin >> e.distance;
    edges.push_back(e);
    saveEdges();
    cout << "Edge added.\n";
}

void viewNodes() {
    if(nodes.empty()){ cout << "No nodes.\n"; return; }
    cout << left << setw(10) << "ID" << setw(15) << "Name\n";
    cout << string(25,'-') << "\n";
    for(auto &n : nodes)
        cout << left << setw(10) << n.id << setw(15) << n.name << "\n";
}

void viewEdges() {
    if(edges.empty()){ cout << "No edges.\n"; return; }
    cout << left << setw(10) << "From" << setw(10) << "To" << setw(10) << "Distance\n";
    cout << string(30,'-') << "\n";
    for(auto &e : edges)
        cout << left << setw(10) << e.from << setw(10) << e.to << setw(10) << e.distance << "\n";
}


// ---------- Dijkstra ----------
void findShortestPath() {
    if(nodes.empty() || edges.empty()){ cout << "Insufficient data.\n"; return; }
    string start, end;
    cout << "Enter Start Node ID: "; cin >> start;
    cout << "Enter End Node ID: "; cin >> end;

    unordered_map<string, vector<pair<string,int>>> graph;
    for(auto &e : edges){
        graph[e.from].push_back(make_pair(e.to,e.distance));
        // assuming undirected road network
        graph[e.to].push_back(make_pair(e.from,e.distance));
    }

    unordered_map<string,int> dist;
    unordered_map<string,string> prev;
    for(auto &n : nodes) dist[n.id] = INT_MAX;
    dist[start] = 0;

    using P = pair<int,string>;
    priority_queue<P, vector<P>, greater<P>> pq;
    pq.push(make_pair(0,start));

    while(!pq.empty()){
        P top = pq.top(); pq.pop();
        int d = top.first;
        string u = top.second;
        if(d > dist[u]) continue;
        for(size_t i=0;i<graph[u].size();i++){
            string v = graph[u][i].first;
            int w = graph[u][i].second;
            if(dist[u] + w < dist[v]){
                dist[v] = dist[u] + w;
                prev[v] = u;
                pq.push(make_pair(dist[v],v));
            }
        }
    }

    if(dist[end] == INT_MAX){ cout << "No path exists.\n"; return; }

    vector<string> path;
    for(string at=end; at!=start; at=prev[at]) path.push_back(at);
    path.push_back(start);
    reverse(path.begin(), path.end());

    cout << "Shortest Path: ";
    for(auto &p : path) cout << p << " ";
    cout << "\nTotal Distance: " << dist[end] << " km\n";
}


// ---------- Sample Data Loader (10 nodes + 10 edges) ----------
void loadSampleData() {
    // Nodes
    ofstream nOut(NODE_FILE, ios::trunc);
    nOut << "id,name\n";
    nOut << "N001,Plant A\n";
    nOut << "N002,Plant B\n";
    nOut << "N003,Warehouse 1\n";
    nOut << "N004,Warehouse 2\n";
    nOut << "N005,Port\n";
    nOut << "N006,Depot 1\n";
    nOut << "N007,Depot 2\n";
    nOut << "N008,Factory X\n";
    nOut << "N009,Factory Y\n";
    nOut << "N010,Distribution Hub\n";
    nOut.close();

    // Edges (road connections)
    ofstream eOut(EDGE_FILE, ios::trunc);
    eOut << "from,to,distance\n";
    eOut << "N001,N003,10\n";
    eOut << "N003,N005,20\n";
    eOut << "N001,N004,15\n";
    eOut << "N004,N005,25\n";
    eOut << "N002,N003,12\n";
    eOut << "N002,N006,18\n";
    eOut << "N006,N007,8\n";
    eOut << "N007,N005,16\n";
    eOut << "N008,N009,14\n";
    eOut << "N009,N010,10\n";
    eOut.close();

    // Reload into memory
    loadNodes();
    loadEdges();
    cout << "Sample data loaded for industrial nodes and edges.\n";
}


// ---------- Menu ----------
void menu() {
    // Load sample dataset at startup (overwrites existing CSVs)
    loadSampleData();

    while(true) {
        cout << "\n===== INDUSTRIAL TRANSPORT ROUTE MANAGEMENT =====\n";
        cout << "1. Add Node\n2. Add Edge\n3. View Nodes\n4. View Edges\n5. Find Shortest Path\n6. Exit\nChoice: ";
        int c; cin >> c;
        switch(c){
            case 1: addNode(); break;
            case 2: addEdge(); break;
            case 3: viewNodes(); break;
            case 4: viewEdges(); break;
            case 5: findShortestPath(); break;
            case 6: return;
            default: cout << "Invalid choice.\n";
        }
    }
}


// ---------- Main ----------
int main() {
    menu();
    return 0;
}
