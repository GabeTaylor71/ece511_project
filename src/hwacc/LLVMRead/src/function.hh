#ifndef __SALAM_FUNCTION_HH__
#define __SALAM_FUNCTION_HH__

#include "value.hh"
#include "basic_block.hh"
#include "llvm/IR/Function.h"

namespace SALAM {
	class Function : public Value {
	private:
		std::vector<std::shared_ptr<SALAM::BasicBlock>> bbList;
		std::vector<std::shared_ptr<SALAM::Value>> arguments;
		bool top;

		//Reservation Queue for scheduling within this function
		std::vector<Instruction *> reservation;
	public:
		Function(uint64_t id);
		void initialize(llvm::Value * irval, irvmap *vmap, SALAM::valueListTy *valueList, std::string topName);
		bool isTop() { return top; }
		std::vector<std::shared_ptr<SALAM::BasicBlock>> * getBBList() { return &bbList; }
		std::vector<std::shared_ptr<SALAM::Value>> * getArguments() { return & arguments; }
		std::shared_ptr<SALAM::BasicBlock> entry() { return bbList.front(); }
		std::vector<Instruction *> *getReservationQueue() { return &reservation; }
	};
}

#endif //__SALAM_FUNCTION_HH__