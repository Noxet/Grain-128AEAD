# Grain-128AEAD
The C reference implementation of the stream cipher Grain-128AEAD.
Note that this implementation is meant to be close to hardware, i.e., it is not implemented to be run in software, hence it should no be used in benchmarking due to bad performance.

In the NIST folder, there are two versions of Grain. One reference implementation, same as in this folder, following NISTs API calls, and one optimized version that may be used in benchmarking.

For optimized hardware implementations, see the [Grain-128AEAD-VHDL](https://github.com/Noxet/Grain-128AEAD-VHDL) repository.
