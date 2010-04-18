###
# Pod Documentation
###

=head1 NAME

MummerDelta.pm

=head1 SYNOPSIS

Module for reading and writing MUMmer delta files.

=head1 DESCRIPTION

=head1 AUTHOR

Jeffrey Barrick
<jbarrick@msu.edu>

=head1 COPYRIGHT

Copyright 2008.  All rights reserved.

=cut

###
# End Pod Documentation
###

package Breseq::AlignmentCorrection;
require Exporter;

use strict;

our @ISA = qw( Exporter );
our @EXPORT = qw();
our $VERSION = '0.01';

use Data::Dumper;


#compare matches to candidate junctions with matches to original genome
sub correct_alignments
{
	my $verbose = 0;
	my ($settings, $summary, $ref_seq_info) = @_;
	my $gene_list_hash_ref = $ref_seq_info->{gene_lists};
	my $is_list_hash_ref = $ref_seq_info->{is_lists};
	my $flanking_length = $settings->{max_read_length};
	
	## for now we just use mapping qualities from ssaha2, but could load ref sequences this way
	my $reference_faidx_file_name = $settings->file_name('reference_fasta_file_name');
	my $reference_fai = Bio::DB::Sam::Fai->load($reference_faidx_file_name);
		
	my $candidate_junction_file_name = $settings->file_name('candidate_junction_fasta_file_name');
	## if there were no candidate junction (file is empty) then we seg fault if we try to use samtools on it...
	$settings->{no_junction_prediction} = 1 if ( (!-e $candidate_junction_file_name) || (-s $candidate_junction_file_name == 0) );
	my	$candidate_junction_fai = Bio::DB::Sam::Fai->load($candidate_junction_file_name) if (!$settings->{no_junction_prediction});		
	
	my $minimum_best_score = 0;
	my $minimum_best_score_difference = 0;
	
	my %matched_junction;
	my %degenerate_matches;
	my $i = 0;
	
	my $resolved_reference_sam_file_name = $settings->file_name('resolved_reference_sam_file_name');
	my $RREF;
	open $RREF, ">$resolved_reference_sam_file_name" or die;
	
	my $resolved_junction_sam_file_name = $settings->file_name('resolved_junction_sam_file_name');
	my $RCJ;
	open $RCJ, ">$resolved_junction_sam_file_name" or die;


	
	my $reference_header;
	my $candidate_junction_header;
	foreach my $read_struct ($settings->read_structures)
	{	
		my $read_file = $read_struct->{base_name};	
		print STDERR "  READ FILE::$read_file\n";

		## setup the summary
		my $s;
		$s->{unmatched_reads} = 0;

		## use the original fastq file to keep track of order
		## some matches may exist in only one or the other file
		
		my @in_fastq;
		my @fastq_file_name;
		my @fastq_file_index;
		my @out_unmatched_fastq;
		my $i=0;
		for (my $i=0; $i < scalar @{$read_struct->{base_names}}; $i++)
		{
			my $this_read_file = $read_struct->{base_names}->[$i];
			my $this_fastq_file_name = $read_struct->{read_fastq_list}->[$i];
			
			$fastq_file_name[$i] = $settings->read_file_to_fastq_file_name($this_read_file);	
			$fastq_file_index[$i] = $settings->read_file_to_fastq_file_index($this_read_file);		
				
			$in_fastq[$i] = Breseq::Fastq->new(-file => $this_fastq_file_name);
			
			if ($settings->{unmatched_reads})
			{				
				my $unmatched_file_name = $settings->file_name('unmatched_read_file_name', {'#'=>$this_read_file});
				$out_unmatched_fastq[$i] = Breseq::Fastq->new(-file => ">$unmatched_file_name");
			}
		}		

		my $reference_sam_file_name = $settings->file_name('reference_sam_file_name', {'#'=>$read_file});
		my $reference_tam = Bio::DB::Tam->open($reference_sam_file_name) or die "Could not open $reference_sam_file_name";
		my $reference_faidx_file_name = $settings->file_name('reference_faidx_file_name');
		$reference_header = $reference_tam->header_read2($reference_faidx_file_name) or throw("Error reading reference fasta index file: $reference_faidx_file_name");		
		my $reference_al;
		my $last_reference_alignment;
		
		
		my $candidate_junction_tam;
		if (!$settings->{no_junction_prediction})
		{
			my $candidate_junction_sam_file_name = $settings->file_name('candidate_junction_sam_file_name', {'#'=>$read_file});
			$candidate_junction_tam = Bio::DB::Tam->open($candidate_junction_sam_file_name) or die " Could not open candidate junction SAM file\n";
			my $candidate_junction_faidx_file_name = $settings->file_name('candidate_junction_faidx_file_name');
			$candidate_junction_header = $candidate_junction_tam->header_read2($candidate_junction_faidx_file_name) or die("Error reading reference fasta index file: $candidate_junction_faidx_file_name");			
		}
		
		my $candidate_junction_al;
		my $last_candidate_junction_alignment;		
				
		#proceed through all of the alignments
		if (!$settings->{no_junction_prediction})
		{
			($candidate_junction_al, $last_candidate_junction_alignment) 
				= Breseq::Shared::tam_next_read_alignments($candidate_junction_tam, $candidate_junction_header, $last_candidate_junction_alignment);		
		}
		
		($reference_al, $last_reference_alignment) 
			= Breseq::Shared::tam_next_read_alignments($reference_tam, $reference_header, $last_reference_alignment);
		
		my $f = 0;
		READ: while (my $seq = $in_fastq[$f]->next_seq)
		{			
			$i++;
			last if ($settings->{alignment_read_limit} && ($i > $settings->{alignment_read_limit}));
			print STDERR "    READS:$i\n" if ($i % 10000 == 0);
			
			#Does this read have candidate junction matches?
			my $this_candidate_junction_al = [];
			if (($candidate_junction_al) && ($candidate_junction_al->[0]->qname =~ m/^$seq->{id}/))
			{
				$this_candidate_junction_al = $candidate_junction_al;
				($candidate_junction_al, $last_candidate_junction_alignment) 
					= Breseq::Shared::tam_next_read_alignments($candidate_junction_tam, $candidate_junction_header, $last_candidate_junction_alignment);
			}
			
			#Does this read have reference sequence matches?
			my $this_reference_al = [];
			if (($reference_al) && ($reference_al->[0]->qname =~ m/^$seq->{id}/))
			{
				$this_reference_al = $reference_al;
				($reference_al, $last_reference_alignment) 
					= Breseq::Shared::tam_next_read_alignments($reference_tam, $reference_header, $last_reference_alignment);
			}			
						
			###			
			## Matches to candidate junctions may not overlap the junction.
			## Reduce this list to only those that overlap the entire junction.
			###
						
			for (my $i=0; $i<scalar @$this_candidate_junction_al; $i++)
			{
				my $a = $this_candidate_junction_al->[$i];
				next if ($a->unmapped);
				
				my $junction_id = $candidate_junction_header->target_name()->[$a->tid];
				my @scj = Breseq::Shared::junction_name_split($junction_id);
								
				## find the start and end coordinates of the overlap
				my $overlap = $scj[8];
				my ($junction_start, $junction_end);
				
				$junction_start = $flanking_length;
				$junction_end = $flanking_length - $overlap + 1;
								
				if ( ($a->start > $junction_start) || ($a->end < $junction_start)
			      || ($a->start > $junction_end) || ($a->end < $junction_end) )
				{
					splice  @$this_candidate_junction_al, $i, 1;
					$i--;
				}
			}

			###			
			## Determine if the read has a better match to a candidate junction
			## or to the reference sequence.
			###


			##Optionally, remove matches to junctions where the first base of the read does not match
			@$this_candidate_junction_al = grep {_alignment_begins_with_match_in_read($_)} @$this_candidate_junction_al if ($settings->{require_first_read_base_to_match});
			@$this_reference_al = grep {_alignment_begins_with_match_in_read($_)} @$this_reference_al if ($settings->{require_first_read_base_to_match});

			
			###			
			## Determine if the read has a better match to a candidate junction
			## or to the reference sequence.
			###

			my $best_candidate_junction_score = -9999;
			my $best_reference_score = -9999;

			if (@$this_candidate_junction_al)
			{
				my $ca = $this_candidate_junction_al->[0];
				$best_candidate_junction_score = $ca->aux_get("AS");
			}
			
			if (@$this_reference_al)
			{
				my $ra = $this_reference_al->[0];
				$best_reference_score = $ra->aux_get("AS");
			}
						
			### this read potentially supports a candidate junction		
			if ($best_candidate_junction_score > $best_reference_score)
			{
				my @this_dominant_candidate_junction_al = _alignment_list_to_dominant_best($minimum_best_score, undef, @$this_candidate_junction_al);
				
				my $item = {
					alignments => $this_candidate_junction_al, 
					reference_alignments => $this_reference_al, 
					dominant_alignments => \@this_dominant_candidate_junction_al, 
					fastq_file_index => $fastq_file_index[$f],
				};
				
				if (scalar @this_dominant_candidate_junction_al == 1)
				{
					my $a = $this_dominant_candidate_junction_al[0];
					my $junction_id = $candidate_junction_header->target_name()->[$a->tid];
					#print "$junction_id\n";
					push @{$matched_junction{$junction_id}}, $item;
				}	
				else #multiple identical matches, ones with most hits later will win these matches
				{					
					foreach my $a (@this_dominant_candidate_junction_al)
					{	
						my $junction_id = $candidate_junction_header->target_name()->[$a->tid];
						my $read_name = $a->qname;
						$degenerate_matches{$junction_id}->{$read_name} = $item;
					}
				}
			}
			else ### best match is to the reference, record in that SAM file.
			{	
				## are there matches to the reference?
				if (scalar @$this_reference_al > 0)
				{
					_write_reference_matches($minimum_best_score, $minimum_best_score_difference, $reference_fai, $ref_seq_info, $RREF, $reference_header, $fastq_file_index[$f], @$this_reference_al);
				}
				
				## if not, then write to unmatched read file
				elsif ($settings->{unmatched_reads}) 
				{
					$out_unmatched_fastq[$f]->write_seq($seq);
					$s->{unmatched_reads}++;
				}
			}
		} continue {
			$f++;
			$f %= scalar @in_fastq;
		}
	
		## save statistics
		$summary->{alignment_correction}->{$read_file} = $s;
	}	
	
	###			
	## Determine which junctions are real, prefer ones with most matches
	##
	## THIS NEEDS TO BE DEALT WITH!!!!
	## Possible problem: What if all matches to a junction are degenerate because a very similar junction exists?
	###
	
	my @sorted_keys = sort {-(scalar @{$matched_junction{$a}} <=> scalar @{$matched_junction{$b}})} keys %matched_junction;
			
	my @new_keys;
	foreach my $key (@sorted_keys)
	{
		my $failed = _test_junction($key, \%matched_junction, \%degenerate_matches, $flanking_length, $minimum_best_score, $minimum_best_score_difference, $reference_fai, $ref_seq_info, $RREF, $reference_header, $RCJ, $candidate_junction_header);
		push @new_keys, $key if (!$failed);
	}
	
	##strictly speaking... should re-sort after every successful junction prediction... 
	@sorted_keys = sort {-(scalar keys %{$degenerate_matches{$a}} <=> scalar keys %{$degenerate_matches{$b}})} keys %degenerate_matches;
	while (@sorted_keys)
	{
		my $key = shift @sorted_keys;
		
		print "Trying degenerate $key...\n";
		
		next if (!defined $degenerate_matches{$key}); #they can be removed 
		
		my $failed = _test_junction($key, \%matched_junction, \%degenerate_matches, $flanking_length, $minimum_best_score, $minimum_best_score_difference, $reference_fai, $ref_seq_info, $RREF, $reference_header, $RCJ, $candidate_junction_header);
		if (!$failed)
		{
			@sorted_keys = sort {-(scalar keys %{$degenerate_matches{$a}} <=> scalar keys %{$degenerate_matches{$b}})} keys %degenerate_matches;
			push @new_keys, $key;
		}
	}
	@sorted_keys = @new_keys;
	
	#print successful ones out
	print "Successful hybrids\n" if ($verbose);

	#re-sort since some have gained reads from degenerate matches
	@sorted_keys = sort {-(scalar @{$matched_junction{$a}} <=> scalar @{$matched_junction{$b}})} @sorted_keys;

	my @hybrid_predictions;
 	foreach my $key (@sorted_keys)
 	{	  
		my $matched_junction = 
		print "$key\n" if ($verbose);
		my $item = {key => $key, flanking_length => $flanking_length};
		
		# my $junction_id = Breseq::Shared::junction_name_join($hash_seq_id_1, $hash_coord_1, $hash_strand_1, $hash_seq_id_2, $hash_coord_2, $hash_strand_2, $overlap, $unique_read_seq_string);	
		my @split_key = Breseq::Shared::junction_name_split($key);

		my ($upstream_reference, $upstream_position, $upstream_direction, $upstream_redundant) = @split_key[0..3];
		$upstream_direction = -1 if ($upstream_direction == 0);
 
		my ($downstream_reference, $downstream_position, $downstream_direction, $downstream_redundant) = @split_key[4..7];
		$downstream_direction = -1 if ($downstream_direction == 0);
	
		$item->{overlap} = $split_key[8];
				
		##
		## Create three intervals for making alignments and html output
		##    it would be nice to flag whether we think the original junction is still there
		##
	 	
		## (1) The first alignment has information relative to the junction candidates
		if ($item->{overlap} == 0)
		{
			$item->{start} = $item->{flanking_length};
			$item->{end} = $item->{flanking_length}+1;			
			$item->{mark} = '/';
		}
		elsif ($item->{overlap} > 0)
		{
			$item->{start} = $item->{flanking_length} + 1 - $item->{overlap};
			$item->{end} = $item->{flanking_length};
			$item->{mark} = '|';
		}
		else ## ($item->{overlap} < 0)
		{
			$item->{start} = $item->{flanking_length}+1;
			$item->{end} = $item->{flanking_length}-$item->{overlap};
			$item->{mark} = '*';
		}
		$item->{seq_id} = $key;
		
		#### NOTE: For these intervals, the strand indicates which direction (relative to top strand of genome)
		####       you move until you hit the given position to reproduce sequence in new junction.

		## (2) Alignment for the reference sequence that disagrees with the junction #1.
 		$item->{interval_1}->{start} = $upstream_position;
		$item->{interval_1}->{end} = $upstream_position;
		$item->{interval_1}->{strand} = $upstream_direction;		
		$item->{interval_1}->{seq_id} = $upstream_reference;
		$item->{interval_1}->{redundant} = $upstream_redundant;


		## (3) Alignment for the reference sequence that disagrees with the junction #2.
 		$item->{interval_2}->{start} = $downstream_position;
		$item->{interval_2}->{end} = $downstream_position;
		$item->{interval_2}->{strand} = $downstream_direction;
		$item->{interval_2}->{seq_id} = $downstream_reference;
		$item->{interval_2}->{redundant} = $downstream_redundant;

		$item->{total_reads} = scalar @{$matched_junction{$key}};
		
		push @hybrid_predictions, $item;


		#if there is overlapping sequence, and both sides are unique,
		#then give overlap to first side
		
		if (!$item->{interval_1}->{redundant} && !$item->{interval_2}->{redundant})
		{
			if ($item->{overlap} > 0)
			{
				my $strand_direction = ($item->{interval_2}->{strand} > 0) ? +1 : -1;
				
				$item->{interval_2}->{start} += $item->{overlap} * $strand_direction;
				$item->{interval_2}->{end} += $item->{overlap} * $strand_direction;
				$item->{overlap} = 0;			
			}			
		}
		### Note: Other adjustments to voerlap can happen at the later annotation stage
		### because they will not affect coverage for calling deletions or mutations
		### because they will be in redundantly matched sides of junctions
		

		## create matches from UNIQUE sides of each match to reference genome
		## this fixes, for example appearing to not have any coverage at the origin of a circular DNA fragment
		### currently we do not add coverage to the IS element (which we would have to know all copies of to do...)
	
		if ( $settings->{add_split_junction_sides} )
		{
			foreach my $match (@{$matched_junction{$key}})
			{
				my $a = $match->{dominant_alignments}->[0];
				my $fastq_file_index = $match->{fastq_file_index};
				foreach my $side (1,2)
				{
					my $side_key = 'interval_' . $side;

	############## Need to add indication of whether this side of the junction is unique!!
	
	
	
                    ## Do not count for coverage if it is redundant is redundant!!
					if (!$item->{$side_key}->{redundant})
					{
						##write out match corresponding to this part to SAM file						
						my $trim = _trim_ambiguous_ends($a, $candidate_junction_header, $candidate_junction_fai);
						Breseq::Shared::tam_write_moved_alignment($RREF, $item->{$side_key}->{seq_id}, $fastq_file_index, $a, $side, $item->{flanking_length}, $item->{overlap}, $item->{$side_key}->{start}, $item->{$side_key}->{strand}, $trim);		
					}
				}
			}
		}
	}
	
 
 	return @hybrid_predictions;	
}

sub _write_reference_matches
{
	my ($minimum_best_score, $minimum_best_score_difference, $reference_fai, $ref_seq_info, $RREF, $reference_header, $fastq_file_index, @reference_al) = @_;
	
	@reference_al = _alignment_list_to_dominant_best($minimum_best_score, $minimum_best_score_difference, @reference_al);
	
	my @trims;
	foreach my $a (@reference_al)
	{
	#	push @trims, _trim_ambiguous_ends($a, $reference_header, $reference_fai);
		push @trims, _trim_ambiguous_ends($a, $reference_header, $reference_fai, $ref_seq_info); #slightly faster than using fai	
	}
	
	Breseq::Shared::tam_write_read_alignments($RREF, $reference_header, $fastq_file_index, \@reference_al, \@trims);
}

sub _alignment_begins_with_match_in_read
{
	my ($a) = @_;
	return 0 if ($a->unmapped);
	
	my $ca = $a->cigar_array;
	return ($ca->[-1]->[0] eq 'M') if ($a->reversed);
	return ($ca->[0]->[0] eq 'M');
}

sub _alignment_length_on_query
{
	my ($a) = @_;
	
	return 0 if ($a->unmapped);
	
	my $ca = $a->cigar_array;	
	my $start = 1;
	$start += $ca->[0]->[1] if ($ca->[0]->[0] eq 'S');
	my $end = $a->query->length;
	$end -= $ca->[-1]->[1] if ($ca->[-1]->[0] eq 'S');
	
	($start, $end) = ($a->query->length - $start + 1, $a->query->length - $end + 1) if ($a->reversed);
	($start, $end) = ($end, $start) if ($a->reversed);
	
	return ($end-$start+1);
}

sub _alignment_list_to_dominant_best
{
	###MOVE TO SETTINGS
	my $minimum_match_length = 28; 
	
	my ($minimum_best_score, $minimum_best_score_difference, @al) = @_;
	return () if (scalar @al <= 0);
	
	## require a minimum length of the read to be mapped
 	@al = grep {_alignment_length_on_query($_) >= $minimum_match_length} @al;
	return () if (scalar @al == 0);
	
	my $best_score = $al[0]->aux_get("AS");
	
	## no scores meet minimum
	return () if (defined $minimum_best_score && ($best_score < $minimum_best_score));
	
	## how many reads share the best score?
	my $last_best = 0;
	while (($last_best+1 < scalar @al) && ($al[$last_best+1]->aux_get("AS") == $best_score))
	{
		$last_best++;
	}
	
	## no scores meet minimum difference between best and next best
	if (defined $minimum_best_score_difference && (scalar @al > $last_best+1))
	{
		my $second_best_score = $al[$last_best+1]->aux_get("AS");
		return () if ($second_best_score + $minimum_best_score_difference >= $best_score)
	}
	
	return splice @al, 0, $last_best+1;
}


sub _alignment_list_to_mapping_quality
{
	#has no clue what to do about mismatches...
	my (@alignment_list) = @_;
		
	my $mapping_quality = $alignment_list[0]->aux_get("AS");
	$mapping_quality -= $alignment_list[1]->aux_get("AS") if (scalar @alignment_list > 1);
	
	return $mapping_quality;
}


sub _trim_ambiguous_ends
{
	my $verbose = 0;
	my ($a, $header, $fai, $ref_seq_info) = @_;
	
#	print $a->qname . "\n";	
#	$verbose = ($a->qname eq '20AOWAAXX-Lenski:6:6:817:62');
	
	# has two keys: 'left' and 'right' which are how far to inset in REFERENCE
	my $trims;
	
	#which reference sequence?
	my $seq_id = $header->target_name->[$a->tid];
	my ($ref_strings, $ref_seq_length);
	
	## using $fai is more compatible, and must currently be used for junctions
	## using $ref_seq_info is slightly quicker, and currently used for the reference sequence
	if (defined $ref_seq_info)
	{		
		$ref_strings = $ref_seq_info->{ref_strings};	
		$ref_seq_length = length($ref_strings->{$seq_id});
	}
	## >>> transition to not using ref_seq_info
	else
	{
		$ref_seq_length  = $header->target_len->[$a->tid];
	}
	
	#create sequence snippets that we need to pay attention to ends of sequence
	my $expand_by = 36;
	my $expand_left = ($a->start-1 < $expand_by) ? $a->start-1 : $expand_by;
	my $expand_right = ($ref_seq_length - $a->end < $expand_by) ? $ref_seq_length-$a->end : $expand_by;
	
	
	my $expanded_ref_string = '';	
	if (defined $ref_strings)
	{
		$expanded_ref_string = substr $ref_strings->{$seq_id}, $a->start-$expand_left-1, ($a->end+$expand_right) - ($a->start-$expand_left) + 1;
	}
	else
	{
		my $expanded_ref_range = $seq_id . ':' . ($a->start-$expand_left) . '-' . ($a->end+$expand_right);
		$expanded_ref_string = $fai->fetch($expanded_ref_range);		
	}

	my $ref_string;
	if (defined $ref_strings)
	{
		$ref_string = substr $ref_strings->{$seq_id}, $a->start-1, $a->end - $a->start + 1;
	}	
	else
	{
		my $ref_range = $seq_id . ':' . $a->start . '..' . $a->end;
		$ref_string = $fai->fetch( $seq_id . ':' . $a->start . '-' . $a->end );
	}
		
	my ($q_start, $q_end) = Breseq::Shared::alignment_query_start_end($a, { 'no_reverse' => 1} );
	my $q_length = $a->l_qseq;
	my $qry_string = substr $a->qseq, $q_start-1, $q_end - $q_start + 1;
	my $full_qry_string = $a->qseq;
	
	#take maximum of inset for query and reference
	my ($left_ref_inset, $right_ref_inset) = _ambiguous_end_offsets_from_sequence($ref_string);
	
	#add UNALIGNED bases at te end of reads
	$left_ref_inset += $q_start - 1;
	$right_ref_inset += $q_length - $q_end;
	
	
	## save a little time if qry and ref sequences are identical.
	my ($left_qry_inset, $right_qry_inset) = (0,0);
	if ($ref_string ne $qry_string)
	{
		($left_qry_inset, $right_qry_inset) = _ambiguous_end_offsets_from_sequence($qry_string);
		#add UNALIGNED bases at te end of reads
		$left_qry_inset += $q_start - 1;
		$right_qry_inset += $q_length - $q_end;
		
	}
	
	my ($left_full_qry_inset, $right_full_qry_inset) = _ambiguous_end_offsets_from_sequence($full_qry_string);
	my ($left_ref_expanded_inset, $right_ref_expanded_inset) 
		= _ambiguous_end_offsets_from_expanded_sequence($expand_left, $expand_right, $expanded_ref_string);
	
	if ($verbose)
	{
		print "Whole Read: $ref_string\n";
		print "Qry Start, End: $q_start, $q_end\n";	
		print "Ref: $ref_string\n";
		print "Ref insets: $left_ref_inset, $right_ref_inset\n";
		print "Qry: $qry_string\n";
		print "Qry insets: $left_qry_inset, $right_qry_inset\n";
		print "Full Qry: $full_qry_string\n";
		print "Full Qry insets: $left_full_qry_inset, $right_full_qry_inset\n";
		print "Expanded: $expanded_ref_string\n";
		print "Expand: $expand_left, $expand_right\n";
		print "Expanded Ref insets: $left_ref_expanded_inset, $right_ref_expanded_inset\n";		
	}


	$left_qry_inset = ($left_qry_inset > $left_full_qry_inset) ? $left_qry_inset : $left_full_qry_inset;
	$right_qry_inset = ($right_qry_inset > $right_full_qry_inset) ? $right_qry_inset : $right_full_qry_inset;

	$left_ref_inset = ($left_ref_inset > $left_ref_expanded_inset) ? $left_ref_inset : $left_ref_expanded_inset;
	$right_ref_inset = ($right_ref_inset > $right_ref_expanded_inset) ? $right_ref_inset : $right_ref_expanded_inset;

	if ($verbose)
	{
		print "Ref insets: $left_ref_inset, $right_ref_inset\n";
		print "Full Qry insets: $left_full_qry_inset, $right_full_qry_inset\n";
		print "Expanded Ref insets: $left_ref_expanded_inset, $right_ref_expanded_inset\n";		
	}

	##
	# Correct insets in the ref sequence to read coordinates, which must pay attention to gaps
	##
	
	#if no gaps, then just need to take the greater one
	my $left_ref_count = ($left_ref_inset > $left_qry_inset) ? $left_ref_inset : $left_qry_inset;
	my $left_qry_count = $left_ref_count;
	my $right_ref_count = ($right_ref_inset > $right_qry_inset) ? $right_ref_inset : $right_qry_inset;
	my $right_qry_count = $right_ref_count;

	if ($verbose)
	{
		print "Qry Count: $left_qry_count, $right_qry_count\n";
	}
	
		
	# my $cigar = $a->cigar_array;
	# ##skip soft padding
	# shift @$cigar if ($cigar->[0]->[0] eq 'S');
	# 
	# my $ungapped_left_qry_inset = $left_ref_inset;
	# foreach (my $i=0; $i<$left_ref_count; $i++)
	# {
	# 	$i;
	# }

	return ( {'L'=>$left_qry_count, 'R'=>$right_qry_count} );
}

sub _ambiguous_end_offsets_from_expanded_sequence
{
	my ($expand_left, $expand_right, $ref_string) = @_;
	
	my $left_inset = 0;
	my $right_inset = 0;
	
	{ #left side

		#maximum size to check is $expand_by
		my $test_left_inset = 0;
		while ($test_left_inset < $expand_left)
		{
			my $found_left_inset = $test_left_inset;
			my $test_length = $expand_left-$test_left_inset;

			my $match_found = 0;
			my $test_end_string;
			while ( ($test_length > 0) && !$match_found)
			{	
				$test_end_string = substr $ref_string, $found_left_inset, $test_length;
				#force removal of end nucleotides if no higher order repeats found
				$found_left_inset-- if ($test_length == 1);			
				my $test_interior_string = substr $ref_string, $found_left_inset+$test_length, $test_length;
				
				while ($test_end_string eq $test_interior_string)
				{
					$match_found = 1;
					$found_left_inset += $test_length;
					$test_interior_string = substr $ref_string, $found_left_inset+$test_length, $test_length;
				}
				$found_left_inset += $test_length if ($match_found);
				$test_length--;
			} 
				
			#test partial matches (that continue part of repeat further)
			 #note: already starts at one less than actual repeat size
			my $test_partial_size = $test_length; 
			while ($test_partial_size > 0)
			{
				my $test_partial_end_string = substr $test_end_string, 0, $test_partial_size;
				my $test_interior_string = substr $ref_string, $found_left_inset, $test_partial_size;
				if ($test_partial_end_string eq $test_interior_string)
				{
					$found_left_inset += $test_partial_size;
					last;
				}
				$test_partial_size--;
			}
		
			$left_inset = $found_left_inset-$expand_left if ($found_left_inset-$expand_left > $left_inset);
			$test_left_inset++;
		}
	}
	
	{ #right side

		#maximum size to check is $expand_by
		my $test_right_inset = 0;
		while ($test_right_inset < $expand_right)
		{
			my $found_right_inset = $test_right_inset;
			my $test_length = $expand_right-$test_right_inset;

			my $match_found = 0;
			my $test_end_string;
			while ( ($test_length > 0) && !$match_found)
			{	
				$test_end_string = substr $ref_string, $found_right_inset, $test_length;
				#force removal of end nucleotides if no higher order repeats found
				$found_right_inset-- if ($test_length == 1);			
				my $test_interior_string = substr $ref_string, $found_right_inset+$test_length, $test_length;
				
				while ($test_end_string eq $test_interior_string)
				{
					$match_found = 1;
					$found_right_inset += $test_length;
					$test_interior_string = substr $ref_string, $found_right_inset+$test_length, $test_length;
				}
				$found_right_inset += $test_length if ($match_found);
				$test_length--;
			} 
				
			#test partial matches (that continue part of repeat further)
			 #note: already starts at one less than actual repeat size
			my $test_partial_size = $test_length; 
			while ($test_partial_size > 0)
			{
				my $test_partial_end_string = substr $test_end_string, 0, $test_partial_size;
				my $test_interior_string = substr $ref_string, $found_right_inset, $test_partial_size;
				if ($test_partial_end_string eq $test_interior_string)
				{
					$found_right_inset += $test_partial_size;
					last;
				}
				$test_partial_size--;
			}
		
			$right_inset = $found_right_inset-$expand_right if ($found_right_inset-$expand_right > $right_inset);
			$test_right_inset++;
		}
	}

	
	
	return ($left_inset, $right_inset);
}


sub _ambiguous_end_offsets_from_sequence
{
	my ($ref_string) = @_;
	
	#test longest substrings
	my $left_inset = 0;
	my $right_inset = 0;
	
	{ #left side
		my $test_length = int((length $ref_string) / 2);
		my $match_found = 0;
		my $test_end_string;
		while ( ($test_length > 0) && !$match_found)
		{	
			$test_end_string = substr $ref_string, $left_inset, $test_length;
			#force removal of end nucleotides if no higher order repeats found
			$left_inset-- if ($test_length == 1);			
			my $test_interior_string = substr $ref_string, $left_inset+$test_length, $test_length;
			
			while ($test_end_string eq $test_interior_string)
			{
				$match_found = 1;
				$left_inset += $test_length;
				$test_interior_string = substr $ref_string, $left_inset+$test_length, $test_length;
			}
			$left_inset += $test_length if ($match_found);
			$test_length--;
		} 
		
		#test partial matches (that continue part of repeat further)
		 #note: already starts at one less than actual repeat size
		my $test_partial_size = $test_length; 
		while ($test_partial_size > 0)
		{
			my $test_partial_end_string = substr $test_end_string, 0, $test_partial_size;
			my $test_interior_string = substr $ref_string, $left_inset, $test_partial_size;
			if ($test_partial_end_string eq $test_interior_string)
			{
				$left_inset += $test_partial_size;
				last;
			}
			$test_partial_size--;
		}
		
	}
	
	{ #right side
		my $test_length = int((length $ref_string) / 2);
		my $match_found = 0;
		my $test_end_string;
		while ( ($test_length > 0) && !$match_found)
		{	
			$test_end_string = substr $ref_string, -$right_inset-$test_length, $test_length;
			#force removal of end nucleotides if no higher order repeats found
			$right_inset-- if ($test_length == 1);			
			my $test_interior_string = substr $ref_string, -$right_inset-2 * $test_length, $test_length;
						
			while ($test_end_string eq $test_interior_string)
			{
				$match_found = 1;
				$right_inset += $test_length;
				$test_interior_string = substr $ref_string, -$right_inset-2 * $test_length, $test_length;
			}
			$right_inset += $test_length if ($match_found);
			
			$test_length--;
		} 
		
		#test partial matches (that continue part of repeat further)
		 #note: already starts at one less than actual repeat size
		my $test_partial_size = $test_length; 
		while ($test_partial_size > 0)
		{
			my $test_partial_end_string = substr $test_end_string, -$test_partial_size, $test_partial_size;
			my $test_interior_string = substr $ref_string, -$right_inset-$test_partial_size, $test_partial_size;
			if ($test_partial_end_string eq $test_interior_string)
			{
				$right_inset += $test_partial_size;
				last;
			}
			$test_partial_size--;
		}
	}
	
	return ($left_inset, $right_inset);
}


sub _test_junction
{
	my ($key, $matched_junction_ref, $degenerate_matches_ref, $flanking_length, $minimum_best_score, $minimum_best_score_difference, $reference_fai, $ref_seq_info, $RREF, $reference_header, $RCJ, $candidate_junction_header) = @_;
	
	my @unique_matches = ();
	@unique_matches = @{$matched_junction_ref->{$key}} if (defined $matched_junction_ref->{$key});
	my @degenerate_matches = ();
	@degenerate_matches = map { $degenerate_matches_ref->{$key}->{$_} } sort keys %{$degenerate_matches_ref->{$key}} if (defined $degenerate_matches_ref->{$key});		
		
	my $failed = 0;
	print "Junction Candidate: $key Unique Matches: " . (scalar @unique_matches) . " Degenerate Matches: " . (scalar @degenerate_matches) . "\n";

	#### TEST 1: 
#	my $minimum_number_of_reads_for_junction = 3;	
#	$failed = 1 if (scalar @unique_matches < $minimum_number_of_reads_for_junction);

	#### TEST 2: There should be at least one read that goes a certain number of bp into the nonoverlap sequence on each side of the junction
	my $alignment_on_each_side_cutoff = 14; #17
	my $alignment_on_each_side_cutoff_per_strand = 9; #10
	my $max_left_per_strand = { '0'=> 0, '1'=>0 };
	my $max_right_per_strand = { '0'=> 0, '1'=>0 };
	
	### we also need to count degenerate matches b/c sometimes ambiguity unfairly penalizes real reads...
	foreach my $item (@unique_matches, @degenerate_matches)
	{
		##there is only one dominant alignment for these matches at this point
		
		#If there were no degenerate matches, then we could just take the
		#one and only match in the 'dominant_alignments' array
		#my $a = $item->{dominant_alignments}->[0]; 
		
		## as it is, we must be sure we are looking at the one that matches
		my $a;
		DOMINANT_ALIGNMENT: foreach my $candidate_a (@{$item->{dominant_alignments}})
		{
			my $junction_id = $candidate_junction_header->target_name()->[$candidate_a->tid];
			if ($key eq $junction_id)
			{
				$a = $candidate_a;
				last DOMINANT_ALIGNMENT;
			}
		}
		die if (!defined $a);
		
		my $junction_id = $candidate_junction_header->target_name()->[$a->tid];
		my @split_key = Breseq::Shared::junction_name_split($junction_id);
 		my $overlap = $split_key[8];
		my $rev_key = ($a->reversed ? '1' : '0');

		my $this_left = $flanking_length;
		#positive overlap means part of this is in overlap region and should not be counted
		$this_left -= $overlap if ($overlap > 0);
		$this_left = $this_left - $a->start+1;

		my $this_right = $flanking_length+1;
		#negative overlap means we need to offset to get past the unique read sequence
		$this_right -= $overlap if ($overlap < 0);
		$this_right = $a->end - $this_right+1;
		print "  " . $a->start . "-" . $a->end . " " . $overlap . " " . $rev_key . "\n";
		print "  " . $item->{alignments}->[0]->qname . " LEFT: $this_left RIGHT: $this_right\n";


		$max_left_per_strand->{$rev_key} = $this_left if ($max_left_per_strand->{$rev_key} < $this_left);
		$max_right_per_strand->{$rev_key} = $this_right if ($max_right_per_strand->{$rev_key} < $this_right);
	}				
	
	my $max_left = ($max_left_per_strand->{'0'} > $max_left_per_strand->{'1'}) ? $max_left_per_strand->{'0'} : $max_left_per_strand->{'1'};
	my $max_right = ($max_right_per_strand->{'0'} > $max_right_per_strand->{'1'}) ? $max_right_per_strand->{'0'} : $max_right_per_strand->{'1'};

	print "Max left = $max_left_per_strand->{0}, reversed = $max_left_per_strand->{1}\n";
	print "Max Right = $max_right_per_strand->{0}, reversed = $max_right_per_strand->{1}\n";
	print "Max left = $max_left, Max right = $max_right\n";
	
	$failed = ($max_left < $alignment_on_each_side_cutoff) || ($max_right < $alignment_on_each_side_cutoff)
	       || ($max_left_per_strand->{'0'} < $alignment_on_each_side_cutoff_per_strand) || ($max_left_per_strand->{'1'} < $alignment_on_each_side_cutoff_per_strand)
	       || ($max_right_per_strand->{'0'} < $alignment_on_each_side_cutoff_per_strand) || ($max_right_per_strand->{'1'} < $alignment_on_each_side_cutoff_per_strand);

	#### TEST 3: Overlap should not be biased such that one side of the junction often has more of the
	####         read overlapping it than the other. Use a sign test.
	####
	####   >>>>  But it's really not this simple since reads might be biased by the sequences they match???	
	
	## to implement
	
	
	### If we passed all the tests, or we were only testing degenerate junctions
	### add degenerate matches and make them unavailable for other junctions
	
	##degenerate matches is a hash of junction_ids of read_names
	if (!$failed || !scalar (@unique_matches == 0))
	{
		if (defined $degenerate_matches_ref->{$key})
		{
			foreach my $read_name (keys %{$degenerate_matches_ref->{$key}})
			{
				my $degenerate_match = $degenerate_matches_ref->{$key}->{$read_name};
				my $matched_alignment;
			
				#purge all references to this from the degenerate match hash
				#so that they will not be counted for other junctions
				foreach my $a (@{$degenerate_match->{dominant_alignments}})
				{
					my $junction_id = $candidate_junction_header->target_name()->[$a->tid];
					$matched_alignment = $a if ($key eq $junction_id);
					delete $degenerate_matches_ref->{$junction_id}->{$read_name};
					
					if (scalar keys %{$degenerate_matches_ref->{$junction_id}} == 0)
					{
						delete $degenerate_matches_ref->{$junction_id};
					}
				}
			
				## add to the matched_junction
				@{$degenerate_match->{dominant_alignments}} = ($matched_alignment);
				push @{$matched_junction_ref->{$key}}, $degenerate_match;
			}
		}
	}
	
	foreach my $junction_read (@{$matched_junction_ref->{$key}})
	{
		my $fastq_file_index = $junction_read->{fastq_file_index};
		
		if ($failed && (scalar @unique_matches > 0))
		{						
			my $this_reference_al = $junction_read->{reference_alignments};
			_write_reference_matches($minimum_best_score, $minimum_best_score_difference, $reference_fai, $ref_seq_info, $RREF, $reference_header, $fastq_file_index, @$this_reference_al);
		}
		else
		{
			my @this_dominant_candidate_junction_al = @{$junction_read->{dominant_alignments}}; 
			Breseq::Shared::tam_write_read_alignments($RCJ, $candidate_junction_header, $fastq_file_index, \@this_dominant_candidate_junction_al);
			$RCJ->flush();
		}
	}
	
	print "FAILED!!\n" if ($failed);
	
	return $failed;
}


return 1;