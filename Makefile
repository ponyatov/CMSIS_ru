#.PHONY: install
#install: stm32files lib hello

TEX  = CMSIS_ru.tex header.tex
TEX += intro/*.tex
IMG += intro/*.png
TEX += core/*.tex
TEX += ext/*.tex
SRC += ld/*.ld src/*.gdb

IMG += tmp/toolchain.png

LATEX = pdflatex -halt-on-error -output-directory=tmp

tmp/CMSIS_ru.pdf: $(TEX) $(IMG) $(SRC)
	$(LATEX) $< && $(LATEX) $<

.PHONY: release
NOW = $(shell date +%y%m%d)
release: CMSIS_ru_$(NOW).pdf
	git tag $(NOW)
CMSIS_ru_$(NOW).pdf: tmp/CMSIS_ru.pdf
	cp $< $@ ; echo $@
	
tmp/%.png: ext/%.dot Makefile
	dot -Tpng -Gmargin=0 -o $@ $<

.PHONY: install
install: stm32files

.PHONY: stm32files
stm32files:
	cd src/STM32 ; $(MAKE) TOPDIR=$(CURDIR) 

.PHONY: lib
lib:
	cd lib ; $(MAKE) TOPDIR=$(CURDIR) 

.PHONY: hello
hello:
	cd src ; $(MAKE) TOPDIR=$(CURDIR) MCU=STM32L073RZ
