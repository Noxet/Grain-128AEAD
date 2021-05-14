
.PHONY: ref optimized

all: ref optimized

ref:
	$(MAKE) -C ref

optimized:
	$(MAKE) -C optimized

genkat:
	@echo -e "\e[1;34mGenerating reference test vectors ...\e[0m"
	$(MAKE) -C ref genkat
	@echo -e "\e[1;34mGenerating optimized test vectors ...\e[0m"
	$(MAKE) -C optimized genkat

nist:
	diff ref/LWC_AEAD_KAT_128_96.txt optimized/x64/LWC_AEAD_KAT_128_96.txt > /dev/null || (echo -e "\e[1;31mTest vectors mismatch!\e[0m" && /bin/false)
	diff ref/LWC_AEAD_KAT_128_96.txt optimized/sse/LWC_AEAD_KAT_128_96.txt > /dev/null || (echo -e "\e[1;31mTest vectors mismatch!\e[0m" && /bin/false)
	diff ref/LWC_AEAD_KAT_128_96.txt optimized/avx512/LWC_AEAD_KAT_128_96.txt > /dev/null || (echo -e "\e[1;31mTest vectors mismatch!\e[0m" && /bin/false)
	diff ref/LWC_AEAD_KAT_128_96.txt optimized/gf2/LWC_AEAD_KAT_128_96.txt > /dev/null || (echo -e "\e[1;31mTest vectors mismatch!\e[0m" && /bin/false)
	echo -e "\e[1;32mTest vectors match!\e[0m"
	# create NIST folder structure and add code + test vectors
	mkdir -p crypto_aead/grain128aeadv2/ref
	mkdir -p crypto_aead/grain128aeadv2/x64
	mkdir -p crypto_aead/grain128aeadv2/sse
	mkdir -p crypto_aead/grain128aeadv2/avx512
	mkdir -p crypto_aead/grain128aeadv2/gf2
	cp ref/LWC_AEAD_KAT_128_96.txt crypto_aead/grain128aeadv2/
	cp ref/api.h ref/grain128aead-v2.c ref/grain128aead-v2.h crypto_aead/grain128aeadv2/ref
	cp optimized/x64/api.h optimized/x64/grain128aead-v2_opt.cpp optimized/x64/grain128aead-v2_opt.h crypto_aead/grain128aeadv2/x64
	cp optimized/sse/api.h optimized/sse/grain128aead-v2_opt.cpp optimized/sse/grain128aead-v2_opt.h crypto_aead/grain128aeadv2/sse
	cp optimized/avx512/api.h optimized/avx512/grain128aead-v2_opt.cpp optimized/avx512/grain128aead-v2_opt.h crypto_aead/grain128aeadv2/avx512
	cp optimized/gf2/api.h optimized/gf2/grain128aead-v2_opt.cpp optimized/gf2/grain128aead-v2_opt.h crypto_aead/grain128aeadv2/gf2
	# pack it up in a tar ball
	@echo -e "\e[1;34mCreating tar ball ...\e[0m"
	tar -cvzf grain128aeadv2.tar.gz crypto_aead
	# clean up
	rm -f ref.txt LWC_AEAD_KAT_128_96.txt

clean:
	$(MAKE) -C ref clean
	$(MAKE) -C optimized clean
	rm -Rf crypto_aead ref.txt LWC_AEAD_KAT_128_96.txt
