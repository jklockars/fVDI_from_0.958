  ifne gas
	include "pixelmac.gnu"
  else
   ifne lattice
	include "pixelmac.dev"
   else
	include "pixelmac.tas"
   endc
  endc

