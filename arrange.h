#ifndef arrange_h
#define arrange_h

#include "blocks.h"
#include "patternstorage.h"
#include <array>

// Work on seamless mix between pattern loop and arrange
	// Normal loop adds the patterns at start of arrange - adjust end loop point based on longest loop
	
namespace arrangement{
	extern const uint16_t	arrangeBlockSize	;
	extern const int	numberOfArrangements	;
	extern const uint16_t	maxPosition	;
	
	const uint16_t maxDepth = 3;

	enum class nodeType : uint8_t {
		node	,
		pattern	,
	};
	
	class node : public blocks::block_t{
		private:
			nodeType nodePattType = nodeType::node; //Obtained from parent;
		public:
			node(const uint16_t p_address, const bool isPattern);
			
			node(const bool isPattern);
			
			node();
			
			bool isValid() const;
			
			void initialise();
			
			bool isValidPattern() const;
			
			node getNodeAtPosition(const int position) const ;

			void setNodeAtPosition(const int position, const node child);
			
			bool isPattern() const;
			
			const char* showType();
			
			patMem::pattern_t getPattern() const;
			
			void printContents();
			
			friend bool operator == (const node& one, const node& other);
			
			node& operator = (const node& other);
			
	};

	bool operator == (const node& one, const node& other);
	
	bool operator != (const node& one, const node& other);

	typedef std::array<int, maxDepth> nodeIndex;

	nodeIndex getIndexes(const int timelinePosition, const bool logging = false);
	
	int getPositionFromIndexes(nodeIndex indexes, const int depth);
	
	void printIndexes(nodeIndex index);

	class arrangeChannel{
		public:
			arrangeChannel();
			
			patMem::pattern_t getCurrentPattern (const int timelinePosition);
			
			void addPattern (const int timelinePosition, const patMem::pattern_t patt);//Check indexes
			
			void deletePatternIfFound(const patMem::pattern_t patternToDelete);
			
			node getFirstPattern();
			
			void setFirstPattern(const node first);
			
			void removePattern(const int timelinePosition);
		private:
			node firstPattern;// = {zeroPointerReplacer, true};
	};

	arrangeChannel& getArrangeChannel(const int arragenment, const int track);
}
#endif