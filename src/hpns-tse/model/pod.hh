#ifndef POD_H
#define POD_H

#include <memory>
#include <vector>

#include "ns3/supernode.hh"
#include "ns3/track.hh"
#include "ns3/Core.hh"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;
using namespace std;

class Pod {
public:

    uint32_t m_podId;

    Pod(uint32_t podId, uint32_t numSuperNodes, uint32_t numServersPerSuperNode, uint32_t numNpusPerServer, 
        uint32_t numL1SwitchesPerServer, uint32_t numL2SwitchesPerPlane, uint32_t numPlanes,
        uint32_t numAggSwitches, uint32_t numAccSwitches, uint32_t numTracks);

    void ConnectPodToCore(Core& coreNetwork);
    void AssignIpToNpus();  //为POD中的NPU分配IP
    void AssignIpToL1Switches(); //为POD中的L1Switch分配IP
    void AssignIpToL2Switches(); //为POD中的L2Switch分配IP
    void AssignIpToAggSwitches(); //为POD中的AggSwitch分配IP
    void AssignIpToAccSwitches(); //为POD中的AccSwitch分配IP

    Ptr<Node> GetNpuNode(uint32_t superNodeId, uint32_t serverId, uint32_t npuId);

private:
    vector<shared_ptr<SuperNode>> m_superNodes;  // Pod中的SuperNode集合
    vector<shared_ptr<Track>> m_tracks;  // Pod中的Track集合
};

#endif // POD_H
