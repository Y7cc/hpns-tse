/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "ipv4-list-routing-helper.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/node.h"

namespace ns3 {

Ipv4ListRoutingHelper::Ipv4ListRoutingHelper()
{
}

Ipv4ListRoutingHelper::~Ipv4ListRoutingHelper()
{
  for (std::list<std::pair<const Ipv4RoutingHelper *, int16_t> >::iterator i = m_list.begin ();
       i != m_list.end (); ++i)
    {
      delete i->first;
    }
}

Ipv4ListRoutingHelper::Ipv4ListRoutingHelper (const Ipv4ListRoutingHelper &o)
{
  std::list<std::pair<const Ipv4RoutingHelper *, int16_t> >::const_iterator i;
  for (i = o.m_list.begin (); i != o.m_list.end (); ++i)
    {
      m_list.push_back (std::make_pair (const_cast<const Ipv4RoutingHelper *> (i->first->Copy ()), i->second));
    }
}

Ipv4ListRoutingHelper* 
Ipv4ListRoutingHelper::Copy (void) const 
{
  return new Ipv4ListRoutingHelper (*this); 
}

void 
Ipv4ListRoutingHelper::Add (const Ipv4RoutingHelper &routing, int16_t priority)
{
  m_list.push_back (std::make_pair (const_cast<const Ipv4RoutingHelper *> (routing.Copy ()), priority));
}

Ptr<Ipv4RoutingProtocol> 
Ipv4ListRoutingHelper::Create (Ptr<Node> node) const
{
  Ptr<Ipv4ListRouting> list = CreateObject<Ipv4ListRouting> ();
  list->SetNode (node);
  for (std::list<std::pair<const Ipv4RoutingHelper *, int16_t> >::const_iterator i = m_list.begin ();
       i != m_list.end (); ++i)
    {
      Ptr<Ipv4RoutingProtocol> prot = i->first->Create (node);
      list->AddRoutingProtocol (prot,i->second);
    }
  return list;
}

// uint32_t
// Ipv4ListRoutingHelper::GetNRouteEntries(Ptr<Ipv4> ipv4){
//   uint32_t numAllEntry = 0;
//   for(auto it = m_list.begin(); it != m_list.end() ;it++){
//       const Ipv4RoutingHelper * helper = it->first;
//       numAllEntry += helper->GetNRouteEntries(ipv4);
//   }
//   return numAllEntry;
// }

// uint32_t
// Ipv4ListRoutingHelper::GetNRouteEntries(Ptr<Ipv4> ipv4) const {
//     uint32_t numAllEntry = 0;
//     for (auto it = m_list.begin(); it != m_list.end(); ++it) {
//         // const Ipv4RoutingHelper *helper = it->first;

//         // 尝试将 helper 转换为具体类型
//         // if (auto staticHelper = DynamicCast<Ipv4StaticRoutingHelper>(helper)) {
//         //     numAllEntry += staticHelper->GetNRouteEntries(ipv4);
//         // } else if (auto globalHelper = DynamicCast<Ipv4GlobalRoutingHelper>(helper)) {
//         //     numAllEntry += globalHelper->GetNRouteEntries(ipv4);
//         // }
//         if(it->second == 0){
//             Ipv4StaticRoutingHelper *statichelper = *it->first; 
//         }
//     }
//     return numAllEntry;
// }

} // namespace ns3
