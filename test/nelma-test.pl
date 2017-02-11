#!/usr/bin/perl -w

use strict;

use vars qw( $NelmaBin );
use Getopt::Std;

$NelmaBin = "nelma-cap";

sub config_load {
        my ($source, $dest) = @_;
        my ($line);

        %$dest=();

        unless(open(META, "<$source")) {
                warn("Can't open $source to load metadata");
                return(0);
        }

        while(defined($line=<META>)) {
                if($line=~/(\w*): (.*)/) {
                        $$dest{$1}=$2;
                }
        }
        close(META);

        return(1);
}

sub interpolate {
	my ($string, $var, $val) = @_;

	$string=~s/(\W)\$$var(\W)/$1$val$2/g;

	return $string;
}

sub file_save {
	my ($fn, $content)=@_;
	my ($line);

	open(FILE, ">$fn") or die("Can't open $fn: $!");
	print(FILE $content);
	close(FILE);
}

sub file_load {
	my ($fn)=@_;
	my ($result, $line);

	$result="";
	open(FILE, "<$fn") or die("Can't open $fn: $!");
	while(defined($line=<FILE>)) {
		$result.=$line;
	}
	close(FILE);

	return $result;
}

sub file_run {
	my ($args, $fn)=@_;
	my ($line);

	open(FILE, "$NelmaBin $args $fn|") or die("Can't open $fn: $!");
	while(defined($line=<FILE>)) {
		if($line=~/^C.* ([-0-9e.]+)$/) {
			return $1;
		}
	}
	close(FILE);

	return;
}

sub syntax {
	print(<<END
NELMA, Numerical ElectroMagnetics. Testing tool.
Copyright (c) 2006 Tomaz Solc

SYNTAX: nelma-test.pl [ -o OUTPUT_FILE ]
                      [ -r ]
                      config_file.test

Bug reports to <tomaz.solc\@tablix.org>
END
	);
	
}

my %opts;

getopts("o:r", \%opts);

if(($#ARGV+1)!=1) {
	syntax();
	exit;	
}

my %config;
my $value;
my $okflag;

config_load($ARGV[0], \%config);

if(not $opts{o}) {
	if($opts{r}) {
		die("Must specify file name with -o when using -r option");
	}
	$opts{o}="-";
}

if($opts{r}) {
	open(OUTPUT, ">test.tmp");
	open(INPUT, "<$opts{o}");
} else {
	open(OUTPUT, ">$opts{o}");
}

$okflag=1;

print(OUTPUT "# Value\tAnalytical result\tNumerical result\n");

for($value=$config{start};$value<=$config{stop};$value=$value+$config{step}) {
	my $ana_result;
	my $num_result;

	{
		my $x=$value;
		$ana_result=eval($config{formula});
	}
	
	if($opts{r}) 
	{
		my $line;
		
		do {
			$line=<INPUT>;	
		} while($line=~/^\#/);

		if($line=~/(.*)\s+(.*)\s+(.*)\s+/) {
			if($1==$value) {
				$num_result=$3;
			} else {
				print("Unexpected line $line in file (wrong value)\n");
				$num_result=0;
				$okflag=0;
			}
		} else {
			print("Unexpected line $line in file (regexp)\n");
			$okflag=0;
			$num_result=0;
		}
	} else {
		my $r=file_load($config{config});

		$r=interpolate($r, "x", $value);

		file_save("test.tmp", $r);

		$config{arguments}="" if(!defined($config{arguments}));
		$num_result=file_run($config{arguments}, "test.tmp");
	}

	print(OUTPUT "$value\t$ana_result\t$num_result\n");
}

close(OUTPUT);
if($opts{r}) {
	close(INPUT);
	if($okflag) {
		rename("test.tmp", $opts{o});
	} else {
		print("Error encountered. Will not overwrite original file.");
	}
}
