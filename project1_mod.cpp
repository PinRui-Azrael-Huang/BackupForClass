#include "bits/stdc++.h"

using namespace std;

class Flow {
 public:
  unsigned int id, src, dst, size;
};

class Path {
 public:
  unsigned int dst;
  vector<unsigned int> path;
};

class Node {
 public:
  unsigned int id;
  bool isSDN;
  vector<unsigned int> neighborList;
  Path *pathToDst;
  map<unsigned int, vector<pair<unsigned int, double>>> routingTable;
};

class Network {
 public:
  unsigned int numNodes, numSDN, numDst, numLinks, numFlows;
  Node *nodeList;
  unsigned int *dstIDList;
  Flow *flowList;
  list<unsigned int> *adjList;
  vector<unsigned int> sdnNodeList;
};

// Process the input for the network, storing all necessary infos before
// computation.
Network *processInput() {
  Network *network = new Network;
  int numNodes, numSDN, numDst, numLinks, numFlows;
  cin >> numNodes >> numSDN >> numDst >> numLinks >> numFlows;

  network->numNodes = numNodes;
  network->numSDN = numSDN;
  network->numDst = numDst;
  network->numLinks = numLinks;
  network->numFlows = numFlows;

  list<unsigned int> *adjList = new list<unsigned int>[numNodes];
  Node *nodeList = new Node[numNodes];
  for (int i = 0; i < numNodes; i++) {
    nodeList[i].id = i;
    nodeList[i].isSDN = false;
    nodeList[i].pathToDst = new Path[numDst];
  }
  // Process the Nodes that are SDNs;
  vector<unsigned int> sdnNodeList;
  sdnNodeList.resize(numSDN);
  unsigned int sdnID, dstID;
  for (int i = 0; i < numSDN; i++) {
    cin >> sdnID;
    nodeList[sdnID].isSDN = true;
    sdnNodeList[i] = sdnID;
  }
  // Process the list of destinations;
  unsigned int *dstIDList = new unsigned int[numDst];
  for (int i = 0; i < numDst; i++) {
    cin >> dstID;
    dstIDList[i] = dstID;
  }

  // Process all links for the network, also compute graph (in the form of
  // adjacency list)
  unsigned int _linkID, node1, node2;
  for (int i = 0; i < numLinks; i++) {
    cin >> _linkID >> node1 >> node2;
    nodeList[node1].neighborList.push_back(node2);
    nodeList[node2].neighborList.push_back(node1);
    adjList[node1].push_back(node2);
    adjList[node2].push_back(node1);
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

  network->nodeList = nodeList;
  network->dstIDList = dstIDList;
  network->flowList = flowList;
  network->adjList = adjList;
  network->sdnNodeList = sdnNodeList;

  return network;
}

void BFS(Network &network, int dst, int dstID) {
  queue<unsigned int> bfsQueue;
  bool visited[network.numNodes];
  int parent[network.numNodes];

  for (int i = 0; i < network.numNodes; i++) {
    parent[i] = -1;
  }
  memset(visited, false, sizeof(bool) * network.numNodes);
  bfsQueue.push(dst);
  parent[dst] = dst;
  visited[dst] = true;

  int cur;
  while (!bfsQueue.empty()) {
    cur = bfsQueue.front();
    bfsQueue.pop();

    for (unsigned int neighbor : network.adjList[cur]) {
      if (!visited[neighbor]) {
        bfsQueue.push(neighbor);
        visited[neighbor] = true;
        parent[neighbor] = cur;
        if (neighbor == dst) break;
      }
    }
  }

  for (int i = 0; i < network.numNodes; i++) {
    vector<pair<unsigned int, double>> routingPath;
    unsigned int nextID = parent[i];
    routingPath.push_back(pair<unsigned int, double>(nextID, 1.0));
    network.nodeList[i].routingTable.insert(
        pair<unsigned int, vector<pair<unsigned int, double>>>(dst,
                                                               routingPath));
  }
}

// Using BFS to find the shortest path for all destinations.
void shortestPath(Network &network) {
  for (int j = 0; j < network.numDst; j++) {
    unsigned int dst = network.dstIDList[j];
    BFS(network, dst, j);
  }
}

// Append the path for SDN node based on the shortest path network for every
// destination.
void appendPath(Network &network) {
  unsigned int curDstID;
  unsigned int curSDNID;
  map<unsigned int, vector<pair<unsigned int, double>>>::iterator iter;
  for (int i = 0; i < network.numDst; i++) {
    curDstID = network.dstIDList[i];
    for (int j = 0; j < network.numSDN; j++) {
      curSDNID = network.sdnNodeList[j];
      for (unsigned int neighbor : network.nodeList[curSDNID].neighborList) {
        // if the current neighbor's outgoing node is the current SDN node,
        // then do nothing.
        iter = network.nodeList[neighbor].routingTable.find(curDstID);
        if (iter != network.nodeList[neighbor].routingTable.end() &&
            iter->second.front().first == curSDNID) {
          continue;
        }
        // IF the current neighbor is also an SDN node
        // AND it already has the path towards the current SDN node
        // then do nothing
        if (network.nodeList[neighbor].isSDN &&
            iter != network.nodeList[neighbor].routingTable.end() &&
            find(iter->second.begin(), iter->second.end(),
                 pair<unsigned int, double>(curSDNID, 1)) !=
                iter->second.end()) {
          continue;
        }
        // if the current neighbor is the outgoing node of current SDN node,
        // then do nothing.
        iter = network.nodeList[curSDNID].routingTable.find(curDstID);
        if (iter != network.nodeList[curSDNID].routingTable.end() &&
            iter->second.front().first == neighbor) {
          continue;
        }
        // Other than the cases above, the rest are all available paths for the
        // SDN node to append.
        vector<pair<unsigned int, double>> path;
        network.nodeList[curSDNID].routingTable[curDstID].push_back(
            pair<unsigned int, double>(neighbor, 1));
      }

      // Setting default proportion of each SDN node.
      unsigned int numOut =
          network.nodeList[curSDNID].routingTable[curDstID].size();
      double defaultPerct = (1.0 / (double)numOut);
      defaultPerct = round(defaultPerct * 100.0) / 100.0;
      int totalPerc, remainder;
      for (pair<unsigned int, double> &outgoing :
           network.nodeList[curSDNID].routingTable[curDstID]) {
        outgoing.second = defaultPerct;
      }
      if (defaultPerct * numOut != 1.0) {
        totalPerc = round(defaultPerct * numOut * 100.0);
        remainder = 100 - totalPerc;
        network.nodeList[curSDNID].routingTable[curDstID].front().second +=
            (double)remainder / 100.0;
      }
    }
  }
}

void printSol(const Network &network) {
  map<unsigned int, vector<pair<unsigned int, double>>>::iterator iter;
  for (int i = 0; i < network.numNodes; i++) {
    cout << network.nodeList[i].id << endl;
    for (iter = network.nodeList[i].routingTable.begin();
         iter != network.nodeList[i].routingTable.end(); iter++) {
      cout << iter->first << " ";
      cout << iter->second.front().first;
      if (network.nodeList[i].isSDN)
        cout << " 100%" << endl;
      else
        cout << endl;
    }
  }
}

// A generic check function to see the correctness of the input process.
void printInput(Network *network) {
  cout << "Number of Nodes: " << network->numNodes << endl;
  cout << "Number of Destinations: " << network->numDst << endl;
  cout << "Number of SDNs: " << network->numSDN << endl;

  cout << "The ID of SDN nodes are: ";
  for (int i = 0; i < network->numNodes; i++) {
    if (network->nodeList[i].isSDN) cout << network->nodeList[i].id << " ";
  }
  cout << endl;

  cout << "The ID of dst nodes are: ";
  for (int i = 0; i < network->numDst; i++) {
    cout << network->dstIDList[i];
  }
  cout << endl;

  for (int i = 0; i < network->numNodes; i++) {
    cout << "The Node ID: " << network->nodeList[i].id
         << " has the neighbors: ";
    for (int j = 0; j < network->nodeList[i].neighborList.size(); j++) {
      cout << network->nodeList[i].neighborList[j] << " ";
    }
    cout << endl;
  }
  cout << endl;

  cout << "Flow ID\tSource\tDestination\tSize" << endl;
  for (int i = 0; i < network->numFlows; i++) {
    cout << network->flowList[i].id << "\t" << network->flowList[i].src << "\t"
         << network->flowList[i].dst << "\t" << network->flowList[i].size
         << "\t";
    cout << endl;
  }
  cout << endl;
}

int main() {
  Network *network = processInput();
  // printInput(network);
  shortestPath(*network);
  // appendPath(*network);
  printSol(*network);

  system("PAUSE");
  return 0;
}