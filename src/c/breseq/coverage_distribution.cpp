
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

#include "libbreseq/coverage_distribution.h"

#include "libbreseq/genome_diff.h"

using namespace std;

namespace breseq {
  /*! fit
    @abstract Function for the interaction with R using the script coverage_distribution.r
    @param settings Used for file_paths
    @param distribution_file_name Input file for R, created by error_count() and saved as *.unique_only_coverage_distribution.tab
    @param plot_file Output by R
    @return vector<string> Each line contains a parameter set by R
    !*/

  vector<string> CoverageDistribution::fit(
                                           Settings& settings, 
                                           string distribution_file_name, 
                                           string plot_file, 
                                           double deletion_propagation_pr_cutoff, 
                                           double junction_coverage_pr_cutoff, 
                                           double junction_accept_pr_cutoff, 
                                           double junction_keep_pr_cutoff, 
                                           double junction_max_score
                                           )
  {
    pid_t pid = getpid();
    string log_file_name = distribution_file_name + ".r.log";
    string command = "R --vanilla < " + settings.program_data_path + "/coverage_distribution.r" + " > " + log_file_name;
    command += " distribution_file=" + distribution_file_name;
    command += " plot_file=" + plot_file;
    command += " deletion_propagation_pr_cutoff=" + to_string<double>(deletion_propagation_pr_cutoff);
    command += " junction_coverage_pr_cutoff=" + to_string<double>(junction_coverage_pr_cutoff);
    command += " junction_accept_pr_cutoff=" + to_string<double>(junction_accept_pr_cutoff);
    command += " junction_keep_pr_cutoff=" + to_string<double>(junction_keep_pr_cutoff);
    command += " junction_max_score=" + to_string<double>(junction_max_score);
    
    SYSTEM(command);

    ifstream ROUT(log_file_name.c_str());
    string line;
    vector<string> lines;
    while (getline(ROUT, line))
    {
      size_t pos = line.find("[1]");
      if (pos == 0)
      {
        lines.push_back(line.substr(pos+3));
      }
    }
    ROUT.close();
//    remove(log_file_name.c_str());

    return(lines);
  }

  // helper functions
  /*! analyze_unique_coverage_distribution
    @abstract Assigns variables to be sent off to the R script coverage_distribution.r


    @param settings
    @param summary
    @param ref_seq_info
    !*/

  void CoverageDistribution::analyze_unique_coverage_distribution(
                                                                  Settings& settings, 
                                                                  Summary& summary, 
                                                                  cReferenceSequences& ref_seq_info, 
                                                                  string seq_id, 
                                                                  string plot_key, 
                                                                  string distribution_file_name
                                                                  )
  {
    //initialize summary information
    summary.unique_coverage[seq_id].nbinom_size_parameter = NAN;
    summary.unique_coverage[seq_id].nbinom_mean_parameter = NAN;
    summary.unique_coverage[seq_id].nbinom_prob_parameter = NAN;
    summary.unique_coverage[seq_id].average = 1.0;
    summary.unique_coverage[seq_id].variance = NAN;
    summary.unique_coverage[seq_id].dispersion = NAN;
    summary.unique_coverage[seq_id].deletion_coverage_propagation_cutoff = 5.0;
    summary.unique_coverage[seq_id].deletion_coverage_seed_cutoff = 0;

    string unique_only_coverage_plot_file_name = settings.file_name(plot_key, "@", seq_id);
    string unique_only_coverage_distribution_file_name = settings.file_name(distribution_file_name, "@", seq_id);

    // Define various coverage thresholds...
    uint32_t sequence_length = ref_seq_info[ref_seq_info.seq_id_to_index(seq_id)].m_length;

    /// DELETION PROPAGATION CUTOFF
    // One-tailed test p=0.05, Bonferroni correction
    //# my del_propagation_pr_cutoff = 0.05 / sequence_length;

    // One-tailed test p=0.01, no Bonferroni correction
    //#my del_propagation_pr_cutoff = 0.01;

    // We really want somewhere between these two, try this...
    double deletion_propagation_pr_cutoff = 0.05 / sqrt(sequence_length);

    /// NEW JUNCTION COVERAGE CUTOFFS
    // Arbitrary value that seems to work....
    //double junction_coverage_pr_cutoff =  sqrt(settings.junction_accept_pr / static_cast<double>(sequence_length));
    double junction_coverage_pr_cutoff = 0.01;
    
    // We really want somewhere between these two, try this...
    double junction_accept_pr_cutoff = 0.01;
    double junction_keep_pr_cutoff = 0.01 / sqrt(sequence_length);
    int32_t junction_max_score = int(2 * summary.sequence_conversion.avg_read_length);
    
    CoverageDistribution dist;
    vector<string> lines = dist.fit(settings, 
                                    unique_only_coverage_distribution_file_name, 
                                    unique_only_coverage_plot_file_name,
                                    deletion_propagation_pr_cutoff, 
                                    junction_coverage_pr_cutoff, 
                                    junction_accept_pr_cutoff, 
                                    junction_keep_pr_cutoff, 
                                    junction_max_score
                                    );

    // First two lines are negative binomial parameters.
    // Next three lines are average, standard deviation, and index of overdispersion

    // Put these into summary
    summary.unique_coverage[seq_id].nbinom_size_parameter = from_string<double>(lines[0]);
    summary.unique_coverage[seq_id].nbinom_mean_parameter = from_string<double>(lines[1]);
    // Calculated by formula, prob = size/(size + mu)
    summary.unique_coverage[seq_id].nbinom_prob_parameter = summary.unique_coverage[seq_id].nbinom_size_parameter / (summary.unique_coverage[seq_id].nbinom_mean_parameter + summary.unique_coverage[seq_id].nbinom_size_parameter);
    summary.unique_coverage[seq_id].average = from_string<double>(lines[2]);
    summary.unique_coverage[seq_id].variance = from_string<double>(lines[3]);
    summary.unique_coverage[seq_id].dispersion = from_string<double>(lines[4]);

    summary.unique_coverage[seq_id].deletion_coverage_propagation_cutoff = from_string<double>(lines[5]);
    summary.unique_coverage[seq_id].junction_coverage_cutoff = from_string<double>(lines[6]);
    
    // deprecated statistics
    //summary.unique_coverage[seq_id].junction_accept_score_cutoff = from_string<double>(lines[7]);
    //summary.unique_coverage[seq_id].junction_keep_score_cutoff = from_string<double>(lines[8]);
        
    bool verbose = false;
    if (verbose)
    {
      cout << seq_id << endl;
      cout << "nbinom_size_parameter " << summary.unique_coverage[seq_id].nbinom_size_parameter << endl;
      cout << "nbinom_mean_parameter " << summary.unique_coverage[seq_id].nbinom_mean_parameter << endl;
      cout << "nbinom_prob_parameter " << summary.unique_coverage[seq_id].nbinom_prob_parameter << endl;
      cout << "average " << summary.unique_coverage[seq_id].average << endl;
      cout << "variance " << summary.unique_coverage[seq_id].variance << endl;
      cout << "dispersion " << summary.unique_coverage[seq_id].dispersion << endl;
      cout << "deletion_coverage_propagation_cutoff " << summary.unique_coverage[seq_id].deletion_coverage_propagation_cutoff << endl;
      cout << "junction_coverage_cutoff " << summary.unique_coverage[seq_id].junction_coverage_cutoff << endl;
      //cout << "junction_accept_score_cutoff " << summary.unique_coverage[seq_id].junction_accept_score_cutoff << endl;
      //cout << "junction_keep_score_cutoff " << summary.unique_coverage[seq_id].junction_keep_score_cutoff << endl;
      //cout << "pr_no_coverage_position_strand " << summary.unique_coverage[seq_id].pr_no_coverage_position_strand << endl;

    }
  }

  void CoverageDistribution::analyze_unique_coverage_distributions(
                                                                   Settings& settings, 
                                                                   Summary& summary, 
                                                                   cReferenceSequences& ref_seq_info, 
                                                                   string plot_file_name, 
                                                                   string distribution_file_name
                                                                   )
  {
    for (uint32_t i = 0; i < ref_seq_info.size(); i++) {
          
      analyze_unique_coverage_distribution(
                                           settings, 
                                           summary, 
                                           ref_seq_info, 
                                           ref_seq_info[i].m_seq_id, 
                                           plot_file_name, 
                                           distribution_file_name
                                           );
    }
  }
  
  void CoverageDistribution::tile(
                                  bool ignore_redundant_coverage,
                                  string in_file_name,
                                  string out_file_name,
                                  uint32_t tile_size
                                  )
  {
    //The index is the marker of the current tile.
    //It is always a multiple of tile_size
    uint32_t tile_index;
    
    //these are for taking running sums and averaging.
    double tile_sum;
    double average;
    
    //position and coverage are taken from the file.
    uint32_t position;
    double coverage;
    double sum_coverage;
    
    //used for checking if there is any redundant coverage
    double redundant1;
    double redundant2;
    double redundant3;
    double redundant4;
    
    //if ignoring redundant coverage is enabled, this will disable writing the
    //tile
    bool do_write;
    
    //used to make sure the very last position is not written twice.
    uint32_t last_position;

    //used to ignore values in the input file.
    string skip;
    
    ifstream in_file;
    ofstream out_file;
    
    in_file.open ( in_file_name.c_str() );
    out_file.open ( out_file_name.c_str() );
    
    //skipping the first line in the input file.
    getline(in_file, skip);
    
    out_file << "position\tcoverage\t" << tile_size << "\n";
    
    out_file << "0\t0\n";
    
    tile_index = 0;
    tile_sum = 0;
    
    do_write = true;
    
    while ( in_file >> coverage )
    {
      sum_coverage = coverage;
      
      in_file >> coverage;
      sum_coverage += coverage;
      
      //next 4 are redundant values;
      in_file >> redundant1 >> redundant2 >> redundant3 >> redundant4;
      
      //the next value in the line isn't used.
      in_file >> skip;
      
      //last is position
      in_file >> position;
      
      coverage = sum_coverage;
      
      //Check if the current position is in the current tile.
      //If it is, add the coverage to the sum and increment the count.
      //If it isn't, write the last tile and set up the variables for the new 
      //tile
      if ( position <= tile_index + tile_size )
      {
        tile_sum += coverage;
      }
      else
      {
        if ( do_write )
        {
          average = tile_sum / tile_size;
          
          out_file << tile_index + tile_size << "\t" << average << "\n";
        }
        //Set tile_index to position rounded down to the nearest multiple
        //of tile_size
        tile_index = position / tile_size * tile_size;
        
        tile_sum = coverage;
        
        last_position = position;
        
        do_write = true;
      }
      
      if ( ignore_redundant_coverage )
      {
        if ( redundant1 != 0 || 
             redundant2 != 0 ||
             redundant3 != 0 ||
             redundant4 != 0 )
        {
          //skip this tile.
          do_write = false;
        }
      }
      
    }
    
    if ( position % tile_size == 0 && position != last_position )
    {
      average = tile_sum / tile_size;
      
      out_file << tile_index << "\t" << average << "\n";
    }
    
    in_file.close();
    out_file.close();
  }
  
  void CoverageDistribution::find_segments(
                                          string in_file_name,
                                          string out_file_name
                                          )
  {
    //a search entry represents a segment of positions in the file that will be
    //examined. start and end are inclusive.
    //search entries are referencing the index in the file, not position
    pair<uint32_t, uint32_t> next_search;
    pair<uint32_t, uint32_t> current_search;
    
    //this holds all search entries
    vector< pair<uint32_t, uint32_t> > searching;
    
    //used to populate saved_sums
    double running_sum;
    
    //a saved_sum entry represents the sum of all coverage up to a position.
    //saved_sums are first calculated then accessed later.
    map<uint32_t, double> saved_sums;
    
    //ordered_sums holds the order that saved_sums was populated.
    vector<uint32_t> ordered_sums;
    
    //position and coverage are taken from the file.
    uint32_t position;
    double coverage;
    
    //p_i and p_j represent the position of the indeces of i and j in the later
    //nested for-loops
    uint32_t p_i;
    uint32_t p_j;
    
    //best_i and best_j define the segments that scored the best in an iteration
    //these represent indeces, not positions.
    uint32_t best_i;
    uint32_t best_j;
    
    //number of entries in the input file
    uint32_t count;
    
    //highest position value in the input file
    int32_t n;
    
    //Si, Sj and Sn define sums of the segments of 0 to i, 0 to j and 0 to n.
    //i and j are defined later in loo
    double Si;
    double Sj;
    double Sn;
    
    //the z value as per the CBS algorithm is the score of a given segment.
    double z;
    //to easily see the z calculation z1 and z2 are used.
    double z1;
    double z2;
    
    //best_z is the highest Z value found in an iteration.
    double best_z;
    
    //to skip the header
    string skip;
    
    ifstream in_file;
    ofstream out_file;
    
    in_file.open ( in_file_name.c_str() );
    out_file.open ( out_file_name.c_str() );
    
    getline(in_file, skip);
    
    out_file << "Start_Position\tEnd_Position\tZ_Score\n";
    
    //First, populate saved_sums and ordered_positions.
    //and get length
    count = 0;
    running_sum = 0;
    
    while ( in_file >> position >> coverage )
    {
      running_sum += coverage;
      saved_sums[position] = running_sum;
      ordered_sums.push_back( position );
      count++;
      
    }
    
    //the value of position is now the highest position value.
    n = position;
    Sn = running_sum;
    
    //The first search entry goes from 1 to count - 1
    //0 isn only used as a place holder.
    current_search.first = 1;
    current_search.second = count - 1;
    searching.push_back ( current_search );
    
    //now calculate z for each segment inside of current_search
    //repeat until there are no more search entries.
    while ( searching.size() > 0 )
    {
      current_search.first = searching.back().first;
      current_search.second = searching.back().second;
      
      searching.pop_back();
      
      best_z = 0;
      
      //i and j are inclusive boundaries so <= is used in the comparison.
      
      for ( uint32_t i = current_search.first - 1; i <= current_search.second; i++ )
      {
        p_i = ordered_sums[i];
        Si = saved_sums[ ordered_sums[i] ];
        for ( uint32_t j = i + 1; j <= current_search.second; j++ )
        {
          p_j = ordered_sums[j];
          Sj = saved_sums[ ordered_sums[j] ];
          
          z1 = ((Sj - Si) / (p_j - p_i)) - ((Sn - Sj + Si) / (n - p_j + p_i));
          z2 = sqrt((1.0 / (p_j - p_i)) + (1.0 / (n - p_j + p_i)));
          z =  z1 / z2;
          
          if ( abs(z) > best_z )
          {
            best_z = abs(z);
            best_i = i;
            best_j = j;
          } 
        }
      }
      
      //update searching for next iteration.
      //if current_search only spanned one index, only write its indeces
      //and do not update searching.
      //if current_search spans multiple indeces, write the best i and j segment
      //and update for the next iteration.
      
      if ( current_search.first == current_search.second )
      {
        out_file << ordered_sums[current_search.first] << "\t"
        << ordered_sums[current_search.second] << "\t" 
        << "0" << endl;
      }
      else
      {
        //writing.
        
        out_file << ordered_sums[best_i + 1] << "\t"
                 << ordered_sums[best_j] << "\t"
                 << best_z <<  endl;
        
        
        //setting up next_searches
        
        //if best_i doesn't include the first element, add the segment before
        //best_i
        if ( best_i + 1 != current_search.first )
        {
          
          next_search.first = current_search.first;
          next_search.second = best_i;
          searching.push_back( next_search );
          
        }
        
        //if best_j doesn't include the last element, add the segment after
        //best_j
        if ( best_j != current_search.second )
        {
          
          next_search.first = best_j + 1;
          next_search.second = current_search.second;
          searching.push_back( next_search );
          
        }
      }
    }
    
    in_file.close();
    out_file.close();
  }
  
  void CoverageDistribution::smooth_segments(
                                              const Settings& settings,
                                              const string& seq_id,
                                              double summary_average,
                                              string tile_file_name,
                                              string segment_file_name,
                                              string out_file_name,
                                              string gd_file_name
                                              )
  {
    
    cGenomeDiff gd;
    
    //used for finding the mean of a segment in the tile file.
    double segment_sum;
    double segment_mean;
    uint32_t num_tiles;
    
    //keeps track of a segment's bounds.
    uint32_t position_start;
    uint32_t position_end;
    
    //keeps track of an entry in the tile file.
    uint32_t tile_position;
    double tile_coverage;
    
    //holds the value a segment's coverage will be changed to.
    double new_segment_mean;
    
    //used for easily accessing the tile file.
    pair<uint32_t, double> tile_entry;
    vector< pair<uint32_t, double> > tile_data;
    
    //initial parameter.
    uint32_t tile_size;
    
    //used for skipping lines.
    string skip;
    
    ifstream tile_file;
    ifstream segment_file;
    ofstream out_file;
    
    tile_file.open ( tile_file_name.c_str() );
    segment_file.open ( segment_file_name.c_str() );
    out_file.open ( out_file_name.c_str() );
    
    out_file << "Position\tSmooth_Coverage\n";
    
    tile_file >> skip >> skip >> tile_size;
    
    //populate the tile_data vector
    num_tiles = 0;
    while ( tile_file >> tile_position >> tile_coverage )
    {
      tile_entry.first = tile_position;
      tile_entry.second = tile_coverage;
      tile_data.push_back( tile_entry );
    }
    num_tiles = tile_position / 500;
    
    //go through each entry in the segment file.
    getline(segment_file, skip);
    while ( segment_file >> position_start >> position_end >> skip )
    {
      segment_sum = 0;
      num_tiles = 0;
      //get the mean of the segment in the tile file.
      //search through tile_data and find the segment.
      for ( uint32_t i = 0; i < tile_data.size(); i++ )
      {
        tile_entry = tile_data[i];
        
        if ( position_start <= tile_entry.first &&
             position_end >= tile_entry.first )
        {
          segment_sum += tile_entry.second;
          num_tiles++;
        }
        
        else if ( tile_entry.first > position_end )
        {
          break;
        }
        
      }
      segment_mean = segment_sum / num_tiles;
      
      //calculate the new value of the segment by taking the mean of the
      //segment divided by the mean of the tile data.
      new_segment_mean = segment_mean / summary_average;
      
      //this rounding method doesn't handle negative values
      //fortunately, there's no such thing as negative coverage
      new_segment_mean = floor(new_segment_mean + .5);
      
      // @JEB create the genome diff evidence entry if mean is not one
      
      if (new_segment_mean != 1.0) {
        cDiffEntry item(CN);
        item[SEQ_ID] = seq_id;
        item[START] = to_string<uint32_t>(position_start);
        item[END] = to_string<uint32_t>(position_end);
        item["tile_size"] = to_string<double>(settings.copy_number_variation_tile_size);
        item["copy_number"] = to_string<double>(new_segment_mean);
        stringstream num;
        num << fixed << setprecision(2) << (segment_mean / summary_average);
        item["relative_coverage"] = num.str();
        gd.add(item);
      }
      
      
      //set the new value of the coverage for this segment to this value.
      for ( uint32_t i = 0; i < tile_data.size(); i++ )
      {
        tile_entry = tile_data[i];
        
        if ( position_start <= tile_entry.first &&
             position_end >= tile_entry.first )
        {
          tile_data[i].second = new_segment_mean;
        }
        
        else if ( tile_entry.first > position_end )
        {
          break;
        }
      }
    }
    
    //write new data.
    for ( uint32_t i = 0; i < tile_data.size(); i++ )
    {
      tile_entry = tile_data[i];
      out_file << tile_entry.first << "\t" << tile_entry.second << endl;
    }
    
    tile_file.close();
    segment_file.close();
    out_file.close();
    
    gd.write(gd_file_name);
  }

  
} // namespace breseq
