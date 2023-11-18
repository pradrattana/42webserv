#!/usr/bin/perl

use strict;
use warnings;
use Cwd;
use File::Path qw(rmtree);

# Get the environment variable
my $fileName = $ENV{'SCRIPT_NAME'}; # Replace 'YOUR_ENV_VARIABLE_NAME' with the actual environment variable name
my $fullPath = $ENV{'SCRIPT_FILENAME'};
# my $fullPath = "/home/petcha_nop/bb2/html/download/vnilprap.tar";

my $cwd = getcwd();

my $path = $fullPath;

# my $success = "FILE $fullPath deleted successfully.";
my $success = "Status: 200\r\n" . "Content-type: text/plain\r\n" . "\r\n" . "FILE $fullPath deleted successfully.\r\n";
my $fail = "Status: 404\r\n" . "Content-type: text/plain\r\n" . "\r\n" . "File $fullPath not found.\r\n";

# my $msg = pack("Z*", $success\r\n\r\n");

my $successLength = length($success);

if (-e $path) {
    if (-d $path) {
        rmtree($path) or die $fail;
        print $success;
    } else {
        unlink $path or die $fail;
        print $success;
    }
} else {
    print $fail;
}
