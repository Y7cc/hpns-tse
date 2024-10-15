#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include <iostream>

#include <string>
#include <string.h>
#include <fstream>
#include "ns3/netanim-module.h"
#include <time.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("fattree04");

int main(int argc, char *argv[])
{
    Time::SetResolution(Time::NS);
    LogComponentEnable("fattree04", LOG_LEVEL_INFO);

    //设置K
    uint32_t k = 4;
    uint32_t pods = k;
    uint32_t edgeSwitches = k / 2 ;
    uint32_t aggSwitches = k / 2 ;
    uint32_t coreSwitches = (k / 2) * (k / 2);
    uint32_t hosts = k / 2;

    //create nodes for fattree topology
    NodeContainer coreSwitchesContainer;
    coreSwitchesContainer.Create(coreSwitches);

    std::vector<NodeContainer> aggSwitchesContainers(pods);
    std::vector<NodeContainer> edgeSwitchesContainers(pods);
    std::vector<NodeContainer> hostContainers (pods * edgeSwitches);

    for (uint32_t i = 0; i < pods; i++)
    {
        aggSwitchesContainers[i].Create(aggSwitches);
        edgeSwitchesContainers[i].Create(edgeSwitches);
        for (uint32_t j = 0; j < edgeSwitches; ++j)
        {
            hostContainers[i * edgeSwitches + j].Create (hosts);
        }
    }

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    //install internet stack on all nodes
    InternetStackHelper internet;
    Ipv4GlobalRoutingHelper globalRouting;
    internet.SetRoutingHelper(globalRouting);

    internet.Install(coreSwitchesContainer);
    for(uint32_t i = 0; i < pods; i++){
        internet.Install(aggSwitchesContainers[i]);
        internet.Install(edgeSwitchesContainers[i]);
        for (uint32_t j = 0; j < edgeSwitches; ++j)
        {
            internet.Install (hostContainers[i * edgeSwitches + j]);
        }
    }

    Ipv4AddressHelper address;
    // uint32_t subnet = 10;
    uint32_t base = 74;

    std::vector<NetDeviceContainer> netDevices;
    std::vector<Ipv4InterfaceContainer> interfaces;

    for(uint32_t i = 0;i < coreSwitches;i++){
        std::ostringstream subnet;
        subnet << "10." << 74 + i % aggSwitches << "." << i*2 / k  << ".0";
        address.SetBase(subnet.str().c_str(),"255.255.255.0");

        for(u_int32_t j = 0;j < pods ; j++){
            NetDeviceContainer link = p2p.Install(coreSwitchesContainer.Get(i),aggSwitchesContainers[j].Get(i % aggSwitches));
            netDevices.push_back(link);        
            interfaces.push_back(address.Assign(link));
        }
    }

    for(uint32_t i = 0;i < pods;i++){
        for(uint32_t j = 0;j < aggSwitches;j++){
            std::ostringstream subnet;
            subnet << "10." << i << "." << j << ".0";
            address.SetBase(subnet.str().c_str(),"255.255.255.0");
            for(uint32_t l = 0;l < edgeSwitches;l++){
                NetDeviceContainer link = p2p.Install(aggSwitchesContainers[i].Get(j),edgeSwitchesContainers[i].Get(l));
                netDevices.push_back(link);
                interfaces.push_back(address.Assign(link));
            }

        }
    }

    for (uint32_t i = 0; i < pods; ++i){
        for (uint32_t j = 0; j < edgeSwitches; ++j){
            std::ostringstream subnet;
            subnet << "10." << i << "." << j + k / 2 << ".0";
            address.SetBase (subnet.str().c_str(), "255.255.255.0");
            for (uint32_t l = 0; l < hosts; ++l){
                NetDeviceContainer link = p2p.Install (edgeSwitchesContainers[i].Get (j), hostContainers[i * edgeSwitches + j].Get (l));
                netDevices.push_back (link);
                
                interfaces.push_back (address.Assign (link));
                base++;
            }
        }
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    // Output routing tables to a file
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("routing-tables.txt", std::ios::out);
    Ipv4GlobalRoutingHelper routingHelper;
    routingHelper.PrintRoutingTableAllAt(Seconds(5.0),routingStream);
    std::cout << "Size of ns3::Ipv4RoutingTableEntry: " << sizeof(ns3::Ipv4RoutingTableEntry) << " bytes" << std::endl;
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}