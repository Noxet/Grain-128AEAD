# NIST
Here are two implementations of Grain128-AEADv2 according to NIST requirements.
One reference implementation and one optimized using parallelization.

The reference implementation has been updated to version 2, i.e. Grain128-AEADv2 to accomodate the latest changes in the design to prevent some attacks.

*Note: the optimized-v1 version is an old version and should not be used, it is only kept for legacy reasons."*

To create a tar ball with both implementations for SUPERCOP, run
`make nist`
