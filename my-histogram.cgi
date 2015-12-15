#!/usr/bin/perl
#http://perlmaven.com/count-words-in-text-using-perl
#http://perlmaven.com/introduction-to-regexes-in-perl
#http://www.perlmonks.org/?node_id=159085

use CGI qw(:standard);
use strict;
use warnings;

my @args;


for my $name (CGI::param()) {
    for my $val (CGI::param($name)) {
        push(@args, $val);  #Add argument to array	
    }
}

my $count = 0; #Count of pattern matches
my $argNum = @args; #Arguments number
my $pattern; #Pattern to match in file
my $num = 1;
my @countArr; #The array that stores pattern matches counts
my @matches; #The array of matches in specifc line





my $file = $args[0] or die "Usage: $0 FILE\n";
open my $fh, '<', $file or die "Can't open '$file' $!";
#Go through each pattern passed in
while ($num < $argNum) {
    $pattern = $args[$num];  #Set pattern equal to specific argument
    while (my $line = <$fh>) {
        @matches = ($line =~ /$pattern/g); #Add matches to array to be counted
	$count = $count + @matches;  #Count matches in array
	undef @matches;  #Empty array

    }
    push(@countArr, $count);  #Add total count to array
    $num++;
    $count = 0;  #Initialize count back to 0
    seek $fh, 0, 0; #Go back to start of file
}
close ($fh); #Close file handler

$num = 0;
my @xLabels; #Create array of xLabels
while ($num < $argNum) {
    push(@xLabels, $args[$num]);
    $num++;
}

#Find maximum value of counts to be used for yrange
my $countNum = @countArr;
my $max = 0;
for (my $i = 0; $i < $countNum; $i++) {
    if ($countArr[$i] > $max) {
       $max = $countArr[$i];
    }
}

#Create string to be used for xtics
my $xticsString = "(";
for (my $i = 1; $i < $argNum; $i++) {
    if ($i == ($argNum - 1)) {
    $xticsString = $xticsString . "\"$args[$i]\" $i)";
    }
    else {
    $xticsString = $xticsString . "\"$args[$i]\" $i, ";
    }
}

#Create string to be used for bars to be plotted
my $plotString = "";
for (my $i = 0; $i < $countNum; $i++) {
    my $pos = $i + 1;
    $plotString = $plotString . "$pos $countArr[$i]\n";
    
}

#Open gnuplot and create bar graph
open (GNUPLOT, "|gnuplot -persist") or die "no gnuplot";

my $gnuplot_string = <<GNU;
reset
set autoscale
set xtics $xticsString
set xrange [0:$argNum]
set xlabel \"Patterns\"
set ylabel \"Number of Matches\"
set yrange [0:($max + 5)]
set title \"Histogram of Pattern Matches in $args[0]\"
set terminal jpeg
set output "output.jpg"
n=$countNum

plot '-' with boxes fs solid 1
$plotString
e

GNU

print GNUPLOT $gnuplot_string; #Print gnuplot
close(GNUPLOT); #Close gnuplot
