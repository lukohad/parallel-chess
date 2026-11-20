PAPER = paper
RESULTS = ../../chess-engine/results

default: $(PAPER).pdf

# charts and the times table, generated from saved bench.sh output
figures:
	python figures/plot_results.py $(RESULTS)/sequential.txt $(RESULTS)/static.txt

$(PAPER).pdf: $(PAPER).tex biblio.bib
	pdflatex $(PAPER)
	bibtex $(PAPER)
	pdflatex $(PAPER)
	pdflatex $(PAPER)

clean:
	/bin/rm -f $(PAPER).aux $(PAPER).pdf $(PAPER).log $(PAPER).bbl $(PAPER).blg

.PHONY: figures clean
