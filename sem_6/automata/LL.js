const EPSILON = "Є";

// Пример правила LL(1) грамматики
const rules1 = [
	["S", "aAaB", "bAbB"],
	["A", "S", "cb"],
	["B", "cB", "a"],
]

// Пример правила не-LL(1) грамматики
const rules2 = [
	["S", "A", "B"],
	["S", "aA", "a"],
	["B", "bB", "b"],
]

// Пример правила LL(1) грамматики с эпсилон-правилом
const rules3 = [
	["S", "BA"],
	["A", "aBA", EPSILON],
	["B", "DC"],
	["C", "bDC", EPSILON],
	["D", "g", "cSd"],
]

// Пример правила не-LL(1) грамматики с эпсилон-правилом
const rules4 = [
	["S", "aAb", "c"],
	["A", "SB"],
	["B", "bSB", EPSILON],
]

console.log("Is rules1 LL(1)? ->", isLL1(rules1))
console.log("Is rules2 LL(1)? ->", isLL1(rules2))
console.log("Is rules3 LL(1)? ->", isLL1(rules3))
console.log("Is rules4 LL(1)? ->", isLL1(rules4))


// ----------------- //

// Является ли символ терминалом?
function isTerminal(symbol) {
  return !(/[^a-z]/.test(symbol));
}

// Вытаскивает терминал из префикса если таковой есть (используется в FIRST_1)
/* Похоже, не пригодилось
function prefixTerm(symbols) {
	var match = symbols.match(/^[a-z]/)
	return match ? match[0] : null;
}
*/

// Просто слияние таблиц (объектов, как они называются в JS почемуто)
function merge(into, from, filter) {
	filter || (filter = {});
	for (var k in from) {
		if (!filter[k]) {
			into[k] = from[k];
		}
	}
}

var firstCache;
var followCache;

function resetCache() {
	firstCache = {};
	followCache = {};
}

function FIRST(rules, term) {
	if (firstCache[term]) {
		return firstCache[term];
	}

	if (!term.includes(EPSILON)) {
		term = term[0];
	}

	var out = firstCache[term] = {};

	if (isTerminal(term)) {
		out[term] = true;
		return firstCache[term];
	}

	// Проходим по всем правилам, где term - входной символ
	for (var rule of rules) {
		var LHS = rule[0];
		var RHS = rule[1];

		if (LHS != term)
			continue;

		// сливаем FIRST всех выходных символов
		for (var i = 0; i < RHS.length; i++) {
			var rhsTerm = RHS[i];

			// если выходной символ - эпсилон, тут заканчиваем
			if (rhsTerm == EPSILON) {
				out[EPSILON] = true;
				break;
			}

			var first = FIRST(rules, rhsTerm);
			merge(out, first, { [EPSILON]: true });

			// Если в FIRST встретился эпсилон, останавливаемся
			if (!first[EPSILON]) {
				break;
			}
		}
	}

	/*
	for (var term in out) {
		delete out[term];
		var pfx = prefixTerm(term);
		if (pfx) { // т.к. FIRST_1 то мы можем вытащить первый терминал
			out[pfx] = true;
		}
	}
	*/

	return out;
}


function FOLLOW(rules, term) {
	if (followCache[term]) {
		return followCache[term];
	}

	var out = followCache[term] = {};

	// Проходим по всем правилам, где наш входной символ встречается в выходе
	for (var rule of rules) {
		var LHS = rule[0];
		var RHS = rule[1];

		if (!RHS.includes(term))
			continue;

		// Проходим по всему выводу начиная с символа после входного
		// и для каждого символа кроме последнего, соединяем его FIRST с выходом нашего FOLLOW
		// Для последнего символа соединяем его FOLLOW с нашим
		var fIdx = RHS.indexOf(term) + 1;

		for (var i = fIdx; i <= RHS.length; i++) {
			if (i == RHS.length) {
				// Если этот последний символ не тот же, что и вход, то соединяем его FOLLOW
				// (иначе получится бесконечный цикл, например A -> bA)
				if (LHS != term) {
					merge(out, FOLLOW(rules, LHS));
				}
				break;
			}

			var sym = RHS[i];
			var first = FIRST(rules, sym);

			if (!first[EPSILON]) {
				merge(out, first);
				break;
			}

			merge(out, first, { [EPSILON]: true });
		}
	}

	return out;
}

function isLL1(rules, dbg) {
	resetCache();
	var hasEpsilon = false;

	// Сплющим правила вывода ( [ ["S", "ab", "B"] ] -> [ ["S", "ab"], ["S", "B"] ] )
	// Заодно проверим наличие эпсилонов в грамматике

	var flatRules = [];
	for (var rule of rules) {
		for (var i = 1; i < rule.length; i++) {
			flatRules.push([ rule[0], rule[i] ])

			if (rule[i].includes(EPSILON)) {
				hasEpsilon = true;
			}
		}
	}

	var seenTerms = {}; // [LHS]: { [first_term]: true },

	for (var rule of flatRules) {
		var LHS = rule[0];
		var RHS = rule[1];

		var ourSeen = seenTerms[LHS] ? seenTerms[LHS] : {};
		seenTerms[LHS] = ourSeen;

		var first = FIRST(flatRules, RHS);
		var follow = hasEpsilon ? FOLLOW(flatRules, LHS) : null;

		if (dbg) {
			console.log("Rule:", rule)
			console.log("\tFIRST:", first)
			console.log("\tFOLLOW:", follow)
		}

		// Если один из терминалов из FIRST уже встречался, то говорим что пересечение ненулевое, и это не LL(1)
		for (var term in first) {
			if (ourSeen[term]) {
				return false; // Такой терминал уже был из другого правила; ненулевое пересечение
			}

			// Если у нас был эпсилон, проверить что не было пересечений FOLLOW и FIRST
			if (hasEpsilon && follow[term]) {
				return false;
			}
		}

		// Пересечений нет, записываем
		for (var term in first) {
			ourSeen[term] = true;
		}
	}

	return true;
}
