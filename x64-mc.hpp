#pragma once

struct MC {
	enum class Opcode {
		// Storage
		Mov, MovZx, Push, Pop,
		// Maths
		Add, Sub,
		Inc, Dec,
		// Logic
		And, Or, Xor,
		// Comparisons
		Cmp, Test,
		Setl, Setle, Setg, Setge, Sete, Setne,
		// Branching
		Jmp,
		Jl,
		Jle,
		Jg,
		Jge,
		Je,
		Jne,
		Jnz,
		Jz,
		Label,
		Ret,
		Nop
	};

	Opcode op{};
	std::optional<Operand> dst = std::nullopt;
	std::optional<Operand> src = std::nullopt;
	std::optional<Operand> lhs = std::nullopt;
	std::optional<Operand> rhs = std::nullopt;
	std::optional<int> lbl = std::nullopt;

	constexpr bool is_binary_math_operation() const noexcept {
		using enum Opcode;
		switch (op) {
			case Add:
			case Sub:
			return true;
		}
		return false;
	}

	constexpr bool is_setxx() const noexcept {
		using enum Opcode;
		switch (op) {
			case Setl:
			case Setle:
			case Setg:
			case Setge:
			case Sete:
			case Setne:
			return true;
		}
		return false;
	}

	constexpr bool is_conditional_jump() const {
		using enum Opcode;
		switch (op) {
			case Jl:
			case Jle:
			case Jg:
			case Jge:
			case Je:
			case Jne:
			case Jnz:
			case Jz:
			return true;
		}
		return false;
	}

	constexpr MC setxx_to_jumpxx(const Operand& dst) const {
		if (!is_setxx()) {
			throw std::runtime_error("internal error: instruction is not setxx");
		}
		using enum Opcode;
		switch (op) {
			case Setl:  return jl(dst);
			case Setle: return jle(dst);
			case Setg:  return jg(dst);
			case Setge: return jge(dst);
			case Sete:  return je(dst);
			case Setne: return jne(dst);
		}
	}

	constexpr MC negated_jump() const {
		if (!is_conditional_jump()) {
			throw std::runtime_error("internal error: jmp instruction is not conditional");
		}
		using enum Opcode;
		switch (op) {
			case Jl: return jge(*dst);
			case Jle: return jg(*dst);
			case Jg: return jle(*dst);
			case Jge: return jl(*dst);
			case Jz: return jnz(*dst);
			case Jnz: return jz(*dst);
			case Je: return jne(*dst);
			case Jne: return je(*dst);
		}
	}

	constexpr static MC mov(const Operand& dst, const Operand& src) {
		return MC{ .op = Opcode::Mov, .dst = dst, .src = src };
	}

	constexpr static MC movzx(const Operand& dst, const Operand& src) {
		return MC{ .op = Opcode::MovZx, .dst = dst, .src = src };
	}

	constexpr static MC push(const Operand& src) {
		return MC{ .op = Opcode::Push, .src = src };
	}

	constexpr static MC pop(const Operand& src) {
		return MC{ .op = Opcode::Pop, .src = src };
	}

	// Maths
	constexpr static MC add(const Operand& dst, const Operand& src) {
		return MC{ .op = Opcode::Add, .dst = dst, .src = src };
	}

	constexpr static MC sub(const Operand& dst, const Operand& src) {
		return MC{ .op = Opcode::Sub, .dst = dst, .src = src };
	}

	constexpr static MC inc(const Operand& src) {
		return MC{ .op = Opcode::Inc, .src = src };
	}

	constexpr static MC dec(const Operand& src) {
		return MC{ .op = Opcode::Dec, .src = src };
	}

	// Logic
	constexpr static MC l_and(const Operand& dst, const Operand& src) {
		return MC{ .op = Opcode::And, .dst = dst, .src = src };
	}

	constexpr static MC l_or(const Operand& dst, const Operand& src) {
		return MC{ .op = Opcode::Or, .dst = dst, .src = src };
	}

	constexpr static MC l_xor(const Operand& dst, const Operand& src) {
		return MC{ .op = Opcode::Xor, .dst = dst, .src = src };
	}

	// Comparisons
	constexpr static MC cmp(const Operand& lhs, const Operand& rhs) {
		return MC{ .op = Opcode::Cmp, .lhs = lhs, .rhs = rhs };
	}

	constexpr static MC test(const Operand& lhs, const Operand& rhs) {
		return MC{ .op = Opcode::Test, .lhs = lhs, .rhs = rhs };
	}

	constexpr static MC setl(const Operand& dst) {
		return MC{ .op = Opcode::Setl, .dst = dst };
	}

	constexpr static MC setle(const Operand& dst) {
		return MC{ .op = Opcode::Setle, .dst = dst };
	}

	constexpr static MC setg(const Operand& dst) {
		return MC{ .op = Opcode::Setg, .dst = dst };
	}

	constexpr static MC setge(const Operand& dst) {
		return MC{ .op = Opcode::Setge, .dst = dst };
	}

	constexpr static MC sete(const Operand& dst) {
		return MC{ .op = Opcode::Sete, .dst = dst };
	}

	constexpr static MC setne(const Operand& dst) {
		return MC{ .op = Opcode::Setne, .dst = dst };
	}

	// Branches
	constexpr static MC jmp(const Operand& dst) {
		return MC{ .op = Opcode::Jmp, .dst = dst };
	}

	constexpr static MC jz(const Operand& dst) {
		return MC{ .op = Opcode::Jz, .dst = dst };
	}

	constexpr static MC jnz(const Operand& dst) {
		return MC{ .op = Opcode::Jnz, .dst = dst };
	}

	constexpr static MC jl(const Operand& dst) {
		return MC{ .op = Opcode::Jl, .dst = dst };
	}

	constexpr static MC jle(const Operand& dst) {
		return MC{ .op = Opcode::Jle, .dst = dst };
	}

	constexpr static MC jg(const Operand& dst) {
		return MC{ .op = Opcode::Jg, .dst = dst };
	}

	constexpr static MC jge(const Operand& dst) {
		return MC{ .op = Opcode::Jge, .dst = dst };
	}

	constexpr static MC je(const Operand& dst) {
		return MC{ .op = Opcode::Je, .dst = dst };
	}

	constexpr static MC jne(const Operand& dst) {
		return MC{ .op = Opcode::Jne, .dst = dst };
	}

	// Returns
	constexpr static MC ret() {
		return MC{ .op = Opcode::Ret };
	}

	constexpr static MC ret(const Operand& src) {
		return MC{ .op = Opcode::Ret, .src = src };
	}

	constexpr static MC label(const int l) {
		return MC{ .op = Opcode::Label, .lbl = l };
	}

	constexpr static MC nop() {
		return MC{ .op = Opcode::Nop };
	}
};