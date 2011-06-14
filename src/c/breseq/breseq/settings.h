/*****************************************************************************

AUTHORS

  Jeffrey E. Barrick <jeffrey.e.barrick@gmail.com>
  David B. Knoester

LICENSE AND COPYRIGHT

  Copyright (c) 2008-2010 Michigan State University
  Copyright (c) 2011 The University of Texas at Austin

  breseq is free software; you can redistribute it and/or modify it under the  
  terms the GNU General Public License as published by the Free Software 
  Foundation; either version 1, or (at your option) any later version.

*****************************************************************************/


#ifndef _BRESEQ_SETTINGS_H_
#define _BRESEQ_SETTINGS_H_

#include <map>
#include <string>
#include <vector>
#include <list>
#include <stdint.h>

using namespace std;

namespace breseq {
	  
  // We need to be able to group read files for two reasons
  // 1) They may be paired-end, so we want to map them together
  // 2) They may have the same error rates, so we want to treat them together for error analysis

  struct cReadFile {
  public:
    string m_fastq_file_name;
    string m_base_name;
    uint32_t m_paired_end_group;    // indicated what file contains paired reads
    uint32_t m_error_group;         // indicates what other read files have the same error rates
    uint32_t m_id;                  // index used to refer to this fastq file in BAM
  };
  
  typedef vector<vector<cReadFile> > cReadFileGroup;
  
  
  class cReadFiles : public vector<cReadFile> {
    
  protected:
    
  public:
    cReadFiles() {};
    cReadFiles(const vector<string>& read_file_names);
    ~cReadFiles() {};
    
    void Init(const vector<string>& read_file_names);
    
  };
  
  struct Settings
	{
    
    // Set up defaults here
    Settings() {
      
      max_read_mismatches = -1;
      require_complete_match = false;
      
    }
    
		// Fields
    
		map<string, bool> installed;
		string bin_path;
    
		string candidate_junction_score_method;
    
		string candidate_junction_fasta_file_name;
		string candidate_junction_faidx_file_name;
		string candidate_junction_sam_file_name;
		string jc_genome_diff_file_name;
		string preprocess_junction_best_sam_file_name;
		string preprocess_junction_split_sam_file_name;
		string reference_fasta_file_name;
		string reference_faidx_file_name;
		string reference_sam_file_name;
		string resolved_reference_sam_file_name;
		string resolved_junction_sam_file_name;
		string unmatched_read_file_name;
    
		bool no_junction_prediction;
		bool unmatched_reads;
		bool add_split_junction_sides;
		bool require_complete_match;
    
		int32_t alignment_read_limit;
		int32_t candidate_junction_read_limit;
		int32_t minimum_candidate_junction_pos_hash_score;
		int32_t minimum_candidate_junction_min_overlap_score;
		int32_t minimum_candidate_junctions;
		int32_t maximum_candidate_junctions;
		int32_t maximum_candidate_junction_length_factor;
		int32_t max_read_length;
		int32_t maximum_inserted_junction_sequence_length;
		int32_t max_read_mismatches;
		int32_t required_both_unique_length_per_side;
		int32_t required_one_unique_length_per_side;
		int32_t required_extra_pair_total_length;
		int32_t required_match_length;
    
		int32_t preprocess_junction_min_indel_split_length;
    
		cReadFiles read_structures;
    
		// Utility function to substitute specific details into a generic file name
		static string file_name(string file_name_key)
		{
			return file_name_key;
      
      //			my ($self, $file_name_key, $sub_hash)= @_;
      //			my $file_name = $self->{$file_name_key};
      //			$file_name or $self->throw("Settings file \"$file_name_key\" not found.");
      //
      //			return $self->substitute_file_name($file_name, $sub_hash);
		}
    
		string ctool(string tool_name)
		{
      //			my ($self, $tool_name, $allow_fail) = @_;
      //
      //			if (!$self->{installed}->{$tool_name})
      //			{
      //				if ($allow_fail)
      //				{
      //					$self->warn("Executable \"$tool_name\" not found in breseq bin path\"$self->{bin_path}\".");
      //					return undef; # couldn't find it, but it's not an error.
      //				}
      //				else
      //				{
      //					$self->throw("Executable \"$tool_name\" not found in breseq bin path\"$self->{bin_path}\".");
      //				}
      //			}
      
			return bin_path + "/" + tool_name;
		}
	};

  struct Summary
	{
		// Fields
    
		struct AlignmentCorrection
		{
			string read_file;
			struct NewJunction
			{
				int32_t observed_min_overlap_score_distribution;
				int32_t accepted_min_overlap_score_distribution;
				int32_t observed_pos_hash_score_distribution;
				int32_t accepted_pos_hash_score_distribution;
			};
			list<NewJunction> new_junctions;
      
		} alignment_correction;
    
		struct PreprocessCoverage
		{
			int32_t junction_accept_score_cutoff_1;
			int32_t junction_accept_score_cutoff_2;
		} preprocess_coverage;
    
		struct CandidateJunctionSummaryData
		{
			struct Total
			{
				int32_t number;
				int32_t length;
			} total;
      
			struct Accepted
			{
				int32_t number;
				int32_t length;
				int32_t pos_hash_score_cutoff;
				int32_t min_overlap_score_cutoff;
			} accepted;
      
			map<int32_t, int32_t> pos_hash_score_distribution;
			map<int32_t, int32_t> min_overlap_score_distribution;
      
			map<string, map<string, int32_t> > read_file;
		} candidate_junction;
    
		struct SequenceConversion
		{
			int32_t total_reference_sequence_length;
		} sequence_conversion;
	};

  
  
} // breseq namespace

#endif