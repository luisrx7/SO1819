servidor:cliente
			gcc sv.c util.h -o servidor -lncurses -fno-stack-protector
cliente:executa
			gcc cl.c util.h -o cliente -lncurses -fno-stack-protector
executa:
			rm sss || true
			rm ccc* || true
			. ./script.sh
