SUBDIRS=base spectcl

PARTESTS:
	for d in ${SUBDIRS}; do (cd $$d ; make PARTESTS); done


install-exec-local: doc
	$(mkinstalldirs) @datarootdir@ @datarootdir@/docs
	(cd @builddir@/docs/html; tar czf - .) | \
	(cd @datarootdir@/docs ; tar xzf  -)
	echo FRIB analysis framework $(VERSION) > @prefix@/VERSION
	echo Use mpirun in `dirname $(CXX)`  >> @prefix@/VERSION
	echo Built on `hostname` on `date` >> @prefix@/VERSION

doc:
	rm -rf docs
	doxygen
