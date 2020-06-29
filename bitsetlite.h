#ifndef bitset_lite_h
#define bitset_lite_h

//Simple bitset intended for low-count bitsets
//Trivially copyable

template <unsigned int s>
class bitset_lite{
	static const int varSize = 32;
	volatile uint32_t bits[(s+varSize-1)/varSize] = {0};
	
	public:
	bool test (const unsigned int n) volatile const {
		unsigned int v = n / varSize;
		unsigned int b = n % varSize;
		return bits[v] >> b & 1;
	}
	// void set (const unsigned int n, const bool val) volatile {
		// unsigned int v = n / varSize;
		// unsigned int b = n % varSize;
		// if(val)	{bits[v] |= (1 << b); }
		// else	{bits[v] &= ~(1 << b); }
	// }
	void set (const unsigned int n) volatile {
		unsigned int v = n / varSize;
		unsigned int b = n % varSize;
		bits[v] |= (1 << b);
	}
	void reset (const unsigned int n) volatile {
		unsigned int v = n / varSize;
		unsigned int b = n % varSize;
		bits[v] &= ~(1 << b);
	}
	void flip (const unsigned int n) volatile {
		unsigned int v = n / varSize;
		unsigned int b = n % varSize;
		bits[v] ^= (1 << b);
	}
};

#endif