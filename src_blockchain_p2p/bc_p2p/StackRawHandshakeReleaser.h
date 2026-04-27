/*
 * StackRawHandshakeReleaser.h
 *
 *  Created on: Apr 23, 2026
 *      Author: iizuka
 */

#ifndef BC_P2P_STACKRAWHANDSHAKERELEASER_H_
#define BC_P2P_STACKRAWHANDSHAKERELEASER_H_

namespace codablecash {

class P2pHandshake;

class StackRawHandshakeReleaser {
public:
	explicit StackRawHandshakeReleaser(P2pHandshake* handshake);
	virtual ~StackRawHandshakeReleaser();

private:
	P2pHandshake* handshake;
};

} /* namespace codablecash */

#endif /* BC_P2P_STACKRAWHANDSHAKERELEASER_H_ */
