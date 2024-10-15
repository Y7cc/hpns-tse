#include "ns3/core-module.h"
#include "ns3/Core.hh"
#include "ns3/internet-module.h"

using namespace std;

PointToPointHelper Core::p2p;
Ipv4AddressHelper Core::ipv4;
InternetStackHelper Core::internet;

NS_LOG_COMPONENT_DEFINE("Core");
Core::Core(uint32_t numCoreSwitches) {
    // 创建核心交换机
    m_coreSwitches.createSw(numCoreSwitches,0,0);
    // m_coreSwitches.createSw(numCoreSwitches,0);
    NS_LOG_INFO("Core Switches Created");
    
    internet.Install(m_coreSwitches);
}

void Core::ConnectAggToCore(NodeContainer aggSwitches) {
    uint32_t num = m_coreSwitches.GetN();

    p2p.SetDeviceAttribute("DataRate", StringValue("10Gbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    for(uint32_t i=0;i<aggSwitches.GetN();i++){
        p2p.Install(m_coreSwitches.Get(i),aggSwitches.Get(i));
        p2p.Install(m_coreSwitches.Get(i+(num / 2)),aggSwitches.Get(i));
    }
}

void Core::AssignIpToCoreSwitches(){
    for(uint32_t i=0;i<m_coreSwitches.GetN();i++){
        AssignCore(i);
    }
}

void Core::AssignCore(uint32_t coreId){
    uint32_t Tag = 0;
    uint32_t switchType = 0; //000

     uint32_t coreBase = (Tag << 31) |  // Tag 占1位
                    (switchType << 28) |  // Switch Type 占3位
                    coreId << 14;  //coreId占14位

    ostringstream oss;

    oss << ((coreBase >> 24) & 0xFF) << "."
        << ((coreBase >> 16) & 0xFF) << "."
        << ((coreBase >> 8) & 0xFF) << "."
        << (coreBase & 0xFF);

    string ipBase = oss.str();

    ipv4.SetBase(ipBase.c_str(),"255.255.192.0");
    NetDeviceContainer coreDevices = GetCoreNetDevices(coreId);
    ipv4.Assign(coreDevices);
}   

NetDeviceContainer Core::GetCoreNetDevices(uint32_t coreId) {
    Ptr<Node> coreSw = m_coreSwitches.Get(coreId);
    uint32_t numDevices = coreSw->GetNDevices();
    NetDeviceContainer allDevices;

    for (uint32_t i = 0; i < numDevices; i++) {
        allDevices.Add(coreSw->GetDevice(i));
    }
    return allDevices;
}

NodeContainer Core::GetCoreSwitches() {
    return m_coreSwitches;
}
