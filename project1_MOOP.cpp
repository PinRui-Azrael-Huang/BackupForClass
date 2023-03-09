#include "bits/stdc++.h"

using namespace std;

class Flow{
  public:
    unsigned int id, src, dst, size;
    Flow(){};
};

class Node{
  public:
    unsigned int id;
    bool isSDN;
    vector<unsigned int> neighborList;
    unsigned int packetCount;
    map<unsigned int, vector<pair<unsigned int,double>>> routingTable;
    Node(){};
};

class Network{
  public:
    unsigned int numNodes, numSDN, numDst, numLinks, numFlows;
    Node *nodeList;
    unsigned int *dstIDList;
    Flow *flowList;
    Network(){};
};

// Process the input for the network, storing all necessary infos before computation.
Network* processInput(){
  Network* network = new Network;
  int numNodes, numSDN, numDst, numLinks, numFlows;
  cin >> numNodes >> numSDN >> numDst >> numLinks >> numFlows;

  network->numNodes = numNodes;
  network->numSDN = numSDN;
  network->numDst = numDst;
  network->numLinks = numLinks;
  network->numFlows = numFlows;

  Node* nodeList = new Node[numNodes];
  for(int i = 0; i < numNodes; i++){
    nodeList[i].id = i;   
    nodeList[i].isSDN = false;
    nodeList[i].packetCount = 0;
  }
  // Process the Nodes that are SDNs;
  unsigned int sdnID, dstID;
  for(int i = 0; i < numSDN ; i++){
    cin >> sdnID;
    nodeList[sdnID].isSDN = true;
  }
  // Process the list of destinations;
  unsigned int *dstIDList = new unsigned int[numDst];
  for(int i = 0; i < numDst ; i++){
    cin >> dstID;
    dstIDList[i] = dstID;
  }

  // Process all links for the network, also compute graph (in the form of adjacency list)
  unsigned int _linkID, node1, node2;
  for(int i = 0; i < numLinks ; i++){
    cin >> _linkID >>  node1 >> node2;
    nodeList[node1].neighborList.push_back(node2);
    nodeList[node2].neighborList.push_back(node1);
  }

  // Process the flows to be dealt with.
  Flow* flowList = new Flow[numFlows];
  unsigned int id, src, dst, flowSize; 
  for(int i = 0; i < numFlows ; i++){
    cin >> id >> src >> dst >> flowSize;
    flowList[i].id = id;
    flowList[i].src = src;
    flowList[i].dst = dst;
    flowList[i].size = flowSize;
  }

  network->nodeList = nodeList;
  network->dstIDList = dstIDList;
  network->flowList = flowList;

  return network;
}

void sendPacket(Network &network, int src, int dst, int currentCount){
  map<unsigned int, vector<pair<unsigned int,double>>>::iterator iter;
  for(unsigned int neighbor : network.nodeList[src].neighborList){
    // If the packet count of a neighbor > currentCount or is = 0.
    // Replace the packet count of that neighbor and update the routing table
    if(neighbor != dst && (network.nodeList[neighbor].packetCount == 0||
                           network.nodeList[neighbor].packetCount >= currentCount)){
      if (network.nodeList[neighbor].packetCount == currentCount){
        // If the count is the same for two paths, select the one node with lower ID.
        iter = network.nodeList[neighbor].routingTable.find(dst);
        unsigned int currentID = iter->second.front().first;
        if (src < currentID){
          iter->second.pop_back();
          iter->second.push_back(pair<unsigned int, double>(src, 1));
        }
      }
      else{
        network.nodeList[neighbor].packetCount = currentCount;
        // If there isn't any path to the dst in the routing table yet
        // Then create one routing path for the routing table
        if (network.nodeList[neighbor].routingTable.find(dst) == network.nodeList[neighbor].routingTable.end()){
          vector<pair<unsigned int, double>>path;
          path.push_back(pair<unsigned int, double>(src,1));
          network.nodeList[neighbor].routingTable.insert(pair<unsigned int, vector<pair<unsigned int, double>>>(dst, path));
        }
        // There already exists a path to the dst.
        // Needs to update the routing path in the table.
        else{
          iter = network.nodeList[neighbor].routingTable.find(dst);
          iter->second.pop_back();
          iter->second.push_back(pair<unsigned int, double>(src, 1));
        }
      }                    
      // Recursively send the packet outwards, similar to the concept of BFS.
      sendPacket(network, neighbor, dst, currentCount + 1);
    }
    else{
      continue;
    }
  }
}

// Using BFS to find the shortest path for all destinations.
void shortestPath(Network &network){
  unsigned int dst;
  unsigned int packetCount = 1;
  for(int i = 0 ; i < network.numDst ; i++){
    dst = network.dstIDList[i];
    sendPacket(network, dst, dst, packetCount);
  }
}

// A generic check function to see the correctness of the input process.
void printInput(Network* network){
  cout << "Number of Nodes: " << network->numNodes << endl;
  cout << "Number of Destinations: " << network->numDst << endl;
  cout << "Number of SDNs: " << network->numSDN << endl;

  cout << "The ID of SDN nodes are: ";
  for(int i = 0 ; i < network->numNodes ; i++){
    if(network->nodeList[i].isSDN)
      cout << network->nodeList[i].id << " ";
  }
  cout << endl;

  cout << "The ID of dst nodes are: ";
  for(int i = 0 ; i < network->numDst ; i++){
    cout << network->dstIDList[i];
  }
  cout << endl;
  
  for(int i = 0 ; i < network->numNodes ; i++){
    cout << "The Node ID: " << network->nodeList[i].id << " has the neighbors: ";
    for(int j = 0; j < network->nodeList[i].neighborList.size(); j++){
      cout << network->nodeList[i].neighborList[j] << " ";
    }
    cout<<endl;
  }
  cout << endl;
  
  cout << "Flow ID\tSource\tDestination\tSize" << endl;
  for(int i = 0 ; i < network->numFlows ; i++){
    cout << network->flowList[i].id << "\t" << network->flowList[i].src << "\t"
         << network->flowList[i].dst << "\t" << network->flowList[i].size << "\t";
    cout << endl;
  }
  cout << endl;
}

int main (){
  Network *network = processInput();
  //printInput(network);
  shortestPath(*network);

  system("PAUSE");
  return 0;
}