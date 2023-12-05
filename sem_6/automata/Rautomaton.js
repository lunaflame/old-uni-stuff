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
		var rules = qeV[RHS];
		if (!rules) {
			rules = qeV[RHS] = [];
		}

		rules.push(LHS);

		(RHS.match(termRegex) || []).forEach((s) => {
			terms[s] = true;
		});
	}

	qeV[TERMS] = terms;

	return qeV;
}

var clk = 0; // for debug

function writeHistory(history, input, stack) {
	var inpStr = (input.join("") || "e").padEnd(6);
	history[clk - 1] = `Такт ${String(clk).padStart(3)} | ${inpStr} | ${stack.padEnd(8)}`;
}


// stack в данном автомате представлен строкой нежели массивом... так удобнее
function runAutomaton(input, stack, delta, history) {
	if (stack.length >= 50) {
		return false;
	}

	clk++;

	if (stack === ("$" + START_TERM) && input.length == 0) {
		writeHistory(history, input, stack);

		clk++;
		writeHistory(history, input, "e"); // красоты ради, запишем типа (r,e,e) состояние
		clk -= 2;
		return true;
	}

	// Попробуем перенос
	var newInput = input.slice(1);
	var newStack = stack + input[0];
	var ok = runAutomaton(newInput, newStack, delta, history);

	if (ok) { // Вывелось, круто
		writeHistory(history, input, stack);
		clk--;
		return true;
	}

	// Пробуем свёртку
	for (var LHS in delta) {
		if (stack.slice(-LHS.length) == LHS) {
			var RHS = delta[LHS];

			// Заменяем LHS в конце стека на RHS и пытаемся выводить дальше
			newStack = stack.substr(0, stack.length - LHS.length) + RHS;
			ok = runAutomaton(input, newStack, delta, history);

			if (ok) {
				writeHistory(history, input, stack);
				clk--;
				return true;
			}
		}
	}

	// Не нашли подходящих правил
	clk--;
	return false;
}

var dlt = buildDelta(rules);
var history = [];
var ok = runAutomaton( (INPUT).split(""), "$", dlt, history );

console.log("Выводится? =>", ok);
if (ok) {
	console.log(history);
}
