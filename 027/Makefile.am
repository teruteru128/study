#info_TEXINFOS = test.tex
all: test.pdf

%.pdf: %.dvi
	dvipdfmx $<

%.dvi: %.tex
	platex -interaction=batchmode $<

clean: 
	$(RM) test.pdf test.log test.aux
