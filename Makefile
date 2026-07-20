.PHONY: help phase0-check tree clean
help:
	@echo "make phase0-check | make tree | make clean"
phase0-check:
	@python3 tools/scripts/phase0_check.py
tree:
	@find . -path './.git' -prune -o -print | sort
clean:
	@rm -rf build out
