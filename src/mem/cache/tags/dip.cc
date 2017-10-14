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
 * Definitions of a DIP tag store.
 */

#include "debug/CacheRepl.hh"
#include "mem/cache/tags/dip.hh"
#include "mem/cache/base.hh"

int DIP::bipRunningCounter=0;
int DIP:: epsInverse=32;
int DIP:: psel=512; //integer equivalent of 10 bit saturating counter. Takes values from 0 - 1023.
int DIP:: followerPolicy = DIP::POLICY_BIP; //setting default policy.

void
DIP:: incrementBipCounter()
{
	/*cyclic counter between 0 to 31*/
	bipRunningCounter = (bipRunningCounter +1)%epsInverse;
}

void
DIP:: incrementPsel(){
	/*saturating counter i.e. does not go beyonf max value = 1023*/
	if(psel < 1023)
		psel=psel+1;
}

void
DIP:: decrementPsel(){
	/*saturating counter does not go below min value = 0*/
	if(psel>0)
		psel = psel-1;
}

void
DIP:: compareAndSetFollower(){
	/*If psel 10 bit counter MSB 1: BIP, MSB 0:LRU
	Using integer equivalents, follower sets use LRU policy if psel<512. BIP otherwise.*/
	if(psel<512)
		DIP::followerPolicy=DIP::POLICY_LRU;
	else
		DIP::followerPolicy=DIP::POLICY_BIP;
}

/**
	function implements Insertion policy as in LRU.
	i.e. new block moved to head / Most recently used position.
*/
void
DIP:: insertBlockLru(int set, BlkType* blk){
	sets[set].moveToHead(blk);
}

/**
	function implements Bimodal Insertion policy.
	Insert at tail/Lru position. With a probability of epsilon insert at head.
	bip is implemented using a running counter and inserting at head when counter=0
*/
void
DIP:: insertBlockBip(int set, BlkType* blk){
	if(DIP::bipRunningCounter == 0)
		sets[set].moveToHead(blk);
	else
	    sets[set].moveToTail(blk);

    DIP::incrementBipCounter();

}

DIP::DIP(const Params *p)
    : BaseSetAssoc(p)
{
}


BaseSetAssoc::BlkType*
DIP::accessBlock(Addr addr, bool is_secure, Cycles &lat, int master_id)
{
    BlkType *blk = BaseSetAssoc::accessBlock(addr, is_secure, lat, master_id);

    if (blk != NULL) {
        // move this block to head of the MRU list
        sets[blk->set].moveToHead(blk);
        DPRINTF(CacheRepl, "set %x: moving blk %x (%s) to MRU\n",
                blk->set, regenerateBlkAddr(blk->tag, blk->set),
                is_secure ? "s" : "ns");
    }

    return blk;
}

BaseSetAssoc::BlkType*
DIP::findVictim(Addr addr) const
{
    int set = extractSet(addr);
    // grab a replacement candidate
    BlkType *blk = sets[set].blks[assoc - 1];

    if (blk->isValid()) {
        DPRINTF(CacheRepl, "set %x: selecting blk %x for replacement\n",
                set, regenerateBlkAddr(blk->tag, set));
    }

    return blk;
}

void
DIP::insertBlock(PacketPtr pkt, BlkType *blk)
{
    BaseSetAssoc::insertBlock(pkt, blk);

    int set = extractSet(pkt->getAddr());

	if(set == 0||set%33 == 0){
		/*Dedicated LRU set*/
		incrementPsel();
		insertBlockLru(set,blk);
	}else if(set%31 == 0){
		/*dedicated BIP set*/
		decrementPsel();
		insertBlockBip(set,blk);
	}else{
		/*follower set, choose policy based on psel counter value*/
		compareAndSetFollower();

		if(followerPolicy==POLICY_LRU)
			insertBlockLru(set,blk);
		else
			insertBlockBip(set,blk);
	}
}


void
DIP::invalidate(BlkType *blk)
{
    BaseSetAssoc::invalidate(blk);

    // should be evicted before valid blocks
    int set = blk->set;
    sets[set].moveToTail(blk);
}

DIP*
DIPParams::create()
{
    return new DIP(this);
}
