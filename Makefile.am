SUBDIRS=base spectcl

PARTESTS:
	for d in ${SUBDIRS}; do (cd $$d ; make PARTESTS); done


install-exec-local: doc
	$(mkinstalldirs) @datarootdir@ @datarootdir@/docs
	(cd @builddir@/docs/html; tar czf - .) | \
	(cd @datarootdir@/docs ; tar xzf  -)

doc:
	rm -rf docs
	doxygen
