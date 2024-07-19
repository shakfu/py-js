

.PHONY: all pdf clean



all: pdf


pdf:
	@quarto render --to pdf


clean:
	@rm -rf .quarto _book
	@find . \( -name '*.aux' -o -name '*.log' -o -name '*.pdf' -o -name '*.tex' -o -name '*.toc' \) -delete
