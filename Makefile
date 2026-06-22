.PHONY: test clean

test:
	sh tests/selftest.sh

clean:
	rm -rf out build_out

