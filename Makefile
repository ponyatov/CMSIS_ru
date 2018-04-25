TEX  = CMSIS_ru.tex header.tex
TEX += intro/duction.tex intro/components.tex

LATEX = pdflatex -halt-on-error -output-directory=tmp

tmp/CMSIS_ru.pdf: $(TEX)
	$(LATEX) $< && $(LATEX) $<
