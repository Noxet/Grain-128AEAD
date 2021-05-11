# Grain128-AEADv2 reference implementation

This is the version with the NIST-defined API to be run on SUPERCOP.
It is the same version as in the root folder, but here we also authenticate the length of AD, since it is of variable length, given by NIST.

If a protocol defines fixed length packets, there is no need to authenticate the length of the data.
