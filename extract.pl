#!/usr/bin/perl
# Matt Post <post@jhu.edu>

# This script extracts the Charniak & Johnson feature set from a
# single parse tree presented to it on STDIN.  It is a wrapper around
# Charniak and Johnson's extract-spfeatures program, which doesn't
# work unless you also provide gold standard parse trees and format
# the data in a very particular manner.

use strict;
use warnings;
use File::Temp qw/tempfile mktemp/;
use File::Basename;
use Getopt::Long;

# When this feature is activated, only features computable across a
# single hyperedge (including its head and tail nodes) will be
# computed.
my $do_local_only = 0;

# The minimum number of times a feature must be observed (across all
# parse trees in the set) for it to be output.  You'll want to
# increase this (to say 5) if you are processing a large batch of
# trees, but it is set to 1 so you'll see the full feature set if you
# pass in a single parse tree.
my $mincount = 1;

# The C&J feature extractor outputs features in SVM-light format,
# where each feature is a number followed by its value (delimited with
# a colon).  The map between feature ID and the string name of the
# feature is then output to STDERR.  If you'd like this script to undo
# this mapping and output string labels instead, use --undo-map.
my $undo_map = 0;

# The location of the extract program
my $extract = dirname($0) . "/extract-spfeatures";

# arguments: --min changes the feature pruning threshold (defaults to
# 1), while --local turns off non-local features
my $result = GetOptions("min=i" => \$mincount,
						"undo-map!"  => \$undo_map,
						"local" => \$do_local_only);

# read in the parses from STDIN.  two formats are permitted: plain
# parse trees, with an optional score preceding the tree (delimited
# with a tab).
my (@scores,@parses);
while (my $line = <>) {
  chomp($line);
  my ($score,$parse);

  if ($line =~ qr/\(\)/) {
	print STDERR "* FATAL: invalid parse tree at line $.\n";
	exit 1;
  }

  if ($line =~ /\t/) {

	# format 1: each line contains a (score,tree) pair, delimited by a tab
    ($score,$parse) = split(/\t/,$line);

  } else {

	# format 2: just a parse tree
	$score = 0.0;
	$parse = $line;
  }

  push(@scores,$score);
  push(@parses,$parse);
}

# get some temporary files.  C&J's feature extractor expects parallel
# gold-standard parse trees in addition to the trees you send it, and
# each tree set has to be in a very specific format.
my ($parsefh, $parsefilename) = tempfile();
my ($goldfh,  $goldfilename)  = tempfile();

# generate the two files.  The parse tree file has three lines of
# information for each parse tree candidate:
#
# NUMCANDIDATES [tab] SENTENCEID
# CAND1 SCORE
# CAND1 PARSE TREE
# CAND2 SCORE
# CAND2 PARSE TREE
# ...
#
# The gold standard file has one line at the top indicating the number
# of entries, and then a single line for each tree, in the format:
#
# SENTENCEID [tab] PARSE TREE
#
# Since we're not doing n-best reranking, we have just one item on
# each n-best list.
print $goldfh (scalar @scores) . $/;
for (my $i = 0; $i <= $#scores; $i++) {
  print $parsefh "1\tid.$i\n";
  print $parsefh "$scores[$i]\n";
  print $parsefh "$parses[$i]\n";
  
  print $goldfh "id.$i\t$parses[$i]\n";
}

# extract-spfeatures writes the feature file to the file indicated by
# its third argument, and the feature mapping to STDOUT.  we have to
# do further processing of both, so we temporarily save them to temp
# files
my ($featurefilename) = mktemp("/tmp/XXXX");
my ($mapfilename) = mktemp("/tmp/XXXX");

my $local = ($do_local_only) ? "-f local" : "";
my $cmd = qq($extract -s $mincount -ciae $local "cat $parsefilename" "cat $goldfilename" $featurefilename > $mapfilename 2> /dev/null);
system($cmd);

# now read in the feature mapping, if necessary
my %feature_map;
if ($undo_map) {
  open FEATURE_MAP, $mapfilename or die;
  while (my $line = <FEATURE_MAP>) {
	chomp($line);
	my ($id, $feature) = split(' ', $line, 2);
	$feature =~ s/ +/_/g;
	$feature_map{$id} = $feature;
  }
  close(FEATURE_MAP);
}

# munge the feature file output
open FEATURES, $featurefilename;
while ($_ = <FEATURES>) {
  chomp($_);
  next unless $. >= 2;

  # remove features that compare to the gold parse tree
  s/[GNPW0]=\d+(?:\.\d+)?\s*/ /g;

  # get rid of commas
  s/,/ /g;

  # insert extra spaces, then append count of 1 to features where the
  # count was implicit
  s/ /   /g;
  s/ (\d+) /$1:1/g;

  # replace equals signs with colons
  s/[=]/:/g;

  # get rid of extra spaces
  s/\s+/ /g;

  # get rid of leading space
  s/^\s+//;

  if ($undo_map) {
	s/(\d+):/$feature_map{$1} . ':'/ge;
  }

  print "$_\n";
}
close(FEATURES);

# now print the feature mapping to STDERR (unless we undid the map)
if (! $undo_map) {
  open MAP, $mapfilename;
  while (<MAP>) {
	print STDERR;
  }
  close(MAP);
}

# cleanup
close($goldfh);
close($parsefh);
map { unlink($_); } ($featurefilename,$mapfilename,$goldfilename,$parsefilename);
