# maxwell
A small daemon program to feed timer entropy into the Linux random driver

This is version 2 (2016). I originally did v1 in 2012 and I have not
made significant changes since then, though others (Thank you!) have
improved it some.

The main changes are:

	different command-line options
	constants LO_CLAIM & HI_CLAIM
	(instead of a different prime for each option)
	always do some fast output at startup to fill the pool
	(based on an idea from David Jasa)
	moving actual output from main() to do1k()
	random choice of delay in do1k()

Both the ODF/PDF design paper and the man page have been rewritten
to describe the new options, and polished a bit while I was there.