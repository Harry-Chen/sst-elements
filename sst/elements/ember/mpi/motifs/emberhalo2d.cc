// Copyright 2009-2014 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2014, Sandia Corporation
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.


#include <sst_config.h>

#include <stdint.h>
#include <sst/core/math/sqrt.h>

#include "emberhalo2d.h"

using namespace SST::Ember;

EmberHalo2DGenerator::EmberHalo2DGenerator(SST::Component* owner, Params& params) :
	EmberMessagePassingGenerator(owner, params),
	m_loopIndex(0)
{
	m_name = "Halo2D";

	iterations = (uint32_t) params.find_integer("arg.iterations", 10);
	nsCompute = (uint32_t) params.find_integer("arg.computenano", 10);
	messageSizeX = (uint32_t) params.find_integer("arg.messagesizey", 128);
	messageSizeY = (uint32_t) params.find_integer("arg.messagesizex", 128);

	nsCopyTime = (uint32_t) params.find_integer("arg.computecopy", 5);

	sizeX = (uint32_t) params.find_integer("arg.sizex", 0);
	sizeY = (uint32_t) params.find_integer("arg.sizey", 0);

	// Set configuration so we do not exchange messages
	procWest  = 0;
	procEast = 0;
	procNorth = 0;
	procSouth = 0;

	sendWest = false;
	sendEast = false;
	sendNorth = false;
	sendSouth = false;

	messageCount = 0;
}

void EmberHalo2DGenerator::configure()
{
	// Do we need to auto-size the 2D processor array?
	if(0 == sizeX || 0 == sizeY) {
		uint32_t localX = SST::Math::square_root(size());

		while(localX >= 1) {
			uint32_t localY = (uint32_t) size() / localX;

			if(localY * localX == (unsigned) size()) {
				break;
			}

			localX--;
		}

		sizeX = localX;
		sizeY = size() / sizeX;
	}

	// Check we are not leaving ranks behind
	assert((unsigned) size() == (sizeX * sizeY));

	m_output->verbose(CALL_INFO, 2, 0, "Processor grid dimensions, X=%" PRIu32 ", Y=%" PRIu32 "\n",
		sizeX, sizeY);

	if(rank() % sizeX > 0) {
		sendWest = true;
		procWest = rank() - 1;
	}

	if((rank() + 1) % sizeX != 0) {
		sendEast = true;
		procEast = rank() + 1;
	}

	if( (unsigned) rank() >= sizeX) {
		sendNorth = true;
		procNorth = rank() - sizeX;
	}

	if( (unsigned) rank() < (size() - sizeX)) {
		sendSouth = true;
		procSouth = rank() + sizeX;
	}

	m_output->verbose(CALL_INFO, 2, 0, "Rank: %" PRIu32 ", send left=%s %" PRIu32 ", send right= %s %" PRIu32
		", send above=%s %" PRIu32 ", send below=%s %" PRIu32 "\n",
		rank(),
		(sendWest  ? "Y" : "N"), procWest,
		(sendEast ? "Y" : "N"), procEast,
		(sendSouth ? "Y" : "N"), procSouth,
		(sendNorth ? "Y" : "N"), procNorth);
}

void EmberHalo2DGenerator::finish(const SST::Output* output, uint64_t ) {
    output->verbose(CALL_INFO, 2, 0, "Generator finishing, sent: %" PRIu32 " messages.\n", messageCount);
}

bool EmberHalo2DGenerator::generate( std::queue<EmberEvent*>& evQ) {

    if( 0 == m_loopIndex) {
        initOutput();
        GEN_DBG( 1, "rank=%d size=%d\n", rank(),size());
    }

		m_output->verbose(CALL_INFO, 2, 0, "Halo 2D motif generating events for loopIndex %" PRIu32 "\n", m_loopIndex);

		enQ_compute( evQ, nsCompute);

		// Do the horizontal exchange first
		if(rank() % 2 == 0) {
			if(sendEast) {
				enQ_recv( evQ, procEast, messageSizeX, 0, GroupWorld);
				enQ_send( evQ, procEast, messageSizeX, 0, GroupWorld);
				messageCount++;
			}

			if(sendWest) {
				enQ_recv( evQ, procWest, messageSizeX, 0, GroupWorld);
				enQ_send( evQ, procWest, messageSizeX, 0, GroupWorld);
				messageCount++;
			}
		} else {
			if(sendWest) {
				enQ_send( evQ, procWest, messageSizeX, 0, GroupWorld);
				enQ_recv( evQ, procWest, messageSizeX, 0, GroupWorld);
				messageCount++;
			}

			if(sendEast) {
				enQ_send( evQ, procEast, messageSizeX, 0, GroupWorld);
				enQ_recv( evQ, procEast, messageSizeX, 0, GroupWorld);
				messageCount++;
			}
		}

		// Add a compute event to allow any copying for diagonals
		enQ_compute( evQ, nsCopyTime );

		// Now do the vertical exchanges
		if( (rank() / sizeX) % 2 == 0) {
			if(sendNorth) {
				enQ_recv( evQ, procNorth, messageSizeY, 0, GroupWorld);
				enQ_send( evQ, procNorth, messageSizeY, 0, GroupWorld);
				messageCount++;
			}

			if(sendSouth) {
				enQ_recv( evQ, procSouth, messageSizeY, 0, GroupWorld);
				enQ_send( evQ, procSouth, messageSizeY, 0, GroupWorld);
				messageCount++;
			}
		} else {
			if(sendSouth) {
				enQ_send( evQ, procSouth, messageSizeY, 0, GroupWorld);
				enQ_recv( evQ, procSouth, messageSizeY, 0, GroupWorld);
				messageCount++;
			}

			if(sendNorth) {
				enQ_send( evQ, procNorth, messageSizeY, 0, GroupWorld);
				enQ_recv( evQ, procNorth, messageSizeY, 0, GroupWorld);
				messageCount++;
			}
		}

	if ( ++m_loopIndex == iterations ) {
        return true;
    } else {
        return false;
    }
}
