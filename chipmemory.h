#ifndef chipmemory_h
#define chipmemory_h
namespace chipmemory{
const uint8_t memio[] = {
	42,
	43,
	44,
	45,
	46,
	30,
	29,
	28,
};

const uint8_t memaddr[] = {
	40,
	20,
	17,
	16,
	15,
	39,
	38,
	37,
	50,
	51,
	52,
	53,
	54,
	55,
	56,
	57,
	26,
	36,
	35,
	34,
	27,
};

const uint8_t memOE = 41;
const uint8_t memWE = 34;

const long int maxAddress = 2097152; //One more than max

void doADelay(){
	delay(5);
}

void setPins(const bool type){//true is read
	for(int i = 0; i<8; i++){
		pinMode(memio[i], type ? INPUT : OUTPUT);
		doADelay();
	}
}

void setAddress(const long int address){
	for(int i = 0; i<21; i++){
		digitalWriteFast(memaddr[i], (address >> i) & 1);
		doADelay();
	}
}

void setData(const uint8_t data){
	for(int i = 0; i<8; i++){
		digitalWriteFast(memio[i], (data >> i) & 1);
		doADelay();
	}
}

uint8_t readData(){
	setPins(true);
	uint8_t data = 0;
	for(int i = 0; i<8; i++){
		uint8_t bit = digitalReadFast(memio[i]);
		data += (bit << i);
		doADelay();
	}
	return data;
}

void write(const long int address, const uint8_t data){
	setPins(false);
	digitalWriteFast(memWE, 0);
	doADelay();
	setAddress(address);
	doADelay();
	setData(data);
	doADelay();
	digitalWriteFast(memWE, 1);
}

uint8_t read(const long int address){
	digitalWriteFast(memOE, 0);
	doADelay();
	setAddress(address);
	doADelay();
	return readData();
	doADelay();
	digitalWriteFast(memOE, 1);
}

}
#endif