SUBDIRS=base spectcl

PARTESTS:
	for d in ${SUBDIRS}; do (cd $$d ; make PARTESTS); done

doc:
	rm -rf docs
	doxygen
