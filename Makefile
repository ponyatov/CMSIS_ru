TEX = CMSIS_ru.tex header.tex introduction.tex

LATEX = pdflatex -halt-on-error -output-directory=tmp

tmp/CMSIS_ru.pdf: $(TEX)
	$(LATEX) $< && $(LATEX) $<
