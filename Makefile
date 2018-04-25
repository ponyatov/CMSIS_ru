TEX  = CMSIS_ru.tex header.tex
TEX += intro/*.tex
IMG += intro/*.png
TEX += core/*.tex
TEX += ext/*.tex

LATEX = pdflatex -halt-on-error -output-directory=tmp

tmp/CMSIS_ru.pdf: $(TEX) $(IMG)
	$(LATEX) $< && $(LATEX) $<

.PHONY: release
NOW = $(shell date +%y%m%d)
release: CMSIS_ru_$(NOW).pdf
	git tag $(NOW)
CMSIS_ru_$(NOW).pdf: tmp/CMSIS_ru.pdf
	cp $< $@ ; echo $@
