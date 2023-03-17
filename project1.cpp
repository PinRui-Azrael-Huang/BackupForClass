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

void BFS(Network &network, int src, int dst, int dstID) {
  queue<unsigned int> bfsQueue;
  bool visited[network.numNodes];
  int dist[network.numNodes];
  int parent[network.numNodes];

  for (int i = 0; i < network.numNodes; i++) {
    dist[i] = INT_MAX;
    parent[i] = -1;
  }
  memset(visited, false, sizeof(bool) * network.numNodes);
  // since the dummy source node is the last element in the adjList, i.e. the
  // index will be = numNodes.
  bfsQueue.push(src);
  visited[src] = true;

  int cur;
  while (!bfsQueue.empty()) {
    cur = bfsQueue.front();
    bfsQueue.pop();

    for (unsigned int neighbor : network.adjList[cur]) {
      if (!visited[neighbor]) {
        bfsQueue.push(neighbor);
        visited[neighbor] = true;
        dist[neighbor] = dist[cur] + 1;
        parent[neighbor] = cur;
        if (neighbor == dst) break;
      }
    }
  }
  cur = dst;
  vector<unsigned int> path;
  while (cur != src) {
    path.push_back(cur);
    cur = parent[cur];
  }
  reverse(path.begin(), path.end());
  network.nodeList[src].pathToDst[dstID].dst = dst;
  network.nodeList[src].pathToDst[dstID].path = path;

  vector<pair<unsigned int, double>> routingPath;
  unsigned int nextID = network.nodeList[src].pathToDst[dstID].path.front();
  routingPath.push_back(pair<unsigned int, double>(nextID, 1.0));
  network.nodeList[src].routingTable.insert(
      pair<unsigned int, vector<pair<unsigned int, double>>>(dst, routingPath));
}

// Using BFS to find the shortest path for all destinations.
void shortestPath(Network &network) {
  for (int i = 0; i < network.numNodes; i++) {
    for (int j = 0; j < network.numDst; j++) {
      unsigned int dst = network.dstIDList[j];
      if (i != dst) {
        BFS(network, i, dst, j);
      } else {
        network.nodeList[i].pathToDst[j].dst = dst;
        network.nodeList[i].pathToDst[j].path.push_back(dst);
        vector<pair<unsigned int, double>> routingPath;
        unsigned int nextID = network.nodeList[i].pathToDst[j].path.front();
        routingPath.push_back(pair<unsigned int, double>(nextID, 1.0));
        network.nodeList[i].routingTable.insert(
            pair<unsigned int, vector<pair<unsigned int, double>>>(
                dst, routingPath));
      }
    }
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

        iter = network.nodeList[neighbor].routingTable.find(curDstID);

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
      for (int k = 0; k < iter->second.size(); k++) {
        if (iter->second[k].second == 1.0) {
          cout << iter->second[k].first << endl;
          break;
        } else {
          if (k != iter->second.size() - 1)
            cout << iter->second[k].first << " "
                 << (int)(iter->second[k].second * 100.0) << "% ";
          else
            cout << iter->second[k].first << " "
                 << (int)(iter->second[k].second * 100.0) << "%" << endl;
        }
      }
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
  appendPath(*network);
  printSol(*network);

  system("PAUSE");
  return 0;
}