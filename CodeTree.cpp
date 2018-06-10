/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
* All rights reserved. */

#include "CodeTree.h"
#include <set>

using namespace std;

struct CodeWord {
	CodeWord() {
		weight = 0;
		tree = 0;
	}
	bool operator<(const CodeWord& other) const {
		return weight < other.weight;
	}

	int weight;
	CodeTree* tree;
	string value;
};

set<CodeWord> SelectGroup(set<CodeWord>& codewords) {
	set<CodeWord> out;
	return out;
}

int WeighGroup(const set<CodeWord>& words) {
	int out = 0;
	for (auto w : words) {
		out += w.weight;
	}
	return out;
}

CodeTree* CodifyGroup(const set<CodeWord>& words) {
	if (words.size() == 1 && words.begin()->tree) {
		return words.begin()->tree;
	}
	CodeTree* out = new CodeTree();
	if (words.size() == 1) {
		out->value = words.begin()->value;
		return out;
	}
	for (auto w : words) {
		CodeTree* c = w.tree;
		if (!c) {
			c = new CodeTree();
			c->value = w.value;
		}
		out->child.push_back(c);
	}
	return out;
}

CodeTree* CodeTree::Create(const map<string, int>& histogram) {
	set<CodeWord> codewords;
	for (auto x : histogram) {
		CodeWord word;
		word.value = x.first;
		word.weight = x.second;
		codewords.insert(word);
	}
	while (codewords.size() > 1) {
		auto branch = SelectGroup(codewords);
		CodeWord word;
		word.tree = CodifyGroup(branch);
		word.weight = WeighGroup(branch);
		codewords.insert(word);
	}
	if (!codewords.size()) return 0;
	return codewords.begin()->tree;
}
