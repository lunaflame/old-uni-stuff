// Левосторонний автомат

// Специальный символ для массива всех терминалов (правило 2)
var TERMS = "Σ"
var START_TERM = "E"
var INPUT = "a+a*a"

var rules = [
	["E", "E+T"],
	["E", "T"],
	["T", "T*F"],
	["T", "F"],
	["F", "(E)"],
	["F", "a"],
]

// ------------

function isTerminal(symbol) {
	return !(/[A-Z]/.test(symbol));
}

var ntermRegex = /[A-Z]/
var termRegex = /[^A-Z]/

var delta = [];

function buildDelta(rules) {
	var qeV = {};
	var terms = {};

	for (var rule of rules) {
		var [LHS, RHS] = rule;
		var rules = qeV[LHS];
		if (!rules) {
			rules = qeV[LHS] = [];
		}

		rules.push(RHS);

		(RHS.match(termRegex) || []).forEach((s) => {
			terms[s] = true;
		});
	}

	qeV[TERMS] = terms;

	return qeV;
}

var clk = 0; // for debug

function writeHistory(history, input, stack) {
	history[clk - 1] = `Такт ${String(clk).padStart(3)} | ${input.join("").padEnd(6)} | ${stack.join("").padEnd(8)}`;
}

function runAutomaton(input, stack, delta, history) {
	if (stack.length >= 50) {
		return false;
	}

	if (stack.length == 0 && input.length == 0) {
		// Пустой вход и стек... выводится
		return true;
	}

	clk++;

	// console.log()

	var top = stack[0];
	if (isTerminal(top)) {
		// Терминал; убираем его же из входа
		if (input[0] !== top) {
			// В цепочке не тот же символ, что и на стеке... невалидная конструкция
			clk--;
			return false;
		}

		// Прогоняем следующий цикл уже без терминала (что на стеке, что на входе)
		var out = runAutomaton(input.slice(1), stack.slice(1), delta, history);

		if (out) {
			writeHistory(history, input, stack);
		}

		clk--;

		return out;
	} else {
		// Нетерминал, ищем правила и преобразуем
		if (!delta[top]) {
			// Правил нет => не парсится
			clk--;
			return false;
		}

		// Хз, пробуем все правила???
		for (var RHS of delta[top]) {
			var newStack = RHS.split(""); // Делим выхлоп правила на массив (E+T -> ["E", "+", "T"])
			newStack.push(...stack.slice(1)); // Сверху добавляем всё, кроме только что поглощённого терминала

			var success = runAutomaton(input, newStack, delta, history);
			if (success) {
				writeHistory(history, input, stack);
				clk--;
				return true; // ого получилось
			}
		}

		clk--;
		return false;
	}
}

var dlt = buildDelta(rules);
var history = [];
var ok = runAutomaton( (INPUT).split(""), [START_TERM], dlt, history );

console.log("Выводится? =>", ok);
if (ok) {
	console.log(history);
}
