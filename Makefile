TEX  = CMSIS_ru.tex header.tex
TEX += intro/duction.tex intro/components.tex

LATEX = pdflatex -halt-on-error -output-directory=tmp

tmp/CMSIS_ru.pdf: $(TEX)
	$(LATEX) $< && $(LATEX) $<

.PHONY: release
NOW = $(shell date +%y%m%d)
release: CMSIS_ru_$(NOW).pdf
	git tag -b $(NOW)
CMSIS_ru_$(NOW).pdf: tmp/CMSIS_ru.pdf
	cp $< $@ ; echo $@
