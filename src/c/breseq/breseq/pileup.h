/*****************************************************************************

AUTHORS

  Jeffrey E. Barrick <jeffrey.e.barrick@gmail.com>
  David B. Knoester

LICENSE AND COPYRIGHT

  Copyright (c) 2010 Michigan State University

  breseq is free software; you can redistribute it and/or modify it under the  
  terms the GNU General Public License as published by the Free Software 
  Foundation; either version 1, or (at your option) any later version.

*****************************************************************************/

#ifndef _BRESEQ_PILEUP_H_
#define _BRESEQ_PILEUP_H_

#include <vector>
#include <bam.h>

#include "breseq/alignment.h"
#include "breseq/pileup_base.h"

namespace breseq {
	
	// pre-decs:
	class pileup_base;

	/*! Represents a pileup of alignments.
	 */
	class pileup : public std::vector<alignment> {
	public:
		//! Constructor for this pileup.
		pileup(uint32_t tid, uint32_t pos, int n, const bam_pileup1_t *pile, pileup_base& pb);
		
		//! Retrieve the target id for this pileup.
		inline uint32_t target() const { return _tid; }
    
    inline uint32_t target_length() const { return _pb.target_length(_tid); } 
		
		//! Retrieve the name of this target.
		inline const char* target_name() const { return _pb.target_name(_tid); }
		
		//! Retrieve the position of this pileup on the reference sequence (0-indexed).
		inline uint32_t position_0() const { return _pos; }

		//! Retrieve the position of this pileup on the reference sequence (1-indexed).
		inline uint32_t position_1() const { return _pos+1; }

		//! Retrieve the reference sequence for this pileup.
		char* reference_sequence() const;
    
    inline char reference_base_char_0(uint32_t pos0) const 
      { return reference_sequence()[pos0]; } ;

    inline char reference_base_char_1(uint32_t pos1) const 
      { return reference_sequence()[pos1-1]; } ;
      
    //stub function for dealing properly with falling off the end of the genome
    inline uint32_t wrap_position(int32_t) const { assert(false); return 0; } ;

	protected:
		uint32_t _tid; //!< Target id for this pileup.
		uint32_t _pos; //!< Position of this pileup in the reference sequence. 0-indexed.
		pileup_base& _pb; //!< Pileup base class that built this pileup.
	};
	
}

#endif
