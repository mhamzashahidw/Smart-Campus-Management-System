

#include <iostream>
#include <string>
#include <vector>
#include <climits>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <queue>
#include <map>
#include <unordered_map>
#include <functional> // REQUIRED for greater<> in priority_queue

using namespace std;

// ─────────────────────────────────────────────────────────────────────
//  CONSTANTS & DATA STRUCTURES
// ─────────────────────────────────────────────────────────────────────
const int INF = INT_MAX;

struct SearchRecord { int src, dest, distance; };
struct UserRequest { int requestId, userId; string query, status; };
struct LocationInfo { int id; string name; int dist; };

// Global resizable location arrays
vector<string> locationNames;
vector<string> shortCodes;
bool simulationMode = false; // Toggle for Weather/Traffic simulation mode

// =====================================================================
//  GRAPH MODULE (std::vector + std::priority_queue)
// =====================================================================
class CampusGraph {
    vector<vector<pair<int, int>>> adjList;

public:
    CampusGraph(int size) : adjList(size) {}

    void addVertex() {
        adjList.push_back({});
    }

    void removeVertex(int id) {
        if (id < 0 || id >= (int)adjList.size()) return;
        adjList[id].clear(); // Clear outgoing edges

        // Remove incoming edges from other locations
        for (int i = 0; i < (int)adjList.size(); i++) {
            adjList[i].erase(
                remove_if(adjList[i].begin(), adjList[i].end(), [id](const pair<int, int>& edge) {
                    return edge.first == id;
                    }),
                adjList[i].end()
            );
        }
    }

    void addEdge(int u, int v, int dist) {
        if (u >= 0 && u < (int)adjList.size() && v >= 0 && v < (int)adjList.size()) {
            adjList[u].push_back({ v, dist });
            adjList[v].push_back({ u, dist });
        }
    }

    void displayGraph() {
        cout << "==" << "====================================================================" << endl;
        cout << "||" << "\t\tCAMPUS GRAPH (ADJACENCY LIST)" << endl;
        cout << "==" << "====================================================================" << endl;
        for (int i = 0; i < (int)adjList.size(); i++) {
            if (locationNames[i].empty()) continue; // Skip deleted

            string neighborString = "";
            bool first = true;
            for (size_t j = 0; j < adjList[i].size(); j++) {
                int neighbor = adjList[i][j].first;
                if (locationNames[neighbor].empty()) continue;
                if (!first) neighborString += " -> ";
                neighborString += shortCodes[neighbor] + "(" + to_string(adjList[i][j].second) + "m)";
                first = false;
            }
            cout << "|| " << "[" << setw(2) << i << "] " << left << setw(22) << locationNames[i] << " ||\t -> " << neighborString << endl;
        }
        cout << "==" << "====================================================================" << endl;
    }

    void dijkstra(int src, vector<int>& dist, vector<int>& prev, pair<int, int> blockedEdge = { -1, -1 }) {
        int n = adjList.size();
        dist.assign(n, INF);
        prev.assign(n, -1);
        if (src < 0 || src >= n || locationNames[src].empty()) return;
        dist[src] = 0;

        // Min-Priority Queue holding pairs of {distance, node}
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
        pq.push({ 0, src });

        while (!pq.empty()) {
            auto cur = pq.top(); pq.pop();
            int u = cur.second;
            if (cur.first > dist[u]) continue;

            for (auto& edge : adjList[u]) {
                int v = edge.first, w = edge.second;
                if (locationNames[v].empty()) continue; // Skip deleted nodes

                // Skip blocked edges (for alternative path suggestions)
                if ((u == blockedEdge.first && v == blockedEdge.second) ||
                    (u == blockedEdge.second && v == blockedEdge.first)) {
                    continue;
                }

                // Weather/Traffic Simulation: Adjust weights dynamically
                if (simulationMode) {
                    // Hostels area (wet paths/muddy walking)
                    if ((u >= 12 && u <= 15) || (v >= 12 && v <= 15)) {
                        w = (int)(w * 1.5);
                    }
                    // Main entrance gate paths (heavy shuttle traffic)
                    else if (u == 0 || v == 0) {
                        w = (int)(w * 1.3);
                    }
                }

                if (dist[u] != INF && dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    prev[v] = u;
                    pq.push({ dist[v], v });
                }
            }
        }
    }

    void printPath(int src, int dest) {
        int n = adjList.size();
        if (src < 0 || src >= n || dest < 0 || dest >= n || locationNames[src].empty() || locationNames[dest].empty()) {
            cout << "==" << "====================================================================" << endl;
            cout << "||" << "\t ERROR           ||\t -> Invalid or deleted location ID." << endl;
            cout << "==" << "====================================================================" << endl;
            return;
        }

        vector<int> dist(n), prev(n);
        dijkstra(src, dist, prev);
        if (dist[dest] == INF) {
            cout << "==" << "====================================================================" << endl;
            cout << "||" << "\t ERROR           ||\t -> No path found!" << endl;
            cout << "==" << "====================================================================" << endl;
            return;
        }

        vector<int> path;
        for (int v = dest; v != -1; v = prev[v]) path.push_back(v);
        reverse(path.begin(), path.end());

        string pathStr = "";
        for (int i = 0; i < (int)path.size(); i++) {
            pathStr += locationNames[path[i]];
            if (i + 1 < (int)path.size()) pathStr += " -> ";
        }

        cout << "==" << "====================================================================" << endl;
        cout << "||" << "\t\tSHORTEST PATH NAVIGATION (DIJKSTRA)" << endl;
        cout << "==" << "====================================================================" << endl;
        if (simulationMode) {
            cout << "||" << "\t STATUS          ||\t -> [SIMULATION ACTIVE] Dynamic traffic/weather active!" << endl;
        }
        cout << "||" << "\t From            ||\t -> " << locationNames[src] << endl;
        cout << "||" << "\t To              ||\t -> " << locationNames[dest] << endl;
        cout << "||" << "\t Path            ||\t -> " << pathStr << endl;
        cout << "||" << "\t Total Distance  ||\t -> " << dist[dest] << " meters" << endl;
        cout << "||" << "\t Est. Walk Time  ||\t -> ~" << dist[dest] / 80 << " min (80 m/min)" << endl;
        cout << "==" << "====================================================================" << endl;

        // Suggest alternative path if primary path has at least one transition
        if (path.size() >= 2) {
            vector<int> distAlt(n), prevAlt(n);
            pair<int, int> blockedEdge = { path[0], path[1] };
            dijkstra(src, distAlt, prevAlt, blockedEdge);
            if (distAlt[dest] != INF && distAlt[dest] != dist[dest]) {
                vector<int> pathAlt;
                for (int v = dest; v != -1; v = prevAlt[v]) pathAlt.push_back(v);
                reverse(pathAlt.begin(), pathAlt.end());

                string altPathStr = "";
                for (int i = 0; i < (int)pathAlt.size(); i++) {
                    altPathStr += locationNames[pathAlt[i]];
                    if (i + 1 < (int)pathAlt.size()) altPathStr += " -> ";
                }

                cout << "\n==" << "====================================================================" << endl;
                cout << "||" << "\t\tALTERNATIVE PATH SUGGESTION" << endl;
                cout << "==" << "====================================================================" << endl;
                cout << "||" << "\t Avoiding Link   ||\t -> " << locationNames[path[0]] << " <-> " << locationNames[path[1]] << endl;
                cout << "||" << "\t Path            ||\t -> " << altPathStr << endl;
                cout << "||" << "\t Total Distance  ||\t -> " << distAlt[dest] << " meters" << endl;
                cout << "||" << "\t Est. Walk Time  ||\t -> ~" << distAlt[dest] / 80 << " min" << endl;
                cout << "==" << "====================================================================" << endl;
            }
        }
    }

    void BFS(int start) {
        int n = adjList.size();
        if (start < 0 || start >= n || locationNames[start].empty()) return;
        vector<bool> visited(n, false);
        queue<int> q;
        visited[start] = true;
        q.push(start);

        string bfsResult = "";
        while (!q.empty()) {
            int u = q.front(); q.pop();
            bfsResult += shortCodes[u];

            for (auto& edge : adjList[u]) {
                int v = edge.first;
                if (locationNames[v].empty()) continue;
                if (!visited[v]) {
                    visited[v] = true;
                    q.push(v);
                }
            }
            if (!q.empty()) bfsResult += " -> ";
        }

        cout << "==" << "====================================================================" << endl;
        cout << "||" << "\t BFS Traversal   ||\t -> " << bfsResult << " (From: " << locationNames[start] << ")" << endl;
        cout << "==" << "====================================================================" << endl;
    }

    void DFS(int start) {
        int n = adjList.size();
        if (start < 0 || start >= n || locationNames[start].empty()) return;
        vector<bool> visited(n, false);

        string dfsResult = "";
        DFSHelper(start, visited, dfsResult);

        // Trim trailing space if exists
        if (!dfsResult.empty() && dfsResult.back() == ' ') {
            dfsResult.pop_back();
        }

        cout << "==" << "====================================================================" << endl;
        cout << "||" << "\t DFS Traversal   ||\t -> " << dfsResult << " (From: " << locationNames[start] << ")" << endl;
        cout << "==" << "====================================================================" << endl;
    }

    void showNeighbors(int u) {
        int n = adjList.size();
        if (u < 0 || u >= n || locationNames[u].empty()) return;

        cout << "==" << "====================================================================" << endl;
        cout << "||" << "\t Neighbours of   ||\t -> " << locationNames[u] << endl;
        cout << "==" << "====================================================================" << endl;

        int count = 0;
        for (auto& edge : adjList[u]) {
            int v = edge.first;
            if (locationNames[v].empty()) continue;
            cout << "||" << "\t " << left << setw(15) << shortCodes[v] << " ||\t -> " << edge.second << " meters away" << endl;
            count++;
        }
        if (count == 0) {
            cout << "||" << "\t Connections     ||\t -> No connected links available" << endl;
        }
        cout << "==" << "====================================================================" << endl;
    }

private:
    void DFSHelper(int u, vector<bool>& visited, string& result) {
        visited[u] = true;
        result += shortCodes[u] + " ";
        for (auto& edge : adjList[u]) {
            int v = edge.first;
            if (locationNames[v].empty()) continue;
            if (!visited[v]) DFSHelper(v, visited, result);
        }
    }
};






// =====================================================================
//  STACK MODULE (std::vector + File IO)
// =====================================================================
class NavigationStack {
    vector<SearchRecord> data;

    void saveToFile() {
        ofstream outFile("navigation_history.txt");
        if (!outFile) return;
        for (auto& r : data) {
            outFile << r.src << " " << r.dest << " " << r.distance << "\n";
        }
        outFile.close();
    }

    void loadFromFile() {
        ifstream inFile("navigation_history.txt");
        if (!inFile) return;
        int src, dest, distance;
        data.clear();
        while (inFile >> src >> dest >> distance) {
            data.push_back({ src, dest, distance });
        }
        inFile.close();
    }

public:
    NavigationStack() { loadFromFile(); }
    bool isEmpty() { return data.empty(); }

    void push(int src, int dest, int dist) {
        data.push_back({ src, dest, dist });
        saveToFile();
    }

    SearchRecord pop() {
        if (isEmpty()) return { -1, -1, -1 };
        SearchRecord r = data.back();
        data.pop_back();
        saveToFile();
        return r;
    }

    void display() {
        if (isEmpty()) {
            cout << "==" << "====================================================================" << endl;
            cout << "||" << "\t HISTORY         ||\t -> No navigation history recorded yet." << endl;
            cout << "==" << "====================================================================" << endl;
            return;
        }

        cout << "==" << "====================================================================" << endl;
        cout << "||" << "\t\tNAVIGATION HISTORY (STACK -- NEWEST FIRST)" << endl;
        cout << "==" << "====================================================================" << endl;

        int count = 1;
        for (int i = (int)data.size() - 1; i >= 0; i--) {
            int src = data[i].src;
            int dest = data[i].dest;

            string srcName = "Deleted Location";
            string destName = "Deleted Location";

            if (src >= 0 && src < (int)locationNames.size() && !locationNames[src].empty()) {
                srcName = locationNames[src];
            }
            if (dest >= 0 && dest < (int)locationNames.size() && !locationNames[dest].empty()) {
                destName = locationNames[dest];
            }

            string routeStr = srcName + " -> " + destName + " (" + to_string(data[i].distance) + " m)";
            cout << "|| " << "[" << setw(2) << count++ << "] " << left << setw(22) << "Record Entry" << " ||\t -> " << routeStr << endl;
        }
        cout << "==" << "====================================================================" << endl;
    }
};









// =====================================================================
//  QUEUE MODULE (std::queue)
// =====================================================================
class RequestQueue {
    queue<UserRequest> q;
    int nextId = 1;

public:
    bool isEmpty() { return q.empty(); }

    int enqueue(int userId, const string& query) {
        int id = nextId++;
        q.push({ id, userId, query, "PENDING" });
        return id;
    }

    UserRequest dequeue() {
        if (isEmpty()) return { -1, -1, "", "" };
        UserRequest r = q.front();
        q.pop();
        return r;
    }

    void display() {
        if (isEmpty()) {
            cout << "==" << "====================================================================" << endl;
            cout << "||" << "\t QUEUE STATUS    ||\t -> No pending requests available." << endl;
            cout << "==" << "====================================================================" << endl;
            return;
        }

        cout << "==" << "====================================================================" << endl;
        cout << "||" << "\t\tPENDING REQUEST QUEUE (FIRST IN FIRST OUT)" << endl;
        cout << "==" << "====================================================================" << endl;

        auto temp = q;
        while (!temp.empty()) {
            auto r = temp.front();
            temp.pop();

            string requestDetails = "User: " + to_string(r.userId) + " | Query: " + r.query + " [" + r.status + "]";
            cout << "|| " << "Req #" << setw(3) << left << r.requestId << " ||\t -> " << requestDetails << endl;
        }
        cout << "==" << "====================================================================" << endl;
    }

    int size() { return q.size(); }
};








// =====================================================================
//  GRAPH BUILDER
// =====================================================================
void buildCampusGraph(CampusGraph& g) {
    // CRITICAL SAFETY CHECK: Ensure the global mapping vectors 
    // have enough space for maximum index (20 -> 21 total items)
    if (locationNames.size() < 21) {
        locationNames.resize(21, "Unknown Location");
    }
    if (shortCodes.size() < 21) {
        shortCodes.resize(21, "UNK");
    }

    g.addEdge(0, 1, 450);   // Gate <-> Admin
    g.addEdge(0, 20, 280);  // Gate <-> Parking
    g.addEdge(0, 18, 920);  // Gate <-> Medical
    g.addEdge(0, 2, 900);   // Gate <-> CS Dept
    g.addEdge(0, 12, 450);  // Gate <-> Hostel A
    g.addEdge(0, 13, 550);  // Gate <-> Hostel B

    g.addEdge(1, 2, 550);   // Admin <-> CS
    g.addEdge(1, 10, 455);  // Admin <-> Library
    g.addEdge(1, 11, 375);  // Admin <-> Mosque
    g.addEdge(1, 9, 500);   // Admin <-> BSH
    g.addEdge(1, 16, 520);  // Admin <-> Cafeteria
    g.addEdge(1, 18, 210);  // Admin <-> Medical
    g.addEdge(1, 20, 455);  // Admin <-> Parking

    g.addEdge(2, 3, 650);   // CS <-> EE
    g.addEdge(2, 10, 245);  // CS <-> Library
    g.addEdge(2, 9, 400);   // CS <-> BSH
    g.addEdge(2, 8, 150);   // CS <-> Mgmt
    g.addEdge(2, 16, 510);  // CS <-> Cafeteria
    g.addEdge(2, 4, 550);   // CS <-> Mech
    g.addEdge(2, 19, 750);  // CS <-> CERAD
    g.addEdge(2, 5, 500);   // CS <-> Chem

    g.addEdge(3, 4, 180);   // EE <-> Mech
    g.addEdge(3, 19, 1200); // EE <-> CERAD
    g.addEdge(3, 16, 405);  // EE <-> Cafeteria

    g.addEdge(4, 5, 155);   // Mech <-> Chem
    g.addEdge(4, 6, 305);   // Mech <-> Biomed
    g.addEdge(4, 17, 580);  // Mech <-> Sports
    g.addEdge(4, 16, 320);  // Mech <-> Cafeteria
    g.addEdge(4, 19, 1345); // Mech <-> CERAD

    g.addEdge(5, 6, 150);   // Chem <-> Biomed
    g.addEdge(5, 7, 275);   // Chem <-> Food
    g.addEdge(5, 19, 1480); // Chem <-> CERAD

    g.addEdge(6, 7, 140);   // Biomed <-> Food
    g.addEdge(6, 18, 1035); // Biomed <-> Medical

    g.addEdge(7, 8, 880);   // Food <-> Mgmt
    g.addEdge(7, 15, 510);  // Food <-> Hostel D

    g.addEdge(8, 9, 90);    // Mgmt <-> BSH
    g.addEdge(8, 16, 640);  // Mgmt <-> Cafeteria
    g.addEdge(8, 10, 370);  // Mgmt <-> Library

    g.addEdge(9, 10, 280);  // BSH <-> Library
    g.addEdge(9, 11, 400);  // BSH <-> Mosque

    g.addEdge(10, 11, 155); // Library <-> Mosque
    g.addEdge(10, 16, 270); // Library <-> Cafeteria

    g.addEdge(11, 12, 335); // Mosque <-> Hostel A

    g.addEdge(12, 13, 100); // Hostel A <-> Hostel B
    g.addEdge(12, 16, 160); // Hostel A <-> Cafeteria
    g.addEdge(12, 17, 495); // Hostel A <-> Sports
    g.addEdge(12, 18, 855); // Hostel A <-> Medical

    g.addEdge(13, 14, 850); // Hostel B <-> Hostel C
    g.addEdge(13, 16, 260); // Hostel B <-> Cafeteria
    g.addEdge(13, 17, 425); // Hostel B <-> Sports

    g.addEdge(14, 15, 90);  // Hostel C <-> Hostel D
    g.addEdge(14, 17, 175); // Hostel C <-> Sports
    g.addEdge(14, 16, 700); // Hostel C <-> Cafeteria

    g.addEdge(15, 17, 340); // Hostel D <-> Sports
    g.addEdge(15, 16, 860); // Hostel D <-> Cafeteria

    g.addEdge(17, 18, 1295);// Sports <-> Medical

    g.addEdge(18, 20, 250); // Medical <-> Parking
}








// =====================================================================
//  UTILITY & INTERFACE HELPERS
// =====================================================================
void printBanner() {
    cout << "\n";
    cout << "==" << "====================================================================" << endl;
    cout << "||" << "\t\tUET KSK SMART CAMPUS MANAGEMENT SYSTEM" << endl;
    cout << "||" << "\tUniversity of Engineering & Technology -- KSK Campus" << endl;
    cout << "||" << "\tCSC-162 Data Structures & Algorithms -- CCP Project" << endl;
    cout << "==" << "====================================================================" << endl;
    cout << "|| Core DSA  ||\t -> Graph, BST, Hash Table, Stack, Queue, IntroSort" << endl;
    cout << "|| Features  ||\t -> Simulation, Alternate Routes, ASCII Map, Admin Mod" << endl;
    cout << "==" << "====================================================================" << endl;
}

void printMainMenu() {
    cout << "\n==" << "====================================================================" << endl;
    cout << "||" << "\t\t\t      MAIN MENU" << endl;
    cout << "==" << "====================================================================" << endl;
    cout << "||" << "\t [1]  Find Shortest Path (Dijkstra)" << endl;
    cout << "||" << "\t [2]  BFS Traversal from a Location" << endl;
    cout << "||" << "\t [3]  DFS Traversal from a Location" << endl;
    cout << "||" << "\t [4]  View Campus Graph (Adjacency List)" << endl;
    cout << "||" << "\t [5]  Location Management (BST)" << endl;
    cout << "||" << "\t [6]  Hash Table -- Fast Lookup" << endl;
    cout << "||" << "\t [7]  Navigation History (Stack)" << endl;
    cout << "||" << "\t [8]  Request Queue Management" << endl;
    cout << "||" << "\t [9]  Sort Locations (IntroSort)" << endl;
    cout << "||" << "\t [10] Binary Search on Locations" << endl;
    cout << "||" << "\t [11] Show All Locations" << endl;
    cout << "||" << "\t [12] View Neighbours of a Location" << endl;
    cout << "||" << "\t [13] All Shortest Distances from a Node" << endl;
    cout << "||" << "\t [14] Toggle Weather/Traffic Simulation" << endl;
    cout << "||" << "\t [15] Show Text-Based Campus Map" << endl;
    cout << "||" << "\t [0]  Exit" << endl;
    cout << "==" << "====================================================================" << endl;
    cout << "   Enter Choice: ";
}




void printAllLocations() {
    cout << "==" << "====================================================================" << endl;
    cout << "||" << "\t\t\tUET KSK CAMPUS LOCATIONS" << endl;
    cout << "==" << "====================================================================" << endl;
    for (int i = 0; i < (int)locationNames.size(); i++) {
        if (locationNames[i].empty()) continue; // Skip deleted

        string locDetails = locationNames[i] + " [" + shortCodes[i] + "]";
        cout << "|| " << "[" << setw(2) << i << "] " << left << setw(22) << "Location Unit" << " ||\t -> " << locDetails << endl;
    }
    cout << "==" << "====================================================================" << endl;
}

int getValidLocation(const string& prompt, int numLocations) {
    int id;
    while (true) {
        cout << "   " << prompt;
        if (cin >> id) {
            cin.ignore(1000, '\n');
            if (id >= 0 && id < numLocations && id < (int)locationNames.size() && !locationNames[id].empty()) {
                return id;
            }
        }
        else {
            cin.clear();
            cin.ignore(1000, '\n');
        }
        cout << "==" << "====================================================================" << endl;
        cout << "||" << "\t ERROR           ||\t -> Enter a valid ID (0 to " << numLocations - 1 << ")" << endl;
        cout << "==" << "====================================================================" << endl;
    }
}

void showASCIIMap() {
    cout << "==" << "========================================================================" << endl;
    cout << "||" << "\t\t\tUET KSK CAMPUS TEXT-BASED MAP" << endl;
    cout << "==" << "========================================================================" << endl;
    cout << "        [Hostel C] ----- 90m ----- [Hostel D]                           \n";
    cout << "            |                          |                                \n";
    cout << "          175m                       340m                               \n";
    cout << "            |                          |                                \n";
    cout << "       [Sports Complex]                |                                \n";
    cout << "            |                          |                                \n";
    cout << "          425m                       860m                               \n";
    cout << "            |                          |                                \n";
    cout << "    [Mosque] --335m-- [Hostel A] ----- 100m ----- [Hostel B]            \n";
    cout << "        \\                  |                          /                 \n";
    cout << "       400m              160m                       260m                \n";
    cout << "         \\                 |                          /                 \n";
    cout << "          \\         [Student Cafeteria]              /                  \n";
    cout << "           \\               |                        /                   \n";
    cout << "            \\            270m                      /                    \n";
    cout << "             \\             |                      /                     \n";
    cout << "              \\--- [Central Library]             /                      \n";
    cout << "              /            |                    /                       \n";
    cout << "           280m          245m                  /                        \n";
    cout << "            /              |                  /                         \n";
    cout << "     [BSH Block] --400m-- [CS Dept] ---------/                          \n";
    cout << "          |                |    \\                                       \n";
    cout << "         90m              150m   500m                                   \n";
    cout << "          |                |      \\                                     \n";
    cout << "     [MS Dept]           [Mgmt]   [Chem Dept]                           \n";
    cout << "          \\                /           |                                \n";
    cout << "         500m            550m         155m                              \n";
    cout << "           \\              /            |                                \n";
    cout << "            \\-- [Admin Block] ---- [Mech Dept] -- 180m -- [EE Dept]     \n";
    cout << "                   |                  |                      |          \n";
    cout << "                 210m                320m                   405m        \n";
    cout << "                   |                  |                      |          \n";
    cout << "              [Dispensary]            |                      |          \n";
    cout << "                   |                  |                      |          \n";
    cout << "                 250m                 |                      |          \n";
    cout << "                   |                  |                      |          \n";
    cout << "               [Parking]              v                      v          \n";
    cout << "                   |               (Biomed)               (CERAD)       \n";
    cout << "                 280m                                                   \n";
    cout << "                   |                                                    \n";
    cout << "           [Main Entrance Gate]                                         \n";
    cout << "==" << "========================================================================" << endl;
}





// =====================================================================
//  MAIN DRIVER
// =====================================================================
int main() {
    // Initialize standard location arrays
    locationNames = {
        "Main Entrance Gate",
        "Admin Block / Rector Office",
        "CS Department",
        "Electrical Engineering Dept",
        "Mechanical Engineering Dept",
        "Chemical Engineering Dept",
        "Biomedical Engineering Dept",
        "Food Science & Technology",
        "Management Sciences Dept",
        "Basic Sciences & Humanities",
        "Central Library",
        "Mosque",
        "Boys Hostel Block A",
        "Boys Hostel Block B",
        "Boys Hostel Block C",
        "Boys Hostel Block D",
        "Student Cafeteria",
        "Sports Complex",
        "Medical / Dispensary",
        "CERAD (Energy Research)",
        "Parking Area"
    };

    shortCodes = {
        "GATE", "ADMIN", "CS", "EE", "MECH", "CHEM", "BIO",
        "FOOD", "MGMT", "BSH", "LIB", "MOSQUE",
        "HOSTEL_A", "HOSTEL_B", "HOSTEL_C", "HOSTEL_D",
        "CAFE", "SPORTS", "MEDICAL", "CERAD", "PARKING"
    };

    CampusGraph graph(locationNames.size());
    map<string, int> bst;               // Acts as BST Directory
    unordered_map<string, int> hashTable; // Fast Lookup Hash Table
    NavigationStack navStack;
    RequestQueue reqQueue;

    buildCampusGraph(graph);

    // Populate BST and Hash Table lookups
    for (size_t i = 0; i < locationNames.size(); i++) {
        bst[locationNames[i]] = i;

        string fullKey = locationNames[i];
        transform(fullKey.begin(), fullKey.end(), fullKey.begin(), ::tolower);
        hashTable[fullKey] = i;

        string codeKey = shortCodes[i];
        transform(codeKey.begin(), codeKey.end(), codeKey.begin(), ::tolower);
        hashTable[codeKey] = i;
    }

    printBanner();
    printAllLocations();

    bool running = true;
    while (running) {
        printMainMenu();
        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }
        cin.ignore(1000, '\n');

        switch (choice) {
        case 1: {
            printAllLocations();
            int src = getValidLocation("Enter SOURCE location ID      : ", locationNames.size());
            int dest = getValidLocation("Enter DESTINATION location ID : ", locationNames.size());
            graph.printPath(src, dest);

            int n = locationNames.size();
            vector<int> dist(n), prev(n);
            graph.dijkstra(src, dist, prev);
            if (dist[dest] != INF) {
                navStack.push(src, dest, dist[dest]);
                cout << "==" << "====================================================================" << endl;
                cout << "||" << "\t HISTORY STATUS  ||\t -> Search entry saved successfully." << endl;
                cout << "==" << "====================================================================" << endl;
            }
            reqQueue.enqueue(1, "NAVIGATE " + locationNames[src] + " -> " + locationNames[dest]);
            break;
        }
        case 2: {
            printAllLocations();
            int s = getValidLocation("Enter START location ID: ", locationNames.size());
            graph.BFS(s);
            break;
        }
        case 3: {
            printAllLocations();
            int s = getValidLocation("Enter START location ID: ", locationNames.size());
            graph.DFS(s);
            break;
        }
        case 4:
            graph.displayGraph();
            break;
        case 5: {
            bool inSubMenu5 = true;
            while (inSubMenu5) {
                cout << "\n==" << "====================================================================" << endl;
                cout << "||" << "\t\t      LOCATION MANAGEMENT MODULE (BST)" << endl;
                cout << "==" << "====================================================================" << endl;
                cout << "||" << "\t [1] Search Location" << endl;
                cout << "||" << "\t [2] Add New Location" << endl;
                cout << "||" << "\t [3] Delete Location" << endl;
                cout << "||" << "\t [4] Show All Directory Index (Sorted)" << endl;
                cout << "||" << "\t [0] Back to Main Menu" << endl;
                cout << "==" << "====================================================================" << endl;
                cout << "   Enter Choice: ";
                int c;
                if (!(cin >> c)) { cin.clear(); cin.ignore(1000, '\n'); continue; }
                cin.ignore(1000, '\n');

                if (c == 0) {
                    inSubMenu5 = false;
                }
                else if (c == 1) {
                    cout << "   Enter name (full or partial): "; string name; getline(cin, name);
                    string lowerTarget = name; transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(), ::tolower);
                    bool found = false;

                    cout << "==" << "====================================================================" << endl;
                    for (auto& pair : bst) {
                        string lowerKey = pair.first; transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
                        if (lowerKey.find(lowerTarget) != string::npos) {
                            cout << "|| " << "[ID: " << setw(2) << pair.second << "] " << left << setw(23) << pair.first << " ||\t -> Verified Match" << endl;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        cout << "||" << "\t LOOKUP RESULT   ||\t -> Target location not found." << endl;
                    }
                    cout << "==" << "====================================================================" << endl;
                }
                else if (c == 2) {
                    cout << "   Enter full name of new location: "; string name; getline(cin, name);
                    cout << "   Enter short code (e.g. LAB_A)   : "; string code; getline(cin, code);

                    string lowerName = name; transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                    string lowerCode = code; transform(lowerCode.begin(), lowerCode.end(), lowerCode.begin(), ::tolower);

                    cout << "==" << "====================================================================" << endl;
                    if (hashTable.find(lowerName) != hashTable.end() || hashTable.find(lowerCode) != hashTable.end()) {
                        cout << "||" << "\t ERROR           ||\t -> Name or Short Code already exists!" << endl;
                    }
                    else {
                        locationNames.push_back(name);
                        shortCodes.push_back(code);
                        int newId = locationNames.size() - 1;

                        bst[name] = newId;
                        hashTable[lowerName] = newId;
                        hashTable[lowerCode] = newId;

                        graph.addVertex();

                        cout << "||" << "\t CONFIGURATION   ||\t -> Setting up new network linkages..." << endl;
                        cout << "==" << "====================================================================" << endl;
                        cout << "   Enter number of connections to existing locations: ";
                        int numEdges; cin >> numEdges; cin.ignore(1000, '\n');
                        for (int i = 0; i < numEdges; i++) {
                            cout << "     Connection #" << i + 1 << ":\n";
                            int targetId = getValidLocation("Target location ID: ", newId);
                            cout << "     Distance in meters: ";
                            int dVal; cin >> dVal; cin.ignore(1000, '\n');
                            graph.addEdge(newId, targetId, dVal);
                        }
                        cout << "==" << "====================================================================" << endl;
                        cout << "||" << "\t SUCCESS         ||\t -> Created location entry under ID: " << newId << endl;
                    }
                    cout << "==" << "====================================================================" << endl;
                }
                else if (c == 3) {
                    int id = getValidLocation("Enter location ID to delete: ", locationNames.size());
                    string name = locationNames[id];
                    string code = shortCodes[id];

                    bst.erase(name);
                    string lowerName = name; transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                    string lowerCode = code; transform(lowerCode.begin(), lowerCode.end(), lowerCode.begin(), ::tolower);
                    hashTable.erase(lowerName);
                    hashTable.erase(lowerCode);

                    graph.removeVertex(id);

                    locationNames[id] = "";
                    shortCodes[id] = "";

                    cout << "==" << "====================================================================" << endl;
                    cout << "||" << "\t STATUS          ||\t -> Location ID: " << id << " (" << name << ") deleted safely." << endl;
                    cout << "==" << "====================================================================" << endl;
                }
                else if (c == 4) {
                    cout << "==" << "====================================================================" << endl;
                    cout << "||" << "\t\t\tDIRECTORY INDEX MAP (SORTED)" << endl;
                    cout << "==" << "====================================================================" << endl;
                    for (auto& pair : bst) {
                        cout << "|| " << "[" << setw(2) << pair.second << "] " << left << setw(25) << pair.first << " ||\t -> Active Record Entry" << endl;
                    }
                    cout << "==" << "====================================================================" << endl;
                }
                else {
                    cout << "==" << "====================================================================" << endl;
                    cout << "||" << "\t INPUT INVALID   ||\t -> Selection outside bounds [0-4]." << endl;
                    cout << "==" << "====================================================================" << endl;
                }

                if (inSubMenu5) {
                    cout << "\n   Press Enter to return to Location Management Menu...";
                    cin.get();
                }
            }
            break;
        }
        case 6: {
            bool inSubMenu6 = true;
            while (inSubMenu6) {
                cout << "\n==" << "====================================================================" << endl;
                cout << "||" << "\t\t      FAST ROUTING LOOKUP ENGINE (HASH TABLE)" << endl;
                cout << "==" << "====================================================================" << endl;
                cout << "||" << "\t [1] Fast Search Key Query" << endl;
                cout << "||" << "\t [2] View Structural Table Content Entries" << endl;
                cout << "||" << "\t [0] Back to Main Menu" << endl;
                cout << "==" << "====================================================================" << endl;
                cout << "   Enter Choice: ";
                int c;
                if (!(cin >> c)) { cin.clear(); cin.ignore(1000, '\n'); continue; }
                cin.ignore(1000, '\n');

                if (c == 0) {
                    inSubMenu6 = false;
                }
                else if (c == 1) {
                    cout << "   Enter search name or short code: "; string name; getline(cin, name);
                    string key = name; transform(key.begin(), key.end(), key.begin(), ::tolower);
                    auto it = hashTable.find(key);

                    cout << "==" << "====================================================================" << endl;
                    if (it != hashTable.end()) {
                        cout << "||" << "\t MATCH FOUND     ||\t -> ID: " << it->second << " | Name: " << locationNames[it->second] << endl;
                    }
                    else {
                        bool found = false;
                        for (auto& pair : hashTable) {
                            if (pair.first.find(key) != string::npos) {
                                cout << "||" << "\t PARTIAL MATCH   ||\t -> ID: " << pair.second << " | Name: " << locationNames[pair.second] << endl;
                                found = true; break;
                            }
                        }
                        if (!found) cout << "||" << "\t REGISTRATION    ||\t -> Search entry unlisted." << endl;
                    }
                    cout << "==" << "====================================================================" << endl;
                }
                else if (c == 2) {
                    cout << "==" << "====================================================================" << endl;
                    cout << "||" << "\t\t\tTOTAL MAP REGISTER LIST ENTRIES" << endl;
                    cout << "==" << "====================================================================" << endl;
                    for (auto& pair : hashTable) {
                        if (locationNames[pair.second].empty()) continue;
                        cout << "|| " << "Key: " << left << setw(20) << pair.first << " ||\t -> Resolved Value Mapping ID: " << pair.second << endl;
                    }
                    cout << "==" << "====================================================================" << endl;
                }
                else {
                    cout << "==" << "====================================================================" << endl;
                    cout << "||" << "\t INPUT INVALID   ||\t -> Selection outside bounds [0-2]." << endl;
                    cout << "==" << "====================================================================" << endl;
                }

                if (inSubMenu6) {
                    cout << "\n   Press Enter to return to Fast Lookup Menu...";
                    cin.get();
                }
            }
            break;
        }
        case 7: {
            bool inSubMenu7 = true;
            while (inSubMenu7) {
                cout << "\n==" << "====================================================================" << endl;
                cout << "||" << "\t\t\tNAVIGATION HISTORICAL INTERFACE" << endl;
                cout << "==" << "====================================================================" << endl;
                cout << "||" << "\t [1] View Tracking Stack History Logs" << endl;
                cout << "||" << "\t [2] Undo & Revoke Last Navigation Search Route" << endl;
                cout << "||" << "\t [0] Back to Main Menu" << endl;
                cout << "==" << "====================================================================" << endl;
                cout << "   Enter Choice: ";
                int c;
                if (!(cin >> c)) { cin.clear(); cin.ignore(1000, '\n'); continue; }
                cin.ignore(1000, '\n');

                if (c == 0) {
                    inSubMenu7 = false;
                }
                else if (c == 1) {
                    navStack.display();
                }
                else if (c == 2) {
                    cout << "==" << "====================================================================" << endl;
                    if (navStack.isEmpty()) {
                        cout << "||" << "\t STACK ERROR     ||\t -> Navigation record track logs empty." << endl;
                    }
                    else {
                        SearchRecord r = navStack.pop();
                        string srcName = (r.src >= 0 && r.src < (int)locationNames.size() && !locationNames[r.src].empty()) ? locationNames[r.src] : "Deleted Location";
                        string destName = (r.dest >= 0 && r.dest < (int)locationNames.size() && !locationNames[r.dest].empty()) ? locationNames[r.dest] : "Deleted Location";
                        cout << "||" << "\t POPPED ENTRY    ||\t -> Removed: " << srcName << " -> " << destName << " (" << r.distance << " m)" << endl;
                    }
                    cout << "==" << "====================================================================" << endl;
                }
                else {
                    cout << "==" << "====================================================================" << endl;
                    cout << "||" << "\t INPUT INVALID   ||\t -> Selection outside bounds [0-2]." << endl;
                    cout << "==" << "====================================================================" << endl;
                }

                if (inSubMenu7) {
                    cout << "\n   Press Enter to return to Navigation History Menu...";
                    cin.get();
                }
            }
            break;
        }
        case 8: {
            bool inSubMenu8 = true;
            while (inSubMenu8) {
                cout << "\n==" << "====================================================================" << endl;
                cout << "||" << "\t\t\tCAMPUS SYSTEM INCOMING REQUEST QUEUE" << endl;
                cout << "==" << "====================================================================" << endl;
                cout << "||" << "\t [1] Dispatch/File New Campus Request Ticket" << endl;
                cout << "||" << "\t [2] Dequeue & Process Next Awaiting Request Entry" << endl;
                cout << "||" << "\t [3] Print Out Current Pending Queue Track Lines" << endl;
                cout << "||" << "\t [0] Back to Main Menu" << endl;
                cout << "==" << "====================================================================" << endl;
                cout << "   Enter Choice: ";
                int c;
                if (!(cin >> c)) { cin.clear(); cin.ignore(1000, '\n'); continue; }
                cin.ignore(1000, '\n');

                if (c == 0) {
                    inSubMenu8 = false;
                }
                else if (c == 1) {
                    cout << "   User ID: "; int uid; cin >> uid; cin.ignore(1000, '\n');
                    cout << "   Query  : "; string q; getline(cin, q);
                    int rid = reqQueue.enqueue(uid, q);
                    cout << "==" << "====================================================================" << endl;
                    cout << "||" << "\t QUEUE SUCCESS   ||\t -> Request Entry ticket #" << rid << " issued successfully." << endl;
                    cout << "==" << "====================================================================" << endl;
                }
                else if (c == 2) {
                    UserRequest r = reqQueue.dequeue();
                    cout << "==" << "====================================================================" << endl;
                    if (r.requestId == -1) cout << "||" << "\t QUEUE ERROR     ||\t -> No pending tickets available to process." << endl;
                    else cout << "||" << "\t PROCESSING REQ  ||\t -> Ticket #" << r.requestId << " | Request Details: " << r.query << endl;
                    cout << "==" << "====================================================================" << endl;
                }
                else if (c == 3) {
                    reqQueue.display();
                }
                else {
                    cout << "==" << "====================================================================" << endl;
                    cout << "||" << "\t INPUT INVALID   ||\t -> Selection outside bounds [0-3]." << endl;
                    cout << "==" << "====================================================================" << endl;
                }

                if (inSubMenu8) {
                    cout << "\n   Press Enter to return to Request Queue Menu...";
                    cin.get();
                }
            }
            break;
        }
        case 9: {
            bool inSubMenu9 = true;
            while (inSubMenu9) {
                cout << "\n==" << "====================================================================" << endl;
                cout << "||" << "\t\t\tCAMPUS LOCATION SORTING MODULE" << endl;
                cout << "==" << "====================================================================" << endl;
                cout << "||" << "\t [1] Sort Alphabetically (A-Z Name Order)" << endl;
                cout << "||" << "\t [2] Sort Linearly By Proximity Distance From a Location" << endl;
                cout << "||" << "\t [0] Back to Main Menu" << endl;
                cout << "==" << "====================================================================" << endl;
                cout << "   Enter Choice: ";
                int c;
                if (!(cin >> c)) { cin.clear(); cin.ignore(1000, '\n'); continue; }
                cin.ignore(1000, '\n');

                if (c == 0) {
                    inSubMenu9 = false;
                }
                else if (c == 1) {
                    vector<LocationInfo> locs;
                    for (int i = 0; i < (int)locationNames.size(); i++) {
                        if (locationNames[i].empty()) continue;
                        locs.push_back({ i, locationNames[i], 0 });
                    }
                    sort(locs.begin(), locs.end(), [](const LocationInfo& a, const LocationInfo& b) {
                        return a.name < b.name;
                        });

                    cout << "==" << "====================================================================" << endl;
                    cout << "||" << "\t\t       LOCATIONS ALPHABETICALLY SORTED (A-Z)" << endl;
                    cout << "==" << "====================================================================" << endl;
                    for (auto& l : locs) {
                        cout << "|| " << "[" << setw(2) << l.id << "] " << left << setw(25) << l.name << " ||\t -> Verified Alpha Entry" << endl;
                    }
                    cout << "==" << "====================================================================" << endl;
                }
                else if (c == 2) {
                    printAllLocations();
                    int src = getValidLocation("Reference location ID: ", locationNames.size());

                    int n = locationNames.size();
                    vector<int> dist(n), prev(n);
                    graph.dijkstra(src, dist, prev);

                    vector<LocationInfo> locs;
                    for (int i = 0; i < (int)locationNames.size(); i++) {
                        if (locationNames[i].empty()) continue;
                        locs.push_back({ i, locationNames[i], (dist[i] == INF) ? 999999 : dist[i] });
                    }

                    sort(locs.begin(), locs.end(), [](const LocationInfo& a, const LocationInfo& b) {
                        return a.dist < b.dist;
                        });

                    cout << "==" << "====================================================================" << endl;
                    cout << "||" << "\t\t   SORTED BY RADIAL PROXIMITY DISTANCE" << endl;
                    cout << "==" << "====================================================================" << endl;
                    cout << "|| Reference Node: " << locationNames[src] << endl;
                    cout << "==" << "====================================================================" << endl;
                    for (auto& l : locs) {
                        cout << "|| " << "[" << setw(2) << l.id << "] " << left << setw(24) << l.name << " ||\t -> ";
                        if (l.dist == 999999) cout << "Unreachable Path Link" << endl;
                        else cout << l.dist << " meters away" << endl;
                    }
                    cout << "==" << "====================================================================" << endl;
                }
                else {
                    cout << "==" << "====================================================================" << endl;
                    cout << "||" << "\t INPUT INVALID   ||\t -> Selection outside bounds [0-2]." << endl;
                    cout << "==" << "====================================================================" << endl;
                }

                if (inSubMenu9) {
                    cout << "\n   Press Enter to return to Sorting Menu...";
                    cin.get();
                }
            }
            break;
        }
        case 10: {
            vector<LocationInfo> locs;
            for (int i = 0; i < (int)locationNames.size(); i++) {
                if (locationNames[i].empty()) continue; // Skip deleted
                locs.push_back({ i, locationNames[i], 0 });
            }
            sort(locs.begin(), locs.end(), [](const LocationInfo& a, const LocationInfo& b) {
                return a.name < b.name;
                });
            cout << "   Enter target location string for Binary Search: "; string target; getline(cin, target);

            auto it = lower_bound(locs.begin(), locs.end(), target, [](const LocationInfo& a, const string& val) {
                string s1 = a.name, s2 = val;
                transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
                transform(s2.begin(), s2.end(), s2.begin(), ::tolower);
                return s1 < s2;
                });

            cout << "==" << "====================================================================" << endl;
            cout << "||" << "\t\t      DIVIDE-AND-CONQUER BINARY LOOKUP SEARCH" << endl;
            cout << "==" << "====================================================================" << endl;
            if (it != locs.end()) {
                string s1 = it->name, s2 = target;
                transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
                transform(s2.begin(), s2.end(), s2.begin(), ::tolower);
                if (s1 == s2) {
                    int idx = distance(locs.begin(), it);
                    cout << "||" << "\t MATCH FOUND     ||\t -> Sorted Rank Index [" << idx << "]: ID=" << it->id << " | " << it->name << endl;
                }
                else {
                    bool found = false;
                    string t = target; transform(t.begin(), t.end(), t.begin(), ::tolower);
                    for (auto& l : locs) {
                        string s = l.name; transform(s.begin(), s.end(), s.begin(), ::tolower);
                        if (s.find(t) != string::npos) {
                            cout << "||" << "\t SUB-STRING MATCH||\t -> Partial Hit: ID=" << l.id << " | " << l.name << endl;
                            found = true;
                        }
                    }
                    if (!found) cout << "||" << "\t SEARCH ERROR    ||\t -> Explicit target string could not be verified." << endl;
                }
            }
            else {
                cout << "||" << "\t SEARCH ERROR    ||\t -> Query entry returned empty out-of-bounds index." << endl;
            }
            cout << "==" << "====================================================================" << endl;
            break;
        }
        case 11:
            printAllLocations();
            break;
        case 12: {
            printAllLocations();
            int id = getValidLocation("Enter location ID: ", locationNames.size());
            graph.showNeighbors(id);
            break;
        }
        case 13: {
            printAllLocations();
            int src = getValidLocation("Enter SOURCE location ID: ", locationNames.size());

            int n = locationNames.size();
            vector<int> dist(n), prev(n);
            graph.dijkstra(src, dist, prev);

            cout << "==" << "====================================================================" << endl;
            cout << "||" << "\t\t    DIJKSTRA DISTANCE SINGLE SOURCE MATRIX VECTOR" << endl;
            cout << "==" << "====================================================================" << endl;
            cout << "|| From Node Reference Source Point: " << locationNames[src] << endl;
            cout << "==" << "====================================================================" << endl;
            for (int i = 0; i < (int)locationNames.size(); i++) {
                if (locationNames[i].empty()) continue; // Skip deleted
                cout << "|| " << "[" << setw(2) << i << "] " << left << setw(23) << locationNames[i] << " ||\t -> ";
                if (dist[i] == INF) cout << "Link Unreachable" << endl;
                else cout << dist[i] << " meters (~" << dist[i] / 80 << " min walk)" << endl;
            }
            cout << "==" << "====================================================================" << endl;
            break;
        }
        case 14: {
            simulationMode = !simulationMode;
            cout << "==" << "====================================================================" << endl;
            cout << "||" << "\t CLIMATE TOGGLE  ||\t -> Weather/Traffic Simulation: " << (simulationMode ? "ENABLED" : "DISABLED") << endl;
            cout << "==" << "====================================================================" << endl;
            if (simulationMode) {
                cout << "||" << "\t HOSTEL AREAS    ||\t -> Muddy terrain delay multiplier triggered (1.5x weights)." << endl;
                cout << "||" << "\t SHUTTLE ROUTES  ||\t -> Heavy main bus lines delay applied (1.3x weights)." << endl;
            }
            else {
                cout << "||" << "\t RESTORATION     ||\t -> Resetted campus map edge calculations to default state." << endl;
            }
            cout << "==" << "====================================================================" << endl;
            break;
        }
        case 15:
            showASCIIMap();
            break;
        case 0:
            cout << "==" << "====================================================================" << endl;
            cout << "||" << "\t Thank you for using UET KSK Campus Management System!      ||" << endl;
            cout << "||" << "\t                      UET Kala Shah Kaku                    ||" << endl;
            cout << "==" << "====================================================================" << endl;
            running = false;
            break;
        default:
            cout << "==" << "====================================================================" << endl;
            cout << "||" << "\t INPUT INVALID   ||\t -> Option outside bounds. Select choice from [0-15]." << endl;
            cout << "==" << "====================================================================" << endl;
        }

        if (running) {
            cout << "\n   Press Enter to safely continue execution loop...";
            cin.get();
        }
    }
    return 0;
}