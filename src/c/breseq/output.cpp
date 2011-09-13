#include "libbreseq/output.h"
#include "libbreseq/anyoption.h"
#include "libbreseq/alignment_output.h"
#include "libbreseq/coverage_output.h"

using namespace std;
namespace breseq
{

/*-----------------------------------------------------------------------------
 *  Diff_Entry Keywords 
 *-----------------------------------------------------------------------------*/
const char* ALIGNMENT_EMPTY_CHANGE_LINE="alignment_empty_change_line";
const char* ALIGNMENT_OVERLAP="alignment_overlap";
const char* BAM_PATH="bam_path";
const char* DELETED="deleted";
const char* FASTA_PATH="fasta_path";
const char* FILE_NAME="file_name";
const char* FISHER_STRAND_P_VALUE="fisher_strand_p_value";
const char* FLANKING_LEFT="flanking_left";
const char* GENES="genes";
const char* GENE_NAME="gene_name";
const char* GENE_POSITION="gene_position";
const char* GENE_PRODUCT="gene_product";
const char* GHOST_END="ghost_end";
const char* GHOST_SEQ_ID_END="ghost_seq_id_end";
const char* GHOST_SEQ_ID_START="ghost_seq_id_start";
const char* GHOST_START="ghost_start";
const char* GHOST_STRAND_END="ghost_strand_end";
const char* GHOST_STRAND_START="ghost_strand_start";
const char* INSERT_END="insert_end";
const char* INSERT_START="insert_start";
const char* ITEM="item";
const char* KS_QUALITY_P_VALUE="ks_quality_p_value";
const char* MC_SIDE_1="mc_side_1";
const char* MC_SIDE_2="mc_side_2";
const char* NEW_SEQ="new_seq";
const char* NO_SHOW="no_show";
const char* PLOT="plot";
const char* PREFIX="prefix";
const char* SIZE="size";
const char* TRUNCATE_END="truncate_end";
const char* TRUNCATE_START="truncate_start";
const char* _COVERAGE_PLOT_FILE_NAME="_coverage_plot_file_name";
const char* _EVIDENCE_FILE_NAME="_evidence_file_name";
const char* _NEW_JUNCTION_EVIDENCE_FILE_NAME="_new_junction_evidence_file_name";
const char* _SIDE_1_EVIDENCE_FILE_NAME="_side_1_evidence_file_name";
const char* _SIDE_2_EVIDENCE_FILE_NAME="_side_2_evidence_file_name";
//For JC
const char* SIDE_1_OVERLAP="side_1_overlap";
const char* SIDE_1_POSITION="side_1_position";
const char* SIDE_1_SEQ_ID="side_1_seq_id";
const char* SIDE_1_STRAND="side_1_strand";
const char* SIDE_2_POSITION="side_2_position";
const char* SIDE_2_SEQ_ID="side_2_seq_id";
const char* SIDE_2_STRAND="side_2_strand";
const char* SIDE_1_JC="side_1_jc";
const char* SIDE_2_JC="side_2_jc";
namespace output
{

/*
 * =====================================================================================
 *        Class:  HTML
 *  Description:  
 * =====================================================================================
 */
/*-----------------------------------------------------------------------------
 *  HTML Attribute Keywords
 *-----------------------------------------------------------------------------*/
const char* ALIGN_CENTER="align=\"center\"";
const char* ALIGN_RIGHT="align=\"right\"";
const char* ALIGN_LEFT="align=\"left\"";

/*-----------------------------------------------------------------------------
 *  HTML Utility for printing numbers
 *-----------------------------------------------------------------------------*/
string commify(const string& input)
{
  string retval;
  string temp = input;
  reverse(temp.begin(), temp.end());
  for (size_t i = 0; i < input.size(); i++) {
    if ((i+1)%3 > 0) {
      retval.push_back(temp[i]);
    } else  {
      retval.push_back(temp[i]);
      if (i+1 != input.size()) {
        retval.push_back(',');
      }
    }
  }
  reverse(retval.begin(),retval.end());
   return retval;
}
/*-----------------------------------------------------------------------------
 *  HTML Utility for Encoding HTML
 *-----------------------------------------------------------------------------*/
string nonbreaking(const string& input)
{   
  string retval = input;
  
  /* substitute nonbreaking en dash */
  retval = substitute(retval, "–", "&#8211;");
  
  /* substitute nonbreaking hyphen */
  retval = substitute(retval, "-", "&#8209;");

  /* substitute nonbreaking space */
  retval = substitute(retval, " ", "&nbsp;");

  return retval;
}
/*-----------------------------------------------------------------------------
 *  HTML Utility for Encoding HTML
 *-----------------------------------------------------------------------------*/
string htmlize (const string& input) 
{
  string retval = input;
    
  /* substitute nonbreaking en dash */
  retval = substitute(retval, "–", "&#8211;");

  return retval;
}

/*-----------------------------------------------------------------------------
 *  These style definitions are included between the HTML <head> 
 *  tags of every genereated .html page.
 *-----------------------------------------------------------------------------*/
string header_style_string() 
{
  stringstream ss;
  ss << "body {font-family: sans-serif; font-size: 11pt;}"                 << endl;
  ss << "th {background-color: rgb(0,0,0); color: rgb(255,255,255);}"      << endl;
  ss << "table {background-color: rgb(1,0,0); color: rgb(0,0,0);}"         << endl;
  ss << "tr {background-color: rgb(255,255,255);}"                         << endl;
  ss << ".mutation_in_codon {color:red; text-decoration : underline;}"     << endl;
  ss << ".mutation_header_row {background-color: rgb(0,130,0);}"           << endl;
  ss << ".read_alignment_header_row {background-color: rgb(255,0,0);}"     << endl;
  ss << ".missing_coverage_header_row {background-color: rgb(0,100,100);}" << endl;
  ss << ".new_junction_header_row {background-color: rgb(0,0,155);}"       << endl;
  ss << ".alternate_table_row_0 {background-color: rgb(255,255,255);}"     << endl;
  ss << ".alternate_table_row_1 {background-color: rgb(230,230,245);}"     << endl;
  ss << ".gray_table_row {background-color: rgb(230,230,245);}"           << endl;
  ss << ".polymorphism_table_row {background-color: rgb(160,255,160);}"    << endl;
  ss << ".highlight_table_row {background-color: rgb(192,255,255);}"       << endl;
  ss << ".reject_table_row {background-color: rgb(255,200,165);}"          << endl;
  ss << ".information_table_row {background-color: rgb(200,255,255);}"     << endl;
  ss << ".junction_repeat {background-color: rgb(255,165,0)}"              << endl;
  ss << ".junction_gene {}" << endl;
  
return ss.str();
}



void html_index(const string& file_name, const Settings& settings, Summary& summary,
                cReferenceSequences& ref_seq_info, genome_diff& gd)
{
  (void)summary; //TODO: unused?
  
  // Create Stream and Confirm It's Open
  ofstream HTML(file_name.c_str());

  if (!HTML.good()) {
    cerr << "Could not open file: " << file_name << endl;
    assert(HTML.good());
  }
  
  // Build HTML Head
  HTML << html_header("BRESEQ :: Mutation Predictions", settings);
  HTML << breseq_header_string(settings) << endl;
  HTML << "<p>" << endl;
// #   ###
// #   ## Mutation predictions
// #   ###
  HTML << "<!--Mutation Predictions -->" << endl;
  diff_entry_list muts = gd.show_list(make_list<Type>(SNP)(INS)(DEL)(SUB)(MOB)(AMP));
  
  string relative_path = settings.local_evidence_path;
  
  if(!relative_path.empty())
    relative_path += "/";
  
  //Determine if more than one reference sequence is used
  bool one_ref_seq;

  if (get_keys<string,string>(ref_seq_info.ref_strings).size() == 1)
    one_ref_seq = true;
  else
    one_ref_seq = false;

  //Build Mutation Predictions table
  HTML << "<p>" << endl;
  HTML << Html_Mutation_Table_String(settings, gd, muts, relative_path, false, one_ref_seq) << endl;

// #   ###
// #   ## Unassigned evidence
// #   ###
  HTML << "<!--Unassigned evidence-->" << endl;
  
  diff_entry_list mc = gd.filter_used_as_evidence(gd.show_list(make_list<Type>(MC)));
  mc.remove_if(diff_entry::rejected()); 

  if (mc.size() > 0) {
    HTML << "<p>" << html_missing_coverage_table_string(mc, false, "Unassigned missing coverage evidence", relative_path);
  }

  diff_entry_list jc = gd.filter_used_as_evidence(gd.show_list(make_list<Type>(JC)));
  jc.remove_if(diff_entry::rejected()); 

  //Don't show junctions for circular chromosomes
  if (settings.hide_circular_genome_junctions) {
    jc.remove_if(diff_entry::field_exists("circular_chromosome")); 
  }
   
  diff_entry_list jcu = jc;
  if (jcu.size() > 0) {
    HTML << "<p>" << endl;
    HTML << html_new_junction_table_string(jcu, false, "Unassigned new junction evidence...", relative_path);
  }

  HTML << html_footer();
  HTML.close();
}



void html_marginal_predictions(const string& file_name, const Settings& settings,Summary& summary,
                               cReferenceSequences& ref_seq_info, genome_diff& gd)
{
  (void)summary; //TODO: unused?
  
  // Create Stream and Confirm It's Open
  ofstream HTML(file_name.c_str());
  
  if(!HTML.good()) {
    cerr << "Could not open file: " <<  file_name << endl;
    assert(HTML.good());
  }

  // Build HTML Head
  HTML << html_header("BRESEQ :: Marginal Predictions",settings); 
  HTML << breseq_header_string(settings) << endl;
  HTML << "<p>" << endl;
  
  string relative_path = settings.local_evidence_path;
  
  if (!relative_path.empty())
    relative_path += "/";
  
  //Determine if more than one reference sequence is used
  bool one_ref_seq;
  if (get_keys<string,string>(ref_seq_info.ref_strings).size() == 1)
    one_ref_seq = true;
  else
    one_ref_seq = false; 

  // ###
  // ## Marginal evidence
  // ###
  diff_entry_list ra = gd.filter_used_as_evidence(gd.show_list(make_list<Type>(RA)));
  
  if (ra.size() > 0) {
    HTML << "<p>" << endl;
    HTML << html_read_alignment_table_string(ra, false, "Marginal read alignment evidence...",
      relative_path) << endl;
  }
  
    diff_entry_list jc = gd.filter_used_as_evidence(gd.show_list(make_list<Type>(JC)));
    jc.remove_if(not1(diff_entry::field_exists("reject")));
   if (jc.size()) {
     //Sort by score, not by position (the default order)...
     jc.sort(diff_entry::by_scores(
       make_list<diff_entry::key_t>("pos_hash_score")("total_reads"))); 
    
     HTML << "<p>" << endl;
     HTML << html_new_junction_table_string(jc, false, "Marginal new junction evidence...", relative_path);
   }

  HTML << html_footer();
  HTML.close();
}

string html_header (const string& title, const Settings& settings)
{
  stringstream ss(ios_base::out | ios_base::app);  
  
  ss << "<!DOCTYPE html" << endl;
	ss << "PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"" << endl;
  ss << "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">" << endl;
  ss << "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en-US\" xml:lang=\"en-US\">" << endl;
  
  ss << "<html>" << endl;  
  ss << "<head>" << endl;
  ss << "<title>";
  if (!settings.print_run_name.empty()) {
    ss << settings.print_run_name << " :: ";
  }
  ss << title;
  ss << "</title>" << endl;
  
  ss << "<style type = \"text/css\">" << endl;
  ss << header_style_string() << endl;
  ss << "</style>" << endl;
  ss << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\" />" << endl;
  ss << "</head>" << endl;
  ss << "<body>" << endl;
  return ss.str();
}



void html_compare(Settings& settings,const string &file_name, const string &title, genome_diff& gd,
                  bool one_ref_seq, vector<string>& gd_name_list_ref, Options& options)
{
  // Create stream and confirm it's open
  ofstream HTML(file_name.c_str());
  
  if(!HTML.good()) {
    cerr << "Could not open file: " <<  file_name << endl;
    assert(HTML.good());
  }

  //Build html head
  HTML << "<html>" << endl;   
  HTML << "<title>" << title << "</title>" << endl;
  HTML << "<head>" << endl;
  HTML << "<style type=\"text/css\">" << endl;
  HTML << header_style_string() << endl;
  HTML << "</style>" << endl;
  HTML << "</head>" << endl;
  
  diff_entry_list muts = gd.mutation_list();

  HTML << Html_Mutation_Table_String(settings, gd, muts, gd_name_list_ref, options, false, one_ref_seq, "");
  HTML << "</html>";
  HTML.close();
}

void html_compare_polymorphisms(Settings& settings, const string& file_name, const string& title, diff_entry_list& list_ref)
{
  (void)settings; //TODO: unused?

  // Create stream and confirm it's open
  ofstream HTML(file_name.c_str());
  
  if(!HTML.good()) {
    cerr << "Could not open file: " <<  file_name << endl;
    assert(HTML.good());
  }

  //Build html head
  HTML << "<html>" << endl;
  HTML << "<title>" << title << "</title>" << endl;
  HTML << "<head>" << endl;
  HTML << "<style type =\"text/css\">" << endl;
  HTML << header_style_string() << endl;
  HTML << "</style>" << endl;
  HTML << "</head>" << endl;
  HTML << html_read_alignment_table_string(list_ref, true); 
  HTML << "</html>" << endl;
  HTML.close();
}
  

void html_statistics(const string &file_name, const Settings& settings, Summary& summary, cReferenceSequences& ref_seq_info)
{  
  // Create stream and confirm it's open
  ofstream HTML(file_name.c_str());
  
  if(!HTML.good()) {
    cerr << "Could not open file: " <<  file_name << endl;
    assert(HTML.good());
  }

  //Build html head
  HTML << html_header("BRESEQ :: Summary Statistics", settings);
  HTML << breseq_header_string(settings) << endl;
  HTML << "<p>" << endl;

  //Write read file information
  //HTML << "<!-- Write fastq read file informations -->" << endl;
  HTML << "<table border=\"0\" cellspace=\"1\" cellpadding=\"5\">" << endl;
  HTML << "<tr>" << th() << th("fastq read file") << th("reads") << 
                    th("bases") << th("longest") << "</tr>" << endl;
  for(cReadFiles::const_iterator it=settings.read_files.begin(); it!=settings.read_files.end(); it++)
  {
    const AnalyzeFastq& s = summary.sequence_conversion.reads[it->m_base_name];
    
    HTML << "<tr>";
    HTML << td( a(Settings::relative_path( 
                      settings.file_name(settings.error_rates_plot_file_name, "#", it->m_base_name), settings.output_path
                                          ), 
                "errors" 
                )
              );
    HTML << td(it->m_base_name);
    HTML << td(ALIGN_RIGHT, commify(to_string(s.num_reads)));
    HTML << td(ALIGN_RIGHT, commify(to_string(s.num_bases)));
    HTML << td(ALIGN_RIGHT, to_string(s.max_read_length) + "&nbsp;bases");

    HTML << "</tr>";
  }
  
  HTML << "<tr class=\"highlight_table_row\">";
  HTML << td();
  HTML << td(b("total"));
  HTML << td(ALIGN_RIGHT , b(commify(to_string(summary.sequence_conversion.num_reads))) );
  HTML << td(ALIGN_RIGHT , b(commify(to_string(summary.sequence_conversion.num_bases))) );
  HTML << td(b(commify(to_string(summary.sequence_conversion.max_read_length))) + "&nbsp;bases");
  HTML << "</tr></table>";
  
  //Write reference sequence information
  //HTML << "<!-- Write reference sequence information -->" << endl;
  HTML << "<p>" << endl;
  HTML << "<table border=\"0\" cellspacing=\"1\" cellpadding=\"5\" >" << endl;
  HTML << "<tr>" << th() << 
                    th() << 
                    th("reference sequence") << 
                    th("length") << 
                    th(ALIGN_LEFT, "description") << 
          "</tr>" << endl;
             
  size_t total_length = 0;
  for(cReferenceSequences::iterator it=ref_seq_info.begin(); it!=ref_seq_info.end(); it++)
  {
    total_length += it->m_length;
    
    bool fragment_with_fit_coverage = (summary.unique_coverage[it->m_seq_id].nbinom_mean_parameter != 0);
    bool fragment_with_no_coverage = (summary.unique_coverage[it->m_seq_id].average == 0);

    HTML << (fragment_with_fit_coverage ? "<tr>" : "<tr class=\"gray_table_row\">");
    HTML << td( a(Settings::relative_path( 
                                          settings.file_name(settings.overview_coverage_plot_file_name, "@", it->m_seq_id), settings.output_path
                                          ), 
                  "coverage" 
                  )
               );
    
    // There may be absolutely no coverage and no graph will exist...
    if (!fragment_with_no_coverage)
    {
      HTML << td( a(Settings::relative_path( 
                                          settings.file_name(settings.unique_only_coverage_plot_file_name, "@", it->m_seq_id), settings.output_path
                                          ), 
                  "distribution" 
                  )
               ); 
    }
    else
    {
      HTML << td(nonbreaking("none aligned"));
    }
    
    
    HTML << td(it->m_seq_id);
    HTML << td(ALIGN_RIGHT, commify(to_string(it->m_length)));
    HTML << td(it->m_definition);
    HTML << "</tr>";
  }  
  
  HTML << "<tr class=\"highlight_table_row\">";
  HTML << td();
  HTML << td();
  HTML << td(b("total"));
  HTML << td(ALIGN_RIGHT, b(commify(to_string(total_length))) );
  HTML << td();
  HTML << "</tr>" << endl;

// # //TODO @JEB Summary
// #   ## junction only reference sequences
// #   foreach my $seq_id (@{$ref_seq_info->{junction_only_seq_ids}})
// #   {
// #     my $c = $summary->{sequence_conversion}->{reference_sequences}->{$seq_id};
// #     print HTML Tr(
// #       td({-colspan=>"2", -align=>"center"}, "junction&nbsp;only"), 
// #       td($seq_id), 
// #       td({-align=>"right"},commify($c->{length})), 
// #       td($c->{definition})
// #     );
// #     $total_length+= $c->{length};
// #   }
// #   
// #   print HTML end_table();
  HTML << "</table>" << endl;  
  
  // Write Execution Times
  const vector<ExecutionTime>& times = settings.execution_times;
  // HTML << "<!-- Write Times -->" << endl;
  HTML << "<p>"  << endl;
  HTML << h1("Execution Times") << endl;
  HTML << start_table("width=\"100%\" border=\"1\" cellspacing=\"0\" cellpadding=\"3\"") << endl;
  HTML << "<tr>" << th("Step") << th("Start") << th("End") << th("Elapsed") << "</tr>" << endl; 
  double total_time_elapsed = 0; 

  for (vector<ExecutionTime>::const_iterator itr = times.begin(); itr != times.end(); itr ++) {  
    const ExecutionTime& t = (*itr);
    
    if (t._message.empty()) continue;

    HTML << "<tr>";
    HTML << td(t._message);
    HTML << td(nonbreaking(t._formatted_time_start));
    HTML << td(nonbreaking(t._formatted_time_end));
    HTML << td(nonbreaking(t._formatted_time_elapsed));
    HTML << "</tr>" << endl; 

    total_time_elapsed += t._time_elapsed;    
  }

  HTML << "<tr class=\"highlight_table_row\">"<< endl;
  HTML << "<td colspan=\"3\" >" << b("Total") << "</td>" << endl;
  HTML << "<td>" << (b(nonbreaking(Settings::elapsedtime2string(total_time_elapsed)))) << "</td>" << endl;
  HTML << "</tr>" << endl;

  HTML << "</table>";
  HTML << html_footer();
  HTML.close();
}

string breseq_header_string(const Settings& settings)
{
  stringstream ss(ios_base::out | ios_base::app);
  
  //copy over the breseq_graphic which we need if it doesn't exist - don't show command
  if (!file_exists(settings.breseq_small_graphic_to_file_name.c_str())) {
    copy_file(settings.breseq_small_graphic_from_file_name, settings.breseq_small_graphic_to_file_name);
  }
  
  ss << "<table width=\"100%\" border=\"0\" cellspacing=\"0\" cellpadding=\"3\">" << endl;
  ss << "<tr>" << endl;
  ss << td(a(settings.website, img(Settings::relative_path(settings.breseq_small_graphic_to_file_name, settings.output_path))));
  ss << endl;
  ss << start_td("width=\"100%\"") << endl;
  ss << settings.byline << endl;
  ss << "<br>";
  ss << a(Settings::relative_path(settings.index_html_file_name, settings.output_path), "mutation predictions"); 
  ss << " | " << endl;
  ss << a(Settings::relative_path(settings.marginal_html_file_name, settings.output_path), "marginal predictions");
  ss << " | " << endl;
  ss << a(Settings::relative_path(settings.summary_html_file_name, settings.output_path), "summary statistics");
  ss << " | " << endl;
  ss << a(Settings::relative_path(settings.final_genome_diff_file_name, settings.output_path), "genome diff");
  ss << " | " << endl;
  ss << a(Settings::relative_path(settings.log_file_name, settings.output_path), "command line log");
  ss << endl;
  ss << "</td></tr></table>" << endl;
  return ss.str();
}


string html_genome_diff_item_table_string(const Settings& settings, genome_diff& gd, diff_entry_list& list_ref)
{
  if(list_ref.empty()) return "";

  diff_entry& first_item = *list_ref.front();
  //mutation
  if(first_item._type < NOT_MUTATION)
  {
    return Html_Mutation_Table_String(settings, gd, list_ref); 
  }
  //evidence
  else
  {
    if(first_item._type == MC)
    {
      return html_missing_coverage_table_string(list_ref, true);
    }
    else if(first_item._type == RA)
    {
      return html_read_alignment_table_string(list_ref, true);
    }
    else if(first_item._type == JC)
    {
      return html_new_junction_table_string(list_ref,false);
    }
  }  
  return "";
}

/*-----------------------------------------------------------------------------
 *  FORMATTED_MUTATION_ANNOTATION
 *-----------------------------------------------------------------------------*/
string formatted_mutation_annotation(const diff_entry& mut)
{
  stringstream ss;

  // additional formatting for some variables
  if((mut.entry_exists("snp_type")) && (mut["snp_type"] != "intergenic") &&
     (mut["snp_type"] != "noncoding") && (mut["snp_type"] != "pseudogene"))
  {    
    ss << mut["aa_ref_seq"] << mut["aa_position"] << mut["aa_new_seq"];

    string codon_ref_seq = to_underline_red_codon(mut, "codon_ref_seq");
    string codon_new_seq = to_underline_red_codon(mut, "codon_new_seq");
    
    ss << "&nbsp;" << codon_ref_seq << "&rarr;" << codon_new_seq << "&nbsp;";  
  }
  else // mut[SNP_TYPE] == "NC"
  {
    ss << nonbreaking(mut[GENE_POSITION]); 
  }
  return ss.str(); 
}

/*-----------------------------------------------------------------------------
 *  Helper function for formatted_mutation_annotation
 *-----------------------------------------------------------------------------*/
string to_underline_red_codon(const diff_entry& mut, const string& codon_key)
{
  if (!mut.entry_exists(codon_key) || 
      !mut.entry_exists("codon_position") ||
      mut["codon_position"] == "") {
    return "";
  }

  stringstream ss; //!< codon_string
  
  string codon_ref_seq = mut[codon_key];
  for (size_t i = 0; i < codon_ref_seq.size(); i++) {

    if (i == from_string(mut["codon_position"])) {
      ss << font("class=\"mutation_in_codon\"", codon_ref_seq.substr(i,1));
    }
    else 
    {
      ss << codon_ref_seq[i];
    }
  }
  return ss.str();
}

string html_read_alignment_table_string(diff_entry_list& list_ref, bool show_reject_reason, const string& title, const string& relative_link)
{
  stringstream ss; //!< Main build object for function
  stringstream ssf; //!< Used for formatting strings
  
  ss << start_table("border=\"0\" cellspacing=\"1\", cellpadding=\"3\"") << endl;
  
  // Evidence hyperlinks will be the first column if they exist
  bool link;
  if (list_ref.front().get() != 0 && (*list_ref.front()).entry_exists(_EVIDENCE_FILE_NAME)) {
    link = true;
  } else {
    link = false;
  }

  //Determine Number of Columns in Table
  size_t total_cols = link ? 11 : 10;
  
  //Create Column Titles
  //seq_id/position/change/freq/score/cov/annotation/genes/product
  if (title != "") {
    ss << tr(th("colspan=\"" + to_string(total_cols) + 
                "\" align=\"left\" class=\"read_alignment_header_row\"", title)) << endl;
    ss << "<tr>" << endl;
  }
  
  if (link) {
    ss << th("&nbsp;") << endl;
  }
  ss << th("seq&nbsp;id") << endl;
  ss << th("colspan=\"2\"", "position") << endl;
  ss << th("change")     << endl <<
        th("freq")       << endl <<
        th("score")      << endl <<
        th("cov")        << endl <<
        th("annotation") << endl <<
        th("genes")       << endl;
  
  ss << th("width=\"100%\"", "product") << endl;
  ss << "</tr>" << endl;
  
  //Loop through list_ref to build table rows
  for (diff_entry_list::iterator itr = list_ref.begin();
       itr != list_ref.end(); itr ++) {  
    diff_entry& c = **itr;
    
    bool is_polymorphism = false;
    if (c.entry_exists(FREQUENCY) && (from_string<double>(c[FREQUENCY]) != 1.0)) {
      is_polymorphism = true;
    }
    
    string row_class = "normal_table_row";
    
    if (is_polymorphism) {
      row_class = "polymorphism_table_row";
    }
    
    // Start building a single table row
    ss << start_tr("class=\"" + row_class + "\"");
    if (link) {
      ss << td(a(relative_link + c[_EVIDENCE_FILE_NAME], "*"));
    }

    string fisher_p_value;
    if (c.entry_exists("fisher_strand_p_value") &&
       (c["fisher_strand_p_value"] != "ND")) 
    {
      ssf.precision(1);
      ssf << scientific << from_string<double>(c["fisher_strand_p_value"]); //TODO Confirm
      fisher_p_value = nonbreaking("&nbsp;(" + ssf.str() + ")");
      //Clear Formated String Stream
      ssf.str("");
      ssf.clear();
     }

    ss << td(ALIGN_CENTER, nonbreaking(c[SEQ_ID]));
    ss << td(ALIGN_RIGHT, commify(c["position"]));
    ss << td(ALIGN_RIGHT, c["insert_position"]);
    ss << td(ALIGN_CENTER, c["ref_base"] + "&rarr;" + c["new_base"]); // "Change" Column
    ssf.width(4);
    ssf.precision(1);
    ssf << fixed << from_string<double>(c["frequency"]) * 100 << "%" << endl;
    ss << td(ALIGN_RIGHT, ssf.str());
    //Clear formated string stream
    ssf.str("");
    ssf.clear();

    if (is_polymorphism) {
      // display extra score data for polymorphisms...
      string log_fisher = "999";
      string log_ks = "999";
         
      if (c.entry_exists(FISHER_STRAND_P_VALUE) &&
          from_string<double>(c[FISHER_STRAND_P_VALUE]) > 0 )
        log_fisher = to_string(log(from_string<double>(c[FISHER_STRAND_P_VALUE])));

      if (c.entry_exists(KS_QUALITY_P_VALUE) &&
          from_string<double>(c[KS_QUALITY_P_VALUE]) > 0 ) 
        log_ks = to_string(log(from_string<double>(c[KS_QUALITY_P_VALUE]))/log(10));

      ssf.precision(1);
      ssf << fixed << c["polymorphism_quality"] << " " <<
                      log_fisher << " " <<
                      log_ks;
      ss << td(ALIGN_RIGHT, nonbreaking(ssf.str()));
      //Clear formated string stream
      ssf.str("");
      ssf.clear();

    }

    else {
      ssf.precision(1);
      ssf << fixed << c["quality"] << endl;
      ss << td(ALIGN_RIGHT, nonbreaking(ssf.str()));
      ssf.str("");
      ssf.clear();
    }  
    // Build "cov" column value
    vector<string> temp_cov = split(c[TOT_COV], "/");
    string top_cov = temp_cov[0];
    string bot_cov = temp_cov[1];
    string total_cov = to_string(from_string<uint32_t>(top_cov) + 
                                 from_string<uint32_t>(bot_cov));
    ss << td(ALIGN_CENTER, total_cov);// "Cov" Column
    ss << td(ALIGN_CENTER, nonbreaking(formatted_mutation_annotation(c))); //"Annotation" Column
    ss << td(ALIGN_CENTER, i(nonbreaking(c[GENE_NAME])));
    ss << td(ALIGN_LEFT, htmlize(c[GENE_PRODUCT]));
    ss << "</tr>" << endl;

    if (show_reject_reason) 
    {
      vector<string> reject_reasons = c.get_reject_reasons();
      for (vector<string>::iterator itr = reject_reasons.begin(); itr != reject_reasons.end(); itr ++) 
      {  
        string& reject = (*itr);
       ss << tr("class=\"reject_table_row\"", 
                td("colspan=\"" + to_string(total_cols) + "\"",
                   "Rejected: " + decode_reject_reason(reject)));
      }
    
      /* Fisher Strand Test */
      if (c.entry_exists("fisher_strand_p_value")) 
      {
        ssf.precision(2);
        ssf << scientific << from_string<float>(c["fisher_strand_p_value"]);
        string fisher_strand_p_value = ssf.str();
        
        //Clear formated string stream
        ssf.str("");
        ssf.clear();

        ss << tr("class=\"information_table_row\"", 
                 td("colspan=\"" + to_string(total_cols) + "\"",
                    "Strands of reads supporting (+/-):&nbsp;&nbsp;" +
                    b("new") + " base " + "(" + c[NEW_COV] + ")" + ":&nbsp;&nbsp;" +
                    b("ref") + " base " + "(" + c[REF_COV] + ")" + ":&nbsp;&nbsp;" +
                    b("total") + " (" + c[TOT_COV] + ")")); 
        ss << tr("class=\"information_table_row\"", 
                 td("colspan=\"" + to_string(total_cols) + "\"",
                    "Fisher's exact test strand distribution" +
                    i("p") + "-value = " +fisher_strand_p_value));
      } //end fisher_strand_p_value

      /* Kolmogorov-Smirnov Test */
      if (c.entry_exists("ks_quality_p_value")) {
      ssf.precision(2);
      ssf << scientific << (c.entry_exists(KS_QUALITY_P_VALUE) ? 
        from_string<float>(c["ks_quality_p_value"]) :
        0);
      string ks_quality_p_value = ssf.str();
      
      //Clear formated string stream
      ssf.str("");
      ssf.clear();

      ss << tr("class=\"information_table_row\"", 
               td("colspan=\"" + to_string(total_cols) + "\"",
                  " Kolmogorov-Smirnov test that lower quality scores support polymorphism than reference " +
                  i("p") + "-value = " +ks_quality_p_value));
      } //end ks_quality_p_value
    } // end show_reject_reason
  } // end list_ref loop

  ss << "</table>" << endl;
  return ss.str();
}

string html_missing_coverage_table_string(diff_entry_list& list_ref, bool show_reject_reason, const string& title, const string& relative_link)
{
  ASSERT(list_ref.front().get());
  
  stringstream ss; //!< Main Build Object in Function
  
  ss << endl;
  ss << start_table("border=\"0\" cellspacing=\"1\" cellpadding=\"3\" width=\"100%\"") << endl;
  
  bool coverage_plots = list_ref.front()->entry_exists(_EVIDENCE_FILE_NAME);
    
  bool link = ( list_ref.front()->entry_exists(_SIDE_1_EVIDENCE_FILE_NAME) && 
                list_ref.front()->entry_exists(_SIDE_2_EVIDENCE_FILE_NAME) );
  
  size_t total_cols = link ? 11 : 8;

  if (title != "") {
    ss << "<tr>" << th("colspan=\"" + to_string(total_cols) + "\" align=\"left\" class=\"missing_coverage_header_row\"", title) << "</tr>" << endl;   
  }
  
  ss << "<tr>";
    
  if (link) 
  {
    ss << th("&nbsp;") <<  th("&nbsp;");
    if (coverage_plots) 
    {
      ss << th("&nbsp;");
    }
  }

  ss << th("seq&nbsp;id") << endl <<
        th("start")       << endl <<
        th("end")         << endl <<
        th("size")        << endl <<
        th("&larr;cov")   << endl <<
        th("cov&rarr;")   << endl <<
        th("gene")        << endl;
  
  ss << th("width=\"100%\"", "description") << endl;
  ss << "</tr>" << endl;

  for (diff_entry_list::iterator itr = list_ref.begin(); itr != list_ref.end(); itr ++) 
  {  
    diff_entry& c =  **itr;

    ss << "<tr>" << endl;
    if (link) 
    {
      ss << td(a(relative_link + c[_SIDE_1_EVIDENCE_FILE_NAME], "*")) << endl;
      ss << td(a(relative_link + c[_SIDE_2_EVIDENCE_FILE_NAME], "*")) << endl; 
      
      if (coverage_plots) 
        ss << td(a(relative_link + c[_EVIDENCE_FILE_NAME], "&divide;")) << endl;
    }

    string start = c[START];
    if (from_string<uint32_t>(c[START_RANGE]) > 0) 
    {
      start += "–" + 
        to_string(from_string<uint32_t>(c[START]) + 
                  from_string<uint32_t>(c[START_RANGE]));
    }
    string end = c[END];
    if (from_string<uint32_t>(c[END_RANGE]) > 0) 
    {
       end += "–" + 
         to_string(from_string<uint32_t>(c[END]) -
                   from_string<uint32_t>(c[END_RANGE]));
    }

    string size = to_string(from_string<uint32_t>(c[END]) - from_string<uint32_t>(c[START]) + 1);
      
    if (
        (from_string<uint32_t>(c[END_RANGE]) > 0) 
        || (from_string<uint32_t>(c[START_RANGE]) > 0)
        ) 
    {
     
      uint32_t size_value = 
      from_string<uint32_t>(c[END]) - 
      from_string<uint32_t>(c[START]) + 1 -
      from_string<uint32_t>(c[END_RANGE]) -
      from_string<uint32_t>(c[START_RANGE]);
     
      size = to_string(size_value) + "–" + size; 
    }
     
    ss << td(nonbreaking(c[SEQ_ID])) << endl;
    ss << td(ALIGN_RIGHT, nonbreaking(start)) << endl;
    ss << td(ALIGN_RIGHT, nonbreaking(end)) << endl;
    ss << td(ALIGN_RIGHT, nonbreaking(size)) << endl;
    ss << td(ALIGN_CENTER, nonbreaking(c[LEFT_OUTSIDE_COV] + " [" + c[LEFT_INSIDE_COV] + "]")) <<endl;
    ss << td(ALIGN_CENTER, nonbreaking( "[" + c[RIGHT_INSIDE_COV] + "] " + c[RIGHT_OUTSIDE_COV])) << endl;
    ss << td(ALIGN_CENTER, i(nonbreaking(c[GENE_NAME]))) << endl;
    ss << td(ALIGN_LEFT, htmlize(c[GENE_PRODUCT])) << endl;
    ss << "</tr>" << endl;

    if (show_reject_reason && c.entry_exists(REJECT)) 
    {
      vector<string> reject_reasons = c.get_reject_reasons();
      for (vector<string>::iterator itr = reject_reasons.begin(); itr != reject_reasons.end(); itr ++) 
      {  
        string& reject = (*itr);
        ss << tr("class=\"reject_table_row\"", 
                 td("colspan=\"" + to_string(total_cols) + "\"",
                    "Rejected: " + decode_reject_reason(reject)));  
      }
    }
  }
  ss << "</table>" << endl;
  return ss.str();
}

string html_new_junction_table_string(diff_entry_list& list_ref, bool show_reject_reason, const string& title, const string& relative_link)
{
  stringstream ss; //!<< Main Build Object for Function
  diff_entry& test_item = *list_ref.front();

  bool link = (test_item.entry_exists(_SIDE_1_EVIDENCE_FILE_NAME) &&
               test_item.entry_exists(_SIDE_2_EVIDENCE_FILE_NAME) &&
               test_item.entry_exists(_NEW_JUNCTION_EVIDENCE_FILE_NAME));

  ss << start_table("border=\"0\" cellspacing=\"1\" cellpadding=\"3\"") << endl;
  size_t total_cols = link ? 10 : 8;
  
  if (title != "") {
    ss << tr(th("colspan=\"" + to_string(total_cols) + "\" align=\"left\" class=\"new_junction_header_row\"", title)) << endl;
  }
// #     
// #   #####################
// #   #### HEADER LINE ####
// #   #####################
  ss << "<!-- Header Lines for New Junction -->" << endl; 
  ss << "<tr>" << endl;
    
  if (link) {
    ss << th("colspan=\"2\"", "&nbsp;") << endl;
  }
  ss << th("seq&nbsp;id") << endl <<
        th("position")    << endl <<
        th("overlap")     << endl <<
        th("reads")       << endl <<
        th("score")       << endl <<
        th("annotation")  << endl <<
        th("gene")        << endl;
  
  ss << th("width=\"100%\"","product") << endl;
  ss << "</tr>" << endl;
  ss << endl;
// #   
// #   ####################
// #   #### ITEM LINES ####
// #   ####################
  ss << "<!-- Item Lines for New Junction -->" << endl;
// #   
// #   ## the rows in this table are linked (same background color for every two)
  uint32_t row_bg_color_index = 0; 
   
  for (diff_entry_list::iterator itr = list_ref.begin(); itr != list_ref.end(); itr ++) 
  {  
    diff_entry& c = **itr;
// #     ##############
// #     ### Side 1 ###
// #     ##############
    ss << "<!-- Side 1 Item Lines for New Junction -->" << endl;
    
    string key = "side_1";
    string annotate_key = "junction_" + c[key + "_annotate_key"];
    ss << start_tr("class=\"mutation_table_row_" + to_string(row_bg_color_index) +"\"") << endl;

     if (link) {
      ss << td("rowspan=\"2\"", 
              a(relative_link + c[_NEW_JUNCTION_EVIDENCE_FILE_NAME], "*" )) << endl;
     }

     { // Begin hiding data for side 1

      if (link) {   
        ss << td("rowspan=\"1\"", 
                a(relative_link + c[_SIDE_1_EVIDENCE_FILE_NAME], "?")) << endl;
      }
      ss << td("rowspan=\"1\" class=\"" + annotate_key + "\"",
            nonbreaking(c[key + "_seq_id"])) << endl;
       
      if (from_string<int32_t>(c[key + "_strand"]) == 1) { 
        ss << td("align=\"center\" class=\"" + annotate_key +"\"",
                c[key + "_position"] + "&nbsp;=");
      } else {
        ss << td("align=\"center\" class=\"" + annotate_key +"\"",
                "=&nbsp;" + c[key + "_position"]);
      }
      ss << td("rowspan=\"2\" align=\"center\"", c["overlap"]) << endl;
      ss << td("rowspan=\"2\" align=\"center\"", c["total_reads"]) << endl;
      ss << td("rowspan=\"2\" align=\"center\"", c["pos_hash_score"]) << endl;
      ss << td("align=\"center\" class=\"" + annotate_key + "\"", 
              nonbreaking(c["_" + key + GENE_POSITION])) << endl;
      ss << td("align=\"center\" class=\"" + annotate_key + "\"", 
              nonbreaking(c["_" + key + GENE_NAME])) << endl;
      ss << td("class=\"" + annotate_key + "\"",
              htmlize(c["_" + key + GENE_PRODUCT])) << endl;  
// #     }
    } // End hiding data for side 1
    ss << "</tr>" << endl;


// #     ##############
// #     ### Side 2 ###
// #     ##############
    ss << "<!-- Side 2 Item Lines for New Junction -->" << endl;
    key = "side_2";
    annotate_key = "junction_" + c[key + "_annotate_key"];
    ss << start_tr("class=\"mutation_table_row_" + 
                  to_string(row_bg_color_index) + "\"") << endl;

    { //Begin hiding data for side 2
      if (link) {
        ss << td("rowspan=\"1\"", 
                a(relative_link +  c[_SIDE_2_EVIDENCE_FILE_NAME], "?"));
      }
      ss << td("rowspan=\"1\" class=\"" + annotate_key + "\"",
              nonbreaking(c[key + "_seq_id"])) << endl;

      if (from_string<int32_t>(c[key + "_strand"]) == 1) { 
        ss << td("align=\"center\" class=\"" + annotate_key +"\"",
                c[key + "_position"] + "&nbsp;=") << endl;
      } else {
        ss << td("align=\"center\" class=\"" + annotate_key +"\"",
                "=&nbsp;" + c[key + "_position"]) << endl;
      } 
      ss << td("align=\"center\" class=\"" + annotate_key + "\"",
              nonbreaking(c["_" + key + GENE_POSITION])) << endl;
      ss << td("align=\"center\" class=\"" + annotate_key + "\"",
              i(nonbreaking(c["_" + key + GENE_NAME]))) << endl;
      ss << td("class=\"" + annotate_key + "\"",
              htmlize(c["_" + key + GENE_PRODUCT])) << endl;
    } //end hiding data for side 2
    
  ss << "</tr>" << endl;

  if (show_reject_reason && c.entry_exists("reject")) {
    genome_diff gd;

    vector<string> reject_reasons = c.get_reject_reasons();
    
    for (vector<string>::iterator itr = reject_reasons.begin();
         itr != reject_reasons.end(); itr++) {
      string& reject(*itr);
    
      ss << tr("class=\"reject_table_row\"",
              td("colspan=\"" + to_string(total_cols) + "\"",
                "Rejected: " + decode_reject_reason(reject))) << endl;
    }

  }
  row_bg_color_index = (row_bg_color_index+1) % 2;//(row_bg_color_index) % 2; 

  }// End list_ref Loop
  ss << "</table>" << endl;
  return ss.str();
}

string decode_reject_reason(const string& reject)
{
  
  if (reject == "NJ")
  {
    return "Position hash score below cutoff.";
  }
  else if (reject == "EVALUE")
  {
    return "E-value exceeds prediction threshold.";
  }
  else if (reject == "STRAND")
  {
    return "Prediction not supported by reads on both strands.";
  }
  else if (reject == "FREQ")
  {
    return "Prediction has fr==uency below cutoff threshold.";
  }  
  else if (reject == "COV")
  {
    return "Prediction has coverage below cutoff threshold.";
  }
  else if (reject == "BIAS_P_VALUE")
  {
    return "Prediction has biased strand and/or quality scores supporting polymorphism.";
  }
  else if (reject == "KS_QUALITY_P_VALUE")
  {
    return "Prediction has significantly lower quality scores supporting polymorphism compared to reference.";
  }
  else if (reject == "KS_QUALITY_P_VALUE_UNUSUAL_POLY")
  {
    return "Prediction has biased quality score distribution for polymorphism bases.";
  }
  else if (reject == "KS_QUALITY_P_VALUE_UNUSUAL_REF")
  {
    return "Prediction has biased quality score distribution for new bases.";
  }
  else if (reject == "KS_QUALITY_P_VALUE_UNUSUAL_NEW")
  {
    return "Prediction has biased quality score distribution for ref bases.";
  }
  else if (reject == "KS_QUALITY_P_VALUE_UNUSUAL_ALL")
  {
    return "Prediction has biased quality score distribution for all bases.";
  }
  else if (reject == "FISHER_STRAND_P_VALUE")
  {
    return "Prediction has biased read strand distribution supporting polymorphism.";
  }  
  else if (reject == "POLYMORPHISM_STRAND")
  {
    return "Polymorphism prediction not supported by minimum number of reads on both strands.";
  }
  else if (reject == "POLYMORPHISM_FREQUENCY_CUTOFF")
  {
    return "Polymorphism does not pass frequency cutoff.";
  }
  else if (reject == "HOMOPOLYMER_STRETCH")
  {
    return "Polymorphism is in a homopolymer stretch.";
  }
  
  return "";
}
// # 

/*
 * =====================================================================================
 *        Class:  Evidence_Files
 *  Description:  
 * =====================================================================================
 */
Evidence_Files::Evidence_Files(const Settings& settings, genome_diff& gd)
{  
  // Fasta and BAM files for making alignments.
  string reference_bam_file_name = settings.reference_bam_file_name;
  string reference_fasta_file_name = settings.reference_fasta_file_name;

  // hybrids use different BAM files for making the alignments!!!
  string junction_bam_file_name = settings.junction_bam_file_name;
  string junction_fasta_file_name = settings.candidate_junction_fasta_file_name;

  // We make alignments of two regions for deletions: upstream and downstream edges.
  diff_entry_list items_MC = gd.show_list(make_list<Type>(MC));
  //cerr << "Number of MC evidence items: " << items_MC.size() << endl;
  
  for (diff_entry_list::iterator itr = items_MC.begin(); itr != items_MC.end(); itr ++) 
  {  
    diff_entry_ptr item(*itr);
     
    diff_entry_ptr parent_item(gd.parent(*item));
    if (parent_item.get() == NULL)
      parent_item = *itr;

   add_evidence(_SIDE_1_EVIDENCE_FILE_NAME,
                item,
                parent_item,
                make_map<string,string>
                (BAM_PATH, reference_bam_file_name)
                (FASTA_PATH, reference_fasta_file_name)
                (PREFIX, "MC_SIDE_1")
                (SEQ_ID, (*item)[SEQ_ID])            
                (START, to_string(from_string<uint32_t>((*item)[START]) - 1))
                (END,  to_string(from_string<uint32_t>((*item)[START]) - 1)));

    add_evidence(_SIDE_2_EVIDENCE_FILE_NAME,
                item,
                parent_item,
                make_map<string,string>
                (BAM_PATH, reference_bam_file_name)
                (FASTA_PATH, reference_fasta_file_name)
                (PREFIX, "MC_SIDE_2")
                (SEQ_ID, (*item)[SEQ_ID])            
                (START, to_string(from_string<uint32_t>((*item)[END]) + 1))
                (END,  to_string(from_string<uint32_t>((*item)[END]) + 1)));
    
    add_evidence(_EVIDENCE_FILE_NAME,
                item,
                parent_item,
                make_map<string,string>
                (BAM_PATH, reference_bam_file_name)
                (FASTA_PATH, reference_fasta_file_name)
                (PREFIX, "MC_PLOT")
                (SEQ_ID, (*item)[SEQ_ID])            
                (START, (*item)[START])
                (END,  (*item)[END])
                (PLOT, (*item)[_COVERAGE_PLOT_FILE_NAME])); // filled by draw_coverage
    
  } // mc_item list
  
  diff_entry_list items_SNP_INS_DEL_SUB = gd.show_list(make_list<Type>(SNP)(INS)(DEL)(SUB));
  //cerr << "Number of SNP_INS_DEL_SUB evidence items: " << items_SNP_INS_DEL_SUB.size() << endl;

  for (diff_entry_list::iterator itr = items_SNP_INS_DEL_SUB.begin(); itr != items_SNP_INS_DEL_SUB.end(); itr ++) 
  {  
    diff_entry_ptr item = *itr;
    diff_entry_list mutation_evidence_list = gd.mutation_evidence_list(*item);

    // #this reconstructs the proper columns to draw
    uint32_t start = from_string<uint32_t>((*item)[POSITION]);
    uint32_t end = start;
    uint32_t insert_start = 0;
    uint32_t insert_end = 0;

    if (item->_type == INS) 
    {
      insert_start = 1;
      insert_end = (*item)[NEW_SEQ].size();
    }
    else if (item->_type == DEL) 
    {
      bool has_ra_evidence = false;
      for (diff_entry_list::iterator itr = mutation_evidence_list.begin(); itr != mutation_evidence_list.end(); itr ++) 
      {  
        diff_entry& evidence_item = **itr;
        if (evidence_item._type == RA) has_ra_evidence = true;
      }
      if(!has_ra_evidence) continue;  

      end = start + from_string<uint32_t>((*item)[SIZE]) - 1;
    }

    else if (item->_type == SUB ) 
    {
      end = start + (*item)[NEW_SEQ].size() - 1;
    }

    add_evidence(_EVIDENCE_FILE_NAME,
                 item,
                 item,
                 make_map<string,string>
                 (BAM_PATH, reference_bam_file_name)
                 (FASTA_PATH, reference_fasta_file_name)
                 (SEQ_ID, (*item)[SEQ_ID])            
                 (START, to_string(start))
                 (END,  to_string(end))
                 (INSERT_START, to_string(insert_start))
                 (INSERT_END, to_string(insert_end))
                 (PREFIX, to_string((*item)._type)));


    /* Add evidence to RA items as well */
    for (diff_entry_list::iterator itr = mutation_evidence_list.begin(); itr != mutation_evidence_list.end(); itr ++) 
    {  
      diff_entry& evidence_item = **itr;
      if (evidence_item._type != RA) continue;
      evidence_item[_EVIDENCE_FILE_NAME] = (*item)[_EVIDENCE_FILE_NAME];  
    }
  }
  

  // Still create files for RA evidence that was not good enough to predict a mutation from
  diff_entry_list items_RA = gd.filter_used_as_evidence(gd.show_list(make_list<Type>(RA)));
  //cerr << "Number of RA evidence items: " << items_RA.size() << endl;

  for (diff_entry_list::iterator itr = items_RA.begin(); itr != items_RA.end(); itr ++) 
  {  
    diff_entry_ptr item = *itr;

    add_evidence(_EVIDENCE_FILE_NAME,
                 item,
                 item,
                 make_map<string,string>
                 (BAM_PATH, reference_bam_file_name)
                 (FASTA_PATH, reference_fasta_file_name)
                 (SEQ_ID, (*item)[SEQ_ID])            
                 (START, (*item)[POSITION])
                 (END, (*item)[POSITION])
                 (INSERT_START, (*item)[INSERT_POSITION])
                 (INSERT_END, (*item)[INSERT_POSITION])
                 (PREFIX, to_string(item->_type)));
  }
  // This additional information is used for the complex reference line.
  // Note that it is completely determined by the original candidate junction sequence 
  // positions and overlap: alignment_pos and alignment_overlap.
  
  diff_entry_list items_JC = gd.show_list(make_list<Type>(JC));
  //cerr << "Number of JC evidence items: " << items_JC.size() << endl;

  for (diff_entry_list::iterator itr = items_JC.begin(); itr != items_JC.end(); itr ++) 
  {  
    diff_entry_ptr item = *itr;

    diff_entry_ptr parent_item(gd.parent(*item));
    if (parent_item.get() == NULL)
      parent_item = *itr;

    uint32_t start = 0;
    uint32_t end = 0;

    if (from_string<int32_t>((*item)[ALIGNMENT_OVERLAP]) == 0) 
    {
      start = from_string<uint32_t>((*item)[FLANKING_LEFT]);
      end = from_string<uint32_t>((*item)[FLANKING_LEFT]) + 1;
    }
    else if (from_string <int32_t>((*item)[ALIGNMENT_OVERLAP]) > 0) 
    {
      start = from_string<uint32_t>((*item)[FLANKING_LEFT]) + 1;
      end = from_string<uint32_t>((*item)[FLANKING_LEFT]) + 
            from_string<int32_t>((*item)[ALIGNMENT_OVERLAP]);
    }
    else //if (from_string <uint32_t>((*item)[ALIGNMENT_OVERLAP]) > 0) 
    {
      start = from_string<uint32_t>((*item)[FLANKING_LEFT]) + 1;
      end = from_string<uint32_t>((*item)[FLANKING_LEFT]) - from_string<int32_t>((*item)[ALIGNMENT_OVERLAP]);
    }
    
    add_evidence(_NEW_JUNCTION_EVIDENCE_FILE_NAME,
                  item,
                  parent_item,
                  make_map<string,string>
                 (BAM_PATH, junction_bam_file_name)
                 (FASTA_PATH, junction_fasta_file_name)
                 (SEQ_ID, (*item)["key"])
                 (START, to_string(start))
                 (END, to_string(end))
                 (PREFIX, "JC")
                 (ALIGNMENT_EMPTY_CHANGE_LINE, "1")
                 );


    // this is the flagship file that we show first when clicking on evidence from a mutation...
    (*item)[_EVIDENCE_FILE_NAME] = (*item)[_NEW_JUNCTION_EVIDENCE_FILE_NAME];
    string side_1_key_str = "JC_SIDE_1_" + (*item)[SIDE_2_SEQ_ID] + "_" + (*item)[SIDE_2_POSITION] + "_" + (*item)[SIDE_2_POSITION];
    add_evidence(_SIDE_1_EVIDENCE_FILE_NAME,
                 item,
                 parent_item,
                 make_map<string,string>
                 (BAM_PATH, reference_bam_file_name)
                 (FASTA_PATH, reference_fasta_file_name)
                 (SEQ_ID, (*item)[SIDE_1_SEQ_ID])            
                 (START, (*item)[SIDE_1_POSITION])
                 (END, (*item)[SIDE_1_POSITION])
                 (PREFIX, side_1_key_str)
                 ); 

    string side_2_key_str = "JC_SIDE_2_" + (*item)[SIDE_1_SEQ_ID] + "_" + (*item)[SIDE_1_POSITION] + "_" + (*item)[SIDE_1_POSITION];
    add_evidence(_SIDE_2_EVIDENCE_FILE_NAME,
                 item,
                 parent_item,
                 make_map<string,string>
                 (BAM_PATH, reference_bam_file_name)
                 (FASTA_PATH, reference_fasta_file_name)
                 (SEQ_ID, (*item)[SIDE_2_SEQ_ID])            
                 (START, (*item)[SIDE_2_POSITION])
                 (END, (*item)[SIDE_2_POSITION])
                 (PREFIX, side_2_key_str)
                 );
  }

  // now create evidence files
  create_path(settings.evidence_path);
  //cerr << "Total number of evidence items: " << evidence_list.size() << endl;

  for (vector<Evidence_Item>::iterator itr = evidence_list.begin(); itr != evidence_list.end(); itr ++) 
  {  
    Evidence_Item& e = (*itr);

    if (settings.verbose) {
      cerr << "Creating evidence file: " + e[FILE_NAME] << endl;   
    }
    html_evidence_file(settings, gd, e);
  }
}

/*-----------------------------------------------------------------------------
 *  Helper Function For Create_Evidence_Files()
 *-----------------------------------------------------------------------------*/

string Evidence_Files::html_evidence_file_name(Evidence_Item& evidence_item)
{
  
  //set up the file name
  string s = evidence_item[PREFIX];
  s += "_";
  s += evidence_item[SEQ_ID];
  s += "_";
  s += evidence_item[START];
  
  if (evidence_item.entry_exists(INSERT_START))
  {
    s += ".";
    s += evidence_item[INSERT_START];
  }
  
  s += "_";
  s += evidence_item[END];

  if (evidence_item.entry_exists(INSERT_END))
  {
    s += ".";
    s += evidence_item[INSERT_END];
  }
  s += "_alignment.html";

  return s;
}
  
  
void Evidence_Files::add_evidence(const string& evidence_file_name_key, diff_entry_ptr item,
                                  diff_entry_ptr parent_item, map<string,string> fields)
{
  Evidence_Item evidence_item;
  evidence_item._fields = fields;
  evidence_item.item = item;
  evidence_item.parent_item = parent_item;
  evidence_item[FILE_NAME] = html_evidence_file_name(evidence_item);
  
  // this is added to the actual genome diff entry so that we know where to link
  (*item)[evidence_file_name_key] = evidence_item[FILE_NAME];
  
  evidence_list.push_back(evidence_item);
}
/*-----------------------------------------------------------------------------
 *  Helper Function For Create_Evidence_Files()
 *-----------------------------------------------------------------------------*/
string Evidence_Files::file_name(Evidence_Item& evidence_item)
{
  stringstream ss(ios_base::out | ios_base::app);

  ss << evidence_item[PREFIX];
  ss << "_" << evidence_item[SEQ_ID];
  ss << "_" << evidence_item[START];
  ss << evidence_item.entry_exists(INSERT_START) ? "." + evidence_item[INSERT_START] : "";
  ss << "_" << evidence_item[END];
  ss << evidence_item.entry_exists(INSERT_END) ? "." + evidence_item[INSERT_END] : "";
  ss << "_alignment.html";
  
  return ss.str();
}


/*-----------------------------------------------------------------------------
 *  Create the HTML Evidence File
 *-----------------------------------------------------------------------------*/
// # 
// # 
void 
Evidence_Files::html_evidence_file (
                    const Settings& settings, 
                    genome_diff& gd, 
                    Evidence_Item& item
                   )
{  
  string output_path = settings.evidence_path + "/" + item[FILE_NAME];

  // Create Stream and Confirm It's Open
  ofstream HTML(output_path.c_str());
  
  if (!HTML.good()) {
    cerr << "Could not open file: " << item["output_path"] << endl;
    assert(HTML.good());
  }
  
  // Build HTML Head
  HTML << html_header("BRESEQ :: Evidence", settings);
  
  // print a table for the main item
  // followed by auxiliary tables for each piece of evidence

  diff_entry_ptr parent_item = item.parent_item;
  diff_entry_list parent_list;
  parent_list.push_back(parent_item);
  HTML << html_genome_diff_item_table_string(settings, gd, parent_list);
  HTML << "<p>";
  
  diff_entry_list evidence_list = gd.mutation_evidence_list(*parent_item);

  vector<Type> types = make_list<Type>(RA)(MC)(JC);
  
  for (vector<Type>::iterator itr = types.begin(); itr != types.end(); itr ++)
  {  
    const Type type = *itr;
    diff_entry_list this_evidence_list = evidence_list;
    this_evidence_list.remove_if(diff_entry::is_not_type(type));   
    
    if(this_evidence_list.empty()) continue;

    HTML << html_genome_diff_item_table_string(settings, gd, this_evidence_list);
    HTML << "<p>"; 
  }

  
  if (item.entry_exists(PLOT) && !item[PLOT].empty()) {
    HTML << div(ALIGN_CENTER, img(item[PLOT]));
  } else {
    stringstream ss;   
    ss << item[SEQ_ID] << ":" << item[START];
    if (item[INSERT_START].size() > 0) {
      ss << "." << item[INSERT_START];
    }
    ss << "-" << item[END];
    if (item[INSERT_END].size()) {
      ss << "." << item[INSERT_END];
    }
    cerr << "Creating read alignment for region: " << ss.str() << endl;

    if (settings.base_quality_cutoff != 0) {
      item["base_quality_cutoff"] = to_string(settings.base_quality_cutoff);
    }
    
    alignment_output ao(item[BAM_PATH], item[FASTA_PATH], settings.maximum_reads_to_align, settings.base_quality_cutoff);

    HTML << ao.html_alignment(ss.str());

  }
  HTML << html_footer();
  HTML.close();
}


/*-----------------------------------------------------------------------------
 *  //End Create_Evidence_Files
 *-----------------------------------------------------------------------------*/

void draw_coverage(Settings& settings, cReferenceSequences& ref_seq_info, genome_diff& gd)
{  
  coverage_output co(
                     settings.reference_bam_file_name, 
                     settings.reference_fasta_file_name, 
                     settings.coverage_plot_r_script_file_name, 
                     settings.coverage_plot_path
                     );
  co.output_format("png");
  
  create_path(settings.coverage_plot_path);
  string coverage_plot_path = settings.coverage_plot_path;
  
  // Coverage overview plots of entire reference sequences
  for (cReferenceSequences::iterator it = ref_seq_info.begin(); it != ref_seq_info.end(); ++it)
  {
    cAnnotatedSequence& seq = *it;
    string region = seq.m_seq_id + ":" + "1" + "-" + to_string(seq.m_length);
    string this_complete_coverage_text_file_name = settings.file_name(settings.overview_coverage_plot_file_name, "@", seq.m_seq_id);
    
    cerr << "Creating coverage plot for region: " << region << endl;
    co.plot(region, this_complete_coverage_text_file_name);
   }
  
  // Zoom-in plots of individual deletions
  vector<Type> mc_types = make_list<Type>(MC);
	diff_entry_list mc = gd.show_list(mc_types);
  for (diff_entry_list::iterator it=mc.begin(); it!=mc.end(); it++)
  {
    diff_entry_ptr& item = *it;
    uint32_t start = from_string<uint32_t>((*item)[START]);
    uint32_t end = from_string<uint32_t>((*item)[END]);
    uint32_t size = end - start + 1;
    
    uint32_t _shaded_flanking = floor(static_cast<double>(size) / 10.0);
    if (_shaded_flanking < 100) _shaded_flanking = 100;
    co.shaded_flanking(_shaded_flanking);
    
    string region = (*item)[SEQ_ID] + ":" + (*item)[START] + "-" + (*item)[END];
    string coverage_plot_file_name = settings.evidence_path + "/" + (*item)[SEQ_ID] + "_" + (*item)[START] + "-" + (*item)[END] + "." + co.output_format();

    string link_coverage_plot_file_name = Settings::relative_path(coverage_plot_file_name, settings.evidence_path);    
    (*item)[_COVERAGE_PLOT_FILE_NAME] = link_coverage_plot_file_name;
    
    cerr << "Creating coverage plot for region: " << region << endl;
    co.plot(region, coverage_plot_file_name);
  }
}

/*
 * =====================================================================================
 *        Class:  Html_Mutation_Table_String
 *  Description:  
 * =====================================================================================
 */
Html_Mutation_Table_String::Html_Mutation_Table_String(
                                                       const Settings& settings,
                                                       genome_diff& gd,
                                                       diff_entry_list& list_ref,
                                                       vector<string>& gd_name_list_ref,
                                                       Options& options,
                                                       bool legend_row, 
                                                       bool one_ref_seq,
                                                       const string& relative_link
                                                       )
  : string()
  , total_cols(0)
  , settings(settings)
  , gd(gd)
  , list_ref(list_ref)
  , legend_row(legend_row)
  , one_ref_seq(one_ref_seq)
  , gd_name_list_ref(gd_name_list_ref)
  , options(options)
  , relative_link(relative_link)
{
  (*this) += "<!--Output Html_Mutation_Table_String-->\n";
  (*this) += "<table border=\"0\" cellspacing=\"1\" cellpadding=\"3\">\n";
  
  this->Header_Line();
  this->Item_Lines();
}

Html_Mutation_Table_String::Html_Mutation_Table_String(
                                                       const Settings& settings,
                                                       genome_diff& gd,
                                                       diff_entry_list& list_ref,
  			                                               const string& relative_path, 
                                                       bool legend_row, 
                                                       bool one_ref_seq
                                                       )
  : string()
  , total_cols(0)
  , settings(settings)
  , gd(gd)
  , list_ref(list_ref)
  , legend_row(legend_row)
  , one_ref_seq(one_ref_seq)
  , relative_link(relative_path)

{


  
  (*this) += "<!--Output Html_Mutation_Table_String-->\n";
  (*this) += "<table border=\"0\" cellspacing=\"1\" cellpadding=\"3\">\n";
  vector<string> gd_name_list_ref;
  this->gd_name_list_ref = gd_name_list_ref;
  
  Options options;
  options.repeat_header = false;
  this->options = options;
  
  this->Header_Line();
  this->Item_Lines();
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Html_Mutation_Table_String
 *      Method:  Html_Mutation_Table_String :: Header_Line
 * Description:  
 *--------------------------------------------------------------------------------------
 */
void Html_Mutation_Table_String::Header_Line()
{
  // #####################
  // #### HEADER LINE ####
  // #####################
  stringstream ss; //<! Main Build Object for Function
  string header_text = ((list_ref.size() > 1) ? "Predicted mutations" : "Predicted mutation");

  // There are three possibilities for the frequency column(s)
  // (1) We don't want it at all. (Single genome no poly prediction)   
  vector<string> freq_header_list;

  if (gd_name_list_ref.size() > 0) {
    freq_header_list = gd_name_list_ref;
  } 
  else if(settings.polymorphism_prediction) {
    freq_header_list = make_list<string>("freq");
  }

  if (settings.lenski_format) {
    vector<string> header_list = split(freq_header_list.front(), "|");
    size_t header_rows = header_list.size() - 1; //!< -1 is necessary for C++

    total_cols = 7 + freq_header_list.size() ;
    if(!one_ref_seq) total_cols += 1; 
    if(!settings.no_evidence) total_cols += 1;

    for (size_t i = 1; i <= header_rows; i++) {
     ss << "<!-- Header Line -->" << endl;
     ss << "<tr>" << endl;
      if(!settings.no_evidence)
        ss << th("evidence") << endl;
      if(!one_ref_seq)
       ss << th(nonbreaking("seq id")) << endl;

      ss << th( (header_rows == i) ? "position" : "") << endl;
      ss << th( (header_rows == i) ? "mutation" : "") << endl;
      ss << th( (header_rows == i) ? "annotation" : "") << endl;
      ss << th( (header_rows == i) ? "gene" : "") << endl;
      ss << th("width=\"100%\"", (header_rows == i) ? "description" : "") << endl;
      for (vector<string>::iterator itr = freq_header_list.begin(); itr != freq_header_list.end(); itr++) {
        string& freq_header_item(*itr);        

        vector<string> header_list = split(freq_header_item, "|");        
        string this_header_string = header_list[i-1];
        while(this_header_string.find("_") != string::npos)
        {
          size_t pos = this_header_string.find("_");
          this_header_string.replace(pos, 1, "&nbsp;");//TODO confim "_" is 1 char
        }
        string this_header_string_1 = header_list.front();
        string this_header_string_2 = header_list[1];

        string color = "black";  
  
        if( this_header_string_1 == "UC" )
        color = "gray";
        else if( this_header_string_1 == "clade_1" )
        color = "green";  
        else if( this_header_string_1 == "clade_2" )
          color = "blue";  
        else if( this_header_string_1 == "clade_3" &&
                 this_header_string_2 == "ZDB483" ||
                 this_header_string_2 == "ZDB30" )
          color = "orange";
        else if( this_header_string_1 == "clade_3" )
          color = "red";

        ss << th("style=\"background-color:" + color + "\"", this_header_string) << endl;  
        ss << th(this_header_string) << endl;
      }    
      ss << th(header_rows == i ? "position" : "") << endl;
      ss << "</tr>" << endl;
    }
  } else {
    total_cols = 5 + freq_header_list.size();
    if (!one_ref_seq) {
    total_cols += 1;
    }
    if (!settings.no_evidence) {
      total_cols += 1;
    }

    ss <<  "<tr>" << endl;
   if (!settings.no_evidence) {
     ss << th("evidence") << endl;
   } 
   if(!one_ref_seq) {
     ss << th(nonbreaking("seq id")) << endl;
   }

   ss << th("position") << endl;
   ss << th("mutation") << endl;

   if(freq_header_list.size() > 0) {
     for (vector<string>::iterator itr = freq_header_list.begin() ;
          itr != freq_header_list.end() ; itr++) {
       string& freq_header_item = *itr;
       ss << th(freq_header_item) << endl;  
     }
   }
 
   ss << th("annotation") << endl;
   ss << th("gene") << endl;
   ss << th("width=\"100%\"","description") << endl;
   ss << "</tr>" << endl; 
  }

  if(!settings.no_header) {
        (*this) += tr(th("colspan=\"" + to_string(total_cols) + "\" align=\"left\" class=\"mutation_header_row\"", header_text));
  }

 ss << endl;
 (*this) += ss.str(); 
}
//===============================================================================
//       CLASS: Html_Mutation_Table_String
//      METHOD: Item_Lines
// DESCRIPTION: 
//===============================================================================
void Html_Mutation_Table_String::Item_Lines()
{
// #   ####################
// #   #### ITEM LINES ####
// #   ####################
// # 
  size_t row_num = 0;

  stringstream ss; 
  ss << "<!-- Item Lines -->" << endl;
  for (diff_entry_list::iterator itr = list_ref.begin(); itr != list_ref.end(); itr ++) { 
    diff_entry& mut = (**itr);
    if ((row_num != 0) && (options.repeat_header != 0) && (row_num % options.repeat_header == 0))
    {
      Header_Line();
    }
    row_num++;
        
    // Build Evidence Column
    string evidence_string;
    if (!settings.no_evidence) {
      bool already_added_RA = false;
       
      diff_entry_list mutation_evidence_list = gd.mutation_evidence_list(mut);
      
      for (diff_entry_list::iterator evitr = mutation_evidence_list.begin(); evitr != mutation_evidence_list.end(); evitr ++) {  
        diff_entry& evidence_item = **evitr;

        if (evidence_item._type == RA) {
          if (already_added_RA) 
            continue;
          else 
            already_added_RA = true;
        }
        
        if (!evidence_string.empty()) evidence_string += "&nbsp;";
        evidence_string += a(relative_link + evidence_item[_EVIDENCE_FILE_NAME], to_string(evidence_item._type));
      }
    }
  
    string row_class = "normal_table_row";

    // There are three possibilities for the frequency column(s)
    // (1) We don't want it at all. (Single genome no poly prediction)   
    vector<string> freq_list;
    
  //TODO @JEB 
// #     # (2) We want multiple columns because we are comparing genomes.
// #     if (defined $gd_name_list_ref) {
// #       #"_freq_[name]" keys were made within GenomeDiff structure        
// #       @freq_list = map { $mut->{"frequency_$_"} } @$gd_name_list_ref; 
// #       $row_class = "alternate_table_row_" . ($row_num % 2);
// #     }
   
    // (3) We want a single column (polymorphism prediction)
    if (settings.polymorphism_prediction) {
      // polymorphisms get highlighted
      if(mut.entry_exists(FREQUENCY) && (from_string<double>(FREQUENCY) != 1)) {
        row_class = "polymorphism_table_row";
        freq_list.push_back(mut[FREQUENCY]);
      }
      else // frequencies of other entries assumed to be 1.00
      {
        freq_list.push_back("1");
      }
    }
      
    // ### marshal cells defined depending on mutation type
    string cell_seq_id = nonbreaking(mut[SEQ_ID]);
    string cell_position = commify(mut[POSITION]);
    string cell_mutation;
    string cell_mutation_annotation = nonbreaking(formatted_mutation_annotation(mut));
    string cell_gene_name = i(nonbreaking(mut[GENE_NAME]));
    string cell_gene_product = htmlize(mut["gene_product"]);

    // build 'mutation' column = description of the genetic change
    if (mut._type == SNP) {
      cell_mutation = mut["_ref_seq"] + "&rarr;" + mut[NEW_SEQ];
    } else if (mut._type == INS) {
      cell_mutation = "+";
      cell_mutation += mut[NEW_SEQ];
    } else if (mut._type == DEL) {
      cell_mutation = nonbreaking("&Delta;" + commify(mut["size"]) + " bp");
      string annotation_str;
      
      // special annotation for mediated- and between repeat elements
      if (mut.entry_exists("mediated")) 
        annotation_str = mut["mediated"] + "-mediated"; 
      if (mut.entry_exists("between")) 
        annotation_str = "between " + mut["between"];
      // default
      if(annotation_str.empty()) {
        annotation_str = nonbreaking(mut["gene_position"]);
      } 
      cell_mutation_annotation =  nonbreaking(annotation_str);
    } else if (mut._type == SUB) {
      cell_mutation = nonbreaking(mut["size"] + " bp&rarr;" + mut["new_seq"]);
    } else if (mut._type == CON) {
      cell_mutation = nonbreaking(mut["size"] + " bp&rarr;" + mut["region"]);
    } else if (mut._type == MOB) {
      stringstream s;
       
      stringstream s_start;
      if (mut.entry_exists("ins_start")) {
        s_start << "+" << mut["ins_start"];
      }
      if (mut.entry_exists("del_start")) {
        s_start << "&Delta;" << mut["del_start"];
      }
      if (!(s_start.str()).empty()) {
        s << s_start.str() << " :: ";
      }

      s << mut["repeat_name"] << " (";
      switch (from_string<int32_t>(mut["strand"]))
      {
        case -1:
          s << "+";
          break;
        case 0:
          s << "?";
          break;
        case +1:
          s << "–";
          break;
      }
      s << ")";

      stringstream s_end;
      if (mut.entry_exists("del_end")) {
        s_end << " &Delta;" << mut["del_end"];
      }
      if (mut.entry_exists("ins_end")) {
        s_end << " +" << mut["ins_end"];
      }
      if (!(s_end.str()).empty()) {
        s << " ::" << s_end.str();
      }
      if (from_string<int32_t>(mut["duplication_size"]) > 0) {
        s << " +" << mut["duplication_size"] << " bp";
      } else if (from_string<int32_t>(mut["duplication_size"]) < 0) {
        s << " &Delta;" << mut["duplication_size"] << " bp";
      }
      cell_mutation = nonbreaking(s.str());
    } else if (mut._type == INV) {
      cell_mutation = nonbreaking(commify(mut["size"]) + " bp inversion");
      cell_gene_name = i(nonbreaking(mut["gene_name_1"])) + "&darr;" +
                       i(nonbreaking(mut["gene_name_2"]));
      cell_gene_product = htmlize(mut["gene_product_1"]) + "&darr;" + 
                          htmlize(mut["gene_product_2"]);
    } else if (mut._type == AMP) {
      cell_mutation = nonbreaking(commify(mut["size"]) + " bp x " + mut["new_copy_number"]);
      cell_mutation_annotation = 
        from_string<uint32_t>(mut["new_copy_number"]) == 2 ?
          "duplication" : "amplification";
    }
    // ###### PRINT THE TABLE ROW ####
    ss << endl << "<!-- Print The Table Row -->" << endl; 
    ss << start_tr("class=\"" + row_class + "\"") << endl;

    if (!settings.no_evidence) {
      ss << td(ALIGN_CENTER, evidence_string) << "<!-- Evidence -->" << endl;
    }
    if (!one_ref_seq) {
      ss << td(ALIGN_CENTER, cell_seq_id) << "<!-- Seq_Id -->" << endl;
    }
    ss << td(ALIGN_RIGHT, cell_position) << "<!-- Position -->" << endl;

    ss << td(ALIGN_CENTER, cell_mutation) << "<!-- Cell Mutation -->" << endl;
    if (settings.lenski_format) {
      ss << "<!-- Lenski_Format -->" << endl;
      ss << td(ALIGN_CENTER, cell_mutation_annotation) << endl;
      ss << td(ALIGN_CENTER, cell_gene_name) << endl;
      ss << td(ALIGN_LEFT, cell_gene_product) << endl;
    }
    
    //Need if statement for C++
    if (freq_list.size() >= 1 && !freq_list[0].empty()) {
      ss << freq_cols(freq_list) << endl;
    } 
    if (settings.lenski_format) {
      ss << "<!-- Lenski Format -->" << endl;
      ss << td(ALIGN_CENTER, cell_position) << endl;
    } else {
      ss << td(ALIGN_CENTER, cell_mutation_annotation) << endl;
      ss << td(ALIGN_CENTER, cell_gene_name) << endl;
      ss << td(ALIGN_LEFT, cell_gene_product) << endl;
    }
    ss << "</tr>" << endl;
    
    ss << "<!-- End Table Row -->" << endl;
  } //##### END TABLE ROW ####
  
  if (legend_row) {
    ss << "<tr>" << endl;
    ss << td("colspan=\"" + to_string(total_cols) + "\"", 
                    b("Evidence codes: RA = read alignment, MC = missing coverage, JC = new junction"));
    ss << "</tr>" << endl;
  }
  ss << "</table>";
  
  (*this) += ss.str();
}
// # 
//===============================================================================
//       CLASS: Html_Mutation_Table_String
//      METHOD: freq_to_string  
// DESCRIPTION: Helper function used in Item_Lines
//===============================================================================
string Html_Mutation_Table_String::freq_to_string(const string& freq)
{
  if (freq == "H") {
    return "H";
  }
  if (from_string<double>(freq) == 0.0) {
    return "";
  }
  stringstream ss;
  if (from_string<double>(freq) == 1.0 || freq.empty()) {
    ss << "100%";
  } 
  else {
    double conv_freq = from_string<double>(freq) * 100;
    ss.width(4);
    ss.setf(ios_base::fixed);
    ss.precision(1);
    ss << conv_freq << "%";
  }
  return ss.str(); 
}

//===============================================================================
//       CLASS: Html_Mutation_Table_String
//      METHOD: freq_cols  
// DESCRIPTION: Helper function used in Item_Lines
//===============================================================================
string Html_Mutation_Table_String::freq_cols(vector<string> freq_list)
{
  stringstream ss;
  for (vector<string>::iterator itr = freq_list.begin();
       itr != freq_list.end(); itr ++) {  
    string& freq = (*itr);
    if (settings.shade_frequencies) {
      string bgcolor;
      if (freq == "1") {
       bgcolor = "Blue";
      }
      if (!bgcolor.empty()) {
        ss << td("align=\"right\" bgcolor=\"" + bgcolor +"\"", "&nbsp;"); //TODO Check
      } 
      else {
        ss << td(ALIGN_RIGHT,"&nbsp;");
      }
    }
    else {
      ss << td (ALIGN_RIGHT, freq_to_string(freq));
    }
  }
  return ss.str();
}


}//end namespace output
}//end namespace breseq

