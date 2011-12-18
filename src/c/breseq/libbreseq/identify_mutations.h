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

#ifndef _BRESEQ_IDENTIFY_MUTATIONS_H_
#define _BRESEQ_IDENTIFY_MUTATIONS_H_

#include "common.h"
#include "chisquare.h"
#include "error_count.h"
#include "genome_diff.h"
#include "pileup_base.h"

using namespace std;

namespace breseq {
	      
	/*! Calculate errors in the given BAM file based on reference FAI files.
	 
	 \param bam is the read file generated by breseq.
	 \param fastas is the list of FASTA files that correspond to the reference sequences; generates FAI files as needed.
	 \param output_dir is the directory in which output files will be placed.
	 \param readfiles is a list of read files that were used to build the bam (do not include filename extension)
	 */
	void identify_mutations(const string& bam,
													const string& fasta,
													const string& error_dir,
													const string& gd_file,
													const string& output_dir,
													const vector<string>& readfiles,
													const string& coverage_dir,
                          const vector<double>& deletion_propagation_cutoff,
                          const vector<double>& deletion_seed_cutoff,
													double mutation_cutoff,
													bool predict_deletions,
													bool predict_polymorphisms,
                          uint8_t min_qual_score,
                          double polymorphism_cutoff,
                          double polymorphism_frequency_cutoff,
                          const string& error_table_file,
                          bool print_per_position_file
                          );
	
	
	/*! Position information struct.
	 
	 Note: The "triples" in this struct are to be indexed via the strand, which is
	 either +1 or -1.  As a result, use 1-based indexing, e.g.: unique_cov[1+strand].
	 */
	struct position_info {
		//! Constructor.
		position_info() {
			bzero(this,sizeof(position_info));
		}
		
		int unique_cov[3];
		int unique_trimmed_cov[3];
		int mutation_cov[3];
	};
	
	
	/*! Position coverage struct.
	 
	 Note: The "triples" in this struct are to be indexed via the strand, which is
	 either +1 or -1.  As a result, use 1-based indexing, e.g.: unique_cov[1+strand].
	 Additionally, we use the 1th element as the total field for both strands.
	 */
	struct position_coverage {
		//! Constructor.
		position_coverage() {
			bzero(this,sizeof(position_coverage));
		}

		//! Constructor.
		position_coverage(double v) {
			bzero(this,sizeof(position_coverage));
			unique[0] = v; unique[1] = v; unique[2] = v;
			redundant[0] = v; redundant[1] = v; redundant[2] = v;
		}
		
		position_coverage& operator=(const position_coverage& that) {
			if(this != &that) {
				memcpy(this, &that, sizeof(that));
			}
			return *this;
		}
		
		//! Sum the position coverage fields.
		void sum() {
			unique[1] = unique[0]+unique[2];
			redundant[1] = redundant[0]+redundant[2];
			raw_redundant[1] = raw_redundant[0]+raw_redundant[2];
			total = (int)round(unique[1]) + (int)round(redundant[1]);
		}
		
		double unique[3];
		double redundant[3];
		int raw_redundant[3];
		int total;
	};
	

	/*! Polymorphism data struct.
	 */
	struct polymorphism_data {
		//! Constructor.
		polymorphism_data(uint8_t b, uint8_t q, int s, int32_t f, const covariate_values_t& cv)
		: _base_char(b), _quality(q), _strand(s), _fastq_file_index(f), _cv(cv) {
		}
		
		polymorphism_data(uint8_t b, uint8_t q, int s, int32_t f)
		: _base_char(b), _quality(q), _strand(s), _fastq_file_index(f) {
		}

		base_char _base_char;
		uint8_t _quality;
		int _strand;
		int32_t _fastq_file_index;
    covariate_values_t _cv;
	};

	/*! Polymorphism prediction data struct.
	 */
	struct polymorphism_prediction {
		//! Constructor.
		polymorphism_prediction(double f = 0.0, double l = 0.0, double p = 0.0)
		: frequency(f), log10_base_likelihood(l), p_value(p) {
      log10_e_value = NAN;
		}
		
		double frequency;
		double log10_base_likelihood;
		long double p_value;
    double log10_e_value;
	};	
	
  
	/*! Error-counting class.
	 
	 This class is used by the above identify_mutations() function in order to count errors.
	 */
	class identify_mutations_pileup : public breseq::pileup_base {
	public:
		typedef map<string,int> base_count_t;
		typedef map<uint8_t,base_count_t> qual_map_t;
    typedef map<int32_t,qual_map_t> fastq_map_t;
    
    //! Information that is tracked per-sequence.
		struct sequence_info {
			/*! Coverage count table.
			 
			 This is a table of non-deletion reads per position to non-redundancy counts.
			 For example, given unique_only_coverage[i] = x, for all aligned positions p:
			 i is the number of reads that do not indicate a deletion at p
			 x is the number of positions that have no redundancies
			 */
			vector<int> unique_only_coverage;
		};
		
		struct shared_info {
			shared_info() : coverage_unique_total(0), coverage_unique_uncalled(0), coverage_unique_called(0) { }
			int coverage_unique_total;
			int coverage_unique_uncalled;
			int coverage_unique_called;
		};
		
		
		//! Constructor.
		identify_mutations_pileup(
                              const string& bam,
															const string& fasta,
															const string& error_dir,
															const string& gd_file,
															const string& output_dir,
															const vector<string>& readfiles,
															const string& coverage_dir,
                              const vector<double>& deletion_propagation_cutoff,
                              const vector<double>& deletion_seed_cutoffs,
															double mutation_cutoff,
															bool predict_deletions,
															bool predict_polymorphisms,
                              uint8_t min_qual_score,
                              double polymorphism_cutoff,
                              double polymorphism_frequency_cutoff,
                              const string& error_table_file,
                              bool print_per_position_file
                            );
				
		//! Destructor.
		virtual ~identify_mutations_pileup();		
		
		//! Called for each alignment.
		virtual void pileup_callback(const pileup& p);
		
		//! Called at the end of the pileup.
		virtual void at_target_end(const uint32_t tid);
		
	protected:
		//! Helper method to track deletions.
		void check_deletion_completion(uint32_t position, uint32_t seq_id, const position_coverage& this_position_coverage, double e_value_call);

		//! Helper method to track unknowns.
		void update_unknown_intervals(uint32_t position, uint32_t seq_id, bool base_predicted, bool this_position_unique_only_coverage);

		//! Predict whether there is a significant polymorphism.
    polymorphism_prediction predict_polymorphism (base_char best_base_char, base_char second_best_base_char, vector<polymorphism_data>& pdata );

		//! Find best mixture of two bases and likelihood of producing observed read bases.
    pair<double,double> best_two_base_model_log10_likelihood(base_char best_base_char, base_char second_best_base_char, vector<polymorphism_data>& pdata);

		//! Calculate likelihood of a specific mixture of two bases producing observed read bases.
    double calculate_two_base_model_log10_likelihood (base_char best_base_char, base_char second_best_base_char, const vector<polymorphism_data>& pdata, double best_base_freq);
		
    //! Settings passed at command line
		error_count_results _ecr; //!< Error count results.
		cGenomeDiff _gd; //!< Genome diff.
    string _gd_file; //!< file name for Genome diff
    uint8_t _min_qual_score; //!< minimum quality score to count base for RA
    vector<double> _deletion_seed_cutoffs; //!< Coverage below which deletions are cutoff.
    vector<double> _deletion_propagation_cutoffs; //!< Coverage above which deletions are cutoff.
		double _mutation_cutoff; //!< log10 e-value cutoff value for mutation predictions.
		bool _predict_deletions; //!< Whether to predict mutations.
		bool _predict_polymorphisms; //!< Whether to predict polymorphisms.
    double _polymorphism_cutoff; //!< log10 e-value cutoff for predicted polymorphisms.
    double _polymorphism_frequency_cutoff; //!< Frequency cutoff for predicted polymorphisms.
		const string _coverage_dir; //!< Directory in which to store coverage data.
    string _output_dir; //!< Directory containing output

    //! Settings calculated during initialization
		double _log10_ref_length; //!< log10 of the total reference sequence.
    
    //! Flag to use new error model...
    cErrorTable m_error_table;
    
		vector<sequence_info> _seq_info; //!< information about each sequence.
		fastq_map_t error_hash; //!< fastq_file_index -> quality map.
		shared_info s; // summary stats
		
		// this is used to output detailed coverage data:
		bool _print_coverage_data; //!< whether or not to print
		ofstream _coverage_data;

		// this is used to output strand and quality information for R to process:
		ofstream _polymorphism_r_input_file;
		
		// these are state variables used by the deletion-prediction method.
		uint32_t _on_deletion_seq_id;
    double _this_deletion_propagation_cutoff;
    double _this_deletion_seed_cutoff;
		uint32_t _last_deletion_start_position;
		uint32_t _last_deletion_end_position;
		uint32_t _last_deletion_redundant_start_position;
		uint32_t _last_deletion_redundant_end_position;
		bool _this_deletion_reaches_seed_value;
		bool _this_deletion_redundant_reached_zero;
		uint32_t _last_position_coverage_printed;
		position_coverage _left_outside_coverage_item;
		position_coverage _left_inside_coverage_item;
		position_coverage _last_position_coverage;
    
		bool _print_per_position_file;
		ofstream _per_position_file;
    
		// these are state variables used by the unknown prediction method.
		uint32_t _last_start_unknown_interval;
	};

  
  /*! cDiscreteSNPCaller
	 
	 This class is used to predicting SNPs in a single-genome sample.
   
	 */
  
  class cDiscreteSNPCaller {
  	public:
      cDiscreteSNPCaller(uint8_t ploidy);
        
      virtual ~cDiscreteSNPCaller() {};
      
      void update(const covariate_values_t& cv, bool obs_top_strand, cErrorTable& et);
      vector<double> get_log10_probabilities() { return _log10_probabilities; };
      pair<uint8_t,double> get_prediction();
      
    protected:
      uint32_t _observations;
      vector<double> _log10_priors;
      vector<double> _log10_probabilities;
      bool _normalized; 
      
      uint8_t _ploidy;
      error_count_results* _base_error_model;
  };
  
  
} // breseq namespace

#endif
