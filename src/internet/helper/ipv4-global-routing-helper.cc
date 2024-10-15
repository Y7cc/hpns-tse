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
#include "ipv4-global-routing-helper.h"
#include "ns3/global-router-interface.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("GlobalRoutingHelper");

Ipv4GlobalRoutingHelper::Ipv4GlobalRoutingHelper ()
{
}

Ipv4GlobalRoutingHelper::Ipv4GlobalRoutingHelper (const Ipv4GlobalRoutingHelper &o)
{
}

Ipv4GlobalRoutingHelper*
Ipv4GlobalRoutingHelper::Copy (void) const
{
  return new Ipv4GlobalRoutingHelper (*this);
}

Ptr<Ipv4RoutingProtocol>
Ipv4GlobalRoutingHelper::Create (Ptr<Node> node) const
{
  NS_LOG_LOGIC ("Adding GlobalRouter interface to node " <<
                node->GetId ());

  Ptr<GlobalRouter> globalRouter = CreateObject<GlobalRouter> ();
  node->AggregateObject (globalRouter);

  NS_LOG_LOGIC ("Adding GlobalRouting Protocol to node " << node->GetId ());
  Ptr<Ipv4GlobalRouting> globalRouting = CreateObject<Ipv4GlobalRouting> ();
  globalRouter->SetRoutingProtocol (globalRouting);

  return globalRouting;
}

void 
Ipv4GlobalRoutingHelper::PopulateRoutingTables (void)
{
  GlobalRouteManager::BuildGlobalRoutingDatabase ();
  GlobalRouteManager::InitializeRoutes ();
}
void 
Ipv4GlobalRoutingHelper::RecomputeRoutingTables (void)
{
  GlobalRouteManager::DeleteGlobalRoutes ();
  GlobalRouteManager::BuildGlobalRoutingDatabase ();
  GlobalRouteManager::InitializeRoutes ();
}

Ptr<Ipv4GlobalRouting>
Ipv4GlobalRoutingHelper::GetGlobalRouting (Ptr<Ipv4> ipv4) const
{
  NS_LOG_FUNCTION (this);
  Ptr<Ipv4RoutingProtocol> ipv4rp = ipv4->GetRoutingProtocol ();
  NS_ASSERT_MSG (ipv4rp, "No routing protocol associated with Ipv4");
  if (DynamicCast<Ipv4GlobalRouting> (ipv4rp))
    {
      NS_LOG_LOGIC ("Global routing found as the main IPv4 routing protocol.");
      return DynamicCast<Ipv4GlobalRouting> (ipv4rp); 
    } 
  if (DynamicCast<Ipv4ListRouting> (ipv4rp))
    {
      Ptr<Ipv4ListRouting> lrp = DynamicCast<Ipv4ListRouting> (ipv4rp);
      int16_t priority;
      for (uint32_t i = 0; i < lrp->GetNRoutingProtocols ();  i++)
        {
          NS_LOG_LOGIC ("Searching for global routing in list");
          Ptr<Ipv4RoutingProtocol> temp = lrp->GetRoutingProtocol (i, priority);
          if (DynamicCast<Ipv4GlobalRouting> (temp))
            {
              NS_LOG_LOGIC ("Found global routing in list");
              return DynamicCast<Ipv4GlobalRouting> (temp);
            }
        }
    }
  NS_LOG_LOGIC ("Global routing not found");
  return 0;
}

uint32_t
Ipv4GlobalRoutingHelper::GetNRouteEntries(Ptr<Ipv4> ipv4){
    Ptr<Ipv4GlobalRouting> globalRouting = GetGlobalRouting(ipv4);
    return globalRouting->GetNRoutes();
}


} // namespace ns3
