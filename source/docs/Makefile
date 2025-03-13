

.PHONY: all update build pdf clean

all: build

update:
	@./scripts/update.py

build: update pdf
	@echo "Update and PDF build complete."

pdf:
	@quarto render --to pdf

clean:
	@rm -rf .quarto _book
	@find . \( -name '*.aux' -o -name '*.log' -o -name '*.pdf' -o -name '*.tex' -o -name '*.toc' \) -delete
