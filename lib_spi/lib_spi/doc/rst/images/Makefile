ODGS = $(wildcard *.odg)
PDFS = $(ODGS:.odg=.pdf)

all: $(PDFS)
	@echo PDFs created

_uncropped:
	mkdir _uncropped

_uncropped/%.pdf: %.odg | _uncropped
	soffice -env:UserInstallation=file:///home/$(USER)/.libreoffice-alt --headless --convert-to pdf $< --outdir _uncropped

%.pdf: _uncropped/%.pdf
	pdfcrop $< $@

wavedrom:
	WaveDromEditor source wavedrom_mode0.js png wavedrom_mode0.png
	WaveDromEditor source wavedrom_mode1.js png wavedrom_mode1.png
	WaveDromEditor source wavedrom_mode2.js png wavedrom_mode2.png
	WaveDromEditor source wavedrom_mode3.js png wavedrom_mode3.png

clean:
	-rm $(PDFS)
	-rm _uncropped/*.pdf
	-rmdir _uncropped
