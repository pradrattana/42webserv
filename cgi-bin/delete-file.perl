#!/usr/bin/perl

use strict;
use warnings;
use Cwd;

# Get the environment variable
my $fileName = $ENV{'filename'}; # Replace 'YOUR_ENV_VARIABLE_NAME' with the actual environment variable name
my $fullPath = $ENV{'fullpath'};
# my $fullPath = "/home/petcha_nop/bb2/html/download/vnilprap.tar";

my $cwd = getcwd();

my $path = $cwd . "/html" . $fullPath;

my $success = "FILE $fileName deleted successfully.";
my $fail = "File $fileName not found." . "\0";

# my $msg = pack("Z*", $success\r\n\r\n");

my $successLength = length($success);

if (-e $path) {
    unlink $path or die $fail;
    print "$success\r\n" . "\0\r\n";
} else {
    print $fail;
}