#include"x64.hpp"
#include <format>

// Grand allocation table

struct AllocationStrategy {
	std::optional<X64::ValueLifetime> lifetime = std::nullopt;
	std::vector<bool> consumption{};

	AllocationStrategy(const std::string& strategy) {
		switch (strategy[0]) {
			case 'p': lifetime = X64::ValueLifetime::Persistent; break;
			case 't': lifetime = X64::ValueLifetime::Temporary; break;
			case 's': lifetime = X64::ValueLifetime::Scratch; break;
			case 'x': break;
			default:
			throw std::runtime_error("undefined destination strategy");
		}

		for (int i = 1; i < strategy.size(); ++i) {
			if (strategy[i] == 'y') {
				consumption.push_back(true);
			} else if (strategy[i] == 'n') {
				consumption.push_back(false);
			} else {
				throw std::runtime_error("undefined consumption strategy");
			}
		}
	}
};

const static std::unordered_map<Opcode, AllocationStrategy> alloc_strategy = {
	{Opcode::Alloc,				{"p"}},
	{Opcode::Const,				{"tn"}},
	{Opcode::Store,				{"xny"}},
	{Opcode::Load,				{"tn"}},
	{Opcode::Add,				{"tny"}},
	{Opcode::Sub,				{"tny"}},
	{Opcode::Lesser,			{"tny"}},
	{Opcode::LesserOrEqual,		{"tny"}},
	{Opcode::Greater,			{"tny"}},
	{Opcode::GreaterOrEqual,	{"tny"}},
	{Opcode::Equal,				{"tny"}},
	{Opcode::NotEqual,			{"tny"}},
	{Opcode::And,				{"tny"}},
	{Opcode::Or,				{"tny"}},
	{Opcode::Xor,				{"tny"}},
	{Opcode::Label,				{"xn"}},
	{Opcode::Branch,			{"xynn"}},
	{Opcode::Jump,				{"xn"}},
	{Opcode::Return,			{"xn"}},
};

void X64::module() {
	textstream << "bits 64\n";
	textstream << "section .text\n";
	for (const auto& [fn_name, fn] : ir.mod.functions) {
		textstream << "global " << fn_name << '\n';
		function(fn_name, fn);
	}
}

void X64::function(const std::string& name, const Function& fn) {
	const auto lifetime = [&](const ValueId value_id) {
		if (ir.constants.contains(value_id)) {
			return ValueLifetime::Temporary;
		} else {
			return ValueLifetime::Persistent;
		}
	};

	// pass 1 - allocate
	for (const auto& inst : fn.insts) {
		const auto& strat = alloc_strategy.at(inst.opcode);
		const auto& maybe_lifetime = strat.lifetime;
		if (!maybe_lifetime) continue;
		//produces(inst.result, *maybe_lifetime);
	}

	const auto align_16 = [](const auto value) {
		return (value + 15) & ~std::size_t(15);
	};

	textstream << name << ":\n";
	textstream << "\tpush rbp\n";
	textstream << "\tmov rbp, rsp\n";
	if (stack_size) {
		textstream << "\tsub rbp, " << align_16(stack_size) - 8 << '\n';
	}

	mc.clear();

	// pass 2 - generate machine code
	for (const auto& inst : fn.insts) {
		instruction(inst);
	}

	// Optimization
	optimize();

	// Assembly
	emit(mc);
}

constexpr static X64::Operand reg(const X64::Reg r, const ValueId value_id = NoValue) {
	return X64::Operand::make_reg(r, value_id);
};

void X64::instruction(const Inst& inst) {
	const auto& strat = alloc_strategy.at(inst.opcode);

	if (strat.lifetime && *strat.lifetime != ValueLifetime::Scratch) {
		alloc_on_demand(inst.result);
	}

	using enum Reg;
	using enum Opcode;
	const auto result = [&]() { return operand(inst.result); };
	const auto inst_operand = [&](int index) { return operand(inst.operands[index]); };
	const auto src = [&]() { return inst_operand(0); };
	const auto lhs = [&]() { return inst_operand(0); };
	const auto rhs = [&]() { return inst_operand(1); };


	const auto cmp = [&](auto&& set_reg_to_result) {
		push_mc(MC::mov(reg(rax), lhs()));
		push_mc(MC::cmp(reg(rax), rhs()));
		push_mc(set_reg_to_result(reg(al)));
		push_mc(MC::movzx(reg(rax), reg(al)));
		push_mc(MC::mov(result(), reg(rax)));
	};

	switch (inst.opcode) {
		case Alloc: break;
		case Const: {
			const auto& v = ir.constants.at(inst.result);
			std::visit([&](auto&& val) {
				push_mc(MC::mov(operand(inst.result), Operand::make_imm(val)));
			}, v.data);
		} break;
		case Store: push_mc(MC::mov(inst_operand(0), inst_operand(1)));  break;
		case Load:
		push_mc(MC::mov(reg(rax), inst_operand(0)));
		push_mc(MC::mov(result(), reg(rax)));
		break;
		case Add:
		push_mc(MC::mov(reg(rax), inst_operand(0)));
		push_mc(MC::add(reg(rax), inst_operand(1)));
		push_mc(MC::mov(result(), reg(rax)));
		break;
		case Sub:
		push_mc(MC::mov(reg(rax), inst_operand(0)));
		push_mc(MC::sub(reg(rax), inst_operand(1)));
		push_mc(MC::mov(result(), reg(rax)));
		break;
		case Lesser: cmp(MC::setl); break;
		case LesserOrEqual: cmp(MC::setle); break;
		case Greater: cmp(MC::setg); break;
		case GreaterOrEqual: cmp(MC::setge); break;
		case Equal: cmp(MC::sete); break;
		case NotEqual: cmp(MC::setne); break;
		case And: break;
		case Or: break;
		case Xor: break;
		case Label: push_mc(MC::label(inst.operands[0])); break;
		case Branch:
		push_mc(MC::mov(reg(rax), src()));
		push_mc(MC::test(reg(rax), reg(rax)));
		push_mc(MC::jnz(Operand::make_imm(inst.operands[1])));
		push_mc(MC::jz(Operand::make_imm(inst.operands[2])));
		break;
		case Jump:
		push_mc(MC::jmp(Operand::make_imm(inst.operands[0])));
		break;
		case Return:
		if (inst.operands[0] != NoValue) {
			push_mc(MC::ret(inst_operand(0)));
		} else {
			push_mc(MC::ret());
		}
		break;
	}

	// second pass: consumption
	for (int i = 0; i <inst.operands.size(); ++i) {
		if (strat.consumption[i]) {
			consume(inst.operands[i]);
		}
	}
}

void X64::push_mc(const MC& mci) {
	mc.push_back(mci);
}

void X64::produces(const ValueId value_id, const ValueLifetime lifetime) {
	if (locations.contains(value_id)) return;

	if (lifetime == ValueLifetime::Scratch) {
		// scratch value are produced on demand
		// alloc_reg(value_id, Reg::rax, lifetime);
		return;
	}

	// prefer to use volatile registers
	for (const auto vol_reg : volatile_regs) {
		// rax is totally scratch and should never be used as a variable
		if (vol_reg == Reg::rax) continue;
		if (claimed_regs.contains(vol_reg)) continue;
		alloc_reg(value_id, vol_reg, lifetime);
		return;
	}

	// otherwise use callee saved
	for (const auto cs_reg : callee_saved_regs) {
		if (claimed_regs.contains(cs_reg)) continue;
		alloc_reg(value_id, cs_reg, lifetime);
		return;
	}

	// if we fail to allocate a reg, spill to the stack
	alloc_stack(value_id, lifetime);
}

void X64::consume(const ValueId value_id) {
	if (!locations.contains(value_id)) return;
	auto& l = locations.at(value_id);
	if (l.lifetime == ValueLifetime::Persistent) return;

	if (l.kind == ValueLocation::Kind::Reg) {
		claimed_regs.erase(l.loc.reg);
		locations.erase(value_id);
	}
}

X64::TypeSize X64::type_size(const AST::Type& type) {
	if (!type.qualifiers.empty()) {
		for (const auto& qual : type.qualifiers)
			if (qual.kind == AST::Type::Qualifier::Kind::Pointer)
				return { .elem_size = RegSize::Qword, .num_bytes = 8, .is_array = false };
	}

	if (type.name == "char")  return { .elem_size = RegSize::Byte, .num_bytes = 1, .is_array = false };
	if (type.name == "short") return { .elem_size = RegSize::Word, .num_bytes = 2, .is_array = false };
	if (type.name == "int")   return { .elem_size = RegSize::Dword, .num_bytes = 4, .is_array = false };
	if (type.name == "long")  return { .elem_size = RegSize::Qword, .num_bytes = 8, .is_array = false };
	return { .elem_size = RegSize::Byte, .num_bytes = 1, .is_array = false };
}

void X64::alloc_stack(const ValueId value_id, const ValueLifetime lifetime) {
	auto& value = ir.values[value_id];
	stack_size += type_size(value.type).num_bytes;
	locations[value_id] = {
		.kind = ValueLocation::Kind::Stack,
		.loc = stack_size,
		.lifetime = lifetime };
}

void X64::alloc_reg(const ValueId value_id, const Reg reg, const ValueLifetime lifetime) {
	if (claimed_regs.contains(reg)) {
		throw std::runtime_error(std::format("internal error: {} is already claimed", reg_to_string(reg)));
	}
	claimed_regs[reg] = value_id;
	locations[value_id] = {
		.kind = ValueLocation::Kind::Reg,
		.loc = (int)reg,
		.lifetime = lifetime
	};
}

bool X64::is_temporary(const ValueId value_id) {
	if (!locations.contains(value_id)) {
		return false;
	}
	return locations.at(value_id).lifetime == ValueLifetime::Temporary;
}

void X64::alloc_on_demand(const ValueId value_id) {
	// First, try the volatile regs
	for (auto r : volatile_regs) {
		if (r == Reg::rax) continue;
		if (!claimed_regs.contains(r)) {
			alloc_reg(value_id, r, ValueLifetime::Temporary);
			return;
		}
	}

	// Then the callee saved
	for (auto r : callee_saved_regs) {
		if (!claimed_regs.contains(r)) {
			alloc_reg(value_id, r, ValueLifetime::Temporary);
			return;
		}
	}

	// Spill
	alloc_stack(value_id, ValueLifetime::Temporary);
}
X64::Operand X64::operand(const ValueId value_id) {
	if (ir.constants.contains(value_id)) {
		// It must be a constant
		return std::visit([&](auto&& v) {
			return Operand::make_imm((int64_t)v, value_id);
		}, ir.constants.at(value_id).data);
	}


	if (locations.contains(value_id)) {
		// must have a location
		const auto& loc = locations.at(value_id);
		if (loc.kind == ValueLocation::Kind::Reg) {
			return Operand::make_reg((Reg)loc.loc.reg, value_id);
		} else {
			return Operand::make_mem(loc.loc.stack, value_id);
		}
	}
	throw std::runtime_error("internal error: use of unallocated value");
}

std::string X64::emit(const Operand& op) {
	switch (op.kind) {
		case Operand::Kind::Reg:
		return reg_to_string(op.reg);
		case Operand::Kind::Mem:
		return std::format("[rbp-{}]", op.stack);
		case Operand::Kind::Imm:
		return std::to_string(op.imm);
	}
	//std::unreachable();
}

void X64::emit(const std::vector<MC> &mc) {
	using enum MC::Opcode;
	using std::format;
	auto& t = textstream;

	for (const auto& ins : mc) {
		switch (ins.op) {
			// Mov
			case Mov:
			if (ins.dst->value_id != NoValue && ins.dst->is_mem()) {
				t << format("\tmov {} {}, {}\n", type_size(ir.values.at(ins.dst->value_id).type).str(), emit(*ins.dst), emit(*ins.src)); break;
			} else {
				t << format("\tmov {}, {}\n", emit(*ins.dst), emit(*ins.src)); break;
			}
			break;

			case MovZx:	t << format("\tmovzx {}, {}\n", emit(*ins.dst), emit(*ins.src)); break;
				// Math
			case Add:	t << format("\tadd {}, {}\n", emit(*ins.dst), emit(*ins.src)); break;
			case Sub:	t << format("\tsub {}, {}\n", emit(*ins.dst), emit(*ins.src)); break;
			case Inc:	t << format("\tinc {}\n", emit(*ins.src)); break;
			case Dec:	t << format("\tdec {}\n", emit(*ins.src)); break;
				// Logic
			case And:	t << format("\tand {}, {}\n", emit(*ins.dst), emit(*ins.src)); break;
			case Or:	t << format("\tor {}, {}\n", emit(*ins.dst), emit(*ins.src)); break;
			case Xor:	t << format("\txor {}, {}\n", emit(*ins.dst), emit(*ins.src)); break;
				// Cmps
			case Cmp:	t << format("\tcmp {}, {}\n", emit(*ins.lhs), emit(*ins.rhs)); break;
			case Test:	t << format("\ttest {}, {}\n", emit(*ins.lhs), emit(*ins.rhs)); break;
				// Setxx
			case Setl:	t << format("\tsetl {}\n", emit(*ins.dst)); break;
			case Setle:	t << format("\tsetle {}\n", emit(*ins.dst)); break;
			case Setg:	t << format("\tsetg {}\n", emit(*ins.dst)); break;
			case Setge:	t << format("\tsetge {}\n", emit(*ins.dst)); break;
			case Sete:	t << format("\tsete {}\n", emit(*ins.dst)); break;
			case Setne:	t << format("\tsetne {}\n", emit(*ins.dst)); break;
				// Jumps
			case Jmp:	t << format("\tjmp .L{}\n", emit(*ins.dst)); break;
			case Jnz:	t << format("\tjnz .L{}\n", emit(*ins.dst)); break;
			case Jz:	t << format("\tjz .L{}\n", emit(*ins.dst)); break;
			case Jl:	t << format("\tjl .L{}\n", emit(*ins.dst)); break;
			case Jle:	t << format("\tjle .L{}\n", emit(*ins.dst)); break;
			case Jg:	t << format("\tjg .L{}\n", emit(*ins.dst)); break;
			case Jge:	t << format("\tjge .L{}\n", emit(*ins.dst)); break;
			case Je:	t << format("\tje .L{}\n", emit(*ins.dst)); break;
			case Jne:	t << format("\tjne .L{}\n", emit(*ins.dst)); break;
				// Decl
			case Label: t << format(".L{}:\n", *ins.lbl); break;
			case Ret:
			if (ins.src) {
				t << format("\tmov rax, {}\n", emit(*ins.src));
			}
			t << format("\tpop rbp\n");
			t << format("\tret\n");
			break;
			case Nop:	t << format("\tnop\n"); break;
		}
	}
}