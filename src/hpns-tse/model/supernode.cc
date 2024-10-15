#include <vector>
#include <memory>
#include <iterator>

#include "ns3/Server.hh"
#include "ns3/supernode.hh"
#include "ns3/internet-module.h"
#include "ns3/core-module.h"

NS_LOG_COMPONENT_DEFINE("SuperNode");

using namespace ns3;
using namespace std;

PointToPointHelper SuperNode::p2p;
Ipv4AddressHelper SuperNode::ipv4;
InternetStackHelper SuperNode::internet;
SuperNode::SuperNode(uint32_t podId, uint32_t superNodeId, uint32_t numServers, uint32_t numNpusPerServer, uint32_t numL1SwitchesPerServer, uint32_t numL2SwitchesPerPlane, uint32_t numPlane, uint32_t numSpnode) {

    this->m_superNodeId = superNodeId;
    this->m_podId = podId;

    uint32_t areaId = podId *  numSpnode + superNodeId + 1;

    for (uint32_t i = 0; i < numServers; ++i) {
        m_servers.push_back(make_shared<Server>(podId, superNodeId ,i , numNpusPerServer, numL1SwitchesPerServer, areaId));
        NS_LOG_INFO("Server：" << i << " has been created");
    }

    //分平面初始化L2层交换机
    for(uint32_t m = 0; m < numPlane; ++m){
        NodeContainer planes;
        // planes.createSw(numL2SwitchesPerPlane, areaId);
        planes.createSw(numL2SwitchesPerPlane, areaId,podId);
        internet.Install(planes);
        m_l2Switches.push_back(planes);
        NS_LOG_INFO("L2 Switches in plane" << m << " has been created");
    }

    //遍历supernode中的每一个server
    for (auto &server : m_servers) {
        NodeContainer l1Switches = server->GetL1Switches();
        for (uint32_t j = 0; j < l1Switches.GetN(); ++j) {
            //j号l1层交换机与第j个平面的l2交换机相连
            NodeContainer l2SwitchesPerPlane = m_l2Switches[j];
            p2p.SetDeviceAttribute("DataRate", StringValue("40Gbps"));
            p2p.SetChannelAttribute("Delay", StringValue("2ms"));

            for(uint32_t k = 0; k < l2SwitchesPerPlane.GetN(); ++k){
                p2p.Install(l1Switches.Get(j), l2SwitchesPerPlane.Get(k));
            }
        }
    }
}

std::vector<std::shared_ptr<Server>> SuperNode::GetServers() const {
    return m_servers;
}

std::vector<NodeContainer> SuperNode::GetL2Switches() const {
    return m_l2Switches;
}


//获取某一个Supernode内所有的某一组的NPU
NodeContainer SuperNode::GetNpusByGroup(uint32_t groupId) {
    NodeContainer npusAllGroup;
    for (auto& server : m_servers) {
        NodeContainer groupNpus = server->GetNpusByGroup(groupId);
        npusAllGroup.Add(groupNpus);
    }
    return npusAllGroup;
}

void SuperNode::AssignIpToNpus(uint32_t podId){
    for(auto& server : m_servers){
        server->AssignIpToNpus(podId, m_superNodeId);
    }
}


/**
 * 为L1层交换机分配IP
*/
void SuperNode::AssignIpToL1Switches(uint32_t podId){
    for(auto &server : m_servers){
        server->AssignIpToL1Switches(podId, m_superNodeId);
    }
}

/**
 * 为L2层交换机分配IP
*/
 void SuperNode::AssignIpToL2Switches(uint32_t podId){
    //遍历每一个Plane;
    for(uint32_t numPlane = 0; numPlane < m_l2Switches.size(); ++numPlane){
        for(uint32_t i=0; i < m_l2Switches[numPlane].GetN(); ++i){
            AssignL2(podId,numPlane,i,m_l2Switches[numPlane].Get(i)); 
        }

    }
 }

 void 
 SuperNode::AssignL2(uint32_t podId, uint32_t planeId, uint32_t l2Id, Ptr<Node> l2){
    uint32_t tag = 0; // 0表示是交换机
    uint32_t switchType = 4; //100

    uint32_t l2Base = (tag << 31) |  // Tag 占1位
                    (switchType << 28) |  // Switch Type 占3位
                    (podId << 22) |  // Pod ID 占6位
                    (m_superNodeId << 17) |  // SuperNode ID 占5位
                    (planeId << 13) | // Plane ID 占4位
                    l2Id << 7; //l2Id占6位

    ostringstream oss;

    oss << ((l2Base >> 24) & 0xFF) << "."
        << ((l2Base >> 16) & 0xFF) << "."
        << ((l2Base >> 8) & 0xFF) << "."
        << (l2Base & 0xFF);

    string ipBase = oss.str();

    ipv4.SetBase(ipBase.c_str(),"255.255.255.128");
    NetDeviceContainer l2Devices = GetL2NetDevices(planeId,l2Id);
    ipv4.Assign(l2Devices);
}
      
NetDeviceContainer 
SuperNode::GetL2NetDevices(uint32_t planeId, uint32_t l2Id){
    NodeContainer l2SwitchesPerPlane = m_l2Switches[planeId];
    Ptr<Node> l2Switch = l2SwitchesPerPlane.Get(l2Id);

    uint32_t numDevices = l2Switch->GetNDevices();
    NetDeviceContainer allNetDevices;

    for(uint32_t i = 0; i < numDevices; ++i){
        allNetDevices.Add(l2Switch->GetDevice(i));
    }

    return allNetDevices;
}

Ptr<Node>
SuperNode::GetNpuNode(uint32_t serverId,uint32_t  npuId){
    shared_ptr<Server> server = this->m_servers[serverId];
    return server->GetNpuNode(npuId);
}