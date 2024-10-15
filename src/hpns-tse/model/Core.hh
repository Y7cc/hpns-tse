#ifndef CORE_H
#define CORE_H


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/internet-module.h"

using namespace ns3;
using namespace std;

class Core {
public:
    static PointToPointHelper p2p;
    static Ipv4AddressHelper ipv4;
    static InternetStackHelper internet;

    Core(uint32_t numCoreSwitches);
    
    void ConnectAggToCore(NodeContainer aggSwitches);
    void AssignIpToCoreSwitches();
    void AssignCore(uint32_t coreId);
    NetDeviceContainer GetCoreNetDevices(uint32_t coreId);
    NodeContainer GetCoreSwitches();

private:
    NodeContainer m_coreSwitches;  // 核心交换机集合
};

#endif // CORE_H
