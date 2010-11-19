/*
* ============================================================================
*
* Copyright [2010] maidsafe.net limited
*
* Description:  Class for manipulating database of system packets
* Version:      1.0
* Created:      14/10/2010 11:43:59
* Revision:     none
* Company:      maidsafe.net limited
*
* The following source code is property of maidsafe.net limited and is not
* meant for external use.  The use of this code is governed by the license
* file LICENSE.TXT found in the root of this directory and also on
* www.maidsafe.net.
*
* You are not free to copy, amend or otherwise use this source code without
* the explicit written permission of the board of directors of maidsafe.net.
*
* ============================================================================
*/

#ifndef MAIDSAFE_PASSPORT_SYSTEMPACKETHANDLER_H_
#define MAIDSAFE_PASSPORT_SYSTEMPACKETHANDLER_H_

#include <boost/thread/mutex.hpp>
#include <boost/tr1/memory.hpp>
#include <map>
#include <string>
#include "maidsafe/passport/systempackets.h"

namespace maidsafe {

namespace passport {

class SystemPacketHandler {
 public:
  SystemPacketHandler() : packets_(), mutex_() {}
  ~SystemPacketHandler() {}
  // Add packet which is pending confirmation of storing
  bool AddPendingPacket(std::tr1::shared_ptr<pki::Packet> packet);
  // Change packet from pending to stored
  int ConfirmPacket(std::tr1::shared_ptr<pki::Packet> packet);
  // Removes a pending packet (leaving last stored copy)
  bool RevertPacket(const PacketType &packet_type);
  // Returns a *copy* of the confirmed packet
  std::tr1::shared_ptr<pki::Packet> Packet(const PacketType &packet_type);
  // Returns a *copy* of the pending packet
  std::tr1::shared_ptr<pki::Packet> PendingPacket(
      const PacketType &packet_type);
  bool Confirmed(const PacketType &packet_type);
  std::string SerialiseKeyring();
  int ParseKeyring(const std::string &serialised_keyring);
  void ClearKeyring();
  void Clear();
 private:
  struct PacketInfo {
    PacketInfo() : pending(), stored() {}
    explicit PacketInfo(std::tr1::shared_ptr<pki::Packet> pend)
        : pending(pend), stored() {}
    std::tr1::shared_ptr<pki::Packet> pending, stored;
  };
  typedef std::map<PacketType, PacketInfo> SystemPacketMap;
  SystemPacketHandler &operator=(const SystemPacketHandler&);
  SystemPacketHandler(const SystemPacketHandler&);
  std::tr1::shared_ptr<pki::Packet> GetPacket(const PacketType &packet_type,
                                              bool confirmed);
  bool IsConfirmed(SystemPacketMap::iterator it);
  SystemPacketMap packets_;
  boost::mutex mutex_;
};

}  // namespace passport

}  // namespace maidsafe

#endif  // MAIDSAFE_PASSPORT_SYSTEMPACKETHANDLER_H_
