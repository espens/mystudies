all: main

main: main.o \
      irq.o \
      l1_phys.o l2_link.o l3_net.o l4_trans.o l5_app.o \
      delayed_sendto.o delayed_dropping_sendto.o slow_receiver.o
	  gcc -g -o main $^

%.o: %.c
	gcc -g -c -Wall $^

clean:
	rm -f *.o
	rm -f main
	rm -f tmp.c

realclean: clean
	rm -f *~

