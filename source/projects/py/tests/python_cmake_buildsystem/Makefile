
.PHONEY: build log clean

all: build


build: clean
	python3 build.py

log: clean
	python3 build.py &> run.log


clean:
	@rm -rf python-cmake*
	@rm -rf Python*
	@rm -f run.log
