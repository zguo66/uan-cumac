/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
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
 * Author: Leonard Tracy <lentracy@gmail.com>
 */

#include "ns3/log.h"

#include "uan-mac-cumac-channel-manager.h"

NS_LOG_COMPONENT_DEFINE ("UanMacCumacChannelManager");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED ( UanMacCumacChannelManager);

UanMacCumacChannelManager::UanMacCumacChannelManager () :
  Object ()
{
}


UanMacCumacChannelManager::~UanMacCumacChannelManager ()
{
}

void
UanMacCumacChannelManager::DoDispose ()
{
  Object::DoDispose ();
}

TypeId
UanMacCumacChannelManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UanMacCumacChannelManager").
          SetParent<Object> ().
          AddConstructor<UanMacCumacChannelManager> ();
  return tid;
}

void
UanMacCumacChannelManager::SetMobilityModel (Ptr<MobilityModel> mobility)
{
  m_mobility = mobility;
}

void
UanMacCumacChannelManager::RegisterTransmission(uint8_t channelNo,
                                                Time start, Time finish,
                                                Vector srcPosition, Vector dstPosition)
{
  m_transmissions.push_back (Entry (channelNo, start, finish, srcPosition, dstPosition));
}

void
UanMacCumacChannelManager::ClearExpired (Time now)
{
  EntryList::iterator it = m_transmissions.begin ();
  for (; it != m_transmissions.end (); it++) {
    Entry &entry = *it;

    if (entry.IsExpired (now)) {
      std::cout << entry.GetSrcPosition () << " expired" << std::endl;
      m_transmissions.erase (it);
    }
  }
}

/*
 * check if channelNo will be free to transmit to dstPosition at a given time
 */
bool
UanMacCumacChannelManager::CanTransmit (uint8_t channelNo, Time start, Time finish,
                                        Vector srcPosition, Vector dstPosition)
{
  ClearExpired (start);

  return CanTransmitFromSrc (channelNo, start, finish, srcPosition)
          && CanTransmitToDst(channelNo, start, finish, dstPosition);
}

bool
UanMacCumacChannelManager::CanTransmitFromSrc (uint8_t channelNo, Time start, Time finish,
                                               Vector srcPosition)
{
//  std::cout << "UMCCM " << start.GetSeconds ()  << " " << finish.GetSeconds () << " " << srcPosition << std::endl;


  EntryList::iterator it = m_transmissions.begin ();
  for (; it != m_transmissions.end(); it++) {
    Entry &entry = *it;

//    std::cout << (int) channelNo << " <=> " << (int) entry.GetChannel () << std::endl;
    if (channelNo != entry.GetChannel ())
      continue;

    // Check if current tx will be interfered by the src node
//    std::cout << "Distance" << CalculateDistance (srcPosition, entry.GetDstPosition ()) << std::endl
    if (CalculateDistance (entry.GetDstPosition (), srcPosition) <= 550) {
      Time startTimeAtDst = start + CalculateDelay (srcPosition, entry.GetDstPosition ());
      Time finishTimeAtDst = finish + CalculateDelay (srcPosition, entry.GetDstPosition ());

//      std::cout << "UMCCM AT DST" << entry.GetStartTime ().GetSeconds () << " " << entry.GetFinishTime ().GetSeconds () << std::endl;
      if ((entry.GetStartTime () >= startTimeAtDst && entry.GetStartTime () <= finishTimeAtDst)
              || (entry.GetFinishTime () >= startTimeAtDst && entry.GetFinishTime () <= finishTimeAtDst))
        return false;
    }
  }

  return true;


}

bool
UanMacCumacChannelManager::CanTransmitToDst (uint8_t channelNo, Time start, Time finish,
                                             Vector dstPosition)
{


  EntryList::iterator it = m_transmissions.begin ();
  for (; it != m_transmissions.end(); it++) {
    Entry &entry = *it;

    if (channelNo != entry.GetChannel ())
      continue;

    // Check if current tx will interfere with dst node
//    std::cout << "Distance" << CalculateDistance (entry.GetSrcPosition (), dstPosition) << std::endl;
    if (CalculateDistance (entry.GetSrcPosition (), dstPosition) <= 550) {
      Time startTimeAtDst = entry.GetStartTime () + CalculateDelay (entry.GetSrcPosition (), dstPosition);
      Time finishTimeAtDst = entry.GetFinishTime () + CalculateDelay (entry.GetSrcPosition (), dstPosition);

      if ((start >= startTimeAtDst && start <= finishTimeAtDst)
              || (finish >= startTimeAtDst && finish <= finishTimeAtDst))
          return false;
    }

  }

  return true;

}

}

