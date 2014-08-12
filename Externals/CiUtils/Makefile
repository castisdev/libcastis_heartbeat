all: clean make-lib install

make-lib:
	$(MAKE) -C Linux;

install:
	$(MAKE) -C Linux install;

ctags:
	$(MAKE) -C Linux ctags;
	find Include -name '*.h' | ctags -L - -f tags

clean:
	$(MAKE) -C Linux clean;
	$(RM) tags

