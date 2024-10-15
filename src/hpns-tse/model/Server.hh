#ifndef SERVER_H
#define SERVER_H

// #include "ns3/track.hh"

#include "ns3/ipv4-address-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"


using namespace ns3;
using namespace std;

class Server {
public:

    Server(uint32_t podId, uint32_t superNodeId, uint32_t serverId, uint32_t numNpusPerServer, uint32_t numL1SwitchesPerServer, uint32_t areaId);

    static PointToPointHelper p2p;
    static Ipv4AddressHelper ipv4;
    static InternetStackHelper internet;

    
    uint32_t groupCountPerServer;  //每一个Server内的npu组的个数 对应track数量
    uint32_t npuCountPerGroup;  //一个npu组的npu个数
    uint32_t m_serverId;
    uint32_t m_superNodeId;
    uint32_t m_podId;

    NodeContainer GetNpus() const;
    NodeContainer GetNpusByGroup(uint32_t groupId) const;
    NodeContainer GetL1Switches() const;

    void AssignIpToNpus(uint32_t podId, uint32_t superNodeId); //为Server中的npu分配ip
    void AssignIpToL1Switches(uint32_t podId, uint32_t superNodeId); //为Server中的L1交换机分配ip
    void AssignNpu(uint32_t podId, uint32_t superNodeId, uint32_t trackId, uint32_t npuId, Ptr<Node> npu); //为单个npu分配ip
    void AssignL1(uint32_t podId, uint32_t superNodeId, uint32_t serverId, uint32_t l1Id, Ptr<Node> l1);    //为单个L1交换机分配ip

    NetDeviceContainer GetNpuNetDevices(uint32_t npuId);
    NetDeviceContainer GetL1NetDevices(uint32_t l1Id);

    Ptr<Node> GetNpuNode(uint32_t npuId);

private:

    NodeContainer m_npus;
    NodeContainer m_l1Switches;
};

#endif // SERVER_H
