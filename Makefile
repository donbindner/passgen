passgen: passgen.c
	gcc -DNDEBUG -lm -o passgen passgen.c

clean:
	rm -f passgen

install: passgen
	cp passgen /usr/local/bin
	cp passgen.1 /usr/local/share/man/man1/

uninstall:
	rm -f /usr/local/bin/passgen
	rm -f /usr/local/share/man/man1/passgen.1
