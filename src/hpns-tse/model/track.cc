#include <vector>
#include <memory>

#include "ns3/track.hh"
#include "ns3/point-to-point-helper.h"
#include "ns3/string.h"
#include "ns3/internet-module.h"


using namespace std;

PointToPointHelper Track::p2p;
Ipv4AddressHelper Track::ipv4;
InternetStackHelper Track::internet;
Track::Track(uint32_t trackId, uint32_t switchCount, uint32_t podId) {

    this->m_trackId = trackId;

    m_aggSwitches.createSw(switchCount,0,podId);  // 创建AGG层交换机
    // m_aggSwitches.createSw(switchCount,0);  // 创建AGG层交换机
    internet.Install(m_aggSwitches);    //安装网络层协议栈

    m_accSwitches.createOuterL1(switchCount,0,podId);  // 创建ACC层交换机
    // m_accSwitches.createOuterL1(switchCount,0);  // 创建ACC层交换机
    internet.Install(m_accSwitches);

    this->numPort = switchCount * 2;
    p2p.SetDeviceAttribute("DataRate", StringValue("200Gbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));

    // 将AGG层交换机连接到ACC层交换机
    for (uint32_t i = 0; i < switchCount; ++i) {
        for (uint32_t j = 0; j < switchCount; ++j) {
            p2p.Install(m_aggSwitches.Get(i), m_accSwitches.Get(j));  // 连接ACC层到AGG层交换机
        }
    }
}

// 连接每个ACC交换机到对应的NPU组
void Track::ConnectSuperNodeToTrack(vector<shared_ptr<SuperNode>>& m_superNodes, uint32_t groupId){
    uint32_t numSuperNodes = m_superNodes.size();
    NodeContainer npus;

    p2p.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    for(uint32_t i=0; i<numSuperNodes; i++){
        npus.Add(m_superNodes[i]->GetNpusByGroup(groupId));
    }

    uint32_t numAccSwitches = m_accSwitches.GetN();
    // uint32_t numNpus = npus.GetN();

    //将Track中的ACC交换机链接到其对应NPU组中的所有NPU上
    for (uint32_t accIdx = 0; accIdx < numAccSwitches; ++accIdx) {
        for(uint32_t j = 0; j < numAccSwitches; ++j){
            p2p.Install(m_accSwitches.Get(accIdx), npus.Get(accIdx * (Track::numPort / 2) + j));
        }
    }
}


void Track::AssignIpToAggSwitches(uint32_t podId){
    uint32_t numAggSwitches = m_aggSwitches.GetN();
    for(uint32_t i=0; i<numAggSwitches; i++){
        AssignAgg(podId, i);
    }
}   
void Track::AssignIpToAccSwitches(uint32_t podId){
    uint32_t numAccSwitches = m_accSwitches.GetN();
    for(uint32_t i=0; i<numAccSwitches; i++){
        AssignAcc(podId, i);
    }
}

void Track::AssignAgg(uint32_t podId ,uint32_t aggId){
    uint32_t Tag = 0;
    uint32_t switchType = 5; //101

    uint32_t aggBase = (Tag << 31) |    // tag 占1位
                    (switchType << 28) |  // switchType 占3位
                    (podId << 22) |  // Pod ID 占6位
                    (m_trackId << 19) | //trackID 占3位
                    aggId << 10;//aggId 占9位

    ostringstream oss;
    oss << ((aggBase >> 24) & 0xFF) << "."
        << ((aggBase >> 16) & 0xFF) << "."
        << ((aggBase >> 8) & 0xFF) << "."
        << (aggBase & 0xFF);

    string ipBase = oss.str(); 

    ipv4.SetBase(ipBase.c_str(),"255.255.252.0");
    NetDeviceContainer aggDevices = GetAggNetDevices(aggId);
    ipv4.Assign(aggDevices);
}

void Track::AssignAcc(uint32_t podId ,uint32_t accId){
    uint32_t Tag = 0;
    uint32_t switchType = 6; //110

    uint32_t accBase = (Tag << 31) |    // tag 占1位
                    (switchType << 28) |  // switchType 占3位
                    (podId << 22) |  // Pod ID 占6位
                    (m_trackId << 19) | //trackID 占3位
                    accId << 10;//aggId 占9位

    ostringstream oss;
    oss << ((accBase >> 24) & 0xFF) << "."
        << ((accBase >> 16) & 0xFF) << "."
        << ((accBase >> 8) & 0xFF) << "."
        << (accBase & 0xFF);

    string ipBase = oss.str(); 

    ipv4.SetBase(ipBase.c_str(),"255.255.252.0");
    NetDeviceContainer accDevices = GetAccNetDevices(accId);
    ipv4.Assign(accDevices);
}


NetDeviceContainer Track::GetAggNetDevices(uint32_t aggId) const {
    Ptr<Node> aggSw = m_aggSwitches.Get(aggId);
    uint32_t numDevices = aggSw->GetNDevices();

    NetDeviceContainer allDevices;

    for(uint32_t i=0; i<numDevices; i++){
        allDevices.Add(aggSw->GetDevice(i));
    }

    return allDevices;
}   

NetDeviceContainer Track::GetAccNetDevices(uint32_t accId) const {
    Ptr<Node> accSw = m_accSwitches.Get(accId);
    uint32_t numDevices = accSw->GetNDevices();

    NetDeviceContainer allDevices;

    for(uint32_t i=0; i<numDevices; i++){
        allDevices.Add(accSw->GetDevice(i));
    }

    return allDevices;
}

NodeContainer Track::GetAggSwitches() const {
    return m_aggSwitches;  // 返回AGG层交换机
}

NodeContainer Track::GetAccSwitches() const {
    return m_accSwitches;  // 返回ACC层交换机
}
