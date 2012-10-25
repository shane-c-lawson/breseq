 /*****************************************************************************

AUTHORS

  Jeffrey E. Barrick <jeffrey.e.barrick@gmail.com>
  David B. Knoester

LICENSE AND COPYRIGHT

  Copyright (c) 2008-2010 Michigan State University
  Copyright (c) 2011-2012 The University of Texas at Austin

  breseq is free software; you can redistribute it and/or modify it under the  
  terms the GNU General Public License as published by the Free Software 
  Foundation; either version 1, or (at your option) any later version.

*****************************************************************************/

#include <iostream>
#include <string>
#include <vector>

#include "libbreseq/anyoption.h"
#include "libbreseq/alignment_output.h"
#include "libbreseq/reference_sequence.h"
#include "libbreseq/calculate_trims.h"
#include "libbreseq/candidate_junctions.h"
#include "libbreseq/contingency_loci.h"
#include "libbreseq/coverage_distribution.h"
#include "libbreseq/coverage_output.h"
#include "libbreseq/error_count.h"
#include "libbreseq/fastq.h"
#include "libbreseq/genome_diff.h"
#include "libbreseq/identify_mutations.h"
#include "libbreseq/resolve_alignments.h"
#include "libbreseq/settings.h"
#include "libbreseq/summary.h"
#include "libbreseq/contingency_loci.h"
#include "libbreseq/mutation_predictor.h"
#include "libbreseq/rna_seq.h"
#include "libbreseq/output.h"

using namespace breseq;
using namespace std;


/*! bam2aln
 Draw HTML alignment from BAM
 */
int do_bam2aln(int argc, char* argv[]) {
  
  // setup and parse configuration options:
	AnyOption options("Usage: breseq BAM2ALN [--bam=reference.bam --fasta=reference.fasta --region=accession:start-end --output=alignment.html --max-reads=200] accession:start-end");
	options
  ("help,h", "produce this help message", TAKES_NO_ARGUMENT)
  ("bam,b", "bam file containing sequences to be aligned", "data/reference.bam")
	("fasta,f", "FASTA file of reference sequence", "data/reference.fasta")
  ("output,o", "output to file [region.html]", "alignment.html")
  ("region,r", "region to print (accession:start-end), may also be provided as unnamed arguments at the end of the command line.")
  ("max-reads,n", "maximum number of reads to show in alignment", 200)
  ("repeat", "show reads with multiple best matches in reference", TAKES_NO_ARGUMENT)
  ("quality-score-cutoff,c", "quality score cutoff", 0)
  ("stdout", "write output to stdout", TAKES_NO_ARGUMENT)
  .processCommandArgs(argc, argv);  
  
	// make sure that the config options are good:
	if(options.count("help")
     || !file_exists(options["fasta"].c_str())
     || !file_exists(options["bam"].c_str()) )
  {
		options.printUsage();
		return -1;
	}
  
  vector<string> region_list;
  if (options.count("region"))
    region_list= from_string<vector<string> >(options["region"]);
  
  // Also take regions off the command line
  for (int32_t i = 0; i < options.getArgc(); i++)
  {
    string region = options.getArgv(i);
    region_list.push_back(region);
  }  
  
  if (!region_list.size()) {
    options.addUsage("");
    options.addUsage("You must supply the --region option for input.");
    options.printUsage();
    return -1;
  }  
  
  for(uint32_t j = 0; j < region_list.size(); j++)
  {
    // clean commas
    region_list[j] = substitute(region_list[j], ",", "");
    
    // Generate Alignment!
    alignment_output ao(
                        options["bam"],
                        options["fasta"],
                        from_string<uint32_t>(options["max-reads"]),
                        from_string<uint32_t>(options["quality-score-cutoff"]),
                        options.count("repeat")
                        );
    
    string html_output = ao.html_alignment(region_list[j]);
    
    if (options.count("stdout"))
    {
      cout << html_output << endl;
    }
    else
    {
      ///Write to html file
      string file_name = region_list[j] + ".html";
      if (options.count("output"))
      {
        file_name = options["output"];
        if(region_list.size() > 1)  {
          file_name = (split(options["output"], "."))[0] + "_" + region_list[j] + ".html";  }
      }
      
      ofstream myfile (file_name.c_str());
      if (myfile.is_open())
      {
        myfile << html_output;
        myfile.close();
      }
      else cerr << "Unable to open file";
    }
  }
    
  return 0;
}

/*! bam2cov
 Draw HTML coverage from BAM
 */
int do_bam2cov(int argc, char* argv[]) {
  // setup and parse configuration options:
	AnyOption options("Usage: bam2aln -b reference.bam -f reference.fasta [--plot-format PNG -o output.png] seq_id1:start1-end1 [seq_id2:start2-end2 ...]");
	options
  ("help,h", "Produce advanced help message", TAKES_NO_ARGUMENT)
  // required options
  ("bam,b", "BAM file containing sequences to be aligned", "data/reference.bam")
	("fasta,f", "FASTA file of reference sequence", "data/reference.fasta")
  ("output,o", "Base name of output files. Region specification (seq_id:start-end) appended if there are multiple output files. Defaults to seq_id:start-end for single regions.")
  // which regions to create files for
  ("tile-size", "In tiling mode, the size of each tile", 0, ADVANCED_OPTION)
  ("tile-overlap", "In tiling mode, overlap between adjacent tiles (1/2 on each side)", 0, ADVANCED_OPTION)
  // options controlling what files are output
  ("plot-format", "Format of output plot: PNG or PDF", "PNG")
  ("table,t", "Create tab delimited file of coverage instead of a plot", TAKES_NO_ARGUMENT)
//  ("read_start_output,r", "file name for table file binned by read start bases (DEFAULT: OFF)")
//  ("gc_output,g", "create additional table file binned by GC content of reads (DEFAULT: OFF)")
  // options controlling information that is output
  ("total-only,1", "Only plot/tabulate total coverage, not per strand coverage", TAKES_NO_ARGUMENT)
  ("resolution", "Number of positions to output coverage information for in interval (0=ALL)", 600)
  .processCommandArgs(argc, argv);
  
  options.addUsage("");
  options.addUsage("Create a coverage plot or table for the specified region or regions.");

  // Print advanced help
  if (options.count("help"))
  {
    options.printAdvanced();
    exit(-1);
  }
  
  // make sure that the required config options are good:
	if(   !file_exists(options["fasta"].c_str() )
     || !file_exists(options["bam"].c_str()   ) )
  {
		options.printUsage();
		return -1;
	}
  
  vector<string> region_list;
  
  // Take regions off the command line
  for (int32_t i = 0; i < options.getArgc(); i++)
  {
    string region = options.getArgv(i);
    region_list.push_back(region);
  }
  
  bool tiling_mode = options.count("tile-size") && options.count("tile-overlap");
  ASSERT(tiling_mode || (!options.count("tile-size") && !options.count("tile-overlap")),
          "--tile-size and --tile-overlap args must both be provided to activate tile mode");
  
  if (tiling_mode && (region_list.size() > 0))
  {
    WARN("Tiling mode activated. Ignoring " + to_string(region_list.size()) + " regions that were specified.");
    region_list.clear();
  }
  
  // Did they specify some regions?
  if(!tiling_mode && (region_list.size() == 0)) {
    options.addUsage("");
    options.addUsage("No regions specified.");
    options.printUsage();
    return -1;
  }
  
  // create empty settings object to have R script name
  Settings settings;
    
  // generate coverage table/plot!
  coverage_output co(
                      options["bam"],
                      options["fasta"],
                      settings.coverage_plot_r_script_file_name
                      );
  
  // Set options
  co.total_only(options.count("total-only"));
  co.output_format( options["plot-format"] );
  
  // create regions that tile the genome
  if (tiling_mode)
  {
    int32_t tile_size = from_string<int32_t>(options["tile-size"]);
    int32_t tile_overlap = from_string<int32_t>(options["tile-overlap"]);
    tile_overlap = static_cast<int32_t>(floor( static_cast<double>(tile_overlap) / 2.0));

    for(uint32_t target_id=0; target_id < co.num_targets(); target_id++)
    {
      const char* target_name = co.target_name(target_id);
      int32_t target_length = co.target_length(target_id);
      
      int32_t start = 1;
      while (start < target_length)
      {
        int32_t end = start + tile_size - 1;
        
        int32_t offset_start = start - tile_overlap;
        if (offset_start < 1) offset_start = 1;
        
        int32_t offset_end = end + tile_overlap;
        if (offset_end > target_length) offset_end = target_length;
        
        string region = target_name;
        region += ":" + to_string(offset_start) + "-" + to_string(offset_end);
        
        region_list.push_back(region);
        
        start += tile_size;
      }
    }
  }
  
  for(vector<string>::iterator it = region_list.begin(); it!= region_list.end(); it++)
  {
// these are experimental... additional table files
//    if (options.count("read_start_output"))
//      co.read_begin_output_file_name(options["read_start_output"]);
//    if (options.count("gc_output"))
//      co.gc_output_file_name(options["gc_output"]);
    
    // clean commas
    *it = substitute(*it, ",", "");
    
    string file_name = options["output"];
    if ((region_list.size() > 0) || (file_name == ""))  file_name += *it;
    
    if (options.count("table")) {
      cout << "Tabulating coverage for region: " << *it << endl;
      co.table(*it, file_name + ".tab", from_string<uint32_t>(options["resolution"]));
    } else {
      cout << "Plotting coverage for region: " << *it << endl;
      co.plot(*it, file_name + "." + to_lower(options["plot-format"]) , from_string<uint32_t>(options["resolution"]));
    }
  }
  
  return 0;
}

int do_convert_fastq(int argc, char* argv[])
{
  
	// setup and parse configuration options:
	AnyOption options("Usage: breseq ANALYZE_FASTQ --input input.fastq --convert converted.fastq");
	options
  ("help,h", "produce this help message", TAKES_NO_ARGUMENT)
  ("input,i", "input FASTQ file")
  ("output,o", "output FASTQ file")
  ("in-format,1", "format to convert from")
  ("out-format,2", "format to convert to")
  
  //("output,o", "out to files") // outputs to STDOUT for now
	.processCommandArgs(argc, argv);
	
	// make sure that the config options are good:
	if(options.count("help")
		 || !options.count("input")
     || !options.count("output")
     || !options.count("in-format")
     || !options.count("out-format")
		 ) {
    options.addUsage("Valid formats are 'SANGER', 'SOLEXA', 'ILLUMINA_1.3+'");
		options.printUsage();
		return -1;
	}                       
    
  convert_fastq(options["input"], options["output"], options["in-format"], options["out-format"]);

  return 0;
}

int do_convert_genbank(int argc, char* argv[]) {
	
	// setup and parse configuration options:
	AnyOption options("Usage: breseq CONVERT-REFERENCE --input <reference> [--fasta <output.fasta>] OR [--gff <output.gff>]");
	options
		("input,i", "Input reference file(s). (Format: Fasta, GFF, GenBank)")
		("fasta",   "FASTA format output path.")
    ("gff",     "GFF format output path.")
	.processCommandArgs(argc, argv);
	
	// make sure that the config options are good:
  if (!options.count("input")) {
    options.addUsage("No input reference file given.");
		options.printUsage();
		return -1;
	}

  if (!options.count("fasta") && !options.count("gff")) {
    options.addUsage("No output path and format given.");
		options.printUsage();
		return -1;
  }

  
  cReferenceSequences refs;
  refs.LoadFiles(from_string<vector<string> >(options["input"]));

  if (options.count("fasta")) refs.WriteFASTA(options["fasta"]);

  if (options.count("gff")) refs.WriteGFF(options["gff"]);

	
	return 0;
}


/*! Error Count
 
Calculate error calibrations from FASTA and BAM reference files.
 
 */
int do_error_count(int argc, char* argv[]) {
	  
	// setup and parse configuration options:
	AnyOption options("Usage: breseq ERROR_COUNT --bam reference.bam --fasta reference.fasta --output test --readfile reads.fastq --covariates ref_base,obs_base,quality=40 [--coverage] [--errors] [--minimum-quality-score 3]");
	options
		("help,h", "produce this help message", TAKES_NO_ARGUMENT)
		("bam,b", "bam file containing sequences to be aligned", "data/reference.bam")
		("fasta,f", "FASTA file of reference sequence", "data/reference.fasta")
		("output,o", "output directory", "./")
		("readfile,r", "name of readfile (no extension). may occur multiple times")
		("coverage", "generate unique coverage distribution output", TAKES_NO_ARGUMENT)
		("errors", "generate unique error count output", TAKES_NO_ARGUMENT)
    ("covariates", "covariates for error model. a comma separated list (no spaces) of these choices: ref_base, obs_base, prev_base, quality, read_set, ref_pos, read_pos, base_repeat. For quality, read_pos, and base_repeat you must specify the maximum value possible, e.g. quality=40")
    ("minimum-quality-score", "ignore base quality scores lower than this", 0)
	.processCommandArgs(argc, argv);
  
	// make sure that the config options are good:
	if(options.count("help")
		 || !options.count("readfile")
		 || (!options.count("coverage") && !options.count("errors")) ) {
		options.printUsage();
		return -1;
	}
  
  if(options.count("errors") && !options.count("covariates") ) {
    WARN("Must provide --covariates when --errors specified.");
		options.printUsage();
		return -1;
	}
	
	// attempt to calculate error calibrations:
	try {
    Summary summary;
    Settings settings;
		error_count(
                settings,
                summary,
                options["bam"],
                options["fasta"],
                options["output"],
                split(options["readfile"], "\n"),
                options.count("coverage"),
                options.count("errors"),
                false,
                from_string<uint32_t>(options["minimum-quality-score"]),
                options["covariates"]
                );
	} catch(...) {
		// failed; 
    std::cout << "<<<Failed>>>" << std::endl;
		return -1;
	}
  
	return 0;
}



/*! Contingency Loci
 
 Analyze lengths of homopolymer repeats in mixed samples.
 
 */
int do_tabulate_contingency_loci(int argc, char* argv[]) {
	
	// setup and parse configuration options:
	AnyOption options("Usage: breseq TABULATE_CL --bam <sequences.bam> --fasta <reference.fasta> --reference <reference.gff> --output <path> [--loci <loci.txt> --strict]");
	options
  ("help,h", "produce this help message", TAKES_NO_ARGUMENT)
  ("bam,b", "bam file containing sequences to be aligned", "data/reference.bam")
  ("fasta,f", "FASTA file of reference sequence", "data/reference.fasta")
  ("reference,r","reference sequence file with annotation", "data/reference.gff3")
  ("output,o", "output file", "contingency_loci.tab")
  ("loci,l", "Contingency loci coordinates", "")
  ("strict,s", "exclude non-perfect matches in surrounding 5 bases", TAKES_NO_ARGUMENT)
	.processCommandArgs(argc, argv);
  
	// make sure that the config options are good:
	if(options.count("help")
		 ) {
		options.printUsage();
		return -1;
	}                       
  
	// attempt to calculate error calibrations:
	try {
    
    analyze_contingency_loci(
                             options["bam"],
                             options["fasta"],
                             from_string<vector<string> >(options["reference"]),
                             options["output"],
                             options["loci"],
                             options.count("strict")
                             );
	} catch(...) {
		// failed; 
		return -1;
	}
	
	return 0;
}

int do_analyze_contingency_loci_significance( int argc, char* argv[]){
    // setup and parse configuration options:
	AnyOption options("Usage: breseq CL_SIGNIFICANCE --output <path> --loci <loci.txt> ");
	options
    ("help,h", "produce this help message", TAKES_NO_ARGUMENT)
    ("output,o", "output file", "contingency_loci.tab")
    ("loci,l", "Contingency loci files", "")
	.processCommandArgs(argc, argv);
    
  vector<string> strain_files = from_string<vector<string> >(options["loci"]);
  analyze_contingency_loci_significance(
                                       options["output"],
                                       strain_files
                                       );
  return 0;
}

int do_identify_candidate_junctions(int argc, char* argv[]) {

	// setup and parse configuration options:
	AnyOption options("Usage: breseq_utils IDENTIFY_CANDIDATE_JUNCTIONS --fasta=reference.fasta --sam=reference.sam --output=output.fasta");
	options
		("help,h", "produce this help message", TAKES_NO_ARGUMENT)
    ("candidate-junction-path", "path where candidate junction files will be created")
    ("data-path", "path of data")
    ("read-file,r", "FASTQ read files (multiple allowed, comma-separated)")

    ("candidate-junction-read-limit", "limit handled reads to this many", static_cast<unsigned long>(0))
    ("required-both-unique-length-per-side,1",
     "Only count reads where both matches extend this many bases outside of the overlap.", static_cast<unsigned long>(5))
    ("required-one-unique-length-per-side,2",
     "Only count reads where at least one match extends this many bases outside of the overlap.", static_cast<unsigned long>(10))
    ("require-match-length,4",
     "At least this many bases in the read must match the reference genome for it to count.", static_cast<unsigned long>(28))
    ("required-extra-pair-total-length,5",
     "Each match pair must have at least this many bases not overlapping for it to count.", static_cast<unsigned long>(2))
    ("maximum-read-length", "Length of the longest read.")

    // Defaults should be moved to Settings constructor
    ("maximum-candidate-junctions",
     "Maximum number of candidate junction to create.", static_cast<uint32_t>(5000))
    ("minimum-candidate-junctions",
     "Minimum number of candidate junctions to create.", static_cast<uint32_t>(200))
    // This should be in the summary...
    ("reference-sequence-length",
     "Total length of reference sequences.")  
    ("maximum-candidate-junction-length-factor",
     "Total length of candidate junction sequences must be no more than this times reference sequence length.", static_cast<double>(0.1))    
	.processCommandArgs(argc, argv);
  
  //These options are almost always default values
  //TODO: Supply default values?

	// make sure that the config options are good:
	if(options.count("help")
		 || !options.count("data-path")
     || !options.count("candidate-junction-path")
     || !options.count("read-file")
     || !options.count("maximum-read-length")
     || !options.count("reference-sequence-length")
		 ) {
		options.printUsage();
		return -1;
	}                       
  
	try {
    
    // plain function

    Settings settings;
      
    // File settings
    settings.read_files.Init(from_string<vector<string> >(options["read-file"]), false);
    settings.preprocess_junction_split_sam_file_name = options["candidate-junction-path"] + "/#.split.sam";
    settings.reference_fasta_file_name = options["data-path"] + "/reference.fasta";     
    settings.candidate_junction_fasta_file_name = options["candidate-junction-path"] + "/candidate_junction.fasta";

    // Other settings
    settings.candidate_junction_read_limit = from_string<int32_t>(options["candidate-junction-read-limit"]);
    settings.require_match_length  = from_string<int32_t>(options["require-match-length"]);
    settings.required_one_unique_length_per_side = from_string<int32_t>(options["required-one-unique-length-per-side"]);
    settings.required_both_unique_length_per_side = from_string<int32_t>(options["required-both-unique-length-per-side"]);
    
    settings.maximum_candidate_junctions = from_string<int32_t>(options["maximum-candidate-junctions"]);
    settings.minimum_candidate_junctions = from_string<int32_t>(options["minimum-candidate-junctions"]);
    settings.maximum_candidate_junction_length_factor = from_string<double>(options["maximum-candidate-junction-length-factor"]);

    // We should inherit the summary object from earlier steps
    Summary summary;
    summary.sequence_conversion.total_reference_sequence_length = from_string<uint32_t>(options["reference-sequence-length"]);
    summary.sequence_conversion.max_read_length = from_string<int32_t>(options["maximum-read-length"]);

    cReferenceSequences ref_seq_info;
    ref_seq_info.ReadFASTA(options["data-path"] + "/reference.fasta");
        
    CandidateJunctions::identify_candidate_junctions(settings, summary, ref_seq_info);
    
  } catch(...) {
		// failed; 
		return -1;
	}
  
  return 0;
}








int do_simulate_read(int argc, char *argv[])
{
  AnyOption options("Usage: breseq SIMULATE-READ -g <genome diff> -r <reference file> -c <average coverage> -o <output file>");

  options
  ("genome_diff,g", "Genome diff file.")
  ("reference,r",   "Reference file for input.")
  ("coverage,c",    "Average coverage value to simulate.", static_cast<uint32_t>(80))
  ("length,l",      "Read length to simulate.", static_cast<uint32_t>(50))
  ("output,o",      "Output fastq file name.")
  ("gff3,3",        "Output Applied GFF3 File. (Flag)", TAKES_NO_ARGUMENT)
  ("verbose,v",     "Verbose Mode (Flag)", TAKES_NO_ARGUMENT)
  ("paired-ends",   "Will create two read files *_1.fastq, *_2.fastq that are mate-paired.(Flag)", TAKES_NO_ARGUMENT)
  ("mean",          "Mean fragment size to use for pair ended reads.", static_cast<uint32_t>(200))
  ("stdev",         "Standard deviation of fragment size to use for pair ended reads.", static_cast<uint32_t>(20))
  ("seed",          "Seed value to use for random number generation.")
  ;
  options.processCommandArgs(argc, argv);
  
  options.addUsage("");
  options.addUsage("Takes a genome diff file and applies the mutations to the reference.");
  options.addUsage("Then using the applied reference, it simulates reads based on it.");
  options.addUsage("Output is a .fastq that if run normally against the reference");
  options.addUsage("should produce the same GenomeDiff file you entered.");
  
  if(argc == 1)  {
    options.printUsage();
    return -1;  }
  
  if (!options.count("genome_diff")) {
    options.addUsage("");
    options.addUsage("You must supply the --genome_diff option for input.");
    options.printUsage();
    return -1;
  }
  
  if (!options.count("reference")) {
    options.addUsage("");
    options.addUsage("You must supply the --reference option for input.");
    options.printUsage();
    return -1;
  }
  
  if (!options.count("output")) {
    options.addUsage("");
    options.addUsage("You must supply the --output option for output.");
    options.printUsage();
    return -1;
  }

  if (options.count("seed")) {
    cSimFastqSequence::SEED_VALUE = from_string<int32_t>(options["seed"]);
  }

  const bool verbose = options.count("verbose");

//! Step: Load reference sequence file.
  cReferenceSequences ref_seq_info;
  cReferenceSequences new_ref_seq_info;
  const string &ref_file_name = options["reference"];
  ref_seq_info.LoadFile(ref_file_name);
  new_ref_seq_info.LoadFile(ref_file_name);


  //! Step: Apply genome diff mutations to reference sequence.
  const string &gd_file_name = options["genome_diff"];
  cGenomeDiff gd(gd_file_name);

  gd.apply_to_sequences(ref_seq_info, new_ref_seq_info, verbose);
  
  //! Write applied GFF3 file if requested.
  if(options.count("gff3"))new_ref_seq_info.WriteGFF(options["output"] + ".gff3", options.count("verbose"));


  const cAnnotatedSequence &sequence = new_ref_seq_info[0];
  uint32_t coverage = from_string<uint32_t>(options["coverage"]);
  uint32_t read_size = from_string<uint32_t>(options["length"]);

  uint32_t n_reads = (sequence.get_sequence_size() / read_size) * coverage;

  if (options.count("paired-ends")) {
    cString output = options["output"];

    string extension = output.get_file_extension();
    output.remove_ending(extension);

    string pair_1_file_name = output + "_1" + extension;
    string pair_2_file_name = output + "_2" + extension;

    uint32_t mean  = from_string<uint32_t>(options["mean"]);
    uint32_t stdev = from_string<uint32_t>(options["stdev"]);

    cSimFastqSequence::simulate_paired_ends(sequence,
                                            n_reads,
                                            read_size,
                                            mean,
                                            stdev,
                                            pair_1_file_name,
                                            pair_2_file_name,
                                            verbose);

  } else {
    cSimFastqSequence::simulate_single_ends(sequence,
                                            n_reads,
                                            read_size,
                                            options["output"],
                                            verbose);
  }

  return 0;
}




int do_copy_number_variation(int argc, char *argv[])
{
	Settings settings(argc, argv);
  
  //(re)load the reference sequences from our converted files
  cReferenceSequences ref_seq_info;
  ref_seq_info.ReadGFF(settings.reference_gff3_file_name);
  
  Summary summary;
  summary.unique_coverage.retrieve(settings.error_rates_summary_file_name);
    
  //create directory
  create_path( settings.copy_number_variation_path );
  
  for (cReferenceSequences::iterator it = ref_seq_info.begin(); it != ref_seq_info.end(); ++it)
  {
    cAnnotatedSequence& seq = *it;
    string this_complete_coverage_text_file_name = settings.file_name(settings.complete_coverage_text_file_name, "@", seq.m_seq_id);
    string this_tiled_complete_coverage_text_file_name = settings.file_name(settings.tiled_complete_coverage_text_file_name, "@", seq.m_seq_id);
    CoverageDistribution::tile(settings.ignore_redundant_coverage, this_complete_coverage_text_file_name, this_tiled_complete_coverage_text_file_name, settings.copy_number_variation_tile_size);
    
    string this_ranges_text_file_name = settings.file_name(settings.ranges_text_file_name, "@", seq.m_seq_id);
    string this_cnv_history_text_file_name = settings.file_name(settings.cnv_history_text_file_name, "@", seq.m_seq_id);
    CoverageDistribution::find_segments(settings,
                                        summary.unique_coverage[seq.m_seq_id].average,
                                        this_tiled_complete_coverage_text_file_name,
                                        this_ranges_text_file_name,
                                        this_cnv_history_text_file_name
                                        );
    
    string this_smoothed_ranges_text_file_name = settings.file_name(settings.smoothed_ranges_text_file_name, "@", seq.m_seq_id);
    string this_final_cnv_file_name = settings.file_name(settings.final_cnv_text_file_name, "@", seq.m_seq_id);
    string this_copy_number_variation_cn_genome_diff_file_name = settings.file_name(settings.copy_number_variation_cn_genome_diff_file_name, "@", seq.m_seq_id);
    CoverageDistribution::smooth_segments(settings,
                                          seq.m_seq_id,
                                          summary.unique_coverage[seq.m_seq_id].average, 
                                          this_tiled_complete_coverage_text_file_name, 
                                          this_ranges_text_file_name, 
                                          this_smoothed_ranges_text_file_name,
                                          this_final_cnv_file_name,
                                          this_copy_number_variation_cn_genome_diff_file_name
                                          );
    
   }
  
  return 0;
}

int do_periodicity(int argc, char *argv[])
{
	Settings settings(argc, argv);
  
  //(re)load the reference sequences from our converted files
  cReferenceSequences ref_seq_info;
  ref_seq_info.ReadGFF(settings.reference_gff3_file_name);
  
  Summary summary;
  summary.unique_coverage.retrieve(settings.error_rates_summary_file_name);
  
  CandidateJunctions::identify_candidate_junctions(settings, summary, ref_seq_info);
  
  //create directory
  create_path( settings.copy_number_variation_path );
  
  for (cReferenceSequences::iterator it = ref_seq_info.begin(); it != ref_seq_info.end(); ++it)
  {
    cAnnotatedSequence& seq = *it;
    string this_complete_coverage_text_file_name = settings.file_name(settings.complete_coverage_text_file_name, "@", seq.m_seq_id);
    string this_periodicity_complete_coverage_text_file_name = settings.file_name(settings.periodicity_table_file_name, "@", seq.m_seq_id);
    CoverageDistribution::calculate_periodicity(
          this_complete_coverage_text_file_name, 
          this_periodicity_complete_coverage_text_file_name,
          settings.periodicity_method,
          settings.periodicity_start,
          settings.periodicity_end,
          settings.periodicity_step
          );
    
   }
  
  return 0;
}



int do_get_sequence(int argc, char *argv[])
{
  AnyOption options("Usage: breseq GET-SEQUENCE -r <reference> -o <output.fasta> -p <REL606:50-100>");
  options("reference,r",".gbk/.gff3/.fasta reference sequence file", "data/reference.fasta");
  options("output,o","output FASTA file");  
  options("position,p","Sequence ID:Start-End");
  options("reverse-complement,c","Reverse Complement (Flag)", TAKES_NO_ARGUMENT);
  options("verbose,v","Verbose Mode (Flag)", TAKES_NO_ARGUMENT);
  options.processCommandArgs(argc, argv);
  
  options.addUsage("");
  options.addUsage("Takes a reference sequence and outputs to FASTA a subset");
  options.addUsage("of the sequence.  Using '0' as the End position will set");  
  options.addUsage("it to the length of the relevant sequence.");
  
  if(argc == 1)  {
    options.printUsage();
    return -1;  }
  
  if(!file_exists(options["reference"].c_str()))  {
    options.addUsage("");
    options.addUsage("You must supply a valid --reference option for input.");
    options.addUsage("Could not find:");
    options.addUsage(options["reference"].c_str());
    options.printUsage();
    return -1;    
  }
   
  vector<string> region_list;  
  
  bool do_reverse_complement = options.count("reverse-complement");
  bool verbose = options.count("verbose") || !options.count("output");
  
  if (options.count("position"))  {
    region_list = from_string<vector<string> >(options["position"]);  }
  
  // Also take positions off the command line
  for (int32_t i = 0; i < options.getArgc(); i++)
  {
    string position = options.getArgv(i);
    region_list.push_back(position);
  }
  
  if (!region_list.size()) {
    options.addUsage("");
    options.addUsage("You must supply the --position option for input.");
    options.printUsage();
    return -1;
  }
  
  cReferenceSequences ref_seq_info, new_seq_info;
  ref_seq_info.LoadFiles(from_string<vector<string> >(options["reference"]));
  
  for(uint32_t j = 0; j < region_list.size(); j++)
  {    
    uint32_t replace_target_id, replace_start, replace_end;
    string seq_name = "";
    
    ref_seq_info.parse_region(region_list[j], replace_target_id, replace_start, replace_end);
    
    if(!replace_end)  {
      replace_end = ref_seq_info[replace_target_id].m_length;  
    }
    
    if (replace_start > replace_end) {
      cout << "START greater than END." << " Creating reverse complement" << endl;
      swap(replace_start, replace_end);
      do_reverse_complement = true;
    }
    
    if(verbose)
    {
      cout << "Sequence ID:      " << ref_seq_info[replace_target_id].m_seq_id << endl;
      cout << "Start Position:   " << replace_start << endl;
      cout << "End Position:     " << replace_end << endl;
      cout << "Genome Strand:    " << (do_reverse_complement ? "Bottom" : "Top") << endl;
    }
    
    ASSERT((uint32_t)ref_seq_info[replace_target_id].m_length >= replace_start && (uint32_t)ref_seq_info[replace_target_id].m_length >= replace_end,
           "START:\t" + to_string(replace_start) + "\n" +
           "END:\t" + to_string(replace_end) + "\n" +
           "SIZE:\t" + to_string(ref_seq_info[replace_target_id].m_length) + "\n" +
           "Neither START or END can be greater than the SIZE of " + ref_seq_info[replace_target_id].m_seq_id + ".");
    
    seq_name = ref_seq_info[replace_target_id].m_seq_id + ":" + to_string(replace_start) + "-" + to_string(replace_end);
    
    new_seq_info.add_new_seq(seq_name);
    cAnnotatedSequence& new_seq = new_seq_info[seq_name];
    new_seq.m_fasta_sequence = ref_seq_info[replace_target_id].m_fasta_sequence;    
    new_seq.m_fasta_sequence.m_name = seq_name;
    new_seq.m_fasta_sequence.m_sequence = to_upper(ref_seq_info[replace_target_id].m_fasta_sequence.m_sequence.substr(replace_start -1, (replace_end - replace_start) + 1));
    if(do_reverse_complement) new_seq.m_fasta_sequence.m_sequence = reverse_complement(new_seq.m_fasta_sequence.m_sequence);
    new_seq.m_seq_id = seq_name;
    new_seq.m_length = new_seq.m_fasta_sequence.m_sequence.size();
    
    if(verbose)  {
      cout << new_seq.m_fasta_sequence << endl;  
    }
  }
  
  if(options.count("output"))  {
    new_seq_info.WriteFASTA(options["output"], verbose);  
  }  
  
  return 0;
}

int do_junction_polymorphism(int argc, char *argv[])
{
  AnyOption options("Usage: breseq JUNCTION-POLYMORPHISM -g input.gd [ -o output.gd ]");  
  options.addUsage("Counts the number of reads supporting a new junction versus supporting the reference junction.");
  options("genome-diff,g","Input Genome Diff file");  
  options("output,o","Output Genome Diff file", "output.gd");
  options("verbose,v","Verbose output", TAKES_NO_ARGUMENT);
  options.processCommandArgs(argc, argv);
  
  // Check options
  if ( !options.count("genome-diff") ) {
    options.printUsage();
    return -1;
  }

  Settings settings;
  settings.verbose = options.count("verbose");
  cGenomeDiff gd(options["genome-diff"]);
  assign_junction_read_counts(settings, gd);
  gd.write(options["output"]);
  return 0;
}

int do_ref_aln()
{
  SYSTEM("samtools view -bt data/reference.fasta.fai 02_reference_alignment/*.sam > data/ref_aln.bam");
  SYSTEM("samtools sort data/ref_aln.bam data/ref_aln.sorted");
  SYSTEM("samtools index data/ref_aln.sorted.bam");
  
  return 0;
}

int breseq_default_action(int argc, char* argv[])
{  
  
	///
	/// Get options from the command line
	///
  Summary summary;
	Settings settings(argc, argv);
	settings.check_installed();
  
	//
	// 01_sequence_conversion 
  // * Convert the input reference into FASTA for alignment and GFF3 for reloading features
  // * Rename reads in the input FASTQ and change quality scores to Sanger
	//
	create_path(settings.sequence_conversion_path);
  create_path(settings.data_path);

  cReferenceSequences ref_seq_info;

	if (settings.do_step(settings.sequence_conversion_done_file_name, "Read and reference sequence file input"))
	{
		Summary::SequenceConversion s;
    cReferenceSequences conv_ref_seq_info;
    
    // Load all of the reference sequences and convert to FASTA and GFF3
    conv_ref_seq_info.LoadFiles(settings.reference_file_names);
    conv_ref_seq_info.WriteFASTA(settings.reference_fasta_file_name);
    conv_ref_seq_info.WriteGFF(settings.reference_gff3_file_name);

    // No conversion if already is sam mode
    if (!settings.aligned_sam_mode) {
      
      //Check the FASTQ format and collect some information about the input read files at the same time
      uint32_t overall_max_read_length = UNDEFINED_UINT32;
      uint32_t overall_max_qual = 0;

      s.num_reads = 0;
      s.num_bases = 0;
      for (uint32_t i = 0; i < settings.read_files.size(); i++)
      {
        string base_name = settings.read_files[i].m_base_name;
        cerr << "  READ FILE::" << base_name << endl;
        string fastq_file_name = settings.base_name_to_read_file_name(base_name);
        string convert_file_name =  settings.file_name(settings.converted_fastq_file_name, "#", base_name);

        // Parse output
        Summary::AnalyzeFastq s_rf = normalize_fastq(fastq_file_name, convert_file_name, i+1, settings.quality_score_trim, !settings.no_read_filtering);
        
        // Save the converted file name -- have to save it in summary because only that
        // is reloaded if we skip this step.
        s.converted_fastq_name[base_name] = s_rf.converted_fastq_name;

        // Record statistics
        if ((overall_max_read_length == UNDEFINED_UINT32) || (s_rf.max_read_length > overall_max_read_length))
          overall_max_read_length = s_rf.max_read_length;
        if ((overall_max_qual == UNDEFINED_UINT32) || (s_rf.max_quality_score > overall_max_qual))
          overall_max_qual = s_rf.max_quality_score;
        s.num_reads += s_rf.num_reads;
        s.original_num_reads += s_rf.original_reads;
        s.num_bases += s_rf.num_bases;
        s.original_num_bases += s_rf.original_num_bases;        

        s.reads[base_name] = s_rf;
      }
      s.avg_read_length = static_cast<double>(s.num_bases) / static_cast<double>(s.num_reads);
      s.max_read_length = overall_max_read_length;
      s.max_qual = overall_max_qual;
      summary.sequence_conversion = s;
      
      // Print totals
      cerr << "  ::TOTAL::" << endl;
      cerr << "    Original reads: " << s.original_num_reads << " bases: " << s.original_num_bases << endl;
      cerr << "    Analyzed reads: " << s.num_reads << " bases: " << s.num_bases << endl;

    }
    

      
		// create SAM faidx
		string samtools = settings.ctool("samtools");
		string command = samtools + " faidx " + settings.reference_fasta_file_name;
		SYSTEM(command);
    
		// calculate trim files
		calculate_trims(settings.reference_fasta_file_name, settings.sequence_conversion_path);

		// store summary information
		summary.sequence_conversion.store(settings.sequence_conversion_summary_file_name);
		settings.done_step(settings.sequence_conversion_done_file_name);
	}

	summary.sequence_conversion.retrieve(settings.sequence_conversion_summary_file_name);
	ASSERT(summary.sequence_conversion.max_read_length != UNDEFINED_UINT32, "Can't retrieve max read length from file: " + settings.sequence_conversion_summary_file_name);

  //(re)load the reference sequences from our converted files
  ref_seq_info.ReadGFF(settings.reference_gff3_file_name);
  
  // Calculate the total reference sequence length
  summary.sequence_conversion.total_reference_sequence_length = ref_seq_info.total_length();
  
  // @JEB -- This is a bit of an ugly wart from when converting the input file was optional.
	// reload certain information into $settings from $summary  
	for (map<string, Summary::AnalyzeFastq>::iterator it = summary.sequence_conversion.reads.begin(); it != summary.sequence_conversion.reads.end(); it++)
	{
		string read_file = it->first;
		if (it->second.converted_fastq_name.size() > 0)
			settings.read_files.read_file_to_converted_fastq_file_name_map[read_file] = it->second.converted_fastq_name;
	}

	//
  // 02_reference_alignment
	// * Match all reads against the reference genome
	//

	if (
      settings.do_step(settings.reference_alignment_done_file_name, "Read alignment to reference genome")
      && !settings.aligned_sam_mode
      )
	{
		create_path(settings.reference_alignment_path);

    // Staged alignment with Bowtie
    //
    if (settings.bowtie2) {
      string reference_hash_file_name = settings.reference_hash_file_name;
      string reference_fasta_file_name = settings.reference_fasta_file_name;
      string command = "bowtie2-build -q " + settings.reference_fasta_file_name + " " + reference_hash_file_name;
      SYSTEM(command);
      
		  for (uint32_t i = 0; i < settings.read_files.size(); i++)
		  {
		  	cReadFile read_file = settings.read_files[i];
        string base_read_file_name = read_file.base_name();
				string read_fastq_file = settings.base_name_to_read_file_name(base_read_file_name);
                
        //Paths
        string stage1_reference_sam_file_name = settings.file_name(settings.stage1_reference_sam_file_name, "#", base_read_file_name);
        string stage1_unmatched_fastq_file_name = settings.file_name(settings.stage1_unmatched_fastq_file_name, "#", base_read_file_name);
        
        //Split alignment into unmatched and matched files.
        string command = "bowtie2 -t -p " + s(settings.num_processors) + " --local -a " + settings.bowtie2_score_parameters + " " + settings.bowtie2_min_score_stringent + " --reorder -x " + reference_hash_file_name + " -U " + read_fastq_file + " -S " + stage1_reference_sam_file_name + " --un " + stage1_unmatched_fastq_file_name; 
        SYSTEM(command);
      }
    }

		/// create ssaha2 hash
		string reference_hash_file_name = settings.reference_hash_file_name;
		string reference_fasta_file_name = settings.reference_fasta_file_name;

    if (settings.bowtie2_align) {
      string command = "bowtie2-build -q " + reference_fasta_file_name + " " + reference_hash_file_name;
			SYSTEM(command);
    } else {
			string command = "ssaha2Build -kmer " + to_string(settings.ssaha2_seed_length) +  " -skip " + to_string(settings.ssaha2_skip_length) + " -save " + reference_hash_file_name + " " + reference_fasta_file_name;
			SYSTEM(command);
		}
  
 
		/// align reads to reference sequences
		for (uint32_t i = 0; i < settings.read_files.size(); i++)
		{
			cReadFile read_file = settings.read_files[i];


			//reads are paired -- Needs to be re-implemented with major changes elsewhere. @JEB
			//if (is_defined(read_struct.min_pair_dist) && is_defined(read_struct.max_pair_dist))
			//if (is_defined(read_file.m_paired_end_group))
			//{
				// JEB this is not working currently
			//	breseq_assert(false);
				/*
				die "Paired end mapping is broken.";
				die if (scalar @{$read_struct->{read_fastq_list}} != 2);

				my $fastq_1 = $read_struct->{read_fastq_list}->[0];
				my $fastq_2 = $read_struct->{read_fastq_list}->[1];
				my $min = $read_struct->{min_pair_dist};
				my $max = $read_struct->{max_pair_dist};

				my $reference_sam_file_name = $settings->file_name("reference_sam_file_name", {"//"=>$read_struct->{base_name}});
				Breseq::Shared::system("ssaha2 -disk 2 -save $reference_hash_file_name -kmer 13 -skip 1 -seeds 1 -score 12 -cmatch 9 -ckmer 1 -output sam_soft -outfile $reference_sam_file_name -multi 1 -mthresh 9 -pair $min,$max $fastq_1 $fastq_2");
				*/
			//}
			//else //reads are not paired
    
			{
				string base_read_file_name = read_file.base_name();
				string read_fastq_file = settings.base_name_to_read_file_name(base_read_file_name);
				string reference_sam_file_name = settings.file_name(settings.reference_sam_file_name, "#", base_read_file_name);

        
        // If we are doing staged alignment -- only align the unmatched reads with SSAHA2 and save to different name initially
        if (settings.bowtie2) {
          read_fastq_file = settings.file_name(settings.stage1_unmatched_fastq_file_name, "#", base_read_file_name);
          reference_sam_file_name = settings.file_name(settings.stage2_reference_sam_file_name, "#", base_read_file_name);
        }

        if (settings.bowtie2_align) {
          
          // This setting prevents indels > 1 bp from being predicted entirely.
          string command = "bowtie2 -t -p " + s(settings.num_processors) + " --local -a " + settings.bowtie2_score_parameters + " " + settings.bowtie2_min_score_relaxed + " --no-unal --reorder -x " + reference_hash_file_name + " -U " + read_fastq_file + " -S " + reference_sam_file_name; 
         
          SYSTEM(command);
        } else {
					string command = "ssaha2 -disk 2 -save " + reference_hash_file_name + " -kmer " + to_string(settings.ssaha2_seed_length) + " -skip " + to_string(settings.ssaha2_skip_length) + " -seeds 1 -score 12 -cmatch " + to_string(settings.ssaha2_seed_length) + " -ckmer 1 -output sam_soft -outfile " + reference_sam_file_name + " " + read_fastq_file;
					SYSTEM(command);
          
          //Check for SSAHA2 32-bit File Memory Error.
          uint32_t bytes = ifstream(reference_sam_file_name.c_str()).rdbuf()->in_avail();
          CHECK(bytes != 2147483647, "Encountered SSAHA2 32 bit version file memory limit.");
				}
        
        if (settings.bowtie2) {
          
          string base_read_file_name = read_file.base_name();
          string read_fastq_file = settings.base_name_to_read_file_name(base_read_file_name);

          string stage1_reference_sam_file_name = settings.file_name(settings.stage1_reference_sam_file_name, "#", base_read_file_name);
          string stage2_reference_sam_file_name = settings.file_name(settings.stage2_reference_sam_file_name, "#", base_read_file_name);
          string reference_sam_file_name = settings.file_name(settings.reference_sam_file_name, "#", base_read_file_name);
          string reference_fasta_file_name = settings.file_name(settings.reference_fasta_file_name, "#", base_read_file_name);
          
          PreprocessAlignments::merge_sort_sam_files(
                                                     stage1_reference_sam_file_name,
                                                     stage2_reference_sam_file_name,
                                                     reference_sam_file_name
                                                     );
        }
			}
		}

    
		/// Delete the hash files immediately
		if (!settings.keep_all_intermediates)
		{
			remove( (reference_hash_file_name + ".base").c_str() );
			remove( (reference_hash_file_name + ".body").c_str() );
			remove( (reference_hash_file_name + ".head").c_str() );
			remove( (reference_hash_file_name + ".name").c_str() );
			remove( (reference_hash_file_name + ".size").c_str() );
		}

		settings.done_step(settings.reference_alignment_done_file_name);
	}

  //
	// 03_candidate_junctions
	// * Identify candidate junctions from split read alignments
	//
	if (
      !settings.no_junction_prediction
      && !settings.aligned_sam_mode
      )
	{
		create_path(settings.candidate_junction_path);

    string preprocess_junction_done_file_name = settings.preprocess_junction_done_file_name;

    if (settings.do_step(settings.preprocess_junction_done_file_name, "Preprocessing alignments for candidate junction identification"))
    {
      PreprocessAlignments::preprocess_alignments(settings, summary, ref_seq_info);
      settings.done_step(settings.preprocess_junction_done_file_name);
    }

    
    if (settings.do_step(settings.coverage_junction_done_file_name, "Preliminary analysis of coverage distribution"))
    {
      string reference_faidx_file_name = settings.reference_faidx_file_name;
      string preprocess_junction_best_sam_file_name = settings.preprocess_junction_best_sam_file_name;
      string coverage_junction_best_bam_file_name = settings.coverage_junction_best_bam_file_name;
      string coverage_junction_best_bam_prefix = settings.coverage_junction_best_bam_prefix;
      string coverage_junction_best_bam_unsorted_file_name = settings.coverage_junction_best_bam_unsorted_file_name;

      string samtools = settings.ctool("samtools");

      string command = samtools + " import " + reference_faidx_file_name + " " + preprocess_junction_best_sam_file_name + " " + coverage_junction_best_bam_unsorted_file_name;
      SYSTEM(command);
      command = samtools + " sort " + coverage_junction_best_bam_unsorted_file_name + " " + coverage_junction_best_bam_prefix;
      SYSTEM(command);
      if (!settings.keep_all_intermediates)
        remove(coverage_junction_best_bam_unsorted_file_name.c_str());
      command = samtools + " index " + coverage_junction_best_bam_file_name;
      SYSTEM(command);

      // Count errors
      string reference_fasta_file_name = settings.reference_fasta_file_name;
      string reference_bam_file_name = settings.coverage_junction_best_bam_file_name;

      error_count(
        settings,
        summary,
        reference_bam_file_name,
        reference_fasta_file_name,
        settings.candidate_junction_path,
        settings.read_file_names,
        true, // coverage
        false, // errors
        true, //preprocess
        settings.base_quality_cutoff,
        "" //covariates
      );

      CoverageDistribution::analyze_unique_coverage_distributions(settings, 
                                                                  summary, 
                                                                  ref_seq_info,
                                                                  settings.coverage_junction_plot_file_name, 
                                                                  settings.coverage_junction_distribution_file_name
                                                                  );

      // Note that storing from unique_coverage and reloading in preprocess_coverage is by design
      summary.unique_coverage.store(settings.coverage_junction_summary_file_name);
      summary.preprocess_error_count.store(settings.coverage_junction_error_count_summary_file_name);
      settings.done_step(settings.coverage_junction_done_file_name);
		}
    summary.preprocess_coverage.retrieve(settings.coverage_junction_summary_file_name);
    summary.preprocess_error_count.retrieve(settings.coverage_junction_error_count_summary_file_name);
    
		string candidate_junction_summary_file_name = settings.candidate_junction_summary_file_name;
		if (settings.do_step(settings.candidate_junction_done_file_name, "Identifying junction candidates"))
		{
      CandidateJunctions::identify_candidate_junctions(settings, summary, ref_seq_info);

			string samtools = settings.ctool("samtools");
			string faidx_command = samtools + " faidx " + settings.candidate_junction_fasta_file_name;
			if (!file_empty(settings.candidate_junction_fasta_file_name.c_str()))
				SYSTEM(faidx_command);

			summary.candidate_junction.store(candidate_junction_summary_file_name);
			settings.done_step(settings.candidate_junction_done_file_name);
		}
		summary.candidate_junction.retrieve(candidate_junction_summary_file_name);

    
    //
    // 04 candidate_junction_alignment
    // * Align reads to new junction candidates
    //
		if (
        settings.do_step(settings.candidate_junction_alignment_done_file_name, "Re-alignment to junction candidates")
        && !settings.aligned_sam_mode
        )
		{
			create_path(settings.candidate_junction_alignment_path);

			/// create ssaha2 hash
			string candidate_junction_hash_file_name = settings.candidate_junction_hash_file_name;
			string candidate_junction_fasta_file_name = settings.candidate_junction_fasta_file_name;

			if (!file_empty(candidate_junction_fasta_file_name.c_str()))
			{
        if (settings.bowtie2_align) {
          string command = "bowtie2-build -q " + candidate_junction_fasta_file_name + " " + candidate_junction_hash_file_name;
          SYSTEM(command);
        }
        else
				{
					string command = "ssaha2Build -kmer " + to_string(settings.ssaha2_seed_length) + " -skip " + to_string(settings.ssaha2_skip_length) + " -save " + candidate_junction_hash_file_name + " " + candidate_junction_fasta_file_name;
					SYSTEM(command);
				}
			}

			/// ssaha2 align reads to candidate junction sequences
			for (uint32_t i = 0; i < settings.read_files.size(); i++)
			{
				string base_read_file_name = settings.read_files[i].m_base_name;
				string candidate_junction_sam_file_name = settings.file_name(settings.candidate_junction_sam_file_name, "#", base_read_file_name);

				string read_fastq_file = settings.base_name_to_read_file_name(base_read_file_name);
        if (settings.bowtie2_align) 
        {
          string filename = candidate_junction_hash_file_name + ".1.bt2";
          if (file_exists(filename.c_str())) 
          {
            string command = "bowtie2 -t -p " + s(settings.num_processors) + " --local -a " + settings.bowtie2_score_parameters + " " + settings.bowtie2_min_score_stringent + " --no-unal --reorder -x " + candidate_junction_hash_file_name + " -U " + read_fastq_file + " -S " + candidate_junction_sam_file_name; 
            SYSTEM(command);
          }
        }
        else
        {
          string filename = candidate_junction_hash_file_name + ".base";
          if (file_exists(filename.c_str())) 
          {
            //string command = "ssaha2 -disk 2 -save " + candidate_junction_hash_file_name + " -best 1 -kmer " + to_string(settings.ssaha2_seed_length) + " -skip " + to_string(settings.ssaha2_skip_length) + " -seeds 1 -score 12 -cmatch " + to_string(settings.ssaha2_seed_length) + " -ckmer 1 -output sam_soft -outfile " + candidate_junction_sam_file_name + " " + read_fastq_file;
            string command = "ssaha2 -disk 2 -save " + candidate_junction_hash_file_name + " -kmer " + to_string(settings.ssaha2_seed_length) + " -skip " + to_string(settings.ssaha2_skip_length) + " -seeds 1 -score 12 -cmatch " + to_string(settings.ssaha2_seed_length) + " -ckmer 1 -output sam_soft -outfile " + candidate_junction_sam_file_name + " " + read_fastq_file;
            SYSTEM(command);
            // Note: Added -best parameter to try to avoid too many matches to redundant junctions!
          }
        }
			}

			/// Delete the hash files immediately
			if (!settings.keep_all_intermediates)
			{
				remove((candidate_junction_hash_file_name + ".base").c_str());
				remove((candidate_junction_hash_file_name + ".body").c_str());
				remove((candidate_junction_hash_file_name + ".head").c_str());
				remove((candidate_junction_hash_file_name + ".name").c_str());
				remove((candidate_junction_hash_file_name + ".size").c_str());
			}

			settings.done_step(settings.candidate_junction_alignment_done_file_name);
		}
  }

	
	//
  // 05 alignment_correction
	// * Resolve matches to new junction candidates
	//
	if (settings.do_step(settings.alignment_correction_done_file_name, "Resolving alignments with junction candidates"))
	{
		create_path(settings.alignment_resolution_path);

    bool junction_prediction = !settings.no_junction_prediction;
    if (junction_prediction && file_empty(settings.candidate_junction_fasta_file_name.c_str())) junction_prediction = false;
    
		resolve_alignments(
			settings,
			summary,
			ref_seq_info,
      junction_prediction,
			settings.read_files
		);

		summary.alignment_resolution.store(settings.alignment_resolution_summary_file_name);
		settings.done_step(settings.alignment_correction_done_file_name);
	}
  
	if (file_exists(settings.alignment_resolution_summary_file_name.c_str()))
    summary.alignment_resolution.retrieve(settings.alignment_resolution_summary_file_name);
  
	//
  // 05 bam
	// * Create BAM read alignment database files
	//

	if (settings.do_step(settings.bam_done_file_name, "Creating BAM files"))
	{
		create_path(settings.bam_path);

		string reference_faidx_file_name = settings.reference_faidx_file_name;
		string candidate_junction_faidx_file_name = settings.candidate_junction_faidx_file_name;

		string resolved_junction_sam_file_name = settings.resolved_junction_sam_file_name;
		string junction_bam_unsorted_file_name = settings.junction_bam_unsorted_file_name;
		string junction_bam_prefix = settings.junction_bam_prefix;
		string junction_bam_file_name = settings.junction_bam_file_name;

		string samtools = settings.ctool("samtools");
		string command;

    // only run samtools if we are predicting junctions and there were results in the sam file
    // first part of conditional really not necessary @JEB
		if (!file_empty(resolved_junction_sam_file_name.c_str()))
		{
			command = samtools + " import " + candidate_junction_faidx_file_name + " " + resolved_junction_sam_file_name + " " + junction_bam_unsorted_file_name;
			SYSTEM(command);
			command = samtools + " sort " + junction_bam_unsorted_file_name + " " + junction_bam_prefix;
      SYSTEM(command);
			if (!settings.keep_all_intermediates)
				remove(junction_bam_unsorted_file_name.c_str());
			command = samtools + " index " + junction_bam_file_name;
			SYSTEM(command);
		}

		string resolved_reference_sam_file_name = settings.resolved_reference_sam_file_name;
		string reference_bam_unsorted_file_name = settings.reference_bam_unsorted_file_name;
		string reference_bam_prefix = settings.reference_bam_prefix;
		string reference_bam_file_name = settings.reference_bam_file_name;

		command = samtools + " import " + reference_faidx_file_name + " " + resolved_reference_sam_file_name + " " + reference_bam_unsorted_file_name;
    SYSTEM(command);
		command = samtools + " sort " + reference_bam_unsorted_file_name + " " + reference_bam_prefix;
    SYSTEM(command);
		if (!settings.keep_all_intermediates)
			remove(reference_bam_unsorted_file_name.c_str());
		command = samtools + " index " + reference_bam_file_name;
    SYSTEM(command);

		settings.done_step(settings.bam_done_file_name);
	}

	//
	//#  Graph paired read outliers (experimental)
	//# sub paired_read_distances {}
	//#
	//# {
	//# 	my @rs = settings.read_structures;
	//#
	//# 	my @min_pair_dist;
	//# 	my @max_pair_dist;
	//#
	//# 	my $paired = 0;
	//#
	//# 	my $i=0;
	//# 	foreach my $rfi (@{settings.{read_file_index_to_struct_index}})
	//# 	{
	//# 		$min_pair_dist[$i] = 0;
	//# 		$max_pair_dist[$i] = 0;
	//#
	//# 		if ($rs[$rfi]->{paired})
	//# 		{
	//# 			$paired = 1;
	//# 			$min_pair_dist[$i] = $rs[$rfi]->{min_pair_dist};
	//# 			$max_pair_dist[$i] = $rs[$rfi]->{max_pair_dist};
	//# 		}
	//# 		$i++;
	//# 	}
	//#
	//# 	my $long_pairs_file_name = settings.file_name("long_pairs_file_name");
	//#
	//# 	if ($paired && (!-e $long_pairs_file_name))
	//# 	{
	//#
	//# 		my $reference_sam_file_name = settings.file_name("resolved_reference_sam_file_name");
	//# 		my $reference_tam = Bio::DB::Tam->open($reference_sam_file_name) or die "Could not open $reference_sam_file_name";
	//#
	//# 		my $reference_faidx_file_name = settings.file_name("reference_faidx_file_name");
	//# 		my $reference_header = $reference_tam->header_read2($reference_faidx_file_name) or throw("Error reading reference fasta index file: $reference_faidx_file_name");
	//# 		my $target_names = $reference_header->target_name;
	//#
	//# 		my $save;
	//# 		my $on_alignment = 0;
	//# 		my $last;
	//#
	//# 		while (1)
	//# 		{
	//# 			$a = Bio::DB::Bam::Alignment->new();
	//# 			my $bytes = $reference_tam->read1($reference_header, $a);
	//# 			last if ($bytes <= 0);
	//#
	//#
	//# 			my $start       = $a->start;
	//# 		    my $end         = $a->end;
	//# 		    my $seqid       = $target_names->[$a->tid];
	//#
	//# 			$on_alignment++;
	//# 			print "$on_alignment\n" if ($on_alignment % 10000 == 0);
	//#
	//# 			//#last if ($on_alignment > 100000);
	//#
	//# 			//#print $a->qname . "" << endl;
	//#
	//# 			if (!$a->unmapped)
	//# 			{
	//# 				my $mate_insert_size = abs($a->isize);
	//# 				my $mate_end = $a->mate_end;
	//# 				my $mate_start = $a->mate_start;
	//# 				my $mate_reversed = 2*$a->mreversed + $a->reversed;
	//# 		 		my $mreversed = $a->mreversed;
	//# 		 		my $reversed = $a->reversed;
	//#
	//# 				my $fastq_file_index = $a->aux_get("X2");
	//# 				//#print "$mate_insert_size $min_pair_dist[$fastq_file_index] $max_pair_dist[$fastq_file_index]" << endl;
	//# 				//#if (($mate_insert_size < $min_pair_dist[$fastq_file_index]) || ($mate_insert_size > $max_pair_dist[$fastq_file_index]))
	//# 				if ((($mate_insert_size >= 400) && ($mate_insert_size < $min_pair_dist[$fastq_file_index])) || ($mate_insert_size > $max_pair_dist[$fastq_file_index]))
	//# 				{
	//# 					//#correct pair
	//#
	//# 					if ($last && ($last->{start} == $mate_start))
	//# 					{
	//# 						$save->{int($start/100)}->{int($mate_start/100)}->{$mate_reversed}++;
	//# 						$save->{int($last->{start}/100)}->{int($last->{mate_start}/100)}->{$last->{mate_reversed}}++;
	//# 						undef $last;
	//# 					}
	//# 					else
	//# 					{
	//# 						($last->{mate_reversed}, $last->{start}, $last->{mate_start}) = ($mate_reversed, $start, $mate_start);
	//# 					}
	//#
	//# 					//#$save->{$mate_reversed}->{int($start/100)}->{int($mate_start/100)}++;
	//# 				    //print $a->qname," aligns to $seqid:$start..$end, $mate_start $mate_reversed ($mreversed $reversed) $mate_insert_size" << endl;
	//# 				}
	//#
	//# 			}
	//# 		}
	//#
	//# 		open LP, ">$long_pairs_file_name" or die;
	//#
	//# 		foreach my $key_1 (sort {$a <=> $b} keys %$save)
	//# 		{
	//# 			foreach my $key_2 (sort {$a <=> $b} keys %{$save->{$key_1}})
	//# 			{
	//# 				foreach my $key_reversed (sort {$a <=> $b} keys %{$save->{$key_1}->{$key_2}})
	//# 				{
	//# 					print LP "$key_1\t$key_2\t$key_reversed\t$save->{$key_1}->{$key_2}->{$key_reversed}" << endl;
	//# 				}
	//# 			}
	//# 		}
	//# 		close LP;
	//# 	}
	//#
	//# 	if ($paired)
	//# 	{
	//# 		open LP, "$long_pairs_file_name" or die;
	//# 		while ($_ = <LP>)
	//# 		{
	//# 			chomp $_;
	//# 			my ($start, $end, $key_reversed);
	//# 		}
	//# 	}
	//# }
	//#

	//
	// Tabulate error counts and coverage distribution at unique only sites
	//

	if (settings.do_step(settings.error_counts_done_file_name, "Tabulating error counts"))
	{
		create_path(settings.error_calibration_path);

		string reference_fasta_file_name = settings.reference_fasta_file_name;
		string reference_bam_file_name = settings.reference_bam_file_name;

		//my $cbreseq = settings.ctool("cbreseq");

		// deal with distribution or error count keys being undefined...
		string coverage_fn = settings.file_name(settings.unique_only_coverage_distribution_file_name, "@", "");
		string outputdir = dirname(coverage_fn) + "/";

		uint32_t num_read_files = settings.read_files.size();
    uint32_t num_qual;
    if (!settings.aligned_sam_mode) {
      num_qual = summary.sequence_conversion.max_qual + 1; // only filled in when using FASTQ input
    } else {
      num_qual = summary.alignment_resolution.max_sam_base_quality_score + 1; // only filled in when using aligned_sam_mode
    }

		error_count(
      settings,
      summary,
			reference_bam_file_name, // bam
			reference_fasta_file_name, // fasta
			settings.error_calibration_path, // output
			settings.read_files.base_names(), // readfile
			true, // coverage
			true, // errors
      false, //preprocess
			settings.base_quality_cutoff, // minimum quality score
			"read_set=" + to_string(num_read_files) + ",obs_base,ref_base,quality=" + to_string(num_qual) // covariates
		);

		settings.done_step(settings.error_counts_done_file_name);
	}


	//
	// Calculate error rates
	//

	create_path(settings.output_path); //need output for plots
  create_path(settings.output_calibration_path);
  
	if (settings.do_step(settings.error_rates_done_file_name, "Re-calibrating base error rates"))
	{
		if (!settings.no_deletion_prediction)
			CoverageDistribution::analyze_unique_coverage_distributions(
                                                                  settings, 
                                                                  summary, 
                                                                  ref_seq_info,
                                                                  settings.unique_only_coverage_plot_file_name, 
                                                                  settings.unique_only_coverage_distribution_file_name
                                                                  );

    //Coverage distribution user option --deletion-coverage-propagation-cutoff
    if (settings.deletion_coverage_propagation_cutoff) {
      if (settings.deletion_coverage_propagation_cutoff < 1) {
        for (uint32_t i = 0; i < ref_seq_info.size(); i++) {
          string seq_id = ref_seq_info[i].m_seq_id;
          double average = summary.unique_coverage[seq_id].average;
          double &deletion_coverage_propagation_cutoff = summary.unique_coverage[seq_id].deletion_coverage_propagation_cutoff;

          deletion_coverage_propagation_cutoff = average * settings.deletion_coverage_propagation_cutoff;
        }
      } else if (settings.deletion_coverage_propagation_cutoff >= 1) {
        for (uint32_t i = 0; i < ref_seq_info.size(); i++) {
          string seq_id = ref_seq_info[i].m_seq_id;
          double &deletion_coverage_propagation_cutoff = summary.unique_coverage[seq_id].deletion_coverage_propagation_cutoff;

          deletion_coverage_propagation_cutoff = settings.deletion_coverage_propagation_cutoff;
        }
      }
    }

    //Coverage distribution user option --deletion-coverage-seed-cutoff
    if (settings.deletion_coverage_seed_cutoff) {
      if (settings.deletion_coverage_seed_cutoff < 1) {
        for (uint32_t i = 0; i < ref_seq_info.size(); i++) {
          string seq_id = ref_seq_info[i].m_seq_id;
          double average = summary.unique_coverage[seq_id].average;
          double &deletion_coverage_seed_cutoff = summary.unique_coverage[seq_id].deletion_coverage_seed_cutoff;

          deletion_coverage_seed_cutoff = average * settings.deletion_coverage_seed_cutoff;
        }
    } else if (settings.deletion_coverage_seed_cutoff >= 1) {
        for (uint32_t i = 0; i < ref_seq_info.size(); i++) {
          string seq_id = ref_seq_info[i].m_seq_id;
          double &deletion_coverage_seed_cutoff = summary.unique_coverage[seq_id].deletion_coverage_seed_cutoff;

          deletion_coverage_seed_cutoff = settings.deletion_coverage_seed_cutoff;
        }
      }
    }
		string command;
		for (uint32_t i = 0; i<settings.read_files.size(); i++) {
			string base_name = settings.read_files[i].base_name();
			string error_rates_base_qual_error_prob_file_name = settings.file_name(settings.error_rates_base_qual_error_prob_file_name, "#", base_name);
			string plot_error_rates_r_script_file_name = settings.plot_error_rates_r_script_file_name;
			string plot_error_rates_r_script_log_file_name = settings.file_name(settings.plot_error_rates_r_script_log_file_name, "#", base_name);
			string error_rates_plot_file_name = settings.file_name(settings.error_rates_plot_file_name, "#", base_name);
			command = "R --vanilla in_file=" + cString(error_rates_base_qual_error_prob_file_name).escape_shell_chars() +
        " out_file=" + cString(error_rates_plot_file_name).escape_shell_chars() +
        " < "        + cString(plot_error_rates_r_script_file_name).escape_shell_chars() +
        " > "        + cString(plot_error_rates_r_script_log_file_name).escape_shell_chars();
			SYSTEM(command,false, false, false); //NOTE: Not escaping shell characters here.
		}

		summary.unique_coverage.store(settings.error_rates_summary_file_name);
		settings.done_step(settings.error_rates_done_file_name);
	}
	summary.unique_coverage.retrieve(settings.error_rates_summary_file_name);

  //
	// 08 Mutation Identification
	// Make predictions of point mutations, small indels, and large deletions
	//

	if (!settings.no_mutation_prediction)
	{
		create_path(settings.mutation_identification_path);

		if (settings.do_step(settings.mutation_identification_done_file_name, "Examining read alignment evidence"))
		{
			string reference_fasta_file_name = settings.reference_fasta_file_name;
			string reference_bam_file_name = settings.reference_bam_file_name;

			string output_dir = settings.mutation_identification_path;
			string ra_mc_genome_diff_file_name = settings.ra_mc_genome_diff_file_name;

			// It is important that these are in consistent order with the fasta file!!
      vector<double> deletion_propagation_cutoffs;
      vector<double> deletion_seed_cutoffs;
      for (uint32_t i = 0; i < ref_seq_info.size(); i++) {
        deletion_propagation_cutoffs.push_back(summary.unique_coverage[ref_seq_info[i].m_seq_id].deletion_coverage_propagation_cutoff);
        deletion_seed_cutoffs.push_back(summary.unique_coverage[ref_seq_info[i].m_seq_id].deletion_coverage_seed_cutoff);
        //cout << ref_seq_info[i].m_seq_id << " " << to_string(deletion_propagation_cutoffs.back()) << " " << to_string(deletion_seed_cutoffs.back()) << endl;
      }

			identify_mutations(
        settings,
        summary,
				reference_bam_file_name,
				reference_fasta_file_name,
				ra_mc_genome_diff_file_name,
        deletion_propagation_cutoffs,
        deletion_seed_cutoffs,
				settings.mutation_log10_e_value_cutoff, // mutation_cutoff
				settings.polymorphism_log10_e_value_cutoff, // polymorphism_cutoff
				settings.polymorphism_frequency_cutoff, //polymorphism_frequency_cutoff
				settings.print_mutation_identification_per_position_file //per_position_file
			);

			settings.done_step(settings.mutation_identification_done_file_name);
		}

    // extra processing for polymorphisms
		if (settings.polymorphism_prediction && settings.do_step(settings.polymorphism_statistics_done_file_name, "Polymorphism statistics"))
		{
			ref_seq_info.polymorphism_statistics(settings, summary);
			settings.done_step(settings.polymorphism_statistics_done_file_name);
		}
	}

	//rewire which GenomeDiff we get data from if we have the elaborated polymorphism_statistics version
	 if (settings.polymorphism_prediction)
		settings.ra_mc_genome_diff_file_name = settings.polymorphism_statistics_ra_mc_genome_diff_file_name;
  
  //
	// 09 Copy number variation
	//
  
  if (settings.do_copy_number_variation) {
    create_path( settings.copy_number_variation_path );

    if (settings.do_step(settings.copy_number_variation_done_file_name, "Predicting copy number variation")) { 
      for (cReferenceSequences::iterator it = ref_seq_info.begin(); it != ref_seq_info.end(); ++it) {
        cAnnotatedSequence& seq = *it;
        string this_complete_coverage_text_file_name = settings.file_name(settings.complete_coverage_text_file_name, "@", seq.m_seq_id);
        string this_tiled_complete_coverage_text_file_name = settings.file_name(settings.tiled_complete_coverage_text_file_name, "@", seq.m_seq_id);

        cerr << "Tiling coverage" << endl;
        CoverageDistribution::tile(settings.ignore_redundant_coverage, this_complete_coverage_text_file_name, this_tiled_complete_coverage_text_file_name, settings.copy_number_variation_tile_size);
       
        string this_ranges_text_file_name = settings.file_name(settings.ranges_text_file_name, "@", seq.m_seq_id);
        string this_cnv_history_text_file_name = settings.file_name(settings.cnv_history_text_file_name, "@", seq.m_seq_id);
        cerr << "Predicting changed segments" << endl;
        CoverageDistribution::find_segments(settings,
                                            summary.unique_coverage[seq.m_seq_id].average,
                                            this_tiled_complete_coverage_text_file_name,
                                            this_ranges_text_file_name, 
                                            this_cnv_history_text_file_name
                                            );
       
        string this_smoothed_ranges_text_file_name = settings.file_name(settings.smoothed_ranges_text_file_name, "@", seq.m_seq_id);
        string this_final_cnv_file_name = settings.file_name(settings.final_cnv_text_file_name, "@", seq.m_seq_id);
        string this_copy_number_variation_cn_genome_diff_file_name = settings.file_name(settings.copy_number_variation_cn_genome_diff_file_name, "@", seq.m_seq_id);

        cerr << "Smoothing changed segments" << endl;
        CoverageDistribution::smooth_segments(settings,
                                              seq.m_seq_id,
                                              summary.unique_coverage[seq.m_seq_id].average, 
                                              this_tiled_complete_coverage_text_file_name, 
                                              this_ranges_text_file_name, 
                                              this_smoothed_ranges_text_file_name,
                                              this_final_cnv_file_name,
                                              this_copy_number_variation_cn_genome_diff_file_name
                                              );
      } 
      settings.done_step(settings.copy_number_variation_done_file_name);
    }
  }
  
  if (settings.do_periodicity){
    create_path (settings.copy_number_variation_path);
      
    if (settings.do_step(settings.periodicity_done_file_name, "Periodicity")){
      for (cReferenceSequences::iterator it = ref_seq_info.begin(); it != ref_seq_info.end(); ++it){
        cAnnotatedSequence& seq = *it;
        
        string this_complete_coverage_text_file_name = settings.file_name(settings.complete_coverage_text_file_name, "@", seq.m_seq_id);
        string this_period_complete_coverage_text_file_name = settings.file_name(settings.periodicity_table_file_name, "@", seq.m_seq_id);
        
        CoverageDistribution::calculate_periodicity(
              this_complete_coverage_text_file_name,
              this_period_complete_coverage_text_file_name,
              settings.periodicity_method,
              settings.periodicity_start,
              settings.periodicity_end,
              settings.periodicity_step
              );
      }
      settings.done_step(settings.periodicity_done_file_name);
    }
  }
   
  create_path(settings.evidence_path); //need output for plots

	if (settings.do_step(settings.output_done_file_name, "Output"))
	{
		///
		// Output Genome Diff File
		///
		cerr << "Creating merged genome diff evidence file..." << endl;

		// merge all of the evidence GenomeDiff files into one...
		create_path(settings.evidence_path);
    
    cGenomeDiff jc_gd(settings.jc_genome_diff_file_name);
         
    // Add read count information to the JC entries -- call fails if no predictions, because BAM does not exist
    if ( jc_gd.list().size() )
      assign_junction_read_counts(settings, jc_gd);
    
    cGenomeDiff ra_mc_gd(settings.ra_mc_genome_diff_file_name);
    
    cGenomeDiff evidence_gd;
    evidence_gd.fast_merge(jc_gd);
    evidence_gd.fast_merge(ra_mc_gd);
    
    // there is a copy number genome diff for each sequence separately
    if (settings.do_copy_number_variation) {
      for (cReferenceSequences::iterator it = ref_seq_info.begin(); it != ref_seq_info.end(); ++it) {
        cAnnotatedSequence& seq = *it;
        string this_copy_number_variation_cn_genome_diff_file_name = settings.file_name(settings.copy_number_variation_cn_genome_diff_file_name, "@", seq.m_seq_id);
        cGenomeDiff cn_gd(this_copy_number_variation_cn_genome_diff_file_name);
        evidence_gd.fast_merge(cn_gd);
      }
    }
		evidence_gd.write(settings.evidence_genome_diff_file_name);

		// predict mutations from evidence in the GenomeDiff
		cerr << "Predicting mutations from evidence..." << endl;

		MutationPredictor mp(ref_seq_info);
    cGenomeDiff mpgd(settings.evidence_genome_diff_file_name);
    mp.predict(settings, summary, mpgd);
    mpgd.assign_unique_ids();

    //#=REFSEQ header lines.
    mpgd.metadata.ref_seqs.resize(settings.reference_file_names.size());
    for (size_t i = 0; i < settings.reference_file_names.size(); i++) {
      mpgd.metadata.ref_seqs[i] = settings.reference_file_names[i];
    }

    //#=READSEQ header lines.
    mpgd.metadata.read_seqs.resize(settings.read_files.size());
    for (size_t i = 0; i < settings.read_files.size(); i++) {
      mpgd.metadata.read_seqs[i] = settings.read_files[i].file_name();
    }

    // Add additional header lines if needed.
    if (settings.add_metadata_to_gd){
      for (storable_map<string, Summary::Coverage>::iterator it = summary.unique_coverage.begin();
            it != summary.unique_coverage.end(); it ++) {
         //Usually needed for gathering breseq data.
       }
    }

    // Write and reload 
    mpgd.write(settings.final_genome_diff_file_name);
    cGenomeDiff gd(settings.final_genome_diff_file_name);

    //
    // Mark marginal items as no_show to prevent further processing
    //
    output::mark_gd_entries_no_show(settings, gd);

    //
		// Annotate mutations
		//
		cerr << "Annotating mutations..." << endl;
		ref_seq_info.annotate_mutations(gd);
    
    gd.write(settings.annotated_genome_diff_file_name);
    
		//
		// Plot coverage of genome and large deletions
		//
		cerr << "Drawing coverage plots..." << endl;
		output::draw_coverage(settings, ref_seq_info, gd);
    
		//
		// Create evidence files containing alignments and coverage plots
		//
		if (!settings.no_alignment_generation)
			output::Evidence_Files(settings, gd);

		///
		// HTML output
		///

		cerr << "Creating index HTML table..." << endl;

		output::html_index(settings.index_html_file_name, settings, summary, ref_seq_info, gd);
		output::html_marginal_predictions(settings.marginal_html_file_name, settings, summary, ref_seq_info, gd);
        
		// record the final time and print summary table
		settings.record_end_time("Output");

		output::html_summary(settings.summary_html_file_name, settings, summary, ref_seq_info);

		settings.done_step(settings.output_done_file_name);
	}
  
  return 0;
}

/*! breseq commands
 
    First argument is a command that should be removed from argv.
 
 */
int main(int argc, char* argv[]) {

	// Extract the sub-command argument
	string command;
	char* argv_new[argc];
	int argc_new = argc - 1;

  // Print out our generic header
  Settings::command_line_run_header();
  
	if (argc > 1) {

		command = argv[1];
		argv_new[0] = argv[0];
		for (int32_t i = 1; i < argc; i++)
			argv_new[i] = argv[i + 1];

	} else {
    breseq_default_action(argc, argv); // Gives default usage in this case.
    return -1; 
	}

	// Pass the command to the proper handler
	command = to_upper(command);
  
  // Sequence Utility Commands:
  if (command == "CONVERT-FASTQ") {
		return do_convert_fastq(argc_new, argv_new);
	} else if (command == "CONVERT-REFERENCE") {
		return do_convert_genbank(argc_new, argv_new);
  } else if ((command == "GET-SEQUENCE") || (command == "SUBSEQUENCE")) {
    return do_get_sequence(argc_new, argv_new);
    
  // Breseq Post-Run Commands:
  } else if (command == "BAM2ALN") {
    return do_bam2aln( argc_new, argv_new);  
  } else if (command == "BAM2COV") {
    return do_bam2cov( argc_new, argv_new);
    
    
  // Experimental and Development Commands:
  //
  // None of these commands are documented for use by others. 
  // They may change without warning.
  } else if ((command == "SIMULATE-READ") ||  (command == "SIMULATE-READS")) {
    return do_simulate_read(argc_new, argv_new); 
  } else if (command == "REFALN") {
    return do_ref_aln();  
  } else if (command == "CNV") {
    return do_copy_number_variation(argc_new, argv_new);
  } else if (command == "PERIODICITY"){
    return do_periodicity(argc_new, argv_new);
  } else if (command == "ERROR_COUNT") {
    return do_error_count(argc_new, argv_new);
  } else if (command == "IDENTIFY_CANDIDATE_JUNCTIONS") {
    return do_identify_candidate_junctions(argc_new, argv_new);
  } else if (command == "JUNCTION-POLYMORPHISM") {
    return do_junction_polymorphism(argc_new, argv_new);
  } else if (command == "CL_TABULATE") {
    return do_tabulate_contingency_loci(argc_new, argv_new);
  } else if (command == "CL_SIGNIFICANCE") {
    return do_analyze_contingency_loci_significance( argc_new, argv_new);
  }
  else {
    // Not a sub-command. Use original argument list.
    return breseq_default_action(argc, argv);
  }
	return -1;
}
