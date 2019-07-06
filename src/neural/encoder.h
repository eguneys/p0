#pragma once

#include "soko/position.h"
#include "neural/network.h"

namespace pzero {
  
  InputPlanes EncodePositionForNN(const PositionHistory& history,
                                  int history_planes);

} // namespace pzero
