/*
 * StackRawHandshakeReleaser.cpp
 *
 *  Created on: Apr 23, 2026
 *      Author: iizuka
 */

#include "bc_p2p/StackRawHandshakeReleaser.h"

#include "pubsub/P2pHandshake.h"

namespace codablecash {

StackRawHandshakeReleaser::StackRawHandshakeReleaser(P2pHandshake* handshake) {
	this->handshake = handshake;
}

StackRawHandshakeReleaser::~StackRawHandshakeReleaser() {
	if(this->handshake != nullptr){
		this->handshake->decRef();
	}
}

} /* namespace codablecash */
