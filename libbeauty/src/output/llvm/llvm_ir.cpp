/* Test creation of a .bc file for LLVM IR*/

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <global_struct.h>

#include <output.h>
#include <debug_llvm.h>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

#define STORE_DIRECT 0

using namespace llvm;

struct declaration_s {
	std::vector<Type*>FuncTy_0_args;
	FunctionType *FT;
	Function *F;
};

		CmpInst::Predicate predicate_to_llvm_table[] =  {
			ICmpInst::FCMP_FALSE,  /// None
			ICmpInst::FCMP_FALSE,  /// FLAG_OVERFLOW
			ICmpInst::FCMP_FALSE,  /// FLAG_NOT_OVERFLOW
			ICmpInst::ICMP_ULT,  ///< unsigned less than. FLAG_BELOW
			ICmpInst::ICMP_UGE,  ///< unsigned greater or equal. FLAG_NOT_BELOW
			ICmpInst::ICMP_EQ,  ///< equal. FLAG_EQUAL
			ICmpInst::ICMP_NE,  ///< not equal. FLAG_NOT_EQUAL
			ICmpInst::ICMP_ULE,  ///< unsigned less or equal. FLAG_BELOW_EQUAL
			ICmpInst::ICMP_UGT,  ///< unsigned greater than. FLAG_ABOVE
			ICmpInst::FCMP_FALSE, /// FLAG_SIGNED
			ICmpInst::FCMP_FALSE, /// FLAG_NOT_SIGNED
			ICmpInst::FCMP_FALSE, /// FLAG_PARITY
			ICmpInst::FCMP_FALSE, /// FLAG_NOT_PARITY
			ICmpInst::ICMP_SLT,  ///< signed less than
			ICmpInst::ICMP_SGE,  ///< signed greater or equal
			ICmpInst::ICMP_SLE,  ///< signed less or equal
			ICmpInst::ICMP_SGT,  ///< signed greater than. 
		};

class LLVM_ir_export
{
	public:
		int find_function_member_node(struct self_s *self, struct external_entry_point_s *external_entry_point, int node_to_find, int *member_node);
		int add_instruction(struct self_s *self, Module *mod, struct declaration_s *declaration, Value **value, BasicBlock **bb, int node, int external_entry, int inst);
		int add_node_instructions(struct self_s *self, Module *mod, struct declaration_s *declaration, Value **value, BasicBlock **bb, int node, int external_entry);
		int fill_value(struct self_s *self, Value **value, int value_id, int external_entry);
		int output(struct self_s *self);


	private:
		LLVMContext Context;
};

int LLVM_ir_export::find_function_member_node(struct self_s *self, struct external_entry_point_s *external_entry_point, int node_to_find, int *member_node)
{
	int found = 1;
	int n;

	*member_node = 0;
	for (n = 0; n < external_entry_point->member_nodes_size; n++) {
		if (node_to_find == external_entry_point->member_nodes[n]) {
			found = 0;
			*member_node = n;
			break;
		}
	}
	return found;
}

int sprint_value(raw_string_ostream &OS1, Value *valueA)
{
	valueA->print(OS1);
	OS1 << "\n";
	OS1.flush();
	return 0;
}

int sprint_srcA_srcB(raw_string_ostream &OS1, Value *srcA, Value *srcB)
{
	if (!srcA) {
		debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: srcA NULL\n");
		exit(1);
	}
	if (!srcB) {
		debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: srcB NULL\n");
		exit(1);
	}
	srcA->print(OS1);
	OS1 << ", ";
	srcB->print(OS1);
	OS1 << ", ";
	OS1 << "srcA_type = ";
	srcA->getType()->print(OS1);
	OS1 << ", srcB_type = ";
	srcB->getType()->print(OS1);
	OS1.flush();
	return 0;
}


int LLVM_ir_export::add_instruction(struct self_s *self, Module *mod, struct declaration_s *declaration, Value **value, BasicBlock **bb, int node, int external_entry, int inst)
{
	struct inst_log_entry_s *inst_log_entry = self->inst_log_entry;
	struct inst_log_entry_s *inst_log1 = &inst_log_entry[inst];
	struct external_entry_point_s *external_entry_point = &(self->external_entry_points[external_entry]);
	struct control_flow_node_s *nodes = external_entry_point->nodes;;
	Value *srcA;
	Value *srcB;
	Value *dstA;
	Value *value_tmp;
	uint64_t srcA_size;
	uint64_t srcB_size;
	int value_id;
	int value_id_dst;
	struct label_s *label;
	int tmp;
	char buffer[1024];
	int node_true;
	int node_false;
	int result = 0;
	int n;
	std::string Buf1;
	raw_string_ostream OS1(Buf1);

	switch (inst_log1->instruction.opcode) {
	case 1:  // MOV
		/* 2 forms, 1) MOV REG,REG and 2) MOV IMM,REG
		 * (1) is a NOP in LLVM IR, (2) is a fill value but no OP.
		 */
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:MOV\n", inst, inst_log1->instruction.opcode);
//		if (inst_log1->instruction.dstA.index == 0x28) {
//			/* Skip the 0x28 reg as it is the SP reg */
//			break;
//		}
		debug_print(DEBUG_OUTPUT_LLVM, 1, "value_id1 = 0x%lx->0x%lx, value_id3 = 0x%lx->0x%lx\n",
			inst_log1->value1.value_id,
			external_entry_point->label_redirect[inst_log1->value1.value_id].redirect,
			inst_log1->value3.value_id,
			external_entry_point->label_redirect[inst_log1->value3.value_id].redirect);
		if (inst_log1->instruction.srcA.store == 0) {  /* IMM */
			value_id = external_entry_point->label_redirect[inst_log1->value1.value_id].redirect;
			if (!value[value_id]) {
				tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
				if (tmp) {
					debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. dstA value_id = 0x%x\n", value_id);
					exit(1);
				}
			}
		}
		break;
	case 2:  // ADD
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:ADD\n", inst, inst_log1->instruction.opcode);
//		if (inst_log1->instruction.dstA.index == 0x28) {
//			/* Skip the 0x28 reg as it is the SP reg */
//			break;
//		}
		debug_print(DEBUG_OUTPUT_LLVM, 1, "value_id1 = 0x%lx->0x%lx, value_id2 = 0x%lx->0x%lx\n",
			inst_log1->value1.value_id,
			external_entry_point->label_redirect[inst_log1->value1.value_id].redirect,
			inst_log1->value2.value_id,
			external_entry_point->label_redirect[inst_log1->value2.value_id].redirect);
		value_id = external_entry_point->label_redirect[inst_log1->value1.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcA value_id = 0x%x\n", value_id);
				exit(1);
			}
		}
		srcA = value[value_id];
		value_id = external_entry_point->label_redirect[inst_log1->value2.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcB value_id = 0x%x\n", value_id);
				exit(1);
			}
		}
		srcB = value[value_id];

		debug_print(DEBUG_OUTPUT_LLVM, 1, "srcA = %p, srcB = %p\n", srcA, srcB);
		sprint_srcA_srcB(OS1, srcA, srcB);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();

		tmp = label_to_string(&external_entry_point->labels[inst_log1->value3.value_id], buffer, 1023);
		dstA = BinaryOperator::CreateAdd(srcA, srcB, buffer, bb[node]);
		value[inst_log1->value3.value_id] = dstA;
		sprint_value(OS1, dstA);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();
		break;
	case 4:  // SUB
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:SUB\n", inst, inst_log1->instruction.opcode);
//		if (inst_log1->instruction.dstA.index == 0x28) {
//			/* Skip the 0x28 reg as it is the SP reg */
//			break;
//		}
		debug_print(DEBUG_OUTPUT_LLVM, 1, "value_id1 = 0x%lx->0x%lx, value_id2 = 0x%lx->0x%lx\n",
			inst_log1->value1.value_id,
			external_entry_point->label_redirect[inst_log1->value1.value_id].redirect,
			inst_log1->value2.value_id,
			external_entry_point->label_redirect[inst_log1->value2.value_id].redirect);
		value_id = external_entry_point->label_redirect[inst_log1->value1.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcA value_id = 0x%x\n", value_id);
				exit(1);
			}
		}
		srcA = value[value_id];
		srcA_size = external_entry_point->labels[value_id].size_bits;
		debug_print(DEBUG_OUTPUT_LLVM, 1, "srcA: scope=0x%lx, type=0x%lx value=0x%lx size_bits=0x%lx pointer_type_size_bits=0x%lx lab_pointer=0x%lx lab_signed=0x%lx lab_unsigned=0x%lx name=%s\n",
			external_entry_point->labels[value_id].scope,
			external_entry_point->labels[value_id].type,
			external_entry_point->labels[value_id].value,
			external_entry_point->labels[value_id].size_bits,
			external_entry_point->labels[value_id].pointer_type_size_bits,
			external_entry_point->labels[value_id].lab_pointer,
			external_entry_point->labels[value_id].lab_signed,
			external_entry_point->labels[value_id].lab_unsigned,
			external_entry_point->labels[value_id].name);

		value_id = external_entry_point->label_redirect[inst_log1->value2.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcB value_id = 0x%x\n", value_id);
				exit(1);
			}
		}
		srcB = value[value_id];
		srcB_size = external_entry_point->labels[value_id].size_bits;
		debug_print(DEBUG_OUTPUT_LLVM, 1, "srcB: scope=0x%lx, type=0x%lx value=0x%lx size_bits=0x%lx pointer_type_size_bits=0x%lx lab_pointer=0x%lx lab_signed=0x%lx lab_unsigned=0x%lx name=%s\n",
			external_entry_point->labels[value_id].scope,
			external_entry_point->labels[value_id].type,
			external_entry_point->labels[value_id].value,
			external_entry_point->labels[value_id].size_bits,
			external_entry_point->labels[value_id].pointer_type_size_bits,
			external_entry_point->labels[value_id].lab_pointer,
			external_entry_point->labels[value_id].lab_signed,
			external_entry_point->labels[value_id].lab_unsigned,
			external_entry_point->labels[value_id].name);

		debug_print(DEBUG_OUTPUT_LLVM, 1, "srcA = %p, srcB = %p\n", srcA, srcB);

		sprint_srcA_srcB(OS1, srcA, srcB);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();

		debug_print(DEBUG_OUTPUT_LLVM, 1, "srcA_size = 0x%lx, srcB_size = 0x%lx\n", srcA_size, srcB_size);
		tmp = label_to_string(&external_entry_point->labels[inst_log1->value3.value_id], buffer, 1023);
		dstA = BinaryOperator::CreateSub(srcA, srcB, buffer, bb[node]);
		value[inst_log1->value3.value_id] = dstA;
		sprint_value(OS1, dstA);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();
		break;
	case 7:  // XOR
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:XOR\n", inst, inst_log1->instruction.opcode);
//		if (inst_log1->instruction.dstA.index == 0x28) {
//			/* Skip the 0x28 reg as it is the SP reg */
//			break;
//		}
		debug_print(DEBUG_OUTPUT_LLVM, 1, "value_id1 = 0x%lx->0x%lx, value_id2 = 0x%lx->0x%lx\n",
			inst_log1->value1.value_id,
			external_entry_point->label_redirect[inst_log1->value1.value_id].redirect,
			inst_log1->value2.value_id,
			external_entry_point->label_redirect[inst_log1->value2.value_id].redirect);
		value_id = external_entry_point->label_redirect[inst_log1->value1.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcA value_id = 0x%x\n", value_id);
				exit(1);
			}
		}
		srcA = value[value_id];
		value_id = external_entry_point->label_redirect[inst_log1->value2.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcB value_id = 0x%x\n", value_id);
				exit(1);
			}
		}
		srcB = value[value_id];

		debug_print(DEBUG_OUTPUT_LLVM, 1, "srcA = %p, srcB = %p\n", srcA, srcB);
		sprint_srcA_srcB(OS1, srcA, srcB);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();

		tmp = label_to_string(&external_entry_point->labels[inst_log1->value3.value_id], buffer, 1023);
		dstA = BinaryOperator::CreateXor(srcA, srcB, buffer, bb[node]);
		value[inst_log1->value3.value_id] = dstA;
		sprint_value(OS1, dstA);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();
		break;
	case 0xd:  // MUL
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:MUL\n", inst, inst_log1->instruction.opcode);
//		if (inst_log1->instruction.dstA.index == 0x28) {
//			/* Skip the 0x28 reg as it is the SP reg */
//			break;
//		}
		debug_print(DEBUG_OUTPUT_LLVM, 1, "value_id1 = 0x%lx->0x%lx, value_id2 = 0x%lx->0x%lx\n",
			inst_log1->value1.value_id,
			external_entry_point->label_redirect[inst_log1->value1.value_id].redirect,
			inst_log1->value2.value_id,
			external_entry_point->label_redirect[inst_log1->value2.value_id].redirect);
		value_id = external_entry_point->label_redirect[inst_log1->value1.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcA value_id = 0x%x\n", value_id);
				exit(1);
			}
		}
		srcA = value[value_id];
		value_id = external_entry_point->label_redirect[inst_log1->value2.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcB value_id = 0x%x\n", value_id);
				exit(1);
			}
		}
		srcB = value[value_id];
		debug_print(DEBUG_OUTPUT_LLVM, 1, "srcA = %p, srcB = %p\n", srcA, srcB);

		sprint_srcA_srcB(OS1, srcA, srcB);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();

		tmp = label_to_string(&external_entry_point->labels[inst_log1->value3.value_id], buffer, 1023);
		dstA = BinaryOperator::Create(Instruction::Mul, srcA, srcB, buffer, bb[node]);
		value[inst_log1->value3.value_id] = dstA;
		sprint_value(OS1, dstA);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();
		break;
	case 0xe:  // IMUL
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:IMUL\n", inst, inst_log1->instruction.opcode);
//		if (inst_log1->instruction.dstA.index == 0x28) {
//			/* Skip the 0x28 reg as it is the SP reg */
//			break;
//		}
		debug_print(DEBUG_OUTPUT_LLVM, 1, "value_id1 = 0x%lx->0x%lx, value_id2 = 0x%lx->0x%lx\n",
			inst_log1->value1.value_id,
			external_entry_point->label_redirect[inst_log1->value1.value_id].redirect,
			inst_log1->value2.value_id,
			external_entry_point->label_redirect[inst_log1->value2.value_id].redirect);
		value_id = external_entry_point->label_redirect[inst_log1->value1.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcA value_id = 0x%x\n", value_id);
				exit(1);
			}
		}
		srcA = value[value_id];
		value_id = external_entry_point->label_redirect[inst_log1->value2.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcB value_id = 0x%x\n", value_id);
				exit(1);
			}
		}
		srcB = value[value_id];
		debug_print(DEBUG_OUTPUT_LLVM, 1, "srcA = %p, srcB = %p\n", srcA, srcB);

		sprint_srcA_srcB(OS1, srcA, srcB);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();

		tmp = label_to_string(&external_entry_point->labels[inst_log1->value3.value_id], buffer, 1023);
		dstA = BinaryOperator::Create(Instruction::Mul, srcA, srcB, buffer, bb[node]);
		value[inst_log1->value3.value_id] = dstA;
		sprint_value(OS1, dstA);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();
		break;
	case 0x11:  // JMP
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:JMP node_end = 0x%x\n", inst, inst_log1->instruction.opcode, inst_log1->node_end);
		if (inst_log1->node_end) {
			node_true = nodes[node].link_next[0].node;
			dstA = BranchInst::Create(bb[node_true], bb[node]);
			sprint_value(OS1, dstA);
			debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
			Buf1.clear();
			result = 1;
		}
		break;
	case 0x12:  // CALL
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:CALL\n", inst, inst_log1->instruction.opcode);
		{
			struct extension_call_s *call_info = static_cast<struct extension_call_s *> (inst_log1->extension);
			std::vector<Value*> vector_params;
			int function_to_call = 0;

			debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: params_size = 0x%x:0x%x\n", inst, call_info->params_reg_size, declaration[0].FT->getNumParams());
			for (n = 0; n < call_info->params_reg_size; n++) {
				value_id = external_entry_point->label_redirect[call_info->params_reg[n]].redirect;
				debug_print(DEBUG_OUTPUT_LLVM, 1, "call_info_params = 0x%x->0x%x, %p\n", call_info->params_reg[n], value_id, value[value_id]);
				if (!value_id) {
					debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: invalid call_info_param\n");
					exit(1);
				}
				vector_params.push_back(value[value_id]);
			}
			PointerType* PointerTy_1 = PointerType::get(IntegerType::get(mod->getContext(), 64), 0);
			ConstantPointerNull* const_ptr_5 = ConstantPointerNull::get(PointerTy_1);
			vector_params.push_back(const_ptr_5); /* EIP */
			debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: args_size = 0x%lx\n", inst, vector_params.size());
			tmp = label_to_string(&external_entry_point->labels[inst_log1->value3.value_id], buffer, 1023);
			declaration[0].F->print(OS1);
			debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
			Buf1.clear();
			debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: declaration dump done.\n", inst);
			for(auto i : vector_params) {
				debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: dumping vector_params %p\n", inst, i);
				if (i) {
					i->print(OS1);
					debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
					Buf1.clear();
				}
			}
			function_to_call = 0;
			if ((1 == inst_log1->instruction.srcA.relocated) &&
				(STORE_DIRECT == inst_log1->instruction.srcA.store)) {
				function_to_call = inst_log1->instruction.srcA.index;
			}
			CallInst* call_inst = CallInst::Create(declaration[function_to_call].F, vector_params, buffer, bb[node]);
			debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: call_inst %p\n", inst, call_inst);

			call_inst->setCallingConv(CallingConv::C);
			call_inst->setTailCall(false);
			dstA = call_inst;
			value[inst_log1->value3.value_id] = dstA;
			debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: dstA %p\n", inst, dstA);
			sprint_value(OS1, dstA);
			debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
			Buf1.clear();
		}
		break;
	case 0x1e:  // RET
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:RET\n", inst, inst_log1->instruction.opcode);
		value_id = external_entry_point->label_redirect[inst_log1->value1.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL\n");
				result = 2;
				exit(1);
				break;
			}
		}
		srcA = value[value_id];
		sprint_value(OS1, srcA);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();
		dstA = ReturnInst::Create(Context, srcA, bb[node]);
		sprint_value(OS1, dstA);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();
		result = 1;
		break;
	case 0x1f:  // SEX
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:SEX\n", inst, inst_log1->instruction.opcode);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "value_id1 = 0x%lx->0x%lx, value_id3 = 0x%lx->0x%lx\n",
			inst_log1->value1.value_id,
			external_entry_point->label_redirect[inst_log1->value1.value_id].redirect,
			inst_log1->value3.value_id,
			external_entry_point->label_redirect[inst_log1->value3.value_id].redirect);
		value_id = external_entry_point->label_redirect[inst_log1->value1.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcA value_id = 0x%x\n", value_id);
				exit(1);
			}
		}
		srcA = value[value_id];
		sprint_value(OS1, srcA);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();
		value_id_dst = external_entry_point->label_redirect[inst_log1->value3.value_id].redirect;
		label = &external_entry_point->labels[value_id_dst];
		tmp = label_to_string(label, buffer, 1023);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "label->size_bits = 0x%lx\n", label->size_bits);
		dstA = new SExtInst(srcA, IntegerType::get(mod->getContext(), label->size_bits), buffer, bb[node]);
		value[value_id_dst] = dstA;
		sprint_value(OS1, dstA);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();
		break;
	case 0x23:  // ICMP
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:ICMP\n", inst, inst_log1->instruction.opcode);
//		if (inst_log1->instruction.dstA.index == 0x28) {
//			/* Skip the 0x28 reg as it is the SP reg */
//			break;
//		}
		debug_print(DEBUG_OUTPUT_LLVM, 1, "ICMP predicate = 0x%x\n", inst_log1->instruction.predicate);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "value_id1 = 0x%lx->0x%lx, value_id2 = 0x%lx->0x%lx\n",
			inst_log1->value1.value_id,
			external_entry_point->label_redirect[inst_log1->value1.value_id].redirect,
			inst_log1->value2.value_id,
			external_entry_point->label_redirect[inst_log1->value2.value_id].redirect);
		value_id = external_entry_point->label_redirect[inst_log1->value1.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				tmp = label_to_string(&external_entry_point->labels[value_id], buffer, 1023);
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcA value_id = 0x%x:%s\n", value_id, buffer);
				exit(1);
			}
		}
		srcA = value[value_id];
		value_id = external_entry_point->label_redirect[inst_log1->value2.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				tmp = label_to_string(&external_entry_point->labels[value_id], buffer, 1023);
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcB value_id = 0x%x:%s\n", value_id, buffer);
				exit(1);
			}
		}
		srcB = value[value_id];

		debug_print(DEBUG_OUTPUT_LLVM, 1, "srcA = %p, srcB = %p\n", srcA, srcB);
		sprint_srcA_srcB(OS1, srcA, srcB);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();

		tmp = label_to_string(&external_entry_point->labels[inst_log1->value3.value_id], buffer, 1023);
		//dstA = new ICmpInst(*bb, ICmpInst::ICMP_EQ, srcA, srcB, buffer);
		dstA = new ICmpInst(*bb[node], predicate_to_llvm_table[inst_log1->instruction.predicate], srcA, srcB, buffer);
		value[inst_log1->value3.value_id] = dstA;
		sprint_value(OS1, dstA);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();
		break;
	case 0x24:  // BC
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:BC\n", inst, inst_log1->instruction.opcode);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "value_id1 = 0x%lx->0x%lx\n",
			inst_log1->value1.value_id,
			external_entry_point->label_redirect[inst_log1->value1.value_id].redirect);
		value_id = external_entry_point->label_redirect[inst_log1->value1.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				tmp = label_to_string(&external_entry_point->labels[value_id], buffer, 1023);
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcA value_id = 0x%x:%s\n", value_id, buffer);
				exit(1);
			}
		}
		srcA = value[value_id];
		sprint_value(OS1, srcA);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();
		//BranchInst::Create(label_7, label_9, int1_11, label_6);
		node_true = nodes[node].link_next[0].node;
		node_false = nodes[node].link_next[1].node;
		dstA = BranchInst::Create(bb[node_true], bb[node_false], srcA, bb[node]);
		sprint_value(OS1, dstA);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();
		result = 1;
		break;
	case 0x25:  // LOAD
		LoadInst* dstA_load;
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:LOAD\n", inst, inst_log1->instruction.opcode);
//		if (inst_log1->instruction.dstA.index == 0x28) {
//			/* Skip the 0x28 reg as it is the SP reg */
//			break;
//		}
		switch (inst_log1->instruction.srcA.indirect) {
		case 1:  // Memory
			debug_print(DEBUG_OUTPUT_LLVM, 1, "value_id1 = 0x%lx->0x%lx, value_id2 = 0x%lx->0x%lx value_id3 = 0x%lx->0x%lx\n",
				inst_log1->value1.value_id,
				external_entry_point->label_redirect[inst_log1->value1.value_id].redirect,
				inst_log1->value2.value_id,
				external_entry_point->label_redirect[inst_log1->value2.value_id].redirect,
				inst_log1->value3.value_id,
				external_entry_point->label_redirect[inst_log1->value3.value_id].redirect);
			value_id = external_entry_point->label_redirect[inst_log1->value1.value_id].redirect;
			if (!value[value_id]) {
				tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
				if (tmp) {
					debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcA value_id = 0x%x\n", value_id);
					exit(1);
				}
			}
			srcA = value[value_id];
			value_id = external_entry_point->label_redirect[inst_log1->value2.value_id].redirect;
			if (!value[value_id]) {
				tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
				if (tmp) {
					debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcB value_id = 0x%x\n", value_id);
					exit(1);
				}
			}
			srcB = value[value_id];

			debug_print(DEBUG_OUTPUT_LLVM, 1, "srcA = %p, srcB = %p\n", srcA, srcB);
			sprint_srcA_srcB(OS1, srcA, srcB);
			debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
			Buf1.clear();

			value_id_dst = external_entry_point->label_redirect[inst_log1->value3.value_id].redirect;
			label = &external_entry_point->labels[value_id_dst];
			tmp = label_to_string(label, buffer, 1023);
			dstA_load = new LoadInst(srcA, buffer, false, bb[node]);
			dstA_load->setAlignment(label->size_bits >> 3);
			dstA = dstA_load;

			dstA->print(OS1);
			OS1.flush();
			debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
			Buf1.clear();

			if (value_id_dst) {
				value[value_id_dst] = dstA;
			} else {
				debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: FIXME: Invalid value_id\n", inst);
			}
			break;
		case 2:  // Stack
			debug_print(DEBUG_OUTPUT_LLVM, 1, "value_id1 = 0x%lx->0x%lx, value_id2 = 0x%lx->0x%lx value_id3 = 0x%lx->0x%lx\n",
				inst_log1->value1.value_id,
				external_entry_point->label_redirect[inst_log1->value1.value_id].redirect,
				inst_log1->value2.value_id,
				external_entry_point->label_redirect[inst_log1->value2.value_id].redirect,
				inst_log1->value3.value_id,
				external_entry_point->label_redirect[inst_log1->value3.value_id].redirect);
			value_id = external_entry_point->label_redirect[inst_log1->value1.value_id].redirect;
			if (!value[value_id]) {
				tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
				if (tmp) {
					debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcA value_id = 0x%x\n", value_id);
					exit(1);
				}
			}
			srcA = value[value_id];
			value_id = external_entry_point->label_redirect[inst_log1->value2.value_id].redirect;
			if (!value[value_id]) {
				tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
				if (tmp) {
					debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcB value_id = 0x%x\n", value_id);
					exit(1);
				}
			}
			srcB = value[value_id];
			debug_print(DEBUG_OUTPUT_LLVM, 1, "srcA = %p, srcB = %p\n", srcA, srcB);
			sprint_srcA_srcB(OS1, srcA, srcB);
			debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
			Buf1.clear();

			value_id_dst = external_entry_point->label_redirect[inst_log1->value3.value_id].redirect;
			label = &external_entry_point->labels[value_id_dst];
			tmp = label_to_string(label, buffer, 1023);
			dstA_load = new LoadInst(srcA, buffer, false, bb[node]);
			dstA_load->setAlignment(label->size_bits >> 3);
			dstA = dstA_load;

			dstA->print(OS1);
			OS1.flush();
			debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
			Buf1.clear();

			if (value_id_dst) {
				value[value_id_dst] = dstA;
			} else {
				debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: FIXME: Invalid value_id\n", inst);
			}
			break;
		default:
			debug_print(DEBUG_OUTPUT_LLVM, 1, "FIXME: LOAD Indirect = 0x%x not yet handled\n", inst_log1->instruction.srcA.indirect);
			break;
		}
		break;
	case 0x26:  // STORE
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:STORE\n", inst, inst_log1->instruction.opcode);
//		if (inst_log1->instruction.dstA.index == 0x28) {
//			/* Skip the 0x28 reg as it is the SP reg */
//			break;
//		}
		debug_print(DEBUG_OUTPUT_LLVM, 1, "value_id1 = 0x%lx->0x%lx, value_id2 = 0x%lx->0x%lx value_id3 = 0x%lx->0x%lx\n",
			inst_log1->value1.value_id,
			external_entry_point->label_redirect[inst_log1->value1.value_id].redirect,
			inst_log1->value2.value_id,
			external_entry_point->label_redirect[inst_log1->value2.value_id].redirect,
			inst_log1->value3.value_id,
			external_entry_point->label_redirect[inst_log1->value3.value_id].redirect);
		value_id = external_entry_point->label_redirect[inst_log1->value1.value_id].redirect;
		if (value_id) {
			debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: srcA value_id 0x%x\n", inst, value_id);
			if (!value[value_id]) {
				tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
				if (tmp) {
					debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcA value_id = 0x%x\n", value_id);
					exit(1);
				}
			}
			srcA = value[value_id];
		} else {
			debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: FIXME: Invalid srcA value_id\n", inst);
			break;
		}
		/* Note: The srcB here should be value3 as it is a STORE instruction */
		/*       But it depends on whether the value3 is a constant or a calculated pointer */
		value_id = external_entry_point->label_redirect[inst_log1->value2.value_id].redirect;
		if (value_id) {
			debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: dstA value_id 0x%x\n", inst, value_id);
			if (!value[value_id]) {
				tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
				if (tmp) {
					debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. dstA value_id = 0x%x\n", value_id);
					exit(1);
				}
			}
			srcB = value[value_id];
		} else {
			debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: FIXME: Invalid dstA value_id\n", inst);
			break;
		}
		debug_print(DEBUG_OUTPUT_LLVM, 1, "srcA = %p, dstA = %p\n", srcA, srcB);
		sprint_srcA_srcB(OS1, srcA, srcB);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();
		dstA = new StoreInst(srcA, srcB, false, bb[node]);
		dstA->print(OS1);
		OS1 << "\n";
		OS1.flush();
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();
		break;
	case 0x2F:  // GEP1
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:GEP1\n", inst, inst_log1->instruction.opcode);
//		if (inst_log1->instruction.dstA.index == 0x28) {
//			/* Skip the 0x28 reg as it is the SP reg */
//			break;
//		}
		debug_print(DEBUG_OUTPUT_LLVM, 1, "value_id1 = 0x%lx->0x%lx, value_id2 = 0x%lx->0x%lx value_id3 = 0x%lx->0x%lx\n",
			inst_log1->value1.value_id,
			external_entry_point->label_redirect[inst_log1->value1.value_id].redirect,
			inst_log1->value2.value_id,
			external_entry_point->label_redirect[inst_log1->value2.value_id].redirect,
			inst_log1->value3.value_id,
			external_entry_point->label_redirect[inst_log1->value3.value_id].redirect);
		value_id = external_entry_point->label_redirect[inst_log1->value1.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcA value_id = 0x%x\n", value_id);
				exit(1);
			}
		}
		srcA = value[value_id];
		value_id = external_entry_point->label_redirect[inst_log1->value2.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcB value_id = 0x%x\n", value_id);
				exit(1);
			}
		}
		srcB = value[value_id];
		label = &external_entry_point->labels[value_id];
		if (label->lab_pointer) {
			/* Swap srcA and srcB */
			debug_print(DEBUG_OUTPUT_LLVM, 1, "GEP swap srcA and srcB\n");
			value_tmp = srcA;
			srcA = srcB;
			srcB = value_tmp;
		}

		debug_print(DEBUG_OUTPUT_LLVM, 1, "srcA = %p, srcB = %p\n", srcA, srcB);
		sprint_srcA_srcB(OS1, srcA, srcB);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();

		tmp = label_to_string(&external_entry_point->labels[inst_log1->value3.value_id], buffer, 1023);
		dstA = GetElementPtrInst::Create(srcA, srcB, buffer, bb[node]);
		value[inst_log1->value3.value_id] = dstA;

		dstA->print(OS1);
		OS1 << "\n";
		OS1.flush();
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();
		break;

	case 0x36:  // TRUNC
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:TRUNC\n", inst, inst_log1->instruction.opcode);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "value_id1 = 0x%lx->0x%lx, value_id3 = 0x%lx->0x%lx\n",
			inst_log1->value1.value_id,
			external_entry_point->label_redirect[inst_log1->value1.value_id].redirect,
			inst_log1->value3.value_id,
			external_entry_point->label_redirect[inst_log1->value3.value_id].redirect);
		value_id = external_entry_point->label_redirect[inst_log1->value1.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcA value_id = 0x%x\n", value_id);
				exit(1);
			}
		}
		srcA = value[value_id];

		srcA->print(OS1);
		OS1 << "\n";
		OS1.flush();
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();

		value_id_dst = external_entry_point->label_redirect[inst_log1->value3.value_id].redirect;
		label = &external_entry_point->labels[value_id_dst];
		tmp = label_to_string(label, buffer, 1023);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "label->size_bits = 0x%lx\n", label->size_bits);
		dstA = new TruncInst(srcA, IntegerType::get(mod->getContext(), label->size_bits), buffer, bb[node]);
		value[value_id_dst] = dstA;

		dstA->print(OS1);
		OS1 << "\n";
		OS1.flush();
		debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
		Buf1.clear();
		break;

	case 0x37:  // ZEXT
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM 0x%x: OPCODE = 0x%x:ZEXT\n", inst, inst_log1->instruction.opcode);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "value_id1 = 0x%lx->0x%lx, value_id3 = 0x%lx->0x%lx\n",
			inst_log1->value1.value_id,
			external_entry_point->label_redirect[inst_log1->value1.value_id].redirect,
			inst_log1->value3.value_id,
			external_entry_point->label_redirect[inst_log1->value3.value_id].redirect);
		value_id = external_entry_point->label_redirect[inst_log1->value1.value_id].redirect;
		if (!value[value_id]) {
			tmp = LLVM_ir_export::fill_value(self, value, value_id, external_entry);
			if (tmp) {
				debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: failed LLVM Value is NULL. srcA value_id = 0x%x\n", value_id);
				exit(1);
			}
		}
		srcA = value[value_id];
		value_id_dst = external_entry_point->label_redirect[inst_log1->value3.value_id].redirect;
		label = &external_entry_point->labels[value_id_dst];
		tmp = label_to_string(label, buffer, 1023);
		debug_print(DEBUG_OUTPUT_LLVM, 1, "label->size_bits = 0x%lx\n", label->size_bits);
		dstA = new ZExtInst(srcA, IntegerType::get(mod->getContext(), label->size_bits), buffer, bb[node]);
		value[value_id_dst] = dstA;
		break;

	default:
		debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: LLVM 0x%x: OPCODE = 0x%x. Not yet handled.\n", inst, inst_log1->instruction.opcode);
		exit(1);
		result = 1;
		break;
	}

	return result;
} 

int LLVM_ir_export::add_node_instructions(struct self_s *self, Module *mod, struct declaration_s *declaration, Value** value, BasicBlock **bb, int node, int external_entry) 
{
	struct inst_log_entry_s *inst_log1;
	struct inst_log_entry_s *inst_log_entry = self->inst_log_entry;
	struct external_entry_point_s *external_entry_point = &(self->external_entry_points[external_entry]);
	struct control_flow_node_s *nodes = external_entry_point->nodes;
	int nodes_size = external_entry_point->nodes_size;
	int l,m,n;
	int inst;
	int inst_next;
	int tmp;
	int node_true;
	int block_end;

	debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM Node 0x%x\n", node);
	inst = nodes[node].inst_start;
	inst_next = inst;

	do {
		inst = inst_next;
		inst_log1 =  &inst_log_entry[inst];
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM node end: inst_end = 0x%x, next_size = 0x%x, node_end = 0x%x\n",
			nodes[node].inst_end, inst_log1->next_size, inst_log1->node_end);
		tmp = add_instruction(self, mod, declaration, value, bb, node, external_entry, inst);
		if (inst_log1->next_size > 0) {
			inst_next = inst_log1->next[0];
		}
		debug_print(DEBUG_OUTPUT_LLVM, 1, "tmp = 0x%x\n", tmp);
		/* FIXME: is tmp really needed for block_end detection? */
		block_end = (inst_log1->node_end || !(inst_log1->next_size) || tmp);
		//block_end = (inst_log1->node_end || !(inst_log1->next_size));
	} while (!block_end);

	if (!tmp) {
		/* Only output the extra branch if the node did not do any branches or returns itself. */
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM node end: node = 0x%x, inst_end = 0x%x, next_size = 0x%x\n",
			node, nodes[node].inst_end, nodes[node].next_size);
		node_true = nodes[node].link_next[0].node;
		BranchInst::Create(bb[node_true], bb[node]);
	}
	return 0;
}

int LLVM_ir_export::fill_value(struct self_s *self, Value **value, int value_id, int external_entry)
{
	struct external_entry_point_s *external_entry_point = &(self->external_entry_points[external_entry]);
	struct label_s *label = &(external_entry_point->labels[value_id]);
	int labels_size = external_entry_point->variable_id;

	if ((label->scope == 3) &&
		(label->type == 3)) {
		if (label->size_bits == 32) {
			value[value_id] = ConstantInt::get(Type::getInt32Ty(Context), label->value);
		} else if (label->size_bits == 64) {
			value[value_id] = ConstantInt::get(Type::getInt64Ty(Context), label->value);
		} else {
			debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM fill_value() failed with size_bits = 0x%lx\n", label->size_bits);
			return 1;
		}
		return 0;
	} else {
		debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM fill_value(): value_id = 0x%x, label->scope = 0x%lx, label->type = 0x%lx\n",
			value_id,
			label->scope,
			label->type);
	}

	return 1;
}

int LLVM_ir_export::output(struct self_s *self)
{
	const char *function_name = "test123";
	char output_filename[512];
	int n;
	int m;
	int l;
	int tmp;
	struct control_flow_node_s *nodes;
	int nodes_size;
	int node;
	struct label_s *labels;
	int labels_size;
	struct label_redirect_s *label_redirect;
	struct label_s *label;
	char buffer[1024];
	int index;
	std::string Buf1;
	raw_string_ostream OS1(Buf1);
	
	struct external_entry_point_s *external_entry_points = self->external_entry_points;
	struct declaration_s *declaration = static_cast <struct declaration_s *> (calloc(EXTERNAL_ENTRY_POINTS_MAX, sizeof (struct declaration_s)));

	for (n = 0; n < EXTERNAL_ENTRY_POINTS_MAX; n++) {
		if ((external_entry_points[n].valid != 0) &&
			(external_entry_points[n].type == 1) && 
			(external_entry_points[n].nodes_size)) {
			Value** value = (Value**) calloc(external_entry_points[n].variable_id, sizeof(Value*));
			nodes = external_entry_points[n].nodes;
			nodes_size = external_entry_points[n].nodes_size;
			labels = external_entry_points[n].labels;
			labels_size = external_entry_points[n].variable_id;
			label_redirect = external_entry_points[n].label_redirect;
			Module *mod = new Module("test_llvm_export", Context);
 			mod->setDataLayout("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128");
			mod->setTargetTriple("x86_64-pc-linux-gnu");

			/* Add globals */
			for (m = 0; m < labels_size; m++) {
				label = &labels[label_redirect[m].redirect];
				if ((3 == label->scope) && (2 == label->type)) {
					debug_print(DEBUG_OUTPUT_LLVM, 1, "Label:0x%x: &data found. size=0x%lx\n", m, label->size_bits);
					GlobalVariable* gvar_int32_mem1 = new GlobalVariable(/*Module=*/*mod,
						/*Type=*/IntegerType::get(mod->getContext(), label->size_bits),
						/*isConstant=*/false,
						/*Linkage=*/GlobalValue::InternalLinkage,
						/*Initializer=*/0, // has initializer, specified below
						/*Name=*/"data0");
					gvar_int32_mem1->setAlignment(label->size_bits >> 3);
					value[m] = gvar_int32_mem1;
				}
			}

			for (l = 0; l < EXTERNAL_ENTRY_POINTS_MAX; l++) {
				if ((external_entry_points[l].valid != 0) &&
					(external_entry_points[l].type == 1) &&
					(n == 0)) {
					//std::vector<Type*>FuncTy_0_args;
					struct label_s *labels_ext = external_entry_points[l].labels;
					for (m = 0; m < external_entry_points[l].params_reg_ordered_size; m++) {
						index = external_entry_points[l].params_reg_ordered[m];
						if (labels_ext[index].lab_pointer > 0) {
							int size = labels_ext[index].pointer_type_size_bits;
							debug_print(DEBUG_OUTPUT_LLVM, 1, "Reg Param=0x%x: Pointer Label 0x%x, size_bits = 0x%x\n", m, index, size);
							if (size < 8) {
								debug_print(DEBUG_OUTPUT_LLVM, 1, "FIXME: size too small\n");
								size = 8;
							}
							declaration[l].FuncTy_0_args.push_back(PointerType::get(IntegerType::get(mod->getContext(), size), 0));
						} else {
							int size = labels_ext[index].size_bits;
							debug_print(DEBUG_OUTPUT_LLVM, 1, "Reg Param=0x%x: Label 0x%x, size_bits = 0x%x\n", m, index, size);
							declaration[l].FuncTy_0_args.push_back(IntegerType::get(mod->getContext(), size));
						}
					}
					for (m = 0; m < external_entry_points[l].params_stack_ordered_size; m++) {
						index = external_entry_points[l].params_stack_ordered[m];
						if (index == 3) {
						/* EIP or param_stack0000 */
						}
						if (labels_ext[index].lab_pointer > 0) {
							int size = labels_ext[index].pointer_type_size_bits;
							debug_print(DEBUG_OUTPUT_LLVM, 1, "Stack Param=0x%x: Pointer Label 0x%x, size_bits = 0x%x\n", m, index, size);
							if (size < 8) {
								debug_print(DEBUG_OUTPUT_LLVM, 1, "FIXME: size too small\n");
								size = 64;
							}
							declaration[l].FuncTy_0_args.push_back(PointerType::get(IntegerType::get(mod->getContext(), size), 0));
						} else {
							int size = labels_ext[index].size_bits;
							debug_print(DEBUG_OUTPUT_LLVM, 1, "Stack Param=0x%x: Label 0x%x, size_bits = 0x%x\n", m, index, size);
							declaration[l].FuncTy_0_args.push_back(IntegerType::get(mod->getContext(), size));
						}
					}
				}
			}

			for (l = 0; l < EXTERNAL_ENTRY_POINTS_MAX; l++) {
				if ((external_entry_points[l].valid != 0) &&
					(external_entry_points[l].type == 1)) {
					FunctionType *FT =
						FunctionType::get(Type::getInt32Ty(Context),
							declaration[l].FuncTy_0_args,
							false); /*not vararg*/
					declaration[l].FT = FT;
				}
			}
			for (l = 0; l < EXTERNAL_ENTRY_POINTS_MAX; l++) {
				if ((external_entry_points[l].valid != 0) &&
					(external_entry_points[l].type == 1)) {
					function_name = external_entry_points[l].name;
					Function *F =
						Function::Create(declaration[l].FT, Function::ExternalLinkage, function_name, mod);
					declaration[l].F = F;
					declaration[l].F->print(OS1);
					debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
					Buf1.clear();
				}
			}

#if 0
			for (l = 0; l < EXTERNAL_ENTRY_POINTS_MAX; l++) {
				if ((external_entry_points[l].valid != 0) &&
					(external_entry_points[l].type == 1)) {
					Function::arg_iterator args = declaration[l].F->arg_begin();
					debug_print(DEBUG_OUTPUT_LLVM, 1, "Function: %s()  param_size = 0x%x\n", function_name, external_entry_points[l].params_size);
					for (m = 0; m < external_entry_points[l].params_reg_ordered_size; m++) {
						index = external_entry_points[l].params_reg_ordered[m];
						tmp = label_to_string(&(labels[index]), buffer, 1023);
						debug_print(DEBUG_OUTPUT_LLVM, 1, "Adding reg param:%s:value index=0x%x\n", buffer, index);
						args->setName(buffer);
						args++;
					}
					for (m = 0; m < external_entry_points[l].params_stack_ordered_size; m++) {
						index = external_entry_points[l].params_stack_ordered[m];
						tmp = label_to_string(&(labels[index]), buffer, 1023);
						debug_print(DEBUG_OUTPUT_LLVM, 1, "Adding stack param:%s:value index=0x%x\n", buffer, index);
						args->setName(buffer);
						args++;
					}
					declaration[l].F->dump();
				}
			}
#endif


			function_name = external_entry_points[n].name;
			snprintf(output_filename, 500, "./llvm/%s.bc", function_name);
#if 1
			Function::arg_iterator args = declaration[n].F->arg_begin();
			debug_print(DEBUG_OUTPUT_LLVM, 1, "Function: %s()  param_size = 0x%x\n", function_name, external_entry_points[n].params_size);
			for (m = 0; m < external_entry_points[n].params_reg_ordered_size; m++) {
				index = external_entry_points[n].params_reg_ordered[m];
				if (!index) {
					debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: value[index]: Index = 0. \n");
					exit(1);
				}
				value[index] = args;
				tmp = label_to_string(&(labels[index]), buffer, 1023);
				debug_print(DEBUG_OUTPUT_LLVM, 1, "Adding reg param:%s:value index=0x%x\n", buffer, index);
				value[index]->setName(buffer);
				sprint_value(OS1, value[index]);
				debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
				Buf1.clear();
				args++;
			}
			for (m = 0; m < external_entry_points[n].params_stack_ordered_size; m++) {
				index = external_entry_points[n].params_stack_ordered[m];
				if (!index) {
					debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: value[index]: Index = 0. \n");
					exit(1);
				}
				value[index] = args;
				tmp = label_to_string(&(labels[index]), buffer, 1023);
				debug_print(DEBUG_OUTPUT_LLVM, 1, "Adding stack param:%s:value index=0x%x\n", buffer, index);
				value[index]->setName(buffer);
				sprint_value(OS1, value[index]);
				debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
				Buf1.clear();
				args++;
			}
#endif

			/* Create all the nodes/basic blocks */
			BasicBlock **bb = (BasicBlock **)calloc(nodes_size + 1, sizeof (BasicBlock *));
			for (m = 1; m < nodes_size; m++) {
				std::string node_string;
				std::stringstream tmp_str;
				tmp_str << "Node_0x" << std::hex << m;
				node_string = tmp_str.str();
				debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM2: %s\n", node_string.c_str());
				bb[m] = BasicBlock::Create(Context, node_string, declaration[n].F);
			}

			/* Create the AllocaInst's */
			/* labels[0] should be empty and is a invalid value to errors can be caught. */
			for (m = 1; m < labels_size; m++) {
				int size_bits;
				/* local_stack */
				if ((labels[m].scope == 1) && 
					(labels[m].type == 2)) {
					tmp = label_to_string(&labels[m], buffer, 1023);
					if (labels[m].lab_pointer && labels[m].pointer_type == 2) {
						size_bits = labels[m].pointer_type_size_bits;
						debug_print(DEBUG_OUTPUT_LLVM, 1, "Creating alloca for ptr to int label 0x%x, size_bits = 0x%x\n", m, size_bits);
						AllocaInst* ptr_local = new AllocaInst(IntegerType::get(mod->getContext(), size_bits), buffer, bb[1]);
						ptr_local->setAlignment(size_bits >> 3);
						value[m] = ptr_local;
					} else {
						size_bits = labels[m].pointer_type_size_bits;
						debug_print(DEBUG_OUTPUT_LLVM, 1, "Creating alloca for ptr to ptr label 0x%x, size_bits = 0x%x\n", m, size_bits);
						PointerType* PointerTy_1 = PointerType::get(IntegerType::get(mod->getContext(), size_bits), 0);
						AllocaInst* ptr_local = new AllocaInst(PointerTy_1, buffer, bb[1]);
						ptr_local->setAlignment(size_bits >> 3);
						value[m] = ptr_local;
					}
				}
			}
				
			/* FIXME: this needs the node to follow paths so the value[] is filled in the correct order */
			debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM: starting nodes\n");
			for (m = 1; m < nodes_size; m++) {
				debug_print(DEBUG_OUTPUT_LLVM, 1, "JCD12: node:0x%x: next_size = 0x%x\n", m, nodes[m].next_size);
			};
			for (node = 1; node < nodes_size; node++) {
				debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM: PHI PHASE 1: node=0x%x\n", node);

				/* Output PHI instructions first */
				for (m = 0; m < nodes[node].phi_size; m++) {
					int value_id = nodes[node].phi[m].value_id;
					int size_bits;
					int value_id1;
					int redirect_value_id;
					int first_previous_node;
					PHINode* phi_node;
					debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM:phi 0x%x, value_id = 0x%x, reg=0x%x\n", m, value_id, nodes[node].phi[m].reg);
					if (!value_id) {
						debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: labels[value_id]: value_id = 0. \n");
						exit(1);
					}
					tmp = label_to_string(&labels[value_id], buffer, 1023);
					if (labels[value_id].lab_pointer) {
						size_bits = labels[m].pointer_type_size_bits;
						/* FIXME:size 8 */
						if (!size_bits) size_bits = 8;
						PointerType* PointerTy_1 = PointerType::get(IntegerType::get(mod->getContext(), size_bits), 0);
						phi_node = PHINode::Create(PointerTy_1,
							nodes[node].phi[m].phi_node_size,
							buffer, bb[node]);
						value[value_id] = phi_node;
					} else {
						size_bits = labels[value_id].size_bits;
						debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM phi base size = 0x%x\n", size_bits);
						phi_node = PHINode::Create(IntegerType::get(mod->getContext(), size_bits),
							nodes[node].phi[m].phi_node_size,
							buffer, bb[node]);
						value[value_id] = phi_node;
					}
					value_id1 = nodes[node].phi[m].phi_node[0].value_id;
					if (!value_id1) {
						debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: labels_redirect[value_id1]: value_id = 0. \n");
						exit(1);
					}
					redirect_value_id = label_redirect[value_id1].redirect;
					first_previous_node = nodes[node].phi[m].phi_node[0].first_prev_node;
					debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM phi value_id1 = 0x%x, fpn = 0x%x\n", redirect_value_id, first_previous_node);
					sprint_value(OS1, value[value_id]);
					debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
					Buf1.clear();
					sprint_value(OS1, value[redirect_value_id]);
					debug_print(DEBUG_OUTPUT_LLVM, 1, "%s\n", Buf1.c_str());
					Buf1.clear();
					if (redirect_value_id > 0) {
						phi_node->addIncoming(value[redirect_value_id], bb[first_previous_node]);
					}
					/* The rest of the PHI instruction is added later */
				}
				LLVM_ir_export::add_node_instructions(self, mod, declaration, value, bb, node, n);
			}

			for (node = 1; node < nodes_size; node++) {
				debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM: PHI PHASE 2: node=0x%x\n", node);

				for (m = 0; m < nodes[node].phi_size; m++) {
					int size_bits = labels[nodes[node].phi[m].value_id].size_bits;
					debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM:phi 0x%x\n", m);
					debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM phi base size = 0x%x\n", size_bits);
					PHINode* phi_node = (PHINode*)value[nodes[node].phi[m].value_id];
					/* l = 0 has already been handled */
					for (l = 1; l < nodes[node].phi[m].phi_node_size; l++) {
						int value_id;
						int redirect_value_id;
						int first_previous_node;
						value_id = nodes[node].phi[m].phi_node[l].value_id;
						if (!value_id) {
							debug_print(DEBUG_OUTPUT_LLVM, 0, "ERROR: labels_redirect[value_id]: value_id = 0. \n");
							exit(1);
						}
						redirect_value_id = label_redirect[value_id].redirect;
						first_previous_node = nodes[node].phi[m].phi_node[l].first_prev_node;
						debug_print(DEBUG_OUTPUT_LLVM, 1, "LLVM:phi 0x%x:0x%x FPN=0x%x, SN=0x%x, value_id=0x%x, redirected_value_id=0x%x, size=0x%lx\n",
							m, l,
							nodes[node].phi[m].phi_node[l].first_prev_node,
							nodes[node].phi[m].phi_node[l].node,
							value_id,
							redirect_value_id,
							labels[redirect_value_id].size_bits);
						if (value_id > 0) {
							phi_node->addIncoming(value[redirect_value_id], bb[first_previous_node]);
						}
					}
				}
			}
			std::string ErrorInfo;
			std::error_code error_code;
			raw_fd_ostream OS(output_filename, error_code, llvm::sys::fs::F_None);

			if (error_code) {
				// *ErrorMessage = strdup(error_code.message().c_str());
				return -1;
			}

			WriteBitcodeToFile(mod, OS);
			delete mod;
		}
	}

	return 0;
}

int LLVM_ir_export_entry(struct self_s *self)
{
	int tmp;
	LLVM_ir_export object;
	tmp = object.output(self);
	return tmp;
}

extern "C" int llvm_export(struct self_s *self)
{
	int tmp;
	tmp = LLVM_ir_export_entry(self);
	return tmp;
}

