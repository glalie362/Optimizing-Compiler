#include "bb.hpp"
#include <stdexcept>

void BasicBlockGenerator::module(const Module& mod) {
	for (const auto& [fn_name, fn] : mod.functions) {
		auto blocks = function(fn);
		link_blocks(blocks, fn);
		fn_to_bbs[fn_name] = blocks;
	}
}

std::vector<BasicBlock> BasicBlockGenerator::function(const Function& fn) {
	std::vector<BasicBlock> blocks;
	bool push_block = true;

	for (BasicBlock* bb{}; const auto& ins : fn.insts) {
		if (push_block) {
			bb = &blocks.emplace_back();
			push_block = false;

			if (ins.opcode == Opcode::Label) {
				bb->lbl_entry = ins.operands[0];
			} else {
				throw std::runtime_error("internal error: BB's should always begin with a label");
			}
		}

		bb->inst.push_back(ins);

		if (ins.is_block_terminator()) {
			push_block = true;
			continue;
		}
	}

	return blocks;
}

void BasicBlockGenerator::link_blocks(std::vector<BasicBlock>& blocks, const Function& fn) {
	std::unordered_map<LabelId, BasicBlock*> lbl_to_bb;

	using enum Opcode;

	for (auto& bb : blocks) {
		for (const auto& ins : bb.inst) {
			if (ins.opcode == Label) {
				lbl_to_bb[ins.operands[0]] = &bb;
				continue;
			}
		}
	}

	// Look at the last instruction to connect the blocks
	for (std::size_t i = 0; i < blocks.size(); ++i) {
		auto& bb = blocks[i];
		if (bb.inst.empty()) continue;
		auto& ins = bb.inst.back();

		if (ins.opcode == Jump) {
			bb.successors.push_back(lbl_to_bb.at(ins.operands[0]));
			continue;
		}
		if (ins.opcode == Branch) {
			bb.successors.push_back(lbl_to_bb.at(ins.operands[1]));
			bb.successors.push_back(lbl_to_bb.at(ins.operands[2]));
			continue;
		}
		if (ins.opcode == Return) {
			bb.successors.push_back(lbl_to_bb.at(fn.epi_lbl));
			continue;
		} else {
			if (i + 1 < blocks.size()) {
				bb.successors.push_back(&blocks[i + 1]);
			}
		}
	}
}

const std::vector<BasicBlock>& BasicBlockGenerator::get_fn_bbs(const std::string& function_name) const {
	return fn_to_bbs.at(function_name);
}
