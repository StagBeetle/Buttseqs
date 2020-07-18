#ifndef forMap_h
#define forMap_h
namespace forMap{
	
	//I think this is simpler to use than the STL alternative, though less efficient
	
	template <typename key, typename value, int num>
	class for_map{
		private:
			std::array<std::pair<key, value>, num> entries;
		public:
			for_map(std::array<std::pair<key, value>, num> c_entries){
				entries = c_entries;
			}
			
			int find(const key k){
				// lg();
				for(unsigned int i = 0; i<num; i++){
					// lg((entries)[i].second);
					// lg((long int)((entries)[i].first));
					// lg((long int)(k));
					// lg();
					if((entries)[i].first == k){
						return i;
					}
				}
				return -1;
			}
			
			value getValue(const int i){
				value v = (entries)[i].second;
				return v;
			}
			
			value findAndGet(const key k){
				const int position = find(k);
				if(position == -1){
					return value();
				}
				return getValue(position);
			}
	};
}
#endif