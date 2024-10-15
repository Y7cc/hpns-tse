#include <sstream>
#include "ns3/core-module.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/Server.hh"
#include "ns3/internet-module.h"

NS_LOG_COMPONENT_DEFINE("Server");

using namespace std;

PointToPointHelper Server::p2p;
Ipv4AddressHelper Server::ipv4;
InternetStackHelper Server::internet;
Server::Server(uint32_t podId, uint32_t superNodeId, uint32_t serverId, uint32_t numNpusPerServer, uint32_t numL1SwitchesPerServer, uint32_t areaId){

    this->m_serverId = serverId;
    this->m_superNodeId = superNodeId;
    this->m_podId = podId;

    m_npus.createNpu(numNpusPerServer,areaId,podId);
    // m_npus.createNpu(numNpusPerServer,areaId);
    internet.Install(m_npus);
    m_l1Switches.createInnerL1(numL1SwitchesPerServer,areaId,podId);
    // m_l1Switches.createInnerL1(numL1SwitchesPerServer,areaId);
    internet.Install(m_l1Switches);

    p2p.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));

    // 连接NPU与L1交换机
    for (uint32_t i = 0; i < numNpusPerServer; ++i) {
        for (uint32_t j = 0; j < numL1SwitchesPerServer; ++j) {
            p2p.Install(m_npus.Get(i), m_l1Switches.Get(j));
        }
    }

    //链接NPU之间的channel
    for(uint32_t i=0;i<numNpusPerServer-1;i=i+2){
        uint32_t j=i+1;
        p2p.Install(m_npus.Get(i), m_npus.Get(j));
    }


}

//输出server中某一组的所有NPU
NodeContainer Server::GetNpusByGroup(uint32_t groupId) const {
    NodeContainer npusInGroup;
    uint32_t npusPerGroup = m_npus.GetN() / 4;  //4是track数
    for (uint32_t i = groupId * npusPerGroup; i < (groupId + 1) * npusPerGroup; ++i) {
        npusInGroup.Add(m_npus.Get(i));
    }
    return npusInGroup;
}

NodeContainer Server::GetNpus() const {
    return m_npus;
}

NodeContainer Server::GetL1Switches() const {
    return m_l1Switches;
}

//为Server中的NPU分配IP地址
void Server::AssignIpToNpus(uint32_t podId, uint32_t superNodeId){
    uint32_t npusPerGroup = m_npus.GetN() / 4;  //4是track数  记录了每组有多少个NPU
    uint32_t trackId;

    for(uint32_t i=0;i<m_npus.GetN();i++){
        trackId = i / npusPerGroup;
        AssignNpu(podId, superNodeId, trackId, i, m_npus.Get(i));
    }
}

void Server::AssignIpToL1Switches(uint32_t podId, uint32_t superNodeId){
    for(uint32_t i=0;i<m_l1Switches.GetN();i++){
        AssignL1(podId, superNodeId, m_serverId, i, m_l1Switches.Get(i));
    }
}

void 
Server::AssignNpu(uint32_t podId, uint32_t superNodeId, uint32_t trackId, uint32_t npuId, Ptr<Node> npu){
    uint32_t tag = 1;  //代表是NPU 的IP

    //构建NPU IP地址base
    uint32_t npuBase = (tag << 31) |  // Tag 占1位
                    (podId << 25) |  // Pod ID 占6位
                    (trackId << 22) |  // Track ID 占3位
                    (superNodeId << 17) |  // SuperNode ID 占5位
                    (m_serverId << 10) |  // Server ID 占7位
                    npuId << 5; // NPU in Server 占5位
    
    ostringstream oss;

    oss << ((npuBase >> 24) & 0xFF) << "."
        << ((npuBase >> 16) & 0xFF) << "."
        << ((npuBase >> 8) & 0xFF) << "."
        << (npuBase & 0xFF);

    string ipBase = oss.str();
    
    ipv4.SetBase(ipBase.c_str(), "255.255.255.240");
    NetDeviceContainer npuDevices = GetNpuNetDevices(npuId);
    ipv4.Assign(npuDevices);
} 


void
Server::AssignL1(uint32_t podId, uint32_t superNodeId, uint32_t serverId, uint32_t l1Id, Ptr<Node> l1){
    uint32_t tag = 0;
    uint32_t switchType = 3; //011

    uint32_t l1Base = (tag << 31) |  // Tag 占1位
                    (switchType << 28) |  // Switch Type 占3位
                    (podId << 22) |  // Pod ID 占6位
                    (superNodeId << 17) |  // SuperNode ID 占5位
                    (serverId << 10) |  // Server ID 占7位
                    l1Id << 6; // L1 in Server 占4位

    ostringstream oss;

    oss << ((l1Base >> 24) & 0xFF) << "."
        << ((l1Base >> 16) & 0xFF) << "."
        << ((l1Base >> 8) & 0xFF) << "."
        << (l1Base & 0xFF);

    string ipBase = oss.str();

    ipv4.SetBase(ipBase.c_str(), "255.255.255.192");
    NetDeviceContainer l1Devices = GetL1NetDevices(l1Id);
    ipv4.Assign(l1Devices);

}


NetDeviceContainer Server::GetNpuNetDevices(uint32_t npuId){
    Ptr<Node> npu = m_npus.Get(npuId);
    uint32_t numDevices = npu->GetNDevices();

    NetDeviceContainer allNetDevices;

    for(uint32_t i=0;i<numDevices;i++){
        allNetDevices.Add(npu->GetDevice(i));
    }

    return allNetDevices;
}

NetDeviceContainer Server::GetL1NetDevices(uint32_t l1Id){
    Ptr<Node> l1 = m_l1Switches.Get(l1Id);
    uint32_t numDevices = l1->GetNDevices();

    NetDeviceContainer allNetDevices;

    for(uint32_t i=0;i<numDevices;i++){
        allNetDevices.Add(l1->GetDevice(i));
    }

    return allNetDevices;
}

Ptr<Node> Server::GetNpuNode(uint32_t npuId){
    return this->m_npus.Get(npuId);
}
