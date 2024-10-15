#ifndef TRACK_H
#define TRACK_H
#include <vector>
#include <memory>

#include "ns3/supernode.hh"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"

using namespace ns3;
using namespace std;

class Track {
public:

    static PointToPointHelper p2p;
    static Ipv4AddressHelper ipv4;
    static InternetStackHelper internet;

    Track(uint32_t trackId, uint32_t switchCount, uint32_t podId);

    uint32_t m_trackId;
    uint32_t numPort;  //交换机的端口数

    void ConnectSuperNodeToTrack(vector<shared_ptr<SuperNode>>& m_superNodes, uint32_t groupId);

    NetDeviceContainer GetAggNetDevices(uint32_t aggId) const;
    NetDeviceContainer GetAccNetDevices(uint32_t accId) const;
    NodeContainer GetAggSwitches() const;
    NodeContainer GetAccSwitches() const;

    void AssignIpToAggSwitches(uint32_t podId);
    void AssignIpToAccSwitches(uint32_t podId);

    void AssignAgg(uint32_t podId ,uint32_t aggId);
    void AssignAcc(uint32_t podId ,uint32_t accId);

private:

    NodeContainer m_aggSwitches; // AGG 层交换机
    NodeContainer m_accSwitches; // ACC 层交换机
    // vector<Track*> m_tracks;  //所有的Tra
};

#endif // TRACK_H
