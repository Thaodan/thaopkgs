DESTDIR=/
PREFIX=${DESTDIR}/usr/local
install:
	mkdir -p ${PREFIX}/bin/ ${PREFIX}/share/man/man6/
	install -m 755 nadvsh ${PREFIX}/bin/
	cp nadvsh.6 ${PREFIX}/share/man/man6/
	mkdir -p -m 1777 ${DESTDIR}/var/tmp
	mkdir -p -m 777 ${DESTDIR}/var/tmp/nadvsh
	@if test -f ${PREFIX}/man/man1/nadvsh.1; then printf '\nAttention! The manual page for nadvsh has moved to section 6.\nThe (obsolete) manual page nadvsh(1) was found on your system.\nPlease remove it manually.\n\n'; fi

