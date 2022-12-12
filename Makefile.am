SUBDIRS=base spectcl

PARTESTS:
	for d in ${SUBDIRS}; do (cd $$d ; make PARTESTS); done

doxy:
	rm -rf docs
	doxygen
