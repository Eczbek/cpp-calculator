
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>


struct Thing {
	char operation = 0;
	double value = 0;
};

int main() {
	std::string inputString;
	std::getline(std::cin, inputString);
	std::stringstream input(inputString);
	std::vector<Thing> things;

	Thing thing;
	input >> thing.value;
	things.push_back(thing);
	while (input) {
		Thing thing;
		input >> thing.operation >> thing.value;
		things.push_back(thing);
	}
	things.erase(things.end() - 1);

	for (const std::set<char>& operations : std::vector<std::set<char>> { { '*', '/', '%' }, { '+', '-' } })
		for (std::vector<Thing>::iterator i = things.begin(); i != things.end(); ++i) {
			const Thing current = *i;
			if (operations.contains(current.operation)) {
				things.erase(i--);
				Thing& previous = *i;
				switch (current.operation) {
				case '+':
					previous.value += current.value;
					break;
				case '-':
					previous.value -= current.value;
					break;
				case '*':
					previous.value *= current.value;
					break;
				case '/':
					previous.value /= current.value;
					break;
				}
			}
		}
	std::cout << things[0].value << '\n';
}
