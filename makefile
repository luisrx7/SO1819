servidor:cliente
			gcc sv.c util.h -o servidor -lncurses
cliente:executa
			gcc cl.c util.h -o cliente -lncurses
executa:
			. ./script.sh
