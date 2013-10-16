#! /usr/bin/perl
while(<>) {
  if( /0x([0-9a-f])([0-9a-f])([0-9a-f])([0-9a-f])/ ) {
    $m = hex($3);
    $h = "$3$4$1$2";
    $d = hex($h);

    if( $m < 8 ) {
      $s = "";
    } else {
      $s = "-";
      $d -= 1;
      $d ^= hex("ffff");
         }
   $d *= 0.0625;
   $d /= 8;
   $t = $s.$d;
   print "$t\n";
   }
}
