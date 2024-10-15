#ifndef SUPERNODE_H
#define SUPERNODE_H

#include <vector>
#include <memory>
#include <iterator>


#include "ns3/Server.hh"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"

using namespace ns3;
using namespace std;

class SuperNode {
public:

    static PointToPointHelper p2p;
    static Ipv4AddressHelper ipv4;
    static InternetStackHelper internet;

    uint32_t m_superNodeId;
    uint32_t m_podId;

    SuperNode(uint32_t podId, uint32_t superNodeId, uint32_t numServers, uint32_t numNpusPerServer, uint32_t numL1SwitchesPerServer, uint32_t numL2SwitchesPerPlane, uint32_t numPlane, uint32_t numSpnode);

    std::vector<std::shared_ptr<Server>> GetServers() const;
    std::vector<NodeContainer> GetL2Switches() const;
    NodeContainer GetNpusByGroup(uint32_t groupId);

    void AssignIpToNpus(uint32_t podId); //为NPU分配IP
    void AssignIpToL1Switches(uint32_t podId); //为L1层交换机分配IP
    void AssignIpToL2Switches(uint32_t podId); //为L2层交换机分配IP

    void AssignL2(uint32_t podId, uint32_t planeId, uint32_t l2Id, Ptr<Node> l2);
    
    NetDeviceContainer GetL2NetDevices(uint32_t planeId, uint32_t l2Id);
    Ptr<Node> GetNpuNode(uint32_t serverId,uint32_t  npuId);
    
private:
    
    std::vector<std::shared_ptr<Server>> m_servers;
    std::vector<NodeContainer> m_l2Switches;
};

#endif // SUPERNODE_H
