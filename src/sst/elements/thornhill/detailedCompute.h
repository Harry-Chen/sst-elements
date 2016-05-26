// Copyright 2013-2016 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2013-2016, Sandia Corporation
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef _H_THORNHILL_DETAILED_COMPUTE
#define _H_THORNHILL_DETAILED_COMPUTE

#include <sst/core/module.h>
//#include <sst/core/component.h>
#include <sst/core/subcomponent.h>

namespace SST {
namespace Thornhill {

class DetailedCompute : public SubComponent {

  public:
    DetailedCompute( SST::Component* owner ) : SubComponent( owner ) {}

    virtual ~DetailedCompute(){};
    virtual void start( std::string& name, SST::Params& params,
                 std::function<int()> ) = 0;
    virtual bool isConnected() = 0;
};

}
}

#endif