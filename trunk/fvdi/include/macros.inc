  ifne gas
	include "macros.gnu"
  else
   ifne lattice
	include "macros.dev"
   else
	include "macros.tas"
   endc
  endc

