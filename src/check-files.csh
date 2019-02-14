#!/bin/tcsh -f

set sdir=/root/src/cbrief-for-jed-jed/src

foreach f ( *.c )
	if ( ! { diff -q  $f $sdir/$f } ) then
		set base=$f:t:r
		diff -c $sdir/$f $f > ${base}.patch
	endif
end
