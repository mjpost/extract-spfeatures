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

# add in --local for local features only
my $do_local_only = "";
my $mincount = 1;
my $extract = dirname($0) . "/extract-spfeatures";

my $result = GetOptions("min=i" => \$mincount,
						"local" => \$do_local_only);

my (@scores,@parses);
while (my $line = <>) {
  chomp($line);
  my ($score,$parse);

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

my $numlines = @scores;

my ($parsefh, $parsefilename) = tempfile();
my ($goldfh,  $goldfilename)  = tempfile();

print $goldfh "$numlines\n";
for (my $i = 0; $i <= $#scores; $i++) {
  print $parsefh "1\tid.$i\n";
  print $parsefh "$scores[$i]\n";
  print $parsefh "$parses[$i]\n";
  
  print $goldfh "id.$i\t$parses[$i]\n";
}

my ($featurefilename) = mktemp("/tmp/XXXX");
my ($mapfilename) = mktemp("/tmp/XXXX");

my $cmd = qq($extract -s $mincount -ciae $do_local_only "cat $parsefilename" "cat $goldfilename" $featurefilename > $mapfilename 2> /dev/null);
system($cmd);

open FEATURES, $featurefilename;
while ($_ = <FEATURES>) {
  chomp($_);
  next unless $. >= 2;

  # remove features that compare to the gold parse tree
  s/[GNPW0]=\d+(?:\.\d+)?\s+/ /g;

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

  print "$_\n";
}
close(FEATURES);

# print the features to STDERR (were written to STDOUT by extract-spfeatures)
open MAP, $mapfilename;
while (<MAP>) {
  print STDERR;
}
close(MAP);

# cleanup
close($goldfh);
close($parsefh);

map { unlink($file); } ($mapfilename,$goldfilename, $parsefilename);
