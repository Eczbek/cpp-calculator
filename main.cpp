#include <cstddef>
#include <cstdio>
#include <format>
#include <print>
#include <stdexcept>
#include <string_view>
#include <vector>
#include "../xieite/include/xieite/io/read.hpp"
#include "../xieite/include/xieite/math/parse_number.hpp"

using namespace std::literals;

int main() {
	std::println("{}", ([](std::string_view str = xieite::read(stdin, '\n')) static -> double {
		struct [[nodiscard]] error : std::runtime_error {
			error(std::string_view str, std::size_t pos)
			: std::runtime_error(std::format("\x1B[91merror: at {}: unexpected '{}'\x1B[0m", pos, str[pos])) {}
	
			error(std::string_view str, std::size_t pos, std::string_view expected)
			: std::runtime_error(std::format("\x1B[91merror: at {}: expected {}{}\x1B[0m", pos, expected, (pos < str.size()) ? std::format(", got '{}'", str[pos]) : ""s)) {}
		};
		std::size_t pos = 0;
		auto whitespace = [str, &pos] mutable -> void {
			while ((pos < str.size()) && " \t\n"sv.contains(str[pos])) {
				++pos;
			}
		};
		const double value = ([str, &pos, whitespace](this auto paren) -> double {
			return ([paren, str, &pos, whitespace] mutable -> double {
				enum struct types {
					literal,
					plus,
					dash,
					star,
					slash
				};
				struct token {
					types type;
					double value = 0;
				};
				auto unary = [paren, str, &pos, whitespace] mutable -> double {
					bool neg = false;
					for (;; pos++) {
						whitespace();
						if (pos < str.size()) {
							if (str[pos] == '+') {
								continue;
							}
							if (str[pos] == '-') {
								neg ^= 1;
								continue;
							}
						}
						break;
					}
					double value;
					whitespace();
					if ((pos < str.size()) && (str[pos] == '(')) {
						++pos;
						value = paren();
						whitespace();
						if ((pos >= str.size()) || (str[pos++] != ')')) {
							throw error(str, pos, "')'");
						}
					} else {
						whitespace();
						const std::size_t start = pos;
						bool has_dot = false;
						for (; pos < str.size(); ++pos) {
							if ((str[pos] >= '0') && (str[pos] <= '9')) {
								continue;
							}
							if (!has_dot && (str[pos] == '.')) {
								has_dot = true;
								continue;
							}
							break;
						}
						if (((pos < str.size()) && (str[pos] == '.')) || (pos == start)) {
							throw error(str, pos, "digit");
						}
						value = xieite::parse_number<double>(str.substr(start, pos - start));
					}
					return neg ? -value : value;
				};
				std::vector<token> tokens = { token(types::literal, unary()) };
				([&] -> void {
					while (true) {
						whitespace();
						if (pos >= str.size()) {
							break;
						}
						switch (str[pos]) while (true) {
							++pos;
							tokens.emplace_back(types::literal, unary());
							break;
						case '+':
							tokens.emplace_back(types::plus);
							continue;
						case '-':
							tokens.emplace_back(types::dash);
							continue;
						case '*':
							tokens.emplace_back(types::star);
							continue;
						case '/':
							tokens.emplace_back(types::slash);
							continue;
						default:
							return;
						}
					}
				})();
				return ([&tokens] -> double {
					auto factor = [&tokens](std::size_t& i) -> double {
						double left = tokens[i - 1].value;
						for (; i < tokens.size(); i += 2) {
							if (tokens[i].type == types::star) {
								left *= tokens[i + 1].value;
							} else if (tokens[i].type == types::slash) {
								left /= tokens[i + 1].value;
							} else {
								break;
							}
						}
						return left;
					};
					std::size_t i = 1;
					double left = factor(i);
					while (i < tokens.size()) {
						if (tokens[i].type == types::plus) {
							left += factor(i += 2);
						} else { // types::dash
							left -= factor(i += 2);
						}
					}
					return left;
				})();
			})();
		})();
		whitespace();
		if (pos < str.size()) {
			throw error(str, pos);
		}
		return value;
	})());
}
