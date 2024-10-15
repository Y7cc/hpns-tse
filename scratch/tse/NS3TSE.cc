#include "ns3/pod.hh"
#include "ns3/Core.hh"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/applications-module.h"
#include "ns3/udp-echo-helper.h"
#include "ns3/udp-echo-server.h"
#include "ns3/udp-echo-client.h"
#include "ns3/mpi-interface.h"
#include "ns3/Logger.hh"


#include <vector>
#include <memory>

#ifdef NS3_MPI
#include <mpi.h>
#endif


using namespace ns3;
using namespace std;

string log_dir;

void parse_args(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.AddValue("log-dir", "Sdplog output direction", log_dir);
    cmd.Parse(argc, argv);
}

// NS_LOG_COMPONENT_DEFINE("tse-topobuild");
int main(int argc, char *argv[]) {
    parse_args(argc, argv);
    Logger::setLogFilename(log_dir + "/tse.log");
    Logger::getLogger("NS3TSE")->info("TSE + NS3");

    Time::SetResolution(Time::NS);
    // LogComponentEnable("tse-topobuild", LOG_LEVEL_INFO);
    // LogComponentEnable("Pod", LOG_LEVEL_INFO); // 启用 Pod 的日志
    // LogComponentEnable("SuperNode", LOG_LEVEL_INFO); // 启用 SuperNode 的日志
    // // LogComponentEnable("Track", LOG_LEVEL_DEBUG); // 启用 Track 的日志
    // LogComponentEnable("GlobalRouteManagerImpl", LOG_LEVEL_INFO);
    

    uint32_t numCore = 8; //32;  //核心层交换机的数量 
    uint32_t numPod = 2; //Pod的数量

    uint32_t numTrack = 4; // POD内Track的数量 = server内NPU 的分组书
    uint32_t numPortInTrack = 8; //32; //ACC层交换机和AGG层交换机的端口数 除以2 = 每个Track内ACC层交换机的个数
    uint32_t numServersPerSuperNode = 4;//8; // 每个SuperNode内server的数量
    uint32_t numNpusPerServer = 8; //16; // 每个server内NPU的数量
    uint32_t numL1SwitchesPerServer = 7; // 每个server内L1交换机的数量
    uint32_t numL2SwitchesPerPlane = 16; // 每个plane内L2交换机的数量
    uint32_t numPlanes = 7; //L2层交换机分平面的数量
    uint32_t numAggSwitches = numPortInTrack / 2; 
    uint32_t numAccSwitches = numPortInTrack / 2;

    MpiInterface::Enable (&argc, &argv);
    uint32_t systemId = MpiInterface::GetSystemId ();
    uint32_t systemCount = MpiInterface::GetSize ();

    if (systemCount != 2)
    {
      std::cout << "This simulation requires 2 and only 2 logical processors." << std::endl;
      return 1;
    }

    vector<shared_ptr<Pod>> pods;

    //计算每个POD可连接的SuperNode数量
    uint32_t numSuperNodes = (numTrack * numAccSwitches * numAccSwitches) / (numServersPerSuperNode * numNpusPerServer);

    //创建Pod
    for(uint32_t i = 0; i < numPod; i++){
        pods.push_back(make_shared<Pod>(i, numSuperNodes, numServersPerSuperNode, numNpusPerServer, numL1SwitchesPerServer, numL2SwitchesPerPlane, numPlanes, numAggSwitches, numAccSwitches, numTrack));
        // NS_LOG_INFO("Pod " << i << " create finished");
    }

    // 创建Core网络
    Core coreNetwork(numCore);

    // 连接Pod到Core
    for(uint32_t i = 0; i < numPod; i++){
        pods[i]->ConnectPodToCore(coreNetwork);
        // NS_LOG_INFO("Pod " << i << " connected to Core Network.");
    }

    //分配IP地址
    for(uint32_t i = 0; i < numPod; i++){
        pods[i]->AssignIpToNpus();
        pods[i]->AssignIpToL1Switches();
        pods[i]->AssignIpToL2Switches();
        pods[i]->AssignIpToAccSwitches();
        pods[i]->AssignIpToAggSwitches();
        // NS_LOG_INFO("Pod " << i << " ip assigned finished.");
    }

    coreNetwork.AssignIpToCoreSwitches();
    // NS_LOG_INFO("coresw ip assigned finished.");
    
    // NS_LOG_INFO("routing table populate begin");
    Logger::getLogger("Global")->trace("routing table populate begin");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    Logger::getLogger("Global")->trace("routing table populate end");

    // Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("1.txt", std::ios::out);
    // Ipv4GlobalRoutingHelper routingHelper;
    // routingHelper.PrintRoutingTableAllAt(Seconds(5000.0),routingStream);

    //计算路由表内存占用
    Ipv4StaticRoutingHelper staticHelper;
    Ipv4GlobalRoutingHelper globalHelper;

    uint32_t port = 9;

    Ptr<Node> serverNode = pods[1]->GetNpuNode(0,0,1);

    if(systemId == 1){
        // Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("780-1.txt", std::ios::out);
        // Ipv4GlobalRoutingHelper routingHelper;
        // routingHelper.PrintRoutingTableAllAt(Seconds(5000.0),routingStream);
        NodeList::Iterator listEnd = NodeList::End ();
        uint32_t sum = 0;
        uint32_t nodesum = 0;
        for(NodeList::Iterator i = NodeList::Begin (); i != listEnd; i++){
            Ptr<Node> node = *i;
            Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
            // sum += staticHelper.GetNRouteEntries(ipv4);
            sum += globalHelper.GetNRouteEntries(ipv4);
            nodesum ++;
        }
        // <<  "进程： "<< systemId 
        // std::cout <<  "进程： "<< systemId  <<"Node 节点总数：" << nodesum << std::endl;
        Logger::getLogger("Global")->trace("进程:{} 路由表总内存占用：{} Bytes", systemId, sum * 16);

        //通信
        // NS_LOG_INFO("communication test begin");
        UdpEchoServerHelper echoServer(port);
        ApplicationContainer serverApps = echoServer.Install(serverNode);
        //启用服务端APP
        serverApps.Start(Seconds(100000.0));
        serverApps.Stop(Seconds(110000.0));
    }

    if(systemId == 0){
        Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("780-0.txt", std::ios::out);
        Ipv4GlobalRoutingHelper routingHelper;
        routingHelper.PrintRoutingTableAllAt(Seconds(5000.0),routingStream);
        NodeList::Iterator listEnd = NodeList::End ();
        uint32_t sum = 0;
        uint32_t nodesum = 0;
        for(NodeList::Iterator i = NodeList::Begin (); i != listEnd; i++){
            Ptr<Node> node = *i;
            Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
            // sum += staticHelper.GetNRouteEntries(ipv4);
            sum += globalHelper.GetNRouteEntries(ipv4);
            nodesum ++;
        }
        // std::cout << "进程： "<< systemId <<"Node 节点总数：" << nodesum << std::endl;
        // std::cout <<  "进程： "<< systemId << "路由表内存占用：" << sum * 16 << " bytes" << std::endl;
        Logger::getLogger("Global")->trace("进程:{} 路由表总内存占用：{} Bytes", systemId, sum * 16);
        Ptr<Node> clientNode = pods[0]->GetNpuNode(0,0,0);
        Ptr<Ipv4> serverIpv4 = serverNode->GetObject<Ipv4>();
        Ipv4Address serverIp = serverIpv4->GetAddress(1,0).GetLocal(); //获取了服务端IP地址

        UdpEchoClientHelper echoClient(serverIp, port);
        //设置参数
        echoClient.SetAttribute("MaxPackets", UintegerValue(1));
        echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
        echoClient.SetAttribute("PacketSize", UintegerValue(1024));

        ApplicationContainer clientApps = echoClient.Install(clientNode);
        clientApps.Start(Seconds(100002.0));
        clientApps.Stop(Seconds(100102.0));
    }
    // // Output routing tables to a file
    // Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("1010-4.txt", std::ios::out);
    // Ipv4GlobalRoutingHelper routingHelper;
    // routingHelper.PrintRoutingTableAllAt(Seconds(5000.0),routingStream);
    // std::cout << "Size of ns3::Ipv4RoutingTableEntry: " << sizeof(ns3::Ipv4RoutingTableEntry) << " bytes" << std::endl;
    // AnimationInterface anim("hwtopology.xml");
    // Simulator::Stop (Seconds (5));
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    Logger::getLogger("NS3TSE")->info("Process {} RUN START",systemId);
    Simulator::Run();
    Logger::getLogger("NS3TSE")->info("Process {} Run finished",systemId);
    Simulator::Destroy();
    MpiInterface::Disable ();
    return 0;
}
