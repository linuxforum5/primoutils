#!/usr/bin/perl -w
use strict;
use warnings;

my $DI = 232;
my $A = 255;
my $CM = 17129;
my $CP = $CM + 64; # 17193;
printf "%04X\n", $CM;

# $CM -től
for( my $i=0; $i<4; $i++ ) { # 4*16 byte = 64
    printf "%02X %02X %02X %02X %02X %02X %02X %02X\n",  0, $A,  0, $A,  0, $A,  0, $A;
    printf "%02X %02X %02X %02X %02X %02X %02X %02X\n", $A,  0, $A,  0, $A,  0, $A,  0;
}
# CP-től
printf "%04X\n", $CP;
my @bytes = ( 17,233,66,33,0,$DI,62,8,245,229,14,8,26,213,17,30,0,6,24,119,35,119,35,119,25,16,248,209,19,13,32,236,225,35,35,35,241,61,32,224,33,224,$DI-4,62,4,245,62,128,6,24,17,0,3,25,17,32,0,25,119,16,252,241,61,32,236,33,247,$DI-1,62,4,245,62,1,6,24,17,0,3,25,17,32,0,25,119,16,252,241,61,32,236,62,255,6,24,17,0,$DI,33,224,$DI+23,18,119,19,35,16,250,201 );
my $cnt = 0;
open( F, '>', 's_4329.bin' );
foreach my $byte (@bytes) {
    printf F "%c", $byte;
    if ( $byte =~ /^\d+$/o ) {
        printf "%02X", $byte;
        $cnt++;
        print ( ( $cnt % 8 ) ? " " : "\n" );
    } else {
        print STDERR "\nInvalid data: '$byte'\n";
        exit;
    }
}
print "\n";
close( F );