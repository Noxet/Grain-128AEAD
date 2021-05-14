.PHONY: genkat

all:
	@echo -e "\e[1;34mBuilding x64...\e[0m"
	$(MAKE) -C x64
	@echo -e "\e[1;34mBuilding SSE...\e[0m"
	$(MAKE) -C sse
	@echo -e "\e[1;34mBuilding AVX512...\e[0m"
	$(MAKE) -C avx512
	@echo -e "\e[1;34mBuilding GF2...\e[0m"
	$(MAKE) -C gf2

genkat: all
	cd x64 && ./genkat_aead
	cd sse && ./genkat_aead
	cd avx512 && ./genkat_aead
	cd gf2 && ./genkat_aead

clean:
	$(MAKE) -C x64 clean
	$(MAKE) -C sse clean
	$(MAKE) -C avx512 clean
	$(MAKE) -C gf2 clean
