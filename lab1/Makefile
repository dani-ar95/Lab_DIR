DIROBJ := obj/
DIREXE := exec/
DIRHEA := include/
DIRSRC := src/

CFLAGS := -I$(DIRHEA) -c
LDLIBS := -lpthread -lrt -lm
CC := mpicc

all : dirs toroide hipercubo

dirs:
	mkdir -p $(DIROBJ) $(DIREXE)

toroide: $(DIROBJ)toroide.o
	$(CC) -o $(DIREXE)$@ $^ $(LDLIBS)

hipercubo: $(DIROBJ)hipercubo.o
	$(CC) -o $(DIREXE)$@ $^ $(LDLIBS)

$(DIROBJ)%.o: $(DIRSRC)%.c
	$(CC) $(CFLAGS) $^ -o $@

toroide_test:
	mpirun --mca btl tcp,self -n 16 --hostfile host ./$(DIREXE)toroide

hipercubo_test:
	mpirun --mca btl tcp,self -n 16 --hostfile host ./$(DIREXE)hipercubo

clean : 
	rm -rf *~ core $(DIROBJ) $(DIREXE) $(DIRHEA)*~ $(DIRSRC)*~
