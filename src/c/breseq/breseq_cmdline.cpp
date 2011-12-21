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

#include <iostream>
#include <string>
#include <vector>

#include "libbreseq/anyoption.h"
#include "libbreseq/alignment_output.h"
#include "libbreseq/annotated_sequence.h"
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
#include "libbreseq/contingency_loci.h"
#include "libbreseq/mutation_predictor.h"
#include "libbreseq/output.h"

using namespace breseq;
using namespace std;


/*! bam2aln
 Draw HTML alignment from BAM
 */
int do_bam2aln(int argc, char* argv[]) {
  
  // setup and parse configuration options:
	AnyOption options("Usage: bam2aln --bam=<reference.bam> --fasta=<reference.fasta> --region=<accession:start-end> --output=<output.html> [--max-reads=1000]");
	options
  ("help,h", "produce this help message", TAKES_NO_ARGUMENT)
  ("bam,b", "bam file containing sequences to be aligned", "data/reference.bam")
	("fasta,f", "FASTA file of reference sequence", "data/reference.fasta")
  ("output,o", "output to file [region.html]")
  ("region,r", "region to print (accession:start-end)")
  ("max-reads,n", "maximum number of reads to show in alignment", 200)
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
    // Generate Alignment!
    alignment_output ao(
                        options["bam"],
                        options["fasta"],
                        from_string<uint32_t>(options["max-reads"]),
                        from_string<uint32_t>(options["quality-score-cutoff"])
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
	AnyOption options("Usage: bam2aln --bam=<reference.bam> --fasta=<reference.fasta> --region=<accession:start-end> --output=<output.html> [--max-reads=1000]");
	options
  ("help,h", "produce this help message", TAKES_NO_ARGUMENT)
  // required options
  ("bam,b", "bam file containing sequences to be aligned", "data/reference.bam")
	("fasta,f", "FASTA file of reference sequence", "data/reference.fasta")
  ("output,o", "base name of output file. region appended if multiple output files")
  // which regions to create files for
  ("region,r", "region to print (accession:start-end)", "")
  ("tile-size", "size of tiles")
  ("tile-overlap", "overlap between tiles (1/2 on each side)")
  // options controlling what files are output
  ("plot,p", "create graphical plot of coverage", TAKES_NO_ARGUMENT)
  ("plot-format", "plot format: PNG or PDF", "PNG")
  ("table,t", "create text table of coverage", TAKES_NO_ARGUMENT)
//  ("read_start_output,r", "file name for table file binned by read start bases (DEFAULT: OFF)")
//  ("gc_output,g", "create additional table file binned by GC content of reads (DEFAULT: OFF)")
  // options controlling information that is output
  ("total-only,1", "only plot/tabulate total coverage, not per strand", TAKES_NO_ARGUMENT)
  ("resolution", "rough number of positions to output coverage information for in interval (0=ALL)", 600)
  .processCommandArgs(argc, argv);
  
  // make sure that the required config options are good:
	if(options.count("help")
     || !file_exists(options["fasta"].c_str())
     || !file_exists(options["bam"].c_str()) )
  {
		options.printUsage();
		return -1;
	}
  
  ASSERT(options.count("plot") || options.count("table"), "Must specify either --plot or --table.");

  vector<string> region_list;
  if (options.count("region"))
    region_list= from_string<vector<string> >(options["region"]);
  
  // also take regions off the command line
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
    WARN("Tiling mode activated. Ignoring " + to_string(region_list.size()) + "regions that were specified.");
    region_list.clear();
  }
  
  ASSERT(tiling_mode || (region_list.size() > 0), "No regions specified.");
  
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
    tile_overlap = floor( static_cast<double>(tile_overlap) / 2.0);

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
    
    string file_name = options["output"];
    if ((region_list.size() > 0) || (file_name == ""))  file_name += *it;
    cout << "Coverage for region: " << *it << endl;
    
    if (options.count("table"))
      co.table(*it, file_name + ".tab", from_string<uint32_t>(options["resolution"]));
    
    if (options.count("plot"))
      co.plot(*it, file_name + "." + to_lower(options["plot-format"]) , from_string<uint32_t>(options["resolution"]));
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
		options.printUsage();
		return -1;
	}                       
  
	try {
    
    convert_fastq(options["input"], options["output"], options["in-format"], options["out-format"]);
    
    } catch(...) {
      // failed; 
      return -1;
    }
    
    return 0;
  
}

/*! Convert Genbank
 
 Create a tab-delimited file of information about genes and a
 FASTA sequence file from an input GenBank file.
 */

// Helper function
void convert_genbank(const vector<string>& in, const string& fasta, const string& ft, const string& gff3 ) {

  cReferenceSequences refseqs;

  // Load the GenBank file
  
  
  for (vector<string>::const_iterator it = in.begin(); it < in.end(); it++) 
  {
    refseqs.ReadGenBank(*it);
  }
  
  // Output sequence
  if (fasta != "") refseqs.WriteFASTA(fasta);

  // Output feature table
  if (ft != "") refseqs.WriteFeatureTable(ft);

  if (gff3 != "" ) refseqs.WriteGFF( gff3 );
}

int do_convert_genbank(int argc, char* argv[]) {
	
	// setup and parse configuration options:
	AnyOption options("Usage: breseq CONVERT_GENBANK --input <sequence.gbk> [--fasta <output.fasta> --features <output.tab>]");
	options
		("help,h", "produce this help message", TAKES_NO_ARGUMENT)
		("input,i", "input GenBank flatfile (multiple allowed, comma-separated)")
		("features,g", "output feature table", "")
		("fasta,f", "FASTA file of reference sequences", "")
    ("gff3,v", "GFF file of features", "" )
	.processCommandArgs(argc, argv);
	
	// make sure that the config options are good:
	if(options.count("help")
		 || !options.count("input")
		 || (!options.count("features") && !options.count("fasta"))  
		 ) {
		options.printUsage();
		return -1;
	}
  
	// attempt to calculate error calibrations:
	try {
        
		convert_genbank(  
                    from_string<vector<string> >(options["input"]),
                    options["fasta"],
                    options["features"],
                    options["gff3"]
                    );
  } catch(...) {
		// failed; 
		return -1;
	}
	
	return 0;
}

/*! Calculate Trims
 
 Calculate how much to ignore on the end of reads due to ambiguous alignments
 of those bases.
 
 */
int do_calculate_trims(int argc, char* argv[]) {
	
	// setup and parse configuration options:
	AnyOption options("Usage: breseq CALCULATE_TRIMS --bam <sequences.bam> --fasta <reference.fasta> --error_dir <path> --cGenomeDiff <path> --output <path> --readfile <filename> --coverage_dir <dirname> [--minimum-quality-score 3]");
	options
		("help,h", "produce this help message", TAKES_NO_ARGUMENT)
		("fasta,f", "FASTA file of reference sequence")
		("output,o", "output directory")
	.processCommandArgs(argc, argv);
	
	// make sure that the config options are good:
	if(options.count("help")
		 || !options.count("fasta")
		 || !options.count("output")
		 ) {
		options.printUsage();
		return -1;
	}                       
  
	// attempt to calculate error calibrations:
	try {
		calculate_trims(options["fasta"],options["output"]);
  } catch(...) {
		// failed; 
		return -1;
	}
	
	return 0;
}


/*!  Predict Mutations
 
 Predict mutations from evidence in a genome diff file.
 
 */

int do_predict_mutations(int argc, char* argv[]) {
	
	// setup and parse configuration options:
	AnyOption options("Usage: breseq PREDICT_MUTATIONS ... ");
	options
  ("help,h", "produce this help message", TAKES_NO_ARGUMENT)
  // convert to basing everything off the main output path, so we don't have to set so many options
  ("path", "path to breseq output")
  ("maximum-read-length,m", "number of flanking bases in candidate junctions")
	.processCommandArgs(argc, argv);
  
	// make sure that the config options are good:
	if(options.count("help")
     || !options.count("path")
		 || !options.count("maximum-read-length")
		 ) {
		options.printUsage();
		return -1;
	}                       
  
	try {
    
    Settings settings(options["path"]);
            
    // Load the reference sequence info
    cReferenceSequences ref_seq_info;
    ref_seq_info.LoadFile(settings.reference_gff3_file_name);
        
    MutationPredictor mp(ref_seq_info);
    
    cGenomeDiff gd( settings.evidence_genome_diff_file_name );
    
    mp.predict(
               settings,
               gd,
               from_string<uint32_t>(options["maximum-read-length"])
               );
    
    gd.write(settings.final_genome_diff_file_name);
    
  } catch(...) {
		// failed; 
    
		return -1;
	}
	
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
		breseq::error_count(
                        summary,
                        options["bam"],
												options["fasta"],
												options["output"],
												split(options["readfile"], "\n"),
												options.count("coverage"),
                        options.count("errors"),
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
	AnyOption options("Usage: breseq TABULATE_CL --bam <sequences.bam> --fasta <reference.fasta> --output <path> --loci <loci.txt> --strict");
	options
  ("help,h", "produce this help message", TAKES_NO_ARGUMENT)
  ("bam,b", "bam file containing sequences to be aligned")
  ("fasta,f", "FASTA file of reference sequence")
  ("output,o", "output file")
  ("loci,l", "Contingency loci coordinates" )
  ("strict,s", "exclude non-perfect matches in surrounding 5 bases", TAKES_NO_ARGUMENT)
	.processCommandArgs(argc, argv);
  
	// make sure that the config options are good:
	if(options.count("help")
		 || !options.count("bam")
		 || !options.count("fasta")
		 || !options.count("output")
		 ) {
		options.printUsage();
		return -1;
	}                       
  
	// attempt to calculate error calibrations:
	try {
    
    string loci_file = "";
    if (options.count("loci")) loci_file = options["loci"];
    
    analyze_contingency_loci(
                             options["bam"],
                             options["fasta"],
                             options["output"],
                             loci_file,
                             options.count("strict")
                             );
	} catch(...) {
		// failed; 
		return -1;
	}
	
	return 0;
}


/*! Graph Contingency Loci
 
 Combines output of tabulate contingency loci across difference genomes, 
 and uses R to produce bar plots from this combined file
 
 */
int do_graph_contingency_loci(int argc, char* argv[]) {
	
	// setup and parse configuration options:
	AnyOption options("Usage: breseq GRAPH_CL --bam <sequences.bam> --fasta <reference.fasta> --output <path> --loci <loci.txt> --strict");
	options
  ("help,h", "produce this help message", TAKES_NO_ARGUMENT)
  ("output,o", "output file")
  ("loci,l", "Contingency loci coordinates" )
  ("strict,s", "exclude non-perfect matches in surrounding 5 bases", TAKES_NO_ARGUMENT)
	.processCommandArgs(argc, argv);
  
	// make sure that the config options are good:
	if(options.count("help")
		 || !options.count("bam")
		 || !options.count("fasta")
		 || !options.count("output")
		 ) {
		options.printUsage();
		return -1;
	}                       
  
	// attempt to calculate error calibrations:
	try {
    
    vector<string> v = options.getRemainingArgs();
    
    string loci_file = "";
    if (options.count("loci")) loci_file = options["loci"];
    
    analyze_contingency_loci(
                             options["bam"],
                             options["fasta"],
                             options["output"],
                             loci_file,
                             options.count("strict")
                             );
	} catch(...) {
		// failed; 
		return -1;
	}
	
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
    settings.read_files.Init(from_string<vector<string> >(options["read-file"]));
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



int do_convert_gvf( int argc, char* argv[]){
  AnyOption options("Usage: breseq GD2GVF --input <input.gd> --output <output.gvf>"); 

  options
    ("help,h", "produce this help message", TAKES_NO_ARGUMENT)
    ("input,i","gd file to convert") 
    ("output,o","name of output file")
    ("snv-only", "only output SNV entries", TAKES_NO_ARGUMENT)
    ;
  options.processCommandArgs( argc,argv);
  
  if( !options.count("input") || !options.count("output") ){
      options.printUsage(); return -1;
  }
  
  try{
      GDtoGVF( options["input"], options["output"], options.count("snv-only") );
  } 
  catch(...){ 
      return -1; // failed 
  }
  
  return 0;
}

int do_convert_gd( int argc, char* argv[])
{
    AnyOption options("Usage: VCF2GD --vcf <vcf.vcf> --output <gd.gd>");
    options("vcf,i","gd file to convert");
    options("output,o","name of output file");
    options.processCommandArgs( argc,argv);
    
    if( !options.count("vcf") || !options.count("output") ){
        options.printUsage(); return -1;
    }
    
    try{
        VCFtoGD( options["vcf"], options["output"] );
    } 
    catch(...){ 
        return -1; // failed 
    }
    
    return 0;
}

int do_mutate(int argc, char *argv[])
{
  AnyOption options("Usage: breseq APPLY -g <file.gd> -r <reference>");
  options("genomediff,g", "genome diff file");
  options("reference,r",".gbk/.gff3/.fasta/.bull reference sequence file");
  options("fasta,f","output FASTA file");
  options("gff3,3","output GFF3 file");
  options("verbose,v","Verbose Mode (Flag)", TAKES_NO_ARGUMENT);
  options.processCommandArgs(argc, argv);
  
  options.addUsage("");
  options.addUsage("Input a single GenomeDiff, and as many reference files");
  options.addUsage("as you like.  Using the GenomeDiff, this will apply all");
  options.addUsage("the mutations to the reference sequences, output is to");
  options.addUsage("a single file that includes all the references.");
  
  if (!options.count("genomediff") ||
      !options.count("reference")) {
    options.addUsage("");
    options.addUsage("You must supply BOTH the --genomediff and --reference");
    options.addUsage("options for input.");
    options.printUsage();
    return -1;
  }
  
  if (!options.count("gff3") && !options.count("fasta")) {
    options.addUsage("");
    options.addUsage("You must supply at least one of the --fasta or --gff3 options for output.");
    options.printUsage();
    return -1;
  }  
  
  cGenomeDiff gd(options["genomediff"]);
  cReferenceSequences ref_seq_info;
  cReferenceSequences new_ref_seq_info;
  ref_seq_info.LoadFiles(from_string<vector<string> >(options["reference"]));
  new_ref_seq_info.LoadFiles(from_string<vector<string> >(options["reference"]));

  //Check to see if every item in the loaded .gd is
  // applicable to the reference file.
  ASSERT(gd.is_valid(ref_seq_info, options.count("verbose")), "Reference file and GenomeDiff file don't match.");
  
  gd.apply_to_sequences(ref_seq_info, new_ref_seq_info, options.count("verbose"));

  if (options.count("fasta"))
    new_ref_seq_info.WriteFASTA(options["fasta"], options.count("verbose"));
  if (options.count("gff3"))
    new_ref_seq_info.WriteGFF(options["gff3"], options.count("verbose"));

  return 0;
}

int do_subtract(int argc, char *argv[])
{
  AnyOption options("Usage: breseq SUBTRACT -1 <file.gd> -2 <file.gd> -o <output.gd>");
  options("input1,1","input GD file 1");
  options("input2,2","input GD file 2");
  options("output,o","output GD file");
  options("verbose,v","Verbose Mode (Flag)", TAKES_NO_ARGUMENT);
  options.processCommandArgs(argc, argv);
  
  options.addUsage("");
  options.addUsage("Input two GenomeDiff files and compare mutations.");
  options.addUsage("Mutations that appear in BOTH will be subtracted from");
  options.addUsage("--input1 and results will output to a new file specified.");
  
  if (!options.count("input1") ||
      !options.count("input2")) {
    options.addUsage("");
    options.addUsage("You must supply BOTH the --input1 and --input2 options");
    options.addUsage("for input.");
    options.printUsage();
    return -1;
  }
  
  if (!options.count("output")) {
    options.addUsage("");
    options.addUsage("You must supply the --output option for output.");
    options.printUsage();
    return -1;
  }
  
  cGenomeDiff gd1(options["input1"]);
  cGenomeDiff gd2(options["input2"]);
  
  gd1.subtract(gd2, options.count("verbose"));
  
  gd1.write(options["output"]);
  
  return 0;
}

int do_merge(int argc, char *argv[])
{
  AnyOption options("Usage: breseq MERGE -g <file1.gd file2.gd file3.gd ...> -o <output.gd>");
  options("genomediff,g","input GD files");
  options("output,o","output GD file");
  options("unique,u","Unique Entries Only (Flag)", TAKES_NO_ARGUMENT);  
  options("id,i","Reorder IDs (Flag)", TAKES_NO_ARGUMENT);
  options("verbose,v","Verbose Mode (Flag)", TAKES_NO_ARGUMENT);
  options.processCommandArgs(argc, argv);
  
  options.addUsage("");
  options.addUsage("Input as many GenomeDiff files as you want, and have them");
  options.addUsage("merged together into a single GenomeDiff file specified.");
  options.addUsage("Unique IDs will remain unique across files, any IDs that");
  options.addUsage("aren't unique will get new ones.  Mutations that use those");
  options.addUsage("IDs will be properly updated to acknowledge the change.");
  
  if (!options.count("genomediff")) {
    options.addUsage("");
    options.addUsage("You must supply the --genomediff option for input.");
    options.printUsage();
    return -1;
  }
  
  if (!options.count("output")) {
    options.addUsage("");
    options.addUsage("You must supply the --output option for output.");
    options.printUsage();
    return -1;
  }
  
  vector<string> gd_list = from_string<vector<string> >(options["genomediff"]);  
  cGenomeDiff gd1(gd_list[0]);
  
  //Load all the GD files that were input with -g.
  for(uint32_t i = 1; i < gd_list.size(); i++)
  {
    cGenomeDiff gd2(gd_list[i]);
    gd1.merge(gd2, options.count("unique"), options.count("id"), options.count("verbose"));
  }
  
  //Treat EVERY extra argument passed as another GD file.
  for(int32_t i = 0; i < options.getArgc() ; i++)
  {
    cGenomeDiff gd2(options.getArgv(i));
    gd1.merge(gd2, options.count("unique"), options.count("id"), options.count("verbose"));
  }
  
  gd1.write(options["output"]);
  
  return 0;
}

int do_not_evidence(int argc, char *argv[])
{
  AnyOption options("Usage: breseq NOT_EVIDENCE -g <genomediff.gd> -o <output.gd>");
  options("genomediff,g","input GD files");
  options("output,o","output GD file");
  options("id,i","Reorder IDs (Flag)", TAKES_NO_ARGUMENT);
  options("verbose,v","Verbose Mode (Flag)", TAKES_NO_ARGUMENT);
  options.processCommandArgs(argc, argv);
  
  options.addUsage("");
  options.addUsage("Takes a GenomeDiff file and removes all of the entries that");
  options.addUsage("are NOT used as evidence by a mutation.  Outputs to a new");
  options.addUsage("GenomeDiff file if specified.  If no output is specified,");
  options.addUsage("verbose will still inform what evidence isn't being used.");
  
  if (!options.count("genomediff")) {
    options.addUsage("");
    options.addUsage("You must supply the --genomediff option for input.");
    options.printUsage();
    return -1;
  }
  
  cGenomeDiff gd1(options["genomediff"]);
  
  gd1.filter_not_used_as_evidence(options.count("verbose"));
  
  if(options.count("output"))
  {
    if(options.count("id"))
    {
      cGenomeDiff gd2;
      gd2.merge(gd1, true, true);
      gd2.write(options["output"]);
    }
    else  {
      gd1.write(options["output"]);  }
  }
  
  return 0;
}

int do_compare(int argc, char *argv[])
{
  AnyOption options("Usage: breseq COMPARE -c <control.gd> -t <test.gd> -o <output.gd>");
  options("control,c", "control genome diff file, mutations within are assumed to be accurate");
  options("test,t",    "test genome diff file, mutations not been checked for accuracy");
  options("output,o",  "output compared genome diff file name");
  options.processCommandArgs(argc, argv);

  options.addUsage("Usage: breseq COMPARE -c <control.gd> -t <test.gd> -o <output.gd>");

  if (!options.count("control") ||
      !options.count("test")    ||
      !options.count("output")) {
    options.printUsage();
    return -1;
  }

  cGenomeDiff control_gd(options["control"]);
  cGenomeDiff test_gd(options["test"]);

  cGenomeDiff compare_gd = cGenomeDiff::compare_genome_diff_files(control_gd, test_gd);

  compare_gd.write(options["output"]);


  return 0;
}

int do_intersection(int argc, char *argv[])
{
  AnyOption options("Usage: breseq INTERSECTION -o <output.gd> <file1.gd file2.gd file3.gd ...>");
  options("output,o", "output intersection mutations to this file");
  options.processCommandArgs(argc, argv);

  typedef vector<string> string_vector_t;
  string_vector_t gd_file_names;
  //Get GD Files
  for (int32_t i = 0; i < options.getArgc() ; i++) {
    const string &file_name = options.getArgv(i);
    gd_file_names.push_back(file_name);
  }

  if (!options.count("output") ||
      gd_file_names.size() < 2) {
    printf("\n");
    printf("ERROR!\n");
    printf("Ouput:%s\n", options["output"].c_str());
    printf("GenomeDiff file count: %i\n", static_cast<int>(gd_file_names.size()));
    printf("\n");
    options.printUsage();
    return -1;
  }

  cGenomeDiff first_gd(*gd_file_names.begin());
  const diff_entry_list_t &first_mutations = first_gd.mutation_list();

  //Strip counted_ptr
  typedef set<cDiffEntry> diff_entry_set_t;
  diff_entry_set_t first_mutations_set;
  for (diff_entry_list_t::const_iterator it = first_mutations.begin();
       it != first_mutations.end(); it++) {
    first_mutations_set.insert(**it);
  }

  /*! Step: Compare to other genome diffs and reassign first_mutations_set when applicable.
    Stop when there is no intersection or we run out of file_names
  */
  string_vector_t::const_iterator it_file_name = gd_file_names.begin();
  advance(it_file_name,1);
  while(it_file_name != gd_file_names.end()) {

    cGenomeDiff second_gd(*it_file_name);
    const diff_entry_list_t &second_mutations = second_gd.mutation_list();

    //Strip counted_ptr
    diff_entry_set_t second_mutations_set;
    for (diff_entry_list_t::const_iterator it = second_mutations.begin();
         it != second_mutations.end(); it++) {
      second_mutations_set.insert(**it);
    }

    //Find intersection
    diff_entry_set_t intersecting_mutations_set;
    set_intersection(first_mutations_set.begin(), first_mutations_set.end(),
                     second_mutations_set.begin(), second_mutations_set.end(),
                     inserter(intersecting_mutations_set, intersecting_mutations_set.begin()));

    if (intersecting_mutations_set.empty()) {
      printf("No intersecting mutations were found across the current file: %s\n",
             it_file_name->c_str());
      return -1;
    }


    first_mutations_set = intersecting_mutations_set;

    it_file_name++;
  }

  printf("Found %i intersecting mutations across files: %s.\n",
         static_cast<int>(first_mutations_set.size()), join(gd_file_names, ", ").c_str());

  cGenomeDiff intersecting_gd;

  for (diff_entry_set_t::iterator it = first_mutations_set.begin();
       it != first_mutations_set.end(); it++) {
    //cDiffEntry de = *it;
    intersecting_gd.add(*it);
  }

  intersecting_gd.write(options["output"]);

	return 0;
}

int do_annotate(int argc, char* argv[])
{
  AnyOption options("Usage: breseq ANNOTATE -r reference.gbk  -i input.gd -o annotated.gd");
  
  options
  ("help,h", "produce advanced help message", TAKES_NO_ARGUMENT)
  // convert to basing everything off the main output path, so we don't have to set so many options
  ("input,i", "path to input genome diff (REQUIRED)")
  ("output,o", "path to output genome diff with added mutation data (REQUIRED)")
  ("reference,r", "reference sequence in GenBank flatfile format (REQUIRED)")
  ("ignore-pseudogenes", "treats pseudogenes as normal genes for calling AA changes", TAKES_NO_ARGUMENT)
  ;
  options.processCommandArgs(argc, argv);
  
  if ( !options.count("input")
    || !options.count("output")
    || !options.count("reference")
      ) {
    options.printUsage();
    return -1;
  }
  
  cGenomeDiff gd(options["input"]);
  
  vector<string> reference_file_names = from_string<vector<string> >(options["reference"]);
  cReferenceSequences ref_seq_info;
  ref_seq_info.LoadFiles(reference_file_names);
  ref_seq_info.annotate_mutations(gd, false, options.count("ignore-pseudogenes"));
  gd.write(options["output"]);
  
  return 0;
}

int do_simulate_read(int argc, char *argv[])
{
  AnyOption options("Usage: breseq SIMULATE-READ -g <genome diff> -r <reference file> -c <average coverage> -o <output file>");

  options
  ("genome_diff,g", "Genome diff file.")
  ("reference,r", "Reference file for input.")
  ("coverage,c", "Average coverage value to simulate.", static_cast<uint32_t>(10))
  ("length,l", "Read length to simulate.", static_cast<uint32_t>(36))
  ("output,o", "Output fastq file name.")  
  ("gff3,3", "Output Applied GFF3 File. (Flag)", TAKES_NO_ARGUMENT)
  ("verbose,v", "Verbose Mode (Flag)", TAKES_NO_ARGUMENT)
  ;
  options.processCommandArgs(argc, argv);
  
  options.addUsage("");
  options.addUsage("Takes a genome diff file and applies the mutations to the reference.");
  options.addUsage("Then using the applied reference, it simulates reads based on it.");
  options.addUsage("Output is a .fastq that if run normally against the reference");
  options.addUsage("should produce the same GenomeDiff file you entered.");
  
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

//! Step: Load reference sequence file.
  cReferenceSequences ref_seq_info;
  cReferenceSequences new_ref_seq_info;
  const string &ref_file_name = options["reference"];
  ref_seq_info.LoadFile(ref_file_name);
  new_ref_seq_info.LoadFile(ref_file_name);


  //! Step: Apply genome diff mutations to reference sequence.
  const string &gd_file_name = options["genome_diff"];
  bool verbose = options.count("verbose");
  cGenomeDiff gd(gd_file_name);

  gd.apply_to_sequences(ref_seq_info, new_ref_seq_info, verbose);
  
  //! Write applied GFF3 file if requested.
  if(options.count("gff3"))new_ref_seq_info.WriteGFF(options["output"] + ".gff3", options.count("verbose"));

  const cAnnotatedSequence &sequence = new_ref_seq_info[0];

  //! Step: Create simulated read sequence.
  //Parameters to create simulated read.
  const uint32_t average_coverage = from_string<uint32_t>(options["coverage"]);
  const uint32_t read_length = from_string<uint32_t>(options["length"]);

  const cFastqSequenceVector &fsv =
      cFastqSequenceVector::simulate_from_sequence(sequence, average_coverage, read_length, verbose);

  //! Step: Write simulated reads to file.
  if(verbose){cout << "Writing FASTQ\n" << "\t" << options["output"] << endl;}
  cFastqFile ff(options["output"], ios_base::out);

  const size_t &fsv_size = fsv.size();
  for (size_t i = 0; i < fsv_size; i++) {
    ff.write_sequence(fsv[i]);
    if(verbose && !(i % 10000) && i){cout << "\tREAD: " << i << endl;}
  }
  if(verbose){cout << "\t**FASTQ Complete**" << endl;}
  return 0;
}


int do_normalize_gd(int argc, char* argv[])
{
  AnyOption options("Usage: breseq NORMALIZE-GD -g <input.gd> -r <reference> -o <output.gd>");
  options
  ("genome_diff,g", "Input genome diff file.")
  ("reference,r"  , "Input reference file.")
  ("output,o"     , "Output normalized genome diff file.")
  ("verbose,v"    , "Verbose Mode (Flag)", TAKES_NO_ARGUMENT);

  options.processCommandArgs(argc, argv);
  options.addUsage("");
  options.addUsage("Takes a genome diff file to be normalized.");
  options.addUsage("Then normalizes the diff entries to the reference.");
  options.addUsage("Outputs a normalized genome diff file.");


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

  bool verbose = options.count("verbose");

  vector<string> rfns = from_string<vector<string> >(options["reference"]);
  cReferenceSequences rs;
  rs.LoadFiles(rfns);

  cGenomeDiff gd(options["genome_diff"]);

  printf("\n++Normalizing genome diff file: %s to reference file: %s \n\n",
          options["genome_diff"].c_str(), join(rfns,",").c_str());

  gd.normalize_to_sequence(rs);
  const diff_entry_list_t &muts = gd.mutation_list();

  ofstream out(options["output"].c_str());
  fprintf(out, "#=GENOME_DIFF 1.0\n");
  for (diff_entry_list_t::const_iterator i = muts.begin();
       i != muts.end(); i++) {
    if ((**i).entry_exists("norm") && (**i)["norm"] == "is_not_valid"){
      fprintf(out, "#%s\n", (**i).to_string().c_str());
      printf("\tINVALID_MUTATION:%s\n",(**i).to_string().c_str());
    } else {
      fprintf(out, "%s\n", (**i).to_string().c_str());
    }
  }
  printf("\n++Normilization completed.\n\n");

  return 0;
}

int do_runfile(int argc, char *argv[])
{
  stringstream ss;
  ss << "Usage: breseq RUNFILE -e <executable> -d <downloads dir> -o <output dir> -l <error log dir> -r <runfile name> -g <genome diff data dir>\n";
  ss << "Usage: breseq RUNFILE -e <executable> -d <downloads dir> -o <output dir> -l <error log dir> -r <runfile name> <file1.gd file2.gd file3.gd ...>";
  AnyOption options(ss.str());
  options("executable,e",     "Executable program to run, add extra options here.", "breseq");
  options("downloads_dir,d",  "Downloads directory where read and reference files are located.", "02_Downloads");
  options("output_dir,o",     "Output directory for commands within the runfile.", "03_Output");
  options("runfile,r",        "Name of the run file to be output.", "commands");
  options("data_dir,g",       "Directory to searched for genome diff files.", "01_Data");
  options("error_log_dir,l",  "Directory for error log file that captures the executable's stdout and sterr.", "04_Logs");
  options.addUsage("\n");
  options.addUsage("***Reminder: Create the error log directory before running TACC job.");
  options.addUsage("\n");
  options.addUsage("Examples:");
  options.addUsage("\tCommand: breseq runfile -o 1B4_Mutated -l 1B4_Mutated_Errors 1B4.gd");
  options.addUsage("\t  Output: breseq -o 1B4_Mutated -r NC_012660.1.gbk SRR172993.fastq >& 1B4_Mutated_Errors/1B4.errors.txt");
  options.addUsage("\n");
  options.addUsage("\tCommand: breseq runfile -d 02_Downloads -l 04_Errors -g 01_Data");
  options.addUsage("\t  Output: breseq -o 1B4 -r 02_Downloads/NC_012660.1.gbk 02_Downloads/SRR172993.fastq >& 04_Errors/1B4.errors.txt");
  options.addUsage("\t  Output: breseq -o ZDB111 -r 02_Downloads/REL606.5.gbk 02_Downloads/SRR098039.fastq >& 04_Errors/ZDB111.errors.txt");
  options.processCommandArgs(argc, argv);


  if (!options.getArgc() && !options.count("data_dir")) {
  options.addUsage("");
    options.addUsage("Error: You must input genome diff files or a directory to search for genome diff files.");
    options.printUsage();
    return -1;
  }

  /*! Step: Gather genome diff file names from either user input or a given
   directory to search. */
  vector<string> file_names;
  if (options.count("data_dir")) {
    char *data_dir = strdup(options["data_dir"].c_str());
    const size_t n = strlen(data_dir);
    if (data_dir[n - 1] == '/') {
      data_dir[n - 1] = '\0';
    }
    string command("");
    sprintf(command, "ls %s/*gd", data_dir);
    file_names = split(SYSTEM_CAPTURE(command, true), "\n");
  } else {
    const size_t n = options.getArgc();
    file_names.resize(n);
    for (size_t i = 0; i < n; i++) {
      file_names[i] = options.getArgv(i);
    }
  }
  assert(file_names.size());

  const char *exe = options["executable"].c_str();

  char *downloads_dir = strdup(options["downloads_dir"].c_str());
  if (downloads_dir[strlen(downloads_dir) - 1] == '/') {
    downloads_dir[strlen(downloads_dir) - 1] = '\0';
  }
  char *output_dir = strdup(options["output_dir"].c_str());
  if (output_dir[strlen(output_dir) - 1] == '/') {
    output_dir[strlen(output_dir) - 1] = '\0';
  }

  //! Step: Read every gd file to gather header line info.
  ofstream run_file(options["runfile"].c_str());
  const size_t n = file_names.size();
  for (size_t i = i; i < n; i++) {
    cGenomeDiff gd(file_names[i]);

    //Unique output directory name is parsed from the base name of the file.
    char this_output_dir[1000];
    const char *run_name = gd.metadata.run_name.c_str();
    if (options.count("output_dir")) {
      sprintf(this_output_dir, "-o %s/%s", output_dir, run_name);
    } else {
      sprintf(this_output_dir, "-o %s", run_name);
    }

    //Build reference commands.
    vector<string> refs = gd.metadata.ref_seqs;
    for (size_t j = 0; j < refs.size(); j++) {
      string *ref = &refs[j];
      if (ref->find_last_of(":/") != string::npos) {
        ref->erase(0, ref->find_last_of(":/") + 1);
      }
      if (ref->find(".gbk") == string::npos) {
        ref->append(".gbk");
      }
      if (options.count("downloads_dir")) {
        sprintf(*ref, "-r %s/%s", downloads_dir, refs[j].c_str());
      } else {
        sprintf(*ref, "-r %s", refs[j].c_str());
      }
    }

    //Build read commands.
    vector<string> reads = gd.metadata.read_seqs;
    for (size_t j = 0; j < reads.size(); j++) {
      string *read = &reads[j];
      if (read->find_last_of(":/") != string::npos) {
        read->erase(0, reads[j].find_last_of(":/") + 1);
      }

      if (read->find(".fastq") == string::npos) {
        read->append(".fastq");
      }
      if (read->find(".fastq.gz") != string::npos) {
        read->erase(reads[j].find(".fastq.gz"));
        read->append(".fastq");
      }

      if (options.count("downloads_dir")) {
        sprintf(*read, "%s/%s", downloads_dir, reads[j].c_str());
      } else {
        sprintf(*read, "%s", reads[j].c_str());
      }
    }

    //Error log file option.
    string error_log_path = "";
    {
      const char *path = options["error_log_dir"].c_str();

      if (options.count("error_log_dir")) {
        sprintf(error_log_path, ">& %s/%s.errors.txt", path, run_name);
      } else {
        sprintf(error_log_path, ">& %s.errors.txt", run_name);
      }
    }

    //Build run file line.
    vector<string> run_file_line_args;
    run_file_line_args.push_back(exe);
    run_file_line_args.push_back(this_output_dir);
    run_file_line_args.push_back(join(refs, " "));
    run_file_line_args.push_back(join(reads, " "));
    if (error_log_path.size()) {
      run_file_line_args.push_back(error_log_path);
    }

    const string &run_file_line = join(run_file_line_args, " ");
    printf("%s\n", run_file_line.c_str());
    fprintf(run_file, "%s\n", run_file_line.c_str());
  }

  return 0;
}

int do_copy_number_variation(int argc, char *argv[])
{
	Settings settings(argc, argv);
  
  //(re)load the reference sequences from our converted files
  cReferenceSequences ref_seq_info;
  ref_seq_info.ReadGFF(settings.reference_gff3_file_name);
  
  //create directory
  create_path( settings.copy_number_variation_path );
  
  for (cReferenceSequences::iterator it = ref_seq_info.begin(); it != ref_seq_info.end(); ++it)
  {
    cAnnotatedSequence& seq = *it;
    string this_complete_coverage_text_file_name = settings.file_name(settings.complete_coverage_text_file_name, "@", seq.m_seq_id);
    string this_tiled_complete_coverage_text_file_name = settings.file_name(settings.tiled_complete_coverage_text_file_name, "@", seq.m_seq_id);
    CoverageDistribution::tile(this_complete_coverage_text_file_name, this_tiled_complete_coverage_text_file_name, 500);
    
    string this_ranges_text_file_name = settings.file_name(settings.ranges_text_file_name, "@", seq.m_seq_id);
    CoverageDistribution::find_segments(this_tiled_complete_coverage_text_file_name, this_ranges_text_file_name);
    
    string this_smoothed_ranges_text_file_name = settings.file_name(settings.smoothed_ranges_text_file_name, "@", seq.m_seq_id);
    CoverageDistribution::smooth_segments(this_tiled_complete_coverage_text_file_name, this_ranges_text_file_name, this_smoothed_ranges_text_file_name);
    
   }
  
  return 0;
}

int do_download(int argc, char *argv[])
{
  stringstream ss;
  ss << "Usage: breseq DOWNLOAD -l <user:password> -d <download_dir> -g <genome_diff_dir>\n";
  ss << "Usage: breseq DOWNLOAD -l <user:password> -d <download_dir> <file1.gd file2.gd file3.gd ...>\n";

  AnyOption options(ss.str());
  options("login,l",           "Login user:password information for private server access.", "");
  options("download_dir,d",    "Output directory to download file to.", "02_Downloads");
  options("genome_diff_dir,g", "Directory to searched for genome diff files.", "01_Data");
  
  options.processCommandArgs(argc, argv);

  if (!options.getArgc() && options["genome_diff_dir"].empty()) {
    options.addUsage("\nYou must input genome diff files or a directory to search for genome diff files.");
    options.addUsage("Examples:");
    options.addUsage("\t breseq DOWNLOAD -l john:1234 -d downloads -g data");
    options.addUsage("\t breseq DOWNLOAD -l john:1234 -d downloads 1B4.gd GRC2000.gd");
    options.printUsage();
    return -1;
  }

  printf("\n++Starting download.\n");

  //! Step: Initialized user parameters.
  const bool private_access = options.count("login");
  string user = "";
  string password = "";
  if (private_access) {
    const string &login = options["login"];

    size_t pos = login.find_first_of(':');
    assert (pos != string::npos);

    user = login.substr(0, pos);
    password = login.substr(pos + 1);
    printf("Private access enabled, user:%s password:%s\n", user.c_str(), password.c_str());
  }

  string download_dir = options["download_dir"];
  if (download_dir[download_dir.size() - 1] == '/') 
    download_dir.erase(download_dir.size() - 1);
  create_path(download_dir);

  string genome_diff_dir = options["genome_diff_dir"];
  if (genome_diff_dir[genome_diff_dir.size() - 1] == '/') 
    genome_diff_dir.erase(genome_diff_dir.size() - 1);
  create_path(genome_diff_dir);

  //! Step: Define currently used genome diff header tags.
  //These header keys determine which URL to download from.
  enum {
    GENBANK,
    SRA,
    BARRICK_PUBLIC,
    BARRICK_PRIVATE
  };

  map<string, int32_t> key_lookup
      = make_map<string, int32_t>
      ("Genbank"            , GENBANK)
      ("SRA"                , SRA)
      ("BarrickLab-Public"  , BARRICK_PUBLIC)
      ("BarrickLab-Private" , BARRICK_PRIVATE);

  //Map the keys to a C-style format string that represents the URL.
  map<int32_t, string> key_url_format
      = make_map<int32_t, string>
      (GENBANK,        "http://www.ncbi.nlm.nih.gov/sviewer/?db=nuccore&val=%s&report=gbwithparts&retmode=text")
      (SRA,            "ftp://ftp.sra.ebi.ac.uk/vol1/fastq/%s/%s/%s.fastq.gz")
      (BARRICK_PUBLIC, "http://barricklab.org/%s")
      (BARRICK_PRIVATE,"ftp://" + user + ":" + password + "@barricklab.org//community/%s");
    // the double slash is not a typo
    //(BARRICK_PRIVATE,"ftp://" + user + ":" + password + "@backup.barricklab.org/%s"); //@JEB temporary change


  //Map the keys to a C-style format string that represents the download file's to-be-downloaded/local file path.
  map<int32_t, string>key_file_path_format
      = make_map<int32_t, string>
      (GENBANK,         download_dir + "/%s.gbk")
      (SRA,             download_dir + "/%s.fastq.gz")
      (BARRICK_PUBLIC,  download_dir + "/%s")
      (BARRICK_PRIVATE, download_dir + "/%s");


  //! Step: Collect genome diff file names from either user directory or file input.
  list<string> file_names;
  if (!options.getArgc()) {
    printf("Searching directory %s for genome diff files.\n", genome_diff_dir.c_str());
    string cmd = "";
    sprintf(cmd, "ls %s/*gd", genome_diff_dir.c_str());
    vector<string> temp = split(SYSTEM_CAPTURE(cmd, true), "\n");
    file_names = list<string>(temp.begin(), temp.end());
  } else {
    printf("User input genome diff files.\n");
    const size_t n = options.getArgc();
    for (size_t i = 0; i < n; i++) {
      file_names.push_back(options.getArgv(i));
    }
  }
  copy(file_names.begin(), file_names.end(), ostream_iterator<string>(cout, "\n"));
  assert(file_names.size());

  //! Step: Parse given download directory to avoid re-downloading files.
  set<string> downloaded;//Used to filter files that have already been downloaded.
  {
    string cmd = "";
    sprintf(cmd, "ls %s", download_dir.c_str());
    vector<string> files = split(SYSTEM_CAPTURE(cmd, true), "\n");
    for (size_t i = 0; i < files.size(); i++) {
      string &file = files[i];
      downloaded.insert(download_dir + "/" + file);
      //Check that it doesn't exist as a compressed file.
      if (file.find(".gz") != file.size() - 3) {
        downloaded.insert(download_dir + "/" + file.append(".gz"));
      }
    }
  }

  //! Step: Begin parsing the genome diff files to collect key:value pairs.
  list<pair<string, string> > key_values;
  set<string> values; //Used to filter duplicate values.
  for(;file_names.size(); file_names.pop_front()) {
    cGenomeDiff gd(file_names.front());

    const vector<string> &refs = gd.metadata.ref_seqs;
    for (size_t i = 0; i < refs.size(); i++) {
      const string &ref = refs[i];
      const size_t pos = ref.find_first_of(':');
      if (pos == string::npos) {
        continue;
      }
      const string &key = ref.substr(0, pos);
      const string &value = ref.substr(pos + 1);
      if (!values.count(value)) {
        key_values.push_back(pair<string, string>(key, value));
        values.insert(value);
        downloaded.insert(value);
      }
    }

    const vector<string> &reads = gd.metadata.read_seqs;
    for (size_t i = 0; i < reads.size(); i++) {
      const string &read = reads[i];
      const size_t pos = read.find_first_of(':');
      if (pos == string::npos) {
        continue;
      }
      const string &key = read.substr(0, pos);
      const string &value = read.substr(pos + 1);
      if (!values.count(value)) {
        key_values.push_back(pair<string, string>(key, value));
        values.insert(value);
        downloaded.insert(value);
      }
    }
  }
  assert (key_values.size());

  //! Step: Build url and file paths depending on given key.
  list<pair<string, string> > url_file_paths;
  for (;key_values.size(); key_values.pop_front()) {
    ASSERT(key_lookup.count(key_values.front().first), "Could not find key: " + key_values.front().first );
    const int32_t key =  key_lookup[key_values.front().first];
    const string value = key_values.front().second;

    // strip path from filename
    string filename = value;
    size_t pos = filename.find_last_of('/');
    if (pos == string::npos) 
      pos = 0;
    else
      pos++;
    filename.erase(0, pos);
    
    const char *url_format       = key_url_format[key].c_str();
    const char *file_path_format = key_file_path_format[key].c_str();

    string url = "";
    string file_path = "";

    switch (key)
    {
    case GENBANK:
    {
      sprintf(url, url_format, value.c_str());
      sprintf(file_path, file_path_format, filename.c_str());

    } break;

    case SRA:
    {
      const char *first  = value.substr(0,6).c_str();
      const char *second = value.substr(0,9).c_str();
      const char *third  = value.c_str();
      sprintf(url, url_format, first, second, third);
      sprintf(file_path, file_path_format, filename.c_str());
    } break;

    case BARRICK_PUBLIC:
    {
      sprintf(url, url_format, value.c_str());
      sprintf(file_path, file_path_format, filename.c_str());
    } break;

    case BARRICK_PRIVATE:
    {
      if(!private_access) continue;
      sprintf(url, url_format, value.c_str());
      sprintf(file_path, file_path_format, filename.c_str());
    } break;

    default: break;
    }//End switch.

    assert(url.size() || file_path.size());

    url_file_paths.push_back(pair<string, string>(url, file_path));
  }

  //! Step: Create wget commands to download files.
  list<string> gzip_files;//Collect compressed files.
  for (;url_file_paths.size(); url_file_paths.pop_front()) {
    const string &url = url_file_paths.front().first;
    const string &file_path = url_file_paths.front().second;
    bool is_gzipped = (file_path.rfind(".gz") == file_path.size() - 3);
    if (is_gzipped) {
      //Check to see if the file was already downloaded and gunzipped.
      const string &gunzip_file_path = file_path.substr(0, file_path.rfind(".gz"));
      if (file_exists(gunzip_file_path.c_str()) && !file_empty(gunzip_file_path.c_str())) {
        printf("File:%s already exists in directory %s.\n", gunzip_file_path.c_str(), download_dir.c_str());
        continue;
      } else {
        gzip_files.push_back(file_path);
      }
    }

    if (file_exists(file_path.c_str()) && !file_empty(file_path.c_str())) {
      printf("File:%s already exists in directory %s.\n", file_path.c_str(), download_dir.c_str());
      continue;
    }

    string cmd = "";
    sprintf(cmd, "wget -O %s %s", file_path.c_str(), url.c_str());
    SYSTEM(cmd);

  }

  //! Step: Uncompress files that need it.
  for (;gzip_files.size(); gzip_files.pop_front()) {
    const string &file_path = gzip_files.front();
    string cmd = "";
    sprintf(cmd, "gunzip %s", file_path.c_str());
    SYSTEM(cmd);
  }

  return 0;
}


int do_subsequence(int argc, char *argv[])
{
  AnyOption options("Usage: breseq SUBSEQUENCE -r <reference> -o <output.fasta> -p <REL606:50-100>");
  options("reference,r",".gbk/.gff3/.fasta reference sequence file", "data/reference.fasta");
  options("output,o","output FASTA file");  
  options("position,p","Sequence ID:Start-End");
  options("complement,c","Reverse Complement (Flag)", TAKES_NO_ARGUMENT);
  options("verbose,v","Verbose Mode (Flag)", TAKES_NO_ARGUMENT);
  options.processCommandArgs(argc, argv);
  
  options.addUsage("");
  options.addUsage("Takes a reference sequence and outputs to FASTA a subset");
  options.addUsage("of the sequence.  Using '0' as the End position will set");  
  options.addUsage("it to the length of the relevant sequence.");
  
  if(!file_exists(options["reference"].c_str()))  {
    options.addUsage("");
    options.addUsage("You must supply a valid --reference option for input.");
    options.addUsage("Could not find:");
    options.addUsage(options["reference"].c_str());
    options.printUsage();
    return -1;    
  }
   
  vector<string> region_list;  
  
  bool reverse = options.count("complement");
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
      replace_end = ref_seq_info[replace_target_id].m_length;  }
    
    if(verbose)
    {
      cout << "SEQ_ID:\t\t" << ref_seq_info[replace_target_id].m_seq_id << endl;
      cout << "SEQ_INDEX:\t" << replace_target_id << endl;
      cout << "START:\t\t" << replace_start << endl;
      cout << "END:\t\t" << replace_end << endl;
    }
    
    CHECK(replace_start <= replace_end,
          "START:\t" + to_string(replace_start) + "\n" +
          "END:\t" + to_string(replace_end) + "\n" +
          "START greater than END.");
    
    ASSERT((uint32_t)ref_seq_info[replace_target_id].m_length >= replace_start && (uint32_t)ref_seq_info[replace_target_id].m_length >= replace_end,
           "START:\t" + to_string(replace_start) + "\n" +
           "END:\t" + to_string(replace_end) + "\n" +
           "SIZE:\t" + to_string(ref_seq_info[replace_target_id].m_length) + "\n" +
           "Neither Start or End can be greater than the size of " + ref_seq_info[replace_target_id].m_seq_id + ".");
    
    seq_name = ref_seq_info[replace_target_id].m_seq_id + ":" + to_string(replace_start) + "-" + to_string(replace_end);
    
    new_seq_info.add_new_seq(seq_name);
    cAnnotatedSequence& new_seq = new_seq_info[seq_name];
    new_seq.m_fasta_sequence = ref_seq_info[replace_target_id].m_fasta_sequence;    
    new_seq.m_fasta_sequence.m_name = seq_name;
    new_seq.m_fasta_sequence.m_sequence = ref_seq_info[replace_target_id].m_fasta_sequence.m_sequence.substr(replace_start -1, (replace_end - replace_start) + 1);
    if(reverse)new_seq.m_fasta_sequence.m_sequence = reverse_complement(new_seq.m_fasta_sequence.m_sequence);
    new_seq.m_seq_id = seq_name;
    new_seq.m_length = new_seq.m_fasta_sequence.m_sequence.size();
    
    if(verbose)  {
      cout << new_seq.m_fasta_sequence.m_sequence << endl;  }
  }
  
  if(options.count("output"))  {
    new_seq_info.WriteFASTA(options["output"], verbose);  }  
  
  return 0;
}

int do_convert_exact_match(int argc, char *argv[])
{
  AnyOption options("Usage: breseq CONVERT_EXACT_MATCH");
  options("input,i","input file");
  options("output,o","output file");
  options("verbose,v","Verbose Mode (Flag)", TAKES_NO_ARGUMENT);
  options.processCommandArgs(argc, argv);
  
  options.addUsage("");
  options.addUsage("Takes an input file with Start1, Start2, and Size columns");  
  options.addUsage("that delineate exact matches and sorts them.  Output is to"); 
  options.addUsage("tab delimmited file with no header info.");
  
  if (!options.count("input")) {
    options.addUsage("");
    options.addUsage("You must supply the --input option for input.");
    options.printUsage();
    return -1;
  }
  
  if (!options.count("output")) {
    options.addUsage("");
    options.addUsage("You must supply the --output option for output.");
    options.printUsage();
    return -1;
  }
  
  if (!file_exists(options["input"].c_str())) {
    options.addUsage("");
    options.addUsage("File selected for --input does not exist.");
    options.printUsage();
    return -1;
  }
  
  ifstream in(options["input"].c_str());
  ASSERT(in.good(), "Could not open reference file: " + options["input"]);
  
  string line;
  
  // Grab 2 lines.  The file we'll be dealing with has a single header line
  // followed by the column names.  This will drop that info.
  getline(in, line);
  getline(in, line);
  
  map<uint32_t, uint32_t> match_list;
  
  while (!in.eof() && getline(in,line))
  {
    RemoveLeadingTrailingWhitespace(line);
    vector<string> cols = split_on_whitespace(line);
    
    bool complement = (cols[1].find("r") != string::npos);
    if(complement)  {
      cols[1].resize(cols[1].size() - 1);  }
    
    ASSERT(cols.size() == 3, "Column size is incorrect");
    
    uint32_t start1 = from_string<uint32_t>(cols[0]);
    uint32_t start2 = from_string<uint32_t>(cols[1]);
    uint32_t match_size = from_string<uint32_t>(cols[2]);
    
    if(complement)
    {
      start2 = start2 - (match_size - 1);
    }
    
    if(match_list.count(start1))
    {
      if(match_list[start1] < match_size)  {
        match_list[start1] = match_size;  }
    }
    else
    {
      match_list[start1] = match_size;
    }
    
    if(match_list.count(start2))
    {
      if(match_list[start2] < match_size)  {
        match_list[start2] = match_size;  }
    }
    else
    {
      match_list[start2] = match_size;
    }
  }
  
  in.close();
  
  // Go through everything in the list and combine entries
  // that overlap the same area.
  for(map<uint32_t, uint32_t>::iterator i = match_list.begin(); i != match_list.end();)
  {
    bool no_loop = true;
    uint32_t end_pos = (*i).first + (*i).second;
    uint32_t& uSize = (*i).second;
    
    for(map<uint32_t, uint32_t>::iterator j = ++i; j != match_list.end() && ((*j).first - 1) <= end_pos;)
    {
      uint32_t pot_end_pos = (*j).first + (*j).second;
      if(pot_end_pos > end_pos)
      {
        uSize += pot_end_pos - end_pos;
        end_pos = pot_end_pos;
      }
      
      map<uint32_t, uint32_t>::iterator j_temp = j;
      j_temp++;
      match_list.erase(j);      
      j = j_temp;
      i = j;
      no_loop = false;
    }
    
    if(no_loop)  {
      i++;  }
  }
  
  // Everything has been read in, and combined where necessary.
  // Now is the time for output.
  ofstream out(options["output"].c_str());
  ASSERT(!out.fail(), "Failed to open/create " + options["output"]);
  
  for(map<uint32_t, uint32_t>::iterator k = match_list.begin(); k != match_list.end(); k++)
  {
    out << (*k).first << "\t" << (*k).second << endl;
  }
  
  return 0;
}

int do_rand_muts(int argc, char *argv[])
{
  AnyOption options("Usage: breseq CONVERT_EXACT_MATCH");
  options("exclude,e","input file");
  options("type,t","output file");
  options("number,n","output file");
  options("length,l","output file");
  options("reference,r","output file");
  options("seq,s","output file");  
  options("output,o","output file");
  options("verbose,v","Verbose Mode (Flag)", TAKES_NO_ARGUMENT);
  options.processCommandArgs(argc, argv);
  
  
  cReferenceSequences ref_seq_info;
  ref_seq_info.LoadFiles(from_string<vector<string> >(options["reference"]));
  
  cGenomeDiff gd1;
  gd1.random_mutations(options["exclude"], options["type"], from_string<uint32_t>(options["number"]), from_string<uint32_t>(options["length"]), ref_seq_info[0]);
  
  gd1.write(options["output"]);
  
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

    //Check the FASTQ format and collect some information about the input read files at the same time
		cerr << "  Analyzing FASTQ read files..." << endl;
		uint32_t overall_max_read_length = UNDEFINED_UINT32;
		uint32_t overall_max_qual = 0;

		s.num_reads = 0;
    s.num_bases = 0;
		for (uint32_t i = 0; i < settings.read_files.size(); i++)
		{
			string base_name = settings.read_files[i].m_base_name;
			cerr << "    READ FILE::" << base_name << endl;
			string fastq_file_name = settings.base_name_to_read_file_name(base_name);
			string convert_file_name =  settings.file_name(settings.converted_fastq_file_name, "#", base_name);

			// Parse output
			AnalyzeFastq s_rf = normalize_fastq(fastq_file_name, convert_file_name, i+1);
      
			// Save the converted file name -- have to save it in summary because only that
			// is reloaded if we skip this step.
			s.converted_fastq_name[base_name] = s_rf.converted_fastq_name;

			// Record statistics
			if ((overall_max_read_length == UNDEFINED_UINT32) || (s_rf.max_read_length > overall_max_read_length))
				overall_max_read_length = s_rf.max_read_length;
			if ((overall_max_qual == UNDEFINED_UINT32) || (s_rf.max_quality_score > overall_max_qual))
				overall_max_qual = s_rf.max_quality_score;
			s.num_reads += s_rf.num_reads;
			s.num_bases += s_rf.num_bases;

			s.reads[base_name] = s_rf;
		}
		s.avg_read_length = s.num_bases / s.num_reads;
		s.max_read_length = overall_max_read_length;
		s.max_qual = overall_max_qual;
		summary.sequence_conversion = s;

		// create SAM faidx
		string samtools = settings.ctool("samtools");
		string command = samtools + " faidx " + settings.reference_fasta_file_name;
		SYSTEM(command.c_str());
    
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
	for (map<string, AnalyzeFastq>::iterator it = summary.sequence_conversion.reads.begin(); it != summary.sequence_conversion.reads.end(); it++)
	{
		string read_file = it->first;
		if (it->second.converted_fastq_name.size() > 0)
			settings.read_files.read_file_to_converted_fastq_file_name_map[read_file] = it->second.converted_fastq_name;
	}

	//
  // 02_reference_alignment
	// * Match all reads against the reference genome
	//

	if (settings.do_step(settings.reference_alignment_done_file_name, "Read alignment to reference genome"))
	{
		create_path(settings.reference_alignment_path);

		/// create ssaha2 hash
		string reference_hash_file_name = settings.reference_hash_file_name;
		string reference_fasta_file_name = settings.reference_fasta_file_name;

		if (!settings.smalt)
		{
			string command = "ssaha2Build -rtype solexa -skip " + to_string(settings.ssaha2_skip_length) + " -save " + reference_hash_file_name + " " + reference_fasta_file_name;
			SYSTEM(command.c_str());
		}
		else
		{
			string smalt = settings.ctool("smalt");
			string command = smalt + " index -k " + to_string(settings.ssaha2_seed_length) + " -s " + to_string(settings.ssaha2_skip_length) + " " + reference_hash_file_name + " " + reference_fasta_file_name;
      SYSTEM(command.c_str());
		}
		/// ssaha2 align reads to reference sequences
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

				if (!settings.smalt)
				{
					string command = "ssaha2 -disk 2 -save " + reference_hash_file_name + " -kmer " + to_string(settings.ssaha2_seed_length) + " -skip " + to_string(settings.ssaha2_skip_length) + " -seeds 1 -score 12 -cmatch 9 -ckmer 1 -output sam_soft -outfile " + reference_sam_file_name + " " + read_fastq_file;
					SYSTEM(command.c_str());
				}
				else
				{
					string smalt = settings.ctool("smalt");
					string command = smalt + " map -n 2 -d " + to_string(settings.max_smalt_diff) + " -f samsoft -o " + reference_sam_file_name + " " + reference_hash_file_name + " " + read_fastq_file; // -m 12
					SYSTEM(command.c_str());
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
	if (settings.junction_prediction)
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
      SYSTEM(command.c_str());
      command = samtools + " sort " + coverage_junction_best_bam_unsorted_file_name + " " + coverage_junction_best_bam_prefix;
      SYSTEM(command.c_str());
      if (!settings.keep_all_intermediates)
        remove(coverage_junction_best_bam_unsorted_file_name.c_str());
      command = samtools + " index " + coverage_junction_best_bam_file_name;
      SYSTEM(command.c_str());

      // Count errors
      string reference_fasta_file_name = settings.reference_fasta_file_name;
      string reference_bam_file_name = settings.coverage_junction_best_bam_file_name;

      error_count(
        summary,
        reference_bam_file_name,
        reference_fasta_file_name,
        settings.candidate_junction_path,
        settings.read_file_names,
        true, // coverage
        false, // errors
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
		if (settings.do_step(settings.candidate_junction_done_file_name, "Identifying candidate junctions"))
		{
			cerr << "Identifying candidate junctions..." << endl;
      CandidateJunctions::identify_candidate_junctions(settings, summary, ref_seq_info);

			string samtools = settings.ctool("samtools");
			string faidx_command = samtools + " faidx " + settings.candidate_junction_fasta_file_name;
			if (!file_empty(settings.candidate_junction_fasta_file_name.c_str()))
				SYSTEM(faidx_command.c_str());

			summary.candidate_junction.store(candidate_junction_summary_file_name);
			settings.done_step(settings.candidate_junction_done_file_name);
		}
		summary.candidate_junction.retrieve(candidate_junction_summary_file_name);

    
    //
    // 04 candidate_junction_alignment
    // * Align reads to new junction candidates
    //
		if (settings.do_step(settings.candidate_junction_alignment_done_file_name, "Candidate junction alignment"))
		{
			create_path(settings.candidate_junction_alignment_path);

			/// create ssaha2 hash
			string candidate_junction_hash_file_name = settings.candidate_junction_hash_file_name;
			string candidate_junction_fasta_file_name = settings.candidate_junction_fasta_file_name;

			if (!file_empty(candidate_junction_fasta_file_name.c_str()))
			{
				if (!settings.smalt)
				{
					string command = "ssaha2Build -rtype solexa -skip " + to_string(settings.ssaha2_skip_length) + " -save " + candidate_junction_hash_file_name + " " + candidate_junction_fasta_file_name;
					SYSTEM(command.c_str());
				}
				else
				{
					string smalt = settings.ctool("smalt");
					string command = smalt + " index -k " + to_string(settings.ssaha2_seed_length) + " -s " + to_string(settings.ssaha2_skip_length) + " " + candidate_junction_hash_file_name + " "+ candidate_junction_fasta_file_name;
					SYSTEM(command.c_str());
				}
			}

			/// ssaha2 align reads to candidate junction sequences
			for (uint32_t i = 0; i < settings.read_files.size(); i++)
			{
				string base_read_file_name = settings.read_files[i].m_base_name;
				string candidate_junction_sam_file_name = settings.file_name(settings.candidate_junction_sam_file_name, "#", base_read_file_name);

				string read_fastq_file = settings.base_name_to_read_file_name(base_read_file_name);
				string filename = candidate_junction_hash_file_name + ".base";
				if (!settings.smalt && file_exists(filename.c_str()))
				{
					string command = "ssaha2 -disk 2 -save " + candidate_junction_hash_file_name + " -best 1 -rtype solexa -kmer " + to_string(settings.ssaha2_seed_length) + " -skip " + to_string(settings.ssaha2_skip_length) + " -seeds 1 -output sam_soft -outfile " + candidate_junction_sam_file_name + " " + read_fastq_file;
					SYSTEM(command.c_str());
					// Note: Added -best parameter to try to avoid too many matches to redundant junctions!
				}
				else
				{
					filename = candidate_junction_hash_file_name + ".sma";
					if (file_exists(filename.c_str()))
					{
						string smalt = settings.ctool("smalt");
						string command = smalt + " map -c 0.8 -x -n 2 -d 1 -f samsoft -o " + candidate_junction_sam_file_name + " " + candidate_junction_hash_file_name + " " + read_fastq_file;
						SYSTEM(command.c_str());
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
	if (settings.do_step(settings.alignment_correction_done_file_name, "Resolving alignments with candidate junctions"))
	{
		create_path(settings.alignment_resolution_path);
    
		// should be one coverage cutoff value for each reference sequence
		//vector<double> coverage_cutoffs;
		//for (uint32_t i = 0; i < ref_seq_info.size(); i++)
    //{
    //  Coverage f = summary.preprocess_coverage[ref_seq_info[i].m_seq_id];
	//		coverage_cutoffs.push_back(summary.preprocess_coverage[ref_seq_info[i].m_seq_id].junction_accept_score_cutoff);
		//}
    //assert(coverage_cutoffs.size() == ref_seq_info.size());

    bool junction_prediction = settings.junction_prediction;
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
		if (settings.junction_prediction && !file_empty(resolved_junction_sam_file_name.c_str()))
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

		uint32_t num_read_files = summary.sequence_conversion.reads.size();
		uint32_t num_qual = summary.sequence_conversion.max_qual + 1;

		error_count(
      summary,
			reference_bam_file_name, // bam
			reference_fasta_file_name, // fasta
			settings.error_calibration_path, // output
			settings.read_files.base_names(), // readfile
			true, // coverage
			true, // errors
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
			command = "R --vanilla in_file=" + error_rates_base_qual_error_prob_file_name + " out_file=" + error_rates_plot_file_name + " < " + plot_error_rates_r_script_file_name + " > " + plot_error_rates_r_script_log_file_name;
			SYSTEM(command);
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

		if (settings.do_step(settings.mutation_identification_done_file_name, "Read alignment mutations"))
		{
			string reference_fasta_file_name = settings.reference_fasta_file_name;
			string reference_bam_file_name = settings.reference_bam_file_name;

			string coverage_fn = settings.file_name(settings.unique_only_coverage_distribution_file_name, "@", "");
			string error_dir = dirname(coverage_fn) + "/";
			string output_dir = settings.mutation_identification_path;
			string ra_mc_genome_diff_file_name = settings.ra_mc_genome_diff_file_name;
			string coverage_tab_file_name = settings.file_name(settings.complete_coverage_text_file_name, "@", "");
			string coverage_dir = dirname(coverage_tab_file_name) + "/";

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
				output_dir,
				coverage_dir,
        deletion_propagation_cutoffs,
        deletion_seed_cutoffs,
				settings.mutation_log10_e_value_cutoff, // mutation_cutoff
				settings.base_quality_cutoff, // minimum_quality_score
				settings.polymorphism_log10_e_value_cutoff, // polymorphism_cutoff
				settings.polymorphism_frequency_cutoff, //polymorphism_frequency_cutoff
				error_dir + "/error_rates.tab",
				false //per_position_file
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

    if (settings.do_step(settings.copy_number_variation_done_file_name, "Copy number variation")) { 
      for (cReferenceSequences::iterator it = ref_seq_info.begin(); it != ref_seq_info.end(); ++it) {
        cAnnotatedSequence& seq = *it;
        string this_complete_coverage_text_file_name = settings.file_name(settings.complete_coverage_text_file_name, "@", seq.m_seq_id);
        string this_tiled_complete_coverage_text_file_name = settings.file_name(settings.tiled_complete_coverage_text_file_name, "@", seq.m_seq_id);
        CoverageDistribution::tile(this_complete_coverage_text_file_name, this_tiled_complete_coverage_text_file_name, 500);
       
        string this_ranges_text_file_name = settings.file_name(settings.ranges_text_file_name, "@", seq.m_seq_id);
        CoverageDistribution::find_segments(this_tiled_complete_coverage_text_file_name, this_ranges_text_file_name);
       
        string this_smoothed_ranges_text_file_name = settings.file_name(settings.smoothed_ranges_text_file_name, "@", seq.m_seq_id);
        CoverageDistribution::smooth_segments(this_tiled_complete_coverage_text_file_name, this_ranges_text_file_name, this_smoothed_ranges_text_file_name);
      } 
      settings.done_step(settings.copy_number_variation_done_file_name);
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
    cGenomeDiff ra_mc_gd(settings.ra_mc_genome_diff_file_name);
        
    cGenomeDiff merged_genome_diff;
    cGenomeDiff evidence_gd = cGenomeDiff::fast_merge(jc_gd, ra_mc_gd);
		evidence_gd.write(settings.evidence_genome_diff_file_name);

		// predict mutations from evidence in the GenomeDiff
		cerr << "Predicting mutations from evidence..." << endl;

		MutationPredictor mp(ref_seq_info);
    cGenomeDiff mpgd(settings.evidence_genome_diff_file_name);
    mp.predict(settings, mpgd, summary.sequence_conversion.max_read_length, summary.sequence_conversion.avg_read_length);

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
       for (storable_map<string, Coverage>::iterator it = summary.unique_coverage.begin();
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

		output::html_statistics(settings.summary_html_file_name, settings, summary, ref_seq_info);

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
  if (command == "CONVERT_FASTQ") {
		return do_convert_fastq(argc_new, argv_new);
	} else if (command == "CALCULATE_TRIMS") {
		return do_calculate_trims(argc_new, argv_new);
	} else if (command == "CONVERT_GENBANK") {
		return do_convert_genbank(argc_new, argv_new);
	} else if (command == "TABULATE_CL") {
		return do_tabulate_contingency_loci(argc_new, argv_new);
	} else if (command == "GRAPH_CL") {
		return do_graph_contingency_loci(argc_new, argv_new);
	} else if (command == "ERROR_COUNT") {
		return do_error_count(argc_new, argv_new);
	} else if (command == "IDENTIFY_CANDIDATE_JUNCTIONS") {
		return do_identify_candidate_junctions(argc_new, argv_new);
  } else if (command == "PREDICT_MUTATIONS") {
		return do_predict_mutations(argc_new, argv_new);
	} else if (command == "GD2GVF") {
    return do_convert_gvf(argc_new, argv_new);
  } else if (command == "VCF2GD") {
    return do_convert_gd( argc_new, argv_new);
  } else if (command == "BAM2ALN") {
    return do_bam2aln( argc_new, argv_new);  
  } else if (command == "BAM2COV") {
    return do_bam2cov( argc_new, argv_new);    
  } else if ((command == "APPLY") || (command == "MUTATE")) {
    return do_mutate(argc_new, argv_new);    
  } else if (command == "SUBTRACT") {
    return do_subtract(argc_new, argv_new);    
  } else if (command == "UNION" || command == "MERGE") {
    return do_merge(argc_new, argv_new);    
  } else if (command == "NOT_EVIDENCE") {
    return do_not_evidence(argc_new, argv_new);
  } else if (command == "COMPARE") {
    return do_compare(argc_new, argv_new);
  } else if (command == "INTERSECTION") {
    return do_intersection(argc_new, argv_new);
  } else if (command == "ANNOTATE") {
    return do_annotate(argc_new, argv_new);
  } else if (command == "SIMULATE-READ") {
    return do_simulate_read(argc_new, argv_new);
  } else if (command == "NORMALIZE-GD") {
    return do_normalize_gd(argc_new, argv_new);
  } else if ((command == "SUBSEQUENCE") || (command == "SUB")) {
    return do_subsequence(argc_new, argv_new);
  } else if (command == "RUNFILE") {
    return do_runfile(argc_new, argv_new);
  } else if (command == "CNV") {
    return do_copy_number_variation(argc_new, argv_new);
  } else if (command == "DOWNLOAD") {
    return do_download(argc_new, argv_new);
  } else if ((command == "CONVERT_EXACT_MATCH") || (command == "CEV")) {
    return do_convert_exact_match(argc_new, argv_new);
  } else if ((command == "RANDOM_MUTATIONS") || (command == "RAND_MUTS")) {
    return do_rand_muts(argc_new, argv_new);
  }
  else {
    // Not a sub-command. Use original argument list.
    return breseq_default_action(argc, argv);
  }
	return -1;
}
