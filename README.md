# 🏫 Smart Campus Navigation & Management System (UET KSK)

A high-performance, graph-theoretic campus routing and structural information dashboard engineered in C++. Specifically modeled after the physical topology of the **University of Engineering and Technology (UET) Lahore, KSK Campus**, this system integrates advanced pathfinding algorithms with discrete data structure components to optimize pedestrian transit, coordinate system request tickets, and index spatial directory records.

---

## 🛠️ Data Structures & Complexity Blueprint

This platform maps real-world campus logistics into interconnected custom and standard data structures, balancing memory overhead against rapid execution constraints:

| System Module | Backing Data Structure | Underlying Mechanism & Utility | Time Complexity |
| :--- | :--- | :--- | :--- |
| **Network Topology** | `Adjacency List Graph` | Models the physical KSK campus paths using dynamically sized vector vectors of weight pairs. | $O(V + E)$ |
| **Pathfinding Engine** | `Min-Priority Queue` | Drives Dijkstra's Algorithm using priority heaps to extract the closest spatial node. | $O(E \log V)$ |
| **Fast Lookup Directory**| `Chained Hash Table` | Provides instantaneous index mapping for both full-string names and specialized short codes. | $O(1)$ Average |
| **Sorted Directory** | `Binary Search Tree Map` | Organizes spatial nodes lexicographically to enable alphabetized printing and rank tracking. | $O(\log n)$ Search |
| **Navigation History** | `File-Persistent Stack` | Captures transactional route histories under a LIFO protocol with automated flat-file persistence. | $O(1)$ Push/Pop |
| **System Ticket Line** | `FIFO Request Queue` | Schedules inbound administrative tasks or security tickets sequentially. | $O(1)$ Enqueue |

---

## 📂 Project Architecture


```text
├── Campus Navigation Core
│   ├── CampusGraph          # Encapsulates Dijkstra navigation loops, alternative routing, BFS, and DFS
│   └── buildCampusGraph     # Factory injector mapping the true layout distances of the UET KSK Campus
├── Data Management & Lookups
│   ├── NavigationStack      # Tracks historical search patterns with flat-file I/O synchronization
│   └── RequestQueue         # Manages multi-actor incoming ticket data arrays under FIFO constraints
├── Structural Data Models
│   ├── SearchRecord         # Explicit layout for saving source/destination history records
│   ├── UserRequest          # Tracks administrative parameters, user IDs, and operational query tokens
│   └── LocationInfo         # Holds contextual distance keys for sorting and binary lookups
└── Driver Workspace
    └── main()               # Runs the central console matrix and recursive execution loops
