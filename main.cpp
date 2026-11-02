#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <limits>
#include <climits>
using namespace std;

// Structure to represent a pathway
struct Edge {
    int destination;
    int distance;
};

// Structure to represent a warehouse task
struct Task {
    int id;
    int pickup;
    int destination;
    int priority;
};

// Priority queue comparator
struct CompareTask {
    bool operator()(Task a, Task b) {
        if (a.priority == b.priority)
            return a.id > b.id;
        return a.priority < b.priority;
    }
};

// Structure to represent a robot
struct Robot {
    int id;
    int location;
    int battery;
    int workload;
    string status;
};

// Warehouse Coordination System
class Warehouse {

    vector<string> locations = {
        "Storage", "Packing", "Loading",
        "Charging", "Receiving", "Dispatch"
    };

    vector<vector<Edge>> graph;

    vector<Robot> robots;

    queue<Task> normalTasks;

    priority_queue<Task, vector<Task>, CompareTask> urgentTasks;

    map<int, string> taskStatus;

    set<int> taskIDs;

    set<pair<int, int>> blockedPaths;

    set<pair<int, int>> occupiedPaths;

    int nextTaskID = 1;

public:

    Warehouse() {
        graph.resize(locations.size());

        // Creating warehouse pathways
        addPath(0, 1, 4);
        addPath(0, 4, 3);
        addPath(1, 2, 5);
        addPath(1, 3, 6);
        addPath(2, 5, 3);
        addPath(3, 4, 4);
        addPath(4, 5, 7);

        // Initial robot details
        robots.push_back({101, 0, 90, 0, "Available"});
        robots.push_back({102, 4, 80, 0, "Available"});
        robots.push_back({103, 3, 40, 0, "Available"});
    }

    // Add pathway to graph
    void addPath(int u, int v, int w) {
        graph[u].push_back({v, w});
        graph[v].push_back({u, w});
    }

    // Find location number
    int getLocation(string name) {
        for (int i = 0; i < locations.size(); i++) {
            if (locations[i] == name)
                return i;
        }
        return -1;
    }

    // Display warehouse locations
    void displayLocations() {
        cout << "\nWAREHOUSE LOCATIONS\n";

        for (int i = 0; i < locations.size(); i++) {
            cout << i << " - " << locations[i] << endl;
        }
    }

    // Display robot information
    void displayRobots() {
        cout << "\nROBOT DETAILS\n";

        for (auto r : robots) {
            cout << "\nRobot ID: " << r.id;
            cout << "\nLocation: " << locations[r.location];
            cout << "\nBattery: " << r.battery << "%";
            cout << "\nWorkload: " << r.workload;
            cout << "\nStatus: " << r.status << endl;
        }
    }

    // Add customer task
    void addTask(int pickup, int destination, int priority) {

        Task t = {
            nextTaskID++,
            pickup,
            destination,
            priority
        };

        taskIDs.insert(t.id);
        taskStatus[t.id] = "Pending";

        if (priority > 1) {
            urgentTasks.push(t);
            cout << "\nUrgent task added successfully!";
        }
        else {
            normalTasks.push(t);
            cout << "\nNormal task added successfully!";
        }

        cout << "\nTask ID: " << t.id << endl;
    }

    // Find shortest path using Dijkstra's algorithm
    vector<int> dijkstra(int source, int destination) {

        int n = locations.size();

        vector<int> dist(n, INT_MAX);
        vector<int> parent(n, -1);

        priority_queue<
            pair<int, int>,
            vector<pair<int, int>>,
            greater<pair<int, int>>
        > pq;

        dist[source] = 0;
        pq.push({0, source});

        while (!pq.empty()) {

            int u = pq.top().second;
            pq.pop();

            for (auto edge : graph[u]) {

                int v = edge.destination;
                int w = edge.distance;

                pair<int, int> path =
                    {min(u, v), max(u, v)};

                if (blockedPaths.count(path))
                    continue;

                if (dist[u] != INT_MAX &&
                    dist[u] + w < dist[v]) {

                    dist[v] = dist[u] + w;
                    parent[v] = u;

                    pq.push({dist[v], v});
                }
            }
        }

        vector<int> path;

        if (dist[destination] == INT_MAX)
            return path;

        for (int v = destination; v != -1; v = parent[v])
            path.push_back(v);

        reverse(path.begin(), path.end());

        return path;
    }

    // BFS traversal
    void bfs(int source) {

        vector<bool> visited(locations.size(), false);

        queue<int> q;

        visited[source] = true;
        q.push(source);

        cout << "\nBFS Traversal: ";

        while (!q.empty()) {

            int u = q.front();
            q.pop();

            cout << locations[u] << " -> ";

            for (auto edge : graph[u]) {

                int v = edge.destination;

                pair<int, int> path =
                    {min(u, v), max(u, v)};

                if (!blockedPaths.count(path) &&
                    !visited[v]) {

                    visited[v] = true;
                    q.push(v);
                }
            }
        }

        cout << "END\n";
    }

    // Display shortest route
    void displayRoute(int source, int destination) {

        vector<int> path = dijkstra(source, destination);

        if (path.empty()) {
            cout << "\nNo available route found!\n";
            return;
        }

        cout << "\nShortest Route: ";

        for (int i = 0; i < path.size(); i++) {

            cout << locations[path[i]];

            if (i != path.size() - 1)
                cout << " -> ";
        }

        int distance = 0;

        for (int i = 0; i < path.size() - 1; i++) {

            int u = path[i];
            int v = path[i + 1];

            for (auto e : graph[u]) {
                if (e.destination == v)
                    distance += e.distance;
            }
        }

        cout << "\nTotal Distance: " << distance << endl;
    }

    // Find suitable robot
    int selectRobot(int pickup) {

        int selected = -1;
        int minimumDistance = INT_MAX;

        for (int i = 0; i < robots.size(); i++) {

            if (robots[i].status != "Available")
                continue;

            if (robots[i].battery < 15)
                continue;

            vector<int> path =
                dijkstra(robots[i].location, pickup);

            if (path.empty())
                continue;

            int distance = 0;

            for (int j = 0; j < path.size() - 1; j++) {

                for (auto e : graph[path[j]]) {
                    if (e.destination == path[j + 1])
                        distance += e.distance;
                }
            }

            if (distance < minimumDistance) {
                minimumDistance = distance;
                selected = i;
            }
        }

        return selected;
    }

    // Move robot along the selected route
    bool moveRobot(int index, int destination) {

        int source = robots[index].location;

        vector<int> path = dijkstra(source, destination);

        if (path.empty())
            return false;

        int totalDistance = 0;

        for (int i = 0; i < path.size() - 1; i++) {

            int u = path[i];
            int v = path[i + 1];

            pair<int, int> road =
                {min(u, v), max(u, v)};

            for (auto e : graph[u]) {
                if (e.destination == v)
                    totalDistance += e.distance;
            }

            if (occupiedPaths.count(road)) {
                cout << "\nPath occupied. Robot must wait.\n";
                return false;
            }
        }

        if (robots[index].battery < totalDistance) {
            cout << "\nInsufficient battery. Charging required.\n";
            robots[index].status = "Charging";
            return false;
        }

        robots[index].status = "Moving";

        for (int i = 0; i < path.size() - 1; i++) {

            int u = path[i];
            int v = path[i + 1];

            pair<int, int> road =
                {min(u, v), max(u, v)};

            occupiedPaths.insert(road);

            cout << locations[u] << " -> "
                 << locations[v] << endl;

            occupiedPaths.erase(road);
        }

        robots[index].location = destination;
        robots[index].battery -= totalDistance;

        return true;
    }

    // Process customer orders
    void processTask() {

        if (urgentTasks.empty() && normalTasks.empty()) {
            cout << "\nNo pending tasks!\n";
            return;
        }

        Task t;

        if (!urgentTasks.empty()) {
            t = urgentTasks.top();
            urgentTasks.pop();
            cout << "\nProcessing URGENT task";
        }
        else {
            t = normalTasks.front();
            normalTasks.pop();
            cout << "\nProcessing NORMAL task";
        }

        if (taskStatus[t.id] == "Completed") {
            cout << "\nTask already completed!\n";
            return;
        }

        int robotIndex = selectRobot(t.pickup);

        if (robotIndex == -1) {
            cout << "\nNo suitable robot available!\n";
            requeueTask(t);
            return;
        }

        cout << "\nTask ID: " << t.id;
        cout << "\nSelected Robot: "
             << robots[robotIndex].id << endl;

        taskStatus[t.id] = "Assigned";

        // Move to pickup location
        if (!moveRobot(robotIndex, t.pickup)) {
            requeueTask(t);
            return;
        }

        taskStatus[t.id] = "In Progress";

        // Move to destination
        if (!moveRobot(robotIndex, t.destination)) {
            requeueTask(t);
            return;
        }

        robots[robotIndex].status = "Available";
        robots[robotIndex].workload++;

        taskStatus[t.id] = "Completed";

        cout << "\nTask completed successfully!";
        cout << "\nTask Status: Completed\n";
    }

    // Return unfinished task to queue
    void requeueTask(Task t) {

        taskStatus[t.id] = "Pending";

        if (t.priority > 1)
            urgentTasks.push(t);
        else
            normalTasks.push(t);

        cout << "\nTask returned to pending queue.\n";
    }

    // Block a pathway
    void blockPath(int u, int v) {

        pair<int, int> road =
            {min(u, v), max(u, v)};

        blockedPaths.insert(road);

        cout << "\nPath blocked successfully!\n";
    }

    // Unblock a pathway
    void unblockPath(int u, int v) {

        pair<int, int> road =
            {min(u, v), max(u, v)};

        blockedPaths.erase(road);

        cout << "\nPath is now available!\n";
    }

    // Mark robot as failed
    void failRobot(int id) {

        for (auto &r : robots) {

            if (r.id == id) {
                r.status = "Failed";
                cout << "\nRobot marked as Failed!\n";
                return;
            }
        }

        cout << "\nRobot not found!\n";
    }

    // Recharge robot
    void rechargeRobot(int id) {

        for (auto &r : robots) {

            if (r.id == id) {

                r.battery = 100;
                r.status = "Available";

                cout << "\nRobot charged successfully!\n";
                return;
            }
        }

        cout << "\nRobot not found!\n";
    }

    // Display task status
    void displayTasks() {

        cout << "\nTASK STATUS\n";

        for (auto t : taskStatus) {
            cout << "Task ID: " << t.first
                 << " | Status: " << t.second << endl;
        }
    }

    // Performance summary
    void performance() {

        int completed = 0;
        int pending = 0;

        for (auto t : taskStatus) {

            if (t.second == "Completed")
                completed++;

            else if (t.second == "Pending")
                pending++;
        }

        cout << "\n--- PERFORMANCE REPORT ---\n";

        cout << "Total Tasks: " << taskStatus.size();
        cout << "\nCompleted Tasks: " << completed;
        cout << "\nPending Tasks: " << pending;
        cout << "\nMovement Conflicts: "
             << occupiedPaths.size();

        cout << "\n--------------------------\n";
    }
};

// Main function
int main() {

    Warehouse w;

    int choice;

    do {

        cout << "\n\n================================";
        cout << "\n ADAPTIVE WAREHOUSE COORDINATION";
        cout << "\n================================";

        cout << "\n1. Display Warehouse Locations";
        cout << "\n2. Display Robot Details";
        cout << "\n3. Add Normal Task";
        cout << "\n4. Add Urgent Task";
        cout << "\n5. Process Next Task";
        cout << "\n6. Find Shortest Route";
        cout << "\n7. BFS Traversal";
        cout << "\n8. Block Pathway";
        cout << "\n9. Unblock Pathway";
        cout << "\n10. Mark Robot as Failed";
        cout << "\n11. Recharge Robot";
        cout << "\n12. Display Task Status";
        cout << "\n13. Performance Report";
        cout << "\n0. Exit";

        cout << "\n\nEnter your choice: ";
        cin >> choice;

        int source, destination, id;
        string name1, name2;

        switch (choice) {

        case 1:
            w.displayLocations();
            break;

        case 2:
            w.displayRobots();
            break;

        case 3:
        case 4:

            w.displayLocations();

            cout << "\nEnter pickup location name: ";
            cin >> name1;

            cout << "Enter destination location name: ";
            cin >> name2;

            source = w.getLocation(name1);
            destination = w.getLocation(name2);

            if (source == -1 || destination == -1 ||
                source == destination) {
                cout << "\nInvalid locations!\n";
                break;
            }

            w.addTask(source, destination,
                      choice == 4 ? 2 : 1);
            break;

        case 5:
            w.processTask();
            break;

        case 6:

            w.displayLocations();

            cout << "\nEnter source location: ";
            cin >> name1;

            cout << "Enter destination location: ";
            cin >> name2;

            source = w.getLocation(name1);
            destination = w.getLocation(name2);

            if (source == -1 || destination == -1) {
                cout << "\nInvalid location!\n";
                break;
            }

            w.displayRoute(source, destination);
            break;

        case 7:

            w.displayLocations();

            cout << "\nEnter starting location: ";
            cin >> name1;

            source = w.getLocation(name1);

            if (source == -1) {
                cout << "\nInvalid location!\n";
                break;
            }

            w.bfs(source);
            break;

        case 8:
        case 9:

            w.displayLocations();

            cout << "\nEnter first location: ";
            cin >> name1;

            cout << "Enter second location: ";
            cin >> name2;

            source = w.getLocation(name1);
            destination = w.getLocation(name2);

            if (source == -1 || destination == -1) {
                cout << "\nInvalid locations!\n";
                break;
            }

            if (choice == 8)
                w.blockPath(source, destination);
            else
                w.unblockPath(source, destination);

            break;

        case 10:

            cout << "\nEnter Robot ID: ";
            cin >> id;

            w.failRobot(id);
            break;

        case 11:

            cout << "\nEnter Robot ID: ";
            cin >> id;

            w.rechargeRobot(id);
            break;

        case 12:
            w.displayTasks();
            break;

        case 13:
            w.performance();
            break;

        case 0:
            cout << "\nExiting Warehouse System...\n";
            break;

        default:
            cout << "\nInvalid choice! Try again.\n";
        }

    } while (choice != 0);

    return 0;
}
