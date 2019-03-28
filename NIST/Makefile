
.PHONY: ref optimized

all: ref optimized

ref:
	$(MAKE) -C ref

optimized:
	$(MAKE) -C optimized

nist: ref optimized
	@echo -e "\e[1;34mGenerating reference test vectors ...\e[0m"
	ref/genkat_aead
	# save for comparing later
	cp LWC_AEAD_KAT_128_96.txt ref.txt
	@echo -e "\e[1;34mGenerating opt32 test vectors ...\e[0m"
	optimized/genkat_aead
	diff LWC_AEAD_KAT_128_96.txt ref.txt > /dev/null || (echo -e "\e[1;31mTest vectors mismatch!\e[0m" && /bin/false)
	echo -e "\e[1;32mTest vectors match!\e[0m"
	# create NIST folder structure and add code + test vectors
	mkdir -p crypto_aead/grain128aead/ref
	mkdir -p crypto_aead/grain128aead/opt32
	cp LWC_AEAD_KAT_128_96.txt crypto_aead/grain128aead/
	cp ref/api.h ref/grain128aead.c ref/grain128aead.h crypto_aead/grain128aead/ref
	cp optimized/api.h optimized/grain128aead_32p.c optimized/grain128aead_32p.h crypto_aead/grain128aead/opt32
	# pack it up in a tar ball
	@echo -e "\e[1;34mCreating tar ball ...\e[0m"
	tar -cvjf grain128aead.tar.gz crypto_aead
	# clean up
	rm -f ref.txt LWC_AEAD_KAT_128_96.txt

clean:
	$(MAKE) -C ref clean
	$(MAKE) -C optimized clean
	rm -Rf crypto_aead ref.txt LWC_AEAD_KAT_128_96.txt
