#include <memory>
#include "ns3/core-module.h"
#include "ns3/track.hh"
#include "ns3/pod.hh"

NS_LOG_COMPONENT_DEFINE("Pod");

using namespace std;
Pod::Pod(uint32_t podId, uint32_t numSuperNodes, uint32_t numServersPerSuperNode, uint32_t numNpusPerServer, 
         uint32_t numL1SwitchesPerServer, uint32_t numL2SwitchesPerPlane, uint32_t numPlanes,
         uint32_t numAggSwitches, uint32_t numAccSwitches, uint32_t numTracks) {

    this->m_podId = podId;

    // 创建SuperNodes
    for (uint32_t i = 0; i < numSuperNodes; ++i) {
        // m_superNodes.emplace_back(numServersPerSuperNode, numNpusPerServer, numL1SwitchesPerServer, numL2SwitchesPerPlane);
        m_superNodes.push_back(make_shared<SuperNode>(podId ,i , numServersPerSuperNode, numNpusPerServer, numL1SwitchesPerServer, numL2SwitchesPerPlane , numPlanes, numSuperNodes));
        NS_LOG_INFO("supernode " << i << " create finished");
    }

    // 创建Tracks
    for (uint32_t i = 0; i < numTracks; ++i) {
        // m_tracks.emplace_back(numAggSwitches, numAccSwitches);
        m_tracks.push_back(make_shared<Track>(i, numAccSwitches,podId));
        NS_LOG_INFO("Track " << i << " create finished");
    }

    // 将SuperNodes连接到对应的Tracks
    for (uint32_t trackIdx = 0; trackIdx < numTracks; ++trackIdx) {
        for (uint32_t superNodeIdx = 0; superNodeIdx < numSuperNodes; ++superNodeIdx) {
            // 每个Track只连接对应组的NPU
            m_tracks[trackIdx]->ConnectSuperNodeToTrack(m_superNodes, trackIdx);
        }
    }
}

void Pod::ConnectPodToCore(Core& coreNetwork) {
    // 连接每个Track到Core
    for (auto& track : m_tracks) {
        coreNetwork.ConnectAggToCore(track->GetAggSwitches());
    }
}

/**
 * 
 * 给NPU分配IP地址
 * 
*/
void Pod::AssignIpToNpus(){
    for (auto& superNode : m_superNodes) {
        superNode->AssignIpToNpus(m_podId);
    }
}

/**
 * 给L1层交换机分配IP
*/
void Pod::AssignIpToL1Switches(){
    for (auto& superNode : m_superNodes) {
        superNode->AssignIpToL1Switches(m_podId);
    }
}

/**
 * 给L2层交换机分配IP
*/
void Pod::AssignIpToL2Switches(){
    for (auto& superNode : m_superNodes) {
        superNode->AssignIpToL2Switches(m_podId);
    }
}

/**
 * 给AGG层交换机分配IP
*/
void Pod::AssignIpToAggSwitches(){
    for (auto& track : m_tracks) {
        track->AssignIpToAggSwitches(m_podId);
    }
}

/**
 * 给ACC层交换机分配IP
*/
void Pod::AssignIpToAccSwitches(){
    for (auto& track : m_tracks) {
        track->AssignIpToAccSwitches(m_podId);
    }
}

Ptr<Node> Pod::GetNpuNode(uint32_t superNodeId, uint32_t serverId, uint32_t npuId){
    shared_ptr<SuperNode> superNode = this->m_superNodes[superNodeId];
    return superNode->GetNpuNode(serverId, npuId);
}


