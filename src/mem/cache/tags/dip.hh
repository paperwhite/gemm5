/*
 * Copyright (c) 2012-2013 ARM Limited
 * All rights reserved.
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2003-2005,2014 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Erik Hallnor
 */

/**
 * @file
 * Declaration of a LRU tag store.
 * The LRU tags guarantee that the true least-recently-used way in
 * a set will always be evicted.
 */

#ifndef __MEM_CACHE_TAGS_DIP_HH__
#define __MEM_CACHE_TAGS_DIP_HH__

#include "mem/cache/tags/base_set_assoc.hh"
#include "params/DIP.hh"

class DIP : public BaseSetAssoc
{
  public:
    /**variables for -----BIP-----
	Implementing BIP using the 1 out of 2^n policy where n=log2(epsinverse)
	A running counter "bipRunningCounter": saturating counter from 0 to epsInverse-1. 
	"epsInverse" : stores 1/epsilon. stored as inverse for ease of computation
	*/
	static int bipRunningCounter; //maintains a saturating counter from 0 to epsInverse-1
	static int epsInverse; //stores inverse of epsilon-probability of inserting new block to MRU position
	

	/**variables for -----Dedicated set Selection policy-----
		psel: policy selector ranges from 0 to 1023. integer equivalent of 10bit saturating counter
		followerPolicy: policy of follower sets. chooses from {POLICY_LRU,POLICY_BIP}
	*/
	static int psel;
	static int const POLICY_LRU=0;
	static int const POLICY_BIP=1;
	static int const FOLLOWER = 2;
	static int followerPolicy;

	/** Convenience typedef. */
    typedef DIPParams Params;

    /**
     * Construct and initialize this tag store.
     */
    DIP(const Params *p);

    /**
     * Destructor
     */
    ~DIP() {}

    BlkType* accessBlock(Addr addr, bool is_secure, Cycles &lat,
                         int context_src);
    BlkType* findVictim(Addr addr) const;
    void insertBlock(PacketPtr pkt, BlkType *blk);
    void invalidate(BlkType *blk);

	/**functions added to support extra comparision logic in dip*/
	void incrementBipCounter();
	void incrementPsel();
	void decrementPsel();
	void compareAndSetFollower();
	void insertBlockLru(int set,BlkType *blk);
	void insertBlockBip(int set,BlkType *blk);

};

#endif // __MEM_CACHE_TAGS_DIP_HH__
