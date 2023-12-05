var useless_nonterminal = "H";
var start_term = "A";
var epsilon = "Є"; // Для правила S -> e

var rules = [ // Пример 3.4
	["A", "1B"],
	["A", "1"],
	["B", "0B"],
	["B", "0"],
	["B", "1B"],
	["B", "1"],

	// пример эпсилон-правила:
	// ["S", epsilon],
];


// --------- //

var NTrules = {}; // "a": [ "aB", "a" ], ...

function isTerminal(symbol) {
	return !(/[A-Z]/.test(symbol));
}

var ntermRegex = /[A-Z]/
var termRegex = /[^A-Z]/

for (var rule of rules) {
	var cur;
	NTrules[rule[0]] = cur = (NTrules[rule[0]] ?? []);

	cur.push(rule[1]);
}

// 1. Пополнить грамматику правилом A -> aH для каждого правила вида A -> a (если нет A -> aB; B - любой НТ)

var allTerms = {}; // Терминалы; понадобятся в следующих этапе

for (var LHS in NTrules) {
	var outs = NTrules[LHS];

	var needNew = true;
	var termsOnly = [];

	for (var out of outs) {
		// собираем все терминалы на будущее
		var terms = out.match(new RegExp(termRegex, "g"));
		for (var term of terms)
			allTerms[term] = true;


		var is1Term = isTerminal(out[0]);

		if (is1Term && !isTerminal(out[1]) && out.length == 2) {
			// Вывод вида "aA", значит пополнять грамматику не нужно
			needNew = false;
			break;
		}

		if (is1Term && out.length == 1) {
			// Вывод из одного лишь терминала; возможно, понадобится A -> aH
			termsOnly.push(out);
		}
	}

	if (needNew) {
		for (var term of termsOnly) {
			// Для каждого вывода из одного лишь терминала, добавляем ещё одно правило вида aH
			outs.push(term + useless_nonterminal)
		}
	}
}

// 2. Преобразование правил в функции переходов дельта
function pushDelta(deltas, inTerm, inNonterm, outNonterm) {
	console.assert(isTerminal(inTerm));
	console.assert(!isTerminal(inNonterm));
	console.assert(!isTerminal(outNonterm));

	for (var dlt of deltas) {
		// убого, ну да ладно
		if (dlt[0] == inTerm && dlt[1] == inNonterm && dlt[2] == outNonterm) {
			return;
		}
	}

	deltas.push([inTerm, inNonterm, outNonterm]);
}


var finishStates = {}; // Заключительные состояния
var deltas = [
	// ["a", "A", "B"] означает "dlt(a, A) = B"
];

for (var LHS in NTrules) {
	var outs = NTrules[LHS];

	// Пары правил; если существуют оба правила <терминал><Нетерминал> и <терминал>,
	// то наш входной нетерминал (LHS) будет заключительным состояние

	var rightNTs = {}; // список правых нетерминалов ("A": true)
	var haveTermOnly = false; // существует ли правило вида <терминал>

	for (var out of outs) {
		if (isTerminal(out[0]) && !isTerminal(out[1]) && out.length == 2) {
			// Правило вида <терминал><Нетерминал>
			pushDelta(deltas, out[0], LHS, out[1]);

			rightNTs[out[1]] = true;
		}

		if (isTerminal(out[0]) && out.length == 1) {
			// Правило вида <терминал>
			haveTermOnly = true;
		}

		if (LHS === start_term && out === epsilon) {
			// Правило вида S -> e
			finishStates[LHS] = true;
		}
	}

	// 3. Формирование заключительных состояний
	if (haveTermOnly) {
		for (var RT in rightNTs) {
			finishStates[RT] = true;
		}
	}
}


// 4. Если автомат недетерм., то преобразовываем (Алгоритм Томпсона)
var queue = [start_term]
var detStates = {};
var newDeltas = [];

while (queue[0]) {
	var state = queue.shift();

	// Цикл по всем терминалам; смотрим, в какое состояние приведёт
	// переход из `state` по терминалу term
	for (var term in allTerms) {
		for (var dlt of deltas) { // O(n)... ну и ладно
			if (dlt[0] == term && dlt[1] == state) {
				// Нашли переход dlt(term, state) -> newState
				var newState = dlt[2];

				// Если еще не встречалось, пройдёмся циклом по новому состоянию тоже
				if (!detStates[newState]) {
					queue.push(newState);
					detStates[newState] = true;
				}

				// Добавляем новый дельта-переход
				pushDelta(newDeltas, term, state, newState);
			}
		}
	}
}

console.log("Дельта-функции:")
for (var dlt of newDeltas) {
	console.log(`δ(${dlt[0]}, ${dlt[1]}) = ${dlt[2]}`)
}

var finStatesStr = "";
for (var fin in finishStates) {
	finStatesStr += fin + ", ";
}

finStatesStr = finStatesStr.substr(0, finStatesStr.length - 2)

console.log("\nЗаключительные состояния: { " + finStatesStr + " }");