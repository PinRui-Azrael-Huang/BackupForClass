#include "bits/stdc++.h"

using namespace std;

class Flow {
 public:
  unsigned int id, src, dst, size;

  Flow(){};
};

class Node {
 public:
  unsigned int id;
  bool isSDN;
  vector<unsigned int> neighborList;
  vector<unsigned int> packetCount;
  map<unsigned int, vector<pair<unsigned int, double>>> routingTable;
  unsigned int totalFlow;

  Node(){};
};

class Network {
 public:
  unsigned int numNodes, numSDN, numDst, numLinks, numFlows;
  unsigned int* dstIDList;
  vector<unsigned int> sdnNodeList;
  Node* nodeList;
  Flow* flowList;

  Network(){};
};

// Process the input for the network, storing all necessary infos before
// computation.
Network* processInput() {
  Network* network = new Network;
  int numNodes, numSDN, numDst, numLinks, numFlows;
  cin >> numNodes >> numSDN >> numDst >> numLinks >> numFlows;

  network->numNodes = numNodes;
  network->numSDN = numSDN;
  network->numDst = numDst;
  network->numLinks = numLinks;
  network->numFlows = numFlows;

  Node* nodeList = new Node[numNodes];
  for (int i = 0; i < numNodes; i++) {
    nodeList[i].id = i;
    nodeList[i].isSDN = false;
    nodeList[i].packetCount.resize(numDst, 0);
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
  unsigned int* dstIDList = new unsigned int[numDst];
  for (int i = 0; i < numDst; i++) {
    cin >> dstID;
    dstIDList[i] = dstID;
  }

  // Process all links for the network.
  unsigned int _linkID, node1, node2;
  for (int i = 0; i < numLinks; i++) {
    cin >> _linkID >> node1 >> node2;
    nodeList[node1].neighborList.push_back(node2);
    nodeList[node2].neighborList.push_back(node1);
  }

  // Process the flows to be dealt with.
  Flow* flowList = new Flow[numFlows];
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
  network->sdnNodeList = sdnNodeList;

  return network;
}

void sendPacket(Network& network, int src, int dst, int currentCount,
                unsigned int dstID) {
  // Insert the node itself to the routing table to itself first.
  vector<pair<unsigned int, double>> defaultPath;
  defaultPath.push_back(pair<unsigned int, double>(dst, 1));
  network.nodeList[src].routingTable.insert(
      pair<unsigned int, vector<pair<unsigned int, double>>>(dst, defaultPath));

  map<unsigned int, vector<pair<unsigned int, double>>>::iterator iter;
  for (unsigned int neighbor : network.nodeList[src].neighborList) {
    // If the packet count of a neighbor > currentCount or is = 0.
    // Replace the packet count of that neighbor and update the routing table
    if (neighbor != dst &&
        (network.nodeList[neighbor].packetCount[dstID] == 0 ||
         network.nodeList[neighbor].packetCount[dstID] >= currentCount)) {
      // If the packetCount is the same for two paths, then choose the one node
      // with the lower ID.
      if (network.nodeList[neighbor].packetCount[dstID] == currentCount) {
        iter = network.nodeList[neighbor].routingTable.find(dst);
        if (iter == network.nodeList[neighbor].routingTable.end()) {
          vector<pair<unsigned int, double>> path;
          path.push_back(pair<unsigned int, double>(src, 1));
          network.nodeList[neighbor].routingTable.insert(
              pair<unsigned int, vector<pair<unsigned int, double>>>(dst,
                                                                     path));
          continue;
        }
        unsigned int currentID = iter->second.front().first;
        if (src < currentID) {
          iter->second.pop_back();
          iter->second.push_back(pair<unsigned int, double>(src, 1));
        }
      } else {
        network.nodeList[neighbor].packetCount[dstID] = currentCount;
        // If there isn't any path to the dst in the routing table yet
        // Then create one routing path for the routing table
        if (network.nodeList[neighbor].routingTable.find(dst) ==
            network.nodeList[neighbor].routingTable.end()) {
          vector<pair<unsigned int, double>> path;
          path.push_back(pair<unsigned int, double>(src, 1));
          network.nodeList[neighbor].routingTable.insert(
              pair<unsigned int, vector<pair<unsigned int, double>>>(dst,
                                                                     path));
        }
        // There already exists a path to the dst.
        // Needs to update the routing path in the table.
        else {
          iter = network.nodeList[neighbor].routingTable.find(dst);
          iter->second.pop_back();
          iter->second.push_back(pair<unsigned int, double>(src, 1));
        }
      }
      // Recursively send the packet outwards, similar to the concept of BFS.
      sendPacket(network, neighbor, dst, currentCount + 1, dstID);
    } else {
      continue;
    }
  }
}

// Using BFS to find the shortest path for all destinations.
void shortestPath(Network& network) {
  unsigned int dst;
  unsigned int packetCount = 1;
  for (int i = 0; i < network.numDst; i++) {
    dst = network.dstIDList[i];
    sendPacket(network, dst, dst, packetCount, i);
  }
}

// Append the path for SDN node based on the shortest path network for every
// destination.
void appendPath(Network& network) {
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
        // if the current neighbor is the outgoing node of current SDN node,
        // then do nothing.
        iter = network.nodeList[curSDNID].routingTable.find(curDstID);
        if (iter != network.nodeList[curSDNID].routingTable.end() &&
            iter->second.front().first == neighbor) {
          continue;
        }
        // IF the current neighbor is also an SDN node
        // AND it already has the path towards the current SDN node
        // then do nothing
        iter = network.nodeList[neighbor].routingTable.find(curDstID);
        if (network.nodeList[neighbor].isSDN &&
            iter != network.nodeList[neighbor].routingTable.end() &&
            find(iter->second.begin(), iter->second.end(),
                 pair<unsigned int, double>(curSDNID, 1)) !=
                iter->second.end()) {
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
      double defaultPerct = (1.0 / numOut);
      int totalPerc, remainder;
      if (defaultPerct * numOut * 100.0 != 100.0) {
        totalPerc = (int)(defaultPerct * numOut * 100.0);
        remainder = 100 - totalPerc;
      }
      for (pair<unsigned int, double>& outgoing :
           network.nodeList[curSDNID].routingTable[curDstID]) {
        outgoing.second = defaultPerct;
      }
      network.nodeList[curSDNID].routingTable[curDstID].front().second +=
          remainder;
    }
  }
}

void printSol(const Network& network) {
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
void printInput(Network* network) {
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
  Network* network = processInput();
  // printInput(network);
  shortestPath(*network);
  appendPath(*network);
  printSol(*network);

  system("PAUSE");
  return 0;
}