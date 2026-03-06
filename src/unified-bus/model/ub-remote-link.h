// SPDX-License-Identifier: GPL-2.0-only
#ifndef UB_REMOTE_LINK_H
#define UB_REMOTE_LINK_H

#include "ns3/ub-link.h"

namespace ns3 {

class UbRemoteLink : public UbLink
{
  public:
    static TypeId GetTypeId(void);

    UbRemoteLink();

    ~UbRemoteLink() override;

    bool TransmitStart(Ptr<Packet> p, Ptr<UbPort> src, Time txTime) override;

    bool IsRemote(void) const override;
};

} // namespace ns3

#endif /* UB_REMOTE_LINK_H */
