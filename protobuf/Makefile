include ../config-host.mak

CC=gcc

FILENAME=frame.piqi
EXT=pb-c
PROTOFILE=$(TRACEWRAP_DIR)/piqi/$(FILENAME)

all: libprotoframes.a

$(FILENAME).proto: $(PROTOFILE)
	piqi to-proto $(PROTOFILE) -o $(FILENAME).proto

$(FILENAME).$(EXT).c: $(FILENAME).proto
	protoc-c --c_out=. $(FILENAME).proto
	sed -i 's/->base/->__base/g' $(FILENAME).$(EXT).[ch]
	sed -i 's/ProtobufCMessage base;/ProtobufCMessage __base;/g' $(FILENAME).$(EXT).[ch]

$(FILENAME).$(EXT).o: $(FILENAME).$(EXT).c $(FILENAME).$(EXT).h
	$(CC) -fPIC -c -o $(FILENAME).$(EXT).o $(FILENAME).$(EXT).c

libprotoframes.a: $(FILENAME).$(EXT).o
	ar rcs libprotoframes.a $(FILENAME).$(EXT).o

clean:
	rm $(FILENAME).$(EXT).[hco] $(FILENAME).proto
