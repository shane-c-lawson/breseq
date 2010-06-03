#!/bin/bash

SELF=`dirname ${BASH_SOURCE}`
GENBANK=${SELF}/../data/REL606/REL606.fragment.gbk
MUTATED=${SELF}/REL606.mutated.fna
OUTPUT=${SELF}/REL606.fragment.1.fastq


## This script runs commands to generate the fastq data for this test
## Add new mutations to make the test more comprehensive

## first convert the reference Genbank to a FASTA
genbank_utils.pl FASTA -o ${MUTATED} ${GENBANK}


## In general mutate higher positions first...
## especially important when introducing the target site duplications adjacent to IS elements



##
## IS insertions
##
## There are 2 IS1 elements in this fragment Coords: 9767-10534 and 31302-32069
##
## Reserved for positions 40,000 and higher

## Flush IS insertion on + strand, 3 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 45000-0-GGTGATGCTGCCAACTTACTGATTTAGTGTATGATGGTGTTTTTGAGGTGCTCCAGTGGCTTCTGTTTCTATCAGCTGTCCCTCCTGTTCAGCTACTGACGGGGTGGTGCGTAACGGCAAAAGCACCGCCGGACATCAGCGCTATCTCTGCTCTCACTGCCGTAAAACATGGCAACTGCAGTTCACTTACACCGCTTCTCAACCCGGTACGCACCAGAAAATCATTGATATGGCCATGAATGGCGTTGGATGCCGGGCAACTGCCCGCATTATGGGCGTTGGCCTCAACACGATTTTACGTCACTTAAAAAACTCAGGCCGCAGTCGGTAACCTCGCGCATACAGCCGGGCAGTGACGTCATCGTCTGCGCGGAAATGGACGAACAGTGGGGCTATGTCGGGGCTAAATCGCGCCAGCGCTGGCTGTTTTACGCGTATGACAGTCTCCGGAAGACGGTTGTTGCGCACGTATTCGGTGAACGCACTATGGCGACGCTGGGGCGTCTTATGAGCCTGCTGTCACCCTTTGACGTGGTGATATGGATGACGGATGGCTGGCCGCTGTATGAATCCCGCCTGAAGGGAAAGCTGCACGTAATCAGCAAGCGATATACGCAGCGAATTGAGCGGCATAACCTGAATCTGAGGCAGCACCTGGCACGGCTGGGACGGAAGTCGCTGTCGTTCTCAAAATCGGTGGAGCTGCATGACAAAGTCATCGGGCATTATCTGAACATAAAACACTATCAATAAGTTGGAGTCATTACC
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 45000-0-CAT

## Flush IS insertion on - strand, 5 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 44800-0-GGTAATGACTCCAACTTATTGATAGTGTTTTATGTTCAGATAATGCCCGATGACTTTGTCATGCAGCTCCACCGATTTTGAGAACGACAGCGACTTCCGTCCCAGCCGTGCCAGGTGCTGCCTCAGATTCAGGTTATGCCGCTCAATTCGCTGCGTATATCGCTTGCTGATTACGTGCAGCTTTCCCTTCAGGCGGGATTCATACAGCGGCCAGCCATCCGTCATCCATATCACCACGTCAAAGGGTGACAGCAGGCTCATAAGACGCCCCAGCGTCGCCATAGTGCGTTCACCGAATACGTGCGCAACAACCGTCTTCCGGAGACTGTCATACGCGTAAAACAGCCAGCGCTGGCGCGATTTAGCCCCGACATAGCCCCACTGTTCGTCCATTTCCGCGCAGACGATGACGTCACTGCCCGGCTGTATGCGCGAGGTTACCGACTGCGGCCTGAGTTTTTTAAGTGACGTAAAATCGTGTTGAGGCCAACGCCCATAATGCGGGCAGTTGCCCGGCATCCAACGCCATTCATGGCCATATCAATGATTTTCTGGTGCGTACCGGGTTGAGAAGCGGTGTAAGTGAACTGCAGTTGCCATGTTTTACGGCAGTGAGAGCAGAGATAGCGCTGATGTCCGGCGGTGCTTTTGCCGTTACGCACCACCCCGTCAGTAGCTGAACAGGAGGGACAGCTGATAGAAACAGAAGCCACTGGAGCACCTCAAAAACACCATCATACACTAAATCAGTAAGTTGGCAGCATCACC
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 44800-0-GGTAC

## IS insertion on + strand, missing first base of IS element, 4 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 44600-0-GTGATGCTGCCAACTTACTGATTTAGTGTATGATGGTGTTTTTGAGGTGCTCCAGTGGCTTCTGTTTCTATCAGCTGTCCCTCCTGTTCAGCTACTGACGGGGTGGTGCGTAACGGCAAAAGCACCGCCGGACATCAGCGCTATCTCTGCTCTCACTGCCGTAAAACATGGCAACTGCAGTTCACTTACACCGCTTCTCAACCCGGTACGCACCAGAAAATCATTGATATGGCCATGAATGGCGTTGGATGCCGGGCAACTGCCCGCATTATGGGCGTTGGCCTCAACACGATTTTACGTCACTTAAAAAACTCAGGCCGCAGTCGGTAACCTCGCGCATACAGCCGGGCAGTGACGTCATCGTCTGCGCGGAAATGGACGAACAGTGGGGCTATGTCGGGGCTAAATCGCGCCAGCGCTGGCTGTTTTACGCGTATGACAGTCTCCGGAAGACGGTTGTTGCGCACGTATTCGGTGAACGCACTATGGCGACGCTGGGGCGTCTTATGAGCCTGCTGTCACCCTTTGACGTGGTGATATGGATGACGGATGGCTGGCCGCTGTATGAATCCCGCCTGAAGGGAAAGCTGCACGTAATCAGCAAGCGATATACGCAGCGAATTGAGCGGCATAACCTGAATCTGAGGCAGCACCTGGCACGGCTGGGACGGAAGTCGCTGTCGTTCTCAAAATCGGTGGAGCTGCATGACAAAGTCATCGGGCATTATCTGAACATAAAACACTATCAATAAGTTGGAGTCATTACC
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 44600-0-AACT

## IS insertion on + strand, missing last 2 bases of IS element, 3 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 44400-0-GGTGATGCTGCCAACTTACTGATTTAGTGTATGATGGTGTTTTTGAGGTGCTCCAGTGGCTTCTGTTTCTATCAGCTGTCCCTCCTGTTCAGCTACTGACGGGGTGGTGCGTAACGGCAAAAGCACCGCCGGACATCAGCGCTATCTCTGCTCTCACTGCCGTAAAACATGGCAACTGCAGTTCACTTACACCGCTTCTCAACCCGGTACGCACCAGAAAATCATTGATATGGCCATGAATGGCGTTGGATGCCGGGCAACTGCCCGCATTATGGGCGTTGGCCTCAACACGATTTTACGTCACTTAAAAAACTCAGGCCGCAGTCGGTAACCTCGCGCATACAGCCGGGCAGTGACGTCATCGTCTGCGCGGAAATGGACGAACAGTGGGGCTATGTCGGGGCTAAATCGCGCCAGCGCTGGCTGTTTTACGCGTATGACAGTCTCCGGAAGACGGTTGTTGCGCACGTATTCGGTGAACGCACTATGGCGACGCTGGGGCGTCTTATGAGCCTGCTGTCACCCTTTGACGTGGTGATATGGATGACGGATGGCTGGCCGCTGTATGAATCCCGCCTGAAGGGAAAGCTGCACGTAATCAGCAAGCGATATACGCAGCGAATTGAGCGGCATAACCTGAATCTGAGGCAGCACCTGGCACGGCTGGGACGGAAGTCGCTGTCGTTCTCAAAATCGGTGGAGCTGCATGACAAAGTCATCGGGCATTATCTGAACATAAAACACTATCAATAAGTTGGAGTCATTA
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 44400-0-AGA

## IS insertion on - strand, missing first 2 bases of IS element, 4 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 44220-0-GGTAATGACTCCAACTTATTGATAGTGTTTTATGTTCAGATAATGCCCGATGACTTTGTCATGCAGCTCCACCGATTTTGAGAACGACAGCGACTTCCGTCCCAGCCGTGCCAGGTGCTGCCTCAGATTCAGGTTATGCCGCTCAATTCGCTGCGTATATCGCTTGCTGATTACGTGCAGCTTTCCCTTCAGGCGGGATTCATACAGCGGCCAGCCATCCGTCATCCATATCACCACGTCAAAGGGTGACAGCAGGCTCATAAGACGCCCCAGCGTCGCCATAGTGCGTTCACCGAATACGTGCGCAACAACCGTCTTCCGGAGACTGTCATACGCGTAAAACAGCCAGCGCTGGCGCGATTTAGCCCCGACATAGCCCCACTGTTCGTCCATTTCCGCGCAGACGATGACGTCACTGCCCGGCTGTATGCGCGAGGTTACCGACTGCGGCCTGAGTTTTTTAAGTGACGTAAAATCGTGTTGAGGCCAACGCCCATAATGCGGGCAGTTGCCCGGCATCCAACGCCATTCATGGCCATATCAATGATTTTCTGGTGCGTACCGGGTTGAGAAGCGGTGTAAGTGAACTGCAGTTGCCATGTTTTACGGCAGTGAGAGCAGAGATAGCGCTGATGTCCGGCGGTGCTTTTGCCGTTACGCACCACCCCGTCAGTAGCTGAACAGGAGGGACAGCTGATAGAAACAGAAGCCACTGGAGCACCTCAAAAACACCATCATACACTAAATCAGTAAGTTGGCAGCATCA
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 44220-0-GGTC

## IS insertion on - strand, missing last base of IS element, 2 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 44000-0-GTAATGACTCCAACTTATTGATAGTGTTTTATGTTCAGATAATGCCCGATGACTTTGTCATGCAGCTCCACCGATTTTGAGAACGACAGCGACTTCCGTCCCAGCCGTGCCAGGTGCTGCCTCAGATTCAGGTTATGCCGCTCAATTCGCTGCGTATATCGCTTGCTGATTACGTGCAGCTTTCCCTTCAGGCGGGATTCATACAGCGGCCAGCCATCCGTCATCCATATCACCACGTCAAAGGGTGACAGCAGGCTCATAAGACGCCCCAGCGTCGCCATAGTGCGTTCACCGAATACGTGCGCAACAACCGTCTTCCGGAGACTGTCATACGCGTAAAACAGCCAGCGCTGGCGCGATTTAGCCCCGACATAGCCCCACTGTTCGTCCATTTCCGCGCAGACGATGACGTCACTGCCCGGCTGTATGCGCGAGGTTACCGACTGCGGCCTGAGTTTTTTAAGTGACGTAAAATCGTGTTGAGGCCAACGCCCATAATGCGGGCAGTTGCCCGGCATCCAACGCCATTCATGGCCATATCAATGATTTTCTGGTGCGTACCGGGTTGAGAAGCGGTGTAAGTGAACTGCAGTTGCCATGTTTTACGGCAGTGAGAGCAGAGATAGCGCTGATGTCCGGCGGTGCTTTTGCCGTTACGCACCACCCCGTCAGTAGCTGAACAGGAGGGACAGCTGATAGAAACAGAAGCCACTGGAGCACCTCAAAAACACCATCATACACTAAATCAGTAAGTTGGCAGCATCACC
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 44000-0-TA

## IS insertion on + strand, novel base upstream of IS element, 4 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 43800-0-AGGTGATGCTGCCAACTTACTGATTTAGTGTATGATGGTGTTTTTGAGGTGCTCCAGTGGCTTCTGTTTCTATCAGCTGTCCCTCCTGTTCAGCTACTGACGGGGTGGTGCGTAACGGCAAAAGCACCGCCGGACATCAGCGCTATCTCTGCTCTCACTGCCGTAAAACATGGCAACTGCAGTTCACTTACACCGCTTCTCAACCCGGTACGCACCAGAAAATCATTGATATGGCCATGAATGGCGTTGGATGCCGGGCAACTGCCCGCATTATGGGCGTTGGCCTCAACACGATTTTACGTCACTTAAAAAACTCAGGCCGCAGTCGGTAACCTCGCGCATACAGCCGGGCAGTGACGTCATCGTCTGCGCGGAAATGGACGAACAGTGGGGCTATGTCGGGGCTAAATCGCGCCAGCGCTGGCTGTTTTACGCGTATGACAGTCTCCGGAAGACGGTTGTTGCGCACGTATTCGGTGAACGCACTATGGCGACGCTGGGGCGTCTTATGAGCCTGCTGTCACCCTTTGACGTGGTGATATGGATGACGGATGGCTGGCCGCTGTATGAATCCCGCCTGAAGGGAAAGCTGCACGTAATCAGCAAGCGATATACGCAGCGAATTGAGCGGCATAACCTGAATCTGAGGCAGCACCTGGCACGGCTGGGACGGAAGTCGCTGTCGTTCTCAAAATCGGTGGAGCTGCATGACAAAGTCATCGGGCATTATCTGAACATAAAACACTATCAATAAGTTGGAGTCATTACC
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 43800-0-AGAT

## IS insertion on + strand, novel 2 bases downstream of IS element, 3 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 43600-0-GGTGATGCTGCCAACTTACTGATTTAGTGTATGATGGTGTTTTTGAGGTGCTCCAGTGGCTTCTGTTTCTATCAGCTGTCCCTCCTGTTCAGCTACTGACGGGGTGGTGCGTAACGGCAAAAGCACCGCCGGACATCAGCGCTATCTCTGCTCTCACTGCCGTAAAACATGGCAACTGCAGTTCACTTACACCGCTTCTCAACCCGGTACGCACCAGAAAATCATTGATATGGCCATGAATGGCGTTGGATGCCGGGCAACTGCCCGCATTATGGGCGTTGGCCTCAACACGATTTTACGTCACTTAAAAAACTCAGGCCGCAGTCGGTAACCTCGCGCATACAGCCGGGCAGTGACGTCATCGTCTGCGCGGAAATGGACGAACAGTGGGGCTATGTCGGGGCTAAATCGCGCCAGCGCTGGCTGTTTTACGCGTATGACAGTCTCCGGAAGACGGTTGTTGCGCACGTATTCGGTGAACGCACTATGGCGACGCTGGGGCGTCTTATGAGCCTGCTGTCACCCTTTGACGTGGTGATATGGATGACGGATGGCTGGCCGCTGTATGAATCCCGCCTGAAGGGAAAGCTGCACGTAATCAGCAAGCGATATACGCAGCGAATTGAGCGGCATAACCTGAATCTGAGGCAGCACCTGGCACGGCTGGGACGGAAGTCGCTGTCGTTCTCAAAATCGGTGGAGCTGCATGACAAAGTCATCGGGCATTATCTGAACATAAAACACTATCAATAAGTTGGAGTCATTACCT
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 43600-0-TAA

## IS insertion on - strand, novel 2 bases upstream of IS element, 4 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 43400-0-TAGGTAATGACTCCAACTTATTGATAGTGTTTTATGTTCAGATAATGCCCGATGACTTTGTCATGCAGCTCCACCGATTTTGAGAACGACAGCGACTTCCGTCCCAGCCGTGCCAGGTGCTGCCTCAGATTCAGGTTATGCCGCTCAATTCGCTGCGTATATCGCTTGCTGATTACGTGCAGCTTTCCCTTCAGGCGGGATTCATACAGCGGCCAGCCATCCGTCATCCATATCACCACGTCAAAGGGTGACAGCAGGCTCATAAGACGCCCCAGCGTCGCCATAGTGCGTTCACCGAATACGTGCGCAACAACCGTCTTCCGGAGACTGTCATACGCGTAAAACAGCCAGCGCTGGCGCGATTTAGCCCCGACATAGCCCCACTGTTCGTCCATTTCCGCGCAGACGATGACGTCACTGCCCGGCTGTATGCGCGAGGTTACCGACTGCGGCCTGAGTTTTTTAAGTGACGTAAAATCGTGTTGAGGCCAACGCCCATAATGCGGGCAGTTGCCCGGCATCCAACGCCATTCATGGCCATATCAATGATTTTCTGGTGCGTACCGGGTTGAGAAGCGGTGTAAGTGAACTGCAGTTGCCATGTTTTACGGCAGTGAGAGCAGAGATAGCGCTGATGTCCGGCGGTGCTTTTGCCGTTACGCACCACCCCGTCAGTAGCTGAACAGGAGGGACAGCTGATAGAAACAGAAGCCACTGGAGCACCTCAAAAACACCATCATACACTAAATCAGTAAGTTGGCAGCATCACC
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 43400-0-TCCT

## IS insertion on - strand, novel base downstream of IS element, 2 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 43200-0-GGTAATGACTCCAACTTATTGATAGTGTTTTATGTTCAGATAATGCCCGATGACTTTGTCATGCAGCTCCACCGATTTTGAGAACGACAGCGACTTCCGTCCCAGCCGTGCCAGGTGCTGCCTCAGATTCAGGTTATGCCGCTCAATTCGCTGCGTATATCGCTTGCTGATTACGTGCAGCTTTCCCTTCAGGCGGGATTCATACAGCGGCCAGCCATCCGTCATCCATATCACCACGTCAAAGGGTGACAGCAGGCTCATAAGACGCCCCAGCGTCGCCATAGTGCGTTCACCGAATACGTGCGCAACAACCGTCTTCCGGAGACTGTCATACGCGTAAAACAGCCAGCGCTGGCGCGATTTAGCCCCGACATAGCCCCACTGTTCGTCCATTTCCGCGCAGACGATGACGTCACTGCCCGGCTGTATGCGCGAGGTTACCGACTGCGGCCTGAGTTTTTTAAGTGACGTAAAATCGTGTTGAGGCCAACGCCCATAATGCGGGCAGTTGCCCGGCATCCAACGCCATTCATGGCCATATCAATGATTTTCTGGTGCGTACCGGGTTGAGAAGCGGTGTAAGTGAACTGCAGTTGCCATGTTTTACGGCAGTGAGAGCAGAGATAGCGCTGATGTCCGGCGGTGCTTTTGCCGTTACGCACCACCCCGTCAGTAGCTGAACAGGAGGGACAGCTGATAGAAACAGAAGCCACTGGAGCACCTCAAAAACACCATCATACACTAAATCAGTAAGTTGGCAGCATCACCT
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 43200-0-GG

##
## SNPs
##

## Adjacent SNPs spanning two codons
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 15000-1-G
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 15001-1-G
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 15002-1-G
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 15003-1-G

## Some random SNPs
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 12000-1-C
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 13000-1-C
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 14000-1-A


##
## Deletions
##

## One base deletion
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 25000-1-.
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 12100-0-GTG

#large deletion with inserted nucleotides in place
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 34000-2000-ATGA

#medium sized deletion
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 29000-134-.

## Flush IS insertion on + strand, 3 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 5000-0-GGTGATGCTGCCAACTTACTGATTTAGTGTATGATGGTGTTTTTGAGGTGCTCCAGTGGCTTCTGTTTCTATCAGCTGTCCCTCCTGTTCAGCTACTGACGGGGTGGTGCGTAACGGCAAAAGCACCGCCGGACATCAGCGCTATCTCTGCTCTCACTGCCGTAAAACATGGCAACTGCAGTTCACTTACACCGCTTCTCAACCCGGTACGCACCAGAAAATCATTGATATGGCCATGAATGGCGTTGGATGCCGGGCAACTGCCCGCATTATGGGCGTTGGCCTCAACACGATTTTACGTCACTTAAAAAACTCAGGCCGCAGTCGGTAACCTCGCGCATACAGCCGGGCAGTGACGTCATCGTCTGCGCGGAAATGGACGAACAGTGGGGCTATGTCGGGGCTAAATCGCGCCAGCGCTGGCTGTTTTACGCGTATGACAGTCTCCGGAAGACGGTTGTTGCGCACGTATTCGGTGAACGCACTATGGCGACGCTGGGGCGTCTTATGAGCCTGCTGTCACCCTTTGACGTGGTGATATGGATGACGGATGGCTGGCCGCTGTATGAATCCCGCCTGAAGGGAAAGCTGCACGTAATCAGCAAGCGATATACGCAGCGAATTGAGCGGCATAACCTGAATCTGAGGCAGCACCTGGCACGGCTGGGACGGAAGTCGCTGTCGTTCTCAAAATCGGTGGAGCTGCATGACAAAGTCATCGGGCATTATCTGAACATAAAACACTATCAATAAGTTGGAGTCATTACC
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 5000-0-AGG

## Flush IS insertion on - strand, 5 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 4800-0-GGTAATGACTCCAACTTATTGATAGTGTTTTATGTTCAGATAATGCCCGATGACTTTGTCATGCAGCTCCACCGATTTTGAGAACGACAGCGACTTCCGTCCCAGCCGTGCCAGGTGCTGCCTCAGATTCAGGTTATGCCGCTCAATTCGCTGCGTATATCGCTTGCTGATTACGTGCAGCTTTCCCTTCAGGCGGGATTCATACAGCGGCCAGCCATCCGTCATCCATATCACCACGTCAAAGGGTGACAGCAGGCTCATAAGACGCCCCAGCGTCGCCATAGTGCGTTCACCGAATACGTGCGCAACAACCGTCTTCCGGAGACTGTCATACGCGTAAAACAGCCAGCGCTGGCGCGATTTAGCCCCGACATAGCCCCACTGTTCGTCCATTTCCGCGCAGACGATGACGTCACTGCCCGGCTGTATGCGCGAGGTTACCGACTGCGGCCTGAGTTTTTTAAGTGACGTAAAATCGTGTTGAGGCCAACGCCCATAATGCGGGCAGTTGCCCGGCATCCAACGCCATTCATGGCCATATCAATGATTTTCTGGTGCGTACCGGGTTGAGAAGCGGTGTAAGTGAACTGCAGTTGCCATGTTTTACGGCAGTGAGAGCAGAGATAGCGCTGATGTCCGGCGGTGCTTTTGCCGTTACGCACCACCCCGTCAGTAGCTGAACAGGAGGGACAGCTGATAGAAACAGAAGCCACTGGAGCACCTCAAAAACACCATCATACACTAAATCAGTAAGTTGGCAGCATCACC
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 4800-0-CTGTT

## IS insertion on + strand, missing first base of IS element, 4 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 4600-0-GTGATGCTGCCAACTTACTGATTTAGTGTATGATGGTGTTTTTGAGGTGCTCCAGTGGCTTCTGTTTCTATCAGCTGTCCCTCCTGTTCAGCTACTGACGGGGTGGTGCGTAACGGCAAAAGCACCGCCGGACATCAGCGCTATCTCTGCTCTCACTGCCGTAAAACATGGCAACTGCAGTTCACTTACACCGCTTCTCAACCCGGTACGCACCAGAAAATCATTGATATGGCCATGAATGGCGTTGGATGCCGGGCAACTGCCCGCATTATGGGCGTTGGCCTCAACACGATTTTACGTCACTTAAAAAACTCAGGCCGCAGTCGGTAACCTCGCGCATACAGCCGGGCAGTGACGTCATCGTCTGCGCGGAAATGGACGAACAGTGGGGCTATGTCGGGGCTAAATCGCGCCAGCGCTGGCTGTTTTACGCGTATGACAGTCTCCGGAAGACGGTTGTTGCGCACGTATTCGGTGAACGCACTATGGCGACGCTGGGGCGTCTTATGAGCCTGCTGTCACCCTTTGACGTGGTGATATGGATGACGGATGGCTGGCCGCTGTATGAATCCCGCCTGAAGGGAAAGCTGCACGTAATCAGCAAGCGATATACGCAGCGAATTGAGCGGCATAACCTGAATCTGAGGCAGCACCTGGCACGGCTGGGACGGAAGTCGCTGTCGTTCTCAAAATCGGTGGAGCTGCATGACAAAGTCATCGGGCATTATCTGAACATAAAACACTATCAATAAGTTGGAGTCATTACC
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 4600-0-GCAC

## IS insertion on + strand, missing last 2 bases of IS element, 3 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 4400-0-GGTGATGCTGCCAACTTACTGATTTAGTGTATGATGGTGTTTTTGAGGTGCTCCAGTGGCTTCTGTTTCTATCAGCTGTCCCTCCTGTTCAGCTACTGACGGGGTGGTGCGTAACGGCAAAAGCACCGCCGGACATCAGCGCTATCTCTGCTCTCACTGCCGTAAAACATGGCAACTGCAGTTCACTTACACCGCTTCTCAACCCGGTACGCACCAGAAAATCATTGATATGGCCATGAATGGCGTTGGATGCCGGGCAACTGCCCGCATTATGGGCGTTGGCCTCAACACGATTTTACGTCACTTAAAAAACTCAGGCCGCAGTCGGTAACCTCGCGCATACAGCCGGGCAGTGACGTCATCGTCTGCGCGGAAATGGACGAACAGTGGGGCTATGTCGGGGCTAAATCGCGCCAGCGCTGGCTGTTTTACGCGTATGACAGTCTCCGGAAGACGGTTGTTGCGCACGTATTCGGTGAACGCACTATGGCGACGCTGGGGCGTCTTATGAGCCTGCTGTCACCCTTTGACGTGGTGATATGGATGACGGATGGCTGGCCGCTGTATGAATCCCGCCTGAAGGGAAAGCTGCACGTAATCAGCAAGCGATATACGCAGCGAATTGAGCGGCATAACCTGAATCTGAGGCAGCACCTGGCACGGCTGGGACGGAAGTCGCTGTCGTTCTCAAAATCGGTGGAGCTGCATGACAAAGTCATCGGGCATTATCTGAACATAAAACACTATCAATAAGTTGGAGTCATTA
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 4400-0-CGT

## IS insertion on - strand, missing first 2 bases of IS element, 4 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 4200-0-GGTAATGACTCCAACTTATTGATAGTGTTTTATGTTCAGATAATGCCCGATGACTTTGTCATGCAGCTCCACCGATTTTGAGAACGACAGCGACTTCCGTCCCAGCCGTGCCAGGTGCTGCCTCAGATTCAGGTTATGCCGCTCAATTCGCTGCGTATATCGCTTGCTGATTACGTGCAGCTTTCCCTTCAGGCGGGATTCATACAGCGGCCAGCCATCCGTCATCCATATCACCACGTCAAAGGGTGACAGCAGGCTCATAAGACGCCCCAGCGTCGCCATAGTGCGTTCACCGAATACGTGCGCAACAACCGTCTTCCGGAGACTGTCATACGCGTAAAACAGCCAGCGCTGGCGCGATTTAGCCCCGACATAGCCCCACTGTTCGTCCATTTCCGCGCAGACGATGACGTCACTGCCCGGCTGTATGCGCGAGGTTACCGACTGCGGCCTGAGTTTTTTAAGTGACGTAAAATCGTGTTGAGGCCAACGCCCATAATGCGGGCAGTTGCCCGGCATCCAACGCCATTCATGGCCATATCAATGATTTTCTGGTGCGTACCGGGTTGAGAAGCGGTGTAAGTGAACTGCAGTTGCCATGTTTTACGGCAGTGAGAGCAGAGATAGCGCTGATGTCCGGCGGTGCTTTTGCCGTTACGCACCACCCCGTCAGTAGCTGAACAGGAGGGACAGCTGATAGAAACAGAAGCCACTGGAGCACCTCAAAAACACCATCATACACTAAATCAGTAAGTTGGCAGCATCA
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 4200-0-CTGT

## IS insertion on - strand, missing last base of IS element, 2 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 4000-0-GTAATGACTCCAACTTATTGATAGTGTTTTATGTTCAGATAATGCCCGATGACTTTGTCATGCAGCTCCACCGATTTTGAGAACGACAGCGACTTCCGTCCCAGCCGTGCCAGGTGCTGCCTCAGATTCAGGTTATGCCGCTCAATTCGCTGCGTATATCGCTTGCTGATTACGTGCAGCTTTCCCTTCAGGCGGGATTCATACAGCGGCCAGCCATCCGTCATCCATATCACCACGTCAAAGGGTGACAGCAGGCTCATAAGACGCCCCAGCGTCGCCATAGTGCGTTCACCGAATACGTGCGCAACAACCGTCTTCCGGAGACTGTCATACGCGTAAAACAGCCAGCGCTGGCGCGATTTAGCCCCGACATAGCCCCACTGTTCGTCCATTTCCGCGCAGACGATGACGTCACTGCCCGGCTGTATGCGCGAGGTTACCGACTGCGGCCTGAGTTTTTTAAGTGACGTAAAATCGTGTTGAGGCCAACGCCCATAATGCGGGCAGTTGCCCGGCATCCAACGCCATTCATGGCCATATCAATGATTTTCTGGTGCGTACCGGGTTGAGAAGCGGTGTAAGTGAACTGCAGTTGCCATGTTTTACGGCAGTGAGAGCAGAGATAGCGCTGATGTCCGGCGGTGCTTTTGCCGTTACGCACCACCCCGTCAGTAGCTGAACAGGAGGGACAGCTGATAGAAACAGAAGCCACTGGAGCACCTCAAAAACACCATCATACACTAAATCAGTAAGTTGGCAGCATCACC
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 4000-0-GG

## IS insertion on + strand, novel base upstream of IS element, 4 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 3800-0-AGGTGATGCTGCCAACTTACTGATTTAGTGTATGATGGTGTTTTTGAGGTGCTCCAGTGGCTTCTGTTTCTATCAGCTGTCCCTCCTGTTCAGCTACTGACGGGGTGGTGCGTAACGGCAAAAGCACCGCCGGACATCAGCGCTATCTCTGCTCTCACTGCCGTAAAACATGGCAACTGCAGTTCACTTACACCGCTTCTCAACCCGGTACGCACCAGAAAATCATTGATATGGCCATGAATGGCGTTGGATGCCGGGCAACTGCCCGCATTATGGGCGTTGGCCTCAACACGATTTTACGTCACTTAAAAAACTCAGGCCGCAGTCGGTAACCTCGCGCATACAGCCGGGCAGTGACGTCATCGTCTGCGCGGAAATGGACGAACAGTGGGGCTATGTCGGGGCTAAATCGCGCCAGCGCTGGCTGTTTTACGCGTATGACAGTCTCCGGAAGACGGTTGTTGCGCACGTATTCGGTGAACGCACTATGGCGACGCTGGGGCGTCTTATGAGCCTGCTGTCACCCTTTGACGTGGTGATATGGATGACGGATGGCTGGCCGCTGTATGAATCCCGCCTGAAGGGAAAGCTGCACGTAATCAGCAAGCGATATACGCAGCGAATTGAGCGGCATAACCTGAATCTGAGGCAGCACCTGGCACGGCTGGGACGGAAGTCGCTGTCGTTCTCAAAATCGGTGGAGCTGCATGACAAAGTCATCGGGCATTATCTGAACATAAAACACTATCAATAAGTTGGAGTCATTACC
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 3800-0-AGAT

## IS insertion on + strand, novel 2 bases downstream of IS element, 3 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 3600-0-GGTGATGCTGCCAACTTACTGATTTAGTGTATGATGGTGTTTTTGAGGTGCTCCAGTGGCTTCTGTTTCTATCAGCTGTCCCTCCTGTTCAGCTACTGACGGGGTGGTGCGTAACGGCAAAAGCACCGCCGGACATCAGCGCTATCTCTGCTCTCACTGCCGTAAAACATGGCAACTGCAGTTCACTTACACCGCTTCTCAACCCGGTACGCACCAGAAAATCATTGATATGGCCATGAATGGCGTTGGATGCCGGGCAACTGCCCGCATTATGGGCGTTGGCCTCAACACGATTTTACGTCACTTAAAAAACTCAGGCCGCAGTCGGTAACCTCGCGCATACAGCCGGGCAGTGACGTCATCGTCTGCGCGGAAATGGACGAACAGTGGGGCTATGTCGGGGCTAAATCGCGCCAGCGCTGGCTGTTTTACGCGTATGACAGTCTCCGGAAGACGGTTGTTGCGCACGTATTCGGTGAACGCACTATGGCGACGCTGGGGCGTCTTATGAGCCTGCTGTCACCCTTTGACGTGGTGATATGGATGACGGATGGCTGGCCGCTGTATGAATCCCGCCTGAAGGGAAAGCTGCACGTAATCAGCAAGCGATATACGCAGCGAATTGAGCGGCATAACCTGAATCTGAGGCAGCACCTGGCACGGCTGGGACGGAAGTCGCTGTCGTTCTCAAAATCGGTGGAGCTGCATGACAAAGTCATCGGGCATTATCTGAACATAAAACACTATCAATAAGTTGGAGTCATTACCT
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 3600-0-CTG

## IS insertion on - strand, novel 2 bases upstream of IS element, 4 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 3400-0-TAGGTAATGACTCCAACTTATTGATAGTGTTTTATGTTCAGATAATGCCCGATGACTTTGTCATGCAGCTCCACCGATTTTGAGAACGACAGCGACTTCCGTCCCAGCCGTGCCAGGTGCTGCCTCAGATTCAGGTTATGCCGCTCAATTCGCTGCGTATATCGCTTGCTGATTACGTGCAGCTTTCCCTTCAGGCGGGATTCATACAGCGGCCAGCCATCCGTCATCCATATCACCACGTCAAAGGGTGACAGCAGGCTCATAAGACGCCCCAGCGTCGCCATAGTGCGTTCACCGAATACGTGCGCAACAACCGTCTTCCGGAGACTGTCATACGCGTAAAACAGCCAGCGCTGGCGCGATTTAGCCCCGACATAGCCCCACTGTTCGTCCATTTCCGCGCAGACGATGACGTCACTGCCCGGCTGTATGCGCGAGGTTACCGACTGCGGCCTGAGTTTTTTAAGTGACGTAAAATCGTGTTGAGGCCAACGCCCATAATGCGGGCAGTTGCCCGGCATCCAACGCCATTCATGGCCATATCAATGATTTTCTGGTGCGTACCGGGTTGAGAAGCGGTGTAAGTGAACTGCAGTTGCCATGTTTTACGGCAGTGAGAGCAGAGATAGCGCTGATGTCCGGCGGTGCTTTTGCCGTTACGCACCACCCCGTCAGTAGCTGAACAGGAGGGACAGCTGATAGAAACAGAAGCCACTGGAGCACCTCAAAAACACCATCATACACTAAATCAGTAAGTTGGCAGCATCACC
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 3400-0-CGCA

## IS insertion on - strand, novel base downstream of IS element, 2 base target site duplication
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 3200-0-GGTAATGACTCCAACTTATTGATAGTGTTTTATGTTCAGATAATGCCCGATGACTTTGTCATGCAGCTCCACCGATTTTGAGAACGACAGCGACTTCCGTCCCAGCCGTGCCAGGTGCTGCCTCAGATTCAGGTTATGCCGCTCAATTCGCTGCGTATATCGCTTGCTGATTACGTGCAGCTTTCCCTTCAGGCGGGATTCATACAGCGGCCAGCCATCCGTCATCCATATCACCACGTCAAAGGGTGACAGCAGGCTCATAAGACGCCCCAGCGTCGCCATAGTGCGTTCACCGAATACGTGCGCAACAACCGTCTTCCGGAGACTGTCATACGCGTAAAACAGCCAGCGCTGGCGCGATTTAGCCCCGACATAGCCCCACTGTTCGTCCATTTCCGCGCAGACGATGACGTCACTGCCCGGCTGTATGCGCGAGGTTACCGACTGCGGCCTGAGTTTTTTAAGTGACGTAAAATCGTGTTGAGGCCAACGCCCATAATGCGGGCAGTTGCCCGGCATCCAACGCCATTCATGGCCATATCAATGATTTTCTGGTGCGTACCGGGTTGAGAAGCGGTGTAAGTGAACTGCAGTTGCCATGTTTTACGGCAGTGAGAGCAGAGATAGCGCTGATGTCCGGCGGTGCTTTTGCCGTTACGCACCACCCCGTCAGTAGCTGAACAGGAGGGACAGCTGATAGAAACAGAAGCCACTGGAGCACCTCAAAAACACCATCATACACTAAATCAGTAAGTTGGCAGCATCACCT
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 3200-0-TT


#20 nt tandem duplication at the beginning of the genome, which should
#result in different flanking lengths
genbank_utils.pl MUTATE -o ${MUTATED} ${MUTATED} 1-0-AAAACTTACAACGCCGAAGA

fastq_utils.pl SIMULATE -l 36 -c 35 -o ${OUTPUT} ${MUTATED}
