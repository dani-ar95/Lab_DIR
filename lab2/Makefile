DIROBJ := obj/
DIREXE := exec/
DIRHEA := include/
DIRSRC := src/

CFLAGS := -I$(DIRHEA) -c
LDLIBS := -lpthread -lrt -lX11
CC := mpicc

all : dirs link_pract2

dirs:
	mkdir -p $(DIROBJ) $(DIREXE)

pract2: $(DIROBJ)pract2.o
	$(CC) -o $(DIREXE)$@ $^ $(LDLIBS)

$(DIROBJ)%.o: $(DIRSRC)%.c
	$(CC) $(CFLAGS) $^ -o $@

test:
	mpirun -n 1 --oversubscribe ./$(DIREXE)pract2

link_pract2: pract2
	ln -s $(DIREXE)pract2 ./pract2

clean : 
	rm -rf *~ core $(DIROBJ) $(DIREXE) $(DIRHEA)*~ $(DIRSRC)*~ pract2
