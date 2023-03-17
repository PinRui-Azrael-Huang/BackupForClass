/* An Absolute Baseline Solution */

#include "bits/stdc++.h"

using namespace std;

// This is used to store all the information about every flow in the network.
// P.S. This is essentially insignificant for baseline solution.
class Flow {
 public:
  unsigned int id, src, dst, size;
  Flow(){};
};

// This is used to store all the information of a node in the network.
class Node {
 public:
  unsigned int id;
  // If an node is SDN node, this will be true
  bool isSDN;
  vector<unsigned int> neighborList;
  // {destination node       |  nextNodeID  , proportion }
  map<unsigned int, vector<pair<unsigned int, double>>> routingTable;
  Node(){};
};

// This is used to store all the information of a network
class Network {
 public:
  unsigned int numNodes, numSDN, numDst, numLinks, numFlows;
  // This is an array to store the list of all destinations (ID)
  unsigned int *dstIDList;
  // This is an array to store all the node in a network.
  Node *nodeList;
  Flow *flowList;
  Network(){};
};

// Create and process the input for the network, storing all necessary infos
// before computation.
Network *processInput() {
  Network *network = new Network;
  int numNodes, numSDN, numDst, numLinks, numFlows;
  cin >> numNodes >> numSDN >> numDst >> numLinks >> numFlows;

  network->numNodes = numNodes;
  network->numSDN = numSDN;
  network->numDst = numDst;
  network->numLinks = numLinks;
  network->numFlows = numFlows;

  Node *nodeList = new Node[numNodes];
  for (int i = 0; i < numNodes; i++) {
    nodeList[i].id = i;
    nodeList[i].isSDN = false;
  }

  // Process the Nodes that are SDNs;
  unsigned int sdnID, dstID;
  for (int i = 0; i < numSDN; i++) {
    cin >> sdnID;
    nodeList[sdnID].isSDN = true;
  }

  // Process the list of destinations;
  unsigned int *dstIDList = new unsigned int[numDst];
  for (int i = 0; i < numDst; i++) {
    cin >> dstID;
    dstIDList[i] = dstID;
  }

  // Process all links for the network, push the link into the corresponding
  // neighbor list
  unsigned int _linkID, node1, node2;
  for (int i = 0; i < numLinks; i++) {
    cin >> _linkID >> node1 >> node2;

    // Link : node1 <-> node 2
    nodeList[node1].neighborList.push_back(node2);
    nodeList[node2].neighborList.push_back(node1);
  }

  // Process the flows to be dealt with.
  Flow *flowList = new Flow[numFlows];
  unsigned int id, src, dst, flowSize;
  for (int i = 0; i < numFlows; i++) {
    cin >> id >> src >> dst >> flowSize;
    flowList[i].id = id;
    flowList[i].src = src;
    flowList[i].dst = dst;
    flowList[i].size = flowSize;
  }

  // Properly assign the info to the network.
  network->nodeList = nodeList;
  network->dstIDList = dstIDList;
  network->flowList = flowList;

  return network;
}

// Using generic BFS on the dst, and uses "parent array" to store the
// inheritance of the SP tree.
// i.e. "the parent node of node i" will be "the next node in the routing table
// of node i"
void BFS(Network &network, int dst, int dstID) {
  queue<unsigned int> bfsQueue;
  bool visited[network.numNodes];
  int parent[network.numNodes];

  for (int i = 0; i < network.numNodes; i++) {
    parent[i] = i;
  }
  memset(visited, false, sizeof(bool) * network.numNodes);
  bfsQueue.push(dst);
  visited[dst] = true;

  int cur;
  while (!bfsQueue.empty()) {
    cur = bfsQueue.front();
    bfsQueue.pop();

    for (unsigned int neighbor : network.nodeList[cur].neighborList) {
      if (!visited[neighbor]) {
        bfsQueue.push(neighbor);
        visited[neighbor] = true;
        parent[neighbor] = cur;
        if (neighbor == dst) break;
      }
    }
  }

  // After BFS, compute the routing table using the information
  // i.e. uses parent array to get the next node's ID for node i
  for (int i = 0; i < network.numNodes; i++) {
    vector<pair<unsigned int, double>> path;
    // The id of next node for node i is parent[i]
    unsigned int nextID = parent[i];
    // the default proportion of link is set to 1.0
    path.push_back(pair<unsigned int, double>(nextID, 1.0));
    network.nodeList[i].routingTable.insert(
        pair<unsigned int, vector<pair<unsigned int, double>>>(dst, path));
  }
}

// A funcion using BFS to find the shortest path for all destinations.
void shortestPath(Network &network) {
  for (int j = 0; j < network.numDst; j++) {
    unsigned int dst = network.dstIDList[j];
    BFS(network, dst, j);
  }
}

// Print the solution.
void printSol(const Network &network) {
  map<unsigned int, vector<pair<unsigned int, double>>>::iterator iter;
  for (int i = 0; i < network.numNodes; i++) {
    // This prints the node's ID
    cout << network.nodeList[i].id << endl;
    for (iter = network.nodeList[i].routingTable.begin();
         iter != network.nodeList[i].routingTable.end(); iter++) {
      //   " destination ID         next node ID  "
      cout << iter->first << " " << iter->second.front().first;
      // Only if a certain node is SDN do we need to print the proportion.
      if (network.nodeList[i].isSDN)
        // Since it's baseline, the proportion is always 100% (No other paths)
        cout << " 100%" << endl;
      else
        cout << endl;
    }
  }
}

int main() {
  Network *network = processInput();
  shortestPath(*network);
  printSol(*network);

  // system("PAUSE");
  delete network;
  return 0;
}